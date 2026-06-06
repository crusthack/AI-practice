#pragma once

// Learning goal: Capstone combining multi-object scene graph, per-pixel Phong lighting,
// animated orbiting camera, and a post-process color grading pass (RTT + fullscreen effect).
// Implementation status: Implemented.
//
// Frame structure:
//   Pass 1 — Scene pass: lit objects rendered into an offscreen texture (RTT)
//   Pass 2 — Post pass:  fullscreen quad samples the RTT, applies tone-map + vignette

#include <wrl/client.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

using Microsoft::WRL::ComPtr;

// ---------------------------------------------------------------------------
// Context structs
// ---------------------------------------------------------------------------

struct LearningStageSetupContext
{
    ID3D12Device* Device = nullptr;
    uint32_t Width  = 0;
    uint32_t Height = 0;
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
    uint32_t Width  = 0;
    uint32_t Height = 0;
};

// ---------------------------------------------------------------------------
// Per-frame constant buffer (GPU-side).  Must be 256-byte aligned.
// ---------------------------------------------------------------------------
struct alignas(256) SceneFrameConstants
{
    float ViewProj[4][4];
    float LightDir[3]; float _p0;
    float CameraPos[3]; float _p1;
};

// ---------------------------------------------------------------------------
// Stage state
// ---------------------------------------------------------------------------

struct LearningStageState
{
    // Scene pass resources
    ComPtr<ID3D12RootSignature> SceneRootSignature;
    ComPtr<ID3D12PipelineState> ScenePipelineState;
    ComPtr<ID3D12Resource>      QuadVertexBuffer;
    ComPtr<ID3D12Resource>      QuadIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW    QuadVertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW     QuadIndexBufferView  = {};

    // Per-frame CBV (persistently mapped upload buffer)
    ComPtr<ID3D12Resource>      SceneFrameCbv;
    SceneFrameConstants*        MappedFrame = nullptr;

    // RTT resources
    ComPtr<ID3D12Resource>      OffscreenTexture;
    ComPtr<ID3D12DescriptorHeap> OffscreenRtvHeap;
    ComPtr<ID3D12DescriptorHeap> SrvHeap;       // SHADER_VISIBLE; slot 0 = CBV, slot 1 = scene SRV
    D3D12_CPU_DESCRIPTOR_HANDLE OffscreenRtv    = {};
    D3D12_GPU_DESCRIPTOR_HANDLE OffscreenSrv    = {};

    // Post-process resources
    ComPtr<ID3D12RootSignature> PostRootSignature;
    ComPtr<ID3D12PipelineState> PostPipelineState;

    // Common
    D3D12_VIEWPORT Viewport            = {};
    D3D12_RECT     Scissor             = {};
    float          ClearColor[4]       = { 0.06f, 0.08f, 0.11f, 1.0f };
    float          OffscreenClearColor[4] = { 0.03f, 0.04f, 0.06f, 1.0f };
    float          TimeSeconds         = 0.0f;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

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
    desc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width            = bytes;
    desc.Height           = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels        = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
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
    b.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource   = resource;
    b.Transition.StateBefore = before;
    b.Transition.StateAfter  = after;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return b;
}

// ---------------------------------------------------------------------------
// Matrix math helpers (hand-coded, no external library)
// ---------------------------------------------------------------------------

// Multiply two 4x4 row-major matrices: out = a * b
inline void MatMul4x4(float out[4][4], const float a[4][4], const float b[4][4])
{
    float tmp[4][4] = {};
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            for (int k = 0; k < 4; ++k)
                tmp[r][c] += a[r][k] * b[k][c];
    std::memcpy(out, tmp, sizeof(tmp));
}

