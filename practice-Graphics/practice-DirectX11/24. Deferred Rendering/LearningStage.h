#pragma once
#include <DirectXMath.h>
#include <cstring>

struct GBufferTarget
{
    ID3D11Texture2D* Texture = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;
    ID3D11ShaderResourceView* SRV = nullptr;
};

struct DeferredQuadVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 Uv;
};

inline GBufferTarget g_stageGBuffer[3];
inline ID3D11VertexShader* g_stageDeferredVS = nullptr;
inline ID3D11PixelShader* g_stageLightingPS = nullptr;
inline ID3D11PixelShader* g_stageGBufferPreviewPS = nullptr;
inline ID3D11InputLayout* g_stageDeferredLayout = nullptr;
inline ID3D11Buffer* g_stageDeferredQuadVB = nullptr;
inline ID3D11Buffer* g_stageGBufferPreviewVB = nullptr;
inline ID3D11SamplerState* g_stageDeferredSampler = nullptr;

inline HRESULT CompileDeferredShader( const char* source, const char* entryPoint, const char* profile, ID3DBlob** blob )
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

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext*, UINT width, UINT height )
{
    // G-Buffer formats: Albedo (R8G8B8A8), Normal (R16G16B16A16_FLOAT for precision),
    // Depth (R8G8B8A8 — 8-bit is sufficient for this demo but a production deferred
    // renderer would use R16F or R32F for more depth range and precision).
    DXGI_FORMAT formats[3] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    for( int i = 0; i < 3; ++i )
    {
        D3D11_TEXTURE2D_DESC tex = {};
        tex.Width = width;
        tex.Height = height;
        tex.MipLevels = 1;
        tex.ArraySize = 1;
        tex.Format = formats[i];
        tex.SampleDesc.Count = 1;
        tex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        device->CreateTexture2D( &tex, nullptr, &g_stageGBuffer[i].Texture );
        if( g_stageGBuffer[i].Texture )
        {
            device->CreateRenderTargetView( g_stageGBuffer[i].Texture, nullptr, &g_stageGBuffer[i].RTV );
            device->CreateShaderResourceView( g_stageGBuffer[i].Texture, nullptr, &g_stageGBuffer[i].SRV );
        }
    }

    const char* shader = R"(
Texture2D GAlbedo : register(t0);
Texture2D GNormal : register(t1);
Texture2D GDepth : register(t2);
SamplerState LinearSampler : register(s0);

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Uv : TEXCOORD0;
};

struct GBUFFER_OUTPUT
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Depth : SV_Target2;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = float4(input.Pos, 1.0f);
    output.Uv = input.Uv;
    return output;
}

float4 PSLighting(PS_INPUT input) : SV_Target
{
    float3 albedo = GAlbedo.Sample(LinearSampler, input.Uv).rgb;
    float3 normal = GNormal.Sample(LinearSampler, input.Uv).rgb * 2.0f - 1.0f;
    float depth = GDepth.Sample(LinearSampler, input.Uv).r;
    if (depth <= 0.001f)
        return float4(0.02f, 0.025f, 0.035f, 1.0f);
    float3 lightDir0 = normalize(float3(-0.577f, 0.577f, -0.577f));
    float3 lightDir1 = normalize(float3(0.35f, 0.2f, -0.9f));
    float diffuse0 = saturate(dot(normalize(normal), lightDir0));
    float diffuse1 = saturate(dot(normalize(normal), lightDir1));
    float ambient = 0.14f;
    float3 lit = albedo * (ambient + diffuse0 * 0.72f + diffuse1 * float3(0.35f, 0.08f, 0.08f));
    float depthFade = saturate(1.0f - depth * 1.4f);
    return float4(lit + depthFade * 0.04f, 1.0f);
}

