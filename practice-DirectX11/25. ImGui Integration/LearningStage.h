#pragma once
#include <DirectXMath.h>
#include <cstring>

struct DebugParameters
{
    float LightIntensity = 1.0f;
    float MaterialRoughness = 0.35f;
    bool ShowWireframe = false;
    bool ShowDebugPanel = true;
};

struct DebugPanelVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT4 Color;
};

struct DebugUiFrame
{
    float LightSlider = 0.0f;
    float RoughnessSlider = 0.0f;
    float WireframeToggle = 0.0f;
    float Visible = 1.0f;
};

inline DebugParameters g_debugParameters;
inline DebugUiFrame g_debugUiFrame;
inline ID3D11Buffer* g_stageDebugParameterBuffer = nullptr;
inline ID3D11Buffer* g_stageDebugPanelVB = nullptr;
inline ID3D11VertexShader* g_stageDebugPanelVS = nullptr;
inline ID3D11PixelShader* g_stageDebugPanelPS = nullptr;
inline ID3D11InputLayout* g_stageDebugPanelLayout = nullptr;

inline HRESULT CompileDebugPanelShader( const char* source, const char* entryPoint, const char* profile, ID3DBlob** blob )
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

inline void PushDebugRect( DebugPanelVertex* vertices, int& cursor, float left, float top, float right, float bottom,
                           const DirectX::XMFLOAT4& color )
{
    vertices[cursor++] = { DirectX::XMFLOAT3( left, bottom, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( left, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( left, bottom, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, bottom, 0.0f ), color };
}

inline void UpdateDebugPanelVertices( ID3D11DeviceContext* context )
{
    if( !g_stageDebugPanelVB )
        return;

    DebugPanelVertex vertices[24] = {};
    int cursor = 0;
    PushDebugRect( vertices, cursor, -0.96f, 0.94f, -0.48f, 0.54f, DirectX::XMFLOAT4( 0.04f, 0.05f, 0.07f, 0.92f ) );
    PushDebugRect( vertices, cursor, -0.92f, 0.84f, -0.92f + 0.36f * g_debugUiFrame.LightSlider, 0.76f,
                   DirectX::XMFLOAT4( 1.0f, 0.82f, 0.25f, 1.0f ) );
    PushDebugRect( vertices, cursor, -0.92f, 0.70f, -0.92f + 0.36f * g_debugUiFrame.RoughnessSlider, 0.62f,
                   DirectX::XMFLOAT4( 0.35f, 0.75f, 1.0f, 1.0f ) );
    PushDebugRect( vertices, cursor, -0.92f, 0.58f, g_debugUiFrame.WireframeToggle > 0.5f ? -0.56f : -0.80f, 0.55f,
                   DirectX::XMFLOAT4( 0.55f, 1.0f, 0.45f, 1.0f ) );

    context->UpdateSubresource( g_stageDebugPanelVB, 0, nullptr, vertices, 0, 0 );
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = 16;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &desc, nullptr, &g_stageDebugParameterBuffer );

    const char* shader = R"(
struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = float4(input.Pos, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return input.Color;
}
)";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if( SUCCEEDED( CompileDebugPanelShader( shader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageDebugPanelVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                   &g_stageDebugPanelLayout );
    }
    if( vsBlob ) vsBlob->Release();

    if( SUCCEEDED( CompileDebugPanelShader( shader, "PS", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageDebugPanelPS );
    if( psBlob ) psBlob->Release();

    D3D11_BUFFER_DESC vb = {};
    vb.Usage = D3D11_USAGE_DEFAULT;
    vb.ByteWidth = sizeof( DebugPanelVertex ) * 24;
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    device->CreateBuffer( &vb, nullptr, &g_stageDebugPanelVB );
    OutputDebugStringA( "ImGui integration: runtime parameters are represented by a debug panel. Replace this panel with ImGui widgets after adding the backend files.\n" );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageDebugPanelVB ) g_stageDebugPanelVB->Release();
    if( g_stageDebugPanelLayout ) g_stageDebugPanelLayout->Release();
    if( g_stageDebugPanelPS ) g_stageDebugPanelPS->Release();
    if( g_stageDebugPanelVS ) g_stageDebugPanelVS->Release();
    if( g_stageDebugParameterBuffer ) g_stageDebugParameterBuffer->Release();
    g_stageDebugPanelVB = nullptr;
    g_stageDebugPanelLayout = nullptr;
    g_stageDebugPanelPS = nullptr;
    g_stageDebugPanelVS = nullptr;
    g_stageDebugParameterBuffer = nullptr;
}

