#include <Windows.h>
#include <CommCtrl.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <psapi.h>
#include <strsafe.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_ProcessExplorer";
constexpr int IDC_REFRESH  = 1001;
constexpr int IDC_DUMP     = 1002;
constexpr int IDC_KILL     = 1003;
constexpr int IDC_LIST     = 1004;
constexpr int IDC_STATUS   = 1005;
constexpr int IDC_FILTER   = 1006;

// Column indices
enum Col { ColPID, ColParent, ColThreads, ColSession, ColWS_KB, ColPriority, ColImage, ColCount };

HWND gList{};
HWND gStatus{};
HWND gFilter{};

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

void SetStatus(const wchar_t* text)
{
    SendMessageW(gStatus, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(text));
}

void AddColumn(int index, int width, const wchar_t* text)
{
    LVCOLUMNW column{};
    column.mask     = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.cx       = width;
    column.iSubItem = index;
    column.pszText  = const_cast<wchar_t*>(text);
    ListView_InsertColumn(gList, index, &column);
}

void SetSubItem(int row, int col, const wchar_t* text)
{
    ListView_SetItemText(gList, row, col, const_cast<wchar_t*>(text));
}

const wchar_t* PriorityText(DWORD cls)
{
    switch (cls)
    {
    case IDLE_PRIORITY_CLASS:         return L"Idle";
    case BELOW_NORMAL_PRIORITY_CLASS: return L"Below Normal";
    case NORMAL_PRIORITY_CLASS:       return L"Normal";
    case ABOVE_NORMAL_PRIORITY_CLASS: return L"Above Normal";
    case HIGH_PRIORITY_CLASS:         return L"High";
    case REALTIME_PRIORITY_CLASS:     return L"Realtime";
    default:                          return L"Unknown";
    }
}

void RefreshProcesses()
{
    wchar_t filterText[64]{};
    GetWindowTextW(gFilter, filterText, ARRAYSIZE(filterText));
    bool hasFilter = filterText[0] != L'\0';

    ListView_DeleteAllItems(gList);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        SetStatus(L"CreateToolhelp32Snapshot failed.");
        return;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    int row = 0;

    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            // Apply filter
            if (hasFilter)
            {
                wchar_t lower[MAX_PATH]{};
                StringCchCopyW(lower, ARRAYSIZE(lower), entry.szExeFile);
                CharLowerW(lower);
                wchar_t filterLower[64]{};
                StringCchCopyW(filterLower, ARRAYSIZE(filterLower), filterText);
                CharLowerW(filterLower);
                if (!wcsstr(lower, filterLower))
                    continue;
            }

            wchar_t pid[16]{};
            wchar_t parent[16]{};
            wchar_t threads[16]{};
            wchar_t session[16]{};
            wchar_t ws[24]{};
            wchar_t priority[24]{};

            StringCchPrintfW(pid,     ARRAYSIZE(pid),     L"%lu", entry.th32ProcessID);
            StringCchPrintfW(parent,  ARRAYSIZE(parent),  L"%lu", entry.th32ParentProcessID);
            StringCchPrintfW(threads, ARRAYSIZE(threads), L"%lu", entry.cntThreads);

            // Session ID
            DWORD sessionId{};
            if (ProcessIdToSessionId(entry.th32ProcessID, &sessionId))
                StringCchPrintfW(session, ARRAYSIZE(session), L"%lu", sessionId);
            else
                StringCchCopyW(session, ARRAYSIZE(session), L"-");

            // Working set + priority class
            HANDLE hProc = OpenProcess(
                PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                FALSE, entry.th32ProcessID);
            if (hProc)
            {
                PROCESS_MEMORY_COUNTERS pmc{};
                pmc.cb = sizeof(pmc);
                if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc)))
                    StringCchPrintfW(ws, ARRAYSIZE(ws), L"%zu", pmc.WorkingSetSize / 1024);
                else
                    StringCchCopyW(ws, ARRAYSIZE(ws), L"—");

                DWORD cls = GetPriorityClass(hProc);
                StringCchCopyW(priority, ARRAYSIZE(priority),
                    cls ? PriorityText(cls) : L"—");

                CloseHandle(hProc);
            }
            else
            {
                StringCchCopyW(ws,       ARRAYSIZE(ws),       L"—");
                StringCchCopyW(priority, ARRAYSIZE(priority), L"—");
            }

            LVITEMW item{};
            item.mask   = LVIF_TEXT | LVIF_PARAM;
            item.iItem  = row;
            item.pszText = pid;
            item.lParam  = static_cast<LPARAM>(entry.th32ProcessID);
            ListView_InsertItem(gList, &item);
            SetSubItem(row, ColParent,  parent);
            SetSubItem(row, ColThreads, threads);
            SetSubItem(row, ColSession, session);
            SetSubItem(row, ColWS_KB,   ws);
            SetSubItem(row, ColPriority,priority);
            SetSubItem(row, ColImage,   entry.szExeFile);
            ++row;
        }
        while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    wchar_t status[128]{};
    StringCchPrintfW(status, ARRAYSIZE(status),
        hasFilter ? L"Processes: %d  (filter: \"%s\")" : L"Processes: %d",
        row, filterText);
    SetStatus(status);
}

