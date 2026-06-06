#pragma once

// Learning goal: Dispatch a compute shader and synchronize its output for rendering or readback.
// Implementation status: Implemented.

#include <wrl/client.h>

#include <cstdint>
#include <stdexcept>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

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

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> ComputeRootSignature;
    ComPtr<ID3D12RootSignature> GraphicsRootSignature;
    ComPtr<ID3D12PipelineState> ComputePipelineState;
    ComPtr<ID3D12PipelineState> GraphicsPipelineState;
    ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
    ComPtr<ID3D12Resource> ComputeTexture;

    D3D12_GPU_DESCRIPTOR_HANDLE UavHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE SrvHandle = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT Scissor = {};
    float TimeSeconds = 0.0f;
    float ClearColor[4] = { 0.02f, 0.03f, 0.05f, 1.0f };
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

inline ComPtr<ID3D12RootSignature> CreateRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& desc)
{
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> errors;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors),
        errors ? static_cast<const char*>(errors->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");

    ComPtr<ID3D12RootSignature> rootSignature;
    StageThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "CreateRootSignature failed.");
    return rootSignature;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = 2;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&stage.DescriptorHeap)), "CreateDescriptorHeap failed.");

    const uint32_t descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE uavCpu = stage.DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = uavCpu;
    srvCpu.ptr += descriptorSize;

    stage.UavHandle = stage.DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    stage.SrvHandle = stage.UavHandle;
    stage.SrvHandle.ptr += descriptorSize;

    D3D12_HEAP_PROPERTIES textureHeap = {};
    textureHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = context.Width;
    textureDesc.Height = context.Height;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    StageThrowIfFailed(
        device->CreateCommittedResource(
            &textureHeap,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&stage.ComputeTexture)),
        "CreateCommittedResource compute texture failed.");

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = textureDesc.Format;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(stage.ComputeTexture.Get(), nullptr, &uavDesc, uavCpu);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(stage.ComputeTexture.Get(), &srvDesc, srvCpu);

    D3D12_DESCRIPTOR_RANGE uavRange = {};
    uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uavRange.NumDescriptors = 1;
    uavRange.BaseShaderRegister = 0;
    uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER computeParams[2] = {};
    computeParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    computeParams[0].DescriptorTable.NumDescriptorRanges = 1;
    computeParams[0].DescriptorTable.pDescriptorRanges = &uavRange;
    computeParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    computeParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    computeParams[1].Constants.Num32BitValues = 4;
    computeParams[1].Constants.ShaderRegister = 0;
    computeParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC computeRootDesc = {};
    computeRootDesc.NumParameters = 2;
    computeRootDesc.pParameters = computeParams;
    stage.ComputeRootSignature = CreateRootSignature(device, computeRootDesc);

    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER graphicsParam = {};
    graphicsParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    graphicsParam.DescriptorTable.NumDescriptorRanges = 1;
    graphicsParam.DescriptorTable.pDescriptorRanges = &srvRange;
    graphicsParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ShaderRegister = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC graphicsRootDesc = {};
    graphicsRootDesc.NumParameters = 1;
    graphicsRootDesc.pParameters = &graphicsParam;
    graphicsRootDesc.NumStaticSamplers = 1;
    graphicsRootDesc.pStaticSamplers = &sampler;
    graphicsRootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    stage.GraphicsRootSignature = CreateRootSignature(device, graphicsRootDesc);

    ComPtr<ID3DBlob> computeShader = CompileStageShader(L"shaders\\Compute Shader.hlsl", "ComputeMain", "cs_5_0");
    ComPtr<ID3DBlob> vertexShader = CompileStageShader(L"shaders\\Compute Shader.hlsl", "ScreenVS", "vs_5_0");
    ComPtr<ID3DBlob> pixelShader = CompileStageShader(L"shaders\\Compute Shader.hlsl", "ScreenPS", "ps_5_0");

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = stage.ComputeRootSignature.Get();
    computePsoDesc.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };
    StageThrowIfFailed(device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&stage.ComputePipelineState)), "CreateComputePipelineState failed.");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPsoDesc = {};
    graphicsPsoDesc.pRootSignature = stage.GraphicsRootSignature.Get();
    graphicsPsoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    graphicsPsoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    graphicsPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    graphicsPsoDesc.SampleMask = UINT_MAX;
    graphicsPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    graphicsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    graphicsPsoDesc.RasterizerState.DepthClipEnable = TRUE;
    graphicsPsoDesc.DepthStencilState.DepthEnable = FALSE;
    graphicsPsoDesc.DepthStencilState.StencilEnable = FALSE;
    graphicsPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPsoDesc.NumRenderTargets = 1;
    graphicsPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    graphicsPsoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&graphicsPsoDesc, IID_PPV_ARGS(&stage.GraphicsPipelineState)), "CreateGraphicsPipelineState failed.");

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* commandList = context.CommandList;
    ID3D12DescriptorHeap* descriptorHeaps[] = { stage.DescriptorHeap.Get() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    commandList->SetComputeRootSignature(stage.ComputeRootSignature.Get());
    commandList->SetPipelineState(stage.ComputePipelineState.Get());
    commandList->SetComputeRootDescriptorTable(0, stage.UavHandle);

    struct ComputeConstants
    {
        float Time;
        float Width;
        float Height;
        float Padding;
    } constants = { stage.TimeSeconds, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f };
    commandList->SetComputeRoot32BitConstants(1, 4, &constants, 0);

    const uint32_t dispatchX = (context.Width + 15u) / 16u;
    const uint32_t dispatchY = (context.Height + 15u) / 16u;
    commandList->Dispatch(dispatchX, dispatchY, 1);

    D3D12_RESOURCE_BARRIER toSrv = MakeTransitionBarrier(
        stage.ComputeTexture.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &toSrv);

    commandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    commandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    commandList->RSSetViewports(1, &stage.Viewport);
    commandList->RSSetScissorRects(1, &stage.Scissor);
    commandList->SetGraphicsRootSignature(stage.GraphicsRootSignature.Get());
    commandList->SetGraphicsRootDescriptorTable(0, stage.SrvHandle);
    commandList->SetPipelineState(stage.GraphicsPipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(6, 1, 0, 0);

    D3D12_RESOURCE_BARRIER toUav = MakeTransitionBarrier(
        stage.ComputeTexture.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &toUav);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.ComputeTexture.Reset();
    stage.DescriptorHeap.Reset();
    stage.GraphicsPipelineState.Reset();
    stage.ComputePipelineState.Reset();
    stage.GraphicsRootSignature.Reset();
    stage.ComputeRootSignature.Reset();
}
