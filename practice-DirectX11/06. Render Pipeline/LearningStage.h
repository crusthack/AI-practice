#pragma once
#include <d3dcompiler.h>
#include <cstring>

enum PipelineStageId
{
    PipelineStageIA = 0,
    PipelineStageVS,
    PipelineStageRS,
    PipelineStagePS,
    PipelineStageOM,
    PipelineStageDraw,
    PipelineStageCount
};

struct PipelineStageInfo
{
    const char* Label;
    float R;
    float G;
    float B;
};

struct PipelineTraceState
{
    bool StageTouched[PipelineStageCount] = {};
    unsigned int DrawIndexedCalls = 0;
    unsigned int LastIndexCount = 0;
    float Time = 0.0f;
};

struct PipelineSnapshot
{
    bool HasInputLayout = false;
    bool HasVertexBuffer = false;
    bool HasIndexBuffer = false;
    bool UsesTriangleList = false;
    bool HasVertexShader = false;
    bool HasVSConstantBuffer = false;
    bool HasViewport = false;
    bool HasPixelShader = false;
    bool HasPSConstantBuffer = false;
    bool HasRenderTarget = false;
    bool HasDepthTarget = false;
};

struct OverlayCBData
{
    float X;
    float Y;
    float W;
    float H;
    float R;
    float G;
    float B;
    float A;
};

static const char* g_pipelineOverlayShader = R"(
cbuffer OverlayCB : register(b1)
{
    float4 Rect;
    float4 Color;
}

float4 VS(uint id : SV_VertexID) : SV_POSITION
{
    float l = Rect.x;
    float t = Rect.y;
    float r = Rect.x + Rect.z;
    float b = Rect.y - Rect.w;
    float2 p[4] = { { l, t }, { r, t }, { l, b }, { r, b } };
    return float4(p[id], 0.0f, 1.0f);
}

float4 PS() : SV_Target
{
    return Color;
}
)";

inline PipelineStageInfo g_pipelineStageInfo[PipelineStageCount] =
{
    { "IA",   0.22f, 0.52f, 0.86f },
    { "VS",   0.20f, 0.72f, 0.45f },
    { "RS",   0.90f, 0.64f, 0.14f },
    { "PS",   0.88f, 0.34f, 0.30f },
    { "OM",   0.60f, 0.44f, 0.88f },
    { "DRAW", 0.70f, 0.72f, 0.78f },
};

inline ID3D11VertexShader*      g_pipelineOverlayVS = nullptr;
inline ID3D11PixelShader*       g_pipelineOverlayPS = nullptr;
inline ID3D11Buffer*            g_pipelineOverlayCB = nullptr;
inline ID3D11DepthStencilState* g_pipelineNoDepth = nullptr;
inline ID3D11RasterizerState*   g_pipelineNoCull = nullptr;
inline PipelineTraceState       g_pipelineTrace;

inline HRESULT CompilePipelineOverlayShader( const char* entryPoint, const char* profile, ID3DBlob** blob )
{
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile( g_pipelineOverlayShader, std::strlen( g_pipelineOverlayShader ), nullptr, nullptr, nullptr,
                             entryPoint, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &errorBlob );
    if( FAILED( hr ) && errorBlob )
        OutputDebugStringA( static_cast<const char*>( errorBlob->GetBufferPointer() ) );
    if( errorBlob )
        errorBlob->Release();
    return hr;
}

