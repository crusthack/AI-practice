#include <Windows.h>
#include <process.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_Synchronization";
constexpr int IDC_RUN  = 1001;
constexpr int IDC_TEXT = 1002;

HWND gText{};

// Shared state for producer/consumer
CRITICAL_SECTION    gCS;
CONDITION_VARIABLE  gCV;
int  gItems{};
bool gDone{};
wchar_t gLog[4096]{};

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

void Log(const wchar_t* text)
{
    StringCchCatW(gLog, ARRAYSIZE(gLog), text);
    StringCchCatW(gLog, ARRAYSIZE(gLog), L"\r\n");
}

// --- Producer/Consumer threads ---
unsigned __stdcall Producer(void*)
{
    for (int i = 0; i < 4; ++i)
    {
        Sleep(80);
        EnterCriticalSection(&gCS);
        ++gItems;
        wchar_t line[64]{};
        StringCchPrintfW(line, ARRAYSIZE(line), L"  Producer: item %d added (queue=%d)", i + 1, gItems);
        Log(line);
        WakeConditionVariable(&gCV);
        LeaveCriticalSection(&gCS);
    }
    EnterCriticalSection(&gCS);
    gDone = true;
    WakeAllConditionVariable(&gCV);
    LeaveCriticalSection(&gCS);
    return 0;
}

unsigned __stdcall Consumer(void*)
{
    for (;;)
    {
        EnterCriticalSection(&gCS);
        while (gItems == 0 && !gDone)
            SleepConditionVariableCS(&gCV, &gCS, INFINITE);
        if (gItems == 0 && gDone)
        {
            LeaveCriticalSection(&gCS);
            break;
        }
        --gItems;
        wchar_t line[64]{};
        StringCchPrintfW(line, ARRAYSIZE(line), L"  Consumer: item consumed (queue=%d)", gItems);
        Log(line);
        LeaveCriticalSection(&gCS);
        Sleep(60);
    }
    SetWindowTextW(gText, gLog);
    return 0;
}

void RunDemo()
{
    wchar_t out[4096] = L"=== 12_Synchronization ===\r\n";

    // --- Named Mutex ---
    {
        HANDLE m = CreateMutexW(nullptr, FALSE, L"Local\\Win32RoadmapMutexDemo");
        DWORD wait = WaitForSingleObject(m, 1000);
        if (wait == WAIT_OBJECT_0)
        {
            StringCchCatW(out, ARRAYSIZE(out), L"[Mutex] Named mutex acquired\r\n");
            ReleaseMutex(m);
            StringCchCatW(out, ARRAYSIZE(out), L"[Mutex] Released\r\n");
        }
        CloseHandle(m);
    }

    // --- Manual-reset Event ---
    {
        HANDLE e = CreateEventW(nullptr, TRUE, FALSE, L"Local\\Win32RoadmapEventDemo");
        SetEvent(e);
        DWORD wait = WaitForSingleObject(e, 0); // should be signaled immediately
        StringCchCatW(out, ARRAYSIZE(out),
            wait == WAIT_OBJECT_0 ? L"[Event] Manual-reset set/wait: OK\r\n"
                                  : L"[Event] Wait failed\r\n");
        ResetEvent(e);
        wait = WaitForSingleObject(e, 0); // should time out (not signaled)
        StringCchCatW(out, ARRAYSIZE(out),
            wait == WAIT_TIMEOUT ? L"[Event] After ResetEvent, WaitForSingleObject=TIMEOUT: OK\r\n"
                                 : L"[Event] Unexpected signal\r\n");
        CloseHandle(e);
    }

    // --- Semaphore ---
    {
        HANDLE sem = CreateSemaphoreW(nullptr, 3, 3, L"Local\\Win32RoadmapSemDemo");
        DWORD r1 = WaitForSingleObject(sem, 0); // acquire slot 1
        DWORD r2 = WaitForSingleObject(sem, 0); // acquire slot 2
        DWORD r3 = WaitForSingleObject(sem, 0); // acquire slot 3
        DWORD r4 = WaitForSingleObject(sem, 0); // 0 slots left → timeout
        wchar_t line[128]{};
        StringCchPrintfW(line, ARRAYSIZE(line),
            L"[Semaphore] Acquire x3: %s %s %s  4th(timeout): %s\r\n",
            r1 == WAIT_OBJECT_0 ? L"OK" : L"FAIL",
            r2 == WAIT_OBJECT_0 ? L"OK" : L"FAIL",
            r3 == WAIT_OBJECT_0 ? L"OK" : L"FAIL",
            r4 == WAIT_TIMEOUT  ? L"TIMEOUT" : L"FAIL");
        StringCchCatW(out, ARRAYSIZE(out), line);
        LONG prev{};
        ReleaseSemaphore(sem, 3, &prev);
        CloseHandle(sem);
    }

    // --- WaitForMultipleObjects ---
    {
        HANDLE e1 = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        HANDLE e2 = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        SetEvent(e1);
        SetEvent(e2);
        HANDLE handles[2] = { e1, e2 };
        DWORD idx = WaitForMultipleObjects(2, handles, TRUE, 0); // wait for ALL
        StringCchCatW(out, ARRAYSIZE(out),
            idx == WAIT_OBJECT_0 ? L"[WaitMultiple] WaitForMultipleObjects(ALL): OK\r\n"
                                 : L"[WaitMultiple] Unexpected result\r\n");
        CloseHandle(e1);
        CloseHandle(e2);
    }

    // --- Interlocked operations ---
    {
        LONG val = 0;
        InterlockedExchange(&val, 100);
        wchar_t line[128]{};
        StringCchPrintfW(line, ARRAYSIZE(line), L"[Interlocked] Exchange->100, Inc->%ld, Dec->%ld\r\n",
            InterlockedIncrement(&val), InterlockedDecrement(&val));
        StringCchCatW(out, ARRAYSIZE(out), line);

        LONG old = InterlockedCompareExchange(&val, 999, 100); // CAS: if val==100, set 999
        StringCchPrintfW(line, ARRAYSIZE(line),
            L"[Interlocked] CAS(100->999): old=%ld, val=%ld\r\n", old, val);
        StringCchCatW(out, ARRAYSIZE(out), line);
    }

    // --- Producer/Consumer (CriticalSection + ConditionVariable) ---
    StringCchCatW(out, ARRAYSIZE(out), L"\r\n[CS+CV] Producer/Consumer (4 items):\r\n");
    StringCchCopyW(gLog, ARRAYSIZE(gLog), out);
    gItems = 0;
    gDone  = false;

    HANDLE threads[2] = {
        reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Producer, nullptr, 0, nullptr)),
        reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Consumer, nullptr, 0, nullptr))
    };
    // Close immediately; Consumer calls SetWindowTextW when done
    for (HANDLE h : threads)
        if (h) CloseHandle(h);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        InitializeCriticalSection(&gCS);
        InitializeConditionVariable(&gCV);
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        CreateWindowW(L"BUTTON", L"Run Synchronization Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 260, 32,
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
        DeleteCriticalSection(&gCS);
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"12 Synchronization",
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
