#pragma once

// Learning goal: Practice explicit resource state transitions for render targets, copy destinations, and shader resources.
// Implementation status: Implemented.
//
// Two barriers fire every frame inside ApplyStageSpecificRender:
//   Barrier 1  RENDER_TARGET -> PIXEL_SHADER_RESOURCE  (makes the offscreen write visible to the screen-pass sampler)
//   Barrier 2  PIXEL_SHADER_RESOURCE -> RENDER_TARGET  (resets state so the next frame can write into the texture again)
//
// Compare with 07 (Texture Upload): that barrier is a one-time init (COPY_DEST -> PIXEL_SHADER_RESOURCE).
// This sample shows the per-frame runtime cycle required when the same resource alternates between RTV and SRV roles.

#include <wrl/client.h>

#include <cstdint>
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

struct LearningStageState
{
    ComPtr<ID3D12RootSignature> OffscreenRootSignature;
    ComPtr<ID3D12RootSignature> ScreenRootSignature;
    ComPtr<ID3D12PipelineState> OffscreenPipelineState;
    ComPtr<ID3D12PipelineState> ScreenPipelineState;
    ComPtr<ID3D12DescriptorHeap> OffscreenRtvHeap;
    ComPtr<ID3D12DescriptorHeap> SrvHeap;
    ComPtr<ID3D12Resource> OffscreenTexture;

    D3D12_CPU_DESCRIPTOR_HANDLE OffscreenRtv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE OffscreenSrv = {};
    D3D12_VIEWPORT Viewport = {};
    D3D12_RECT Scissor = {};
    float BackBufferClearColor[4] = { 0.04f, 0.04f, 0.06f, 1.0f };
    float TimeSeconds = 0.0f;
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

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;
    stage.Width = context.Width;
    stage.Height = context.Height;

    // Offscreen root signature: 3 root constants (time, width, height) for the procedural pattern PS.
    D3D12_ROOT_PARAMETER offscreenParam = {};
    offscreenParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    offscreenParam.Constants.ShaderRegister = 0;
    offscreenParam.Constants.RegisterSpace = 0;
    offscreenParam.Constants.Num32BitValues = 3;
    offscreenParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC offscreenRootDesc = {};
    offscreenRootDesc.NumParameters = 1;
    offscreenRootDesc.pParameters = &offscreenParam;

