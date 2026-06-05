#pragma once
#include <DirectXMath.h>
#include <cstring>
#include <vector>

struct ViewerObject
{
    DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Rotation = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3( 1, 1, 1 );
    DirectX::XMFLOAT4 MaterialColor = DirectX::XMFLOAT4( 0.75f, 0.75f, 0.85f, 1.0f );

    DirectX::XMMATRIX World() const
    {
        return DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) *
               DirectX::XMMatrixRotationRollPitchYaw( Rotation.x, Rotation.y, Rotation.z ) *
               DirectX::XMMatrixTranslation( Position.x, Position.y, Position.z );
    }
};

struct ViewerState
{
    DirectX::XMFLOAT3 CameraPosition = DirectX::XMFLOAT3( 0, 2, -6 );
    float Exposure = 1.0f;
    bool ShowDebugUi = true;
    float CameraOrbit = 0.0f;
    std::vector<ViewerObject> Objects;
};

inline ViewerState g_viewerState;
inline ID3D11Buffer* g_stageViewerBuffer = nullptr;
inline ID3D11Buffer* g_stageViewerDebugVB = nullptr;
inline ID3D11VertexShader* g_stageViewerDebugVS = nullptr;
inline ID3D11PixelShader* g_stageViewerDebugPS = nullptr;
inline ID3D11InputLayout* g_stageViewerDebugLayout = nullptr;

struct ViewerDebugVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT4 Color;
};

inline HRESULT CompileViewerDebugShader( const char* source, const char* entryPoint, const char* profile, ID3DBlob** blob )
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

inline void PushViewerDebugRect( ViewerDebugVertex* vertices, int& cursor, float left, float top, float right, float bottom,
                                 const DirectX::XMFLOAT4& color )
{
    vertices[cursor++] = { DirectX::XMFLOAT3( left, bottom, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( left, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( left, bottom, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, top, 0.0f ), color };
    vertices[cursor++] = { DirectX::XMFLOAT3( right, bottom, 0.0f ), color };
}

inline void UpdateViewerDebugPanel( ID3D11DeviceContext* context )
{
    if( !g_stageViewerDebugVB )
        return;

    ViewerDebugVertex vertices[24] = {};
    int cursor = 0;
    PushViewerDebugRect( vertices, cursor, -0.98f, 0.96f, -0.50f, 0.54f, DirectX::XMFLOAT4( 0.03f, 0.04f, 0.06f, 0.92f ) );
    PushViewerDebugRect( vertices, cursor, -0.94f, 0.86f, -0.94f + 0.36f * g_viewerState.Exposure, 0.78f,
                         DirectX::XMFLOAT4( 0.95f, 0.82f, 0.30f, 1.0f ) );
    PushViewerDebugRect( vertices, cursor, -0.94f, 0.72f, -0.94f + 0.08f * static_cast<float>( g_viewerState.Objects.size() ), 0.64f,
                         DirectX::XMFLOAT4( 0.35f, 0.85f, 1.0f, 1.0f ) );
    PushViewerDebugRect( vertices, cursor, -0.94f, 0.59f, -0.94f + 0.36f * ( 0.5f + 0.5f * sinf( g_viewerState.CameraOrbit ) ), 0.56f,
                         DirectX::XMFLOAT4( 0.45f, 1.0f, 0.55f, 1.0f ) );

    context->UpdateSubresource( g_stageViewerDebugVB, 0, nullptr, vertices, 0, 0 );
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    ViewerObject hero;
    hero.Position = DirectX::XMFLOAT3( 0, 0, 0 );
    hero.MaterialColor = DirectX::XMFLOAT4( 0.75f, 0.78f, 1.0f, 1.0f );
    g_viewerState.Objects.push_back( hero );

    ViewerObject left;
    left.Position = DirectX::XMFLOAT3( -2.2f, -0.15f, 0.4f );
    left.Scale = DirectX::XMFLOAT3( 0.5f, 0.5f, 0.5f );
    left.MaterialColor = DirectX::XMFLOAT4( 1.0f, 0.65f, 0.35f, 1.0f );
    g_viewerState.Objects.push_back( left );

    ViewerObject right;
    right.Position = DirectX::XMFLOAT3( 2.2f, 0.2f, -0.2f );
    right.Scale = DirectX::XMFLOAT3( 0.55f, 1.0f, 0.55f );
    right.MaterialColor = DirectX::XMFLOAT4( 0.35f, 0.9f, 0.65f, 1.0f );
    g_viewerState.Objects.push_back( right );

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = 16;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &desc, nullptr, &g_stageViewerBuffer );

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
    if( SUCCEEDED( CompileViewerDebugShader( shader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageViewerDebugVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                   &g_stageViewerDebugLayout );
    }
    if( vsBlob ) vsBlob->Release();

    if( SUCCEEDED( CompileViewerDebugShader( shader, "PS", "ps_4_0", &psBlob ) ) )
        device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageViewerDebugPS );
    if( psBlob ) psBlob->Release();

    D3D11_BUFFER_DESC debugVB = {};
    debugVB.Usage = D3D11_USAGE_DEFAULT;
    debugVB.ByteWidth = sizeof( ViewerDebugVertex ) * 24;
    debugVB.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    device->CreateBuffer( &debugVB, nullptr, &g_stageViewerDebugVB );
    OutputDebugStringA( "Final viewer: camera movement, model draw list, lighting, material colors, and debug parameters are active.\n" );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageViewerDebugVB ) g_stageViewerDebugVB->Release();
    if( g_stageViewerDebugLayout ) g_stageViewerDebugLayout->Release();
    if( g_stageViewerDebugPS ) g_stageViewerDebugPS->Release();
    if( g_stageViewerDebugVS ) g_stageViewerDebugVS->Release();
    if( g_stageViewerBuffer ) g_stageViewerBuffer->Release();
    g_stageViewerDebugVB = nullptr;
    g_stageViewerDebugLayout = nullptr;
    g_stageViewerDebugPS = nullptr;
    g_stageViewerDebugVS = nullptr;
    g_stageViewerBuffer = nullptr;
    g_viewerState = ViewerState();
}

