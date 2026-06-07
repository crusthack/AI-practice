#pragma once

// Learning goal: Write GPU data through a UAV and read selected results back to the CPU.
// Implementation status: Implemented.

#include <wrl/client.h>

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

struct ReadbackColor
{
    float R;
    float G;
    float B;
    float A;
};

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> ComputeRootSignature;
    ComPtr<ID3D12PipelineState> ComputePipelineState;
    ComPtr<ID3D12DescriptorHeap> UavHeap;
    ComPtr<ID3D12Resource> GpuResultBuffer;
    ComPtr<ID3D12Resource> ReadbackBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE UavHandle = {};
    ReadbackColor CpuColor = { 0.07f, 0.12f, 0.06f, 1.0f };
    float TimeSeconds = 0.0f;
    bool HasSubmittedCopy = false;
};

inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
    {
        throw std::runtime_error(message);
    }
}

inline ComPtr<ID3DBlob> CompileStageShader(const wchar_t* path, const char* entryPoint, const char* target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> bytecode;
    ComPtr<ID3DBlob> errors;
    const HRESULT hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, flags, 0, &bytecode, &errors);
    if (FAILED(hr))
    {
        throw std::runtime_error(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Shader compile failed.");
    }

    return bytecode;
}

inline D3D12_RESOURCE_BARRIER MakeTransitionBarrier(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return barrier;
}

inline D3D12_RESOURCE_DESC BufferDesc(uint64_t byteSize, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = byteSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = flags;
    return desc;
}

inline void ReadPreviousGpuResult(LearningStageState& stage)
{
    if (!stage.HasSubmittedCopy)
    {
        return;
    }

    void* mapped = nullptr;
    D3D12_RANGE readRange = { 0, sizeof(ReadbackColor) };
    if (SUCCEEDED(stage.ReadbackBuffer->Map(0, &readRange, &mapped)))
    {
        std::memcpy(&stage.CpuColor, mapped, sizeof(ReadbackColor));
        const D3D12_RANGE writeRange = { 0, 0 };
        stage.ReadbackBuffer->Unmap(0, &writeRange);
    }
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    (void)context;
    ID3D12Device* device = context.Device;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&stage.UavHeap)), "CreateDescriptorHeap failed.");
    stage.UavHandle = stage.UavHeap->GetGPUDescriptorHandleForHeapStart();

    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC gpuBufferDesc = BufferDesc(sizeof(ReadbackColor), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    StageThrowIfFailed(
        device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &gpuBufferDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&stage.GpuResultBuffer)),
        "CreateCommittedResource GPU UAV buffer failed.");

    D3D12_HEAP_PROPERTIES readbackHeap = {};
    readbackHeap.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC readbackDesc = BufferDesc(sizeof(ReadbackColor));
    StageThrowIfFailed(
        device->CreateCommittedResource(
            &readbackHeap,
            D3D12_HEAP_FLAG_NONE,
            &readbackDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&stage.ReadbackBuffer)),
        "CreateCommittedResource readback buffer failed.");

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = 1;
    uavDesc.Buffer.StructureByteStride = sizeof(ReadbackColor);
    device->CreateUnorderedAccessView(stage.GpuResultBuffer.Get(), nullptr, &uavDesc, stage.UavHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_DESCRIPTOR_RANGE uavRange = {};
    uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uavRange.NumDescriptors = 1;
    uavRange.BaseShaderRegister = 0;
    uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER params[2] = {};
    params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[0].DescriptorTable.NumDescriptorRanges = 1;
    params[0].DescriptorTable.pDescriptorRanges = &uavRange;
    params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    params[1].Constants.Num32BitValues = 1;
    params[1].Constants.ShaderRegister = 0;
    params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 2;
    rootDesc.pParameters = params;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> errors;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors),
        errors ? static_cast<const char*>(errors->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");
    StageThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&stage.ComputeRootSignature)), "CreateRootSignature failed.");

    ComPtr<ID3DBlob> computeShader = CompileStageShader(L"shaders\\UAV And Readback.hlsl", "ComputeMain", "cs_5_0");
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = stage.ComputeRootSignature.Get();
    psoDesc.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };
    StageThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&stage.ComputePipelineState)), "CreateComputePipelineState failed.");
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ReadPreviousGpuResult(stage);

    ID3D12GraphicsCommandList* commandList = context.CommandList;
    ID3D12DescriptorHeap* heaps[] = { stage.UavHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);
    commandList->SetComputeRootSignature(stage.ComputeRootSignature.Get());
    commandList->SetPipelineState(stage.ComputePipelineState.Get());
    commandList->SetComputeRootDescriptorTable(0, stage.UavHandle);
    commandList->SetComputeRoot32BitConstants(1, 1, &stage.TimeSeconds, 0);
    commandList->Dispatch(1, 1, 1);

    // UAV barrier orders the compute write before the copy into the CPU-visible readback heap.
    D3D12_RESOURCE_BARRIER uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = stage.GpuResultBuffer.Get();
    commandList->ResourceBarrier(1, &uavBarrier);

    D3D12_RESOURCE_BARRIER toCopySource = MakeTransitionBarrier(
        stage.GpuResultBuffer.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(1, &toCopySource);
    commandList->CopyResource(stage.ReadbackBuffer.Get(), stage.GpuResultBuffer.Get());

    D3D12_RESOURCE_BARRIER toUav = MakeTransitionBarrier(
        stage.GpuResultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &toUav);

    float clearColor[4] = { stage.CpuColor.R, stage.CpuColor.G, stage.CpuColor.B, 1.0f };
    commandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    commandList->ClearRenderTargetView(context.RenderTargetView, clearColor, 0, nullptr);
    stage.HasSubmittedCopy = true;
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.ReadbackBuffer.Reset();
    stage.GpuResultBuffer.Reset();
    stage.UavHeap.Reset();
    stage.ComputePipelineState.Reset();
    stage.ComputeRootSignature.Reset();
}