float4 PSGBufferPreview(PS_INPUT input) : SV_Target
{
    float2 uv = input.Uv;
    if (uv.x < 0.3333f)
    {
        uv.x = uv.x * 3.0f;
        return float4(GAlbedo.Sample(LinearSampler, uv).rgb, 1.0f);
    }
    if (uv.x < 0.6666f)
    {
        uv.x = (uv.x - 0.3333f) * 3.0f;
        return float4(GNormal.Sample(LinearSampler, uv).rgb, 1.0f);
    }
    uv.x = (uv.x - 0.6666f) * 3.0f;
    float depth = GDepth.Sample(LinearSampler, uv).r;
    return float4(depth, depth, depth, 1.0f);
}
)";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if( SUCCEEDED( CompileDeferredShader( shader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageDeferredVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                   &g_stageDeferredLayout );
    }
    if( vsBlob ) vsBlob->Release();

    if( SUCCEEDED( CompileDeferredShader( shader, "PSLighting", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageLightingPS );
    if( psBlob ) psBlob->Release();
    psBlob = nullptr;

    if( SUCCEEDED( CompileDeferredShader( shader, "PSGBufferPreview", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageGBufferPreviewPS );
    if( psBlob ) psBlob->Release();

    DeferredQuadVertex quad[] =
    {
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -1,  1, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -1, -1, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3(  1,  1, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3(  1, -1, 0 ), DirectX::XMFLOAT2( 1, 1 ) },
    };
    D3D11_BUFFER_DESC vb = {};
    vb.Usage = D3D11_USAGE_DEFAULT;
    vb.ByteWidth = sizeof( quad );
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = quad;
    device->CreateBuffer( &vb, &vbData, &g_stageDeferredQuadVB );

    DeferredQuadVertex previewQuad[] =
    {
        { DirectX::XMFLOAT3( -0.98f, -0.96f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.98f, -0.58f, 0 ), DirectX::XMFLOAT2( 0, 0 ) },
        { DirectX::XMFLOAT3( -0.20f, -0.58f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.98f, -0.96f, 0 ), DirectX::XMFLOAT2( 0, 1 ) },
        { DirectX::XMFLOAT3( -0.20f, -0.58f, 0 ), DirectX::XMFLOAT2( 1, 0 ) },
        { DirectX::XMFLOAT3( -0.20f, -0.96f, 0 ), DirectX::XMFLOAT2( 1, 1 ) },
    };
    vb.ByteWidth = sizeof( previewQuad );
    vbData.pSysMem = previewQuad;
    device->CreateBuffer( &vb, &vbData, &g_stageGBufferPreviewVB );

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState( &sampler, &g_stageDeferredSampler );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageDeferredSampler ) g_stageDeferredSampler->Release();
    if( g_stageGBufferPreviewVB ) g_stageGBufferPreviewVB->Release();
    if( g_stageDeferredQuadVB ) g_stageDeferredQuadVB->Release();
    if( g_stageDeferredLayout ) g_stageDeferredLayout->Release();
    if( g_stageGBufferPreviewPS ) g_stageGBufferPreviewPS->Release();
    if( g_stageLightingPS ) g_stageLightingPS->Release();
    if( g_stageDeferredVS ) g_stageDeferredVS->Release();
    for( int i = 0; i < 3; ++i )
    {
        if( g_stageGBuffer[i].SRV ) g_stageGBuffer[i].SRV->Release();
        if( g_stageGBuffer[i].RTV ) g_stageGBuffer[i].RTV->Release();
        if( g_stageGBuffer[i].Texture ) g_stageGBuffer[i].Texture->Release();
        g_stageGBuffer[i] = GBufferTarget();
    }
    g_stageDeferredSampler = nullptr;
    g_stageGBufferPreviewVB = nullptr;
    g_stageDeferredQuadVB = nullptr;
    g_stageDeferredLayout = nullptr;
    g_stageGBufferPreviewPS = nullptr;
    g_stageLightingPS = nullptr;
    g_stageDeferredVS = nullptr;
}

inline void UpdateStageSpecificDemo( float ) {}
inline bool StageGBufferResourcesReady()
{
    return g_stageGBuffer[0].RTV && g_stageGBuffer[1].RTV && g_stageGBuffer[2].RTV &&
           g_stageGBuffer[0].SRV && g_stageGBuffer[1].SRV && g_stageGBuffer[2].SRV;
}

inline void ApplyStageSpecificBeginGBuffer( ID3D11DeviceContext* context, ID3D11DepthStencilView* depthDSV )
{
    ID3D11RenderTargetView* targets[3] =
    {
        g_stageGBuffer[0].RTV,
        g_stageGBuffer[1].RTV,
        g_stageGBuffer[2].RTV,
    };
    if( !StageGBufferResourcesReady() )
        return;

    ID3D11ShaderResourceView* nullSRVs[3] = {};
    context->PSSetShaderResources( 0, 3, nullSRVs );

    const float clearAlbedo[4] = { 0.02f, 0.025f, 0.035f, 1.0f };
    const float clearNormal[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
    const float clearDepth[4] = { 0, 0, 0, 1 };
    context->OMSetRenderTargets( 3, targets, depthDSV );
    context->ClearRenderTargetView( targets[0], clearAlbedo );
    context->ClearRenderTargetView( targets[1], clearNormal );
    context->ClearRenderTargetView( targets[2], clearDepth );
    if( depthDSV )
        context->ClearDepthStencilView( depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView* backBufferRTV, ID3D11DepthStencilView* depthDSV )
{
    if( !StageGBufferResourcesReady() || !g_stageDeferredVS || !g_stageLightingPS ||
        !g_stageGBufferPreviewPS || !g_stageDeferredLayout || !g_stageDeferredQuadVB ||
        !g_stageGBufferPreviewVB || !g_stageDeferredSampler )
        return;

    ID3D11InputLayout* previousLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY previousTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* previousVB = nullptr;
    UINT previousStride = 0;
    UINT previousOffset = 0;
    ID3D11VertexShader* previousVS = nullptr;
    ID3D11PixelShader* previousPS = nullptr;
    ID3D11ShaderResourceView* previousSRVs[3] = {};
    ID3D11SamplerState* previousSampler = nullptr;
    context->IAGetInputLayout( &previousLayout );
    context->IAGetPrimitiveTopology( &previousTopology );
    context->IAGetVertexBuffers( 0, 1, &previousVB, &previousStride, &previousOffset );
    context->VSGetShader( &previousVS, nullptr, nullptr );
    context->PSGetShader( &previousPS, nullptr, nullptr );
    context->PSGetShaderResources( 0, 3, previousSRVs );
    context->PSGetSamplers( 0, 1, &previousSampler );

    ID3D11ShaderResourceView* nullSRVs[3] = {};
    context->PSSetShaderResources( 0, 3, nullSRVs );

    context->IASetInputLayout( g_stageDeferredLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    UINT stride = sizeof( DeferredQuadVertex );
    UINT offset = 0;
    context->IASetVertexBuffers( 0, 1, &g_stageDeferredQuadVB, &stride, &offset );
    context->VSSetShader( g_stageDeferredVS, nullptr, 0 );
    context->OMSetRenderTargets( 1, &backBufferRTV, nullptr );
    ID3D11ShaderResourceView* srvs[3] =
    {
        g_stageGBuffer[0].SRV,
        g_stageGBuffer[1].SRV,
        g_stageGBuffer[2].SRV,
    };
    context->PSSetShaderResources( 0, 3, srvs );
    context->PSSetSamplers( 0, 1, &g_stageDeferredSampler );
    context->PSSetShader( g_stageLightingPS, nullptr, 0 );
    context->Draw( 6, 0 );

    // GBuffer preview: albedo, normal, and depth are shown in a small strip.
    context->IASetVertexBuffers( 0, 1, &g_stageGBufferPreviewVB, &stride, &offset );
    context->PSSetShader( g_stageGBufferPreviewPS, nullptr, 0 );
    context->Draw( 6, 0 );
    context->PSSetShaderResources( 0, 3, nullSRVs );

    context->OMSetRenderTargets( 1, &backBufferRTV, depthDSV );
    context->IASetInputLayout( previousLayout );
    context->IASetPrimitiveTopology( previousTopology );
    context->IASetVertexBuffers( 0, 1, &previousVB, &previousStride, &previousOffset );
    context->VSSetShader( previousVS, nullptr, 0 );
    context->PSSetShader( previousPS, nullptr, 0 );
    context->PSSetShaderResources( 0, 3, previousSRVs );
    context->PSSetSamplers( 0, 1, &previousSampler );

    if( previousLayout ) previousLayout->Release();
    if( previousVB ) previousVB->Release();
    if( previousVS ) previousVS->Release();
    if( previousPS ) previousPS->Release();
    for( int i = 0; i < 3; ++i )
    {
        if( previousSRVs[i] ) previousSRVs[i]->Release();
    }
    if( previousSampler ) previousSampler->Release();
}
