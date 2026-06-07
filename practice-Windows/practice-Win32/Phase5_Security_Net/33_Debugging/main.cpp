// 33_Debugging — StackWalk, SEH/VEH, Process Inspector, Memory Map, Heap
// Modes: Stack Walk, SEH, VEH, Module Inspector, Memory Map, Heap, Debug Monitor
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <dbghelp.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

enum {
    IDC_BTN_STACK   = 101,
    IDC_BTN_SEH     = 102,
    IDC_BTN_VEH     = 103,
    IDC_BTN_MODULES = 104,
    IDC_BTN_MEMMAP  = 105,
    IDC_BTN_HEAP    = 106,
    IDC_BTN_DBGMON  = 107,
    IDC_LISTVIEW    = 108,
    IDC_EDIT_OUT    = 109,
    IDC_STATUS      = 110,
};

HWND      g_hwnd      = nullptr;
HWND      g_hList     = nullptr;
HWND      g_hEdit     = nullptr;
HWND      g_hStatus   = nullptr;
HINSTANCE g_hInst     = nullptr;
int       g_itemCount = 0;

// VEH handler handle
PVOID     g_vehHandle = nullptr;
BOOL      g_vehFired  = FALSE;

// ── UI Helpers ───────────────────────────────────────────────────────────────

void StatusSet(const wchar_t* msg) {
    SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)msg);
}
void StatusSetR(const wchar_t* r) {
    SendMessage(g_hStatus, SB_SETTEXT, 1, (LPARAM)r);
}

void ListClear() {
    ListView_DeleteAllItems(g_hList);
    while (ListView_DeleteColumn(g_hList, 0));
    g_itemCount = 0;
}

void ListAddColumn(int idx, const wchar_t* text, int width) {
    LVCOLUMN c = {};
    c.mask     = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    c.iSubItem = idx;
    c.pszText  = const_cast<wchar_t*>(text);
    c.cx       = width;
    ListView_InsertColumn(g_hList, idx, &c);
}

int ListAddRow(const wchar_t* c0, const wchar_t* c1 = L"",
               const wchar_t* c2 = L"", const wchar_t* c3 = L"",
               const wchar_t* c4 = L"") {
    LVITEM lvi = {};
    lvi.mask   = LVIF_TEXT;
    lvi.iItem  = g_itemCount++;
    lvi.pszText = const_cast<wchar_t*>(c0);
    int row = ListView_InsertItem(g_hList, &lvi);
    if (c1[0]) ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(c1));
    if (c2[0]) ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(c2));
    if (c3[0]) ListView_SetItemText(g_hList, row, 3, const_cast<wchar_t*>(c3));
    if (c4[0]) ListView_SetItemText(g_hList, row, 4, const_cast<wchar_t*>(c4));
    return row;
}

void OutClear() { SetWindowTextW(g_hEdit, L""); }

void OutLine(const wchar_t* text) {
    int len = GetWindowTextLengthW(g_hEdit);
    SendMessage(g_hEdit, EM_SETSEL, len, len);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
}

void OutFmt(const wchar_t* fmt, ...) {
    wchar_t buf[1024];
    va_list va; va_start(va, fmt);
    StringCchVPrintfW(buf, 1024, fmt, va);
    va_end(va);
    OutLine(buf);
}

const wchar_t* MemStateStr(DWORD state) {
    switch (state) {
    case MEM_COMMIT:  return L"Commit";
    case MEM_RESERVE: return L"Reserve";
    case MEM_FREE:    return L"Free";
    default:          return L"?";
    }
}

const wchar_t* MemTypeStr(DWORD type) {
    switch (type) {
    case MEM_IMAGE:   return L"Image";
    case MEM_MAPPED:  return L"Mapped";
    case MEM_PRIVATE: return L"Private";
    default:          return L"?";
    }
}

const wchar_t* ProtectStr(DWORD p) {
    switch (p & 0xFF) {
    case PAGE_NOACCESS:          return L"NoAccess";
    case PAGE_READONLY:          return L"Read";
    case PAGE_READWRITE:         return L"RW";
    case PAGE_WRITECOPY:         return L"WC";
    case PAGE_EXECUTE:           return L"Exec";
    case PAGE_EXECUTE_READ:      return L"ExecR";
    case PAGE_EXECUTE_READWRITE: return L"ExecRW";
    case PAGE_EXECUTE_WRITECOPY: return L"ExecWC";
    default:                     return L"?";
    }
}

