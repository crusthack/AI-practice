#include <Windows.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_Fiber_UMS";
constexpr int IDC_RUN  = 1001;
constexpr int IDC_TEXT = 1002;

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

// =============================================
// Cooperative coroutine simulation using Fibers
// =============================================

struct FiberCtx
{
    void*    mainFiber;
    wchar_t* out;
    size_t   outCount;
    int      id;
    int      steps;
    int      yieldCount;
};

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

// Each fiber runs a "task" that yields several times back to the scheduler
void CALLBACK FiberTask(void* param)
{
    FiberCtx* ctx = static_cast<FiberCtx*>(param);

    for (int step = 0; step < ctx->steps; ++step)
    {
        Append(ctx->out, ctx->outCount,
            L"  Fiber[%d] step %d/%d  (tid=%lu)",
            ctx->id, step + 1, ctx->steps, GetCurrentThreadId());
        ++ctx->yieldCount;
        SwitchToFiber(ctx->mainFiber); // yield to scheduler
    }

    Append(ctx->out, ctx->outCount, L"  Fiber[%d] DONE", ctx->id);
    SwitchToFiber(ctx->mainFiber); // final yield
}

void RunDemo(HWND textCtrl)
{
    wchar_t out[4096] = L"=== 37_Fiber_UMS ===\r\n";

    // Convert current thread to a fiber (required to use SwitchToFiber)
    void* mainFiber = ConvertThreadToFiber(nullptr);
    if (!mainFiber)
    {
        // Already a fiber if called twice
        mainFiber = GetCurrentFiber();
    }

    Append(out, ARRAYSIZE(out), L"Main fiber: %p", mainFiber);
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[Scheduler] Creating 3 fibers (cooperative multitasking):");

    // Create 3 fibers with different step counts
    FiberCtx ctxs[3]{};
    void*    fibers[3]{};
    for (int i = 0; i < 3; ++i)
    {
        ctxs[i].mainFiber = mainFiber;
        ctxs[i].out       = out;
        ctxs[i].outCount  = ARRAYSIZE(out);
        ctxs[i].id        = i + 1;
        ctxs[i].steps     = i + 2; // 2, 3, 4 steps respectively
        fibers[i] = CreateFiber(0, FiberTask, &ctxs[i]);
    }

    // Round-robin scheduler: run each fiber one step at a time
    Append(out, ARRAYSIZE(out), L"[Scheduler] Round-robin dispatch:");
    bool anyRunning = true;
    int  round = 0;
    while (anyRunning)
    {
        anyRunning = false;
        ++round;
        wchar_t roundLine[64]{};
        StringCchPrintfW(roundLine, ARRAYSIZE(roundLine), L"-- Round %d --", round);
        Append(out, ARRAYSIZE(out), roundLine);

        for (int i = 0; i < 3; ++i)
        {
            if (!fibers[i]) continue;
            anyRunning = true;
            SwitchToFiber(fibers[i]); // run until it yields back

            // Check if fiber is done (all steps used)
            if (ctxs[i].yieldCount >= ctxs[i].steps + 1)
            {
                DeleteFiber(fibers[i]);
                fibers[i] = nullptr;
            }
        }
    }

    // Summary
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[Summary]");
    int totalYields = 0;
    for (int i = 0; i < 3; ++i)
        totalYields += ctxs[i].yieldCount;
    Append(out, ARRAYSIZE(out),
        L"  Rounds: %d  Total yields: %d  Fibers: 3", round, totalYields);

    // Cleanup: convert back to thread (required to keep thread/fiber balance)
    ConvertFiberToThread();
    Append(out, ARRAYSIZE(out), L"  ConvertFiberToThread: OK");

    // =============================================
    // Demonstrate GetFiberData / FLS (Fiber Local Storage)
    // =============================================
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[FLS - Fiber Local Storage]");

    void* mainFiber2 = ConvertThreadToFiber(reinterpret_cast<void*>(0x1234));

    DWORD flsIndex = FlsAlloc(nullptr);
    FlsSetValue(flsIndex, reinterpret_cast<void*>(0xBEEF));
    void* val = FlsGetValue(flsIndex);
    Append(out, ARRAYSIZE(out), L"  FlsAlloc index=%lu  FlsSetValue(0xBEEF)  FlsGetValue=0x%IX",
        flsIndex, reinterpret_cast<ULONG_PTR>(val));
    FlsFree(flsIndex);

    void* fiberData = GetFiberData();
    Append(out, ARRAYSIZE(out), L"  GetFiberData (set at convert) = 0x%IX",
        reinterpret_cast<ULONG_PTR>(fiberData));

    ConvertFiberToThread();

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs: ConvertThreadToFiber, CreateFiber, SwitchToFiber");
    Append(out, ARRAYSIZE(out), L"      DeleteFiber, ConvertFiberToThread, GetFiberData");
    Append(out, ARRAYSIZE(out), L"      FlsAlloc, FlsSetValue, FlsGetValue, FlsFree");

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
        CreateWindowW(L"BUTTON", L"Run Fiber Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 180, 32,
            hwnd, ControlId(IDC_RUN), inst, nullptr);
        text = CreateWindowW(L"EDIT", L"Click Run Fiber Demo.",
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"37 Fiber - Cooperative Scheduler",
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
