#include <Windows.h>
#include <CommCtrl.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_ServiceControl";
constexpr int IDC_REFRESH  = 1001;
constexpr int IDC_RUNNING  = 1002;
constexpr int IDC_ALL      = 1003;
constexpr int IDC_LIST     = 1004;
constexpr int IDC_STATUS   = 1005;

HWND gList{};
HWND gStatus{};

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

const wchar_t* StateText(DWORD state)
{
    switch (state)
    {
    case SERVICE_STOPPED:       return L"Stopped";
    case SERVICE_START_PENDING: return L"Starting";
    case SERVICE_STOP_PENDING:  return L"Stopping";
    case SERVICE_RUNNING:       return L"Running";
    case SERVICE_PAUSED:        return L"Paused";
    default:                    return L"Other";
    }
}

const wchar_t* TypeText(DWORD type)
{
    if (type & SERVICE_WIN32_OWN_PROCESS)   return L"Win32-Own";
    if (type & SERVICE_WIN32_SHARE_PROCESS) return L"Win32-Share";
    if (type & SERVICE_KERNEL_DRIVER)       return L"Driver-Kernel";
    if (type & SERVICE_FILE_SYSTEM_DRIVER)  return L"Driver-FS";
    return L"Other";
}

void AddColumn(int index, int width, const wchar_t* text)
{
    LVCOLUMNW col{};
    col.mask     = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.cx       = width;
    col.iSubItem = index;
    col.pszText  = const_cast<wchar_t*>(text);
    ListView_InsertColumn(gList, index, &col);
}

void RefreshServices(DWORD stateFilter)
{
    ListView_DeleteAllItems(gList);

    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr,
        SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (!scm)
    {
        SendMessageW(gStatus, SB_SETTEXT, 0,
            reinterpret_cast<LPARAM>(L"OpenSCManager failed — try running as administrator."));
        return;
    }

    // Query buffer size
    DWORD bytesNeeded{}, serviceCount{}, resumeHandle{};
    EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
        stateFilter, nullptr, 0, &bytesNeeded, &serviceCount, &resumeHandle, nullptr);

    auto* buf = static_cast<BYTE*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded));
    resumeHandle = 0;
    if (!buf)
    {
        CloseServiceHandle(scm);
        return;
    }

    if (EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
        stateFilter, buf, bytesNeeded, &bytesNeeded, &serviceCount, &resumeHandle, nullptr))
    {
        auto* services = reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSW*>(buf);

        for (DWORD i = 0; i < serviceCount; ++i)
        {
            const auto& svc = services[i];
            const auto& proc = svc.ServiceStatusProcess;

            // Query display name from service config
            SC_HANDLE sh = OpenServiceW(scm, svc.lpServiceName, SERVICE_QUERY_CONFIG);
            wchar_t displayName[256]{};
            if (sh)
            {
                DWORD needed{};
                QueryServiceConfigW(sh, nullptr, 0, &needed);
                auto* cfg = static_cast<QUERY_SERVICE_CONFIGW*>(
                    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, needed));
                if (cfg)
                {
                    if (QueryServiceConfigW(sh, cfg, needed, &needed))
                        StringCchCopyW(displayName, ARRAYSIZE(displayName), cfg->lpDisplayName);
                    HeapFree(GetProcessHeap(), 0, cfg);
                }
                CloseServiceHandle(sh);
            }
            if (displayName[0] == L'\0')
                StringCchCopyW(displayName, ARRAYSIZE(displayName), svc.lpServiceName);

            wchar_t pid[16]{};
            StringCchPrintfW(pid, ARRAYSIZE(pid), L"%lu", proc.dwProcessId);

            int row = static_cast<int>(i);
            LVITEMW item{};
            item.mask   = LVIF_TEXT;
            item.iItem  = row;
            item.pszText = const_cast<wchar_t*>(svc.lpServiceName);
            ListView_InsertItem(gList, &item);
            ListView_SetItemText(gList, row, 1, const_cast<wchar_t*>(StateText(proc.dwCurrentState)));
            ListView_SetItemText(gList, row, 2, const_cast<wchar_t*>(TypeText(proc.dwServiceType)));
            ListView_SetItemText(gList, row, 3, pid);
            ListView_SetItemText(gList, row, 4, displayName);
        }
    }

    HeapFree(GetProcessHeap(), 0, buf);
    CloseServiceHandle(scm);

    wchar_t status[128]{};
    StringCchPrintfW(status, ARRAYSIZE(status),
        L"Services shown: %lu  (filter: %s)",
        serviceCount,
        stateFilter == SERVICE_ACTIVE ? L"Running only" : L"All states");
    SendMessageW(gStatus, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(status));
}

void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    SendMessageW(gStatus, WM_SIZE, 0, 0);

    RECT statusRc{};
    GetWindowRect(gStatus, &statusRc);
    int statusH = statusRc.bottom - statusRc.top;

    MoveWindow(GetDlgItem(hwnd, IDC_REFRESH), 12, 12, 120, 32, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_RUNNING), 144, 12, 120, 32, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_ALL),     276, 12, 100, 32, TRUE);
    MoveWindow(gList, 12, 56, rc.right - 24, rc.bottom - statusH - 68, TRUE);
}

void CreateControls(HWND hwnd, HINSTANCE instance)
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    CreateWindowW(L"BUTTON", L"Refresh All",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_REFRESH), instance, nullptr);
    CreateWindowW(L"BUTTON", L"Running Only",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_RUNNING), instance, nullptr);
    CreateWindowW(L"BUTTON", L"All States",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_ALL), instance, nullptr);

    gList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 0, 0, hwnd, ControlId(IDC_LIST), instance, nullptr);
    gStatus = CreateWindowExW(0, STATUSCLASSNAMEW, L"Ready",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, ControlId(IDC_STATUS), instance, nullptr);

    ListView_SetExtendedListViewStyle(gList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    AddColumn(0, 180, L"Service Name");
    AddColumn(1,  90, L"State");
    AddColumn(2,  90, L"Type");
    AddColumn(3,  70, L"PID");
    AddColumn(4, 280, L"Display Name");

    Layout(hwnd);
    RefreshServices(SERVICE_STATE_ALL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateControls(hwnd, reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance);
        return 0;

    case WM_SIZE:
        Layout(hwnd);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_REFRESH)
            RefreshServices(SERVICE_STATE_ALL);
        if (LOWORD(wParam) == IDC_RUNNING)
            RefreshServices(SERVICE_ACTIVE);
        if (LOWORD(wParam) == IDC_ALL)
            RefreshServices(SERVICE_STATE_ALL);
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"15 ServiceControl",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 600,
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
