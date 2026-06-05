#pragma once
#include <DirectXMath.h>

struct InstanceData
{
    DirectX::XMFLOAT4 OffsetAndScale;
    DirectX::XMFLOAT4 Color;
};

inline ID3D11Buffer* g_stageInstanceBuffer = nullptr;
inline ID3D11Buffer* g_stageInstanceConstantBuffer = nullptr;

struct InstanceConstants
{
    DirectX::XMFLOAT4 Enabled;
};

struct InstanceDrawStats
{
    unsigned int MeshIndexCount = 36;
    unsigned int InstanceCount = 3;
    unsigned int InstanceBufferSlot = 1;
};

inline InstanceConstants g_stageInstanceConstants =
{
    DirectX::XMFLOAT4( 1, 0, 0, 0 )
};

inline InstanceConstants g_stageInstancingDisabled =
{
    DirectX::XMFLOAT4( 0, 0, 0, 0 )
};
inline InstanceDrawStats g_stageInstanceStats;

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* context )
{
    InstanceData instances[] =
    {
        { DirectX::XMFLOAT4( -2, 0, 0, 1 ), DirectX::XMFLOAT4( 1, 0.4f, 0.4f, 1 ) },
        { DirectX::XMFLOAT4( 0, 0, 0, 1 ), DirectX::XMFLOAT4( 0.4f, 1, 0.4f, 1 ) },
        { DirectX::XMFLOAT4( 2, 0, 0, 1 ), DirectX::XMFLOAT4( 0.4f, 0.6f, 1, 1 ) },
    };

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof( instances );
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = instances;
    device->CreateBuffer( &desc, &data, &g_stageInstanceBuffer );

    // Slot 1 is marked PER_INSTANCE_DATA in main.cpp's input layout.
    // Binding it once keeps the ordinary cube draw valid, and the stage draw
    // later uses the same buffer with DrawIndexedInstanced.
    UINT stride = sizeof( InstanceData );
    UINT offset = 0;
    context->IASetVertexBuffers( g_stageInstanceStats.InstanceBufferSlot, 1, &g_stageInstanceBuffer, &stride, &offset );

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof( InstanceConstants );
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SUBRESOURCE_DATA cbData = {};
    cbData.pSysMem = &g_stageInstancingDisabled;
    device->CreateBuffer( &cbDesc, &cbData, &g_stageInstanceConstantBuffer );
    context->VSSetConstantBuffers( 1, 1, &g_stageInstanceConstantBuffer );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageInstanceConstantBuffer ) g_stageInstanceConstantBuffer->Release();
    if( g_stageInstanceBuffer ) g_stageInstanceBuffer->Release();
    g_stageInstanceConstantBuffer = nullptr;
    g_stageInstanceBuffer = nullptr;
}

inline void UpdateStageSpecificDemo( float ) {}
inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( !g_stageInstanceBuffer || !g_stageInstanceConstantBuffer )
        return;

    UINT stride = sizeof( InstanceData );
    UINT offset = 0;
    context->IASetVertexBuffers( g_stageInstanceStats.InstanceBufferSlot, 1, &g_stageInstanceBuffer, &stride, &offset );
    context->UpdateSubresource( g_stageInstanceConstantBuffer, 0, nullptr, &g_stageInstanceConstants, 0, 0 );
    context->VSSetConstantBuffers( 1, 1, &g_stageInstanceConstantBuffer );

    // A single indexed cube mesh is expanded by per-instance data from vertex-buffer slot 1.
    context->DrawIndexedInstanced( g_stageInstanceStats.MeshIndexCount,
                                   g_stageInstanceStats.InstanceCount,
                                   0, 0, 0 );

    context->UpdateSubresource( g_stageInstanceConstantBuffer, 0, nullptr, &g_stageInstancingDisabled, 0, 0 );
}
