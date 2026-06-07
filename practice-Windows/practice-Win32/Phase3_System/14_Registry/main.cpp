#include <Windows.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_Registry";
constexpr int IDC_RUN    = 1001;
constexpr int IDC_TEXT   = 1002;
constexpr int IDC_DELETE = 1003;
constexpr wchar_t kSubKey[] = L"Software\\Win32Roadmap\\RegistryDemo";

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

void Append(wchar_t* buf, size_t count, const wchar_t* fmt, ...)
{
    wchar_t line[512]{};
    va_list args;
    va_start(args, fmt);
    StringCchVPrintfW(line, ARRAYSIZE(line), fmt, args);
    va_end(args);
    StringCchCatW(buf, count, line);
    StringCchCatW(buf, count, L"\r\n");
}

const wchar_t* RegTypeText(DWORD type)
{
    switch (type)
    {
    case REG_SZ:     return L"REG_SZ";
    case REG_DWORD:  return L"REG_DWORD";
    case REG_BINARY: return L"REG_BINARY";
    case REG_QWORD:  return L"REG_QWORD";
    default:         return L"OTHER";
    }
}

void RunDemo(HWND textCtrl)
{
    wchar_t out[4096] = L"=== 14_Registry ===\r\n";

    // --- Create/Open key ---
    HKEY key{};
    DWORD disposition{};
    LSTATUS status = RegCreateKeyExW(HKEY_CURRENT_USER, kSubKey, 0, nullptr, 0,
        KEY_ALL_ACCESS, nullptr, &key, &disposition);
    if (status != ERROR_SUCCESS)
    {
        Append(out, ARRAYSIZE(out), L"RegCreateKeyExW FAILED: %ld", status);
        SetWindowTextW(textCtrl, out);
        return;
    }
    Append(out, ARRAYSIZE(out), L"[Create] HKCU\\%s", kSubKey);
    Append(out, ARRAYSIZE(out), L"         Disposition: %s",
        disposition == REG_CREATED_NEW_KEY ? L"REG_CREATED_NEW_KEY" : L"REG_OPENED_EXISTING_KEY");

    // --- Write values ---
    const wchar_t strVal[] = L"Hello from Win32 Registry demo";
    RegSetValueExW(key, L"Message", 0, REG_SZ,
        reinterpret_cast<const BYTE*>(strVal), sizeof(strVal));

    DWORD dwordVal = GetTickCount() & 0xFFFF;
    RegSetValueExW(key, L"TickLow", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwordVal), sizeof(dwordVal));

    const BYTE binVal[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE };
    RegSetValueExW(key, L"BinaryBlob", 0, REG_BINARY, binVal, sizeof(binVal));

    ULONGLONG qwordVal = 0x0102030405060708ULL;
    RegSetValueExW(key, L"QWord64", 0, REG_QWORD,
        reinterpret_cast<const BYTE*>(&qwordVal), sizeof(qwordVal));

    Append(out, ARRAYSIZE(out),
        L"[Write] Message     = \"%s\"", strVal);
    Append(out, ARRAYSIZE(out),
        L"[Write] TickLow     = %lu (REG_DWORD)", dwordVal);
    Append(out, ARRAYSIZE(out),
        L"[Write] BinaryBlob  = DE AD BE EF CA FE (REG_BINARY)");
    Append(out, ARRAYSIZE(out),
        L"[Write] QWord64     = 0x%016llX (REG_QWORD)", qwordVal);

    // --- Read back ---
    wchar_t readStr[256]{};
    DWORD readBytes = sizeof(readStr);
    DWORD readType{};
    RegQueryValueExW(key, L"Message", nullptr, &readType,
        reinterpret_cast<BYTE*>(readStr), &readBytes);
    Append(out, ARRAYSIZE(out), L"[Read] Message = \"%s\" (%s)", readStr, RegTypeText(readType));

    DWORD readDword{};
    readBytes = sizeof(readDword);
    RegQueryValueExW(key, L"TickLow", nullptr, nullptr,
        reinterpret_cast<BYTE*>(&readDword), &readBytes);
    Append(out, ARRAYSIZE(out), L"[Read] TickLow = %lu", readDword);

    // --- Enumerate values ---
    Append(out, ARRAYSIZE(out), L"\r\n[Enum] Values in key:");
    DWORD idx = 0;
    for (;;)
    {
        wchar_t valueName[64]{};
        DWORD nameLen = ARRAYSIZE(valueName);
        DWORD valType{};
        LSTATUS es = RegEnumValueW(key, idx, valueName, &nameLen,
            nullptr, &valType, nullptr, nullptr);
        if (es != ERROR_SUCCESS)
            break;
        Append(out, ARRAYSIZE(out), L"  [%lu] %-16s (%s)", idx, valueName, RegTypeText(valType));
        ++idx;
    }
    Append(out, ARRAYSIZE(out), L"  Total: %lu value(s)", idx);

    // --- Subkey: create a child key ---
    HKEY child{};
    DWORD childDisp{};
    RegCreateKeyExW(key, L"SubKeyDemo", 0, nullptr, 0,
        KEY_ALL_ACCESS, nullptr, &child, &childDisp);
    const wchar_t childVal[] = L"child key value";
    RegSetValueExW(child, L"Item", 0, REG_SZ,
        reinterpret_cast<const BYTE*>(childVal), sizeof(childVal));
    RegCloseKey(child);
    Append(out, ARRAYSIZE(out), L"\r\n[SubKey] Created SubKeyDemo with Item value");

    // --- Enumerate subkeys ---
    Append(out, ARRAYSIZE(out), L"[Enum] Subkeys:");
    DWORD kidx = 0;
    for (;;)
    {
        wchar_t subName[64]{};
        DWORD subLen = ARRAYSIZE(subName);
        LSTATUS es = RegEnumKeyExW(key, kidx, subName, &subLen,
            nullptr, nullptr, nullptr, nullptr);
        if (es != ERROR_SUCCESS)
            break;
        Append(out, ARRAYSIZE(out), L"  [%lu] %s", kidx, subName);
        ++kidx;
    }

    RegCloseKey(key);

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs: RegCreateKeyEx, RegSetValueEx, RegQueryValueEx");
    Append(out, ARRAYSIZE(out), L"      RegEnumValue, RegEnumKeyEx, RegCloseKey");
    Append(out, ARRAYSIZE(out), L"      (click Delete Key to remove the demo entries)");

    SetWindowTextW(textCtrl, out);
}

