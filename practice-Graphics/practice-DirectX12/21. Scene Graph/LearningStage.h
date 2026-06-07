#pragma once

// Learning goal: Render multiple objects with separate transforms using a parent-child hierarchy.
// Implementation status: Implemented.

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

struct SceneNode
{
    float WorldMatrix[4][4] = {};
    float Color[4] = {};
};

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT Scissor = {};
    float ClearColor[4] = { 0.04f, 0.04f, 0.10f, 1.0f };
    float TimeSeconds = 0.0f;
    std::array<SceneNode, 4> Nodes = {};
};

// Row-major 2D transform: scale * rotate around origin, then translate.
// mul(float4(v, 0, 1), M) in HLSL gives: x' = scale*(v.x*cos - v.y*sin) + tx
inline void BuildTransform2D(float out[4][4], float tx, float ty, float scale, float angle)
{
    const float c = cosf(angle) * scale;
    const float s = sinf(angle) * scale;
    out[0][0] = c;    out[0][1] = s;    out[0][2] = 0.0f; out[0][3] = 0.0f;
    out[1][0] = -s;   out[1][1] = c;    out[1][2] = 0.0f; out[1][3] = 0.0f;
    out[2][0] = 0.0f; out[2][1] = 0.0f; out[2][2] = 1.0f; out[2][3] = 0.0f;
    out[3][0] = tx;   out[3][1] = ty;   out[3][2] = 0.0f; out[3][3] = 1.0f;
}

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
        "CreateCommittedResource (upload) failed.");
    void* mapped = nullptr;
    D3D12_RANGE range = {};
    buf->Map(0, &range, &mapped);
    std::memcpy(mapped, data, static_cast<size_t>(bytes));
    buf->Unmap(0, nullptr);
    return buf;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // Root constants: 20 x float (float4x4 world + float4 color) at b0, VS-only
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues = 20;
    param.Constants.ShaderRegister = 0;
    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters = 1;
    rsDesc.pParameters = &param;
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> sig, sigErr;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
        sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature failed.");
    StageThrowIfFailed(
        device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.RootSignature)),
        "CreateRootSignature failed.");

    ComPtr<ID3DBlob> vs = CompileStageShader(L"shaders\\Scene Graph.hlsl", "VSMain", "vs_5_0");
    ComPtr<ID3DBlob> ps = CompileStageShader(L"shaders\\Scene Graph.hlsl", "PSMain", "ps_5_0");

    // Unit quad: vertices in object space (-1..1). World matrix scales them to NDC.
    struct Vertex { float x, y; };
    Vertex verts[] = { { -1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, -1.0f }, { -1.0f, -1.0f } };
    uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.VertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.IndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(Vertex) };
    stage.IndexBufferView  = { stage.IndexBuffer->GetGPUVirtualAddress(),  sizeof(idxs),  DXGI_FORMAT_R16_UINT };

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = stage.RootSignature.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.InputLayout = { elems, _countof(elems) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.PipelineState)), "CreateGraphicsPipelineState failed.");

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    const float t = stage.TimeSeconds;

    // ---- Parent (Sun): spins in place ----
    const float sunAngle = t * 0.4f;
    BuildTransform2D(stage.Nodes[0].WorldMatrix, 0.0f, 0.0f, 0.12f, sunAngle);
    stage.Nodes[0].Color[0] = 1.00f; stage.Nodes[0].Color[1] = 0.85f;
    stage.Nodes[0].Color[2] = 0.10f; stage.Nodes[0].Color[3] = 1.0f;

    // ---- Child (Planet): orbits the sun ----
    const float planetOrbit = t * 0.9f;
    const float px = cosf(planetOrbit) * 0.52f;
    const float py = sinf(planetOrbit) * 0.52f;
    BuildTransform2D(stage.Nodes[1].WorldMatrix, px, py, 0.06f, t * 2.0f);
    stage.Nodes[1].Color[0] = 0.20f; stage.Nodes[1].Color[1] = 0.50f;
    stage.Nodes[1].Color[2] = 0.95f; stage.Nodes[1].Color[3] = 1.0f;

    // ---- Grandchild (Moon): orbits the planet ----
    const float moonOrbit = t * 2.5f;
    const float mx = px + cosf(moonOrbit) * 0.18f;
    const float my = py + sinf(moonOrbit) * 0.18f;
    BuildTransform2D(stage.Nodes[2].WorldMatrix, mx, my, 0.025f, t * 5.0f);
    stage.Nodes[2].Color[0] = 0.80f; stage.Nodes[2].Color[1] = 0.80f;
    stage.Nodes[2].Color[2] = 0.80f; stage.Nodes[2].Color[3] = 1.0f;

    // ---- Second planet with its own moon ----
    const float p2Orbit = t * 0.55f + 2.1f;
    const float p2x = cosf(p2Orbit) * 0.38f;
    const float p2y = sinf(p2Orbit) * 0.38f;
    BuildTransform2D(stage.Nodes[3].WorldMatrix, p2x, p2y, 0.05f, t * 1.5f);
    stage.Nodes[3].Color[0] = 0.85f; stage.Nodes[3].Color[1] = 0.35f;
    stage.Nodes[3].Color[2] = 0.10f; stage.Nodes[3].Color[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* cmd = context.CommandList;
    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->SetGraphicsRootSignature(stage.RootSignature.Get());
    cmd->SetPipelineState(stage.PipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    cmd->IASetIndexBuffer(&stage.IndexBufferView);

    // Each draw call submits a different world matrix + color via root constants.
    for (const SceneNode& node : stage.Nodes)
    {
        cmd->SetGraphicsRoot32BitConstants(0, 16, node.WorldMatrix, 0);
        cmd->SetGraphicsRoot32BitConstants(0, 4,  node.Color,       16);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
