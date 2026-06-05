// 31_WinHTTP — HTTP Client with Cookies, Download, Connection Reuse
// Modes: GET, POST, Headers, Download, Connection Reuse
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <winhttp.h>
#include <commctrl.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WM_HTTP_DONE (WM_APP + 1)

namespace {

enum {
    IDC_BTN_GET      = 101,
    IDC_BTN_POST     = 102,
    IDC_BTN_HEADERS  = 103,
    IDC_BTN_DOWNLOAD = 104,
    IDC_BTN_CONNREUSE= 105,
    IDC_EDIT_URL     = 106,
    IDC_EDIT_OUT     = 107,
    IDC_STATUS       = 108,
    IDC_LBL_URL      = 109,
};

HWND      g_hwnd    = nullptr;
HWND      g_hEditURL= nullptr;
HWND      g_hEdit   = nullptr;
HWND      g_hStatus = nullptr;
HINSTANCE g_hInst   = nullptr;

// ── UI Helpers ───────────────────────────────────────────────────────────────

void StatusSet(const wchar_t* msg) {
    SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)msg);
}
void StatusSetR(const wchar_t* r) {
    SendMessage(g_hStatus, SB_SETTEXT, 1, (LPARAM)r);
}

void OutClear() { SetWindowTextW(g_hEdit, L""); }

void OutLine(const wchar_t* text) {
    int len = GetWindowTextLengthW(g_hEdit);
    SendMessage(g_hEdit, EM_SETSEL, len, len);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
}

void OutFmt(const wchar_t* fmt, ...) {
    wchar_t buf[2048];
    va_list va; va_start(va, fmt);
    StringCchVPrintfW(buf, 2048, fmt, va);
    va_end(va);
    OutLine(buf);
}

// Parse URL into host + path
BOOL ParseURL(const wchar_t* url, wchar_t* scheme, DWORD schemeCch,
              wchar_t* host, DWORD hostCch,
              wchar_t* path, DWORD pathCch,
              INTERNET_PORT* port, BOOL* useTLS) {
    URL_COMPONENTS uc = {};
    uc.dwStructSize     = sizeof(uc);
    uc.lpszScheme       = scheme;
    uc.dwSchemeLength   = schemeCch;
    uc.lpszHostName     = host;
    uc.dwHostNameLength = hostCch;
    uc.lpszUrlPath      = path;
    uc.dwUrlPathLength  = pathCch;

    if (!WinHttpCrackUrl(url, 0, 0, &uc)) return FALSE;
    *port   = uc.nPort;
    *useTLS = (_wcsicmp(scheme, L"https") == 0);
    return TRUE;
}

// ── Shared HTTP worker context ────────────────────────────────────────────────

struct HttpCtx {
    HWND hEdit;
    HWND hMainWnd;
    int  mode; // 1=GET, 2=POST, 3=Headers, 4=Download, 5=Reuse
    wchar_t url[512];
};

