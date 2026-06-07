#pragma once

// Learning goal: Move a view camera with keyboard and mouse input.
// Implementation status: Implemented.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wrl/client.h>

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <d3d12.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

struct Vertex
{
    float Position[3];
    float Color[4];
};

struct CameraConstants
{
    DirectX::XMFLOAT4X4 WorldViewProjection;
};

struct LearningStageState
{
    float ClearColor[4] = { 0.04f, 0.07f, 0.16f, 1.0f };
    DirectX::XMFLOAT3 CameraPosition = { 0.0f, 1.0f, -6.0f };
    float Yaw = 0.0f;
    float Pitch = -0.10f;
    POINT LastMousePosition = {};
    bool HasMousePosition = false;
    double LastTimeSeconds = 0.0;
    CameraConstants Constants = {};
    CameraConstants* MappedConstants = nullptr;
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12DescriptorHeap> CbvHeap;
    ComPtr<ID3D12DescriptorHeap> DsvHeap;
    ComPtr<ID3D12Resource> ConstantBuffer;
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    ComPtr<ID3D12Resource> DepthBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
    uint32_t IndexCount = 0;
    uint32_t ViewWidth = 1280;
    uint32_t ViewHeight = 720;
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
        return L"10. Camera\\shaders\\Camera.hlsl";
    }
    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"10. Camera\\shaders\\Camera.hlsl" : path.substr(0, slash + 1) + L"shaders\\Camera.hlsl";
}

inline UINT Align256(UINT value)
{
    return (value + 255u) & ~255u;
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

inline void AddCube(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, DirectX::XMFLOAT3 center, float size, DirectX::XMFLOAT4 color)
{
    const float h = size * 0.5f;
    const uint16_t base = static_cast<uint16_t>(vertices.size());
    const DirectX::XMFLOAT3 p[] = {
        { center.x - h, center.y - h, center.z - h },
        { center.x - h, center.y + h, center.z - h },
        { center.x + h, center.y + h, center.z - h },
        { center.x + h, center.y - h, center.z - h },
        { center.x - h, center.y - h, center.z + h },
        { center.x - h, center.y + h, center.z + h },
        { center.x + h, center.y + h, center.z + h },
        { center.x + h, center.y - h, center.z + h },
    };
    for (const DirectX::XMFLOAT3& point : p)
    {
        vertices.push_back({ { point.x, point.y, point.z }, { color.x, color.y, color.z, color.w } });
    }
    const uint16_t cubeIndices[] = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7,
    };
    for (uint16_t index : cubeIndices)
    {
        indices.push_back(static_cast<uint16_t>(base + index));
    }
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device, uint32_t width, uint32_t height)
{
    stage.ViewWidth  = width;
    stage.ViewHeight = height;

    D3D12_DESCRIPTOR_RANGE cbvRange = {};
    cbvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvRange.NumDescriptors = 1;
    cbvRange.BaseShaderRegister = 0;
    cbvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.DescriptorTable.NumDescriptorRanges = 1;
    rootParameter.DescriptorTable.pDescriptorRanges = &cbvRange;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

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
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.PipelineState)), "CreateGraphicsPipelineState failed.");

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&stage.CbvHeap)), "CBV heap creation failed.");

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    StageThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&stage.DsvHeap)), "DSV heap creation failed.");

    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = static_cast<UINT64>(width);
    depthDesc.Height = static_cast<UINT>(height);
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE depthClear = {};
    depthClear.Format = DXGI_FORMAT_D32_FLOAT;
    depthClear.DepthStencil.Depth = 1.0f;
    StageThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClear, IID_PPV_ARGS(&stage.DepthBuffer)), "Depth buffer creation failed.");
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    device->CreateDepthStencilView(stage.DepthBuffer.Get(), &dsvDesc, stage.DsvHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC cbDesc = {};
    cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cbDesc.Width = Align256(sizeof(CameraConstants));
    cbDesc.Height = 1;
    cbDesc.DepthOrArraySize = 1;
    cbDesc.MipLevels = 1;
    cbDesc.SampleDesc.Count = 1;
    cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.ConstantBuffer)), "Constant buffer creation failed.");
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = stage.ConstantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = Align256(sizeof(CameraConstants));
    device->CreateConstantBufferView(&cbvDesc, stage.CbvHeap->GetCPUDescriptorHandleForHeapStart());
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(stage.ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedConstants)), "Constant buffer Map failed.");

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    AddCube(vertices, indices, {  0.0f,  0.0f,  0.0f }, 1.0f, { 0.95f, 0.25f, 0.20f, 1.0f });
    AddCube(vertices, indices, { -2.5f,  0.0f,  1.0f }, 0.8f, { 0.25f, 0.85f, 0.35f, 1.0f });
    AddCube(vertices, indices, {  2.5f,  0.0f,  1.0f }, 0.8f, { 0.25f, 0.45f, 1.0f, 1.0f });
    AddCube(vertices, indices, {  0.0f,  0.0f,  4.0f }, 1.2f, { 0.90f, 0.75f, 0.20f, 1.0f });
    AddCube(vertices, indices, { -2.0f,  0.0f,  5.5f }, 0.7f, { 0.80f, 0.30f, 0.90f, 1.0f });
    AddCube(vertices, indices, {  2.0f,  0.0f,  5.5f }, 0.7f, { 0.20f, 0.80f, 0.80f, 1.0f });
    AddCube(vertices, indices, {  0.0f, -1.0f,  2.5f }, 0.5f, { 0.90f, 0.60f, 0.30f, 1.0f });
    CreateUploadBuffer(device, vertices.data(), static_cast<UINT64>(vertices.size() * sizeof(Vertex)), &stage.VertexBuffer);
    CreateUploadBuffer(device, indices.data(), static_cast<UINT64>(indices.size() * sizeof(uint16_t)), &stage.IndexBuffer);
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), static_cast<UINT>(vertices.size() * sizeof(Vertex)), sizeof(Vertex) };
    stage.IndexBufferView = { stage.IndexBuffer->GetGPUVirtualAddress(), static_cast<UINT>(indices.size() * sizeof(uint16_t)), DXGI_FORMAT_R16_UINT };
    stage.IndexCount = static_cast<uint32_t>(indices.size());
}

