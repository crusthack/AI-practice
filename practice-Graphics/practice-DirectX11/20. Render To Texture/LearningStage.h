#pragma once
#include <DirectXMath.h>

inline ID3D11Texture2D* g_stageOffscreenTexture = nullptr;
inline ID3D11RenderTargetView* g_stageOffscreenRTV = nullptr;
inline ID3D11ShaderResourceView* g_stageOffscreenSRV = nullptr;
inline ID3D11VertexShader* g_stageQuadVS = nullptr;
inline ID3D11PixelShader* g_stageWritePS = nullptr;
inline ID3D11PixelShader* g_stageSamplePS = nullptr;
inline ID3D11InputLayout* g_stageQuadLayout = nullptr;
inline ID3D11Buffer* g_stageFullscreenQuadVB = nullptr;
inline ID3D11Buffer* g_stagePreviewQuadVB = nullptr;
inline ID3D11SamplerState* g_stageSampler = nullptr;

struct RenderTargetQuadVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 Tex;
};

struct RenderTexturePassInfo
{
    unsigned int TextureWidth = 512;
    unsigned int TextureHeight = 512;
    unsigned int WritePassDrawCount = 6;
    unsigned int SamplePassDrawCount = 6;
};

inline RenderTexturePassInfo g_stageRenderTexturePass;

inline HRESULT CompileStageShader( const char* source, const char* entryPoint, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile( source, strlen( source ), nullptr, nullptr, nullptr, entryPoint, profile,
                             D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &errorBlob );
    if( FAILED( hr ) && errorBlob )
        OutputDebugStringA( static_cast<const char*>( errorBlob->GetBufferPointer() ) );
    if( errorBlob )
        errorBlob->Release();
    return hr;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    D3D11_TEXTURE2D_DESC tex = {};
    tex.Width = g_stageRenderTexturePass.TextureWidth;
    tex.Height = g_stageRenderTexturePass.TextureHeight;
    tex.MipLevels = 1;
    tex.ArraySize = 1;
    tex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex.SampleDesc.Count = 1;
    tex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    device->CreateTexture2D( &tex, nullptr, &g_stageOffscreenTexture );
    if( g_stageOffscreenTexture )
    {
        device->CreateRenderTargetView( g_stageOffscreenTexture, nullptr, &g_stageOffscreenRTV );
        device->CreateShaderResourceView( g_stageOffscreenTexture, nullptr, &g_stageOffscreenSRV );
    }

    const char* quadShader = R"(
Texture2D OffscreenTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = float4(input.Pos, 1.0f);
    output.Tex = input.Tex;
    return output;
}

float4 PSWrite(PS_INPUT input) : SV_Target
{
    // First pass: write a visible pattern into the offscreen render target.
    return float4(input.Tex.x, input.Tex.y, 0.35f + input.Tex.x * 0.4f, 1.0f);
}