    ComPtr<ID3DBlob> sig;
    ComPtr<ID3DBlob> sigErrors;
    HRESULT hr = D3D12SerializeRootSignature(&offscreenRootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErrors);
    if (FAILED(hr))
    {
        throw std::runtime_error(sigErrors ? static_cast<const char*>(sigErrors->GetBufferPointer()) : "SerializeRootSignature (offscreen) failed.");
    }
    StageThrowIfFailed(device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.OffscreenRootSignature)), "CreateRootSignature (offscreen) failed.");

    // Screen root signature: SRV descriptor table + static linear sampler.
    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER screenParam = {};
    screenParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    screenParam.DescriptorTable.NumDescriptorRanges = 1;
    screenParam.DescriptorTable.pDescriptorRanges = &srvRange;
    screenParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ShaderRegister = 0;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC screenRootDesc = {};
    screenRootDesc.NumParameters = 1;
    screenRootDesc.pParameters = &screenParam;
    screenRootDesc.NumStaticSamplers = 1;
    screenRootDesc.pStaticSamplers = &sampler;

    sig.Reset();
    sigErrors.Reset();
    hr = D3D12SerializeRootSignature(&screenRootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErrors);
    if (FAILED(hr))
    {
        throw std::runtime_error(sigErrors ? static_cast<const char*>(sigErrors->GetBufferPointer()) : "SerializeRootSignature (screen) failed.");
    }
    StageThrowIfFailed(device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.ScreenRootSignature)), "CreateRootSignature (screen) failed.");

    ComPtr<ID3DBlob> offscreenVs = CompileStageShader(L"shaders\\Resource Barriers.hlsl", "OffscreenVS", "vs_5_0");
    ComPtr<ID3DBlob> offscreenPs = CompileStageShader(L"shaders\\Resource Barriers.hlsl", "OffscreenPS", "ps_5_0");
    ComPtr<ID3DBlob> screenVs    = CompileStageShader(L"shaders\\Resource Barriers.hlsl", "ScreenVS",    "vs_5_0");
    ComPtr<ID3DBlob> screenPs    = CompileStageShader(L"shaders\\Resource Barriers.hlsl", "ScreenPS",    "ps_5_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    psoDesc.pRootSignature = stage.OffscreenRootSignature.Get();
    psoDesc.VS = { offscreenVs->GetBufferPointer(), offscreenVs->GetBufferSize() };
    psoDesc.PS = { offscreenPs->GetBufferPointer(), offscreenPs->GetBufferSize() };
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.OffscreenPipelineState)), "CreateGraphicsPipelineState (offscreen) failed.");

    psoDesc.pRootSignature = stage.ScreenRootSignature.Get();
    psoDesc.VS = { screenVs->GetBufferPointer(), screenVs->GetBufferSize() };
    psoDesc.PS = { screenPs->GetBufferPointer(), screenPs->GetBufferSize() };
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.ScreenPipelineState)), "CreateGraphicsPipelineState (screen) failed.");

    // Offscreen texture with ALLOW_RENDER_TARGET so it can serve as both RTV and SRV.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    StageThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&stage.OffscreenRtvHeap)), "CreateDescriptorHeap (RTV) failed.");
    stage.OffscreenRtv = stage.OffscreenRtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&stage.SrvHeap)), "CreateDescriptorHeap (SRV) failed.");
    stage.OffscreenSrv = stage.SrvHeap->GetGPUDescriptorHandleForHeapStart();

    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = context.Width;
    textureDesc.Height = context.Height;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // Initial state is RENDER_TARGET: the offscreen pass runs first each frame.
    StageThrowIfFailed(
        device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            nullptr,
            IID_PPV_ARGS(&stage.OffscreenTexture)),
        "CreateCommittedResource (offscreen texture) failed.");

    device->CreateRenderTargetView(stage.OffscreenTexture.Get(), nullptr, stage.OffscreenRtv);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(stage.OffscreenTexture.Get(), &srvDesc, stage.SrvHeap->GetCPUDescriptorHandleForHeapStart());

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* commandList = context.CommandList;

    // ---- Offscreen pass ----
    // Texture state entering this block: RENDER_TARGET (initial, or restored at end of previous frame).
    commandList->OMSetRenderTargets(1, &stage.OffscreenRtv, FALSE, nullptr);
    const float offscreenClear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(stage.OffscreenRtv, offscreenClear, 0, nullptr);
    commandList->SetGraphicsRootSignature(stage.OffscreenRootSignature.Get());
    commandList->SetPipelineState(stage.OffscreenPipelineState.Get());
    struct OffscreenConstants { float Time; float Width; float Height; } offConst =
        { stage.TimeSeconds, static_cast<float>(stage.Width), static_cast<float>(stage.Height) };
    commandList->SetGraphicsRoot32BitConstants(0, 3, &offConst, 0);
    commandList->RSSetViewports(1, &stage.Viewport);
    commandList->RSSetScissorRects(1, &stage.Scissor);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    // Barrier 1: RENDER_TARGET -> PIXEL_SHADER_RESOURCE
    // The GPU must finish writing before the screen pass can sample the texture.
    D3D12_RESOURCE_BARRIER toSrv = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &toSrv);

    // ---- Screen pass ----
    ID3D12DescriptorHeap* heaps[] = { stage.SrvHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);
    commandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    commandList->ClearRenderTargetView(context.RenderTargetView, stage.BackBufferClearColor, 0, nullptr);
    commandList->SetGraphicsRootSignature(stage.ScreenRootSignature.Get());
    commandList->SetPipelineState(stage.ScreenPipelineState.Get());
    commandList->SetGraphicsRootDescriptorTable(0, stage.OffscreenSrv);
    commandList->DrawInstanced(6, 1, 0, 0);

    // Barrier 2: PIXEL_SHADER_RESOURCE -> RENDER_TARGET
    // Reset to RENDER_TARGET so the next frame's offscreen pass can write into the texture again.
    D3D12_RESOURCE_BARRIER toRt = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &toRt);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.OffscreenTexture.Reset();
    stage.SrvHeap.Reset();
    stage.OffscreenRtvHeap.Reset();
    stage.ScreenPipelineState.Reset();
    stage.OffscreenPipelineState.Reset();
    stage.ScreenRootSignature.Reset();
    stage.OffscreenRootSignature.Reset();
}