inline DirectX::XMVECTOR CameraForward(const LearningStageState& stage)
{
    return DirectX::XMVector3Normalize(DirectX::XMVectorSet(
        std::cosf(stage.Pitch) * std::sinf(stage.Yaw),
        std::sinf(stage.Pitch),
        std::cosf(stage.Pitch) * std::cosf(stage.Yaw),
        0.0f));
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.ClearColor[0] = 0.04f;
    stage.ClearColor[1] = 0.07f;
    stage.ClearColor[2] = 0.16f;
    stage.ClearColor[3] = 1.0f;

    const double rawDelta = timeSeconds - stage.LastTimeSeconds;
    const float deltaSeconds = static_cast<float>((std::min)(0.05, (std::max)(0.0, rawDelta)));
    stage.LastTimeSeconds = timeSeconds;

    using namespace DirectX;
    XMVECTOR position = XMLoadFloat3(&stage.CameraPosition);
    const XMVECTOR forward = CameraForward(stage);
    const XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), forward));
    const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 6.0f : 2.6f;
    const float step = speed * deltaSeconds;

    if (GetAsyncKeyState('W') & 0x8000) position = XMVectorAdd(position, XMVectorScale(forward, step));
    if (GetAsyncKeyState('S') & 0x8000) position = XMVectorSubtract(position, XMVectorScale(forward, step));
    if (GetAsyncKeyState('D') & 0x8000) position = XMVectorAdd(position, XMVectorScale(right, step));
    if (GetAsyncKeyState('A') & 0x8000) position = XMVectorSubtract(position, XMVectorScale(right, step));
    if (GetAsyncKeyState('E') & 0x8000) position = XMVectorAdd(position, XMVectorScale(up, step));
    if (GetAsyncKeyState('Q') & 0x8000) position = XMVectorSubtract(position, XMVectorScale(up, step));
    XMStoreFloat3(&stage.CameraPosition, position);

    // Hold the right mouse button and move the mouse to adjust yaw and pitch.
    POINT mouse = {};
    if (GetCursorPos(&mouse) && (GetAsyncKeyState(VK_RBUTTON) & 0x8000))
    {
        if (stage.HasMousePosition)
        {
            const float sensitivity = 0.004f;
            stage.Yaw += static_cast<float>(mouse.x - stage.LastMousePosition.x) * sensitivity;
            stage.Pitch -= static_cast<float>(mouse.y - stage.LastMousePosition.y) * sensitivity;
            stage.Pitch = (std::max)(-1.45f, (std::min)(1.45f, stage.Pitch));
        }
        stage.LastMousePosition = mouse;
        stage.HasMousePosition = true;
    }
    else
    {
        stage.HasMousePosition = false;
    }

    const XMMATRIX world = XMMatrixIdentity();
    const XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&stage.CameraPosition), CameraForward(stage), up);
    const float aspect = static_cast<float>(stage.ViewWidth) / static_cast<float>(stage.ViewHeight);
    const XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);
    XMStoreFloat4x4(&stage.Constants.WorldViewProjection, XMMatrixTranspose(world * view * projection));
    std::memcpy(stage.MappedConstants, &stage.Constants, sizeof(CameraConstants));
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = stage.DsvHeap->GetCPUDescriptorHandleForHeapStart();
    context.CommandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, &dsvHandle);
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
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
    context.CommandList->IASetIndexBuffer(&stage.IndexBufferView);
    context.CommandList->DrawIndexedInstanced(stage.IndexCount, 1, 0, 0, 0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.ConstantBuffer)
    {
        stage.ConstantBuffer->Unmap(0, nullptr);
    }
    stage.DepthBuffer.Reset();
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.ConstantBuffer.Reset();
    stage.DsvHeap.Reset();
    stage.CbvHeap.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
