#pragma once
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <cstring>

using namespace DirectX;

struct ShadowPreviewVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Uv;
};

// Must match ConstantBuffer in main.cpp exactly so the existing VS can be reused for the shadow pass
struct ShadowLightCB
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMMATRIX mLightViewProj;
    XMFLOAT4 vLightDir[2];
    XMFLOAT4 vLightColor[2];
    XMFLOAT4 vMaterialDiffuse;
    XMFLOAT4 vOutputColor;
};

inline ID3D11Texture2D*          g_stageShadowMap           = nullptr;
inline ID3D11DepthStencilView*   g_stageShadowDSV           = nullptr;
inline ID3D11ShaderResourceView* g_stageShadowSRV           = nullptr;
inline ID3D11SamplerState*       g_stageShadowSampler       = nullptr;
inline ID3D11VertexShader*       g_stageShadowPreviewVS     = nullptr;
inline ID3D11PixelShader*        g_stageShadowPreviewPS     = nullptr;
inline ID3D11InputLayout*        g_stageShadowPreviewLayout = nullptr;
inline ID3D11Buffer*             g_stageShadowPreviewVB     = nullptr;
inline ID3D11Buffer*             g_stageLightCB             = nullptr;

inline float    g_stageCurTime   = 0.0f;
inline XMMATRIX g_stageLightView;
inline XMMATRIX g_stageLightProj;

// Matches vLightDirs[0] in main.cpp  (direction toward the light, world space)
static const XMFLOAT3 k_LightDir0 = { -0.577f, 0.577f, -0.577f };

inline HRESULT CompileShadowShader( const char* source, const char* entryPoint, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile( source, strlen( source ), nullptr, nullptr, nullptr, entryPoint, profile,
                             D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &errorBlob );
    if( FAILED( hr ) && errorBlob )
        OutputDebugStringA( static_cast<const char*>( errorBlob->GetBufferPointer() ) );
    if( errorBlob ) errorBlob->Release();
    return hr;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    // -----------------------------------------------------------------------
    // Shadow map texture (R24G8_TYPELESS so it can bind as both DSV and SRV)
    // -----------------------------------------------------------------------
    D3D11_TEXTURE2D_DESC tex = {};
    tex.Width = 1024;  tex.Height = 1024;
    tex.MipLevels = 1; tex.ArraySize = 1;
    tex.Format = DXGI_FORMAT_R24G8_TYPELESS;
    tex.SampleDesc.Count = 1;
    tex.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    device->CreateTexture2D( &tex, nullptr, &g_stageShadowMap );

    if( g_stageShadowMap )
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        device->CreateDepthStencilView( g_stageShadowMap, &dsv, &g_stageShadowDSV );

        D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
        srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView( g_stageShadowMap, &srv, &g_stageShadowSRV );
    }

    // -----------------------------------------------------------------------
    // Constant buffer for light-space matrices (same layout as main CB)
    // -----------------------------------------------------------------------
    D3D11_BUFFER_DESC lbd = {};
    lbd.Usage = D3D11_USAGE_DEFAULT;
    lbd.ByteWidth = sizeof( ShadowLightCB );
    lbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &lbd, nullptr, &g_stageLightCB );

    // Build light view: directional light sits 10 units back along the light direction
    XMVECTOR lightPos = XMVectorSet( -k_LightDir0.x * 10.0f,
                                     -k_LightDir0.y * 10.0f,
                                     -k_LightDir0.z * 10.0f, 0.0f );
    g_stageLightView = XMMatrixLookAtLH( lightPos, XMVectorZero(), XMVectorSet( 0, 1, 0, 0 ) );
    // Orthographic projection covers the +/-4 unit world space around the origin
    g_stageLightProj = XMMatrixOrthographicLH( 8.0f, 8.0f, 0.1f, 100.0f );

    // -----------------------------------------------------------------------
    // Shadow map preview shaders (small quad in the bottom-right corner)
    // -----------------------------------------------------------------------
    const char* previewShader = R"(
