#include <Windows.h>
#include <windowsx.h>
#include "resource.h"

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_MenusAccel";
HINSTANCE gInstance{};

struct AppState
{
    int commandCount = 0;
    bool statusVisible = true;
    wchar_t lastCommand[128] = L"Use the menu, accelerators, or right click.";
};

AppState* GetState(HWND hwnd)
{
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void SetLastCommand(HWND hwnd, const wchar_t* text)
{
    AppState* state = GetState(hwnd);
    if (!state)
        return;

    ++state->commandCount;
    lstrcpynW(state->lastCommand, text, ARRAYSIZE(state->lastCommand));
    InvalidateRect(hwnd, nullptr, TRUE);
}

void ShowContextMenu(HWND hwnd, POINT screenPoint)
{
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, IDM_CONTEXT_MARK, L"Mark Context Action");
    AppendMenuW(menu, MF_STRING, IDM_CONTEXT_CLEAR, L"Clear Status");
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, screenPoint.x, screenPoint.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new AppState{}));
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_FILE_NEW:
            SetLastCommand(hwnd, L"File > New, or Ctrl+N, was selected.");
            return 0;
        case IDM_FILE_EXIT:
            DestroyWindow(hwnd);
            return 0;
        case IDM_VIEW_TOGGLE_STATUS:
        {
            AppState* state = GetState(hwnd);
            state->statusVisible = !state->statusVisible;
            SetLastCommand(hwnd, state->statusVisible ? L"Status text is visible." : L"Status text is hidden.");
            return 0;
        }
        case IDM_HELP_ABOUT:
            MessageBoxW(hwnd, L"06 Menus_Accel\nMenu bar, popup menu, and accelerator table.", L"About", MB_OK | MB_ICONINFORMATION);
            SetLastCommand(hwnd, L"Help > About, or F1, was selected.");
            return 0;
        case IDM_CONTEXT_MARK:
            SetLastCommand(hwnd, L"Context menu item was selected.");
            return 0;
        case IDM_CONTEXT_CLEAR:
            if (AppState* state = GetState(hwnd))
            {
                state->commandCount = 0;
                lstrcpynW(state->lastCommand, L"Status cleared from context menu.", ARRAYSIZE(state->lastCommand));
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            return 0;
        }
        return 0;

    case WM_CONTEXTMENU:
    {
        POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (point.x == -1 && point.y == -1)
        {
            RECT rc{};
            GetClientRect(hwnd, &rc);
            point = { rc.left + 24, rc.top + 24 };
            ClientToScreen(hwnd, &point);
        }
        ShowContextMenu(hwnd, point);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        RECT client{};
        GetClientRect(hwnd, &client);
        FillRect(dc, &client, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

        SetBkMode(dc, TRANSPARENT);
        TextOutW(dc, 24, 24, L"06 Menus_Accel", 15);
        TextOutW(dc, 24, 52, L"Try File > New, Ctrl+N, Ctrl+T, F1, or right click.", 57);

        if (AppState* state = GetState(hwnd); state && state->statusVisible)
        {
            wchar_t line[256]{};
            wsprintfW(line, L"Commands handled: %d", state->commandCount);
            TextOutW(dc, 24, 96, line, lstrlenW(line));
            TextOutW(dc, 24, 124, state->lastCommand, lstrlenW(state->lastCommand));
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        delete GetState(hwnd);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    gInstance = instance;

    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        kClassName,
        L"06 Menus and Accelerators",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        700,
        420,
        nullptr,
        LoadMenuW(instance, MAKEINTRESOURCEW(IDM_MAIN_MENU)),
        instance,
        nullptr);

    if (!hwnd)
        return static_cast<int>(GetLastError());

    HACCEL accelerators = LoadAcceleratorsW(instance, MAKEINTRESOURCEW(IDA_MAIN_ACCEL));

    ShowWindow(hwnd, showCommand);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!TranslateAcceleratorW(hwnd, accelerators, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}
