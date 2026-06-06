#pragma once

// Learning goal: Separate CPU-visible upload resources from GPU-local default resources and copy between them.
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
    float ClearColor[4] = { 0.11f, 0.13f, 0.07f, 1.0f };
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12Resource> VertexUploadBuffer;
    ComPtr<ID3D12Resource> IndexUploadBuffer;
    ComPtr<ID3D12Resource> VertexDefaultBuffer;
    ComPtr<ID3D12Resource> IndexDefaultBuffer;
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
        return L"13. Upload And Default Heaps\\shaders\\Upload And Default Heaps.hlsl";
    }
    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"13. Upload And Default Heaps\\shaders\\Upload And Default Heaps.hlsl" : path.substr(0, slash + 1) + L"shaders\\Upload And Default Heaps.hlsl";
}

inline D3D12_RESOURCE_DESC BufferDesc(UINT64 size)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return desc;
}

inline void FillUploadBuffer(ID3D12Resource* resource, const void* data, UINT64 size)
{
    void* mapped = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(resource->Map(0, &readRange, &mapped), "Upload buffer Map failed.");
    std::memcpy(mapped, data, static_cast<size_t>(size));
    resource->Unmap(0, nullptr);
}

inline void WaitForStageQueue(ID3D12CommandQueue* queue, ID3D12Fence* fence, HANDLE fenceEvent, uint64_t fenceValue)
{
    StageThrowIfFailed(queue->Signal(fence, fenceValue), "Stage queue Signal failed.");
    if (fence->GetCompletedValue() < fenceValue)
    {
        StageThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent), "Stage fence wait failed.");
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

    const Vertex vertices[] = {
        { { -0.56f, 0.44f, 0.0f }, { 0.95f, 0.85f, 0.20f, 1.0f } },
        { { 0.56f, 0.44f, 0.0f }, { 0.30f, 0.95f, 0.40f, 1.0f } },
        { { 0.56f, -0.44f, 0.0f }, { 0.35f, 0.55f, 1.0f, 1.0f } },
        { { -0.56f, -0.44f, 0.0f }, { 0.95f, 0.30f, 0.25f, 1.0f } },
    };
    const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
    const UINT64 vertexBufferSize = sizeof(vertices);
    const UINT64 indexBufferSize = sizeof(indices);

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC vertexDesc = BufferDesc(vertexBufferSize);
    D3D12_RESOURCE_DESC indexDesc = BufferDesc(indexBufferSize);
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.VertexUploadBuffer)), "Vertex upload buffer creation failed.");
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.IndexUploadBuffer)), "Index upload buffer creation failed.");
    FillUploadBuffer(stage.VertexUploadBuffer.Get(), vertices, vertexBufferSize);
    FillUploadBuffer(stage.IndexUploadBuffer.Get(), indices, indexBufferSize);

    StageThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&stage.VertexDefaultBuffer)), "Vertex default buffer creation failed.");
    StageThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&stage.IndexDefaultBuffer)), "Index default buffer creation failed.");

    ComPtr<ID3D12CommandQueue> queue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    StageThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)), "Copy queue creation failed.");
    ComPtr<ID3D12CommandAllocator> allocator;
    StageThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "Copy allocator creation failed.");
    ComPtr<ID3D12GraphicsCommandList> list;
    StageThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list)), "Copy command list creation failed.");

    list->CopyResource(stage.VertexDefaultBuffer.Get(), stage.VertexUploadBuffer.Get());
    list->CopyResource(stage.IndexDefaultBuffer.Get(), stage.IndexUploadBuffer.Get());
    D3D12_RESOURCE_BARRIER barriers[2] = {};
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource = stage.VertexDefaultBuffer.Get();
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource = stage.IndexDefaultBuffer.Get();
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    list->ResourceBarrier(2, barriers);
    StageThrowIfFailed(list->Close(), "Copy command list close failed.");
    ID3D12CommandList* lists[] = { list.Get() };
    queue->ExecuteCommandLists(1, lists);
    ComPtr<ID3D12Fence> fence;
    StageThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Copy fence creation failed.");
    HANDLE eventHandle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    StageThrowIfFailed(eventHandle ? S_OK : E_FAIL, "Copy event creation failed.");
    WaitForStageQueue(queue.Get(), fence.Get(), eventHandle, 1);
    CloseHandle(eventHandle);

    stage.VertexBufferView = { stage.VertexDefaultBuffer->GetGPUVirtualAddress(), sizeof(vertices), sizeof(Vertex) };
    stage.IndexBufferView = { stage.IndexDefaultBuffer->GetGPUVirtualAddress(), sizeof(indices), DXGI_FORMAT_R16_UINT };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.11f;
    stage.ClearColor[1] = 0.13f;
    stage.ClearColor[2] = 0.07f;
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
    context.CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.IndexDefaultBuffer.Reset();
    stage.VertexDefaultBuffer.Reset();
    stage.IndexUploadBuffer.Reset();
    stage.VertexUploadBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
