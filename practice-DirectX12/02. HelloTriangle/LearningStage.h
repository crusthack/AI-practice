#pragma once

// Learning goal: Build the first root signature and graphics pipeline state object, then draw a triangle.
// Implementation status: Implemented.

#include <cstdint>
#include <cstring>
#include <climits>
#include <stdexcept>
#include <string>

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

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
        return L"02. HelloTriangle\\shaders\\HelloTriangle.hlsl";
    }

    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L"02. HelloTriangle\\shaders\\HelloTriangle.hlsl";
    }

    return path.substr(0, slash + 1) + L"shaders\\HelloTriangle.hlsl";
}

struct Vertex
{
    float Position[3];
    float Color[4];
};

struct LearningStageState
{
    float ClearColor[4] = { 0.05f, 0.10f, 0.22f, 1.0f };
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
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

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT serializeResult = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob);
    if (FAILED(serializeResult))
    {
        throw std::runtime_error(errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");
    }

    StageThrowIfFailed(
        device->CreateRootSignature(
            0,
            signatureBlob->GetBufferPointer(),
            signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&stage.RootSignature)),
        "CreateRootSignature failed.");

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    const UINT compileFlags =
#if defined(_DEBUG)
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        0;
#endif

    StageThrowIfFailed(
        D3DCompileFromFile(
            StageShaderPath().c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VSMain",
            "vs_5_0",
            compileFlags,
            0,
            &vertexShader,
            &errorBlob),
        errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "Vertex shader compilation failed.");

    errorBlob.Reset();
    StageThrowIfFailed(
        D3DCompileFromFile(
            StageShaderPath().c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain",
            "ps_5_0",
            compileFlags,
            0,
            &pixelShader,
            &errorBlob),
        errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : "Pixel shader compilation failed.");

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

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

    const Vertex vertices[] = {
        { { 0.0f, 0.55f, 0.0f }, { 1.0f, 0.18f, 0.18f, 1.0f } },
        { { 0.55f, -0.45f, 0.0f }, { 0.18f, 0.85f, 0.32f, 1.0f } },
        { { -0.55f, -0.45f, 0.0f }, { 0.22f, 0.45f, 1.0f, 1.0f } },
    };

    const UINT vertexBufferSize = sizeof(vertices);
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = vertexBufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    StageThrowIfFailed(
        device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&stage.VertexBuffer)),
        "CreateCommittedResource for vertex buffer failed.");

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(stage.VertexBuffer->Map(0, &readRange, &mappedData), "Vertex buffer Map failed.");
    std::memcpy(mappedData, vertices, vertexBufferSize);
    stage.VertexBuffer->Unmap(0, nullptr);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.StrideInBytes = sizeof(Vertex);
    stage.VertexBufferView.SizeInBytes = vertexBufferSize;
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.05f;
    stage.ClearColor[1] = 0.10f;
    stage.ClearColor[2] = 0.22f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    context.CommandList->SetPipelineState(stage.PipelineState.Get());

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(context.Width);
    viewport.Height = static_cast<float>(context.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = static_cast<LONG>(context.Width);
    scissorRect.bottom = static_cast<LONG>(context.Height);

    context.CommandList->RSSetViewports(1, &viewport);
    context.CommandList->RSSetScissorRects(1, &scissorRect);
    context.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.CommandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);
    context.CommandList->DrawInstanced(3, 1, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
