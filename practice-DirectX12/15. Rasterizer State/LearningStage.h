#pragma once

// Learning goal: Change fill and culling behavior through rasterizer state in the PSO.
// Implementation status: Implemented.

#include <windows.h>
#include <wrl/client.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <d3d12.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

struct Vertex
{
    float Position[3];
    float Color[4];
};

struct LearningStageState
{
    float ClearColor[4] = { 0.09f, 0.12f, 0.14f, 1.0f };
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> SolidCullNonePso;
    ComPtr<ID3D12PipelineState> WireCullNonePso;
    ComPtr<ID3D12PipelineState> SolidBackCullPso;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t Width = 0;
    uint32_t Height = 0;
};

inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
    {
        throw std::runtime_error(message);
    }
}

inline std::wstring StageShaderPath()
{
    wchar_t exePath[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
    {
        return L"15. Rasterizer State\\shaders\\Rasterizer State.hlsl";
    }
    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"15. Rasterizer State\\shaders\\Rasterizer State.hlsl" : path.substr(0, slash + 1) + L"shaders\\Rasterizer State.hlsl";
}

inline void CreateUploadBuffer(ID3D12Device* device, const void* data, UINT64 size, ID3D12Resource** resource)
{
    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource)), "Upload buffer creation failed.");
    void* mapped = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed((*resource)->Map(0, &readRange, &mapped), "Upload buffer Map failed.");
    std::memcpy(mapped, data, static_cast<size_t>(size));
    (*resource)->Unmap(0, nullptr);
}

inline D3D12_GRAPHICS_PIPELINE_STATE_DESC MakePsoDesc(ID3D12RootSignature* rootSignature, ID3DBlob* vertexShader, ID3DBlob* pixelShader, const D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT inputLayoutCount, D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode)
{
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = fillMode;
    rasterizerDesc.CullMode = cullMode;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.InputLayout = { inputLayout, inputLayoutCount };
    desc.pRootSignature = rootSignature;
    desc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    desc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    desc.RasterizerState = rasterizerDesc;
    desc.BlendState = blendDesc;
    desc.DepthStencilState.DepthEnable = FALSE;
    desc.DepthStencilState.StencilEnable = FALSE;
    desc.SampleMask = UINT_MAX;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    return desc;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT serializeResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(serializeResult))
    {
        throw std::runtime_error(errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");
    }
    StageThrowIfFailed(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&stage.RootSignature)), "CreateRootSignature failed.");

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    const UINT compileFlags =
#if defined(_DEBUG)
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        0;
#endif
    StageThrowIfFailed(D3DCompileFromFile(StageShaderPath().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob), errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "Vertex shader compilation failed.");
    errorBlob.Reset();
    StageThrowIfFailed(D3DCompileFromFile(StageShaderPath().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob), errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "Pixel shader compilation failed.");

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC solidNone = MakePsoDesc(stage.RootSignature.Get(), vertexShader.Get(), pixelShader.Get(), inputLayout, _countof(inputLayout), D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);
    D3D12_GRAPHICS_PIPELINE_STATE_DESC wireNone = MakePsoDesc(stage.RootSignature.Get(), vertexShader.Get(), pixelShader.Get(), inputLayout, _countof(inputLayout), D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE);
    D3D12_GRAPHICS_PIPELINE_STATE_DESC solidBack = MakePsoDesc(stage.RootSignature.Get(), vertexShader.Get(), pixelShader.Get(), inputLayout, _countof(inputLayout), D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK);
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&solidNone, IID_PPV_ARGS(&stage.SolidCullNonePso)), "Solid cull-none PSO creation failed.");
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&wireNone, IID_PPV_ARGS(&stage.WireCullNonePso)), "Wire cull-none PSO creation failed.");
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&solidBack, IID_PPV_ARGS(&stage.SolidBackCullPso)), "Solid back-cull PSO creation failed.");

    const Vertex vertices[] = {
        { { -0.92f, 0.44f, 0.0f }, { 0.95f, 0.35f, 0.30f, 1.0f } },
        { { -0.46f, 0.44f, 0.0f }, { 0.95f, 0.80f, 0.25f, 1.0f } },
        { { -0.46f, -0.44f, 0.0f }, { 0.25f, 0.75f, 1.0f, 1.0f } },
        { { -0.92f, -0.44f, 0.0f }, { 0.25f, 0.95f, 0.45f, 1.0f } },
        { { -0.23f, 0.44f, 0.0f }, { 0.95f, 0.35f, 0.30f, 1.0f } },
        { { 0.23f, 0.44f, 0.0f }, { 0.95f, 0.80f, 0.25f, 1.0f } },
        { { 0.23f, -0.44f, 0.0f }, { 0.25f, 0.75f, 1.0f, 1.0f } },
        { { -0.23f, -0.44f, 0.0f }, { 0.25f, 0.95f, 0.45f, 1.0f } },
        { { 0.46f, 0.44f, 0.0f }, { 1.0f, 0.30f, 0.30f, 1.0f } },
        { { 0.92f, 0.44f, 0.0f }, { 1.0f, 0.80f, 0.25f, 1.0f } },
        { { 0.46f, -0.44f, 0.0f }, { 0.40f, 0.90f, 1.0f, 1.0f } },
        { { 0.92f, -0.44f, 0.0f }, { 0.40f, 1.0f, 0.50f, 1.0f } },
    };
    const uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10,
        9, 11, 10,
    };
    CreateUploadBuffer(device, vertices, sizeof(vertices), &stage.VertexBuffer);
    CreateUploadBuffer(device, indices, sizeof(indices), &stage.IndexBuffer);
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(vertices), sizeof(Vertex) };
    stage.IndexBufferView = { stage.IndexBuffer->GetGPUVirtualAddress(), sizeof(indices), DXGI_FORMAT_R16_UINT };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.09f;
    stage.ClearColor[1] = 0.12f;
    stage.ClearColor[2] = 0.14f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
    context.CommandList->RSSetViewports(1, &viewport);
    context.CommandList->RSSetScissorRects(1, &scissorRect);
    context.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.CommandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    context.CommandList->IASetIndexBuffer(&stage.IndexBufferView);
    context.CommandList->SetPipelineState(stage.SolidCullNonePso.Get());
    context.CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    context.CommandList->SetPipelineState(stage.WireCullNonePso.Get());
    context.CommandList->DrawIndexedInstanced(6, 1, 6, 0, 0);
    context.CommandList->SetPipelineState(stage.SolidBackCullPso.Get());
    context.CommandList->DrawIndexedInstanced(6, 1, 12, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.SolidBackCullPso.Reset();
    stage.WireCullNonePso.Reset();
    stage.SolidCullNonePso.Reset();
    stage.RootSignature.Reset();
}