inline void UpdateStageSpecificDemo( float t )
{
    g_viewerState.Exposure = 0.9f + 0.1f * cosf( t );

    // Arrow keys orbit manually; otherwise auto-orbit slowly
    if( GetAsyncKeyState( VK_LEFT  ) & 0x8000 )       g_viewerState.CameraOrbit -= 0.025f;
    else if( GetAsyncKeyState( VK_RIGHT ) & 0x8000 )  g_viewerState.CameraOrbit += 0.025f;
    else                                               g_viewerState.CameraOrbit += 0.004f;

    for( size_t i = 0; i < g_viewerState.Objects.size(); ++i )
        g_viewerState.Objects[i].Rotation.y = t * ( 0.25f + static_cast<float>( i ) * 0.15f );
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( g_stageViewerBuffer )
    {
        float data[4] =
        {
            g_viewerState.Exposure,
            g_viewerState.ShowDebugUi ? 1.0f : 0.0f,
            g_viewerState.CameraOrbit,
            static_cast<float>( g_viewerState.Objects.size() )
        };
        context->UpdateSubresource( g_stageViewerBuffer, 0, nullptr, data, 0, 0 );
        context->PSSetConstantBuffers( 3, 1, &g_stageViewerBuffer );
    }

    if( !g_viewerState.ShowDebugUi || !g_stageViewerDebugVB || !g_stageViewerDebugLayout ||
        !g_stageViewerDebugVS || !g_stageViewerDebugPS )
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

    UpdateViewerDebugPanel( context );
    UINT stride = sizeof( ViewerDebugVertex );
    UINT offset = 0;
    context->IASetInputLayout( g_stageViewerDebugLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    context->IASetVertexBuffers( 0, 1, &g_stageViewerDebugVB, &stride, &offset );
    context->VSSetShader( g_stageViewerDebugVS, nullptr, 0 );
    context->PSSetShader( g_stageViewerDebugPS, nullptr, 0 );
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

template<typename TConstantBuffer>
inline void RenderFinalViewerObjects( ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, const TConstantBuffer& baseConstants )
{
    for( const ViewerObject& object : g_viewerState.Objects )
    {
        TConstantBuffer constants = baseConstants;
        constants.mWorld = DirectX::XMMatrixTranspose( object.World() );
        constants.vMaterialDiffuse = object.MaterialColor;
        constants.vOutputColor = DirectX::XMFLOAT4( 0, 0, 0, 0 );
        context->UpdateSubresource( constantBuffer, 0, nullptr, &constants, 0, 0 );
        context->DrawIndexed( 36, 0, 0 );
    }
}
