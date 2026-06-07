#pragma once

// Learning goal: Render depth from a light view and sample it to shade shadows.
// Implementation status: Implemented.
// Implementation: Two-pass shadow mapping.
//   Pass 1 (shadow pass): render scene depth into a 512x512 D32_FLOAT texture from the light's POV.
//   Pass 2 (scene pass):  render the scene from the camera, comparing fragment depth in light-space
//                         against the shadow map to determine lit vs shadowed regions.
// Scene: a large floor quad + 3 small caster quads that orbit the Y axis.

#include <wrl/client.h>

#include <array>
#include <cmath>
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

static constexpr uint32_t kShadowMapSize = 512;
static constexpr int      kObjectCount   = 4;   // 1 floor + 3 casters

// ------------------------------------------------------------------
// Per-object geometry: a flat quad with a world transform baked in.
// We pass vertices in world space and let the shaders apply view/proj.
// ------------------------------------------------------------------
struct ShadowVertex
{
    float x, y, z;        // world-space position
    float r, g, b;        // base color (used in scene pass PS)
};

// Per-object description used to rebuild geometry each frame.
struct ObjectDesc
{
    // Center world position
    float cx, cy, cz;
    // Half-extents for a flat (XZ-plane) quad
    float hw, hd;
    // Color
    float r, g, b;
};

struct LearningStageState
{
    // Shadow pass resources
    ComPtr<ID3D12RootSignature> ShadowRootSignature;
    ComPtr<ID3D12PipelineState> ShadowPipelineState;
    ComPtr<ID3D12DescriptorHeap> DsvHeap;
    ComPtr<ID3D12Resource> ShadowMap;

    // Scene pass resources
    ComPtr<ID3D12RootSignature> SceneRootSignature;
    ComPtr<ID3D12PipelineState> ScenePipelineState;
    ComPtr<ID3D12DescriptorHeap> SrvHeap;

    // Shared geometry (world-space vertices, rebuilt each frame into upload buffers)
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> IndexBuffer;
    ShadowVertex* MappedVerts = nullptr;

    D3D12_VERTEX_BUFFER_VIEW VBV = {};
    D3D12_INDEX_BUFFER_VIEW  IBV = {};

    D3D12_VIEWPORT SceneViewport = {};
    D3D12_RECT     SceneScissor  = {};
    D3D12_VIEWPORT ShadowViewport = {};
    D3D12_RECT     ShadowScissor  = {};

    float ClearColor[4] = { 0.08f, 0.09f, 0.10f, 1.0f };
    float TimeSeconds = 0.0f;

    std::array<ObjectDesc, kObjectCount> Objects = {};
};

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------
inline void StageThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr))
        throw std::runtime_error(message);
}

inline ComPtr<ID3DBlob> CompileStageShader(const wchar_t* path, const char* entry, const char* target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> code, errors;
    const HRESULT hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, flags, 0, &code, &errors);
    if (FAILED(hr))
        throw std::runtime_error(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Shader compile failed.");
    return code;
}

inline ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, const void* data, uint64_t bytes)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> buf;
    StageThrowIfFailed(
        device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buf)),
        "CreateUploadBuffer failed.");
    if (data)
    {
        void* mapped = nullptr;
        D3D12_RANGE range = {};
        buf->Map(0, &range, &mapped);
        std::memcpy(mapped, data, static_cast<size_t>(bytes));
        buf->Unmap(0, nullptr);
    }
    return buf;
}

inline D3D12_RESOURCE_BARRIER MakeTransitionBarrier(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER b = {};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource   = resource;
    b.Transition.StateBefore = before;
    b.Transition.StateAfter  = after;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return b;
}

// ------------------------------------------------------------------
// Simple math helpers
// ------------------------------------------------------------------

// Column-major 4x4 multiply: C = A * B (row-major storage, row_major pragma in shader).
// We store matrices as float[4][4] where [row][col].
inline void Mat4Mul(float out[4][4], const float a[4][4], const float b[4][4])
{
    float tmp[4][4] = {};
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            for (int k = 0; k < 4; ++k)
                tmp[r][c] += a[r][k] * b[k][c];
    std::memcpy(out, tmp, sizeof(tmp));
}

