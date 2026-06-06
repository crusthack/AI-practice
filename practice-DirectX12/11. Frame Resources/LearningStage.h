#pragma once

// Learning goal: Use per-frame command allocators, constant buffers, and fence values for multiple frames in flight.
// Implementation status: Implemented.

#include <windows.h>
#include <wrl/client.h>

#include <cstdint>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>

#include <d3d12.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

constexpr uint32_t StageFrameCount = 2;

struct Vertex
{
    float Position[3];
    float Color[4];
};

struct FrameConstants
{
    float Angle;
    float Scale;
    float Offset[2];
    float Tint[4];
};

struct FrameResource
{
    ComPtr<ID3D12Resource> ConstantBuffer;
    FrameConstants* MappedConstants = nullptr;
    uint64_t UpdateSerial = 0;
};

struct LearningStageState
{
    float ClearColor[4] = { 0.10f, 0.10f, 0.12f, 1.0f };
    double TimeSeconds = 0.0;
    uint64_t FrameSerial = 0;
    FrameResource Frames[StageFrameCount];
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    ComPtr<ID3D12DescriptorHeap> CbvHeap;
    ComPtr<ID3D12Resource> VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    uint32_t CbvDescriptorSize = 0;
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t FrameIndex = 0;
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
        return L"11. Frame Resources\\shaders\\Frame Resources.hlsl";
    }
    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"11. Frame Resources\\shaders\\Frame Resources.hlsl" : path.substr(0, slash + 1) + L"shaders\\Frame Resources.hlsl";
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

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = StageFrameCount;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&stage.CbvHeap)), "CBV heap creation failed.");
    stage.CbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC cbDesc = {};
    cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cbDesc.Width = Align256(sizeof(FrameConstants));
    cbDesc.Height = 1;
    cbDesc.DepthOrArraySize = 1;
    cbDesc.MipLevels = 1;
    cbDesc.SampleDesc.Count = 1;
    cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    for (uint32_t frameIndex = 0; frameIndex < StageFrameCount; ++frameIndex)
    {
        StageThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.Frames[frameIndex].ConstantBuffer)), "Per-frame constant buffer creation failed.");
        D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = stage.CbvHeap->GetCPUDescriptorHandleForHeapStart();
        cbvHandle.ptr += static_cast<SIZE_T>(frameIndex) * stage.CbvDescriptorSize;
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = stage.Frames[frameIndex].ConstantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = Align256(sizeof(FrameConstants));
        device->CreateConstantBufferView(&cbvDesc, cbvHandle);
        D3D12_RANGE readRange = { 0, 0 };
        StageThrowIfFailed(stage.Frames[frameIndex].ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&stage.Frames[frameIndex].MappedConstants)), "Per-frame constant buffer Map failed.");
    }

    // 18-vertex hexagonal color wheel: 6 fan triangles sharing a white center.
    static const float kOuter[6][2] = {
        {  0.000f,  0.580f }, {  0.502f,  0.290f },
        {  0.502f, -0.290f }, {  0.000f, -0.580f },
        { -0.502f, -0.290f }, { -0.502f,  0.290f },
    };
    static const float kColors[6][4] = {
        { 1.0f, 0.15f, 0.15f, 1.0f }, { 1.0f, 0.85f, 0.15f, 1.0f },
        { 0.15f, 0.90f, 0.15f, 1.0f }, { 0.15f, 0.90f, 0.90f, 1.0f },
        { 0.30f, 0.35f, 1.00f, 1.0f }, { 0.90f, 0.20f, 0.90f, 1.0f },
    };
    Vertex vertices[18] = {};
    for (int seg = 0; seg < 6; ++seg)
    {
        const int next = (seg + 1) % 6;
        vertices[seg * 3 + 0] = { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        vertices[seg * 3 + 1] = { { kOuter[seg][0],  kOuter[seg][1],  0.0f }, { kColors[seg][0],  kColors[seg][1],  kColors[seg][2],  1.0f } };
        vertices[seg * 3 + 2] = { { kOuter[next][0], kOuter[next][1], 0.0f }, { kColors[next][0], kColors[next][1], kColors[next][2], 1.0f } };
    }
    CreateUploadBuffer(device, vertices, sizeof(vertices), &stage.VertexBuffer);
    stage.VertexBufferView = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(vertices), sizeof(Vertex) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = timeSeconds;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    FrameResource& frame = stage.Frames[context.FrameIndex % StageFrameCount];
    const float t = static_cast<float>(stage.TimeSeconds);
    FrameConstants constants = {};
    constants.Angle    = t * 1.8f;
    constants.Scale    = 1.0f;
    constants.Offset[0] = std::sinf(t * 0.80f) * 0.22f;
    constants.Offset[1] = std::sinf(t * 1.60f + 0.5f) * 0.12f;
    constants.Tint[0] = constants.Tint[1] = constants.Tint[2] = constants.Tint[3] = 1.0f;
    std::memcpy(frame.MappedConstants, &constants, sizeof(constants));
    frame.UpdateSerial = ++stage.FrameSerial;

    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    context.CommandList->SetPipelineState(stage.PipelineState.Get());
    ID3D12DescriptorHeap* heaps[] = { stage.CbvHeap.Get() };
    context.CommandList->SetDescriptorHeaps(1, heaps);
    D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle = stage.CbvHeap->GetGPUDescriptorHandleForHeapStart();
    cbvHandle.ptr += static_cast<UINT64>(context.FrameIndex % StageFrameCount) * stage.CbvDescriptorSize;
    context.CommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
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
    for (FrameResource& frame : stage.Frames)
    {
        if (frame.ConstantBuffer)
        {
            frame.ConstantBuffer->Unmap(0, nullptr);
        }
        frame.ConstantBuffer.Reset();
        frame.MappedConstants = nullptr;
    }
    stage.VertexBuffer.Reset();
    stage.CbvHeap.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
