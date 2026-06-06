#pragma once

#include "DeviceResources.h"

#include <windows.h>

class D3DApp
{
public:
    virtual ~D3DApp() = default;

    HRESULT Initialize(HINSTANCE instance, int cmdShow, const wchar_t* title, UINT width = 640, UINT height = 480);
    int Run();

protected:
    virtual HRESULT OnInitialize() { return S_OK; }
    virtual void OnUpdate(float) {}
    virtual void OnRender() = 0;
    virtual void OnResize(UINT, UINT) {}
    virtual void OnDestroy() {}

    DeviceResources& Graphics() { return m_deviceResources; }
    HWND Window() const { return m_hwnd; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT InitWindow(HINSTANCE instance, int cmdShow, const wchar_t* title, UINT width, UINT height);

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    DeviceResources m_deviceResources;
};