inline void ApplyStageSpecificSetup( ID3D11Device* device, ID3D11DeviceContext* )
{
    ID3DBlob* blob = nullptr;
    if( SUCCEEDED( CompilePipelineOverlayShader( "VS", "vs_4_0", &blob ) ) )
    {
        device->CreateVertexShader( blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &g_pipelineOverlayVS );
        blob->Release();
    }
    blob = nullptr;
    if( SUCCEEDED( CompilePipelineOverlayShader( "PS", "ps_4_0", &blob ) ) )
    {
        device->CreatePixelShader( blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &g_pipelineOverlayPS );
        blob->Release();
    }

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof( OverlayCBData );
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer( &cbDesc, nullptr, &g_pipelineOverlayCB );

    D3D11_DEPTH_STENCIL_DESC dss = {};
    dss.DepthEnable = FALSE;
    dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dss.DepthFunc = D3D11_COMPARISON_ALWAYS;
    device->CreateDepthStencilState( &dss, &g_pipelineNoDepth );

    D3D11_RASTERIZER_DESC rs = {};
    rs.FillMode = D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_NONE;
    rs.DepthClipEnable = TRUE;
    device->CreateRasterizerState( &rs, &g_pipelineNoCull );
}

inline void ApplyStageSpecificCleanup()
{
    if( g_pipelineNoCull ) g_pipelineNoCull->Release();
    if( g_pipelineNoDepth ) g_pipelineNoDepth->Release();
    if( g_pipelineOverlayCB ) g_pipelineOverlayCB->Release();
    if( g_pipelineOverlayPS ) g_pipelineOverlayPS->Release();
    if( g_pipelineOverlayVS ) g_pipelineOverlayVS->Release();
    g_pipelineNoCull = nullptr;
    g_pipelineNoDepth = nullptr;
    g_pipelineOverlayCB = nullptr;
    g_pipelineOverlayPS = nullptr;
    g_pipelineOverlayVS = nullptr;
}

inline void UpdateStageSpecificDemo( float t )
{
    g_pipelineTrace.Time = t;
    for( int i = 0; i < PipelineStageCount; ++i )
        g_pipelineTrace.StageTouched[i] = false;
    g_pipelineTrace.DrawIndexedCalls = 0;
    g_pipelineTrace.LastIndexCount = 0;
}

inline void MarkPipelineStage( PipelineStageId stage )
{
    if( stage >= 0 && stage < PipelineStageCount )
        g_pipelineTrace.StageTouched[stage] = true;
}

inline void RecordPipelineDrawIndexed( unsigned int indexCount )
{
    MarkPipelineStage( PipelineStageDraw );
    ++g_pipelineTrace.DrawIndexedCalls;
    g_pipelineTrace.LastIndexCount = indexCount;
}

inline void DrawOverlayRect( ID3D11DeviceContext* ctx, float x, float y, float w, float h,
                             float r, float g, float b, float a )
{
    OverlayCBData cb = { x, y, w, h, r, g, b, a };
    ctx->UpdateSubresource( g_pipelineOverlayCB, 0, nullptr, &cb, 0, 0 );
    ctx->VSSetConstantBuffers( 1, 1, &g_pipelineOverlayCB );
    ctx->PSSetConstantBuffers( 1, 1, &g_pipelineOverlayCB );
    ctx->Draw( 4, 0 );
}

