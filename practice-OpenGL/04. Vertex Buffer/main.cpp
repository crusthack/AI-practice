#include <windows.h>
#include <gl/GL.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>

#include "LearningStage.h"

namespace
{
constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;

HWND g_window = nullptr;
HDC g_deviceContext = nullptr;
HGLRC g_glContext = nullptr;

void ThrowIfFalse(bool ok, const char* message)
{
    if (!ok)
    {
        throw std::runtime_error(message);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
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

void InitWindow(HINSTANCE instance, int showCommand)
{
    const wchar_t* className = L"OpenGL04VertexBufferWindowClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    ThrowIfFalse(RegisterClassExW(&windowClass) != 0, "RegisterClassExW failed.");

    RECT rect = { 0, 0, WindowWidth, WindowHeight };
    ThrowIfFalse(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) != 0, "AdjustWindowRect failed.");

    g_window = CreateWindowExW(
        0,
        className,
        L"04. Vertex Buffer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);
    ThrowIfFalse(g_window != nullptr, "CreateWindowExW failed.");
    ShowWindow(g_window, showCommand);
}

void InitOpenGL()
{
    g_deviceContext = GetDC(g_window);
    ThrowIfFalse(g_deviceContext != nullptr, "GetDC failed.");

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int pixelFormat = ChoosePixelFormat(g_deviceContext, &pfd);
    ThrowIfFalse(pixelFormat != 0, "ChoosePixelFormat failed.");
    ThrowIfFalse(SetPixelFormat(g_deviceContext, pixelFormat, &pfd) != 0, "SetPixelFormat failed.");

    g_glContext = wglCreateContext(g_deviceContext);
    ThrowIfFalse(g_glContext != nullptr, "wglCreateContext failed.");
    ThrowIfFalse(wglMakeCurrent(g_deviceContext, g_glContext) != 0, "wglMakeCurrent failed.");
}

void CleanupOpenGL()
{
    if (g_glContext)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_glContext);
        g_glContext = nullptr;
    }

    if (g_deviceContext)
    {
        ReleaseDC(g_window, g_deviceContext);
        g_deviceContext = nullptr;
    }
}

void Render(LearningStageState& stage)
{
    LearningStageRenderContext context = {};
    context.Width = WindowWidth;
    context.Height = WindowHeight;
    ApplyStageSpecificRender(stage, context);
    SwapBuffers(g_deviceContext);
}

int Run(HINSTANCE instance, int showCommand)
{
    InitWindow(instance, showCommand);
    InitOpenGL();

    LearningStageState stage = {};
    LearningStageSetupContext setupContext = {};
    setupContext.Width = WindowWidth;
    setupContext.Height = WindowHeight;
    ApplyStageSpecificSetup(stage, setupContext);

    auto startTime = std::chrono::steady_clock::now();
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
        const double timeSeconds = std::chrono::duration<double>(now - startTime).count();
        UpdateStageSpecificDemo(stage, timeSeconds);
        Render(stage);
    }

    ApplyStageSpecificCleanup(stage);
    CleanupOpenGL();
    return static_cast<int>(msg.wParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    try
    {
        return Run(instance, showCommand);
    }
    catch (const std::exception& ex)
    {
        std::wstring message(ex.what(), ex.what() + std::strlen(ex.what()));
        MessageBoxW(nullptr, message.c_str(), L"04. Vertex Buffer Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