// Build an orthographic projection matrix (right-handed, NDC z in [0,1]).
inline void BuildOrtho(float out[4][4], float l, float r, float b, float t, float zn, float zf)
{
    std::memset(out, 0, sizeof(float) * 16);
    out[0][0] = 2.0f / (r - l);
    out[1][1] = 2.0f / (t - b);
    out[2][2] = 1.0f / (zf - zn);
    out[3][0] = -(r + l) / (r - l);
    out[3][1] = -(t + b) / (t - b);
    out[3][2] = -zn / (zf - zn);
    out[3][3] = 1.0f;
}

// Build a look-at view matrix (right-handed).
inline void BuildLookAt(float out[4][4], float ex, float ey, float ez,
                        float tx, float ty, float tz,
                        float ux, float uy, float uz)
{
    // Forward
    float fx = tx - ex, fy = ty - ey, fz = tz - ez;
    const float flen = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= flen; fy /= flen; fz /= flen;
    // Right = Forward x Up
    float rx = fy*uz - fz*uy;
    float ry = fz*ux - fx*uz;
    float rz = fx*uy - fy*ux;
    const float rlen = sqrtf(rx*rx + ry*ry + rz*rz);
    rx /= rlen; ry /= rlen; rz /= rlen;
    // Up = Right x Forward
    const float upx = ry*fz - rz*fy;
    const float upy = rz*fx - rx*fz;
    const float upz = rx*fy - ry*fx;

    std::memset(out, 0, sizeof(float) * 16);
    out[0][0] = rx;  out[0][1] = upx; out[0][2] = fx;  out[0][3] = 0.0f;
    out[1][0] = ry;  out[1][1] = upy; out[1][2] = fy;  out[1][3] = 0.0f;
    out[2][0] = rz;  out[2][1] = upz; out[2][2] = fz;  out[2][3] = 0.0f;
    out[3][0] = -(rx*ex + ry*ey + rz*ez);
    out[3][1] = -(upx*ex + upy*ey + upz*ez);
    out[3][2] = -(fx*ex + fy*ey + fz*ez);
    out[3][3] = 1.0f;
}

// Build a perspective projection matrix (right-handed, z in [0,1]).
inline void BuildPerspective(float out[4][4], float fovYRad, float aspect, float zn, float zf)
{
    const float h = 1.0f / tanf(fovYRad * 0.5f);
    const float w = h / aspect;
    std::memset(out, 0, sizeof(float) * 16);
    out[0][0] = w;
    out[1][1] = h;
    out[2][2] = zf / (zf - zn);
    out[2][3] = 1.0f;
    out[3][2] = -zn * zf / (zf - zn);
}

