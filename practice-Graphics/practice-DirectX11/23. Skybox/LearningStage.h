#pragma once
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <cstring>
#include <cmath>

using namespace DirectX;

// Skybox: a fullscreen quad at max depth (z=0.9999) that samples a TextureCube.
// The per-pixel ray direction is reconstructed from the camera's inverse view matrix
// so the skybox rotates as the camera moves, matching real engine behaviour.

struct SkyboxVertex { XMFLOAT3 Pos; XMFLOAT2 Uv; };

// Separate CB for sky inverse-view (16 bytes vs full matrix)
struct SkyboxCB { XMMATRIX InvView; XMFLOAT4 ProjScale; };

inline ID3D11Texture2D*          g_stageCubeMap      = nullptr;
inline ID3D11ShaderResourceView* g_stageCubeMapSRV   = nullptr;
inline ID3D11SamplerState*       g_stageCubeSampler  = nullptr;
inline ID3D11VertexShader*       g_stageSkyboxVS     = nullptr;
inline ID3D11PixelShader*        g_stageSkyboxPS     = nullptr;
inline ID3D11InputLayout*        g_stageSkyboxLayout = nullptr;
inline ID3D11Buffer*             g_stageSkyboxQuadVB = nullptr;
inline ID3D11Buffer*             g_stageSkyboxCB     = nullptr;
inline ID3D11DepthStencilState*  g_stageSkyDSS       = nullptr;

inline HRESULT CompileSkyboxShader( const char* src, const char* entry, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* err = nullptr;
    HRESULT hr = D3DCompile( src, strlen(src), nullptr, nullptr, nullptr, entry, profile,
                             D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &err );
    if( FAILED(hr) && err ) OutputDebugStringA( (const char*)err->GetBufferPointer() );
    if( err ) err->Release();
    return hr;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    // Procedural 16x16 cubemap. Gradients make TextureCube direction sampling visible;
    // solid colors read too much like a clear color.
    const unsigned int W = 16;
    const unsigned int H = 16;
    unsigned int pixels[6][W * H];
    for( int f = 0; f < 6; ++f )
    {
        for( unsigned int y = 0; y < H; ++y )
        {
            for( unsigned int x = 0; x < W; ++x )
            {
                float u = x / float( W - 1 );
                float v = y / float( H - 1 );
                unsigned int r = 0, g = 0, b = 0;
                switch( f )
                {
                case 0: r = 60 + unsigned int( 120 * u ); g = 110 + unsigned int( 70 * v ); b = 210; break;
                case 1: r = 35 + unsigned int( 80 * v ); g = 70 + unsigned int( 60 * u ); b = 150; break;
                case 2: r = 100 + unsigned int( 80 * u ); g = 170 + unsigned int( 55 * v ); b = 255; break;
                case 3: r = 35; g = 25 + unsigned int( 55 * u ); b = 45 + unsigned int( 45 * v ); break;
                case 4: r = 80 + unsigned int( 80 * u ); g = 130 + unsigned int( 65 * v ); b = 230; break;
                default: r = 55 + unsigned int( 95 * v ); g = 95 + unsigned int( 90 * u ); b = 210; break;
                }
                pixels[f][y * W + x] = 0xff000000 | ( b << 16 ) | ( g << 8 ) | r;
            }
        }
    }

    D3D11_TEXTURE2D_DESC tex = {};
    tex.Width = W; tex.Height = H;
    tex.MipLevels = 1; tex.ArraySize = 6;
    tex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex.SampleDesc.Count = 1;
    tex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SUBRESOURCE_DATA data[6] = {};
    for( int f = 0; f < 6; ++f )
    {
        data[f].pSysMem     = pixels[f];
        data[f].SysMemPitch = W * sizeof(unsigned int);
    }
    device->CreateTexture2D( &tex, data, &g_stageCubeMap );

    if( g_stageCubeMap )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
        srv.Format = tex.Format;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srv.TextureCube.MipLevels = 1;
        device->CreateShaderResourceView( g_stageCubeMap, &srv, &g_stageCubeMapSRV );
    }

    // Skybox CB: inverse view matrix so the VS can reconstruct world-space ray directions
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(SkyboxCB);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &cbd, nullptr, &g_stageSkyboxCB );

    // Depth-stencil state: always pass (draw skybox behind everything without writing depth)
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable    = TRUE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // don't write depth
    dsd.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;  // pass at max depth (0.9999)
    device->CreateDepthStencilState( &dsd, &g_stageSkyDSS );

    // Shader: VS reconstructs a view ray per pixel using the inverse view matrix.
    // The quad sits at NDC z=0.9999, so it renders behind everything.
    const char* shader = R"(