Texture2D ShadowMap : register(t1);
SamplerState ShadowSampler : register(s1);

struct VS_INPUT { float3 Pos : POSITION; float2 Uv : TEXCOORD0; };
struct PS_INPUT { float4 Pos : SV_POSITION; float2 Uv : TEXCOORD0; };

PS_INPUT VS(VS_INPUT i) { PS_INPUT o; o.Pos = float4(i.Pos, 1); o.Uv = i.Uv; return o; }
float4 PS(PS_INPUT i) : SV_Target
{
    float d = ShadowMap.Sample(ShadowSampler, i.Uv).r;
    float visibleDepth = pow( saturate( 1.0f - d ), 0.45f );
    return float4(visibleDepth, visibleDepth, visibleDepth, 1);
}
)";

    ID3DBlob* vsBlob = nullptr, *psBlob = nullptr;
    if( SUCCEEDED( CompileShadowShader( previewShader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageShadowPreviewVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ),
                                   vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_stageShadowPreviewLayout );
        vsBlob->Release();
    }

    if( SUCCEEDED( CompileShadowShader( previewShader, "PS", "ps_4_0", &psBlob ) ) )
    {
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageShadowPreviewPS );
        psBlob->Release();
    }

    ShadowPreviewVertex quad[] =
    {
        { XMFLOAT3( 0.48f, -0.96f, 0.0f ), XMFLOAT2( 0, 1 ) },
        { XMFLOAT3( 0.48f, -0.38f, 0.0f ), XMFLOAT2( 0, 0 ) },
        { XMFLOAT3( 0.98f, -0.38f, 0.0f ), XMFLOAT2( 1, 0 ) },
        { XMFLOAT3( 0.48f, -0.96f, 0.0f ), XMFLOAT2( 0, 1 ) },
        { XMFLOAT3( 0.98f, -0.38f, 0.0f ), XMFLOAT2( 1, 0 ) },
        { XMFLOAT3( 0.98f, -0.96f, 0.0f ), XMFLOAT2( 1, 1 ) },
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof( quad );
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = quad;
    device->CreateBuffer( &vbd, &vbData, &g_stageShadowPreviewVB );

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD   = D3D11_FLOAT32_MAX;
    device->CreateSamplerState( &sampler, &g_stageShadowSampler );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageLightCB             ) { g_stageLightCB->Release();             g_stageLightCB             = nullptr; }
    if( g_stageShadowPreviewVB     ) { g_stageShadowPreviewVB->Release();     g_stageShadowPreviewVB     = nullptr; }
    if( g_stageShadowPreviewLayout ) { g_stageShadowPreviewLayout->Release(); g_stageShadowPreviewLayout = nullptr; }
    if( g_stageShadowPreviewPS     ) { g_stageShadowPreviewPS->Release();     g_stageShadowPreviewPS     = nullptr; }
    if( g_stageShadowPreviewVS     ) { g_stageShadowPreviewVS->Release();     g_stageShadowPreviewVS     = nullptr; }
    if( g_stageShadowSampler       ) { g_stageShadowSampler->Release();       g_stageShadowSampler       = nullptr; }
    if( g_stageShadowSRV           ) { g_stageShadowSRV->Release();           g_stageShadowSRV           = nullptr; }
    if( g_stageShadowDSV           ) { g_stageShadowDSV->Release();           g_stageShadowDSV           = nullptr; }
    if( g_stageShadowMap           ) { g_stageShadowMap->Release();           g_stageShadowMap           = nullptr; }
}

inline void UpdateStageSpecificDemo( float t ) { g_stageCurTime = t; }

inline XMMATRIX GetStageLightViewProj()
{
    return g_stageLightView * g_stageLightProj;
}

