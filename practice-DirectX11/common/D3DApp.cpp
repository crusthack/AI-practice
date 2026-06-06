#include "D3DApp.h"

#include <algorithm>

HRESULT D3DApp::Initialize(HINSTANCE instance, int cmdShow, const wchar_t* title, UINT width, UINT height)
{
    HRESULT hr = InitWindow(instance, cmdShow, title, width, height);
    if (FAILED(hr))
        return hr;

    RECT rc = {};
    GetClientRect(m_hwnd, &rc);
    hr = m_deviceResources.Initialize(
        m_hwnd,
        static_cast<UINT>(rc.right - rc.left),
        static_cast<UINT>(rc.bottom - rc.top));
    if (FAILED(hr))
        return hr;

    return OnInitialize();
}

int D3DApp::Run()
{
    MSG msg = {};
    DWORD timeStart = GetTickCount();

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DWORD timeCur = GetTickCount();
            float elapsed = (timeCur - timeStart) / 1000.0f;
            OnUpdate(elapsed);
            OnRender();
        }
    }

    OnDestroy();
    return static_cast<int>(msg.wParam);
}

HRESULT D3DApp::InitWindow(HINSTANCE instance, int cmdShow, const wchar_t* title, UINT width, UINT height)
{
    m_instance = instance;

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = D3DApp::StaticWndProc;
    wcex.hInstance = instance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"D3D11SampleWindowClass";
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hwnd = CreateWindow(
        L"D3D11SampleWindowClass",
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        instance,
        this);
    if (!m_hwnd)
        return E_FAIL;

    ShowWindow(m_hwnd, cmdShow);
    return S_OK;
}

LRESULT CALLBACK D3DApp::StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        auto app = static_cast<D3DApp*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }

    auto app = reinterpret_cast<D3DApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (app)
        return app->WndProc(hwnd, message, wParam, lParam);

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT D3DApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            m_deviceResources.Resize(width, height);
            OnResize(width, height);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
