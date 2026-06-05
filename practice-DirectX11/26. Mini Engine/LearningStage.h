#pragma once
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <cstring>
#include <string>
#include <vector>

struct ResourceHandle { unsigned int Id = 0; };
struct ResourceRecord { ResourceHandle Handle; const char* Name = ""; };

struct ResourceManager
{
    std::vector<ResourceRecord> Meshes;
    std::vector<ResourceRecord> Materials;
};

struct ShaderManager
{
    ResourceHandle BasicLitShader   = { 1 };
    ResourceHandle SolidColorShader = { 2 };
};

struct SceneNode
{
    DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Rotation = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Scale    = DirectX::XMFLOAT3( 1, 1, 1 );
    ResourceHandle    Mesh     = { 1 };
    ResourceHandle    Material = { 1 };

    DirectX::XMMATRIX World() const
    {
        return DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) *
               DirectX::XMMatrixRotationRollPitchYaw( Rotation.x, Rotation.y, Rotation.z ) *
               DirectX::XMMatrixTranslation( Position.x, Position.y, Position.z );
    }
};

struct RenderQueueItem
{
    unsigned int      MeshId     = 0;
    unsigned int      MaterialId = 0;
    DirectX::XMMATRIX World      = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4 Color      = DirectX::XMFLOAT4( 1, 1, 1, 1 );
};

struct RenderQueueStats
{
    unsigned int SubmittedItems   = 0;
    unsigned int CulledItems      = 0;
    unsigned int MaterialSwitches = 0;
};

struct MiniEngineContext
{
    ResourceManager              Resources;
    ShaderManager                Shaders;
    std::vector<SceneNode>       SceneGraph;
    std::vector<RenderQueueItem> RenderQueue;
    RenderQueueStats             QueueStats;
};

struct QueueOverlayVertex { DirectX::XMFLOAT3 Pos; DirectX::XMFLOAT4 Color; };

inline MiniEngineContext   g_miniEngine;
inline ID3D11Buffer*       g_stageRenderQueueBuffer = nullptr;
inline ID3D11VertexShader* g_stageQueueVS           = nullptr;
inline ID3D11PixelShader*  g_stageQueuePS           = nullptr;
inline ID3D11InputLayout*  g_stageQueueLayout       = nullptr;
inline ID3D11Buffer*       g_stageQueueVB           = nullptr;

inline float g_engineCullRadius     = 11.0f;
inline bool  g_engineCullingEnabled = true;

// Fixed camera position from main.cpp's InitDevice — used for distance culling
static const DirectX::XMFLOAT3 k_EngineCamera = { 0.0f, 4.0f, -10.0f };

inline HRESULT CompileQueueOverlay( const char* src, const char* entry, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* err = nullptr;
    HRESULT hr = D3DCompile( src, strlen( src ), nullptr, nullptr, nullptr, entry, profile,
                             D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &err );
    if( FAILED( hr ) && err ) OutputDebugStringA( (const char*)err->GetBufferPointer() );
    if( err ) err->Release();
    return hr;
}

inline void PushQueueBar( QueueOverlayVertex* v, int& c,
                          float l, float t, float r, float b, DirectX::XMFLOAT4 col )
{
    v[c++] = { DirectX::XMFLOAT3( l, b, 0 ), col };
    v[c++] = { DirectX::XMFLOAT3( l, t, 0 ), col };
    v[c++] = { DirectX::XMFLOAT3( r, t, 0 ), col };
    v[c++] = { DirectX::XMFLOAT3( l, b, 0 ), col };
    v[c++] = { DirectX::XMFLOAT3( r, t, 0 ), col };
    v[c++] = { DirectX::XMFLOAT3( r, b, 0 ), col };
}

