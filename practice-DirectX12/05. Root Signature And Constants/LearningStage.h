#pragma once

// Learning goal: Bind small per-draw values through root constants and compare them with constant-buffer binding.
// Implementation status: Implemented.
//
// Upgrade: seven independent triangles drawn from one shared vertex buffer.
// A single SetGraphicsRoot32BitConstants call before each DrawInstanced supplies a distinct
// offset, scale, rotation angle, and colour tint — no descriptor heap access required.

#include <windows.h>
#include <wrl/client.h>

#include <climits>
#include <cmath>
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

// 8 DWORDs (32 bytes) — fits entirely within the 64-DWORD root-constant budget.
struct RootConstants
{
    float Offset[2]; // NDC-space translation applied after rotation and scale
    float Scale;     // uniform scale in local space
    float Angle;     // rotation angle in radians (CCW)
    float Tint[4];   // per-object RGBA multiplier
};

constexpr int kObjectCount = 7; // 1 central + 6 orbiting

struct LearningStageState
{
    float ClearColor[4] = { 0.04f, 0.06f, 0.10f, 1.0f };
    RootConstants Objects[kObjectCount] = {};
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
        return L"05. Root Signature And Constants\\shaders\\Root Signature And Constants.hlsl";
    }

    std::wstring path(exePath, length);
    const size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L"05. Root Signature And Constants\\shaders\\Root Signature And Constants.hlsl";
    }

    return path.substr(0, slash + 1) + L"shaders\\Root Signature And Constants.hlsl";
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameters[0].Constants.ShaderRegister = 0;
    rootParameters[0].Constants.RegisterSpace = 0;
    rootParameters[0].Constants.Num32BitValues = sizeof(RootConstants) / sizeof(uint32_t);
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
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
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

    // Unit equilateral triangle. All vertices are white so colour comes entirely from the Tint root constant.
    const Vertex vertices[] = {
        { {  0.000f,  1.000f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.866f, -0.500f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.866f, -0.500f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
    };

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(vertices);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    StageThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.VertexBuffer)), "CreateCommittedResource for vertex buffer failed.");

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    StageThrowIfFailed(stage.VertexBuffer->Map(0, &readRange, &mappedData), "Vertex buffer Map failed.");
    std::memcpy(mappedData, vertices, sizeof(vertices));
    stage.VertexBuffer->Unmap(0, nullptr);

    stage.VertexBufferView.BufferLocation = stage.VertexBuffer->GetGPUVirtualAddress();
    stage.VertexBufferView.StrideInBytes = sizeof(Vertex);
    stage.VertexBufferView.SizeInBytes = sizeof(vertices);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.ClearColor[0] = 0.04f;
    stage.ClearColor[1] = 0.06f;
    stage.ClearColor[2] = 0.10f;
    stage.ClearColor[3] = 1.0f;

    const float t = static_cast<float>(timeSeconds);
    constexpr float kTau = 6.28318f;

    // Six distinct hues for the satellite triangles.
    static const float kSatColor[6][3] = {
        { 1.0f, 0.20f, 0.20f }, // red
        { 1.0f, 0.85f, 0.20f }, // yellow
        { 0.25f, 0.90f, 0.25f }, // green
        { 0.20f, 0.85f, 0.95f }, // cyan
        { 0.35f, 0.40f, 1.00f }, // blue
        { 0.90f, 0.25f, 0.90f }, // magenta
    };

    // Central triangle: slow counter-rotation, neutral tint.
    RootConstants& center = stage.Objects[0];
    center.Offset[0] = 0.0f;
    center.Offset[1] = 0.0f;
    center.Scale = 0.32f;
    center.Angle = -t * 0.30f;
    center.Tint[0] = 0.88f;
    center.Tint[1] = 0.88f;
    center.Tint[2] = 0.88f;
    center.Tint[3] = 1.0f;

    // Six satellites orbiting at equal angular spacing.
    for (int i = 0; i < 6; ++i)
    {
        const float baseAngle  = i * (kTau / 6.0f);
        const float orbitAngle = baseAngle + t * 0.65f;
        RootConstants& rc = stage.Objects[i + 1];
        rc.Offset[0] = std::cosf(orbitAngle) * 0.52f;
        rc.Offset[1] = std::sinf(orbitAngle) * 0.52f;
        rc.Scale = 0.17f;
        rc.Angle = -orbitAngle + t * 2.0f;
        rc.Tint[0] = kSatColor[i][0];
        rc.Tint[1] = kSatColor[i][1];
        rc.Tint[2] = kSatColor[i][2];
        rc.Tint[3] = 1.0f;
    }
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
    context.CommandList->SetGraphicsRootSignature(stage.RootSignature.Get());
    context.CommandList->SetPipelineState(stage.PipelineState.Get());

    const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    const D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
    context.CommandList->RSSetViewports(1, &viewport);
    context.CommandList->RSSetScissorRects(1, &scissorRect);
    context.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.CommandList->IASetVertexBuffers(0, 1, &stage.VertexBufferView);

    // Each draw call binds a fresh set of root constants — no descriptor heap, no buffer rebind.
    for (int i = 0; i < kObjectCount; ++i)
    {
        context.CommandList->SetGraphicsRoot32BitConstants(
            0,
            sizeof(RootConstants) / sizeof(uint32_t),
            &stage.Objects[i],
            0);
        context.CommandList->DrawInstanced(3, 1, 0, 0);
    }
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    stage.VertexBuffer.Reset();
    stage.PipelineState.Reset();
    stage.RootSignature.Reset();
}
