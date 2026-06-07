//--------------------------------------------------------------------------------------
// File: main.cpp
//
// Example 1: Dx11 Basic, modernized
// - Uses shared D3DApp/DeviceResources infrastructure.
// - Manages Direct3D interfaces through ComPtr.
// - Handles WM_SIZE through swap-chain resize.
// - Compiles a small external HLSL file at runtime.
//--------------------------------------------------------------------------------------
#include "../common/D3DApp.h"
#include "../common/ShaderUtils.h"

#include <cmath>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

struct ClearColorCB
{
    DirectX::XMFLOAT4 Color;
};

class Dx11BasicApp final : public D3DApp
{
private:
    HRESULT OnInitialize() override
    {
        HRESULT hr = CreateShaders();
        if (FAILED(hr))
            return hr;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(ClearColorCB);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        return Graphics().Device()->CreateBuffer(&desc, nullptr, &m_clearColorCB);
    }

    void OnUpdate(float elapsedSeconds) override
    {
        m_time = elapsedSeconds;
    }

    void OnRender() override
    {
        ID3D11DeviceContext* context = Graphics().Context();
        ID3D11RenderTargetView* rtv = Graphics().RenderTargetView();

        float clearColor[4] =
        {
            0.03f,
            0.04f,
            0.06f,
            1.0f
        };
        context->ClearRenderTargetView(rtv, clearColor);

        ClearColorCB cb =
        {
            DirectX::XMFLOAT4(
                0.5f + 0.5f * sinf(m_time),
                0.5f + 0.5f * sinf(m_time + 2.094f),
                0.5f + 0.5f * sinf(m_time + 4.189f),
                1.0f)
        };
        context->UpdateSubresource(m_clearColorCB.Get(), 0, nullptr, &cb, 0, 0);

        context->OMSetRenderTargets(1, &rtv, nullptr);
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        context->PSSetConstantBuffers(0, 1, m_clearColorCB.GetAddressOf());
        context->Draw(3, 0);

        Graphics().Present(1);
    }

    HRESULT CreateShaders()
    {
        ComPtr<ID3DBlob> vsBlob;
        HRESULT hr = CompileShaderFromFile(L"ClearTriangle.hlsl", "VS", "vs_4_0", vsBlob);
        if (FAILED(hr))
            return hr;

        hr = Graphics().Device()->CreateVertexShader(
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            nullptr,
            &m_vertexShader);
        if (FAILED(hr))
            return hr;

        ComPtr<ID3DBlob> psBlob;
        hr = CompileShaderFromFile(L"ClearTriangle.hlsl", "PS", "ps_4_0", psBlob);
        if (FAILED(hr))
            return hr;

        return Graphics().Device()->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            nullptr,
            &m_pixelShader);
    }

    float m_time = 0.0f;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11Buffer> m_clearColorCB;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    Dx11BasicApp app;
    if (FAILED(app.Initialize(hInstance, nCmdShow, L"Direct3D 11 Tutorial 1: Direct3D 11 Basics")))
        return 0;

    return app.Run();
}