DWORD SelectedPid()
{
    int selected = ListView_GetNextItem(gList, -1, LVNI_SELECTED);
    if (selected < 0)
        return 0;
    LVITEMW item{};
    item.mask  = LVIF_PARAM;
    item.iItem = selected;
    ListView_GetItem(gList, &item);
    return static_cast<DWORD>(item.lParam);
}

void CreateMiniDump(HWND owner)
{
    DWORD pid = SelectedPid();
    if (!pid)
    {
        MessageBoxW(owner, L"Select a process first.", L"MiniDump", MB_OK | MB_ICONINFORMATION);
        return;
    }

    HANDLE process = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process)
    {
        MessageBoxW(owner, L"OpenProcess failed. Try a process owned by the current user.",
            L"MiniDump", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t temp[MAX_PATH]{};
    wchar_t path[MAX_PATH]{};
    GetTempPathW(ARRAYSIZE(temp), temp);
    StringCchPrintfW(path, ARRAYSIZE(path), L"%sWin32Roadmap_%lu.dmp", temp, pid);

    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    BOOL ok = FALSE;
    if (file != INVALID_HANDLE_VALUE)
    {
        ok = MiniDumpWriteDump(process, pid, file, MiniDumpNormal,
            nullptr, nullptr, nullptr);
        CloseHandle(file);
    }
    CloseHandle(process);

    wchar_t message[MAX_PATH + 128]{};
    StringCchPrintfW(message, ARRAYSIZE(message),
        ok ? L"Dump written:\r\n%s" : L"MiniDumpWriteDump failed:\r\n%s", path);
    MessageBoxW(owner, message, L"MiniDump",
        ok ? MB_OK : MB_OK | MB_ICONWARNING);
}

void KillSelectedProcess(HWND owner)
{
    DWORD pid = SelectedPid();
    if (!pid)
    {
        MessageBoxW(owner, L"Select a process first.", L"Kill Process", MB_OK | MB_ICONINFORMATION);
        return;
    }

    wchar_t confirm[128]{};
    StringCchPrintfW(confirm, ARRAYSIZE(confirm),
        L"Terminate process %lu?", pid);
    if (MessageBoxW(owner, confirm, L"Kill Process",
        MB_YESNO | MB_ICONWARNING) != IDYES)
        return;

    HANDLE proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!proc)
    {
        MessageBoxW(owner, L"OpenProcess(TERMINATE) failed.", L"Kill Process",
            MB_OK | MB_ICONERROR);
        return;
    }
    BOOL ok = TerminateProcess(proc, 1);
    CloseHandle(proc);
    if (ok)
        RefreshProcesses();
    else
        MessageBoxW(owner, L"TerminateProcess failed.", L"Kill Process", MB_OK | MB_ICONERROR);
}

void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    SendMessageW(gStatus, WM_SIZE, 0, 0);

    RECT statusRc{};
    GetWindowRect(gStatus, &statusRc);
    int statusH = statusRc.bottom - statusRc.top;

    MoveWindow(GetDlgItem(hwnd, IDC_REFRESH), 12, 12, 100, 32, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_DUMP),    124, 12, 110, 32, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_KILL),    246, 12, 110, 32, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_FILTER),  370, 14, 220, 28, TRUE);
    CreateWindowW(L"STATIC", nullptr, WS_CHILD, 0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
    MoveWindow(gList, 12, 56, rc.right - 24, rc.bottom - statusH - 68, TRUE);
}

void CreateControls(HWND hwnd, HINSTANCE instance)
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    CreateWindowW(L"BUTTON", L"Refresh",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_REFRESH), instance, nullptr);
    CreateWindowW(L"BUTTON", L"MiniDump",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_DUMP), instance, nullptr);
    CreateWindowW(L"BUTTON", L"Kill Process",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, ControlId(IDC_KILL), instance, nullptr);
    gFilter = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd, ControlId(IDC_FILTER), instance, nullptr);

    gList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 0, 0, hwnd, ControlId(IDC_LIST), instance, nullptr);
    gStatus = CreateWindowExW(0, STATUSCLASSNAMEW, L"Ready",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, ControlId(IDC_STATUS), instance, nullptr);

    ListView_SetExtendedListViewStyle(gList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    AddColumn(ColPID,     80,  L"PID");
    AddColumn(ColParent,  80,  L"Parent");
    AddColumn(ColThreads, 70,  L"Threads");
    AddColumn(ColSession, 60,  L"Session");
    AddColumn(ColWS_KB,   90,  L"WS (KB)");
    AddColumn(ColPriority,90,  L"Priority");
    AddColumn(ColImage,   320, L"Image");

    Layout(hwnd);
    RefreshProcesses();
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
        {
            RefreshProcesses();
            return 0;
        }
        if (LOWORD(wParam) == IDC_DUMP)
        {
            CreateMiniDump(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == IDC_KILL)
        {
            KillSelectedProcess(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == IDC_FILTER && HIWORD(wParam) == EN_CHANGE)
        {
            RefreshProcesses();
            return 0;
        }
        return 0;

    case WM_NOTIFY:
        if (reinterpret_cast<NMHDR*>(lParam)->idFrom == IDC_LIST)
        {
            DWORD pid = SelectedPid();
            if (pid)
            {
                wchar_t status[128]{};
                StringCchPrintfW(status, ARRAYSIZE(status), L"Selected PID: %lu", pid);
                SetStatus(status);
            }
        }
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"40 ProcessExplorer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 940, 620,
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