inline void UpdateStageSpecificDemo( float t )
{
    g_debugParameters.LightIntensity = 0.75f + 0.25f * sinf( t );
    g_debugParameters.MaterialRoughness = 0.35f + 0.25f * ( 0.5f + 0.5f * cosf( t * 0.7f ) );
    g_debugParameters.ShowWireframe = sinf( t * 0.5f ) > 0.0f;

    // This frame object mirrors the values that would normally be edited by ImGui widgets.
    g_debugUiFrame.LightSlider = g_debugParameters.LightIntensity;
    g_debugUiFrame.RoughnessSlider = g_debugParameters.MaterialRoughness;
    g_debugUiFrame.WireframeToggle = g_debugParameters.ShowWireframe ? 1.0f : 0.0f;
    g_debugUiFrame.Visible = g_debugParameters.ShowDebugPanel ? 1.0f : 0.0f;
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( g_stageDebugParameterBuffer )
    {
        float data[4] =
        {
            g_debugParameters.LightIntensity,
            g_debugParameters.MaterialRoughness,
            g_debugParameters.ShowWireframe ? 1.0f : 0.0f,
            g_debugParameters.ShowDebugPanel ? 1.0f : 0.0f
        };
        context->UpdateSubresource( g_stageDebugParameterBuffer, 0, nullptr, data, 0, 0 );
        context->PSSetConstantBuffers( 3, 1, &g_stageDebugParameterBuffer );
    }

    if( !g_debugParameters.ShowDebugPanel || !g_stageDebugPanelVB || !g_stageDebugPanelLayout ||
        !g_stageDebugPanelVS || !g_stageDebugPanelPS )
        return;

    ID3D11InputLayout* previousLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY previousTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* previousVB = nullptr;
    UINT previousStride = 0;
    UINT previousOffset = 0;
    ID3D11VertexShader* previousVS = nullptr;
    ID3D11PixelShader* previousPS = nullptr;
    context->IAGetInputLayout( &previousLayout );
    context->IAGetPrimitiveTopology( &previousTopology );
    context->IAGetVertexBuffers( 0, 1, &previousVB, &previousStride, &previousOffset );
    context->VSGetShader( &previousVS, nullptr, nullptr );
    context->PSGetShader( &previousPS, nullptr, nullptr );

    UpdateDebugPanelVertices( context );
    UINT stride = sizeof( DebugPanelVertex );
    UINT offset = 0;
    context->IASetInputLayout( g_stageDebugPanelLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    context->IASetVertexBuffers( 0, 1, &g_stageDebugPanelVB, &stride, &offset );
    context->VSSetShader( g_stageDebugPanelVS, nullptr, 0 );
    context->PSSetShader( g_stageDebugPanelPS, nullptr, 0 );
    context->Draw( 24, 0 );

    context->IASetInputLayout( previousLayout );
    context->IASetPrimitiveTopology( previousTopology );
    context->IASetVertexBuffers( 0, 1, &previousVB, &previousStride, &previousOffset );
    context->VSSetShader( previousVS, nullptr, 0 );
    context->PSSetShader( previousPS, nullptr, 0 );

    if( previousLayout ) previousLayout->Release();
    if( previousVB ) previousVB->Release();
    if( previousVS ) previousVS->Release();
    if( previousPS ) previousPS->Release();
}