// Identity
inline void MatIdentity(float m[4][4])
{
    std::memset(m, 0, 16 * sizeof(float));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

// Scale
inline void MatScale(float m[4][4], float sx, float sy, float sz)
{
    MatIdentity(m);
    m[0][0] = sx; m[1][1] = sy; m[2][2] = sz;
}

// Translation
inline void MatTranslate(float m[4][4], float tx, float ty, float tz)
{
    MatIdentity(m);
    m[3][0] = tx; m[3][1] = ty; m[3][2] = tz;
}

// Rotation around Y axis
inline void MatRotateY(float m[4][4], float angle)
{
    MatIdentity(m);
    const float c = cosf(angle);
    const float s = sinf(angle);
    m[0][0] =  c; m[0][2] = s;
    m[2][0] = -s; m[2][2] = c;
}

// Perspective projection (row-major, left-handed, depth [0,1])
// fovY in radians, aspect = width/height
inline void MatPerspective(float m[4][4], float fovY, float aspect, float zNear, float zFar)
{
    std::memset(m, 0, 16 * sizeof(float));
    const float f = 1.0f / tanf(fovY * 0.5f);
    m[0][0] = f / aspect;
    m[1][1] = f;
    m[2][2] = zFar / (zFar - zNear);
    m[2][3] = 1.0f;
    m[3][2] = -(zNear * zFar) / (zFar - zNear);
}

// Look-at view matrix (row-major, left-handed)
inline void MatLookAt(float m[4][4], float ex, float ey, float ez,
                                     float tx, float ty, float tz)
{
    // Forward = normalize(target - eye)
    float fx = tx - ex, fy = ty - ey, fz = tz - ez;
    float fl = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;

    // Right = normalize(Forward x Up)  (Up = (0,1,0))
    float rx = fz, ry = 0.0f, rz = -fx;  // cross(F, (0,1,0))
    float rl = sqrtf(rx*rx + ry*ry + rz*rz);
    rx /= rl; ry /= rl; rz /= rl;

    // True up = Right x Forward
    float ux = ry*fz - rz*fy;
    float uy = rz*fx - rx*fz;
    float uz = rx*fy - ry*fx;

    std::memset(m, 0, 16 * sizeof(float));
    // Row-major view matrix: basis vectors in rows
    m[0][0] = rx; m[0][1] = ux; m[0][2] = fx;
    m[1][0] = ry; m[1][1] = uy; m[1][2] = fy;
    m[2][0] = rz; m[2][1] = uz; m[2][2] = fz;
    m[3][0] = -(rx*ex + ry*ey + rz*ez);
    m[3][1] = -(ux*ex + uy*ey + uz*ez);
    m[3][2] = -(fx*ex + fy*ey + fz*ez);
    m[3][3] = 1.0f;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    ID3D12Device* device = context.Device;

    // ------------------------------------------------------------------
    // Descriptor heaps
    // ------------------------------------------------------------------

    // Offscreen RTV heap (non-shader-visible)
    {
        D3D12_DESCRIPTOR_HEAP_DESC d = {};
        d.NumDescriptors = 1;
        d.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        StageThrowIfFailed(device->CreateDescriptorHeap(&d, IID_PPV_ARGS(&stage.OffscreenRtvHeap)),
            "CreateDescriptorHeap (offscreen RTV) failed.");
        stage.OffscreenRtv = stage.OffscreenRtvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    // SHADER_VISIBLE CBV_SRV_UAV heap: slot 0 = scene frame CBV, slot 1 = offscreen SRV
    {
        D3D12_DESCRIPTOR_HEAP_DESC d = {};
        d.NumDescriptors = 2;
        d.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        d.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        StageThrowIfFailed(device->CreateDescriptorHeap(&d, IID_PPV_ARGS(&stage.SrvHeap)),
            "CreateDescriptorHeap (SRV, shader-visible) failed.");
    }

    // ------------------------------------------------------------------
    // Offscreen texture (RTT)
    // ------------------------------------------------------------------
    {
        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rd.Width            = context.Width;
        rd.Height           = context.Height;
        rd.DepthOrArraySize = 1;
        rd.MipLevels        = 1;
        rd.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        rd.SampleDesc.Count = 1;
        rd.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rd.Flags            = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE cv = {};
        cv.Format   = DXGI_FORMAT_R8G8B8A8_UNORM;
        cv.Color[0] = stage.OffscreenClearColor[0];
        cv.Color[1] = stage.OffscreenClearColor[1];
        cv.Color[2] = stage.OffscreenClearColor[2];
        cv.Color[3] = stage.OffscreenClearColor[3];

        StageThrowIfFailed(
            device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
                D3D12_RESOURCE_STATE_RENDER_TARGET, &cv,
                IID_PPV_ARGS(&stage.OffscreenTexture)),
            "CreateCommittedResource (offscreen texture) failed.");

        // Create RTV for offscreen texture
        device->CreateRenderTargetView(stage.OffscreenTexture.Get(), nullptr, stage.OffscreenRtv);

        // Create SRV in slot 1 of the SHADER_VISIBLE heap
        const UINT srvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = stage.SrvHeap->GetCPUDescriptorHandleForHeapStart();
        srvCpu.ptr += srvDescSize;  // slot 1

        D3D12_GPU_DESCRIPTOR_HANDLE srvGpu = stage.SrvHeap->GetGPUDescriptorHandleForHeapStart();
        srvGpu.ptr += srvDescSize;
        stage.OffscreenSrv = srvGpu;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels     = 1;
        device->CreateShaderResourceView(stage.OffscreenTexture.Get(), &srvDesc, srvCpu);
    }

    // ------------------------------------------------------------------
    // Per-frame CBV: persistently mapped upload buffer, slot 0 in SRV heap
    // ------------------------------------------------------------------
    {
        stage.SceneFrameCbv = CreateUploadBuffer(device, nullptr, sizeof(SceneFrameConstants));
        D3D12_RANGE readRange = {};
        stage.SceneFrameCbv->Map(0, &readRange, reinterpret_cast<void**>(&stage.MappedFrame));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = stage.SceneFrameCbv->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes    = sizeof(SceneFrameConstants);
        device->CreateConstantBufferView(&cbvDesc, stage.SrvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // ------------------------------------------------------------------
    // Scene root signature:
    //   param 0: CBV descriptor table b0 — per-frame (ViewProj, LightDir, CameraPos)
    //   param 1: 20 root constants  b1  — per-object (WorldMatrix 4x4 + DiffuseColor + SpecPow)
    // ------------------------------------------------------------------
    {
        D3D12_DESCRIPTOR_RANGE cbvRange = {};
        cbvRange.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        cbvRange.NumDescriptors     = 1;
        cbvRange.BaseShaderRegister = 0;
        cbvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER params[2] = {};
        params[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[0].DescriptorTable.NumDescriptorRanges = 1;
        params[0].DescriptorTable.pDescriptorRanges   = &cbvRange;
        params[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        params[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        params[1].Constants.Num32BitValues  = 20;
        params[1].Constants.ShaderRegister  = 1;
        params[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 2;
        rsDesc.pParameters   = params;
        rsDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (scene) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.SceneRootSignature)),
            "CreateRootSignature (scene) failed.");
    }

    // ------------------------------------------------------------------
    // Post root signature:
    //   param 0: SRV descriptor table t0 — offscreen scene texture
    //   static sampler s0 — linear clamp
    //   param 1: 4 root constants b0 — time, vignette strength, 2 unused
    // ------------------------------------------------------------------
    {
        D3D12_DESCRIPTOR_RANGE srvRange = {};
        srvRange.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors     = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER params[2] = {};
        params[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[0].DescriptorTable.NumDescriptorRanges = 1;
        params[0].DescriptorTable.pDescriptorRanges   = &srvRange;
        params[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;

        params[1].ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        params[1].Constants.Num32BitValues = 4;
        params[1].Constants.ShaderRegister = 0;
        params[1].ShaderVisibility         = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter          = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.ShaderRegister  = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters     = 2;
        rsDesc.pParameters       = params;
        rsDesc.NumStaticSamplers = 1;
        rsDesc.pStaticSamplers   = &sampler;
        rsDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sig, sigErr;
        StageThrowIfFailed(
            D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &sigErr),
            sigErr ? static_cast<const char*>(sigErr->GetBufferPointer()) : "SerializeRootSignature (post) failed.");
        StageThrowIfFailed(
            device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&stage.PostRootSignature)),
            "CreateRootSignature (post) failed.");
    }

    // ------------------------------------------------------------------
    // Compile shaders
    // ------------------------------------------------------------------
    ComPtr<ID3DBlob> sceneVS = CompileStageShader(L"shaders\\Mini Renderer.hlsl", "SceneVS", "vs_5_0");
    ComPtr<ID3DBlob> scenePS = CompileStageShader(L"shaders\\Mini Renderer.hlsl", "ScenePS", "ps_5_0");
    ComPtr<ID3DBlob> postVS  = CompileStageShader(L"shaders\\Mini Renderer.hlsl", "PostVS",  "vs_5_0");
    ComPtr<ID3DBlob> postPS  = CompileStageShader(L"shaders\\Mini Renderer.hlsl", "PostPS",  "ps_5_0");

    // ------------------------------------------------------------------
    // Shared unit quad geometry: 3D position (x,y,z) + UV
    // Object space: (-1..1) in XY, 0 in Z
    // ------------------------------------------------------------------
    struct QuadVertex { float x, y, z, u, v; };
    const QuadVertex verts[] = {
        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
    };
    const uint16_t idxs[] = { 0, 1, 2, 0, 2, 3 };

    stage.QuadVertexBuffer = CreateUploadBuffer(device, verts, sizeof(verts));
    stage.QuadIndexBuffer  = CreateUploadBuffer(device, idxs,  sizeof(idxs));
    stage.QuadVertexBufferView = { stage.QuadVertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(QuadVertex) };
    stage.QuadIndexBufferView  = { stage.QuadIndexBuffer->GetGPUVirtualAddress(),  sizeof(idxs),  DXGI_FORMAT_R16_UINT };

    D3D12_INPUT_ELEMENT_DESC elems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ------------------------------------------------------------------
    // Scene PSO (opaque, renders to offscreen texture)
    // ------------------------------------------------------------------
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature    = stage.SceneRootSignature.Get();
        pso.VS                = { sceneVS->GetBufferPointer(), sceneVS->GetBufferSize() };
        pso.PS                = { scenePS->GetBufferPointer(), scenePS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask        = UINT_MAX;
        pso.RasterizerState.FillMode        = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode        = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable   = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout       = { elems, _countof(elems) };
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets  = 1;
        pso.RTVFormats[0]     = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count  = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.ScenePipelineState)),
            "CreateGraphicsPipelineState (scene) failed.");
    }

    // ------------------------------------------------------------------
    // Post PSO (fullscreen quad, samples offscreen SRV, outputs to back buffer)
    // ------------------------------------------------------------------
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature    = stage.PostRootSignature.Get();
        pso.VS                = { postVS->GetBufferPointer(), postVS->GetBufferSize() };
        pso.PS                = { postPS->GetBufferPointer(), postPS->GetBufferSize() };
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask        = UINT_MAX;
        pso.RasterizerState.FillMode        = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode        = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable   = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.InputLayout       = { nullptr, 0 };   // SV_VertexID driven
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets  = 1;
        pso.RTVFormats[0]     = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count  = 1;
        StageThrowIfFailed(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&stage.PostPipelineState)),
            "CreateGraphicsPipelineState (post) failed.");
    }

    stage.Viewport = { 0.0f, 0.0f, static_cast<float>(context.Width), static_cast<float>(context.Height), 0.0f, 1.0f };
    stage.Scissor  = { 0, 0, static_cast<LONG>(context.Width), static_cast<LONG>(context.Height) };
}