void DeleteDemoKey(HWND textCtrl)
{
    // Delete child subkey first, then parent
    HKEY key{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kSubKey, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
        RegDeleteKeyW(key, L"SubKeyDemo");
        RegCloseKey(key);
    }
    LSTATUS s = RegDeleteKeyW(HKEY_CURRENT_USER, kSubKey);
    wchar_t msg[256]{};
    StringCchPrintfW(msg, ARRAYSIZE(msg),
        s == ERROR_SUCCESS ? L"Deleted HKCU\\%s" : L"Delete failed (status=%ld)  HKCU\\%s",
        s == ERROR_SUCCESS ? kSubKey : reinterpret_cast<const wchar_t*>(static_cast<LONG_PTR>(s)),
        kSubKey);
    // simple message for delete result
    wchar_t out[512]{};
    StringCchPrintfW(out, ARRAYSIZE(out),
        s == ERROR_SUCCESS ? L"[Delete] OK - HKCU\\%s removed" : L"[Delete] status=%ld",
        s == ERROR_SUCCESS ? kSubKey : reinterpret_cast<const wchar_t*>(static_cast<LONG_PTR>(s)));
    SetWindowTextW(textCtrl, out);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND text{};
    switch (message)
    {
    case WM_CREATE:
    {
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        CreateWindowW(L"BUTTON", L"Run Registry Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 190, 32,
            hwnd, ControlId(IDC_RUN), inst, nullptr);
        CreateWindowW(L"BUTTON", L"Delete Key",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            225, 20, 120, 32,
            hwnd, ControlId(IDC_DELETE), inst, nullptr);
        text = CreateWindowW(L"EDIT", L"Click  Run Registry Demo.",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            20, 68, 740, 380,
            hwnd, ControlId(IDC_TEXT), inst, nullptr);
        return 0;
    }

    case WM_SIZE:
    {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        MoveWindow(text, 20, 68, rc.right - 40, rc.bottom - 84, TRUE);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RUN)
            RunDemo(text);
        if (LOWORD(wParam) == IDC_DELETE)
            DeleteDemoKey(text);
        return 0;

    case WM_DESTROY:
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"14 Registry",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 540,
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
