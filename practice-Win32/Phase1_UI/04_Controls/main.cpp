#include <Windows.h>

namespace
{
constexpr int IDC_NAME = 1001;
constexpr int IDC_ADD = 1002;
constexpr int IDC_LIST = 1003;
constexpr int IDC_ROLE = 1004;
constexpr int IDC_STATUS = 1005;
constexpr wchar_t kClassName[] = L"Win32Roadmap_Controls";

HMENU ControlId(int id)
{
    return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
}

void CreateChildControls(HWND hwnd, HINSTANCE instance)
{
    CreateWindowW(L"STATIC", L"Name", WS_CHILD | WS_VISIBLE, 20, 22, 80, 24, hwnd, nullptr, instance, nullptr);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 20, 220, 26, hwnd, ControlId(IDC_NAME), instance, nullptr);
    CreateWindowW(L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        100, 56, 220, 160, hwnd, ControlId(IDC_ROLE), instance, nullptr);
    SendDlgItemMessageW(hwnd, IDC_ROLE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Window owner"));
    SendDlgItemMessageW(hwnd, IDC_ROLE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Input handler"));
    SendDlgItemMessageW(hwnd, IDC_ROLE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Renderer"));
    SendDlgItemMessageW(hwnd, IDC_ROLE, CB_SETCURSEL, 0, 0);

    CreateWindowW(L"BUTTON", L"Add to list", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        340, 20, 120, 30, hwnd, ControlId(IDC_ADD), instance, nullptr);
    CreateWindowW(L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        20, 100, 460, 180, hwnd, ControlId(IDC_LIST), instance, nullptr);
    CreateWindowW(L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        20, 300, 460, 24, hwnd, ControlId(IDC_STATUS), instance, nullptr);
}

void AddListItem(HWND hwnd)
{
    wchar_t name[128]{};
    wchar_t role[128]{};
    GetDlgItemTextW(hwnd, IDC_NAME, name, ARRAYSIZE(name));
    const LRESULT index = SendDlgItemMessageW(hwnd, IDC_ROLE, CB_GETCURSEL, 0, 0);
    SendDlgItemMessageW(hwnd, IDC_ROLE, CB_GETLBTEXT, index, reinterpret_cast<LPARAM>(role));

    if (name[0] == L'\0')
    {
        SetDlgItemTextW(hwnd, IDC_STATUS, L"Type a name first.");
        return;
    }

    wchar_t item[256]{};
    wsprintfW(item, L"%s - %s", name, role);
    SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
    SetDlgItemTextW(hwnd, IDC_NAME, L"");
    SetDlgItemTextW(hwnd, IDC_STATUS, L"Item added with WM_COMMAND.");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateChildControls(hwnd, reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_ADD && HIWORD(wParam) == BN_CLICKED)
        {
            AddListItem(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == IDC_LIST && HIWORD(wParam) == LBN_SELCHANGE)
        {
            SetDlgItemTextW(hwnd, IDC_STATUS, L"List selection changed.");
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
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(0, kClassName, L"04 Controls", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 540, 390, nullptr, nullptr, instance, nullptr);
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
