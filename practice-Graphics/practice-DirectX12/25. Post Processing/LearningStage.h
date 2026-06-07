#pragma once

// Learning goal: Apply a full-screen post-processing pass to an offscreen render target.
// Implementation status: Implemented.
// Implementation:
//   Pass 1 (scene pass): render 4 rotating colored quads into an offscreen R8G8B8A8 texture.
//   Pass 2 (post pass):  fullscreen quad that samples the offscreen texture and applies
//                        a grayscale conversion, color-fade oscillation, and vignette darkening.
//
// Two root signatures, two PSOs, one offscreen texture + SRV/RTV heaps.

#include <wrl/client.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <d3d12.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

struct LearningStageSetupContext
{
    ID3D12Device* Device = nullptr;
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct PostSceneNode
{
    float WorldMatrix[4][4] = {};
    float Color[4] = {};
};

struct LearningStageState
{
    // Scene pass resources
    ComPtr<ID3D12RootSignature> SceneRootSignature;
    ComPtr<ID3D12PipelineState> ScenePipelineState;

    // Post pass resources
    ComPtr<ID3D12RootSignature> PostRootSignature;
    ComPtr<ID3D12PipelineState> PostPipelineState;

    // Shared geometry
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VBV = {};
    D3D12_INDEX_BUFFER_VIEW  IBV = {};

    // Offscreen render target
    ComPtr<ID3D12DescriptorHeap> OffscreenRtvHeap;
    ComPtr<ID3D12DescriptorHeap> SrvHeap;
    ComPtr<ID3D12Resource> OffscreenTexture;

    D3D12_CPU_DESCRIPTOR_HANDLE OffscreenRtv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE OffscreenSrv = {};

    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT     Scissor  = {};

    float BackBufferClearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
    float OffscreenClearColor[4]  = { 0.04f, 0.04f, 0.06f, 1.0f };