DWORD WINAPI HttpWorker(LPVOID param) {
    HttpCtx* ctx = (HttpCtx*)param;

    auto AppW = [&](const wchar_t* text) {
        int len = GetWindowTextLengthW(ctx->hEdit);
        SendMessage(ctx->hEdit, EM_SETSEL, len, len);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    };
    auto AppF = [&](const wchar_t* fmt, ...) {
        wchar_t buf[2048];
        va_list va; va_start(va, fmt);
        StringCchVPrintfW(buf, 2048, fmt, va);
        va_end(va);
        AppW(buf);
    };

    wchar_t scheme[32] = {}, host[256] = {}, path[512] = L"/";
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
    BOOL useTLS = FALSE;
    if (!ParseURL(ctx->url, scheme, 32, host, 256, path, 512, &port, &useTLS)) {
        AppF(L"ParseURL failed: %u", GetLastError());
        PostMessage(ctx->hMainWnd, WM_HTTP_DONE, 0, 0);
        HeapFree(GetProcessHeap(), 0, ctx);
        return 1;
    }
    if (!path[0] || path[0] == L'\0') StringCchCopyW(path, 512, L"/");
    AppF(L"Host: %s | Path: %s | Port: %u | TLS: %s",
        host, path, port, useTLS ? L"Yes" : L"No");

    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);

    // Open session
    HINTERNET hSession = WinHttpOpen(
        L"Win32Learn/1.0 WinHTTP",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        AppF(L"WinHttpOpen failed: %u", GetLastError());
        PostMessage(ctx->hMainWnd, WM_HTTP_DONE, 0, 0);
        HeapFree(GetProcessHeap(), 0, ctx);
        return 1;
    }

    // Set timeouts
    WinHttpSetTimeouts(hSession, 10000, 10000, 15000, 15000);

    // Connect
    QueryPerformanceCounter(&t1);
    HINTERNET hConn = WinHttpConnect(hSession, host, port, 0);
    QueryPerformanceCounter(&t2);
    double connMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
    AppF(L"WinHttpConnect: %.2f ms", connMs);

    DWORD openFlags = useTLS ? WINHTTP_FLAG_SECURE : 0;

    if (ctx->mode == 1 || ctx->mode == 3) {
        // ── GET ──
        AppW(L"--- GET Request ---");
        HINTERNET hReq = WinHttpOpenRequest(hConn,
            L"GET", path, nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            openFlags);

        // Add custom headers
        WinHttpAddRequestHeaders(hReq,
            L"Accept: text/html,application/xhtml+xml\r\n"
            L"Accept-Language: en-US\r\n",
            (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

        QueryPerformanceCounter(&t1);
        WinHttpSendRequest(hReq,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        WinHttpReceiveResponse(hReq, nullptr);
        QueryPerformanceCounter(&t2);
        double rspMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
        AppF(L"Response received in %.2f ms", rspMs);

        // Status code
        DWORD statusCode = 0, statusLen = sizeof(statusCode);
        WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusLen, WINHTTP_NO_HEADER_INDEX);
        AppF(L"HTTP Status: %u", statusCode);

        // Response headers
        DWORD headerBufLen = 0;
        WinHttpQueryHeaders(hReq, WINHTTP_QUERY_RAW_HEADERS_CRLF,
            WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &headerBufLen, WINHTTP_NO_HEADER_INDEX);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && headerBufLen > 0) {
            wchar_t* hdrs = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, headerBufLen);
            WinHttpQueryHeaders(hReq, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, hdrs, &headerBufLen, WINHTTP_NO_HEADER_INDEX);

            if (ctx->mode == 3) {
                // Headers mode: show all headers
                AppW(L"\r\n--- All Response Headers ---");
                wchar_t* line = hdrs;
                while (line && *line) {
                    wchar_t* nl = wcschr(line, L'\n');
                    if (nl) *nl = L'\0';
                    wchar_t* cr = wcschr(line, L'\r');
                    if (cr) *cr = L'\0';
                    if (*line) {
                        AppW(line);
                        // Cookie detection
                        if (_wcsnicmp(line, L"Set-Cookie:", 11) == 0) {
                            AppW(L"  [Cookie detected!]");
                            // Parse Name=Value
                            wchar_t* cookie = line + 11;
                            while (*cookie == L' ') cookie++;
                            wchar_t* semi = wcschr(cookie, L';');
                            if (semi) *semi = L'\0';
                            AppF(L"  Cookie Name=Value: %s", cookie);
                        }
                    }
                    if (!nl) break;
                    line = nl + 1;
                }
            } else {
                // GET mode: show first 5 header lines
                AppW(L"\r\n--- Response Headers (first 5) ---");
                wchar_t* line2 = hdrs;
                int lc = 0;
                while (line2 && *line2 && lc < 5) {
                    wchar_t* nl = wcschr(line2, L'\n');
                    if (nl) *nl = L'\0';
                    wchar_t* cr = wcschr(line2, L'\r');
                    if (cr) *cr = L'\0';
                    if (*line2) { AppW(line2); lc++; }
                    if (!nl) break;
                    line2 = nl + 1;
                }
            }
            HeapFree(GetProcessHeap(), 0, hdrs);
        }

        // Content-Type
        wchar_t ct[256] = {}; DWORD ctLen = sizeof(ct);
        if (WinHttpQueryHeaders(hReq, WINHTTP_QUERY_CONTENT_TYPE,
                WINHTTP_HEADER_NAME_BY_INDEX, ct, &ctLen, WINHTTP_NO_HEADER_INDEX))
            AppF(L"Content-Type: %s", ct);

        // Content-Length
        wchar_t cl[32] = {}; DWORD clLen = sizeof(cl);
        if (WinHttpQueryHeaders(hReq, WINHTTP_QUERY_CONTENT_LENGTH,
                WINHTTP_HEADER_NAME_BY_INDEX, cl, &clLen, WINHTTP_NO_HEADER_INDEX))
            AppF(L"Content-Length: %s bytes", cl);

        // Read body (first 2KB)
        AppW(L"\r\n--- Body (first 2048 bytes) ---");
        DWORD totalRead = 0;
        BYTE  bodyBuf[4096] = {};
        DWORD available = 0, downloaded = 0;
        while (totalRead < 2048) {
            WinHttpQueryDataAvailable(hReq, &available);
            if (!available) break;
            DWORD toRead = min(available, (DWORD)(sizeof(bodyBuf) - 1));
            if (!WinHttpReadData(hReq, bodyBuf + totalRead,
                    min(toRead, (DWORD)(2048 - totalRead)), &downloaded))
                break;
            totalRead += downloaded;
            if (!downloaded) break;
        }
        if (totalRead > 0) {
            // Convert body to wide for display (assume UTF-8/ASCII)
            wchar_t wbody[2048] = {};
            int wl = MultiByteToWideChar(CP_UTF8, 0,
                (char*)bodyBuf, totalRead, wbody, 2047);
            wbody[wl] = L'\0';
            // Truncate at 500 chars for display
            if (wl > 500) wbody[500] = L'\0';
            AppW(wbody);
            AppF(L"\r\n[... total %u bytes read ...]", totalRead);
        }

        WinHttpCloseHandle(hReq);
    }
    else if (ctx->mode == 2) {
        // ── POST ──
        AppW(L"--- POST Request ---");
        // POST to httpbin.org/post or example.com
        HINTERNET hReq = WinHttpOpenRequest(hConn,
            L"POST", path[0] ? path : L"/post", nullptr,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, openFlags);

        WinHttpAddRequestHeaders(hReq,
            L"Content-Type: application/x-www-form-urlencoded\r\n",
            (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

        const char* postData = "name=Win32&version=learning&topic=WinHTTP";
        DWORD postLen = (DWORD)lstrlenA(postData);
        AppF(L"POST body: %S (%u bytes)", postData, postLen);

        QueryPerformanceCounter(&t1);
        WinHttpSendRequest(hReq,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            (LPVOID)postData, postLen, postLen, 0);
        WinHttpReceiveResponse(hReq, nullptr);
        QueryPerformanceCounter(&t2);
        double pRspMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

        DWORD sc = 0, scl = sizeof(sc);
        WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &sc, &scl, WINHTTP_NO_HEADER_INDEX);
        AppF(L"POST Response: HTTP %u | Time: %.2f ms", sc, pRspMs);

        // Read response
        DWORD avail = 0, dl = 0, total = 0;
        BYTE rbuf[4096] = {};
        while (total < 2048) {
            WinHttpQueryDataAvailable(hReq, &avail);
            if (!avail) break;
            WinHttpReadData(hReq, rbuf + total, min(avail, (DWORD)(2048-total)), &dl);
            total += dl;
            if (!dl) break;
        }
        if (total > 0) {
            wchar_t wb[2048] = {};
            int wl = MultiByteToWideChar(CP_UTF8, 0, (char*)rbuf, total, wb, 2047);
            if (wl > 500) wl = 500;
            wb[wl] = L'\0';
            AppW(wb);
        }
        WinHttpCloseHandle(hReq);
    }
    else if (ctx->mode == 4) {
        // ── Download to temp file ──
        AppW(L"--- Download to Temp File ---");
        HINTERNET hReq = WinHttpOpenRequest(hConn,
            L"GET", path, nullptr,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, openFlags);
        WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        WinHttpReceiveResponse(hReq, nullptr);

        DWORD sc = 0, scl = sizeof(sc);
        WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &sc, &scl, WINHTTP_NO_HEADER_INDEX);
        AppF(L"HTTP %u", sc);

        // Create temp file
        wchar_t tempDir[MAX_PATH], tempFile[MAX_PATH];
        GetTempPathW(MAX_PATH, tempDir);
        GetTempFileNameW(tempDir, L"whttp", 0, tempFile);
        HANDLE hFile = CreateFileW(tempFile, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        AppF(L"Downloading to: %s", tempFile);

        DWORD avail = 0, dl = 0, totalBytes = 0;
        BYTE dbuf[8192];
        QueryPerformanceCounter(&t1);
        while (TRUE) {
            WinHttpQueryDataAvailable(hReq, &avail);
            if (!avail) break;
            if (!WinHttpReadData(hReq, dbuf, min(avail, (DWORD)sizeof(dbuf)), &dl))
                break;
            if (!dl) break;
            DWORD written = 0;
            WriteFile(hFile, dbuf, dl, &written, nullptr);
            totalBytes += dl;
        }
        QueryPerformanceCounter(&t2);
        double dlMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
        CloseHandle(hFile);

        // Show file size
        WIN32_FILE_ATTRIBUTE_DATA fad = {};
        GetFileAttributesExW(tempFile, GetFileExInfoStandard, &fad);
        DWORD fileSize = fad.nFileSizeLow;

        AppF(L"Downloaded: %u bytes to file (%u bytes on disk) in %.2f ms",
            totalBytes, fileSize, dlMs);
        AppF(L"Download speed: %.1f KB/s",
            totalBytes / 1024.0 / (dlMs / 1000.0));

        // Cleanup temp file
        DeleteFileW(tempFile);
        AppW(L"Temp file deleted after size inspection");
        WinHttpCloseHandle(hReq);
    }
    else if (ctx->mode == 5) {
        // ── Connection Reuse Demo ──
        AppW(L"--- Connection Reuse: 2 requests on same HCONNECT ---");

        for (int i = 1; i <= 2; i++) {
            HINTERNET hReq = WinHttpOpenRequest(hConn,
                L"GET", L"/", nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, openFlags);

            wchar_t extraHdr[64];
            StringCchPrintf(extraHdr, 64, L"X-Request-Num: %d\r\n", i);
            WinHttpAddRequestHeaders(hReq, extraHdr, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

            QueryPerformanceCounter(&t1);
            WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
            WinHttpReceiveResponse(hReq, nullptr);
            QueryPerformanceCounter(&t2);
            double rMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

            DWORD sc = 0, scl = sizeof(sc);
            WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &sc, &scl, WINHTTP_NO_HEADER_INDEX);

            // Read and discard body
            DWORD avail = 0, dl = 0, total = 0;
            BYTE rb[4096];
            while (TRUE) {
                WinHttpQueryDataAvailable(hReq, &avail);
                if (!avail) break;
                WinHttpReadData(hReq, rb, min(avail, (DWORD)sizeof(rb)), &dl);
                total += dl;
                if (!dl) break;
            }

            AppF(L"Request %d: HTTP %u | Body=%u bytes | Time=%.2f ms | (same HCONNECT=reuse)",
                i, sc, total, rMs);
            AppF(L"  (Request 2 should be faster if TCP connection was reused)");

            WinHttpCloseHandle(hReq);
        }
    }

    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hSession);

    PostMessage(ctx->hMainWnd, WM_HTTP_DONE, ctx->mode, 0);
    HeapFree(GetProcessHeap(), 0, ctx);
    return 0;
}

