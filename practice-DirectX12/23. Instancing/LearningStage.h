#pragma once

// Learning goal: Draw many copies of the same mesh with per-instance data.
// Implementation status: Implemented.
// Implementation: 10x10 grid of colored quads drawn in a single DrawIndexedInstanced call.
// Two vertex buffer streams: stream 0 = per-vertex quad positions, stream 1 = per-instance offset + color.
// A single root constant (QuadScale) animates the size of all quads with a slow pulse.

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

struct InstanceData
{
    float OffsetX, OffsetY;
    float R, G, B, A;
};

static constexpr int kInstanceCount = 100;

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    ComPtr<ID3D12Resource> InstanceBuffer;

    InstanceData* MappedInstances = nullptr;

    D3D12_VERTEX_BUFFER_VIEW QuadVBV = {};
    D3D12_VERTEX_BUFFER_VIEW InstanceVBV = {};
    D3D12_INDEX_BUFFER_VIEW  IndexBufferView = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT     Scissor  = {};

    float ClearColor[4] = { 0.03f, 0.05f, 0.08f, 1.0f };
    float TimeSeconds = 0.0f;
};

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

// Convert hue (0..1) to an RGB color (full saturation/value).
inline void HsvToRgb(float h, float& r, float& g, float& b)
{
    const float s = 0.85f;
    const float v = 1.00f;
    const float hh = h * 6.0f;
    const int   i  = static_cast<int>(hh) % 6;
    const float f  = hh - static_cast<float>(static_cast<int>(hh));
    const float p  = v * (1.0f - s);
    const float q  = v * (1.0f - s * f);
    const float t  = v * (1.0f - s * (1.0f - f));
    switch (i)
    {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // Root signature: 1 root constant (QuadScale, 1 x float) at b0, visible to VS.
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues = 1;
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

    ComPtr<ID3DBlob> vs = CompileStageShader(L"shaders\\Instancing.hlsl", "VSMain", "vs_5_0");
    ComPtr<ID3DBlob> ps = CompileStageShader(L"shaders\\Instancing.hlsl", "PSMain", "ps_5_0");

    // Quad vertices: unit square in object space, positions only.
    struct QuadVertex { float x, y; };
    QuadVertex verts[] = {
        { -1.0f,  1.0f },
        {  1.0f,  1.0f },
        {  1.0f, -1.0f },
        { -1.0f, -1.0f },
    };
    uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.VertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.IndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.QuadVBV      = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(QuadVertex) };
    stage.IndexBufferView = { stage.IndexBuffer->GetGPUVirtualAddress(), sizeof(idxs), DXGI_FORMAT_R16_UINT };

    // Instance buffer: persistently mapped upload heap.
    {
        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        rd.Width = sizeof(InstanceData) * kInstanceCount;
        rd.Height = 1;
        rd.DepthOrArraySize = 1;
        rd.MipLevels = 1;
        rd.SampleDesc.Count = 1;
        rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        StageThrowIfFailed(
            device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.InstanceBuffer)),
            "Instance buffer creation failed.");
        D3D12_RANGE readRange = {};
        stage.InstanceBuffer->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedInstances));
    }
    stage.InstanceVBV = {
        stage.InstanceBuffer->GetGPUVirtualAddress(),
        static_cast<UINT>(sizeof(InstanceData) * kInstanceCount),
        sizeof(InstanceData)
    };

    // Pre-fill instance offsets and colors (offsets updated each frame, colors are constant).
    for (int row = 0; row < 10; ++row)
    {
        for (int col = 0; col < 10; ++col)
        {
            const int idx = row * 10 + col;
            // Center the 10x10 grid: indices -4.5..4.5 in each axis, spaced 0.18.
            stage.MappedInstances[idx].OffsetX = (col - 4.5f) * 0.18f;
            stage.MappedInstances[idx].OffsetY = (row - 4.5f) * 0.18f;
            // HSV sweep across all 100 instances.
            HsvToRgb(static_cast<float>(idx) / static_cast<float>(kInstanceCount),
                     stage.MappedInstances[idx].R,
                     stage.MappedInstances[idx].G,
                     stage.MappedInstances[idx].B);
            stage.MappedInstances[idx].A = 1.0f;
        }
    }

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        // Stream 0: per-vertex
        { "POSITION",        0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,   0 },
        // Stream 1: per-instance
        { "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_COLOR",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 8, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
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
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    // The per-instance offsets are static (colors + grid positions don't change).
    // Animation is purely driven by the QuadScale root constant pushed each frame in Render.
    // Nothing to update in the instance buffer here.
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    // Pulse: scale oscillates between 0.055 and 0.085.
    const float quadScale = 0.07f + 0.015f * sinf(stage.TimeSeconds * 1.5f);

    ID3D12GraphicsCommandList* cmd = context.CommandList;
    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->SetGraphicsRootSignature(stage.RootSignature.Get());
    cmd->SetGraphicsRoot32BitConstants(0, 1, &quadScale, 0);
    cmd->SetPipelineState(stage.PipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_VERTEX_BUFFER_VIEW vbvs[] = { stage.QuadVBV, stage.InstanceVBV };
    cmd->IASetVertexBuffers(0, 2, vbvs);
    cmd->IASetIndexBuffer(&stage.IndexBufferView);

    // Single draw call for all 100 instances.
    cmd->DrawIndexedInstanced(6, kInstanceCount, 0, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.MappedInstances)
    {
        stage.InstanceBuffer->Unmap(0, nullptr);
        stage.MappedInstances = nullptr;
    }
    stage.InstanceBuffer.Reset();
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
