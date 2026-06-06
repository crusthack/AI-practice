#pragma once

// Learning goal: Integrate Dear ImGui as a debug UI overlay for DX12 samples.
// Implementation status: Implemented.
//
// This stage demonstrates:
//   1. A SHADER_VISIBLE CBV_SRV_UAV descriptor heap — the kind ImGui needs for its font texture SRV.
//   2. A background scene of 3 rotating colored quads (world-matrix root constants, same as stage 21).
//   3. An alpha-blended overlay of 3 screen-space "debug panel" quads that simulate ImGui windows.
//
// Where actual ImGui calls would live is marked with comments throughout.

#include <wrl/client.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

using Microsoft::WRL::ComPtr;

// ---------------------------------------------------------------------------
// Context structs (same shape as all other implemented stages)
// ---------------------------------------------------------------------------

struct LearningStageSetupContext
{
    ID3D12Device* Device = nullptr;
    uint32_t Width  = 0;
    uint32_t Height = 0;
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t Width  = 0;
    uint32_t Height = 0;
};

// ---------------------------------------------------------------------------
// Stage state
// ---------------------------------------------------------------------------

struct LearningStageState
{
    // Background pass (3 rotating quads)
    ComPtr<ID3D12RootSignature> BackgroundRootSignature;
    ComPtr<ID3D12PipelineState> BackgroundPipelineState;

    // Overlay pass (3 semi-transparent debug panels)
    ComPtr<ID3D12RootSignature> OverlayRootSignature;
    ComPtr<ID3D12PipelineState> OverlayPipelineState;

    // Shared unit quad geometry (position + UV)
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW  IndexBufferView  = {};

    // SHADER_VISIBLE CBV_SRV_UAV heap — the kind ImGui needs for its font texture SRV.
    // ImGui::Init would register its font texture SRV here.
    ComPtr<ID3D12DescriptorHeap> SrvHeap;

    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT     Scissor  = {};

    float ClearColor[4] = { 0.07f, 0.10f, 0.13f, 1.0f };
    float TimeSeconds   = 0.0f;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
        throw std::runtime_error(message);
}

inline ComPtr<ID3DBlob> CompileStageShader(const wchar_t* path, const char* entry, const char* target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> code, errors;
    const HRESULT hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, flags, 0, &code, &errors);
    if (FAILED(hr))
        throw std::runtime_error(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Shader compile failed.");
    return code;
}

inline ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, const void* data, uint64_t bytes)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension         = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width             = bytes;
    desc.Height            = 1;
    desc.DepthOrArraySize  = 1;
    desc.MipLevels         = 1;
    desc.SampleDesc.Count  = 1;
    desc.Layout            = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> buf;
    StageThrowIfFailed(
        device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buf)),
        "CreateUploadBuffer failed.");
    if (data)
    {
        void* mapped = nullptr;
        D3D12_RANGE range = {};
        buf->Map(0, &range, &mapped);
        std::memcpy(mapped, data, static_cast<size_t>(bytes));
        buf->Unmap(0, nullptr);
    }
    return buf;
}