// ------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------
inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // ---- Shadow map depth texture ----
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        StageThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&stage.DsvHeap)), "DSV heap failed.");

        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rd.Width  = kShadowMapSize;
        rd.Height = kShadowMapSize;
        rd.DepthOrArraySize = 1;
        rd.MipLevels = 1;
        rd.Format = DXGI_FORMAT_R32_TYPELESS;
        rd.SampleDesc.Count = 1;
        rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        D3D12_CLEAR_VALUE cv = {};
        cv.Format = DXGI_FORMAT_D32_FLOAT;
        cv.DepthStencil.Depth = 1.0f;
        StageThrowIfFailed(
            device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &cv, IID_PPV_ARGS(&stage.ShadowMap)),
            "Shadow map creation failed.");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        device->CreateDepthStencilView(stage.ShadowMap.Get(), &dsvDesc, stage.DsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // ---- SRV heap (shadow map as shader resource) ----
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 2;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        StageThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&stage.SrvHeap)), "SRV heap failed.");

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(stage.ShadowMap.Get(), &srvDesc, stage.SrvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // ---- Shadow pass root signature: 16 root constants (LightViewProj 4x4) at b0 ----
    {
        D3D12_ROOT_PARAMETER shadowParam = {};
        shadowParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        shadowParam.Constants.Num32BitValues = 16;
        shadowParam.Constants.ShaderRegister = 0;
        shadowParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 1;
        rsDesc.pParameters = &shadowParam;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (shadow) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.ShadowRootSignature)),
            "CreateRootSignature (shadow) failed.");
    }

    // ---- Scene pass root signature ----
    // param 0: 32 root constants (ViewProj 4x4 + LightViewProj 4x4) at b0, VS-visible
    // param 1: descriptor table (SRV t0) for the shadow map, PS-visible
    // static sampler: comparison sampler at s0
    {
        D3D12_DESCRIPTOR_RANGE srvRange = {};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER sceneParams[2] = {};
        sceneParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        sceneParams[0].Constants.Num32BitValues = 32;
        sceneParams[0].Constants.ShaderRegister = 0;
        sceneParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        sceneParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        sceneParams[1].DescriptorTable.NumDescriptorRanges = 1;
        sceneParams[1].DescriptorTable.pDescriptorRanges = &srvRange;
        sceneParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // Comparison sampler for PCF shadow test
        D3D12_STATIC_SAMPLER_DESC cmpSampler = {};
        cmpSampler.Filter         = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        cmpSampler.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        cmpSampler.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        cmpSampler.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        cmpSampler.BorderColor    = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        cmpSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        cmpSampler.ShaderRegister = 0;
        cmpSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 2;
        rsDesc.pParameters = sceneParams;
        rsDesc.NumStaticSamplers = 1;
        rsDesc.pStaticSamplers = &cmpSampler;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (scene) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.SceneRootSignature)),
            "CreateRootSignature (scene) failed.");
    }

    // ---- Compile shaders ----
    ComPtr<ID3DBlob> shadowVS = CompileStageShader(L"shaders\\Shadow Mapping.hlsl", "ShadowVS", "vs_5_0");
    ComPtr<ID3DBlob> sceneVS  = CompileStageShader(L"shaders\\Shadow Mapping.hlsl", "SceneVS",  "vs_5_0");
    ComPtr<ID3DBlob> scenePS  = CompileStageShader(L"shaders\\Shadow Mapping.hlsl", "ScenePS",  "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ---- Shadow pass PSO: depth-only, no color output ----
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = stage.ShadowRootSignature.Get();
        pso.VS = { shadowVS->GetBufferPointer(), shadowVS->GetBufferSize() };
        // No PS, no RTVFormats
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.RasterizerState.DepthBias = 1000;
        pso.RasterizerState.DepthBiasClamp = 0.0f;
        pso.RasterizerState.SlopeScaledDepthBias = 1.5f;
        pso.DepthStencilState.DepthEnable = TRUE;
        pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        pso.InputLayout = { elems, _countof(elems) };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 0;
        pso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        pso.SampleDesc.Count = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.ShadowPipelineState)), "Shadow PSO failed.");
    }

    // ---- Scene pass PSO ----
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = stage.SceneRootSignature.Get();
        pso.VS = { sceneVS->GetBufferPointer(), sceneVS->GetBufferSize() };
        pso.PS = { scenePS->GetBufferPointer(), scenePS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout = { elems, _countof(elems) };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.ScenePipelineState)), "Scene PSO failed.");
    }

    // ---- Geometry: 4 objects x 4 verts = 16 verts, 4 objects x 6 indices = 24 indices ----
    // Vertex buffer is persistently mapped; rebuilt each frame.
    static constexpr int kTotalVerts   = kObjectCount * 4;
    static constexpr int kTotalIndices = kObjectCount * 6;

    {
        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        rd.Width = sizeof(ShadowVertex) * kTotalVerts;
        rd.Height = 1; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
        rd.SampleDesc.Count = 1;
        rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        StageThrowIfFailed(
            device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stage.VertexBuffer)),
            "Vertex buffer failed.");
        D3D12_RANGE readRange = {};
        stage.VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedVerts));
    }
    stage.VBV = { stage.VertexBuffer->GetGPUVirtualAddress(), sizeof(ShadowVertex) * kTotalVerts, sizeof(ShadowVertex) };

    // Index buffer is static: each quad uses the same 6-index pattern.
    {
        uint16_t idxs[kTotalIndices];
        for (int i = 0; i < kObjectCount; ++i)
        {
            const uint16_t base = static_cast<uint16_t>(i * 4);
            idxs[i * 6 + 0] = base + 0;
            idxs[i * 6 + 1] = base + 1;
            idxs[i * 6 + 2] = base + 2;
            idxs[i * 6 + 3] = base + 0;
            idxs[i * 6 + 4] = base + 2;
            idxs[i * 6 + 5] = base + 3;
        }
        stage.IndexBuffer = CreateUploadBuffer(device, idxs, sizeof(idxs));
        stage.IBV = { stage.IndexBuffer->GetGPUVirtualAddress(), sizeof(idxs), DXGI_FORMAT_R16_UINT };
    }

    // ---- Viewports ----
    stage.SceneViewport  = { 0.0f, 0.0f, static_cast<float>(context.Width),  static_cast<float>(context.Height),  0.0f, 1.0f };
    stage.SceneScissor   = { 0, 0, static_cast<LONG>(context.Width),  static_cast<LONG>(context.Height) };
    stage.ShadowViewport = { 0.0f, 0.0f, static_cast<float>(kShadowMapSize), static_cast<float>(kShadowMapSize), 0.0f, 1.0f };
    stage.ShadowScissor  = { 0, 0, static_cast<LONG>(kShadowMapSize), static_cast<LONG>(kShadowMapSize) };
}