    float TimeSeconds = 0.0f;
    std::array<PostSceneNode, 4> Nodes = {};
};

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------
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
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
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

inline D3D12_RESOURCE_BARRIER MakeTransitionBarrier(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER b = {};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource   = resource;
    b.Transition.StateBefore = before;
    b.Transition.StateAfter  = after;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return b;
}

// Build a 2D row-major world matrix: scale * rotate, then translate.
inline void BuildTransform2D(float out[4][4], float tx, float ty, float scale, float angle)
{
    const float c = cosf(angle) * scale;
    const float s = sinf(angle) * scale;
    out[0][0] = c;    out[0][1] = s;    out[0][2] = 0.0f; out[0][3] = 0.0f;
    out[1][0] = -s;   out[1][1] = c;    out[1][2] = 0.0f; out[1][3] = 0.0f;
    out[2][0] = 0.0f; out[2][1] = 0.0f; out[2][2] = 1.0f; out[2][3] = 0.0f;
    out[3][0] = tx;   out[3][1] = ty;   out[3][2] = 0.0f; out[3][3] = 1.0f;
}

// ------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------
inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // ---- Scene root signature: 20 root constants (16 WorldMatrix + 4 Color) at b0, VS-visible ----
    {
        D3D12_ROOT_PARAMETER sceneParam = {};
        sceneParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        sceneParam.Constants.Num32BitValues = 20;
        sceneParam.Constants.ShaderRegister = 0;
        sceneParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 1;
        rsDesc.pParameters = &sceneParam;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (scene) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.SceneRootSignature)),
            "CreateRootSignature (scene) failed.");
    }

    // ---- Post root signature: SRV table (t0) + static linear sampler + 4 root constants (vignette params) ----
    {
        D3D12_DESCRIPTOR_RANGE srvRange = {};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER postParams[2] = {};
        postParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        postParams[0].DescriptorTable.NumDescriptorRanges = 1;
        postParams[0].DescriptorTable.pDescriptorRanges = &srvRange;
        postParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // 4 floats: ColorFade, VignetteInner, VignetteOuter, VignetteStrength
        postParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        postParams[1].Constants.Num32BitValues = 4;
        postParams[1].Constants.ShaderRegister = 0;
        postParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_STATIC_SAMPLER_DESC linearSampler = {};
        linearSampler.Filter   = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        linearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        linearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        linearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        linearSampler.ShaderRegister = 0;
        linearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 2;
        rsDesc.pParameters = postParams;
        rsDesc.NumStaticSamplers = 1;
        rsDesc.pStaticSamplers = &linearSampler;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (post) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.PostRootSignature)),
            "CreateRootSignature (post) failed.");
    }

    // ---- Compile shaders ----
    ComPtr<ID3DBlob> sceneVS = CompileStageShader(L"shaders\\Post Processing.hlsl", "SceneVS", "vs_5_0");
    ComPtr<ID3DBlob> scenePS = CompileStageShader(L"shaders\\Post Processing.hlsl", "ScenePS", "ps_5_0");
    ComPtr<ID3DBlob> postVS  = CompileStageShader(L"shaders\\Post Processing.hlsl", "PostVS",  "vs_5_0");
    ComPtr<ID3DBlob> postPS  = CompileStageShader(L"shaders\\Post Processing.hlsl", "PostPS",  "ps_5_0");

    // ---- Shared geometry: unit quad ----
    struct Vertex { float x, y; };
    Vertex verts[] = {
        { -1.0f,  1.0f },
        {  1.0f,  1.0f },
        {  1.0f, -1.0f },
        { -1.0f, -1.0f },
    };
    uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.VertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.IndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.VBV = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(Vertex) };
    stage.IBV = { stage.IndexBuffer->GetGPUVirtualAddress(),  sizeof(idxs),  DXGI_FORMAT_R16_UINT };

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ---- Scene PSO ----
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = stage.SceneRootSignature.Get();
        pso.VS = { sceneVS->GetBufferPointer(), sceneVS->GetBufferSize() };
        pso.PS = { scenePS->GetBufferPointer(), scenePS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout = { elems, _countof(elems) };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.ScenePipelineState)), "Scene PSO failed.");
    }

    // ---- Post PSO (fullscreen, no vertex buffer input) ----
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = stage.PostRootSignature.Get();
        pso.VS = { postVS->GetBufferPointer(), postVS->GetBufferSize() };
        pso.PS = { postPS->GetBufferPointer(), postPS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout = { nullptr, 0 };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.PostPipelineState)), "Post PSO failed.");
    }

    // ---- Offscreen RTV heap ----
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
        rtvDesc.NumDescriptors = 1;
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        StageThrowIfFailed(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&stage.OffscreenRtvHeap)), "Offscreen RTV heap failed.");
        stage.OffscreenRtv = stage.OffscreenRtvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    // ---- SRV heap (shader-visible) ----
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
        srvDesc.NumDescriptors = 1;
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        StageThrowIfFailed(device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&stage.SrvHeap)), "SRV heap failed.");
        stage.OffscreenSrv = stage.SrvHeap->GetGPUDescriptorHandleForHeapStart();
    }

    // ---- Offscreen texture ----
    {
        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rd.Width = context.Width;
        rd.Height = context.Height;
        rd.DepthOrArraySize = 1;
        rd.MipLevels = 1;
        rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rd.SampleDesc.Count = 1;
        rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        D3D12_CLEAR_VALUE cv = {};
        cv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        cv.Color[0] = stage.OffscreenClearColor[0];
        cv.Color[1] = stage.OffscreenClearColor[1];
        cv.Color[2] = stage.OffscreenClearColor[2];
        cv.Color[3] = stage.OffscreenClearColor[3];
        StageThrowIfFailed(
            device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_RENDER_TARGET, &cv, IID_PPV_ARGS(&stage.OffscreenTexture)),
            "Offscreen texture creation failed.");

        device->CreateRenderTargetView(stage.OffscreenTexture.Get(), nullptr, stage.OffscreenRtv);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(stage.OffscreenTexture.Get(), &srvDesc, stage.SrvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

// ------------------------------------------------------------------
// Update
// ------------------------------------------------------------------
inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    const float t = stage.TimeSeconds;

    // 4 rotating quads: cyan, magenta, yellow, white at different orbits and spin rates.
    // Node 0: Cyan, orbiting at radius 0.45, spinning fast.
    {
        const float ox = cosf(t * 0.8f) * 0.45f;
        const float oy = sinf(t * 0.8f) * 0.45f;
        BuildTransform2D(stage.Nodes[0].WorldMatrix, ox, oy, 0.14f, t * 2.0f);
        stage.Nodes[0].Color[0] = 0.0f; stage.Nodes[0].Color[1] = 0.9f;
        stage.Nodes[0].Color[2] = 0.9f; stage.Nodes[0].Color[3] = 1.0f;
    }
    // Node 1: Magenta, counter-orbit, mid-size.
    {
        const float ox = cosf(-t * 0.6f + 1.0f) * 0.35f;
        const float oy = sinf(-t * 0.6f + 1.0f) * 0.35f;
        BuildTransform2D(stage.Nodes[1].WorldMatrix, ox, oy, 0.11f, -t * 1.5f);
        stage.Nodes[1].Color[0] = 0.9f; stage.Nodes[1].Color[1] = 0.0f;
        stage.Nodes[1].Color[2] = 0.9f; stage.Nodes[1].Color[3] = 1.0f;
    }
    // Node 2: Yellow, slow outer orbit.
    {
        const float ox = cosf(t * 0.4f + 2.5f) * 0.60f;
        const float oy = sinf(t * 0.4f + 2.5f) * 0.60f;
        BuildTransform2D(stage.Nodes[2].WorldMatrix, ox, oy, 0.10f, t * 0.8f);
        stage.Nodes[2].Color[0] = 1.0f; stage.Nodes[2].Color[1] = 1.0f;
        stage.Nodes[2].Color[2] = 0.0f; stage.Nodes[2].Color[3] = 1.0f;
    }
    // Node 3: White, spinning in the centre.
    {
        BuildTransform2D(stage.Nodes[3].WorldMatrix, 0.0f, 0.0f, 0.08f, t * 3.0f);
        stage.Nodes[3].Color[0] = 1.0f; stage.Nodes[3].Color[1] = 1.0f;
        stage.Nodes[3].Color[2] = 1.0f; stage.Nodes[3].Color[3] = 1.0f;
    }
}

