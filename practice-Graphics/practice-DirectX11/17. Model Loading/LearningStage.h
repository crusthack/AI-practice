#pragma once
#include <DirectXMath.h>
#include <sstream>
#include <string>
#include <vector>

struct ObjVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
};

struct ObjMesh
{
    std::vector<ObjVertex> Vertices;
    std::vector<unsigned short> Indices;
};

struct ObjLoadStats
{
    unsigned int PositionCount = 0;
    unsigned int NormalCount = 0;
    unsigned int TriangleCount = 0;
    unsigned int VertexCount = 0;
    unsigned int IndexCount = 0;
};

inline ID3D11Buffer* g_stageObjVertexBuffer = nullptr;
inline ID3D11Buffer* g_stageObjIndexBuffer = nullptr;
inline unsigned int g_stageObjIndexCount = 0;
inline ObjLoadStats g_stageObjStats;

inline bool ParseObjFaceVertex( const std::string& token, int& positionIndex, int& normalIndex )
{
    positionIndex = 0;
    normalIndex = 0;

    const size_t firstSlash = token.find( '/' );
    if( firstSlash == std::string::npos )
        return false;

    const size_t secondSlash = token.find( '/', firstSlash + 1 );
    if( secondSlash == std::string::npos )
        return false;

    // Only pos//normal format is handled. The texture-coordinate index between the
    // two slashes is intentionally skipped; UV support is left for a later stage.
    positionIndex = std::stoi( token.substr( 0, firstSlash ) );
    normalIndex = std::stoi( token.substr( secondSlash + 1 ) );
    return positionIndex > 0 && normalIndex > 0;
}

inline ObjMesh LoadObjFromString( const char* text )
{
    g_stageObjStats = ObjLoadStats();
    ObjMesh mesh;
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::istringstream input( text );
    std::string tag;

    while( input >> tag )
    {
        if( tag == "v" )
        {
            DirectX::XMFLOAT3 p;
            input >> p.x >> p.y >> p.z;
            positions.push_back( p );
            g_stageObjStats.PositionCount = static_cast<unsigned int>( positions.size() );
        }
        else if( tag == "vn" )
        {
            DirectX::XMFLOAT3 n;
            input >> n.x >> n.y >> n.z;
            normals.push_back( n );
            g_stageObjStats.NormalCount = static_cast<unsigned int>( normals.size() );
        }
        else if( tag == "f" )
        {
            for( int i = 0; i < 3; ++i )
            {
                std::string faceToken;
                int p = 0;
                int n = 0;
                input >> faceToken;
                if( !ParseObjFaceVertex( faceToken, p, n ) ||
                    p > static_cast<int>( positions.size() ) ||
                    n > static_cast<int>( normals.size() ) )
                {
                    OutputDebugStringA( "OBJ face references an invalid vertex or normal\n" );
                    continue;
                }
                mesh.Vertices.push_back( { positions[p - 1], normals[n - 1] } );
                // Expand approach: every face-vertex becomes a unique entry (no deduplication).
                // Indices are sequential (0,1,2,...) so the index buffer provides no reuse here.
                // A production loader would deduplicate via a vertex hash map.
                mesh.Indices.push_back( static_cast<unsigned short>( mesh.Indices.size() ) );
            }
            ++g_stageObjStats.TriangleCount;
        }
        else
        {
            std::string rest;
            std::getline( input, rest );
        }
    }

    g_stageObjStats.VertexCount = static_cast<unsigned int>( mesh.Vertices.size() );
    g_stageObjStats.IndexCount = static_cast<unsigned int>( mesh.Indices.size() );
    return mesh;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    const char* embeddedObj =
        "# A small pyramid OBJ kept inline so the sample has no external asset dependency.\n"
        "v 0 1 0\n"
        "v -1 -1 -1\n"
        "v 1 -1 -1\n"
        "v 1 -1 1\n"
        "v -1 -1 1\n"
        "vn 0 0 -1\n"
        "vn 1 0 0\n"
        "vn 0 0 1\n"
        "vn -1 0 0\n"
        "vn 0 -1 0\n"
        "f 1//1 2//1 3//1\n"
        "f 1//2 3//2 4//2\n"
        "f 1//3 4//3 5//3\n"
        "f 1//4 5//4 2//4\n"
        "f 2//5 5//5 4//5\n"
        "f 2//5 4//5 3//5\n";
    ObjMesh mesh = LoadObjFromString( embeddedObj );
    if( mesh.Vertices.empty() )
    {
        OutputDebugStringA( "OBJ loader failed\n" );
        return;
    }

    D3D11_BUFFER_DESC vb = {};
    vb.Usage = D3D11_USAGE_DEFAULT;
    vb.ByteWidth = static_cast<UINT>( sizeof( ObjVertex ) * mesh.Vertices.size() );
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = mesh.Vertices.data();
    device->CreateBuffer( &vb, &vbData, &g_stageObjVertexBuffer );

    D3D11_BUFFER_DESC ib = {};
    ib.Usage = D3D11_USAGE_DEFAULT;
    ib.ByteWidth = static_cast<UINT>( sizeof( unsigned short ) * mesh.Indices.size() );
    ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = mesh.Indices.data();
    device->CreateBuffer( &ib, &ibData, &g_stageObjIndexBuffer );
    g_stageObjIndexCount = static_cast<unsigned int>( mesh.Indices.size() );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_stageObjIndexBuffer ) g_stageObjIndexBuffer->Release();
    if( g_stageObjVertexBuffer ) g_stageObjVertexBuffer->Release();
    g_stageObjIndexBuffer = nullptr;
    g_stageObjVertexBuffer = nullptr;
    g_stageObjIndexCount = 0;
    g_stageObjStats = ObjLoadStats();
}
inline void UpdateStageSpecificDemo( float ) {}
inline void ApplyStageSpecificRender( ID3D11DeviceContext* context, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( !g_stageObjVertexBuffer || !g_stageObjIndexBuffer )
        return;

    ID3D11Buffer* previousVertexBuffer = nullptr;
    UINT previousStride = 0;
    UINT previousOffset = 0;
    ID3D11Buffer* previousIndexBuffer = nullptr;
    DXGI_FORMAT previousIndexFormat = DXGI_FORMAT_UNKNOWN;
    UINT previousIndexOffset = 0;
    context->IAGetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->IAGetIndexBuffer( &previousIndexBuffer, &previousIndexFormat, &previousIndexOffset );

    // OBJ loading turns text data into ordinary IA resources:
    // v/vn/f records -> CPU mesh -> vertex buffer + index buffer -> DrawIndexed.
    UINT stride = sizeof( ObjVertex );
    UINT offset = 0;
    context->IASetVertexBuffers( 0, 1, &g_stageObjVertexBuffer, &stride, &offset );
    context->IASetIndexBuffer( g_stageObjIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    context->DrawIndexed( g_stageObjIndexCount, 0, 0 );

    context->IASetVertexBuffers( 0, 1, &previousVertexBuffer, &previousStride, &previousOffset );
    context->IASetIndexBuffer( previousIndexBuffer, previousIndexFormat, previousIndexOffset );
    if( previousVertexBuffer ) previousVertexBuffer->Release();
    if( previousIndexBuffer ) previousIndexBuffer->Release();
}
