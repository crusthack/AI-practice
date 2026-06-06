#pragma once

// Learning goal: Create shader-visible descriptor heaps and bind CBV/SRV descriptors through descriptor tables.
// Implementation status: Implemented.

#include <windows.h>
#include <wrl/client.h>

#include <cmath>
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

struct SceneConstants
{
    float Offset[2];
    float Padding[2];
    float Tint[4];
};

struct LearningStageState
{
    float ClearColor[4] = { 0.12f, 0.14f, 0.17f, 1.0f };
    SceneConstants Constants = {};
    SceneConstants* MappedConstants = nullptr;
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12DescriptorHeap> CbvHeap;
    ComPtr<ID3D12Resource> ConstantBuffer;
    ComPtr<ID3D12Resource> VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
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
        return L"06. Descriptor Heap\\shaders\\Descriptor Heap.hlsl";
    }
    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"06. Descriptor Heap\\shaders\\Descriptor Heap.hlsl" : path.substr(0, slash + 1) + L"shaders\\Descriptor Heap.hlsl";
}

inline UINT Align256(UINT value)
{
    return (value + 255u) & ~255u;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    D3D12_DESCRIPTOR_RANGE cbvRange = {};
    cbvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvRange.NumDescriptors = 1;
    cbvRange.BaseShaderRegister = 0;
    cbvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.DescriptorTable.NumDescriptorRanges = 1;
    rootParameter.DescriptorTable.pDescriptorRanges = &cbvRange;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
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

    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = stage.RootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.PipelineState)), "CreateGraphicsPipelineState failed.");

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&stage.CbvHeap)), "CreateDescriptorHeap for CBV failed.");

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC constantDesc = {};
    constantDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    constantDesc.Width = Align256(sizeof(SceneConstants));
    constantDesc.Height = 1;
    constantDesc.DepthOrArraySize = 1;
    constantDesc.MipLevels = 1;
    constantDesc.SampleDesc.Count = 1;
    constantDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &constantDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.ConstantBuffer)), "Constant buffer creation failed.");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = stage.ConstantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = Align256(sizeof(SceneConstants));
    device->CreateConstantBufferView(&cbvDesc, stage.CbvHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(stage.ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedConstants)), "Constant buffer Map failed.");

    const Vertex vertices[] = {
        { { 0.0f, 0.52f, 0.0f }, { 1.0f, 0.30f, 0.25f, 1.0f } },
        { { 0.56f, -0.42f, 0.0f }, { 0.25f, 0.95f, 0.40f, 1.0f } },
        { { -0.56f, -0.42f, 0.0f }, { 0.25f, 0.50f, 1.0f, 1.0f } },
    };

    D3D12_RESOURCE_DESC vbDesc = constantDesc;
    vbDesc.Width = sizeof(vertices);
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.VertexBuffer)), "Vertex buffer creation failed.");
    void* mappedVertices = nullptr;
    StageThrowIfFailed(stage.VertexBuffer->Map(0, &readRange, &mappedVertices), "Vertex buffer Map failed.");
    std::memcpy(mappedVertices, vertices, sizeof(vertices));
    stage.VertexBuffer->Unmap(0, nullptr);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.StrideInBytes = sizeof(Vertex);
    stage.VertexBufferView.SizeInBytes = sizeof(vertices);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.ClearColor[0] = 0.12f;
    stage.ClearColor[1] = 0.14f;
    stage.ClearColor[2] = 0.17f;
    stage.ClearColor[3] = 1.0f;
    stage.Constants.Offset[0] = std::sinf(static_cast<float>(timeSeconds)) * 0.12f;
    stage.Constants.Offset[1] = 0.0f;
    stage.Constants.Padding[0] = 0.0f;
    stage.Constants.Padding[1] = 0.0f;
    stage.Constants.Tint[0] = 0.8f;
    stage.Constants.Tint[1] = 0.9f + 0.1f * std::sinf(static_cast<float>(timeSeconds) * 2.0f);
    stage.Constants.Tint[2] = 1.0f;
    stage.Constants.Tint[3] = 1.0f;
    std::memcpy(stage.MappedConstants, &stage.Constants, sizeof(SceneConstants));
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    context.CommandList->SetPipelineState(stage.PipelineState.Get());
    ID3D12DescriptorHeap* heaps[] = { stage.CbvHeap.Get() };
    context.CommandList->SetDescriptorHeaps(1, heaps);
    context.CommandList->SetGraphicsRootDescriptorTable(0, stage.CbvHeap->GetGPUDescriptorHandleForHeapStart());

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
    context.CommandList->RSSetViewports(1, &viewport);
    context.CommandList->RSSetScissorRects(1, &scissorRect);
    context.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.CommandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    context.CommandList->DrawInstanced(3, 1, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.ConstantBuffer)
    {
        stage.ConstantBuffer->Unmap(0, nullptr);
    }
    stage.VertexBuffer.Reset();
    stage.ConstantBuffer.Reset();
    stage.CbvHeap.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