// ── Mode 1: Stack Walk ────────────────────────────────────────────────────────

__declspec(noinline) void StackWalkInner(int depth);
__declspec(noinline) void StackWalkMid(int depth);

__declspec(noinline) void StackWalkInner(int depth) {
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread  = GetCurrentThread();

    // Initialize DbgHelp
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(hProcess, nullptr, TRUE);

    // Capture context
    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&ctx);

    STACKFRAME64 sf = {};
    sf.AddrPC.Mode    = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
#ifdef _WIN64
    sf.AddrPC.Offset    = ctx.Rip;
    sf.AddrFrame.Offset = ctx.Rbp;
    sf.AddrStack.Offset = ctx.Rsp;
    DWORD machType = IMAGE_FILE_MACHINE_AMD64;
#else
    sf.AddrPC.Offset    = ctx.Eip;
    sf.AddrFrame.Offset = ctx.Ebp;
    sf.AddrStack.Offset = ctx.Esp;
    DWORD machType = IMAGE_FILE_MACHINE_I386;
#endif

    OutLine(L"--- Stack Walk (StackWalk64) ---");

    int frameNum = 0;
    while (StackWalk64(machType, hProcess, hThread, &sf, &ctx,
            nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
        if (sf.AddrPC.Offset == 0) break;
        if (++frameNum > 20) break;

        // Resolve symbol
        BYTE symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen   = MAX_SYM_NAME;
        DWORD64 displacement = 0;
        wchar_t symName[256] = L"(no symbol)";
        if (SymFromAddr(hProcess, sf.AddrPC.Offset, &displacement, sym)) {
            MultiByteToWideChar(CP_ACP, 0, sym->Name, -1, symName, 256);
        }

        // Get line info
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);
        DWORD lineDisp = 0;
        wchar_t fileLine[256] = L"(no src)";
        if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &lineDisp, &line)) {
            wchar_t fileW[256];
            MultiByteToWideChar(CP_ACP, 0, line.FileName, -1, fileW, 256);
            StringCchPrintf(fileLine, 256, L"%s:%u", fileW, line.LineNumber);
        }

        wchar_t addrStr[32], dispStr[32];
        StringCchPrintf(addrStr, 32, L"0x%0*I64X",
            (int)sizeof(void*)*2, sf.AddrPC.Offset);
        StringCchPrintf(dispStr, 32, L"+0x%I64X", displacement);

        ListAddRow(addrStr, symName, dispStr, fileLine, L"");
        OutFmt(L"  #%d %s %s+%I64X", frameNum, symName, addrStr, displacement);
    }

    SymCleanup(hProcess);
}

__declspec(noinline) void StackWalkMid(int depth) {
    StackWalkInner(depth + 1);
}

void ModeStackWalk() {
    ListClear();
    ListAddColumn(0, L"Address",    140);
    ListAddColumn(1, L"Symbol",     220);
    ListAddColumn(2, L"Offset",      80);
    ListAddColumn(3, L"Source",     280);
    ListAddColumn(4, L"Notes",       80);
    OutClear();
    OutLine(L"=== Stack Walk via StackWalk64 + SymFromAddr ===");
    OutLine(L"Calling chain: ModeStackWalk → StackWalkMid → StackWalkInner");
    OutLine(L"");

    StackWalkMid(0);

    StatusSet(L"Stack Walk — StackWalk64 + SymFromAddr + SymGetLineFromAddr64");
    StatusSetR(L"Done");
}

// ── Mode 2: SEH Demo ──────────────────────────────────────────────────────────