inline void RebuildMiniEngineRenderQueue()
{
    g_miniEngine.RenderQueue.clear();
    g_miniEngine.QueueStats.CulledItems = 0;

    for( size_t i = 0; i < g_miniEngine.SceneGraph.size(); ++i )
    {
        const SceneNode& node = g_miniEngine.SceneGraph[i];

        // Distance-based culling from the fixed scene camera
        if( g_engineCullingEnabled )
        {
            float dx = node.Position.x - k_EngineCamera.x;
            float dy = node.Position.y - k_EngineCamera.y;
            float dz = node.Position.z - k_EngineCamera.z;
            if( dx*dx + dy*dy + dz*dz > g_engineCullRadius * g_engineCullRadius )
            {
                ++g_miniEngine.QueueStats.CulledItems;
                continue;
            }
        }

        const DirectX::XMFLOAT4 palette[] =
        {
            { 0.65f, 0.75f, 1.00f, 1 },  // blue
            { 1.00f, 0.55f, 0.35f, 1 },  // orange
            { 0.35f, 0.90f, 0.55f, 1 },  // green
            { 0.85f, 0.45f, 1.00f, 1 },  // purple
            { 1.00f, 0.90f, 0.25f, 1 },  // yellow
            { 1.00f, 0.40f, 0.40f, 1 },  // red
        };
        g_miniEngine.RenderQueue.push_back(
            { node.Mesh.Id, node.Material.Id, node.World(), palette[i < 6 ? i : 0] } );
    }

    // Sort by material to minimise state switches — the core render-queue optimisation
    std::sort( g_miniEngine.RenderQueue.begin(), g_miniEngine.RenderQueue.end(),
               []( const RenderQueueItem& a, const RenderQueueItem& b )
               {
                   if( a.MaterialId == b.MaterialId ) return a.MeshId < b.MeshId;
                   return a.MaterialId < b.MaterialId;
               } );

    g_miniEngine.QueueStats.SubmittedItems = static_cast<unsigned int>( g_miniEngine.RenderQueue.size() );
    g_miniEngine.QueueStats.MaterialSwitches = 0;
    unsigned int prevMat = 0;
    for( const RenderQueueItem& item : g_miniEngine.RenderQueue )
    {
        if( item.MaterialId != prevMat ) { ++g_miniEngine.QueueStats.MaterialSwitches; prevMat = item.MaterialId; }
    }
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    g_miniEngine.Resources.Meshes.push_back( { { 1 }, "SharedCubeMesh" } );
    g_miniEngine.Resources.Materials.push_back( { { 1 }, "BlueMaterial" } );
    g_miniEngine.Resources.Materials.push_back( { { 2 }, "OrangeMaterial" } );
    g_miniEngine.Resources.Materials.push_back( { { 3 }, "GreenMaterial" } );
    g_miniEngine.Resources.Materials.push_back( { { 4 }, "PurpleMaterial" } );
    g_miniEngine.Resources.Materials.push_back( { { 5 }, "YellowMaterial" } );

    // Near group — always within the animated cull radius (dist ~6-11 from camera)
    SceneNode center;
    center.Position = DirectX::XMFLOAT3( 0, 0, 0 );           // dist ≈ 10.8
    g_miniEngine.SceneGraph.push_back( center );

    SceneNode left;
    left.Position = DirectX::XMFLOAT3( -2.4f, 0, 0 );          // dist ≈ 11.0
    left.Scale    = DirectX::XMFLOAT3( 0.65f, 0.65f, 0.65f );
    left.Material = { 2 };
    g_miniEngine.SceneGraph.push_back( left );

    SceneNode right;
    right.Position = DirectX::XMFLOAT3( 2.4f, 0, 0 );          // dist ≈ 11.0
    right.Scale    = DirectX::XMFLOAT3( 0.65f, 1.15f, 0.65f );
    right.Material = { 3 };
    g_miniEngine.SceneGraph.push_back( right );

    SceneNode veryNear;
    veryNear.Position = DirectX::XMFLOAT3( 0, 0, -5 );         // dist ≈ 6.4 — always rendered
    veryNear.Scale    = DirectX::XMFLOAT3( 0.8f, 0.8f, 0.8f );
    veryNear.Material = { 4 };
    g_miniEngine.SceneGraph.push_back( veryNear );

    // Far group — only submitted when cull radius peaks (~13 units needed)
    SceneNode farA;
    farA.Position = DirectX::XMFLOAT3( 3, 0, 2 );              // dist ≈ 13.0
    farA.Scale    = DirectX::XMFLOAT3( 0.7f, 1.2f, 0.7f );
    farA.Material = { 5 };
    g_miniEngine.SceneGraph.push_back( farA );

    SceneNode farB;
    farB.Position = DirectX::XMFLOAT3( -3, 0, 2 );             // dist ≈ 13.0
    farB.Scale    = DirectX::XMFLOAT3( 1.2f, 0.7f, 1.2f );
    farB.Material = { 1 };
    g_miniEngine.SceneGraph.push_back( farB );

    RebuildMiniEngineRenderQueue();

    D3D11_BUFFER_DESC desc = {};
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = 16;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &desc, nullptr, &g_stageRenderQueueBuffer );

    // Overlay: coloured bars that visualise submitted / culled / material-switch counts
    const char* overlayShader = R"(