// ------------------------------------------------------------------
// Build a flat XZ quad (y = cy) centred at (cx, cy, cz) with half-extents hw (X) and hd (Z).
// ------------------------------------------------------------------
inline void WriteQuadVerts(ShadowVertex* dst, const ObjectDesc& obj)
{
    const float cx = obj.cx, cy = obj.cy, cz = obj.cz;
    const float hw = obj.hw, hd = obj.hd;
    dst[0] = { cx - hw, cy, cz - hd, obj.r, obj.g, obj.b };
    dst[1] = { cx + hw, cy, cz - hd, obj.r, obj.g, obj.b };
    dst[2] = { cx + hw, cy, cz + hd, obj.r, obj.g, obj.b };
    dst[3] = { cx - hw, cy, cz + hd, obj.r, obj.g, obj.b };
}

// ------------------------------------------------------------------
// Update
// ------------------------------------------------------------------
inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    const float t = stage.TimeSeconds;

    // Object 0: large floor quad (static)
    stage.Objects[0] = { 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.40f, 0.38f, 0.36f };

    // Objects 1-3: small caster quads orbiting the Y axis at different heights/radii/speeds
    const float r1 = 0.6f, h1 = 0.35f;
    stage.Objects[1] = { cosf(t * 0.7f) * r1, h1, sinf(t * 0.7f) * r1, 0.18f, 0.18f, 0.90f, 0.30f, 0.20f };

    const float r2 = 0.4f, h2 = 0.65f;
    stage.Objects[2] = { cosf(t * 1.1f + 2.0f) * r2, h2, sinf(t * 1.1f + 2.0f) * r2, 0.14f, 0.14f, 0.25f, 0.65f, 0.90f };

    const float r3 = 0.75f, h3 = 0.20f;
    stage.Objects[3] = { cosf(t * 0.5f + 4.2f) * r3, h3, sinf(t * 0.5f + 4.2f) * r3, 0.16f, 0.16f, 0.85f, 0.80f, 0.20f };

    // Rebuild geometry in the persistently mapped vertex buffer.
    for (int i = 0; i < kObjectCount; ++i)
        WriteQuadVerts(stage.MappedVerts + i * 4, stage.Objects[i]);
}

