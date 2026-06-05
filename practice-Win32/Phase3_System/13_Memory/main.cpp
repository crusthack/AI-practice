#include <Windows.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_Memory";
constexpr int IDC_RUN  = 1001;
constexpr int IDC_TEXT = 1002;

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

void RunDemo(HWND textCtrl)
{
    wchar_t out[4096] = L"=== 13_Memory ===\r\n";

    // --- System info ---
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    Append(out, ARRAYSIZE(out), L"[System] Page size: %lu B  Alloc granularity: %lu B",
        si.dwPageSize, si.dwAllocationGranularity);
    Append(out, ARRAYSIZE(out), L"[System] User-mode address range: %p - %p",
        si.lpMinimumApplicationAddress, si.lpMaximumApplicationAddress);

    // --- VirtualAlloc / VirtualQuery / VirtualProtect ---
    {
        SIZE_T allocSize = si.dwPageSize * 3;
        void* mem = VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        Append(out, ARRAYSIZE(out), L"[VirtualAlloc] %zu bytes at %p", allocSize, mem);

        MEMORY_BASIC_INFORMATION mbi{};
        VirtualQuery(mem, &mbi, sizeof(mbi));
        Append(out, ARRAYSIZE(out), L"[VirtualQuery] State=0x%lX  Protect=0x%lX  Size=%zu",
            mbi.State, mbi.Protect, mbi.RegionSize);

        // Change first page to read-only
        DWORD oldProt{};
        BOOL ok = VirtualProtect(mem, si.dwPageSize, PAGE_READONLY, &oldProt);
        Append(out, ARRAYSIZE(out), L"[VirtualProtect] PAGE_READWRITE->PAGE_READONLY: %s (old=0x%lX)",
            ok ? L"OK" : L"FAILED", oldProt);

        // Restore
        VirtualProtect(mem, si.dwPageSize, PAGE_READWRITE, &oldProt);

        // Write across all 3 pages
        auto* bytes = static_cast<BYTE*>(mem);
        for (SIZE_T i = 0; i < allocSize; ++i)
            bytes[i] = static_cast<BYTE>(i & 0xFF);
        BYTE sample = bytes[si.dwPageSize * 2 + 1];
        Append(out, ARRAYSIZE(out), L"[VirtualAlloc] Written all pages. Sample byte at page3+1: 0x%02X", sample);

        VirtualFree(mem, 0, MEM_RELEASE);
        Append(out, ARRAYSIZE(out), L"[VirtualFree] Released");
    }

    // --- HeapCreate / HeapAlloc / HeapReAlloc / HeapSize / HeapFree ---
    {
        HANDLE heap = HeapCreate(0, 4096, 0);
        void* block  = HeapAlloc(heap, HEAP_ZERO_MEMORY, 256);
        SIZE_T sz    = HeapSize(heap, 0, block);
        void* block2 = HeapReAlloc(heap, HEAP_ZERO_MEMORY, block, 512);
        SIZE_T sz2   = HeapSize(heap, 0, block2);
        Append(out, ARRAYSIZE(out), L"[HeapCreate] Private heap at %p", heap);
        Append(out, ARRAYSIZE(out), L"[HeapAlloc]  256 B -> HeapSize=%zu  (at %p)", sz, block2);
        Append(out, ARRAYSIZE(out), L"[HeapReAlloc] 512 B -> HeapSize=%zu", sz2);
        HeapFree(heap, 0, block2);
        HeapDestroy(heap);
        Append(out, ARRAYSIZE(out), L"[HeapDestroy] Done");
    }

    // --- Named file mapping (shared memory) ---
    {
        const wchar_t kName[] = L"Local\\Win32RoadmapMemDemo";
        HANDLE mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
            PAGE_READWRITE, 0, 4096, kName);
        wchar_t* view1 = static_cast<wchar_t*>(
            MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
        StringCchCopyW(view1, 128, L"Shared memory between two MapViewOfFile calls");

        // Second view of the same mapping
        wchar_t* view2 = static_cast<wchar_t*>(
            MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0));
        Append(out, ARRAYSIZE(out), L"[MemMapping] view1 wrote: \"%s\"", view1);
        Append(out, ARRAYSIZE(out), L"[MemMapping] view2 read:  \"%s\"", view2);

        UnmapViewOfFile(view2);
        UnmapViewOfFile(view1);
        CloseHandle(mapping);
    }

    // --- Process working set ---
    {
        SIZE_T wsMin{}, wsMax{};
        GetProcessWorkingSetSize(GetCurrentProcess(), &wsMin, &wsMax);
        Append(out, ARRAYSIZE(out), L"[WorkingSet] Min=%zu KB  Max=%zu KB",
            wsMin / 1024, wsMax / 1024);
    }

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs: VirtualAlloc, VirtualFree, VirtualQuery, VirtualProtect");
    Append(out, ARRAYSIZE(out), L"      HeapCreate, HeapAlloc, HeapReAlloc, HeapSize, HeapFree, HeapDestroy");
    Append(out, ARRAYSIZE(out), L"      CreateFileMappingW, MapViewOfFile, UnmapViewOfFile");
    Append(out, ARRAYSIZE(out), L"      GetSystemInfo, GetProcessWorkingSetSize");

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
        CreateWindowW(L"BUTTON", L"Run Memory Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 200, 32,
            hwnd, ControlId(IDC_RUN), inst, nullptr);
        text = CreateWindowW(L"EDIT", L"Click the button.",
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"13 Memory",
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
