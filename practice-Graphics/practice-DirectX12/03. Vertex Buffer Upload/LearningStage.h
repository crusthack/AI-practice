#pragma once

// Learning goal: Upload vertex data into GPU resources and bind a vertex buffer view.
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
    float ClearColor[4] = { 0.10f, 0.08f, 0.24f, 1.0f };
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
        return L"03. Vertex Buffer Upload\\shaders\\Vertex Buffer Upload.hlsl";
    }

    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L"03. Vertex Buffer Upload\\shaders\\Vertex Buffer Upload.hlsl";
    }

    return path.substr(0, slash + 1) + L"shaders\\Vertex Buffer Upload.hlsl";
}

inline void WaitForStageQueue(ID3D12CommandQueue* queue, ID3D12Fence* fence, HANDLE fenceEvent, uint64_t fenceValue)
{
    StageThrowIfFailed(queue->Signal(fence, fenceValue), "Stage queue Signal failed.");
    if (fence->GetCompletedValue() < fenceValue)
    {
        StageThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent), "Stage fence SetEventOnCompletion failed.");
        WaitForSingleObject(fenceEvent, INFINITE);
    }
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

    // Hexagonal colour wheel: 6 triangles fanning from the origin.
    // Each triangle shares two adjacent outer vertices (p[i], p[i+1]) and one white centre.
    // Outer vertices sit on a circle of radius 0.58 at 60-degree intervals (flat-top hex, p0 at 90 degrees).
    // p0=(0, 0.58)  p1=(0.502, 0.29)  p2=(0.502,-0.29)  p3=(0,-0.58)  p4=(-0.502,-0.29)  p5=(-0.502, 0.29)
    const Vertex vertices[] = {
        // Triangle 0: centre–p0–p1 (red → yellow)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.000f,  0.580f, 0.0f }, { 1.0f, 0.15f, 0.15f, 1.0f } },
        { {  0.502f,  0.290f, 0.0f }, { 1.0f, 0.90f, 0.15f, 1.0f } },
        // Triangle 1: centre–p1–p2 (yellow → green)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.502f,  0.290f, 0.0f }, { 1.0f, 0.90f, 0.15f, 1.0f } },
        { {  0.502f, -0.290f, 0.0f }, { 0.20f, 0.90f, 0.20f, 1.0f } },
        // Triangle 2: centre–p2–p3 (green → cyan)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.502f, -0.290f, 0.0f }, { 0.20f, 0.90f, 0.20f, 1.0f } },
        { {  0.000f, -0.580f, 0.0f }, { 0.15f, 0.85f, 0.90f, 1.0f } },
        // Triangle 3: centre–p3–p4 (cyan → blue)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.000f, -0.580f, 0.0f }, { 0.15f, 0.85f, 0.90f, 1.0f } },
        { { -0.502f, -0.290f, 0.0f }, { 0.20f, 0.30f, 1.00f, 1.0f } },
        // Triangle 4: centre–p4–p5 (blue → magenta)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.502f, -0.290f, 0.0f }, { 0.20f, 0.30f, 1.00f, 1.0f } },
        { { -0.502f,  0.290f, 0.0f }, { 0.85f, 0.20f, 0.90f, 1.0f } },
        // Triangle 5: centre–p5–p0 (magenta → red, wraps)
        { {  0.000f,  0.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.502f,  0.290f, 0.0f }, { 0.85f, 0.20f, 0.90f, 1.0f } },
        { {  0.000f,  0.580f, 0.0f }, { 1.0f, 0.15f, 0.15f, 1.0f } },
    };
    const UINT vertexBufferSize = sizeof(vertices);

    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = vertexBufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    StageThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&stage.VertexBuffer)), "Default vertex buffer creation failed.");

    ComPtr<ID3D12Resource> uploadBuffer;
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)), "Upload vertex buffer creation failed.");

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(uploadBuffer->Map(0, &readRange, &mappedData), "Upload buffer Map failed.");
    std::memcpy(mappedData, vertices, vertexBufferSize);
    uploadBuffer->Unmap(0, nullptr);

    ComPtr<ID3D12CommandQueue> uploadQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    StageThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&uploadQueue)), "Stage upload queue creation failed.");

    ComPtr<ID3D12CommandAllocator> allocator;
    StageThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "Stage upload allocator creation failed.");

    ComPtr<ID3D12GraphicsCommandList> commandList;
    StageThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Stage upload command list creation failed.");
    commandList->CopyBufferRegion(stage.VertexBuffer.Get(), 0, uploadBuffer.Get(), 0, vertexBufferSize);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = stage.VertexBuffer.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
    StageThrowIfFailed(commandList->Close(), "Stage upload command list close failed.");

    ID3D12CommandList* commandLists[] = { commandList.Get() };
    uploadQueue->ExecuteCommandLists(1, commandLists);

    ComPtr<ID3D12Fence> fence;
    StageThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Stage upload fence creation failed.");
    HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    StageThrowIfFailed(fenceEvent ? S_OK : E_FAIL, "Stage upload fence event creation failed.");
    WaitForStageQueue(uploadQueue.Get(), fence.Get(), fenceEvent, 1);
    CloseHandle(fenceEvent);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.StrideInBytes = sizeof(Vertex);
    stage.VertexBufferView.SizeInBytes = vertexBufferSize;
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.10f;
    stage.ClearColor[1] = 0.08f;
    stage.ClearColor[2] = 0.24f;
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
    context.CommandList->DrawInstanced(18, 1, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
