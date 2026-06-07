// 32_ETW_Perf — ETW + PDH Performance Monitor with Scrolling Graph
// Collects CPU%, Memory, Disk, Network; draws live GDI line chart
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <pdh.h>
#include <evntprov.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WM_SAMPLE_READY (WM_APP + 1)

namespace {

enum {
    IDC_BTN_START   = 101,
    IDC_BTN_STOP    = 102,
    IDC_BTN_CLEAR   = 103,
    IDC_COMBO_INTV  = 104,
    IDC_GRAPH       = 105,  // custom paint area
    IDC_STATUS      = 106,
};

// ── Circular buffer of 60 samples ────────────────────────────────────────────
#define SAMPLE_COUNT 60

struct PerfSample {
    double cpu;      // 0-100
    double memMB;    // available MB
    double diskRd;   // bytes/sec (read)
    double netBps;   // bytes/sec
};

PerfSample g_samples[SAMPLE_COUNT] = {};
int        g_sampleHead  = 0;   // next write index
int        g_sampleCount = 0;   // how many filled
double     g_maxMemMB    = 8192.0; // scale for memory

HWND      g_hwnd    = nullptr;
HWND      g_hGraph  = nullptr;
HWND      g_hStatus = nullptr;
HINSTANCE g_hInst   = nullptr;

// PDH
PDH_HQUERY  g_hQuery       = nullptr;
PDH_HCOUNTER g_hCtrCPU    = nullptr;
PDH_HCOUNTER g_hCtrMem    = nullptr;
PDH_HCOUNTER g_hCtrDisk   = nullptr;
PDH_HCOUNTER g_hCtrNet    = nullptr;

// Sampling timer
HANDLE g_hSampleThread = nullptr;
BOOL   g_running       = FALSE;
DWORD  g_intervalMs    = 1000;

// ETW provider GUID (custom, for demo)
// {A74E2C2E-5B27-4A39-9A73-4A3A9C0A4A6E}
static const GUID g_ProviderGuid = {
    0xa74e2c2e, 0x5b27, 0x4a39,
    {0x9a, 0x73, 0x4a, 0x3a, 0x9c, 0x0a, 0x4a, 0x6e}
};
REGHANDLE g_hEtwProvider = 0;

// ── Helpers ──────────────────────────────────────────────────────────────────

void StatusSet(const wchar_t* msg) {
    SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)msg);
}
void StatusSetR(const wchar_t* r) {
    SendMessage(g_hStatus, SB_SETTEXT, 1, (LPARAM)r);
}

