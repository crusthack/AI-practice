#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class DeviceResources
{
public:
    HRESULT Initialize(HWND hwnd, UINT width, UINT height);
    void Resize(UINT width, UINT height);
    void Present(UINT syncInterval = 1);

    ID3D11Device* Device() const { return m_device.Get(); }
    ID3D11DeviceContext* Context() const { return m_context.Get(); }
    ID3D11RenderTargetView* RenderTargetView() const { return m_renderTargetView.Get(); }
    UINT Width() const { return m_width; }
    UINT Height() const { return m_height; }

private:
    HRESULT CreateDeviceAndSwapChain(HWND hwnd);
    HRESULT CreateWindowSizeDependentResources();

    HWND m_hwnd = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
};