cbuffer SkyboxCB : register(b2)
{
    matrix InvView;
    float4 ProjScale;   // .x = aspect * tan(FOV/2),  .y = tan(FOV/2)
}
TextureCube SkyboxTexture : register(t1);
SamplerState SkyboxSampler : register(s1);

struct VSI { float3 Pos : POSITION; float2 Uv : TEXCOORD0; };
struct PSI { float4 Pos : SV_POSITION; float3 Dir : TEXCOORD0; };

PSI VS(VSI i)
{
    PSI o;
    o.Pos = float4(i.Pos, 1.0f);
    // Unproject NDC to view-space ray (FOV/aspect-corrected), then into world space
    float3 viewRay = float3(
        (i.Uv.x * 2.0f - 1.0f) * ProjScale.x,
        -(i.Uv.y * 2.0f - 1.0f) * ProjScale.y,
        1.0f);
    o.Dir = mul(float4(viewRay, 0.0f), InvView).xyz;
    return o;
}

float4 PS(PSI i) : SV_Target
{
    return SkyboxTexture.Sample(SkyboxSampler, normalize(i.Dir));
}
)";

    ID3DBlob* vsBlob = nullptr, *psBlob = nullptr;
    if( SUCCEEDED( CompileSkyboxShader( shader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageSkyboxVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_stageSkyboxLayout );
        vsBlob->Release();
    }
    if( SUCCEEDED( CompileSkyboxShader( shader, "PS", "ps_4_0", &psBlob ) ) )
    { device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageSkyboxPS ); psBlob->Release(); }

    // Fullscreen quad at z=0.9999 (just inside far plane)
    SkyboxVertex quad[] =
    {
        { XMFLOAT3(-1,-1,0.9999f), XMFLOAT2(0,1) },
        { XMFLOAT3(-1, 1,0.9999f), XMFLOAT2(0,0) },
        { XMFLOAT3( 1, 1,0.9999f), XMFLOAT2(1,0) },
        { XMFLOAT3(-1,-1,0.9999f), XMFLOAT2(0,1) },
        { XMFLOAT3( 1, 1,0.9999f), XMFLOAT2(1,0) },
        { XMFLOAT3( 1,-1,0.9999f), XMFLOAT2(1,1) },
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(quad);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = quad;
    device->CreateBuffer( &vbd, &vbData, &g_stageSkyboxQuadVB );

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD   = D3D11_FLOAT32_MAX;
    device->CreateSamplerState( &sampler, &g_stageCubeSampler );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageSkyDSS       ) { g_stageSkyDSS->Release();       g_stageSkyDSS       = nullptr; }
    if( g_stageSkyboxCB     ) { g_stageSkyboxCB->Release();     g_stageSkyboxCB     = nullptr; }
    if( g_stageCubeSampler  ) { g_stageCubeSampler->Release();  g_stageCubeSampler  = nullptr; }
    if( g_stageSkyboxQuadVB ) { g_stageSkyboxQuadVB->Release(); g_stageSkyboxQuadVB = nullptr; }
    if( g_stageSkyboxLayout ) { g_stageSkyboxLayout->Release(); g_stageSkyboxLayout = nullptr; }
    if( g_stageSkyboxPS     ) { g_stageSkyboxPS->Release();     g_stageSkyboxPS     = nullptr; }
    if( g_stageSkyboxVS     ) { g_stageSkyboxVS->Release();     g_stageSkyboxVS     = nullptr; }
    if( g_stageCubeMapSRV   ) { g_stageCubeMapSRV->Release();   g_stageCubeMapSRV   = nullptr; }
    if( g_stageCubeMap      ) { g_stageCubeMap->Release();      g_stageCubeMap      = nullptr; }
}

