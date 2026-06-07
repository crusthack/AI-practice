#include <Windows.h>
#include <commdlg.h>
#include "resource.h"

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_Dialogs";
HINSTANCE gInstance{};

INT_PTR CALLBACK InputDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemTextW(dialog, IDC_INPUT_TEXT, L"Hello dialog");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            wchar_t buffer[256]{};
            GetDlgItemTextW(dialog, IDC_INPUT_TEXT, buffer, ARRAYSIZE(buffer));
            EndDialog(dialog, reinterpret_cast<INT_PTR>(_wcsdup(buffer)));
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(dialog, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void ShowInputDialog(HWND owner)
{
    const INT_PTR result = DialogBoxW(gInstance, MAKEINTRESOURCEW(IDD_INPUT_DIALOG), owner, InputDialogProc);
    if (result)
    {
        const wchar_t* text = reinterpret_cast<const wchar_t*>(result);
        SetDlgItemTextW(owner, IDC_RESULT, text);
        free(const_cast<wchar_t*>(text));
    }
}

void ShowOpenFileDialog(HWND owner)
{
    wchar_t fileName[MAX_PATH]{};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.txt\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = ARRAYSIZE(fileName);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
        SetDlgItemTextW(owner, IDC_RESULT, fileName);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Open modal dialog", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 20, 160, 32, hwnd, reinterpret_cast<HMENU>(IDC_OPEN_DIALOG), gInstance, nullptr);
        CreateWindowW(L"BUTTON", L"Open file dialog", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, 20, 160, 32, hwnd, reinterpret_cast<HMENU>(IDC_OPEN_FILE), gInstance, nullptr);
        CreateWindowW(L"STATIC", L"Result appears here.", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
            20, 80, 520, 28, hwnd, reinterpret_cast<HMENU>(IDC_RESULT), gInstance, nullptr);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_OPEN_DIALOG)
        {
            ShowInputDialog(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == IDC_OPEN_FILE)
        {
            ShowOpenFileDialog(hwnd);
            return 0;
        }
        return 0;
    case WM_DESTROY:
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"05 Dialogs", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 220, nullptr, nullptr, instance, nullptr);
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