struct VI { float3 Pos : POSITION; float4 Color : COLOR; };
struct PI { float4 Pos : SV_POSITION; float4 Color : COLOR; };
PI VS(VI i) { PI o; o.Pos = float4(i.Pos, 1); o.Color = i.Color; return o; }
float4 PS(PI i) : SV_Target { return i.Color; }
)";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if( SUCCEEDED( CompileQueueOverlay( overlayShader, "VS", "vs_4_0", &vsBlob ) ) )
    {
        device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_stageQueueVS );
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout( layout, ARRAYSIZE( layout ),
                                   vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_stageQueueLayout );
        vsBlob->Release();
    }
    if( SUCCEEDED( CompileQueueOverlay( overlayShader, "PS", "ps_4_0", &psBlob ) ) )
    { device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_stageQueuePS ); psBlob->Release(); }

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage     = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof( QueueOverlayVertex ) * 24;  // 4 rects × 6 verts
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    device->CreateBuffer( &vbd, nullptr, &g_stageQueueVB );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageQueueVB           ) { g_stageQueueVB->Release();           g_stageQueueVB           = nullptr; }
    if( g_stageQueueLayout       ) { g_stageQueueLayout->Release();       g_stageQueueLayout       = nullptr; }
    if( g_stageQueuePS           ) { g_stageQueuePS->Release();           g_stageQueuePS           = nullptr; }
    if( g_stageQueueVS           ) { g_stageQueueVS->Release();           g_stageQueueVS           = nullptr; }
    if( g_stageRenderQueueBuffer ) { g_stageRenderQueueBuffer->Release(); g_stageRenderQueueBuffer = nullptr; }
    g_miniEngine = MiniEngineContext();
}

