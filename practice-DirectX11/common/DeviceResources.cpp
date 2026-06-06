#include "DeviceResources.h"

#include <algorithm>

HRESULT DeviceResources::Initialize(HWND hwnd, UINT width, UINT height)
{
    m_hwnd = hwnd;
    m_width = std::max<UINT>(width, 1);
    m_height = std::max<UINT>(height, 1);

    HRESULT hr = CreateDeviceAndSwapChain(hwnd);
    if (FAILED(hr))
        return hr;

    return CreateWindowSizeDependentResources();
}

void DeviceResources::Resize(UINT width, UINT height)
{
    width = std::max<UINT>(width, 1);
    height = std::max<UINT>(height, 1);
    if (!m_swapChain || (width == m_width && height == m_height))
        return;

    m_width = width;
    m_height = height;

    if (m_context)
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset();

    HRESULT hr = m_swapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr))
        CreateWindowSizeDependentResources();
}

void DeviceResources::Present(UINT syncInterval)
{
    if (m_swapChain)
        m_swapChain->Present(syncInterval, 0);
}

HRESULT DeviceResources::CreateDeviceAndSwapChain(HWND hwnd)
{
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT hr = E_FAIL;
    for (D3D_DRIVER_TYPE driverType : driverTypes)
    {
        m_driverType = driverType;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            m_driverType,
            nullptr,
            createDeviceFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &sd,
            &m_swapChain,
            &m_device,
            &m_featureLevel,
            &m_context);
        if (SUCCEEDED(hr))
            break;
    }

    return hr;
}

HRESULT DeviceResources::CreateWindowSizeDependentResources()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(hr))
        return hr;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr))
        return hr;

    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(m_width);
    vp.Height = static_cast<FLOAT>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    return S_OK;
}