void ModeSEH() {
    ListClear();
    ListAddColumn(0, L"Exception",   200);
    ListAddColumn(1, L"Code",        120);
    ListAddColumn(2, L"Handled By",  200);
    ListAddColumn(3, L"Address",     160);
    OutClear();
    OutLine(L"=== Structured Exception Handling (SEH) ===");

    // Demo 1: Divide by zero
    OutLine(L"--- Test 1: Divide by Zero ---");
    __try {
        volatile int x = 0;
        volatile int y = 1 / x; // trigger EXCEPTION_INT_DIVIDE_BY_ZERO
        (void)y;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        wchar_t codeStr[32], excName[128];
        StringCchPrintf(codeStr, 32, L"0x%08X", code);
        StringCchPrintf(excName, 128, L"EXCEPTION_INT_DIVIDE_BY_ZERO");
        ListAddRow(excName, codeStr, L"__except(EXECUTE_HANDLER)", L"SEH frame");
        OutFmt(L"Caught: code=0x%08X EXCEPTION_INT_DIVIDE_BY_ZERO", code);
    }

    // Demo 2: Access violation (write to nullptr)
    OutLine(L"--- Test 2: Access Violation ---");
    __try {
        volatile DWORD* p = nullptr;
        *p = 0xDEADBEEF; // write to null
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        wchar_t codeStr[32];
        StringCchPrintf(codeStr, 32, L"0x%08X", code);
        ListAddRow(L"EXCEPTION_ACCESS_VIOLATION", codeStr,
            L"__except(EXECUTE_HANDLER)", L"Write to NULL");
        OutFmt(L"Caught: code=0x%08X EXCEPTION_ACCESS_VIOLATION", code);
    }

    // Demo 3: GetExceptionInformation
    OutLine(L"--- Test 3: GetExceptionInformation ---");
    __try {
        volatile DWORD* p = nullptr;
        volatile DWORD v = *p; // read from null
        (void)v;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        wchar_t codeStr[32];
        StringCchPrintf(codeStr, 32, L"0x%08X", code);
        ListAddRow(L"AV (read)", codeStr,
            L"GetExceptionCode()", L"Read from NULL");
        OutFmt(L"GetExceptionCode() = 0x%08X", code);
    }

    // Demo 4: Stack overflow (partial)
    OutLine(L"--- Test 4: __try/__finally ---");
    __try {
        OutLine(L"In __try block");
        // Intentionally trigger an exception but handle in __finally
        __try {
            RaiseException(0xE0000001, 0, 0, nullptr);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            OutFmt(L"Inner __except: caught 0x%08X", GetExceptionCode());
        }
    } __finally {
        OutLine(L"__finally block always executes (even on exception)");
        ListAddRow(L"__finally", L"Always runs", L"try/__finally", L"Cleanup guarantee");
    }

    // Demo 5: RaiseException custom
    OutLine(L"--- Test 5: RaiseException (custom code) ---");
    __try {
        ULONG_PTR args[2] = {0x1234, 0x5678};
        RaiseException(0xC0000001, 0, 2, args);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        wchar_t codeStr[32];
        StringCchPrintf(codeStr, 32, L"0x%08X", code);
        ListAddRow(L"Custom (0xC0000001)", codeStr,
            L"RaiseException", L"Custom code");
        OutFmt(L"RaiseException custom: 0x%08X", code);
    }

    StatusSet(L"SEH — __try/__except/__finally + RaiseException + GetExceptionCode");
    StatusSetR(L"Done");
}

// ── Mode 3: VEH Demo ─────────────────────────────────────────────────────────

