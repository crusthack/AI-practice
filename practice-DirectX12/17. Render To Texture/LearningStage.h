#pragma once

// Learning goal: Render into an offscreen texture and then sample it while presenting to the swap chain.
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
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> OffscreenPipelineState;
    ComPtr<ID3D12PipelineState> ScreenPipelineState;
    ComPtr<ID3D12DescriptorHeap> OffscreenRtvHeap;
    ComPtr<ID3D12DescriptorHeap> SrvHeap;
    ComPtr<ID3D12Resource> OffscreenTexture;

    D3D12_CPU_DESCRIPTOR_HANDLE OffscreenRtv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE OffscreenSrv = {};
    D3D12_VIEWPORT OffscreenViewport = {};
    D3D12_RECT OffscreenScissor = {};
    float BackBufferClearColor[4] = { 0.03f, 0.04f, 0.06f, 1.0f };
    float OffscreenClearColor[4] = { 0.02f, 0.13f, 0.13f, 1.0f };
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

    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.DescriptorTable.NumDescriptorRanges = 1;
    rootParameter.DescriptorTable.pDescriptorRanges = &srvRange;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &sampler;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> signatureErrors;
    StageThrowIfFailed(
        D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &signatureErrors),
        signatureErrors ? static_cast<const char*>(signatureErrors->GetBufferPointer()) : "D3D12SerializeRootSignature failed.");
    StageThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&stage.RootSignature)), "CreateRootSignature failed.");

    D3D12_DESCRIPTOR_HEAP_DESC offscreenRtvHeapDesc = {};
    offscreenRtvHeapDesc.NumDescriptors = 1;
    offscreenRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    StageThrowIfFailed(device->CreateDescriptorHeap(&offscreenRtvHeapDesc, IID_PPV_ARGS(&stage.OffscreenRtvHeap)), "CreateDescriptorHeap RTV failed.");
    stage.OffscreenRtv = stage.OffscreenRtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    StageThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&stage.SrvHeap)), "CreateDescriptorHeap SRV failed.");
    stage.OffscreenSrv = stage.SrvHeap->GetGPUDescriptorHandleForHeapStart();

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
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearValue.Color[0] = stage.OffscreenClearColor[0];
    clearValue.Color[1] = stage.OffscreenClearColor[1];
    clearValue.Color[2] = stage.OffscreenClearColor[2];
    clearValue.Color[3] = stage.OffscreenClearColor[3];

    StageThrowIfFailed(
        device->CreateCommittedResource(
            &textureHeap,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&stage.OffscreenTexture)),
        "CreateCommittedResource offscreen texture failed.");

    device->CreateRenderTargetView(stage.OffscreenTexture.Get(), nullptr, stage.OffscreenRtv);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(stage.OffscreenTexture.Get(), &srvDesc, stage.SrvHeap->GetCPUDescriptorHandleForHeapStart());

    ComPtr<ID3DBlob> offscreenVs = CompileStageShader(L"shaders\\Render To Texture.hlsl", "OffscreenVS", "vs_5_0");
    ComPtr<ID3DBlob> offscreenPs = CompileStageShader(L"shaders\\Render To Texture.hlsl", "OffscreenPS", "ps_5_0");
    ComPtr<ID3DBlob> screenVs = CompileStageShader(L"shaders\\Render To Texture.hlsl", "ScreenVS", "vs_5_0");
    ComPtr<ID3DBlob> screenPs = CompileStageShader(L"shaders\\Render To Texture.hlsl", "ScreenPS", "ps_5_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = stage.RootSignature.Get();
    psoDesc.VS = { offscreenVs->GetBufferPointer(), offscreenVs->GetBufferSize() };
    psoDesc.PS = { offscreenPs->GetBufferPointer(), offscreenPs->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.InputLayout = { nullptr, 0 };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.OffscreenPipelineState)), "CreateGraphicsPipelineState offscreen failed.");

    psoDesc.VS = { screenVs->GetBufferPointer(), screenVs->GetBufferSize() };
    psoDesc.PS = { screenPs->GetBufferPointer(), screenPs->GetBufferSize() };
    StageThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&stage.ScreenPipelineState)), "CreateGraphicsPipelineState screen failed.");

    stage.OffscreenViewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.OffscreenScissor = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)stage;
    (void)timeSeconds;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* commandList = context.CommandList;

    ID3D12DescriptorHeap* descriptorHeaps[] = { stage.SrvHeap.Get() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);
    commandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    commandList->SetGraphicsRootDescriptorTable(0, stage.OffscreenSrv);

    commandList->RSSetViewports(1, &stage.OffscreenViewport);
    commandList->RSSetScissorRects(1, &stage.OffscreenScissor);

    // First pass: draw into a texture-owned RTV instead of the swap-chain back buffer.
    commandList->OMSetRenderTargets(1, &stage.OffscreenRtv, FALSE, nullptr);
    commandList->ClearRenderTargetView(stage.OffscreenRtv, stage.OffscreenClearColor, 0, nullptr);
    commandList->SetPipelineState(stage.OffscreenPipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    D3D12_RESOURCE_BARRIER toSrv = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &toSrv);

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    D3D12_RECT scissor = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    // Second pass: sample the offscreen texture through an SRV and present it on the back buffer.
    commandList->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    commandList->ClearRenderTargetView(context.RenderTargetView, stage.BackBufferClearColor, 0, nullptr);
    commandList->SetPipelineState(stage.ScreenPipelineState.Get());
    commandList->DrawInstanced(6, 1, 0, 0);

    D3D12_RESOURCE_BARRIER toRenderTarget = MakeTransitionBarrier(
        stage.OffscreenTexture.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &toRenderTarget);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.OffscreenTexture.Reset();
    stage.SrvHeap.Reset();
    stage.OffscreenRtvHeap.Reset();
    stage.ScreenPipelineState.Reset();
    stage.OffscreenPipelineState.Reset();
    stage.RootSignature.Reset();
}