float4 PSSample(PS_INPUT input) : SV_Target
{
    // Second pass: sample the offscreen render target as a texture.
    return OffscreenTexture.Sample(LinearSampler, input.Tex);
}
)";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if( SUCCEEDED( CompileStageShader( quadShader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageQuadVS );

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                   &g_stageQuadLayout );
    }
    if( vsBlob )
        vsBlob->Release();

    if( SUCCEEDED( CompileStageShader( quadShader, "PSWrite", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageWritePS );
    if( psBlob )
        psBlob->Release();
    psBlob = nullptr;

    if( SUCCEEDED( CompileStageShader( quadShader, "PSSample", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageSamplePS );
    if( psBlob )
        psBlob->Release();
    RenderTargetQuadVertex fullscreenQuad[] =
    {
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -1,  1, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3(  1, -1, 0 ), DirectX::XMFLOAT2( 1, 1 ) },
    };
    RenderTargetQuadVertex previewQuad[] =
    {
        { DirectX::XMFLOAT3( -0.95f, -0.95f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.95f, -0.35f, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3( -0.35f, -0.35f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.95f, -0.95f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.35f, -0.35f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.35f, -0.95f, 0 ), DirectX::XMFLOAT2( 1, 1 ) },
    };

    D3D11_BUFFER_DESC vb = {};
    vb.Usage = D3D11_USAGE_DEFAULT;
    vb.ByteWidth = sizeof( fullscreenQuad );
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = fullscreenQuad;
    device->CreateBuffer( &vb, &vbData, &g_stageFullscreenQuadVB );

    vb.ByteWidth = sizeof( previewQuad );
    vbData.pSysMem = previewQuad;
    device->CreateBuffer( &vb, &vbData, &g_stagePreviewQuadVB );

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler.MinLOD = 0;
    sampler.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState( &sampler, &g_stageSampler );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageSampler ) g_stageSampler->Release();
    if( g_stagePreviewQuadVB ) g_stagePreviewQuadVB->Release();
    if( g_stageFullscreenQuadVB ) g_stageFullscreenQuadVB->Release();
    if( g_stageQuadLayout ) g_stageQuadLayout->Release();
    if( g_stageSamplePS ) g_stageSamplePS->Release();
    if( g_stageWritePS ) g_stageWritePS->Release();
    if( g_stageQuadVS ) g_stageQuadVS->Release();
    if( g_stageOffscreenSRV ) g_stageOffscreenSRV->Release();
    if( g_stageOffscreenRTV ) g_stageOffscreenRTV->Release();
    if( g_stageOffscreenTexture ) g_stageOffscreenTexture->Release();
    g_stageSampler = nullptr;
    g_stagePreviewQuadVB = nullptr;
    g_stageFullscreenQuadVB = nullptr;
    g_stageQuadLayout = nullptr;
    g_stageSamplePS = nullptr;
    g_stageWritePS = nullptr;
    g_stageQuadVS = nullptr;
    g_stageOffscreenSRV = nullptr;
    g_stageOffscreenRTV = nullptr;
    g_stageOffscreenTexture = nullptr;
}

inline void UpdateStageSpecificDemo( float ) {}
inline bool RenderTextureResourcesReady()
{
    return g_stageOffscreenRTV && g_stageOffscreenSRV && g_stageQuadVS && g_stageWritePS && g_stageSamplePS &&
           g_stageQuadLayout && g_stageFullscreenQuadVB && g_stagePreviewQuadVB && g_stageSampler;
}

inline void ApplyStageSpecificWritePass( ID3D11DeviceContext* context, ID3D11RenderTargetView* backBufferRTV,
                                         ID3D11DepthStencilView* depthDSV )
{
    if( !RenderTextureResourcesReady() )
        return;

    ID3D11InputLayout* previousLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY previousTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* previousVertexBuffer = nullptr;
    UINT previousStride = 0;
    UINT previousOffset = 0;
    ID3D11VertexShader* previousVS = nullptr;
    ID3D11PixelShader* previousPS = nullptr;
    ID3D11ClassInstance* previousVSClassInstances[8] = {};
    ID3D11ClassInstance* previousPSClassInstances[8] = {};
    UINT previousVSClassCount = ARRAYSIZE( previousVSClassInstances );
    UINT previousPSClassCount = ARRAYSIZE( previousPSClassInstances );
    ID3D11ShaderResourceView* previousSRV = nullptr;
    ID3D11SamplerState* previousSampler = nullptr;

    context->IAGetInputLayout( &previousLayout );
    context->IAGetPrimitiveTopology( &previousTopology );
    context->IAGetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->VSGetShader( &previousVS, previousVSClassInstances, &previousVSClassCount );
    context->PSGetShader( &previousPS, previousPSClassInstances, &previousPSClassCount );
    context->PSGetShaderResources( 0, 1, &previousSRV );
    context->PSGetSamplers( 0, 1, &previousSampler );

    const float offscreenClear[4] = { 0.15f, 0.02f, 0.02f, 1.0f };
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources( 0, 1, &nullSRV );

    // Pass 1: draw into a texture instead of the swap chain back buffer.
    context->OMSetRenderTargets( 1, &g_stageOffscreenRTV, nullptr );
    context->ClearRenderTargetView( g_stageOffscreenRTV, offscreenClear );
    context->IASetInputLayout( g_stageQuadLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    UINT stride = sizeof( RenderTargetQuadVertex );
    UINT offset = 0;
    context->IASetVertexBuffers( 0, 1, &g_stageFullscreenQuadVB, &stride, &offset );
    context->VSSetShader( g_stageQuadVS, nullptr, 0 );
    context->PSSetShader( g_stageWritePS, nullptr, 0 );
    context->Draw( g_stageRenderTexturePass.WritePassDrawCount, 0 );

    context->OMSetRenderTargets( 1, &backBufferRTV, depthDSV );
    context->IASetInputLayout( previousLayout );
    context->IASetPrimitiveTopology( previousTopology );
    context->IASetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->VSSetShader( previousVS, previousVSClassInstances, previousVSClassCount );
    context->PSSetShader( previousPS, previousPSClassInstances, previousPSClassCount );
    context->PSSetShaderResources( 0, 1, &previousSRV );
    context->PSSetSamplers( 0, 1, &previousSampler );

    if( previousLayout ) previousLayout->Release();
    if( previousVertexBuffer ) previousVertexBuffer->Release();
    if( previousVS ) previousVS->Release();
    if( previousPS ) previousPS->Release();
    if( previousSRV ) previousSRV->Release();
    if( previousSampler ) previousSampler->Release();
    for( UINT i = 0; i < previousVSClassCount; ++i )
    {
        if( previousVSClassInstances[i] ) previousVSClassInstances[i]->Release();
    }
    for( UINT i = 0; i < previousPSClassCount; ++i )
    {
        if( previousPSClassInstances[i] ) previousPSClassInstances[i]->Release();
    }
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( !RenderTextureResourcesReady() )
        return;

    ID3D11InputLayout* previousLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY previousTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* previousVertexBuffer = nullptr;
    UINT previousStride = 0;
    UINT previousOffset = 0;
    ID3D11VertexShader* previousVS = nullptr;
    ID3D11PixelShader* previousPS = nullptr;
    ID3D11ClassInstance* previousVSClassInstances[8] = {};
    ID3D11ClassInstance* previousPSClassInstances[8] = {};
    UINT previousVSClassCount = ARRAYSIZE( previousVSClassInstances );
    UINT previousPSClassCount = ARRAYSIZE( previousPSClassInstances );
    ID3D11ShaderResourceView* previousSRV = nullptr;
    ID3D11SamplerState* previousSampler = nullptr;

    context->IAGetInputLayout( &previousLayout );
    context->IAGetPrimitiveTopology( &previousTopology );
    context->IAGetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->VSGetShader( &previousVS, previousVSClassInstances, &previousVSClassCount );
    context->PSGetShader( &previousPS, previousPSClassInstances, &previousPSClassCount );
    context->PSGetShaderResources( 0, 1, &previousSRV );
    context->PSGetSamplers( 0, 1, &previousSampler );

    UINT stride = sizeof( RenderTargetQuadVertex );
    UINT offset = 0;
    context->IASetInputLayout( g_stageQuadLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    context->IASetVertexBuffers( 0, 1, &g_stagePreviewQuadVB, &stride, &offset );
    context->VSSetShader( g_stageQuadVS, nullptr, 0 );
    context->PSSetShader( g_stageSamplePS, nullptr, 0 );
    context->PSSetShaderResources( 0, 1, &g_stageOffscreenSRV );
    context->PSSetSamplers( 0, 1, &g_stageSampler );
    context->Draw( g_stageRenderTexturePass.SamplePassDrawCount, 0 );

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources( 0, 1, &nullSRV );

    context->IASetInputLayout( previousLayout );
    context->IASetPrimitiveTopology( previousTopology );
    context->IASetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->VSSetShader( previousVS, previousVSClassInstances, previousVSClassCount );
    context->PSSetShader( previousPS, previousPSClassInstances, previousPSClassCount );
    context->PSSetShaderResources( 0, 1, &previousSRV );
    context->PSSetSamplers( 0, 1, &previousSampler );

    if( previousLayout ) previousLayout->Release();
    if( previousVertexBuffer ) previousVertexBuffer->Release();
    if( previousVS ) previousVS->Release();
    if( previousPS ) previousPS->Release();
    if( previousSRV ) previousSRV->Release();
    if( previousSampler ) previousSampler->Release();
    for( UINT i = 0; i < previousVSClassCount; ++i )
    {
        if( previousVSClassInstances[i] ) previousVSClassInstances[i]->Release();
    }
    for( UINT i = 0; i < previousPSClassCount; ++i )
    {
        if( previousPSClassInstances[i] ) previousPSClassInstances[i]->Release();
    }
}