// ------------------------------------------------------------------
// Render
// ------------------------------------------------------------------
inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    const float t = stage.TimeSeconds;
    ID3D12GraphicsCommandList* cmd = context.CommandList;

    // ---- Build matrices ----
    // Light: directional from above-right, orthographic projection spanning the scene.
    const float lightEye[3] = { 1.5f, 3.0f, 1.0f };
    float lightView[4][4], lightProj[4][4], lightVP[4][4];
    BuildLookAt(lightView, lightEye[0], lightEye[1], lightEye[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    BuildOrtho(lightProj, -2.5f, 2.5f, -2.5f, 2.5f, 0.1f, 10.0f);
    Mat4Mul(lightVP, lightView, lightProj);

    // Camera: perspective, orbiting slightly.
    float camView[4][4], camProj[4][4], camVP[4][4];
    const float camX = sinf(t * 0.2f) * 0.5f;
    BuildLookAt(camView, camX + 0.0f, 2.5f, -3.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    const float aspect = (context.Width > 0 && context.Height > 0)
        ? static_cast<float>(context.Width) / static_cast<float>(context.Height) : 1.0f;
    BuildPerspective(camProj, 3.14159f / 4.0f, aspect, 0.1f, 20.0f);
    Mat4Mul(camVP, camView, camProj);

    // Combined scene constants: camVP (16 floats) + lightVP (16 floats) = 32 floats
    float sceneConstants[32];
    std::memcpy(sceneConstants,      camVP,   sizeof(camVP));
    std::memcpy(sceneConstants + 16, lightVP, sizeof(lightVP));

    // ---- Pass 1: Shadow pass ----
    // Shadow map is already in DEPTH_WRITE state (initial state + end-of-frame transition).
    cmd->RSSetViewports(1, &stage.ShadowViewport);
    cmd->RSSetScissorRects(1, &stage.ShadowScissor);

    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = stage.DsvHeap->GetCPUDescriptorHandleForHeapStart();
    cmd->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    cmd->OMSetRenderTargets(0, nullptr, FALSE, &dsv);

    cmd->SetGraphicsRootSignature(stage.ShadowRootSignature.Get());
    cmd->SetGraphicsRoot32BitConstants(0, 16, lightVP, 0);
    cmd->SetPipelineState(stage.ShadowPipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VBV);
    cmd->IASetIndexBuffer(&stage.IBV);
    cmd->DrawIndexedInstanced(kObjectCount * 6, 1, 0, 0, 0);

    // Transition shadow map: DEPTH_WRITE -> PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_BARRIER toSrv = MakeTransitionBarrier(
        stage.ShadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd->ResourceBarrier(1, &toSrv);

    // ---- Pass 2: Scene pass ----
    cmd->RSSetViewports(1, &stage.SceneViewport);
    cmd->RSSetScissorRects(1, &stage.SceneScissor);

    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);

    ID3D12DescriptorHeap* heaps[] = { stage.SrvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);
    cmd->SetGraphicsRootSignature(stage.SceneRootSignature.Get());
    cmd->SetGraphicsRoot32BitConstants(0, 32, sceneConstants, 0);
    cmd->SetGraphicsRootDescriptorTable(1, stage.SrvHeap->GetGPUDescriptorHandleForHeapStart());
    cmd->SetPipelineState(stage.ScenePipelineState.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.VBV);
    cmd->IASetIndexBuffer(&stage.IBV);
    cmd->DrawIndexedInstanced(kObjectCount * 6, 1, 0, 0, 0);

    // Transition shadow map back: PIXEL_SHADER_RESOURCE -> DEPTH_WRITE
    D3D12_RESOURCE_BARRIER toDepth = MakeTransitionBarrier(
        stage.ShadowMap.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    cmd->ResourceBarrier(1, &toDepth);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.MappedVerts)
    {
        stage.VertexBuffer->Unmap(0, nullptr);
        stage.MappedVerts = nullptr;
    }
    stage.IndexBuffer.Reset();
    stage.VertexBuffer.Reset();
    stage.SrvHeap.Reset();
    stage.ScenePipelineState.Reset();
    stage.SceneRootSignature.Reset();
    stage.DsvHeap.Reset();
    stage.ShadowMap.Reset();
    stage.ShadowPipelineState.Reset();
    stage.ShadowRootSignature.Reset();
}