LONG WINAPI VehHandler(EXCEPTION_POINTERS* ep) {
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        g_vehFired = TRUE;
        // Let the process continue normally (we'll use __try to catch it)
        return EXCEPTION_CONTINUE_SEARCH; // pass to SEH
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void ModeVEH() {
    ListClear();
    ListAddColumn(0, L"Property",    200);
    ListAddColumn(1, L"Value",       300);
    ListAddColumn(2, L"Notes",       240);
    OutClear();
    OutLine(L"=== Vectored Exception Handler (VEH) ===");

    // Remove old handler if any
    if (g_vehHandle) {
        RemoveVectoredExceptionHandler(g_vehHandle);
        g_vehHandle = nullptr;
    }

    // Register VEH
    g_vehFired  = FALSE;
    g_vehHandle = AddVectoredExceptionHandler(1, VehHandler); // 1=first in chain
    if (g_vehHandle) {
        ListAddRow(L"AddVectoredExceptionHandler", L"OK",
            L"First in chain (1), EXCEPTION_ACCESS_VIOLATION");
        OutLine(L"VEH registered (first in chain)");
    }

    // Trigger exception — VEH fires before SEH
    OutLine(L"Triggering access violation...");
    __try {
        volatile DWORD* p = nullptr;
        volatile DWORD v = *p;
        (void)v;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        wchar_t codeStr[32];
        StringCchPrintf(codeStr, 32, L"0x%08X", code);
        ListAddRow(L"VEH fired?", g_vehFired ? L"YES" : L"NO",
            L"Before SEH __except");
        ListAddRow(L"SEH caught", codeStr, L"After VEH returned CONTINUE_SEARCH");
        OutFmt(L"VEH fired: %s | SEH caught: 0x%08X",
            g_vehFired ? L"YES" : L"NO", code);
    }

    // VEH continue execution demo
    OutLine(L"--- VEH CONTINUE_EXECUTION demo ---");
    // Use a dedicated handler that fixes the fault
    PVOID hVEH2 = AddVectoredExceptionHandler(1, [](EXCEPTION_POINTERS* ep) -> LONG {
        // This handler just passes through
        return EXCEPTION_CONTINUE_SEARCH;
    });

    ListAddRow(L"Second VEH", L"Registered", L"CONTINUE_SEARCH (pass-through)");

    // Remove handlers
    if (hVEH2) RemoveVectoredExceptionHandler(hVEH2);

    // Continuing execution handler
    OutLine(L"--- VEH Order: First registered with dwFirstHandler=1 takes priority ---");
    ListAddRow(L"VEH Priority", L"dwFirstHandler=1 → earliest",
        L"dwFirstHandler=0 → last");

    // Cleanup
    if (g_vehHandle) {
        RemoveVectoredExceptionHandler(g_vehHandle);
        g_vehHandle = nullptr;
        OutLine(L"VEH removed via RemoveVectoredExceptionHandler");
        ListAddRow(L"RemoveVectoredExceptionHandler", L"OK", L"Handler chain cleaned up");
    }

    StatusSet(L"VEH — AddVectoredExceptionHandler + CONTINUE_SEARCH/EXECUTE_HANDLER ordering");
    StatusSetR(L"Done");
}

// ── Mode 4: Module Inspector ──────────────────────────────────────────────────

void ModeModules() {
    ListClear();
    ListAddColumn(0, L"Module",       180);
    ListAddColumn(1, L"Base",          120);
    ListAddColumn(2, L"Size",           80);
    ListAddColumn(3, L"Version",       130);
    ListAddColumn(4, L"Full Path",     220);
    OutClear();
    OutLine(L"=== Process Module Inspector (EnumProcessModules + GetFileVersionInfo) ===");

    HANDLE hProcess = GetCurrentProcess();
    HMODULE mods[256]; DWORD needed;

    if (!EnumProcessModules(hProcess, mods, sizeof(mods), &needed)) {
        OutFmt(L"EnumProcessModules failed: %u", GetLastError());
        return;
    }

    DWORD modCount = needed / sizeof(HMODULE);
    OutFmt(L"EnumProcessModules: %u modules loaded", modCount);

    for (DWORD i = 0; i < modCount && i < 30; i++) {
        wchar_t fullPath[MAX_PATH] = {};
        GetModuleFileNameExW(hProcess, mods[i], fullPath, MAX_PATH);

        MODULEINFO mi = {};
        GetModuleInformation(hProcess, mods[i], &mi, sizeof(mi));

        wchar_t base[24], size[24];
        StringCchPrintf(base, 24, L"0x%p", mi.lpBaseOfDll);
        StringCchPrintf(size, 24, L"%u KB", mi.SizeOfImage / 1024);

        // Version info
        wchar_t version[64] = L"(n/a)";
        DWORD verSize = GetFileVersionInfoSizeW(fullPath, nullptr);
        if (verSize > 0) {
            BYTE* verBuf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, verSize);
            if (verBuf && GetFileVersionInfoW(fullPath, 0, verSize, verBuf)) {
                VS_FIXEDFILEINFO* ffi = nullptr;
                UINT ffiLen = 0;
                if (VerQueryValueW(verBuf, L"\\", (PVOID*)&ffi, &ffiLen) && ffi) {
                    StringCchPrintf(version, 64, L"%u.%u.%u.%u",
                        HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
                        HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
                }
            }
            HeapFree(GetProcessHeap(), 0, verBuf);
        }

        // Module name (basename)
        wchar_t* name = wcsrchr(fullPath, L'\\');
        name = name ? name + 1 : fullPath;

        ListAddRow(name, base, size, version, fullPath);
        OutFmt(L"  [%02u] %s base=%s size=%s ver=%s",
            i, name, base, size, version);
    }

    if (modCount > 30) OutFmt(L"  ... (%u more modules not shown)", modCount - 30);

    StatusSet(L"Modules — EnumProcessModules + GetModuleInformation + GetFileVersionInfo");
    wchar_t cnt[32]; StringCchPrintf(cnt, 32, L"Modules: %u", modCount);
    StatusSetR(cnt);
}

// ── Mode 5: Memory Region Map ─────────────────────────────────────────────────

void ModeMemoryMap() {
    ListClear();
    ListAddColumn(0, L"Address",      130);
    ListAddColumn(1, L"Size",          90);
    ListAddColumn(2, L"State",         70);
    ListAddColumn(3, L"Type",          70);
    ListAddColumn(4, L"Protect",       70);
    OutClear();
    OutLine(L"=== Virtual Address Space Map (VirtualQueryEx) ===");

    HANDLE hProcess = GetCurrentProcess();
    ULONG_PTR addr  = 0;
    SIZE_T  committed = 0, reserved = 0, free = 0;
    int regions = 0;

    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQueryEx(hProcess, (PVOID)addr, &mbi, sizeof(mbi))) {
        wchar_t addrStr[32], sizeStr[32];
        StringCchPrintf(addrStr, 32, L"0x%0*IX",
            (int)sizeof(PVOID)*2, (ULONG_PTR)mbi.BaseAddress);
        if (mbi.RegionSize >= 1024*1024)
            StringCchPrintf(sizeStr, 32, L"%zu MB", mbi.RegionSize / (1024*1024));
        else if (mbi.RegionSize >= 1024)
            StringCchPrintf(sizeStr, 32, L"%zu KB", mbi.RegionSize / 1024);
        else
            StringCchPrintf(sizeStr, 32, L"%zu B", mbi.RegionSize);

        if (regions < 60) { // limit list
            ListAddRow(addrStr, sizeStr,
                MemStateStr(mbi.State),
                mbi.State == MEM_FREE ? L"---" : MemTypeStr(mbi.Type),
                mbi.State == MEM_FREE ? L"---" : ProtectStr(mbi.Protect));
        }

        switch (mbi.State) {
        case MEM_COMMIT:  committed += mbi.RegionSize; break;
        case MEM_RESERVE: reserved  += mbi.RegionSize; break;
        case MEM_FREE:    free      += mbi.RegionSize; break;
        }
        regions++;

        ULONG_PTR next = (ULONG_PTR)mbi.BaseAddress + mbi.RegionSize;
        if (next <= addr) break; // overflow
        addr = next;
    }

    OutFmt(L"Total regions scanned: %d", regions);
    OutFmt(L"Committed:  %.1f MB", (double)committed  / (1024*1024));
    OutFmt(L"Reserved:   %.1f MB", (double)reserved   / (1024*1024));
    OutFmt(L"Free:       %.1f MB", (double)free       / (1024*1024));

    StatusSet(L"Memory Map — VirtualQueryEx walking all VAS regions");
    wchar_t cnt[64];
    StringCchPrintf(cnt, 64, L"Regions: %d | Committed: %.0f MB",
        regions, (double)committed/(1024*1024));
    StatusSetR(cnt);
}