// Builds a row-major 2D affine transform: scale * rotation(angle) + translation.
// mul(float4(pos, 0, 1), M) in HLSL produces the transformed NDC position.
inline void BuildTransform2D(float out[4][4], float tx, float ty, float scale, float angle)
{
    const float c = cosf(angle) * scale;
    const float s = sinf(angle) * scale;
    out[0][0] = c;    out[0][1] = s;    out[0][2] = 0.0f; out[0][3] = 0.0f;
    out[1][0] = -s;   out[1][1] = c;    out[1][2] = 0.0f; out[1][3] = 0.0f;
    out[2][0] = 0.0f; out[2][1] = 0.0f; out[2][2] = 1.0f; out[2][3] = 0.0f;
    out[3][0] = tx;   out[3][1] = ty;   out[3][2] = 0.0f; out[3][3] = 1.0f;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // ------------------------------------------------------------------
    // SHADER_VISIBLE CBV_SRV_UAV heap.
    // This is the heap type ImGui registers its font texture SRV into.
    // ImGui::Init would register its font texture SRV here.
    // ------------------------------------------------------------------
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;    // one slot reserved for ImGui font texture
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        StageThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&stage.SrvHeap)),
            "CreateDescriptorHeap (SRV, shader-visible) failed.");
    }

    // ------------------------------------------------------------------
    // Background root signature — 20 root constants at b0 (world 4x4 + color 4)
    // ------------------------------------------------------------------
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param.Constants.Num32BitValues = 20;
        param.Constants.ShaderRegister = 0;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 1;
        rsDesc.pParameters   = &param;
        rsDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (bg) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.BackgroundRootSignature)),
            "CreateRootSignature (bg) failed.");
    }

    // ------------------------------------------------------------------
    // Overlay root signature — 8 root constants at b0: (x,y,w,h) + RGBA
    // ------------------------------------------------------------------
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param.Constants.Num32BitValues = 8;
        param.Constants.ShaderRegister = 0;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 1;
        rsDesc.pParameters   = &param;
        rsDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (overlay) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.OverlayRootSignature)),
            "CreateRootSignature (overlay) failed.");
    }

    // ------------------------------------------------------------------
    // Compile shaders
    // ------------------------------------------------------------------
    ComPtr<ID3DBlob> bgVS  = CompileStageShader(L"shaders\\ImGui Integration.hlsl", "BackgroundVS", "vs_5_0");
    ComPtr<ID3DBlob> bgPS  = CompileStageShader(L"shaders\\ImGui Integration.hlsl", "BackgroundPS", "ps_5_0");
    ComPtr<ID3DBlob> ovVS  = CompileStageShader(L"shaders\\ImGui Integration.hlsl", "OverlayVS",    "vs_5_0");
    ComPtr<ID3DBlob> ovPS  = CompileStageShader(L"shaders\\ImGui Integration.hlsl", "OverlayPS",    "ps_5_0");

    // ------------------------------------------------------------------
    // Shared unit quad: pos(-1..1) + UV(0..1), 4 vertices / 6 indices
    // ------------------------------------------------------------------
    struct QuadVertex { float x, y, u, v; };
    const QuadVertex verts[] = {
        { -1.0f,  1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f, 1.0f },
    };
    const uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.VertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.IndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(QuadVertex) };
    stage.IndexBufferView  = { stage.IndexBuffer->GetGPUVirtualAddress(),  sizeof(idxs),  DXGI_FORMAT_R16_UINT };

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ------------------------------------------------------------------
    // Background PSO — opaque, uses vertex buffer
    // ------------------------------------------------------------------
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature    = stage.BackgroundRootSignature.Get();
        pso.VS                = { bgVS->GetBufferPointer(), bgVS->GetBufferSize() };
        pso.PS                = { bgPS->GetBufferPointer(), bgPS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        // BlendEnable = FALSE (default) — opaque background quads
        pso.SampleMask        = UINT_MAX;
        pso.RasterizerState.FillMode       = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode       = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable  = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout       = { elems, _countof(elems) };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets  = 1;
        pso.RTVFormats[0]     = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count  = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.BackgroundPipelineState)),
            "CreateGraphicsPipelineState (background) failed.");
    }

    // ------------------------------------------------------------------
    // Overlay PSO — alpha blended, no vertex buffer (SV_VertexID driven)
    // ------------------------------------------------------------------
    {
        D3D12_RENDER_TARGET_BLEND_DESC rtBlend = {};
        rtBlend.BlendEnable    = TRUE;
        rtBlend.SrcBlend       = D3D12_BLEND_SRC_ALPHA;
        rtBlend.DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
        rtBlend.BlendOp        = D3D12_BLEND_OP_ADD;
        rtBlend.SrcBlendAlpha  = D3D12_BLEND_SRC_ALPHA;
        rtBlend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        rtBlend.BlendOpAlpha   = D3D12_BLEND_OP_ADD;
        rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0] = rtBlend;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature    = stage.OverlayRootSignature.Get();
        pso.VS                = { ovVS->GetBufferPointer(), ovVS->GetBufferSize() };
        pso.PS                = { ovPS->GetBufferPointer(), ovPS->GetBufferSize() };
        pso.BlendState        = blendDesc;
        pso.SampleMask        = UINT_MAX;
        pso.RasterizerState.FillMode       = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode       = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable  = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout       = { nullptr, 0 };  // no vertex buffer; VS uses SV_VertexID
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets  = 1;
        pso.RTVFormats[0]     = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count  = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.OverlayPipelineState)),
            "CreateGraphicsPipelineState (overlay) failed.");
    }

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    // ImGui::NewFrame() would go here — starting a new UI frame and processing input.
    stage.TimeSeconds = static_cast<float>(timeSeconds);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* cmd = context.CommandList;

    // Bind the SHADER_VISIBLE SRV heap so the GPU can access it.
    // In a real ImGui integration the font texture SRV lives in this heap.
    ID3D12DescriptorHeap* heaps[] = { stage.SrvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);

    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);

    // ------------------------------------------------------------------
    // Background pass — 3 rotating colored quads
    // ------------------------------------------------------------------
    cmd->SetGraphicsRootSignature(stage.BackgroundRootSignature.Get());
    cmd->SetPipelineState(stage.BackgroundPipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    cmd->IASetIndexBuffer(&stage.IndexBufferView);

    // Quad 0 — warm red, inner ring, fast spin
    {
        const float t  = stage.TimeSeconds;
        const float angle = t * 1.2f;
        float world[4][4] = {};
        BuildTransform2D(world, cosf(t * 0.7f) * 0.4f, sinf(t * 0.7f) * 0.4f, 0.12f, angle);
        const float color[4] = { 0.95f, 0.32f, 0.18f, 1.0f };
        cmd->SetGraphicsRoot32BitConstants(0, 16, world, 0);
        cmd->SetGraphicsRoot32BitConstants(0, 4,  color, 16);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    // Quad 1 — cyan, mid ring, medium spin
    {
        const float t  = stage.TimeSeconds;
        const float angle = t * 0.8f + 2.1f;
        float world[4][4] = {};
        BuildTransform2D(world, cosf(t * 0.45f + 1.0f) * 0.55f, sinf(t * 0.45f + 1.0f) * 0.55f, 0.10f, angle);
        const float color[4] = { 0.18f, 0.78f, 0.95f, 1.0f };
        cmd->SetGraphicsRoot32BitConstants(0, 16, world, 0);
        cmd->SetGraphicsRoot32BitConstants(0, 4,  color, 16);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    // Quad 2 — gold, outer ring, slow spin
    {
        const float t  = stage.TimeSeconds;
        const float angle = t * 0.5f + 4.2f;
        float world[4][4] = {};
        BuildTransform2D(world, cosf(t * 0.3f + 2.5f) * 0.70f, sinf(t * 0.3f + 2.5f) * 0.70f, 0.09f, angle);
        const float color[4] = { 0.95f, 0.80f, 0.15f, 1.0f };
        cmd->SetGraphicsRoot32BitConstants(0, 16, world, 0);
        cmd->SetGraphicsRoot32BitConstants(0, 4,  color, 16);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }

    // ------------------------------------------------------------------
    // Overlay pass — 3 semi-transparent "debug panel" quads in screen-space NDC.
    //
    // ImGui_ImplDX12_RenderDrawData() would go here — submitting the ImGui
    // draw commands that were recorded since ImGui::NewFrame().
    //
    // Panel layout (NDC coords, origin = center, Y up):
    //   Panel 0 — FPS counter window  : top-left,     semi-transparent gray
    //   Panel 1 — Stats window        : bottom-right,  semi-transparent blue
    //   Panel 2 — Render settings     : top-right,     semi-transparent dark
    // ------------------------------------------------------------------
    cmd->SetGraphicsRootSignature(stage.OverlayRootSignature.Get());
    cmd->SetPipelineState(stage.OverlayPipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Each panel: float[8] = { ndcX_left, ndcY_top, ndcW, ndcH,  R, G, B, A }
    struct PanelDesc { float x, y, w, h, r, g, b, a; };
    const PanelDesc panels[] = {
        // Panel 0 — top-left:     FPS counter window  (gray,  alpha 0.70)
        { -1.00f,  1.00f,  0.40f, 0.22f,   0.55f, 0.55f, 0.58f, 0.70f },
        // Panel 1 — bottom-right: stats window        (blue,  alpha 0.65)
        {  0.50f, -0.64f,  0.50f, 0.36f,   0.12f, 0.30f, 0.70f, 0.65f },
        // Panel 2 — top-right:    render settings     (dark,  alpha 0.75)
        {  0.56f,  1.00f,  0.44f, 0.30f,   0.10f, 0.10f, 0.14f, 0.75f },
    };

    for (const PanelDesc& p : panels)
    {
        float constants[8] = { p.x, p.y, p.w, p.h, p.r, p.g, p.b, p.a };
        cmd->SetGraphicsRoot32BitConstants(0, 8, constants, 0);
        cmd->DrawInstanced(6, 1, 0, 0);  // 6 vertices from SV_VertexID
    }
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.SrvHeap.Reset();
    stage.OverlayPipelineState.Reset();
    stage.OverlayRootSignature.Reset();
    stage.BackgroundPipelineState.Reset();
    stage.BackgroundRootSignature.Reset();
}
