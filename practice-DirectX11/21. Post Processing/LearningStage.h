#pragma once
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <cstring>

// Post Processing sample:
// 1. main.cpp renders the normal 3D scene to the back buffer.
// 2. This stage copies that back buffer into a shader-readable texture.
// 3. A full-screen pass samples the copied scene and writes a grayscale result.
// 4. A small inset shows the original copied scene for comparison.

struct PostProcessVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 Uv;
};

inline ID3D11Texture2D*          g_stagePostTexture = nullptr;
inline ID3D11ShaderResourceView* g_stagePostSRV = nullptr;
inline ID3D11VertexShader*       g_stagePostVS = nullptr;
inline ID3D11PixelShader*        g_stageGrayscalePS = nullptr;
inline ID3D11PixelShader*        g_stagePassthroughPS = nullptr;
inline ID3D11InputLayout*        g_stagePostLayout = nullptr;
inline ID3D11Buffer*             g_stagePostVB = nullptr;
inline ID3D11SamplerState*       g_stagePostSampler = nullptr;
inline ID3D11DepthStencilState*  g_stagePostNoDepth = nullptr;
inline unsigned int              g_stagePostWidth = 0;
inline unsigned int              g_stagePostHeight = 0;

inline HRESULT CompilePostShader( const char* src, const char* entry, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* err = nullptr;
    HRESULT hr = D3DCompile( src, strlen( src ), nullptr, nullptr, nullptr, entry, profile,
                             D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &err );
    if( FAILED( hr ) && err )
        OutputDebugStringA( static_cast<const char*>( err->GetBufferPointer() ) );
    if( err )
        err->Release();
    return hr;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    const char* shader = R"(
Texture2D SceneTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct VSI { float3 Pos : POSITION; float2 Uv : TEXCOORD0; };
struct PSI { float4 Pos : SV_POSITION; float2 Uv : TEXCOORD0; };

PSI VS(VSI i)
{
    PSI o;
    o.Pos = float4(i.Pos, 1);
    o.Uv = i.Uv;
    return o;
}

float4 PSGrayscale(PSI i) : SV_Target
{
    float4 c = SceneTexture.Sample(LinearSampler, i.Uv);
    float g = dot(c.rgb, float3(0.299f, 0.587f, 0.114f));
    return float4(g, g, g, c.a);
}

float4 PSPassthrough(PSI i) : SV_Target
{
    return SceneTexture.Sample(LinearSampler, i.Uv);
}
)";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if( SUCCEEDED( CompilePostShader( shader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stagePostVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                   &g_stagePostLayout );
        vsBlob->Release();
    }

    if( SUCCEEDED( CompilePostShader( shader, "PSGrayscale", "ps_4_0", &psBlob ) ) )
    {
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageGrayscalePS );
        psBlob->Release();
    }
    psBlob = nullptr;
    if( SUCCEEDED( CompilePostShader( shader, "PSPassthrough", "ps_4_0", &psBlob ) ) )
    {
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stagePassthroughPS );
        psBlob->Release();
    }

    PostProcessVertex verts[12] =
    {
        // [0..5] final post-process full-screen output
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -1,  1, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3(  1, -1, 0 ), DirectX::XMFLOAT2( 1, 1 ) },

        // [6..11] original source inset
        { DirectX::XMFLOAT3( -0.95f, -0.95f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.95f, -0.55f, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3( -0.45f, -0.55f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.95f, -0.95f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.45f, -0.55f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.45f, -0.95f, 0 ), DirectX::XMFLOAT2( 1, 1 ) },
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof( verts );
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = verts;
    device->CreateBuffer( &vbd, &vbData, &g_stagePostVB );

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState( &sampler, &g_stagePostSampler );

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = FALSE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
    device->CreateDepthStencilState( &dsd, &g_stagePostNoDepth );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stagePostNoDepth ) g_stagePostNoDepth->Release();
    if( g_stagePostSampler ) g_stagePostSampler->Release();
    if( g_stagePostVB ) g_stagePostVB->Release();
    if( g_stagePostLayout ) g_stagePostLayout->Release();
    if( g_stagePassthroughPS ) g_stagePassthroughPS->Release();
    if( g_stageGrayscalePS ) g_stageGrayscalePS->Release();
    if( g_stagePostVS ) g_stagePostVS->Release();
    if( g_stagePostSRV ) g_stagePostSRV->Release();
    if( g_stagePostTexture ) g_stagePostTexture->Release();
    g_stagePostNoDepth = nullptr;
    g_stagePostSampler = nullptr;
    g_stagePostVB = nullptr;
    g_stagePostLayout = nullptr;
    g_stagePassthroughPS = nullptr;
    g_stageGrayscalePS = nullptr;
    g_stagePostVS = nullptr;
    g_stagePostSRV = nullptr;
    g_stagePostTexture = nullptr;
    g_stagePostWidth = 0;
    g_stagePostHeight = 0;
}

