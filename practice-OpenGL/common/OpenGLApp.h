#pragma once

#include "OpenGLRuntime.h"

#include <chrono>

#ifndef OPENGL_SAMPLE_TITLE
#define OPENGL_SAMPLE_TITLE L"OpenGL Sample"
#endif

#ifndef OPENGL_SAMPLE_CLASS
#define OPENGL_SAMPLE_CLASS L"OpenGLSampleWindowClass"
#endif

namespace OpenGLPractice
{
struct AppState
{
    HWND Window = nullptr;
    HDC DeviceContext = nullptr;
    HGLRC Context = nullptr;
    int Width = 1280;
    int Height = 720;
};

inline AppState* g_app = nullptr;

inline void ThrowIfFalse(bool ok, const char* message)
{
    if (!ok)
        throw std::runtime_error(message);
}

inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        if (g_app)
        {
            g_app->Width = LOWORD(lParam);
            g_app->Height = HIWORD(lParam);
        }
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    default:
        break;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

inline void InitWindow(AppState& app, HINSTANCE instance, int showCommand)
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = OPENGL_SAMPLE_CLASS;
    ThrowIfFalse(RegisterClassExW(&wc) != 0, "RegisterClassExW failed.");

    RECT rect = { 0, 0, app.Width, app.Height };
    ThrowIfFalse(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) != 0, "AdjustWindowRect failed.");

    app.Window = CreateWindowExW(
        0,
        OPENGL_SAMPLE_CLASS,
        OPENGL_SAMPLE_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);
    ThrowIfFalse(app.Window != nullptr, "CreateWindowExW failed.");
    ShowWindow(app.Window, showCommand);
}

inline void SetPixelFormatForOpenGL(HDC dc)
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int pixelFormat = ChoosePixelFormat(dc, &pfd);
    ThrowIfFalse(pixelFormat != 0, "ChoosePixelFormat failed.");
    ThrowIfFalse(SetPixelFormat(dc, pixelFormat, &pfd) != 0, "SetPixelFormat failed.");
}

inline void InitOpenGL(AppState& app)
{
    app.DeviceContext = GetDC(app.Window);
    ThrowIfFalse(app.DeviceContext != nullptr, "GetDC failed.");
    SetPixelFormatForOpenGL(app.DeviceContext);

    HGLRC bootstrap = wglCreateContext(app.DeviceContext);
    ThrowIfFalse(bootstrap != nullptr, "wglCreateContext bootstrap failed.");
    ThrowIfFalse(wglMakeCurrent(app.DeviceContext, bootstrap) != 0, "wglMakeCurrent bootstrap failed.");
    LoadWglExtensions();

    if (wglCreateContextAttribsARB_)
    {
        constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
        constexpr int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
        constexpr int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
        constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;
        constexpr int WGL_CONTEXT_FLAGS_ARB = 0x2094;
        constexpr int WGL_CONTEXT_DEBUG_BIT_ARB = 0x00000001;

        const int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if defined(_DEBUG)
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
            0
        };

        HGLRC modern = wglCreateContextAttribsARB_(app.DeviceContext, nullptr, attribs);
        if (!modern)
        {
            const int fallbackAttribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
            modern = wglCreateContextAttribsARB_(app.DeviceContext, nullptr, fallbackAttribs);
        }

        if (modern)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(bootstrap);
            app.Context = modern;
            ThrowIfFalse(wglMakeCurrent(app.DeviceContext, app.Context) != 0, "wglMakeCurrent modern context failed.");
        }
        else
        {
            app.Context = bootstrap;
        }
    }
    else
    {
        app.Context = bootstrap;
    }

    LoadOpenGLFunctions();
    if (wglSwapIntervalEXT_)
        wglSwapIntervalEXT_(1);

    glViewport(0, 0, app.Width, app.Height);
    glEnable(GL_MULTISAMPLE);
}

inline void CleanupOpenGL(AppState& app)
{
    if (app.Context)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(app.Context);
        app.Context = nullptr;
    }
    if (app.DeviceContext)
    {
        ReleaseDC(app.Window, app.DeviceContext);
        app.DeviceContext = nullptr;
    }
}

inline int Run(HINSTANCE instance, int showCommand)
{
    AppState app = {};
    g_app = &app;

    InitWindow(app, instance, showCommand);
    InitOpenGL(app);

    LearningStageState stage = {};
    LearningStageSetupContext setup = {};
    setup.Width = app.Width;
    setup.Height = app.Height;
    setup.Window = app.Window;
    ApplyStageSpecificSetup(stage, setup);

    auto start = std::chrono::steady_clock::now();
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        const double timeSeconds = std::chrono::duration<double>(now - start).count();
        UpdateStageSpecificDemo(stage, timeSeconds);

        LearningStageRenderContext render = {};
        render.Width = app.Width > 0 ? app.Width : 1;
        render.Height = app.Height > 0 ? app.Height : 1;
        ApplyStageSpecificRender(stage, render);
        SwapBuffers(app.DeviceContext);
    }

    ApplyStageSpecificCleanup(stage);
    CleanupOpenGL(app);
    g_app = nullptr;
    return static_cast<int>(msg.wParam);
}

} // namespace OpenGLPractice

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    try
    {
        return OpenGLPractice::Run(instance, showCommand);
    }
    catch (const std::exception& ex)
    {
        const std::string text = ex.what();
        std::wstring wide(text.begin(), text.end());
        MessageBoxW(nullptr, wide.c_str(), OPENGL_SAMPLE_TITLE L" Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}