inline void UpdateStageSpecificDemo( float ) {}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* ctx,
                                      ID3D11RenderTargetView*,
                                      ID3D11DepthStencilView*,
                                      const XMMATRIX& view )
{
    if( !g_stageCubeMapSRV || !g_stageCubeSampler || !g_stageSkyboxVS || !g_stageSkyboxPS ||
        !g_stageSkyboxLayout || !g_stageSkyboxQuadVB || !g_stageSkyboxCB || !g_stageSkyDSS )
        return;

    XMMATRIX invView = XMMatrixInverse( nullptr, view );

    D3D11_VIEWPORT vp = {};
    UINT vpCount = 1;
    ctx->RSGetViewports( &vpCount, &vp );
    float aspect     = ( vpCount > 0 && vp.Height > 0.0f ) ? vp.Width / vp.Height : 1.5f;
    float tanHalfFov = tanf( XM_PIDIV4 * 0.5f );

    SkyboxCB skyCB;
    skyCB.InvView   = XMMatrixTranspose( invView );
    skyCB.ProjScale = XMFLOAT4( aspect * tanHalfFov, tanHalfFov, 0.0f, 0.0f );
    ctx->UpdateSubresource( g_stageSkyboxCB, 0, nullptr, &skyCB, 0, 0 );

    // Save state
    ID3D11InputLayout*   prevIL   = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer*        prevVB   = nullptr;  UINT prevStride = 0, prevOffset = 0;
    ID3D11VertexShader*  prevVS   = nullptr;
    ID3D11PixelShader*   prevPS   = nullptr;
    ID3D11ShaderResourceView* prevSRV  = nullptr;
    ID3D11SamplerState*  prevSamp = nullptr;
    ID3D11DepthStencilState* prevDSS  = nullptr;  UINT prevRef = 0;
    ctx->IAGetInputLayout( &prevIL );
    ctx->IAGetPrimitiveTopology( &prevTopo );
    ctx->IAGetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSGetShader( &prevVS, nullptr, nullptr );
    ctx->PSGetShader( &prevPS, nullptr, nullptr );
    ctx->PSGetShaderResources( 1, 1, &prevSRV );
    ctx->PSGetSamplers( 1, 1, &prevSamp );
    ctx->OMGetDepthStencilState( &prevDSS, &prevRef );

    // Draw skybox
    ctx->VSSetShader( g_stageSkyboxVS, nullptr, 0 );
    ctx->VSSetConstantBuffers( 2, 1, &g_stageSkyboxCB );
    ctx->PSSetShader( g_stageSkyboxPS, nullptr, 0 );
    ctx->PSSetShaderResources( 1, 1, &g_stageCubeMapSRV );
    ctx->PSSetSamplers( 1, 1, &g_stageCubeSampler );
    ctx->IASetInputLayout( g_stageSkyboxLayout );
    ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    UINT stride = sizeof(SkyboxVertex), offset = 0;
    ctx->IASetVertexBuffers( 0, 1, &g_stageSkyboxQuadVB, &stride, &offset );
    ctx->OMSetDepthStencilState( g_stageSkyDSS, 0 );
    ctx->Draw( 6, 0 );

    // Restore state
    ctx->IASetInputLayout( prevIL );
    ctx->IASetPrimitiveTopology( prevTopo );
    ctx->IASetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSSetShader( prevVS, nullptr, 0 );
    ctx->PSSetShader( prevPS, nullptr, 0 );
    ctx->PSSetShaderResources( 1, 1, &prevSRV );
    ctx->PSSetSamplers( 1, 1, &prevSamp );
    ctx->OMSetDepthStencilState( prevDSS, prevRef );

    if( prevIL  ) prevIL->Release();
    if( prevVB  ) prevVB->Release();
    if( prevVS  ) prevVS->Release();
    if( prevPS  ) prevPS->Release();
    if( prevSRV ) prevSRV->Release();
    if( prevSamp ) prevSamp->Release();
    if( prevDSS ) prevDSS->Release();
}