inline const char* PipelineGlyphRow( char c, int row )
{
    static const char* blank[7] = { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
    static const char* glyphA[7] = { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
    static const char* glyphD[7] = { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
    static const char* glyphI[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
    static const char* glyphM[7] = { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
    static const char* glyphO[7] = { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
    static const char* glyphP[7] = { "11110", "10001", "10001", "11110", "10000", "10000", "10000" };
    static const char* glyphR[7] = { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
    static const char* glyphS[7] = { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
    static const char* glyphV[7] = { "10001", "10001", "10001", "10001", "10001", "01010", "00100" };
    static const char* glyphW[7] = { "10001", "10001", "10001", "10101", "10101", "10101", "01010" };
    const char** glyph = blank;
    switch( c )
    {
    case 'A': glyph = glyphA; break;
    case 'D': glyph = glyphD; break;
    case 'I': glyph = glyphI; break;
    case 'M': glyph = glyphM; break;
    case 'O': glyph = glyphO; break;
    case 'P': glyph = glyphP; break;
    case 'R': glyph = glyphR; break;
    case 'S': glyph = glyphS; break;
    case 'V': glyph = glyphV; break;
    case 'W': glyph = glyphW; break;
    default: break;
    }
    return glyph[row];
}

inline void DrawPipelineText( ID3D11DeviceContext* ctx, const char* text, float x, float y, float s,
                              float r, float g, float b, float a )
{
    float cursor = x;
    for( const char* p = text; *p; ++p )
    {
        if( *p == ' ' )
        {
            cursor += s * 4.0f;
            continue;
        }
        for( int row = 0; row < 7; ++row )
        {
            const char* bits = PipelineGlyphRow( *p, row );
            for( int col = 0; col < 5; ++col )
            {
                if( bits[col] == '1' )
                    DrawOverlayRect( ctx, cursor + col * s, y - row * s, s * 0.82f, s * 0.82f, r, g, b, a );
            }
        }
        cursor += s * 6.0f;
    }
}

inline void DrawPipelineMeter( ID3D11DeviceContext* ctx, float x, float y, float w, bool on,
                               float r, float g, float b )
{
    DrawOverlayRect( ctx, x, y, w, 0.018f, 0.05f, 0.06f, 0.07f, 0.72f );
    if( on )
        DrawOverlayRect( ctx, x + 0.004f, y - 0.004f, w - 0.008f, 0.010f, r, g, b, 1.0f );
    else
        DrawOverlayRect( ctx, x + 0.004f, y - 0.004f, w - 0.008f, 0.010f, 0.16f, 0.16f, 0.18f, 1.0f );
}

inline void DrawMiniDiagram( ID3D11DeviceContext* ctx, int stage, float x, float y, float w, float h,
                             const PipelineSnapshot& snapshot )
{
    float r = g_pipelineStageInfo[stage].R;
    float g = g_pipelineStageInfo[stage].G;
    float b = g_pipelineStageInfo[stage].B;
    float cx = x + w * 0.5f;
    float cy = y - h * 0.55f;

    if( stage == PipelineStageIA )
    {
        for( int i = 0; i < 5; ++i )
            DrawOverlayRect( ctx, x + 0.035f + i * 0.036f, y - 0.075f - i * 0.016f, 0.026f, 0.026f, r, g, b, 0.96f );
        DrawOverlayRect( ctx, x + 0.030f, y - 0.190f, w - 0.060f, 0.010f, 0.82f, 0.86f, 0.92f, snapshot.HasIndexBuffer ? 0.9f : 0.25f );
    }
    else if( stage == PipelineStageVS )
    {
        DrawOverlayRect( ctx, cx - 0.018f, cy + 0.090f, 0.036f, 0.036f, r, g, b, 0.95f );
        DrawOverlayRect( ctx, cx - 0.085f, cy - 0.010f, 0.036f, 0.036f, r, g, b, 0.75f );
        DrawOverlayRect( ctx, cx + 0.050f, cy - 0.010f, 0.036f, 0.036f, r, g, b, 0.75f );
        DrawOverlayRect( ctx, cx - 0.055f, cy - 0.102f, 0.110f, 0.018f, r, g, b, snapshot.HasVSConstantBuffer ? 0.95f : 0.25f );
    }
    else if( stage == PipelineStageRS )
    {
        for( int i = 0; i < 4; ++i )
        {
            float px = x + 0.050f + i * 0.055f;
            DrawOverlayRect( ctx, px, y - 0.080f, 0.045f, 0.010f, r, g, b, 0.92f );
            DrawOverlayRect( ctx, px + 0.037f, y - 0.080f, 0.010f, 0.090f, r, g, b, 0.92f );
            DrawOverlayRect( ctx, px, y - 0.160f, 0.045f, 0.010f, r, g, b, 0.92f );
        }
    }
    else if( stage == PipelineStagePS )
    {
        for( int yy = 0; yy < 4; ++yy )
        {
            for( int xx = 0; xx < 5; ++xx )
            {
                float shade = 0.45f + 0.10f * float( xx + yy );
                DrawOverlayRect( ctx, x + 0.052f + xx * 0.030f, y - 0.075f - yy * 0.030f,
                                 0.025f, 0.025f, r * shade, g * shade, b * shade, snapshot.HasPixelShader ? 1.0f : 0.2f );
            }
        }
    }
    else if( stage == PipelineStageOM )
    {
        DrawOverlayRect( ctx, x + 0.055f, y - 0.070f, w - 0.110f, h * 0.42f, 0.05f, 0.08f, 0.13f, 0.94f );
        DrawOverlayRect( ctx, x + 0.070f, y - 0.088f, w - 0.140f, h * 0.14f, r, g, b, snapshot.HasRenderTarget ? 0.95f : 0.22f );
        DrawOverlayRect( ctx, x + 0.070f, y - 0.142f, w - 0.140f, h * 0.14f, 0.18f, 0.28f, 0.42f, snapshot.HasDepthTarget ? 0.95f : 0.22f );
    }
    else
    {
        float pulse = 0.5f + 0.5f * sinf( g_pipelineTrace.Time * 7.0f );
        DrawOverlayRect( ctx, x + 0.050f, y - 0.095f, w - 0.100f, 0.058f, r, g, b, 0.55f + pulse * 0.35f );
        DrawOverlayRect( ctx, x + 0.087f, y - 0.151f, w - 0.174f, 0.058f, r, g, b, 0.36f + pulse * 0.34f );
    }
}

inline PipelineSnapshot CapturePipelineSnapshot( ID3D11DeviceContext* ctx )
{
    PipelineSnapshot s;
    ID3D11InputLayout* inputLayout = nullptr;
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11Buffer* vsCB = nullptr;
    ID3D11Buffer* psCB = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
    UINT stride = 0, offset = 0, indexOffset = 0;
    D3D11_VIEWPORT vp[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
    UINT vpCount = ARRAYSIZE( vp );

    ctx->IAGetInputLayout( &inputLayout );
    ctx->IAGetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );
    ctx->IAGetIndexBuffer( &indexBuffer, &indexFormat, &indexOffset );
    ctx->IAGetPrimitiveTopology( &topology );
    ctx->VSGetShader( &vertexShader, nullptr, nullptr );
    ctx->PSGetShader( &pixelShader, nullptr, nullptr );
    ctx->VSGetConstantBuffers( 0, 1, &vsCB );
    ctx->PSGetConstantBuffers( 0, 1, &psCB );
    ctx->RSGetViewports( &vpCount, vp );
    ctx->OMGetRenderTargets( 1, &rtv, &dsv );

    s.HasInputLayout = inputLayout != nullptr;
    s.HasVertexBuffer = vertexBuffer != nullptr && stride > 0;
    s.HasIndexBuffer = indexBuffer != nullptr && indexFormat != DXGI_FORMAT_UNKNOWN;
    s.UsesTriangleList = topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    s.HasVertexShader = vertexShader != nullptr;
    s.HasVSConstantBuffer = vsCB != nullptr;
    s.HasViewport = vpCount > 0 && vp[0].Width > 0.0f && vp[0].Height > 0.0f;
    s.HasPixelShader = pixelShader != nullptr;
    s.HasPSConstantBuffer = psCB != nullptr;
    s.HasRenderTarget = rtv != nullptr;
    s.HasDepthTarget = dsv != nullptr;

    if( inputLayout ) inputLayout->Release();
    if( vertexBuffer ) vertexBuffer->Release();
    if( indexBuffer ) indexBuffer->Release();
    if( vertexShader ) vertexShader->Release();
    if( pixelShader ) pixelShader->Release();
    if( vsCB ) vsCB->Release();
    if( psCB ) psCB->Release();
    if( rtv ) rtv->Release();
    if( dsv ) dsv->Release();
    return s;
}

inline void DrawPipelinePanel( ID3D11DeviceContext* ctx, int stage, float x, float y, float w, float h,
                               const bool* checks, int checkCount, const PipelineSnapshot& snapshot )
{
    float r = g_pipelineStageInfo[stage].R;
    float g = g_pipelineStageInfo[stage].G;
    float b = g_pipelineStageInfo[stage].B;
    bool touched = g_pipelineTrace.StageTouched[stage];
    float active = touched ? 1.0f : 0.38f;

    DrawOverlayRect( ctx, x, y, w, h, 0.015f, 0.020f, 0.030f, 0.78f );
    DrawOverlayRect( ctx, x, y, w, 0.032f, r, g, b, 0.72f * active );
    DrawPipelineText( ctx, g_pipelineStageInfo[stage].Label, x + 0.018f, y - 0.046f, 0.0075f, 0.92f, 0.95f, 1.0f, active );
    DrawMiniDiagram( ctx, stage, x, y - 0.060f, w, h - 0.070f, snapshot );

    float meterX = x + 0.026f;
    float meterY = y - h + 0.054f;
    float meterW = ( w - 0.068f ) / float( checkCount > 0 ? checkCount : 1 );
    for( int i = 0; i < checkCount; ++i )
        DrawPipelineMeter( ctx, meterX + i * meterW, meterY, meterW - 0.010f, checks[i], r, g, b );
}

inline void ApplyStageSpecificRender( ID3D11DeviceContext* ctx, ID3D11RenderTargetView*, ID3D11DepthStencilView* )
{
    if( !g_pipelineOverlayVS || !g_pipelineOverlayPS || !g_pipelineOverlayCB )
        return;

    PipelineSnapshot snapshot = CapturePipelineSnapshot( ctx );

    ID3D11InputLayout* prevIL = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* prevVB = nullptr;
    UINT prevStride = 0, prevOffset = 0;
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11Buffer* prevVSCB = nullptr;
    ID3D11Buffer* prevPSCB = nullptr;
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencil = 0;
    ID3D11RasterizerState* prevRS = nullptr;

    ctx->IAGetInputLayout( &prevIL );
    ctx->IAGetPrimitiveTopology( &prevTopo );
    ctx->IAGetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSGetShader( &prevVS, nullptr, nullptr );
    ctx->PSGetShader( &prevPS, nullptr, nullptr );
    ctx->VSGetConstantBuffers( 1, 1, &prevVSCB );
    ctx->PSGetConstantBuffers( 1, 1, &prevPSCB );
    ctx->OMGetDepthStencilState( &prevDSS, &prevStencil );
    ctx->RSGetState( &prevRS );

    ctx->IASetInputLayout( nullptr );
    ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    ctx->IASetVertexBuffers( 0, 0, nullptr, nullptr, nullptr );
    ctx->VSSetShader( g_pipelineOverlayVS, nullptr, 0 );
    ctx->PSSetShader( g_pipelineOverlayPS, nullptr, 0 );
    ctx->OMSetDepthStencilState( g_pipelineNoDepth, 0 );
    ctx->RSSetState( g_pipelineNoCull );

    DrawOverlayRect( ctx, -0.985f, 0.970f, 1.970f, 0.035f, 0.01f, 0.012f, 0.018f, 0.82f );

    const float timelineX = -0.930f;
    const float timelineY = 0.942f;
    const float timelineW = 0.230f;
    const float timelineH = 0.026f;
    for( int i = 0; i < PipelineStageCount; ++i )
    {
        float x = timelineX + i * ( timelineW + 0.030f );
        float r = g_pipelineStageInfo[i].R;
        float g = g_pipelineStageInfo[i].G;
        float b = g_pipelineStageInfo[i].B;
        float alpha = g_pipelineTrace.StageTouched[i] ? 0.95f : 0.35f;
        DrawOverlayRect( ctx, x, timelineY, timelineW, timelineH, r, g, b, alpha );
        DrawPipelineText( ctx, g_pipelineStageInfo[i].Label, x + 0.014f, timelineY - 0.006f, 0.0040f, 0.96f, 0.98f, 1.0f, alpha );
        if( i < PipelineStageCount - 1 )
            DrawOverlayRect( ctx, x + timelineW + 0.006f, timelineY - 0.012f, 0.018f, 0.004f, 0.80f, 0.85f, 0.92f, 0.68f );
    }

    bool iaChecks[4] = { snapshot.HasInputLayout, snapshot.HasVertexBuffer, snapshot.HasIndexBuffer, snapshot.UsesTriangleList };
    bool vsChecks[3] = { snapshot.HasVertexShader, snapshot.HasVSConstantBuffer, g_pipelineTrace.StageTouched[PipelineStageVS] };
    bool rsChecks[2] = { snapshot.HasViewport, g_pipelineTrace.StageTouched[PipelineStageRS] };
    bool psChecks[3] = { snapshot.HasPixelShader, snapshot.HasPSConstantBuffer, g_pipelineTrace.StageTouched[PipelineStagePS] };
    bool omChecks[3] = { snapshot.HasRenderTarget, !snapshot.HasDepthTarget, g_pipelineTrace.StageTouched[PipelineStageOM] };
    bool drawChecks[3] = { g_pipelineTrace.DrawIndexedCalls > 0, g_pipelineTrace.LastIndexCount == 6, g_pipelineTrace.StageTouched[PipelineStageDraw] };

    const float panelW = 0.300f;
    const float panelH = 0.265f;
    const float startX = -0.960f;
    const float startY = -0.415f;
    const float gap = 0.022f;
    DrawPipelinePanel( ctx, PipelineStageIA, startX + 0 * ( panelW + gap ), startY, panelW, panelH, iaChecks, 4, snapshot );
    DrawPipelinePanel( ctx, PipelineStageVS, startX + 1 * ( panelW + gap ), startY, panelW, panelH, vsChecks, 3, snapshot );
    DrawPipelinePanel( ctx, PipelineStageRS, startX + 2 * ( panelW + gap ), startY, panelW, panelH, rsChecks, 2, snapshot );
    DrawPipelinePanel( ctx, PipelineStagePS, startX + 3 * ( panelW + gap ), startY, panelW, panelH, psChecks, 3, snapshot );
    DrawPipelinePanel( ctx, PipelineStageOM, startX + 4 * ( panelW + gap ), startY, panelW, panelH, omChecks, 3, snapshot );
    DrawPipelinePanel( ctx, PipelineStageDraw, startX + 5 * ( panelW + gap ), startY, panelW, panelH, drawChecks, 3, snapshot );

    ctx->IASetInputLayout( prevIL );
    ctx->IASetPrimitiveTopology( prevTopo );
    ctx->IASetVertexBuffers( 0, 1, &prevVB, &prevStride, &prevOffset );
    ctx->VSSetShader( prevVS, nullptr, 0 );
    ctx->PSSetShader( prevPS, nullptr, 0 );
    ctx->VSSetConstantBuffers( 1, 1, &prevVSCB );
    ctx->PSSetConstantBuffers( 1, 1, &prevPSCB );
    ctx->OMSetDepthStencilState( prevDSS, prevStencil );
    ctx->RSSetState( prevRS );

    if( prevIL ) prevIL->Release();
    if( prevVB ) prevVB->Release();
    if( prevVS ) prevVS->Release();
    if( prevPS ) prevPS->Release();
    if( prevVSCB ) prevVSCB->Release();
    if( prevPSCB ) prevPSCB->Release();
    if( prevDSS ) prevDSS->Release();
    if( prevRS ) prevRS->Release();
}