void LaunchHttpRequest(int mode) {
    wchar_t url[512];
    GetWindowTextW(g_hEditURL, url, 512);
    if (!url[0]) StringCchCopyW(url, 512, L"http://example.com/");

    OutClear();

    HttpCtx* ctx = (HttpCtx*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HttpCtx));
    ctx->hEdit    = g_hEdit;
    ctx->hMainWnd = g_hwnd;
    ctx->mode     = mode;
    StringCchCopyW(ctx->url, 512, url);

    OutFmt(L"=== WinHTTP Mode %d: URL=%s ===", mode, url);

    StatusSet(L"Working...");
    StatusSetR(L"In Progress");

    HANDLE hThr = CreateThread(nullptr, 0, HttpWorker, ctx, 0, nullptr);
    if (hThr) CloseHandle(hThr);
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"GET",          IDC_BTN_GET},
            {L"POST",         IDC_BTN_POST},
            {L"All Headers",  IDC_BTN_HEADERS},
            {L"Download",     IDC_BTN_DOWNLOAD},
            {L"Conn Reuse",   IDC_BTN_CONNREUSE},
        };
        for (int i = 0; i < 5; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 152, 8, 142, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        HWND hLbl = CreateWindowW(L"STATIC", L"URL:",
            WS_CHILD | WS_VISIBLE, 10, 46, 36, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_LBL_URL, g_hInst, nullptr);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEditURL = CreateWindowW(L"EDIT",
            L"http://example.com/",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            50, 45, 730, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_URL, g_hInst, nullptr);
        SendMessage(g_hEditURL, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 76, 800, 480, hwnd,
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
        SetWindowPos(g_hEditURL, nullptr, 50, 45, W - 60, 22, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 76, W, H - 76 - sh, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_GET:       LaunchHttpRequest(1); break;
        case IDC_BTN_POST:      LaunchHttpRequest(2); break;
        case IDC_BTN_HEADERS:   LaunchHttpRequest(3); break;
        case IDC_BTN_DOWNLOAD:  LaunchHttpRequest(4); break;
        case IDC_BTN_CONNREUSE: LaunchHttpRequest(5); break;
        }
        return 0;

    case WM_HTTP_DONE: {
        const wchar_t* modeNames[] = {
            L"", L"GET", L"POST", L"Headers", L"Download", L"Conn Reuse"
        };
        int mode = (int)wp;
        wchar_t msg2[128];
        StringCchPrintf(msg2, 128, L"WinHTTP — %s complete",
            (mode >= 1 && mode <= 5) ? modeNames[mode] : L"request");
        StatusSet(msg2);
        StatusSetR(L"Done");
        return 0;
    }

    case WM_KEYDOWN:
        if (wp == VK_F5) LaunchHttpRequest(1);
        return 0;

    case WM_DESTROY:
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
    wc.lpszClassName = L"WinHttp31Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"WinHttp31Class",
        L"31 — WinHTTP: GET/POST/Headers/Download/Connection Reuse",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 660,
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