// Get double value from PDH counter (formatted)
double PdhGetValue(PDH_HCOUNTER hCtr) {
    PDH_FMT_COUNTERVALUE val = {};
    if (PdhGetFormattedCounterValue(hCtr, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS)
        return val.doubleValue;
    return 0.0;
}

// ── PDH Setup ─────────────────────────────────────────────────────────────────

BOOL SetupPDH() {
    if (g_hQuery) return TRUE;
    if (PdhOpenQueryW(nullptr, 0, &g_hQuery) != ERROR_SUCCESS) return FALSE;

    // CPU %
    PdhAddCounterW(g_hQuery, L"\\Processor(_Total)\\% Processor Time",
        0, &g_hCtrCPU);
    // Available Memory MB
    PdhAddCounterW(g_hQuery, L"\\Memory\\Available MBytes",
        0, &g_hCtrMem);
    // Disk Read Bytes/sec (all disks)
    PdhAddCounterW(g_hQuery,
        L"\\PhysicalDisk(_Total)\\Disk Read Bytes/sec",
        0, &g_hCtrDisk);
    // Network Bytes/sec (total, first interface found)
    PdhAddCounterW(g_hQuery,
        L"\\Network Interface(*)\\Bytes Total/sec",
        0, &g_hCtrNet);

    // First collect to prime
    PdhCollectQueryData(g_hQuery);
    return TRUE;
}

void TeardownPDH() {
    if (g_hQuery) {
        PdhCloseQuery(g_hQuery);
        g_hQuery = nullptr;
        g_hCtrCPU = g_hCtrMem = g_hCtrDisk = g_hCtrNet = nullptr;
    }
}

// ── Sampling Thread ───────────────────────────────────────────────────────────

DWORD WINAPI SampleThread(LPVOID) {
    while (g_running) {
        Sleep(g_intervalMs);
        if (!g_running) break;

        // Collect PDH
        PdhCollectQueryData(g_hQuery);

        double cpu  = PdhGetValue(g_hCtrCPU);
        double mem  = PdhGetValue(g_hCtrMem);
        double disk = PdhGetValue(g_hCtrDisk);
        double net  = PdhGetValue(g_hCtrNet);

        // Store in circular buffer
        g_samples[g_sampleHead] = {cpu, mem, disk, net};
        g_sampleHead = (g_sampleHead + 1) % SAMPLE_COUNT;
        if (g_sampleCount < SAMPLE_COUNT) g_sampleCount++;

        // ETW: write custom event
        if (g_hEtwProvider) {
            EVENT_DESCRIPTOR evtDesc = {};
            evtDesc.Id      = 1;
            evtDesc.Version = 0;
            evtDesc.Channel = 0;
            evtDesc.Level   = 4; // TRACE_LEVEL_INFORMATION
            evtDesc.Opcode  = 0; // EVENT_TRACE_TYPE_INFO
            evtDesc.Task    = 0;
            evtDesc.Keyword = 0;

            struct { double cpu; double mem; double disk; double net; } evtData = {cpu, mem, disk, net};

            EVENT_DATA_DESCRIPTOR dataDesc[1];
            EventDataDescCreate(&dataDesc[0], &evtData, sizeof(evtData));
            EventWrite(g_hEtwProvider, &evtDesc, 1, dataDesc);
        }

        // Notify UI
        PostMessage(g_hwnd, WM_SAMPLE_READY, 0, 0);
    }
    return 0;
}

// ── Graph Drawing ─────────────────────────────────────────────────────────────

void DrawGraph(HDC hdc, RECT* pRect) {
    int W = pRect->right  - pRect->left;
    int H = pRect->bottom - pRect->top;
    if (W <= 0 || H <= 0) return;

    // Fill dark background
    HBRUSH hBrBg = CreateSolidBrush(RGB(20, 20, 30));
    FillRect(hdc, pRect, hBrBg);
    DeleteObject(hBrBg);

    // Margins
    const int ML = 50, MR = 10, MT = 10, MB = 30;
    int GW = W - ML - MR;
    int GH = H - MT - MB;
    if (GW <= 0 || GH <= 0) return;

    // Grid pen (dim)
    HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(60, 60, 80));
    HPEN hOldPen  = (HPEN)SelectObject(hdc, hGridPen);

    // Y-axis grid lines + labels (0%, 25%, 50%, 75%, 100%)
    SetTextColor(hdc, RGB(140, 140, 160));
    SetBkMode(hdc, TRANSPARENT);
    HFONT hSmFont = CreateFontW(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmFont);

    for (int pct = 0; pct <= 100; pct += 25) {
        int y = pRect->top + MT + GH - (int)((double)pct / 100.0 * GH);
        MoveToEx(hdc, pRect->left + ML, y, nullptr);
        LineTo(hdc, pRect->left + ML + GW, y);
        wchar_t lbl[8]; StringCchPrintf(lbl, 8, L"%d%%", pct);
        TextOutW(hdc, pRect->left + 2, y - 6, lbl, (int)wcslen(lbl));
    }

    // X-axis label
    const wchar_t* xLabel = L"60s";
    TextOutW(hdc, pRect->left + ML + GW - 20,
        pRect->top + MT + GH + 8, xLabel, (int)wcslen(xLabel));

    SelectObject(hdc, hOldPen);
    DeleteObject(hGridPen);

    if (g_sampleCount < 2) {
        SelectObject(hdc, hOldFont);
        DeleteObject(hSmFont);
        return;
    }

    // Plot each counter
    const struct {
        COLORREF color;
        const wchar_t* label;
    } series[] = {
        {RGB(0, 220, 80),   L"CPU%"},
        {RGB(80, 160, 255), L"Mem%"},
        {RGB(255, 220, 0),  L"Disk"},
        {RGB(255, 100, 80), L"Net"},
    };

    for (int s = 0; s < 4; s++) {
        HPEN hPen = CreatePen(PS_SOLID, 2, series[s].color);
        SelectObject(hdc, hPen);

        BOOL first = TRUE;
        for (int i = 0; i < g_sampleCount; i++) {
            // Index in circular buffer
            int idx = (g_sampleHead - g_sampleCount + i + SAMPLE_COUNT) % SAMPLE_COUNT;
            double val = 0.0;
            switch (s) {
            case 0: val = g_samples[idx].cpu; break;
            case 1: val = (g_maxMemMB > 0) ?
                (1.0 - g_samples[idx].memMB / g_maxMemMB) * 100.0 : 0; break;
            case 2: val = min(g_samples[idx].diskRd / 10000000.0 * 100.0, 100.0); break;
            case 3: val = min(g_samples[idx].netBps  / 125000000.0 * 100.0, 100.0); break;
            }
            val = max(0.0, min(100.0, val));

            int x = pRect->left + ML + (int)((double)i / (SAMPLE_COUNT - 1) * GW);
            int y = pRect->top + MT + GH - (int)(val / 100.0 * GH);

            if (first) { MoveToEx(hdc, x, y, nullptr); first = FALSE; }
            else        LineTo(hdc, x, y);
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    // Legend
    int legendX = pRect->left + ML + 5;
    int legendY = pRect->top + MT + 5;
    for (int s = 0; s < 4; s++) {
        HBRUSH hBr = CreateSolidBrush(series[s].color);
        RECT lr = {legendX + s*80, legendY, legendX + s*80 + 12, legendY + 12};
        FillRect(hdc, &lr, hBr);
        DeleteObject(hBr);
        SetTextColor(hdc, series[s].color);
        TextOutW(hdc, legendX + s*80 + 15, legendY,
            series[s].label, (int)wcslen(series[s].label));
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hSmFont);
}

// Graph window class proc
LRESULT CALLBACK GraphProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1; // Prevent flicker

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);

        // Double-buffer
        HDC     hMemDC  = CreateCompatibleDC(hdc);
        HBITMAP hBmp    = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);

        DrawGraph(hMemDC, &rc);

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hOldBmp);
        DeleteObject(hBmp);
        DeleteDC(hMemDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ── Start/Stop ────────────────────────────────────────────────────────────────

void StartSampling() {
    if (g_running) return;
    if (!SetupPDH()) { StatusSet(L"PDH setup failed"); return; }

    // ETW register
    EventRegister(&g_ProviderGuid, nullptr, nullptr, &g_hEtwProvider);

    g_running = TRUE;
    g_hSampleThread = CreateThread(nullptr, 0, SampleThread, nullptr, 0, nullptr);
    StatusSet(L"Sampling started — EventRegister + PdhOpenQuery + PdhCollectQueryData");
}

void StopSampling() {
    g_running = FALSE;
    if (g_hSampleThread) {
        WaitForSingleObject(g_hSampleThread, 3000);
        CloseHandle(g_hSampleThread);
        g_hSampleThread = nullptr;
    }
    if (g_hEtwProvider) {
        EventUnregister(g_hEtwProvider);
        g_hEtwProvider = 0;
    }
    StatusSet(L"Sampling stopped");
}

void ClearSamples() {
    ZeroMemory(g_samples, sizeof(g_samples));
    g_sampleHead  = 0;
    g_sampleCount = 0;
    if (g_hGraph) InvalidateRect(g_hGraph, nullptr, FALSE);
    StatusSet(L"Cleared — 60-sample circular buffer reset");
    StatusSetR(L"0 samples");
}

// ── Main WndProc ──────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Buttons
        struct { const wchar_t* label; int id; } btns[] = {
            {L"Start",  IDC_BTN_START},
            {L"Stop",   IDC_BTN_STOP},
            {L"Clear",  IDC_BTN_CLEAR},
        };
        for (int i = 0; i < 3; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 90, 8, 80, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // Interval combo
        HWND hLbl = CreateWindowW(L"STATIC", L"Interval:",
            WS_CHILD | WS_VISIBLE, 285, 14, 65, 18, hwnd, nullptr, g_hInst, nullptr);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hCombo = CreateWindowW(L"COMBOBOX", nullptr,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            355, 10, 100, 120, hwnd,
            (HMENU)(UINT_PTR)IDC_COMBO_INTV, g_hInst, nullptr);
        SendMessage(hCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"500 ms");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"1 sec");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"2 sec");
        SendMessage(hCombo, CB_SETCURSEL, 1, 0); // default 1s

        // Graph panel (custom window)
        WNDCLASSEXW gc = {};
        gc.cbSize       = sizeof(gc);
        gc.lpfnWndProc  = GraphProc;
        gc.hInstance    = g_hInst;
        gc.hbrBackground= (HBRUSH)GetStockObject(BLACK_BRUSH);
        gc.lpszClassName= L"PerfGraph";
        RegisterClassExW(&gc);

        g_hGraph = CreateWindowExW(WS_EX_CLIENTEDGE, L"PerfGraph", nullptr,
            WS_CHILD | WS_VISIBLE,
            0, 45, 800, 450, hwnd,
            (HMENU)(UINT_PTR)IDC_GRAPH, g_hInst, nullptr);

        // Status bar
        g_hStatus = CreateWindowW(STATUSCLASSNAMEW, L"Ready — click Start to begin sampling",
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
        SetWindowPos(g_hGraph, nullptr, 0, 45, W, H - 45 - sh, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);
        int code = HIWORD(wp);
        switch (id) {
        case IDC_BTN_START:
            StartSampling();
            break;
        case IDC_BTN_STOP:
            StopSampling();
            break;
        case IDC_BTN_CLEAR:
            ClearSamples();
            break;
        case IDC_COMBO_INTV:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendMessage(GetDlgItem(hwnd, IDC_COMBO_INTV),
                    CB_GETCURSEL, 0, 0);
                if (sel == 0) g_intervalMs = 500;
                else if (sel == 1) g_intervalMs = 1000;
                else              g_intervalMs = 2000;
            }
            break;
        }
        return 0;
    }

    case WM_SAMPLE_READY: {
        // Update graph
        if (g_hGraph) InvalidateRect(g_hGraph, nullptr, FALSE);

        // Update status bar with latest sample
        if (g_sampleCount > 0) {
            int lastIdx = (g_sampleHead - 1 + SAMPLE_COUNT) % SAMPLE_COUNT;
            PerfSample& s = g_samples[lastIdx];

            // Determine total physical memory (once)
            static double totalMemMB = 0;
            if (totalMemMB == 0) {
                MEMORYSTATUSEX ms = {sizeof(ms)};
                GlobalMemoryStatusEx(&ms);
                totalMemMB = (double)(ms.ullTotalPhys / (1024 * 1024));
                g_maxMemMB = totalMemMB;
            }

            double memUsedPct = totalMemMB > 0 ?
                (1.0 - s.memMB / totalMemMB) * 100.0 : 0.0;

            wchar_t statusMsg[256];
            StringCchPrintf(statusMsg, 256,
                L"CPU: %.1f%%  |  Mem: %.0fMB avail (%.0f%% used)  |  "
                L"Disk: %.1f KB/s  |  Net: %.1f KB/s",
                s.cpu, s.memMB, memUsedPct,
                s.diskRd / 1024.0, s.netBps / 1024.0);
            StatusSet(statusMsg);

            wchar_t right[64];
            StringCchPrintf(right, 64, L"Samples: %d/%d", g_sampleCount, SAMPLE_COUNT);
            StatusSetR(right);
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (wp == VK_F5) StartSampling();
        if (wp == VK_ESCAPE) StopSampling();
        return 0;

    case WM_DESTROY:
        StopSampling();
        TeardownPDH();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    g_hInst = hInst;
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ETWPerf32Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"ETWPerf32Class",
        L"32 — ETW + PDH: Performance Monitor with Graph",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 940, 640,
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
