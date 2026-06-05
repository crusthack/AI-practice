#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <strsafe.h>
#pragma comment(lib, "comctl32.lib")

namespace
{
constexpr wchar_t kClassName[]  = L"Win32Roadmap_Hook";
constexpr UINT_PTR kClearTimer  = 1;
constexpr int IDC_LOG    = 1001;
constexpr int IDC_CLEAR  = 1002;
constexpr int IDC_STATUS = 1003;

HWND gLog{};
HWND gStatus{};
HHOOK gKeyHook{};
HHOOK gMsgHook{};
int   gMsgHookHits{};
int   gKeyEvents{};

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

void AppendLog(const wchar_t* fmt, ...)
{
    wchar_t line[512]{};
    va_list args;
    va_start(args, fmt);
    StringCchVPrintfW(line, ARRAYSIZE(line), fmt, args);
    va_end(args);

    // Append to existing text
    int len = GetWindowTextLengthW(gLog);
    SendMessageW(gLog, EM_SETSEL, len, len);
    SendMessageW(gLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line));
    SendMessageW(gLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L"\r\n"));
    SendMessageW(gLog, EM_SCROLLCARET, 0, 0);
}

const wchar_t* VkName(DWORD vk)
{
    switch (vk)
    {
    case VK_RETURN:  return L"Enter";
    case VK_SPACE:   return L"Space";
    case VK_BACK:    return L"Backspace";
    case VK_TAB:     return L"Tab";
    case VK_ESCAPE:  return L"Escape";
    case VK_DELETE:  return L"Delete";
    case VK_LEFT:    return L"Left";
    case VK_RIGHT:   return L"Right";
    case VK_UP:      return L"Up";
    case VK_DOWN:    return L"Down";
    case VK_SHIFT:   return L"Shift";
    case VK_CONTROL: return L"Ctrl";
    case VK_MENU:    return L"Alt";
    case VK_F1:      return L"F1";
    case VK_F2:      return L"F2";
    case VK_F3:      return L"F3";
    case VK_F4:      return L"F4";
    case VK_F5:      return L"F5";
    default:         return nullptr;
    }
}

// WH_KEYBOARD hook — thread-local, fires for keyboard events in this thread
LRESULT CALLBACK KeyboardHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= 0 && !(lParam & 0x80000000)) // bit31=0 → key down
    {
        DWORD vk = static_cast<DWORD>(wParam);
        ++gKeyEvents;

        wchar_t keyName[64]{};
        const wchar_t* named = VkName(vk);
        if (named)
            StringCchCopyW(keyName, ARRAYSIZE(keyName), named);
        else if (vk >= 'A' && vk <= 'Z')
            StringCchPrintfW(keyName, ARRAYSIZE(keyName), L"%c", static_cast<wchar_t>(vk));
        else if (vk >= '0' && vk <= '9')
            StringCchPrintfW(keyName, ARRAYSIZE(keyName), L"%c", static_cast<wchar_t>(vk));
        else
            StringCchPrintfW(keyName, ARRAYSIZE(keyName), L"VK=0x%02lX", vk);

        bool ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool shift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;
        bool alt   = (GetAsyncKeyState(VK_MENU)    & 0x8000) != 0;

        wchar_t line[128]{};
        StringCchPrintfW(line, ARRAYSIZE(line),
            L"[KEY#%03d] %-10s %s%s%s",
            gKeyEvents, keyName,
            ctrl  ? L"Ctrl+"  : L"",
            shift ? L"Shift+" : L"",
            alt   ? L"Alt+"   : L"");
        AppendLog(line);

        // Update status bar
        wchar_t status[128]{};
        StringCchPrintfW(status, ARRAYSIZE(status),
            L"WH_KEYBOARD: %d events  |  WH_GETMESSAGE: %d hits",
            gKeyEvents, gMsgHookHits);
        SendMessageW(gStatus, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(status));
    }
    return CallNextHookEx(gKeyHook, code, wParam, lParam);
}

// WH_GETMESSAGE hook — thread-local, fires for every GetMessage/PeekMessage call
LRESULT CALLBACK GetMsgHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= 0 && wParam == PM_REMOVE)
    {
        const MSG* m = reinterpret_cast<const MSG*>(lParam);
        // Count keyboard messages only (WM_KEYDOWN/WM_SYSKEYDOWN)
        if (m->message == WM_KEYDOWN || m->message == WM_SYSKEYDOWN)
            ++gMsgHookHits;
    }
    return CallNextHookEx(gMsgHook, code, wParam, lParam);
}

void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    SendMessageW(gStatus, WM_SIZE, 0, 0);
    RECT statusRc{};
    GetWindowRect(gStatus, &statusRc);
    int statusH = statusRc.bottom - statusRc.top;
    MoveWindow(GetDlgItem(hwnd, IDC_CLEAR), 12, 12, 100, 32, TRUE);
    MoveWindow(gLog, 12, 56, rc.right - 24, rc.bottom - statusH - 68, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_BAR_CLASSES };
        InitCommonControlsEx(&icc);

        CreateWindowW(L"BUTTON", L"Clear Log",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, ControlId(IDC_CLEAR), inst, nullptr);
        gLog = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
            0, 0, 0, 0, hwnd, ControlId(IDC_LOG), inst, nullptr);
        gStatus = CreateWindowExW(0, STATUSCLASSNAMEW,
            L"WH_KEYBOARD + WH_GETMESSAGE installed. Press keys in this window.",
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd, ControlId(IDC_STATUS), inst, nullptr);

        // Install thread-local hooks
        DWORD tid = GetCurrentThreadId();
        gKeyHook = SetWindowsHookExW(WH_KEYBOARD,   KeyboardHook, nullptr, tid);
        gMsgHook = SetWindowsHookExW(WH_GETMESSAGE, GetMsgHook,   nullptr, tid);

        AppendLog(L"=== 35_Hook ===");
        AppendLog(gKeyHook ? L"WH_KEYBOARD hook:   installed (thread-local)" :
                             L"WH_KEYBOARD hook:   FAILED");
        AppendLog(gMsgHook ? L"WH_GETMESSAGE hook: installed (thread-local)" :
                             L"WH_GETMESSAGE hook: FAILED");
        AppendLog(L"");
        AppendLog(L"Press any key inside this window to see hook events.");
        AppendLog(L"(Global keyboard/mouse hooks require a DLL or elevated privileges.)");

        Layout(hwnd);
        return 0;
    }

    case WM_SIZE:
        Layout(hwnd);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_CLEAR)
        {
            SetWindowTextW(gLog, L"");
            gKeyEvents   = 0;
            gMsgHookHits = 0;
            SendMessageW(gStatus, SB_SETTEXT, 0,
                reinterpret_cast<LPARAM>(L"Log cleared. Press keys to see events."));
        }
        return 0;

    case WM_DESTROY:
        if (gKeyHook) { UnhookWindowsHookEx(gKeyHook); gKeyHook = nullptr; }
        if (gMsgHook) { UnhookWindowsHookEx(gMsgHook); gMsgHook = nullptr; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kClassName, L"35 Hook - WH_KEYBOARD + WH_GETMESSAGE",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 760, 560,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd)
        return static_cast<int>(GetLastError());

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
