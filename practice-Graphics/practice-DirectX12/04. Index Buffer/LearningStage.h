#pragma once

// Learning goal: Draw shared-vertex geometry through an index buffer.
// Implementation status: Implemented.

#include <windows.h>
#include <wrl/client.h>

#include <climits>
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
    float ClearColor[4] = { 0.04f, 0.16f, 0.16f, 1.0f };
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
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
        return L"04. Index Buffer\\shaders\\Index Buffer.hlsl";
    }

    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L"04. Index Buffer\\shaders\\Index Buffer.hlsl";
    }

    return path.substr(0, slash + 1) + L"shaders\\Index Buffer.hlsl";
}

inline void CreateUploadBuffer(ID3D12Device* device, const void* data, UINT64 size, ID3D12Resource** resource)
{
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    StageThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource)), "CreateCommittedResource upload buffer failed.");

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed((*resource)->Map(0, &readRange, &mappedData), "Upload buffer Map failed.");
    std::memcpy(mappedData, data, static_cast<size_t>(size));
    (*resource)->Unmap(0, nullptr);
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

    // 12-sided polygon: vertex 0 is the white centre; vertices 1–12 are the outer ring.
    // 12 triangles (36 indices) fan from the centre, each slice a different rainbow hue.
    // Outer vertices lie on a circle of radius 0.56 at 30-degree intervals, starting at 90 degrees.
    const Vertex vertices[] = {
        { {  0.000f,  0.000f, 0.0f }, { 1.00f, 1.00f, 1.00f, 1.0f } }, // 0  centre
        { {  0.000f,  0.560f, 0.0f }, { 1.00f, 0.15f, 0.15f, 1.0f } }, // 1  red
        { {  0.280f,  0.485f, 0.0f }, { 1.00f, 0.55f, 0.15f, 1.0f } }, // 2  orange
        { {  0.485f,  0.280f, 0.0f }, { 1.00f, 0.90f, 0.15f, 1.0f } }, // 3  yellow
        { {  0.560f,  0.000f, 0.0f }, { 0.55f, 0.95f, 0.15f, 1.0f } }, // 4  lime
        { {  0.485f, -0.280f, 0.0f }, { 0.15f, 0.90f, 0.20f, 1.0f } }, // 5  green
        { {  0.280f, -0.485f, 0.0f }, { 0.15f, 0.90f, 0.55f, 1.0f } }, // 6  spring
        { {  0.000f, -0.560f, 0.0f }, { 0.15f, 0.85f, 0.90f, 1.0f } }, // 7  cyan
        { { -0.280f, -0.485f, 0.0f }, { 0.15f, 0.50f, 1.00f, 1.0f } }, // 8  azure
        { { -0.485f, -0.280f, 0.0f }, { 0.20f, 0.20f, 1.00f, 1.0f } }, // 9  blue
        { { -0.560f,  0.000f, 0.0f }, { 0.60f, 0.15f, 1.00f, 1.0f } }, // 10 violet
        { { -0.485f,  0.280f, 0.0f }, { 0.90f, 0.15f, 0.90f, 1.0f } }, // 11 magenta
        { { -0.280f,  0.485f, 0.0f }, { 1.00f, 0.15f, 0.55f, 1.0f } }, // 12 rose
    };
    const uint16_t indices[] = {
        0,  1,  2,   0,  2,  3,   0,  3,  4,   0,  4,  5,
        0,  5,  6,   0,  6,  7,   0,  7,  8,   0,  8,  9,
        0,  9, 10,   0, 10, 11,   0, 11, 12,   0, 12,  1,
    };

    CreateUploadBuffer(device, vertices, sizeof(vertices), &stage.VertexBuffer);
    CreateUploadBuffer(device, indices, sizeof(indices), &stage.IndexBuffer);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.StrideInBytes = sizeof(Vertex);
    stage.VertexBufferView.SizeInBytes = sizeof(vertices);

    stage.IndexBufferView.BufferLocation = stage.IndexBuffer->GetGPUVirtualAddress();
    stage.IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    stage.IndexBufferView.SizeInBytes = sizeof(indices);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.04f;
    stage.ClearColor[1] = 0.16f;
    stage.ClearColor[2] = 0.16f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    context.CommandList->SetPipelineState(stage.PipelineState.Get());

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
    context.CommandList->RSSetViewports(1, &viewport);
    context.CommandList->RSSetScissorRects(1, &scissorRect);
    context.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.CommandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    context.CommandList->IASetIndexBuffer(&stage.IndexBufferView);
    context.CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