inline bool StageShadowResourcesReady()
{
    return g_stageShadowDSV && g_stageShadowSRV && g_stageShadowSampler && g_stageLightCB &&
           g_stageShadowPreviewVS && g_stageShadowPreviewPS && g_stageShadowPreviewLayout &&
           g_stageShadowPreviewVB;
}

inline void ApplyStageSpecificShadowPass( ID3D11DeviceContext* ctx,
                                          ID3D11RenderTargetView* backRTV,
                                          ID3D11DepthStencilView* depthDSV )
{
    if( !StageShadowResourcesReady() )
        return;

    D3D11_VIEWPORT savedVPs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
    UINT vpCount = ARRAYSIZE( savedVPs );
    ctx->RSGetViewports( &vpCount, savedVPs );

    ID3D11Buffer*        savedCB      = nullptr;
    ID3D11InputLayout*   savedIL      = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY savedTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer*        savedVB      = nullptr;
    UINT                 savedStride  = 0, savedOffset = 0;
    ID3D11VertexShader*  savedVS      = nullptr;
    ID3D11PixelShader*   savedPS      = nullptr;
    ID3D11ShaderResourceView* savedSRV1    = nullptr;
    ID3D11SamplerState*       savedSamp1   = nullptr;

    ctx->VSGetConstantBuffers( 0, 1, &savedCB );
    ctx->IAGetInputLayout( &savedIL );
    ctx->IAGetPrimitiveTopology( &savedTopo );
    ctx->IAGetVertexBuffers( 0, 1, &savedVB, &savedStride, &savedOffset );
    ctx->VSGetShader( &savedVS, nullptr, nullptr );
    ctx->PSGetShader( &savedPS, nullptr, nullptr );
    ctx->PSGetShaderResources( 1, 1, &savedSRV1 );
    ctx->PSGetSamplers( 1, 1, &savedSamp1 );

    ShadowLightCB lcb = {};
    lcb.mWorld      = XMMatrixTranspose( XMMatrixRotationY( g_stageCurTime ) );
    lcb.mView       = XMMatrixTranspose( g_stageLightView );
    lcb.mProjection = XMMatrixTranspose( g_stageLightProj );
    lcb.mLightViewProj = XMMatrixTranspose( GetStageLightViewProj() );
    lcb.vLightDir[0]    = XMFLOAT4( k_LightDir0.x, k_LightDir0.y, k_LightDir0.z, 1 );
    lcb.vLightDir[1]    = XMFLOAT4( 0, 0, -1, 1 );
    lcb.vLightColor[0]  = XMFLOAT4( 1, 1, 1, 1 );
    lcb.vLightColor[1]  = XMFLOAT4( 0, 0, 0, 1 );
    lcb.vMaterialDiffuse = XMFLOAT4( 1, 1, 1, 1 );
    lcb.vOutputColor    = XMFLOAT4( 0, 0, 0, 0 );
    ctx->UpdateSubresource( g_stageLightCB, 0, nullptr, &lcb, 0, 0 );

    // Override b0 with light-space matrices so the existing main VS transforms
    // the cube into light clip space during the shadow depth pass
    ctx->VSSetConstantBuffers( 0, 1, &g_stageLightCB );

    // Unbind shadow SRV first (can't read from a texture that is bound as DSV)
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources( 1, 1, &nullSRV );

    D3D11_VIEWPORT shadowVP = { 0, 0, 1024.0f, 1024.0f, 0.0f, 1.0f };
    ctx->RSSetViewports( 1, &shadowVP );
    ctx->OMSetRenderTargets( 0, nullptr, g_stageShadowDSV );
    ctx->ClearDepthStencilView( g_stageShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
    ctx->PSSetShader( nullptr, nullptr, 0 );   // depth-only: no pixel shader
    ctx->DrawIndexed( 36, 0, 0 );              // same cube mesh as main scene

    ctx->OMSetRenderTargets( 1, &backRTV, depthDSV );
    if( vpCount > 0 ) ctx->RSSetViewports( vpCount, savedVPs );
    ctx->VSSetConstantBuffers( 0, 1, &savedCB );
    ctx->IASetInputLayout( savedIL );
    ctx->IASetPrimitiveTopology( savedTopo );
    ctx->IASetVertexBuffers( 0, 1, &savedVB, &savedStride, &savedOffset );
    ctx->VSSetShader( savedVS, nullptr, 0 );
    ctx->PSSetShader( savedPS, nullptr, 0 );
    ctx->PSSetShaderResources( 1, 1, &savedSRV1 );
    ctx->PSSetSamplers( 1, 1, &savedSamp1 );

    if( savedCB    ) savedCB->Release();
    if( savedIL    ) savedIL->Release();
    if( savedVB    ) savedVB->Release();
    if( savedVS    ) savedVS->Release();
    if( savedPS    ) savedPS->Release();
    if( savedSRV1  ) savedSRV1->Release();
    if( savedSamp1 ) savedSamp1->Release();
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* ctx,
                                      ID3D11RenderTargetView* backRTV,
                                      ID3D11DepthStencilView* depthDSV )
{
    if( !StageShadowResourcesReady() )
        return;

    ID3D11InputLayout*   savedIL      = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY savedTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer*        savedVB      = nullptr;
    UINT                 savedStride  = 0, savedOffset = 0;
    ID3D11VertexShader*  savedVS      = nullptr;
    ID3D11PixelShader*   savedPS      = nullptr;
    ID3D11ShaderResourceView* savedSRV1    = nullptr;
    ID3D11SamplerState*       savedSamp1   = nullptr;

    ctx->IAGetInputLayout( &savedIL );
    ctx->IAGetPrimitiveTopology( &savedTopo );
    ctx->IAGetVertexBuffers( 0, 1, &savedVB, &savedStride, &savedOffset );
    ctx->VSGetShader( &savedVS, nullptr, nullptr );
    ctx->PSGetShader( &savedPS, nullptr, nullptr );
    ctx->PSGetShaderResources( 1, 1, &savedSRV1 );
    ctx->PSGetSamplers( 1, 1, &savedSamp1 );

    // -----------------------------------------------------------------------
    // Preview pass: display the shadow depth map as a small inset in the corner
    // -----------------------------------------------------------------------
    ctx->IASetInputLayout( g_stageShadowPreviewLayout );
    ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    UINT stride = sizeof( ShadowPreviewVertex ), offset = 0;
    ctx->IASetVertexBuffers( 0, 1, &g_stageShadowPreviewVB, &stride, &offset );
    ctx->VSSetShader( g_stageShadowPreviewVS, nullptr, 0 );
    ctx->PSSetShader( g_stageShadowPreviewPS, nullptr, 0 );
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources( 1, 1, &g_stageShadowSRV );
    ctx->PSSetSamplers( 1, 1, &g_stageShadowSampler );
    ctx->Draw( 6, 0 );

    // -----------------------------------------------------------------------
    // Restore all saved pipeline state
    // -----------------------------------------------------------------------
    ctx->PSSetShaderResources( 1, 1, &nullSRV );
    ctx->IASetInputLayout( savedIL );
    ctx->IASetPrimitiveTopology( savedTopo );
    ctx->IASetVertexBuffers( 0, 1, &savedVB, &savedStride, &savedOffset );
    ctx->VSSetShader( savedVS, nullptr, 0 );
    ctx->PSSetShader( savedPS, nullptr, 0 );
    ctx->PSSetShaderResources( 1, 1, &savedSRV1 );
    ctx->PSSetSamplers( 1, 1, &savedSamp1 );

    if( savedIL    ) savedIL->Release();
    if( savedVB    ) savedVB->Release();
    if( savedVS    ) savedVS->Release();
    if( savedPS    ) savedPS->Release();
    if( savedSRV1  ) savedSRV1->Release();
    if( savedSamp1 ) savedSamp1->Release();
}
