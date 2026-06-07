#include <Windows.h>
#include <windowsx.h>

namespace
{
constexpr UINT_PTR kAnimationTimer = 1;
constexpr wchar_t kClassName[] = L"Win32Roadmap_MessageLoop";

struct AppState
{
    POINT ball{ 40, 50 };
    POINT velocity{ 5, 3 };
    POINT lastClick{ -1, -1 };
    int keyCount = 0;
};

AppState* GetState(HWND hwnd)
{
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new AppState{}));
        SetTimer(hwnd, kAnimationTimer, 16, nullptr);
        return 0;

    case WM_KEYDOWN:
        if (auto* state = GetState(hwnd))
        {
            ++state->keyCount;
            if (wParam == VK_SPACE)
            {
                state->velocity.x = -state->velocity.x;
                state->velocity.y = -state->velocity.y;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_LBUTTONDOWN:
        if (auto* state = GetState(hwnd))
        {
            state->lastClick.x = GET_X_LPARAM(lParam);
            state->lastClick.y = GET_Y_LPARAM(lParam);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_TIMER:
        if (wParam == kAnimationTimer)
        {
            auto* state = GetState(hwnd);
            RECT client{};
            GetClientRect(hwnd, &client);
            state->ball.x += state->velocity.x;
            state->ball.y += state->velocity.y;
            if (state->ball.x < 0 || state->ball.x > client.right - 30) state->velocity.x *= -1;
            if (state->ball.y < 0 || state->ball.y > client.bottom - 30) state->velocity.y *= -1;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        auto* state = GetState(hwnd);
        FillRect(dc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
        TextOutW(dc, 20, 20, L"WM_TIMER animates. SPACE reverses. Click records mouse position.", 64);
        if (state)
        {
            wchar_t text[128]{};
            wsprintfW(text, L"KeyDown count: %d, last click: (%d, %d)", state->keyCount, state->lastClick.x, state->lastClick.y);
            TextOutW(dc, 20, 48, text, lstrlenW(text));
            HBRUSH brush = CreateSolidBrush(RGB(40, 120, 220));
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            Ellipse(dc, state->ball.x, state->ball.y, state->ball.x + 30, state->ball.y + 30);
            SelectObject(dc, oldBrush);
            DeleteObject(brush);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, kAnimationTimer);
        delete GetState(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kClassName, L"02 MessageLoop", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 760, 460, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return static_cast<int>(GetLastError());

    ShowWindow(hwnd, showCommand);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