// ── Mode 6: Heap Inspector ────────────────────────────────────────────────────

void ModeHeap() {
    ListClear();
    ListAddColumn(0, L"Heap Handle",  130);
    ListAddColumn(1, L"Busy Blocks",   90);
    ListAddColumn(2, L"Free Blocks",   90);
    ListAddColumn(3, L"Total Bytes",  120);
    ListAddColumn(4, L"Valid?",        60);
    OutClear();
    OutLine(L"=== Heap Inspector — GetProcessHeaps + HeapWalk ===");

    // Enumerate all heaps
    HANDLE heaps[32]; DWORD heapCount;
    heapCount = GetProcessHeaps(32, heaps);
    OutFmt(L"GetProcessHeaps: %u heaps in process", heapCount);

    HANDLE hDefaultHeap = GetProcessHeap();

    for (DWORD h = 0; h < heapCount && h < 8; h++) {
        BOOL isDefault = (heaps[h] == hDefaultHeap);
        BOOL valid     = HeapValidate(heaps[h], 0, nullptr);

        wchar_t handleStr[32];
        StringCchPrintf(handleStr, 32, L"0x%p%s",
            heaps[h], isDefault ? L"(default)" : L"");

        // HeapWalk to count blocks
        DWORD busyBlocks = 0, freeBlocks = 0;
        SIZE_T totalBytes = 0;
        PROCESS_HEAP_ENTRY entry = {};
        int sampleCount = 0;
        OutFmt(L"\r\nHeap[%u] = %p %s:", h, heaps[h],
            isDefault ? L"(default heap)" : L"");

        while (HeapWalk(heaps[h], &entry)) {
            if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
                busyBlocks++;
                totalBytes += entry.cbData;
                // Show first 10 busy blocks of default heap
                if (isDefault && sampleCount < 10) {
                    wchar_t blkAddr[32], blkSize[32];
                    StringCchPrintf(blkAddr, 32, L"0x%p", entry.lpData);
                    StringCchPrintf(blkSize, 32, L"%u", entry.cbData);
                    OutFmt(L"  Busy[%d]: addr=%s size=%u flags=0x%X",
                        sampleCount, blkAddr, entry.cbData, entry.wFlags);
                    sampleCount++;
                }
            } else {
                freeBlocks++;
            }
        }
        DWORD walkErr = GetLastError();
        // HeapWalk returns FALSE + ERROR_NO_MORE_ITEMS when done (normal)
        if (walkErr != ERROR_NO_MORE_ITEMS) {
            OutFmt(L"  HeapWalk ended: err=%u", walkErr);
        }

        wchar_t busyStr[32], freeStr[32], bytesStr[64];
        StringCchPrintf(busyStr, 32, L"%u", busyBlocks);
        StringCchPrintf(freeStr, 32, L"%u", freeBlocks);
        StringCchPrintf(bytesStr, 64, L"%.1f KB",
            (double)totalBytes / 1024.0);

        ListAddRow(handleStr, busyStr, freeStr, bytesStr,
            valid ? L"YES" : L"NO");
        OutFmt(L"  Busy=%u Free=%u Bytes=%.1f KB Valid=%s",
            busyBlocks, freeBlocks, (double)totalBytes/1024.0,
            valid ? L"YES" : L"NO");
    }

    // Demo: allocate/free and observe
    OutLine(L"\r\n--- HeapAlloc/HeapFree demo ---");
    HANDLE hHeap = GetProcessHeap();
    void* p1 = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    void* p2 = HeapAlloc(hHeap, 0, 4096);
    OutFmt(L"HeapAlloc(1024): %p", p1);
    OutFmt(L"HeapAlloc(4096): %p", p2);
    OutFmt(L"HeapSize(p1): %zu bytes", HeapSize(hHeap, 0, p1));
    BOOL heapOk = HeapValidate(hHeap, 0, p1);
    OutFmt(L"HeapValidate(p1): %s", heapOk ? L"VALID" : L"CORRUPT");
    HeapFree(hHeap, 0, p1);
    HeapFree(hHeap, 0, p2);
    OutLine(L"HeapFree done");

    StatusSet(L"Heap — GetProcessHeaps + HeapWalk + HeapValidate + HeapAlloc/Free");
    wchar_t cnt[64];
    StringCchPrintf(cnt, 64, L"Heaps: %u", heapCount);
    StatusSetR(cnt);
}