inline void UpdateStageSpecificDemo( float ) {}

inline bool CaptureBackBufferForPost( ID3D11DeviceContext* ctx, ID3D11RenderTargetView* backRTV )
{
    ID3D11Resource* backResource = nullptr;
    backRTV->GetResource( &backResource );
    ID3D11Texture2D* backTexture = nullptr;
    if( backResource )
        backResource->QueryInterface( __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &backTexture ) );
    if( backResource )
        backResource->Release();
    if( !backTexture )
        return false;

    D3D11_TEXTURE2D_DESC backDesc = {};
    backTexture->GetDesc( &backDesc );
    if( !g_stagePostTexture || g_stagePostWidth != backDesc.Width || g_stagePostHeight != backDesc.Height )
    {
        if( g_stagePostSRV )
        {
            g_stagePostSRV->Release();
            g_stagePostSRV = nullptr;
        }
        if( g_stagePostTexture )
        {
            g_stagePostTexture->Release();
            g_stagePostTexture = nullptr;
        }

        ID3D11Device* device = nullptr;
        ctx->GetDevice( &device );
        D3D11_TEXTURE2D_DESC postDesc = backDesc;
        postDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        postDesc.CPUAccessFlags = 0;
        postDesc.MiscFlags = 0;
        postDesc.Usage = D3D11_USAGE_DEFAULT;
        if( device )
        {
            device->CreateTexture2D( &postDesc, nullptr, &g_stagePostTexture );
            if( g_stagePostTexture )
                device->CreateShaderResourceView( g_stagePostTexture, nullptr, &g_stagePostSRV );
            device->Release();
        }
        g_stagePostWidth = backDesc.Width;
        g_stagePostHeight = backDesc.Height;
    }

    if( g_stagePostTexture )
        ctx->CopyResource( g_stagePostTexture, backTexture );
    backTexture->Release();
    return g_stagePostTexture && g_stagePostSRV;
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* ctx, ID3D11RenderTargetView* backRTV,
                                      ID3D11DepthStencilView* depthDSV )
{
    if( !g_stagePostVS || !g_stageGrayscalePS || !g_stagePassthroughPS || !g_stagePostLayout ||
        !g_stagePostVB || !g_stagePostSampler || !g_stagePostNoDepth )
        return;
    if( !CaptureBackBufferForPost( ctx, backRTV ) )
        return;

    ID3D11InputLayout* prevIL = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* prevVB = nullptr;
    UINT prevStride = 0;
    UINT prevOffset = 0;
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11ShaderResourceView* prevSRV = nullptr;
    ID3D11SamplerState* prevSamp = nullptr;
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;

    ctx->IAGetInputLayout( &prevIL );
    ctx->IAGetPrimitiveTopology( &prevTopo );
    ctx->IAGetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSGetShader( &prevVS, nullptr, nullptr );
    ctx->PSGetShader( &prevPS, nullptr, nullptr );
    ctx->PSGetShaderResources( 0, 1, &prevSRV );
    ctx->PSGetSamplers( 0, 1, &prevSamp );
    ctx->OMGetDepthStencilState( &prevDSS, &prevStencilRef );

    ctx->OMSetRenderTargets( 1, &backRTV, depthDSV );
    ctx->OMSetDepthStencilState( g_stagePostNoDepth, 0 );
    ctx->IASetInputLayout( g_stagePostLayout );
    ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    UINT stride = sizeof( PostProcessVertex );
    UINT offset = 0;
    ctx->IASetVertexBuffers( 0, 1, &g_stagePostVB, &stride, &offset );
    ctx->VSSetShader( g_stagePostVS, nullptr, 0 );
    ctx->PSSetShaderResources( 0, 1, &g_stagePostSRV );
    ctx->PSSetSamplers( 0, 1, &g_stagePostSampler );

    ctx->PSSetShader( g_stageGrayscalePS, nullptr, 0 );
    ctx->Draw( 6, 0 );

    ctx->PSSetShader( g_stagePassthroughPS, nullptr, 0 );
    ctx->Draw( 6, 6 );

    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources( 0, 1, &nullSRV );

    ctx->IASetInputLayout( prevIL );
    ctx->IASetPrimitiveTopology( prevTopo );
    ctx->IASetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSSetShader( prevVS, nullptr, 0 );
    ctx->PSSetShader( prevPS, nullptr, 0 );
    ctx->PSSetShaderResources( 0, 1, &prevSRV );
    ctx->PSSetSamplers( 0, 1, &prevSamp );
    ctx->OMSetDepthStencilState( prevDSS, prevStencilRef );

    if( prevIL ) prevIL->Release();
    if( prevVB ) prevVB->Release();
    if( prevVS ) prevVS->Release();
    if( prevPS ) prevPS->Release();
    if( prevSRV ) prevSRV->Release();
    if( prevSamp ) prevSamp->Release();
    if( prevDSS ) prevDSS->Release();
}