// ------------------------------------------------------------------
// Render
// ------------------------------------------------------------------
inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    const float t = stage.TimeSeconds;
    ID3D12GraphicsCommandList* cmd = context.CommandList;

    // ---- Pass 1: render scene into offscreen texture ----
    // Offscreen texture starts in RENDER_TARGET state (initial + end-of-frame restore).
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->OMSetRenderTargets(1, &stage.OffscreenRtv, FALSE, nullptr);
    cmd->ClearRenderTargetView(stage.OffscreenRtv, stage.OffscreenClearColor, 0, nullptr);

    cmd->SetGraphicsRootSignature(stage.SceneRootSignature.Get());
    cmd->SetPipelineState(stage.ScenePipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VBV);
    cmd->IASetIndexBuffer(&stage.IBV);

    for (const PostSceneNode& node : stage.Nodes)
    {
        cmd->SetGraphicsRoot32BitConstants(0, 16, node.WorldMatrix, 0);
        cmd->SetGraphicsRoot32BitConstants(0, 4,  node.Color,       16);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }

    // Transition offscreen texture: RENDER_TARGET -> PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_BARRIER toSrv = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd->ResourceBarrier(1, &toSrv);

    // ---- Pass 2: post-processing fullscreen pass ----
    // Post constants: ColorFade (oscillates 0..1), VignetteInner, VignetteOuter, VignetteStrength
    const float colorFade = 0.5f + 0.5f * sinf(t * 0.8f);
    const float postConstants[4] = { colorFade, 0.3f, 0.85f, 0.75f };

    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.BackBufferClearColor, 0, nullptr);

    ID3D12DescriptorHeap* heaps[] = { stage.SrvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);
    cmd->SetGraphicsRootSignature(stage.PostRootSignature.Get());
    cmd->SetGraphicsRootDescriptorTable(0, stage.OffscreenSrv);
    cmd->SetGraphicsRoot32BitConstants(1, 4, postConstants, 0);
    cmd->SetPipelineState(stage.PostPipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // Fullscreen triangle via vertex ID (no VB bound).
    cmd->DrawInstanced(3, 1, 0, 0);

    // Transition offscreen texture back: PIXEL_SHADER_RESOURCE -> RENDER_TARGET
    D3D12_RESOURCE_BARRIER toRt = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmd->ResourceBarrier(1, &toRt);
}

// ------------------------------------------------------------------
// Cleanup
// ------------------------------------------------------------------
inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.OffscreenTexture.Reset();
    stage.SrvHeap.Reset();
    stage.OffscreenRtvHeap.Reset();
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PostPipelineState.Reset();
    stage.PostRootSignature.Reset();
    stage.ScenePipelineState.Reset();
    stage.SceneRootSignature.Reset();
}