inline void UpdateStageSpecificDemo( float t )
{
    // Animate cull radius 8 → 14 so far nodes visibly enter/leave the queue
    g_engineCullRadius = 11.0f + 3.0f * sinf( t * 0.5f );

    // C key toggles distance culling on/off
    static bool prevC = false;
    bool currC = ( GetAsyncKeyState( 'C' ) & 0x8000 ) != 0;
    if( currC && !prevC ) g_engineCullingEnabled = !g_engineCullingEnabled;
    prevC = currC;

    for( size_t i = 0; i < g_miniEngine.SceneGraph.size(); ++i )
        g_miniEngine.SceneGraph[i].Rotation.y = t * ( 0.35f + static_cast<float>( i ) * 0.2f );

    RebuildMiniEngineRenderQueue();
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( g_stageRenderQueueBuffer )
    {
        float data[4] =
        {
            static_cast<float>( g_miniEngine.QueueStats.SubmittedItems ),
            static_cast<float>( g_miniEngine.QueueStats.CulledItems ),
            static_cast<float>( g_miniEngine.QueueStats.MaterialSwitches ),
            g_engineCullRadius,
        };
        context->UpdateSubresource( g_stageRenderQueueBuffer, 0, nullptr, data, 0, 0 );
        context->VSSetConstantBuffers( 3, 1, &g_stageRenderQueueBuffer );
    }

    if( !g_stageQueueVS || !g_stageQueuePS || !g_stageQueueLayout || !g_stageQueueVB )
        return;

    float total  = static_cast<float>( g_miniEngine.SceneGraph.size() );
    float sub    = static_cast<float>( g_miniEngine.QueueStats.SubmittedItems );
    float culled = static_cast<float>( g_miniEngine.QueueStats.CulledItems );
    float matSw  = static_cast<float>( g_miniEngine.QueueStats.MaterialSwitches );
    float norm   = total > 0.0f ? total : 1.0f;

    QueueOverlayVertex verts[24] = {};
    int cursor = 0;
    // Background panel
    PushQueueBar( verts, cursor, 0.54f, -0.96f, 0.98f, -0.54f, { 0.03f, 0.04f, 0.06f, 0.90f } );
    // Submitted items — green
    PushQueueBar( verts, cursor, 0.58f, -0.66f, 0.58f + 0.36f * ( sub / norm ), -0.59f,
                  { 0.30f, 0.90f, 0.40f, 1.0f } );
    // Culled items — red
    PushQueueBar( verts, cursor, 0.58f, -0.79f, 0.58f + 0.36f * ( culled / norm ), -0.72f,
                  { 0.90f, 0.30f, 0.30f, 1.0f } );
    // Material switches — yellow (normalised by submitted)
    float matFill = sub > 0.0f ? matSw / sub : 0.0f;
    PushQueueBar( verts, cursor, 0.58f, -0.92f, 0.58f + 0.36f * matFill, -0.85f,
                  { 0.95f, 0.80f, 0.20f, 1.0f } );

    context->UpdateSubresource( g_stageQueueVB, 0, nullptr, verts, 0, 0 );

    ID3D11InputLayout*       prevIL   = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer*            prevVB   = nullptr; UINT st = 0, off = 0;
    ID3D11VertexShader*      prevVS   = nullptr;
    ID3D11PixelShader*       prevPS   = nullptr;
    context->IAGetInputLayout( &prevIL );
    context->IAGetPrimitiveTopology( &prevTopo );
    context->IAGetVertexBuffers( 0, 1, &prevVB, &st, &off );
    context->VSGetShader( &prevVS, nullptr, nullptr );
    context->PSGetShader( &prevPS, nullptr, nullptr );

    UINT stride = sizeof( QueueOverlayVertex ), offset = 0;
    context->IASetInputLayout( g_stageQueueLayout );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    context->IASetVertexBuffers( 0, 1, &g_stageQueueVB, &stride, &offset );
    context->VSSetShader( g_stageQueueVS, nullptr, 0 );
    context->PSSetShader( g_stageQueuePS, nullptr, 0 );
    context->Draw( 24, 0 );

    context->IASetInputLayout( prevIL );
    context->IASetPrimitiveTopology( prevTopo );
    context->IASetVertexBuffers( 0, 1, &prevVB, &st, &off );
    context->VSSetShader( prevVS, nullptr, 0 );
    context->PSSetShader( prevPS, nullptr, 0 );
    if( prevIL ) prevIL->Release();
    if( prevVB ) prevVB->Release();
    if( prevVS ) prevVS->Release();
    if( prevPS ) prevPS->Release();
}

template<typename TConstantBuffer>
inline void RenderMiniEngineQueue( ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, const TConstantBuffer& baseConstants )
{
    for( const RenderQueueItem& item : g_miniEngine.RenderQueue )
    {
        TConstantBuffer constants  = baseConstants;
        constants.mWorld           = DirectX::XMMatrixTranspose( item.World );
        constants.vMaterialDiffuse = item.Color;
        constants.vOutputColor     = DirectX::XMFLOAT4( 0, 0, 0, 0 );
        context->UpdateSubresource( constantBuffer, 0, nullptr, &constants, 0, 0 );
        context->DrawIndexed( 36, 0, 0 );
    }
}