// ── Mode 7: Debug String Monitor ──────────────────────────────────────────────

// Shared memory layout used by Windows for OutputDebugString
struct DBWIN_BUFFER {
    DWORD dwProcessId;
    char  data[4096 - sizeof(DWORD)];
};

DWORD WINAPI DbgMonWorker(LPVOID param) {
    HWND hEdit = (HWND)param;

    auto AppW = [&](const wchar_t* text) {
        int len = GetWindowTextLengthW(hEdit);
        SendMessage(hEdit, EM_SETSEL, len, len);
        SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
        SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    };
    auto AppF = [&](const wchar_t* fmt, ...) {
        wchar_t buf[512];
        va_list va; va_start(va, fmt);
        StringCchVPrintfW(buf, 512, fmt, va);
        va_end(va);
        AppW(buf);
    };

    AppW(L"[DbgMon] Monitoring OutputDebugString via DBWIN shared memory...");

    // Named objects used by Windows debug output mechanism
    const DWORD DBWIN_BUFFER_READY_TIMEOUT = 2000;

    SECURITY_ATTRIBUTES sa = {};
    SECURITY_DESCRIPTOR sd = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;

    // Create/open shared file mapping
    HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, &sa,
        PAGE_READWRITE, 0, sizeof(DBWIN_BUFFER), L"DBWIN_BUFFER");
    if (!hMap) {
        AppF(L"[DbgMon] CreateFileMapping(DBWIN_BUFFER) failed: %u (already in use by another debugger?)", GetLastError());
        return 1;
    }

    DBWIN_BUFFER* pBuf = (DBWIN_BUFFER*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pBuf) {
        AppF(L"[DbgMon] MapViewOfFile failed: %u", GetLastError());
        CloseHandle(hMap);
        return 1;
    }

    // Events
    HANDLE hReady  = CreateEventW(&sa, FALSE, FALSE, L"DBWIN_BUFFER_READY");
    HANDLE hData   = CreateEventW(&sa, FALSE, FALSE, L"DBWIN_DATA_READY");
    if (!hReady || !hData) {
        AppF(L"[DbgMon] CreateEvent failed: %u", GetLastError());
        UnmapViewOfFile(pBuf);
        CloseHandle(hMap);
        if (hReady) CloseHandle(hReady);
        if (hData)  CloseHandle(hData);
        return 1;
    }

    AppW(L"[DbgMon] Ready. Sending test OutputDebugString messages...");

    // Send our own messages to trigger the monitor
    for (int i = 1; i <= 5; i++) {
        wchar_t wMsg[256];
        StringCchPrintf(wMsg, 256, L"[Win32Learn DbgMon] Test message #%d", i);
        OutputDebugStringW(wMsg);
        // Also send via A version
        char aMsg[256];
        StringCchPrintfA(aMsg, 256, "[Win32Learn DbgMon] Narrow test #%d", i);
        OutputDebugStringA(aMsg);
    }

    // Wait up to 3 seconds for messages
    int caught = 0;
    for (int attempt = 0; attempt < 20 && caught < 3; attempt++) {
        // Signal buffer ready
        SetEvent(hReady);
        // Wait for data
        DWORD waitRes = WaitForSingleObject(hData, 200);
        if (waitRes == WAIT_OBJECT_0) {
            wchar_t dbgMsg[256];
            MultiByteToWideChar(CP_ACP, 0, pBuf->data, -1, dbgMsg, 256);
            AppF(L"[DbgMon] Captured PID=%u: %s",
                pBuf->dwProcessId, dbgMsg);
            caught++;
        }
    }

    if (caught == 0) {
        AppW(L"[DbgMon] No messages captured (no debugger listening, or another process holds the mapping)");
        AppW(L"[DbgMon] OutputDebugString was called; use DebugView or WinDbg to see output.");
    } else {
        AppF(L"[DbgMon] Captured %d debug strings", caught);
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMap);
    CloseHandle(hReady);
    CloseHandle(hData);
    AppW(L"[DbgMon] Monitor stopped.");
    return 0;
}

