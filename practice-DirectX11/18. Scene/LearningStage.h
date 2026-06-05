#pragma once
#include <DirectXMath.h>
#include <vector>

struct Transform
{
    DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Rotation = DirectX::XMFLOAT3( 0, 0, 0 );
    DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3( 1, 1, 1 );

    DirectX::XMMATRIX World() const
    {
        return DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) *
               DirectX::XMMatrixRotationRollPitchYaw( Rotation.x, Rotation.y, Rotation.z ) *
               DirectX::XMMatrixTranslation( Position.x, Position.y, Position.z );
    }
};

struct MeshRenderer
{
    unsigned int IndexCount = 36;
    DirectX::XMFLOAT4 MaterialDiffuse = DirectX::XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
};

struct Entity
{
    Transform LocalTransform;
    MeshRenderer Renderer;
};

inline std::vector<Entity> g_sceneEntities;
inline ID3D11Buffer* g_stageSceneBuffer = nullptr;

struct SceneConstants
{
    DirectX::XMFLOAT4 EntityCount_Time;
};

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    // Scene practice: Entity owns a Transform and a MeshRenderer.
    // The renderer still uses the shared cube mesh from main.cpp, so this stage
    // focuses on data organization before introducing a full render queue later.
    Entity centerCube;
    centerCube.LocalTransform.Position = DirectX::XMFLOAT3( 0, 0, 0 );
    centerCube.Renderer.MaterialDiffuse = DirectX::XMFLOAT4( 0.7f, 0.7f, 0.9f, 1.0f );
    g_sceneEntities.push_back( centerCube );

    Entity leftCube;
    leftCube.LocalTransform.Position = DirectX::XMFLOAT3( -2.5f, 0, 0 );
    leftCube.LocalTransform.Scale = DirectX::XMFLOAT3( 0.6f, 0.6f, 0.6f );
    leftCube.Renderer.MaterialDiffuse = DirectX::XMFLOAT4( 0.9f, 0.45f, 0.35f, 1.0f );
    g_sceneEntities.push_back( leftCube );

    Entity rightCube;
    rightCube.LocalTransform.Position = DirectX::XMFLOAT3( 2.5f, 0, 0 );
    rightCube.LocalTransform.Scale = DirectX::XMFLOAT3( 0.6f, 1.2f, 0.6f );
    rightCube.Renderer.MaterialDiffuse = DirectX::XMFLOAT4( 0.35f, 0.8f, 0.5f, 1.0f );
    g_sceneEntities.push_back( rightCube );

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof( SceneConstants );
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &desc, nullptr, &g_stageSceneBuffer );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageSceneBuffer ) g_stageSceneBuffer->Release();
    g_stageSceneBuffer = nullptr;
    g_sceneEntities.clear();
}

inline void UpdateStageSpecificDemo( float t )
{
    for( size_t i = 0; i < g_sceneEntities.size(); ++i )
    {
        g_sceneEntities[i].LocalTransform.Rotation.y = t * ( 0.5f + static_cast<float>( i ) * 0.25f );
    }
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( !g_stageSceneBuffer )
        return;

    SceneConstants constants = {};
    constants.EntityCount_Time = DirectX::XMFLOAT4( static_cast<float>( g_sceneEntities.size() ), 0, 0, 0 );
    context->UpdateSubresource( g_stageSceneBuffer, 0, nullptr, &constants, 0, 0 );
    // b3 is not declared in the scene VS shader; this shows the data organization pattern
    // for passing scene-level constants without a full render queue yet.
    context->VSSetConstantBuffers( 3, 1, &g_stageSceneBuffer );
}

template<typename TConstantBuffer>
inline void RenderSceneEntities( ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, const TConstantBuffer& baseConstants )
{
    for( const Entity& entity : g_sceneEntities )
    {
        TConstantBuffer constants = baseConstants;
        constants.mWorld = DirectX::XMMatrixTranspose( entity.LocalTransform.World() );
        constants.vMaterialDiffuse = entity.Renderer.MaterialDiffuse;
        constants.vOutputColor = DirectX::XMFLOAT4( 0, 0, 0, 0 );

        // MeshRenderer decides what mesh range to draw. This sample keeps one
        // shared cube mesh and varies Transform/Material per entity.
        context->UpdateSubresource( constantBuffer, 0, nullptr, &constants, 0, 0 );
        context->DrawIndexed( entity.Renderer.IndexCount, 0, 0 );
    }
}
