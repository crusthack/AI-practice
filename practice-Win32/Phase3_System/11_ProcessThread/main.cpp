#include <Windows.h>
#include <process.h>
#include <TlHelp32.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_ProcessThread";
constexpr int IDC_RUN  = 1001;
constexpr int IDC_TEXT = 1002;

HWND gText{};
LONG gInterlockedCounter{};

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

// Work item for QueueUserWorkItem
DWORD CALLBACK WorkItem(void*)
{
    InterlockedIncrement(&gInterlockedCounter);
    return 0;
}

// Worker thread: create child process and capture stdout
unsigned __stdcall Worker(void* param)
{
    wchar_t* out = static_cast<wchar_t*>(param);

    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE readPipe{}, writePipe{};
    CreatePipe(&readPipe, &writePipe, &sa, 0);
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{ sizeof(si) };
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError  = writePipe;
    PROCESS_INFORMATION pi{};
    wchar_t cmd[] = L"cmd.exe /c echo Child stdout captured via anonymous pipe";

    if (CreateProcessW(nullptr, cmd, nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(writePipe);
        char bytes[256]{};
        DWORD count{};
        ReadFile(readPipe, bytes, sizeof(bytes) - 1, &count, nullptr);
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode{};
        GetExitCodeProcess(pi.hProcess, &exitCode);

        wchar_t line[256]{};
        MultiByteToWideChar(CP_ACP, 0, bytes, -1, line, ARRAYSIZE(line));
        Append(out, 4096, L"[CreateProcess] stdout: %s", line);
        Append(out, 4096, L"[CreateProcess] exit code: %lu", exitCode);
        Append(out, 4096, L"[WorkerThread]  thread id: %lu", GetCurrentThreadId());

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else
    {
        Append(out, 4096, L"[CreateProcess] FAILED (error %lu)", GetLastError());
        CloseHandle(writePipe);
    }

    CloseHandle(readPipe);
    SetWindowTextW(gText, out);
    return 0;
}

void RunDemo()
{
    static wchar_t out[4096];
    StringCchCopyW(out, ARRAYSIZE(out), L"=== 11_ProcessThread ===\r\n");

    // --- Current process / thread info ---
    Append(out, ARRAYSIZE(out), L"[Process] PID=%lu  Parent=%lu  MainThread=%lu",
        GetCurrentProcessId(),
        GetCurrentProcessId(), // we don't enumerate parent here for brevity
        GetCurrentThreadId());
    Append(out, ARRAYSIZE(out), L"[Process] Priority class: %lu", GetPriorityClass(GetCurrentProcess()));

    // --- Toolhelp32: count processes ---
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe{ sizeof(pe) };
        int total = 0;
        if (Process32FirstW(snap, &pe))
            do { ++total; } while (Process32NextW(snap, &pe));
        CloseHandle(snap);
        Append(out, ARRAYSIZE(out), L"[Toolhelp32] Running processes: %d", total);
    }

    // --- Thread count for current process via Toolhelp32 ---
    HANDLE snapT = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapT != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te{ sizeof(te) };
        int myThreads = 0;
        DWORD pid = GetCurrentProcessId();
        if (Thread32First(snapT, &te))
            do { if (te.th32OwnerProcessID == pid) ++myThreads; } while (Thread32Next(snapT, &te));
        CloseHandle(snapT);
        Append(out, ARRAYSIZE(out), L"[Toolhelp32] Threads in this process: %d", myThreads);
    }

    // --- Interlocked ---
    gInterlockedCounter = 0;
    for (int i = 0; i < 50; ++i)
        InterlockedIncrement(&gInterlockedCounter);
    Append(out, ARRAYSIZE(out), L"[Interlocked] InterlockedIncrement x50 = %ld", gInterlockedCounter);

    InterlockedExchange(&gInterlockedCounter, 42);
    Append(out, ARRAYSIZE(out), L"[Interlocked] InterlockedExchange  -> %ld", gInterlockedCounter);

    LONG old = InterlockedCompareExchange(&gInterlockedCounter, 999, 42);
    Append(out, ARRAYSIZE(out), L"[Interlocked] CAS(42->999) old=%ld new=%ld", old, gInterlockedCounter);

    // --- QueueUserWorkItem (thread pool) ---
    gInterlockedCounter = 0;
    for (int i = 0; i < 8; ++i)
        QueueUserWorkItem(WorkItem, nullptr, WT_EXECUTEDEFAULT);
    Sleep(200); // allow pool threads to run
    Append(out, ARRAYSIZE(out), L"[ThreadPool]  QueueUserWorkItem x8, counter=%ld (expect 8)", gInterlockedCounter);

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[Worker thread starting - captures child process stdout]");

    // Worker thread does CreateProcess + stdout capture asynchronously
    uintptr_t thread = _beginthreadex(nullptr, 0, Worker, out, 0, nullptr);
    if (thread)
        CloseHandle(reinterpret_cast<HANDLE>(thread));

    // Note: Worker calls SetWindowTextW when done
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        CreateWindowW(L"BUTTON", L"Run Process/Thread Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 240, 32,
            hwnd, ControlId(IDC_RUN), inst, nullptr);
        gText = CreateWindowW(L"EDIT", L"Click the button.",
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
        MoveWindow(gText, 20, 68, rc.right - 40, rc.bottom - 84, TRUE);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RUN)
            RunDemo();
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"11 ProcessThread",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 540,
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