void ModeDbgMonitor() {
    ListClear();
    ListAddColumn(0, L"Step",       200);
    ListAddColumn(1, L"Details",    400);
    ListAddColumn(2, L"Status",     100);
    OutClear();
    OutLine(L"=== Debug String Monitor (OutputDebugString / DBWIN pattern) ===");

    ListAddRow(L"DBWIN_BUFFER", L"Named file mapping for debug output", L"CreateFileMapping");
    ListAddRow(L"DBWIN_BUFFER_READY", L"Event: listener ready for next msg", L"CreateEvent");
    ListAddRow(L"DBWIN_DATA_READY",   L"Event: data written to buffer",      L"CreateEvent");
    ListAddRow(L"OutputDebugString",  L"5 test messages sent",               L"OutputDebugStringW/A");

    // Also demo OutputDebugString directly
    for (int i = 0; i < 5; i++) {
        wchar_t msg[128];
        StringCchPrintf(msg, 128, L"Win32Learn test debug message %d", i + 1);
        OutputDebugStringW(msg);
    }
    OutLine(L"OutputDebugStringW x5 called (visible in debugger/DebugView)");

    HANDLE hThr = CreateThread(nullptr, 0, DbgMonWorker, g_hEdit, 0, nullptr);
    if (hThr) {
        WaitForSingleObject(hThr, 5000);
        CloseHandle(hThr);
    }

    StatusSet(L"Debug Monitor — DBWIN_BUFFER shared memory + OutputDebugString + CreateEvent");
    StatusSetR(L"Done");
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"Stack Walk", IDC_BTN_STACK},
            {L"SEH",        IDC_BTN_SEH},
            {L"VEH",        IDC_BTN_VEH},
            {L"Modules",    IDC_BTN_MODULES},
            {L"Mem Map",    IDC_BTN_MEMMAP},
            {L"Heap",       IDC_BTN_HEAP},
            {L"DbgMonitor", IDC_BTN_DBGMON},
        };
        for (int i = 0; i < 7; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 120, 8, 110, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        g_hList = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 45, 800, 240, hwnd,
            (HMENU)(UINT_PTR)IDC_LISTVIEW, g_hInst, nullptr);
        ListView_SetExtendedListViewStyle(g_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        SendMessage(g_hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 290, 800, 240, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_OUT, g_hInst, nullptr);
        SendMessage(g_hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hStatus = CreateWindowW(STATUSCLASSNAMEW, L"Ready",
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd,
            (HMENU)(UINT_PTR)IDC_STATUS, g_hInst, nullptr);
        int parts[] = {600, -1};
        SendMessage(g_hStatus, SB_SETPARTS, 2, (LPARAM)parts);
        SendMessage(g_hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
        return 0;
    }

    case WM_SIZE: {
        int W = LOWORD(lp), H = HIWORD(lp);
        SendMessage(g_hStatus, WM_SIZE, 0, 0);
        RECT rs; GetWindowRect(g_hStatus, &rs);
        int sh = rs.bottom - rs.top;
        int parts[] = {W - 200, -1};
        SendMessage(g_hStatus, SB_SETPARTS, 2, (LPARAM)parts);
        int listH = (H - 45 - sh) * 45 / 100;
        int editH = H - 45 - sh - listH;
        SetWindowPos(g_hList, nullptr, 0, 45, W, listH, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 45 + listH, W, editH, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_STACK:   ModeStackWalk();  break;
        case IDC_BTN_SEH:     ModeSEH();        break;
        case IDC_BTN_VEH:     ModeVEH();        break;
        case IDC_BTN_MODULES: ModeModules();    break;
        case IDC_BTN_MEMMAP:  ModeMemoryMap();  break;
        case IDC_BTN_HEAP:    ModeHeap();       break;
        case IDC_BTN_DBGMON:  ModeDbgMonitor(); break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) ModeStackWalk();
        return 0;

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lp;
        if (nm->hwndFrom == g_hList && nm->code == LVN_COLUMNCLICK) {}
        return 0;
    }

    case WM_DESTROY:
        if (g_vehHandle) {
            RemoveVectoredExceptionHandler(g_vehHandle);
            g_vehHandle = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    g_hInst = hInst;
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"Dbg33Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Dbg33Class",
        L"33 — Debugging: StackWalk, SEH/VEH, Process Inspector",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 920, 680,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