// ---------------------------------------------------------------------------
// Update — animate camera, objects, and light; write per-frame CBV
// ---------------------------------------------------------------------------

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.TimeSeconds = static_cast<float>(timeSeconds);
    const float t = stage.TimeSeconds;

    if (!stage.MappedFrame)
        return;

    // Animated camera orbiting the origin at radius 2.5, height 1.5
    const float camAngle = t * 0.25f;
    const float camRadius = 2.5f;
    const float camX = cosf(camAngle) * camRadius;
    const float camY = 1.5f;
    const float camZ = sinf(camAngle) * camRadius;

    // Light direction slowly sweeps
    const float lx = cosf(t * 0.35f) * 0.7f;
    const float ly = 0.6f;
    const float lz = sinf(t * 0.20f) * 0.5f + 0.3f;
    const float ll = sqrtf(lx*lx + ly*ly + lz*lz);

    stage.MappedFrame->LightDir[0]  = lx / ll;
    stage.MappedFrame->LightDir[1]  = ly / ll;
    stage.MappedFrame->LightDir[2]  = lz / ll;
    stage.MappedFrame->CameraPos[0] = camX;
    stage.MappedFrame->CameraPos[1] = camY;
    stage.MappedFrame->CameraPos[2] = camZ;

    // Build ViewProj
    float view[4][4], proj[4][4], viewProj[4][4];
    MatLookAt(view, camX, camY, camZ, 0.0f, 0.0f, 0.0f);
    // Aspect ratio: we only have the time, not the context here; use the viewport values baked at setup.
    // We extract width/height from the viewport stored at setup time.
    const float aspect = (stage.Viewport.Height > 0.0f)
        ? stage.Viewport.Width / stage.Viewport.Height
        : 16.0f / 9.0f;
    MatPerspective(proj, 1.2217f /* ~70 deg */, aspect, 0.1f, 100.0f);
    MatMul4x4(viewProj, view, proj);
    std::memcpy(stage.MappedFrame->ViewProj, viewProj, sizeof(viewProj));
}

