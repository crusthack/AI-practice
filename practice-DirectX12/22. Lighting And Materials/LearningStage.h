#pragma once

// Learning goal: Per-pixel Phong lighting with per-object material properties.
// Implementation status: Implemented.
// Each object uses the same directional light but different Kd / Ks / shininess.
// Normals are derived spherically in the pixel shader from UV coordinates, demonstrating
// per-pixel lighting math without requiring full 3D geometry infrastructure.

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

// Aligns to 256 bytes as required for a real constant buffer view.
struct alignas(256) PerFrameConstants
{
    float LightDir[3];
    float _p0;
    float AmbientColor[3];
    float _p1;
    float TimeSeconds;
    float _p2[3];
};

struct MaterialDesc
{
    float DiffuseColor[3];
    float SpecularPower;
    float SpecularColor[3];
    float Roughness;
};

struct LightingObject
{
    float PosX, PosY, Scale;
    MaterialDesc Material;
};

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    ComPtr<ID3D12Resource> PerFrameCbv;
    ComPtr<ID3D12DescriptorHeap> CbvHeap;
    PerFrameConstants* MappedFrame = nullptr;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW  IndexBufferView  = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT     Scissor  = {};
    float ClearColor[4] = { 0.04f, 0.04f, 0.04f, 1.0f };
    float TimeSeconds = 0.0f;
    std::array<LightingObject, 3> Objects = {};
};

inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr)) throw std::runtime_error(message);
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

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // CBV heap for per-frame light data
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&stage.CbvHeap)), "CreateDescriptorHeap failed.");

    stage.PerFrameCbv = CreateUploadBuffer(device, nullptr, sizeof(PerFrameConstants));
    D3D12_RANGE readRange = {};
    stage.PerFrameCbv->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedFrame));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = stage.PerFrameCbv->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes    = sizeof(PerFrameConstants);
    device->CreateConstantBufferView(&cbvDesc, stage.CbvHeap->GetCPUDescriptorHandleForHeapStart());

    // Root signature:
    //   param 0: CBV descriptor table b0 — per-frame light + time
    //   param 1: 12 root constants b1 — per-object material + position/scale
    D3D12_DESCRIPTOR_RANGE cbvRange = {};
    cbvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvRange.NumDescriptors = 1;
    cbvRange.BaseShaderRegister = 0;
    cbvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER params[2] = {};
    params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[0].DescriptorTable.NumDescriptorRanges = 1;
    params[0].DescriptorTable.pDescriptorRanges = &cbvRange;
    params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // 12 root constants: DiffuseRGB(3) + SpecPow(1) + SpecularRGB(3) + Roughness(1) + PosXY(2) + Scale(1) + AspectH(1)
    params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    params[1].Constants.Num32BitValues = 12;
    params[1].Constants.ShaderRegister = 1;
    params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters = 2;
    rsDesc.pParameters = params;
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> sig, sigErr;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
        sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature failed.");
    StageThrowIfFailed(
        device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.RootSignature)),
        "CreateRootSignature failed.");

    ComPtr<ID3DBlob> vs = CompileStageShader(L"shaders\\Lighting And Materials.hlsl", "VSMain", "vs_5_0");
    ComPtr<ID3DBlob> ps = CompileStageShader(L"shaders\\Lighting And Materials.hlsl", "PSMain", "ps_5_0");

    // Shared unit quad (4 vertices, UV in [0,1])
    struct Vertex { float x, y, u, v; };
    Vertex verts[] = {
        { -1.0f,  1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f, 1.0f },
    };
    uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.VertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.IndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(Vertex) };
    stage.IndexBufferView  = { stage.IndexBuffer->GetGPUVirtualAddress(),  sizeof(idxs),  DXGI_FORMAT_R16_UINT };

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = stage.RootSignature.Get();
    pso.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    pso.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
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
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.PipelineState)), "CreateGraphicsPipelineState failed.");

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };

    // Gold, red plastic, cool metal
    stage.Objects[0] = { -0.52f, 0.0f, 0.18f, { { 0.92f, 0.74f, 0.22f }, 90.0f,  { 1.00f, 0.95f, 0.60f }, 0.15f } };
    stage.Objects[1] = {  0.00f, 0.0f, 0.18f, { { 0.85f, 0.22f, 0.12f }, 10.0f,  { 0.50f, 0.20f, 0.15f }, 0.80f } };
    stage.Objects[2] = {  0.52f, 0.0f, 0.18f, { { 0.65f, 0.78f, 0.92f }, 220.0f, { 0.90f, 0.95f, 1.00f }, 0.05f } };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    const float t = stage.TimeSeconds;
    const float lx = cosf(t * 0.5f) * 0.7f;
    const float ly = 0.6f;
    const float lz = sinf(t * 0.3f) * 0.5f + 0.5f;
    const float len = sqrtf(lx * lx + ly * ly + lz * lz);
    if (stage.MappedFrame)
    {
        stage.MappedFrame->LightDir[0] = lx / len;
        stage.MappedFrame->LightDir[1] = ly / len;
        stage.MappedFrame->LightDir[2] = lz / len;
        stage.MappedFrame->AmbientColor[0] = 0.05f;
        stage.MappedFrame->AmbientColor[1] = 0.05f;
        stage.MappedFrame->AmbientColor[2] = 0.07f;
        stage.MappedFrame->TimeSeconds = t;
    }
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* cmd = context.CommandList;
    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);

    ID3D12DescriptorHeap* heaps[] = { stage.CbvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);
    cmd->SetGraphicsRootSignature(stage.RootSignature.Get());
    cmd->SetGraphicsRootDescriptorTable(0, stage.CbvHeap->GetGPUDescriptorHandleForHeapStart());
    cmd->SetPipelineState(stage.PipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    cmd->IASetIndexBuffer(&stage.IndexBufferView);

    const float aspectH = static_cast<float>(context.Width) / static_cast<float>(context.Height);

    for (const LightingObject& obj : stage.Objects)
    {
        struct ObjectConstants
        {
            float DiffuseColor[3];
            float SpecularPower;
            float SpecularColor[3];
            float Roughness;
            float PosX, PosY, Scale, AspectH;
        } data;
        data.DiffuseColor[0]  = obj.Material.DiffuseColor[0];
        data.DiffuseColor[1]  = obj.Material.DiffuseColor[1];
        data.DiffuseColor[2]  = obj.Material.DiffuseColor[2];
        data.SpecularPower    = obj.Material.SpecularPower;
        data.SpecularColor[0] = obj.Material.SpecularColor[0];
        data.SpecularColor[1] = obj.Material.SpecularColor[1];
        data.SpecularColor[2] = obj.Material.SpecularColor[2];
        data.Roughness        = obj.Material.Roughness;
        data.PosX    = obj.PosX;
        data.PosY    = obj.PosY;
        data.Scale   = obj.Scale;
        data.AspectH = aspectH;
        cmd->SetGraphicsRoot32BitConstants(1, 12, &data, 0);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.MappedFrame)
    {
        stage.PerFrameCbv->Unmap(0, nullptr);
        stage.MappedFrame = nullptr;
    }
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PerFrameCbv.Reset();
    stage.CbvHeap.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