// ---------------------------------------------------------------------------
// Render — two passes
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    ID3D12GraphicsCommandList* cmd = context.CommandList;
    const float t = stage.TimeSeconds;

    // Bind the SHADER_VISIBLE heap for both passes
    ID3D12DescriptorHeap* heaps[] = { stage.SrvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);

    // ==================================================================
    // Pass 1 — Scene pass: render lit objects into the offscreen texture
    // ==================================================================
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->OMSetRenderTargets(1, &stage.OffscreenRtv, FALSE, nullptr);
    cmd->ClearRenderTargetView(stage.OffscreenRtv, stage.OffscreenClearColor, 0, nullptr);

    cmd->SetGraphicsRootSignature(stage.SceneRootSignature.Get());
    cmd->SetPipelineState(stage.ScenePipelineState.Get());

    // Param 0: CBV descriptor table (slot 0 in the heap = SceneFrameCbv)
    cmd->SetGraphicsRootDescriptorTable(0, stage.SrvHeap->GetGPUDescriptorHandleForHeapStart());

    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &stage.QuadVertexBufferView);
    cmd->IASetIndexBuffer(&stage.QuadIndexBufferView);

    // Per-object root constants: WorldMatrix[4][4] (16 floats) + DiffuseColor[3] + SpecPow (4 floats) = 20
    struct ObjectConstants
    {
        float WorldMatrix[4][4];
        float DiffuseColor[3];
        float SpecularPower;
    };

    auto DrawObject = [&](const ObjectConstants& obj)
    {
        cmd->SetGraphicsRoot32BitConstants(1, 20, &obj, 0);
        cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    };

    // Object 0 — floor: wide flat quad (scale 1.5, 0.02, 1.5), centered at origin
    {
        ObjectConstants obj = {};
        float scale[4][4], trans[4][4], world[4][4];
        MatScale(scale, 1.5f, 0.02f, 1.5f);
        MatTranslate(trans, 0.0f, -0.5f, 0.0f);
        MatMul4x4(world, scale, trans);
        std::memcpy(obj.WorldMatrix, world, sizeof(world));
        obj.DiffuseColor[0] = 0.42f; obj.DiffuseColor[1] = 0.40f; obj.DiffuseColor[2] = 0.38f;
        obj.SpecularPower   = 8.0f;
        DrawObject(obj);
    }

    // Objects 1-4 — colored boxes orbiting at different radii and speeds
    struct OrbitDesc { float radius; float speed; float height; float r, g, b; float specPow; };
    const OrbitDesc orbits[] = {
        { 0.60f, 1.10f,  0.0f,  0.90f, 0.25f, 0.18f, 60.0f  },   // red-orange
        { 0.90f, 0.70f,  0.1f,  0.20f, 0.65f, 0.90f, 120.0f },   // cool blue
        { 1.20f, 0.45f, -0.1f,  0.25f, 0.85f, 0.35f, 40.0f  },   // green
        { 0.75f, 1.50f,  0.2f,  0.85f, 0.75f, 0.20f, 200.0f },   // gold
    };

    for (int i = 0; i < 4; ++i)
    {
        const OrbitDesc& o = orbits[i];
        const float angle  = t * o.speed + static_cast<float>(i) * 1.5708f;  // 90 deg offset each
        const float wx     = cosf(angle) * o.radius;
        const float wz     = sinf(angle) * o.radius;
        const float wy     = o.height;

        ObjectConstants obj = {};
        float scale[4][4], rotY[4][4], trans[4][4], tmp[4][4], world[4][4];
        MatScale(scale, 0.15f, 0.15f, 0.15f);
        MatRotateY(rotY, t * 1.3f + static_cast<float>(i));
        MatTranslate(trans, wx, wy, wz);
        MatMul4x4(tmp, scale, rotY);
        MatMul4x4(world, tmp, trans);
        std::memcpy(obj.WorldMatrix, world, sizeof(world));
        obj.DiffuseColor[0] = o.r;
        obj.DiffuseColor[1] = o.g;
        obj.DiffuseColor[2] = o.b;
        obj.SpecularPower   = o.specPow;
        DrawObject(obj);
    }

    // Transition offscreen texture: RenderTarget -> PixelShaderResource
    {
        D3D12_RESOURCE_BARRIER barrier = MakeTransitionBarrier(
            stage.OffscreenTexture.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cmd->ResourceBarrier(1, &barrier);
    }

    // ==================================================================
    // Pass 2 — Post pass: tone-map + vignette, output to back buffer
    // ==================================================================
    cmd->RSSetViewports(1, &stage.Viewport);
    cmd->RSSetScissorRects(1, &stage.Scissor);
    cmd->OMSetRenderTargets(1, &context.RenderTargetView, FALSE, nullptr);
    cmd->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);

    cmd->SetGraphicsRootSignature(stage.PostRootSignature.Get());
    cmd->SetPipelineState(stage.PostPipelineState.Get());

    // Param 0: SRV descriptor table — slot 1 in the heap = offscreen SRV
    cmd->SetGraphicsRootDescriptorTable(0, stage.OffscreenSrv);

    // Param 1: 4 root constants — time, vignette strength (1.0 = full), 2 unused
    const float postConstants[4] = { t, 1.0f, 0.0f, 0.0f };
    cmd->SetGraphicsRoot32BitConstants(1, 4, postConstants, 0);

    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->DrawInstanced(6, 1, 0, 0);  // fullscreen quad from SV_VertexID

    // Transition offscreen texture back: PixelShaderResource -> RenderTarget (ready for next frame)
    {
        D3D12_RESOURCE_BARRIER barrier = MakeTransitionBarrier(
            stage.OffscreenTexture.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmd->ResourceBarrier(1, &barrier);
    }
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.MappedFrame)
    {
        stage.SceneFrameCbv->Unmap(0, nullptr);
        stage.MappedFrame = nullptr;
    }
    stage.QuadIndexBuffer.Reset();
    stage.QuadVertexBuffer.Reset();
    stage.SceneFrameCbv.Reset();
    stage.OffscreenTexture.Reset();
    stage.SrvHeap.Reset();
    stage.OffscreenRtvHeap.Reset();
    stage.PostPipelineState.Reset();
    stage.PostRootSignature.Reset();
    stage.ScenePipelineState.Reset();
    stage.SceneRootSignature.Reset();
}
