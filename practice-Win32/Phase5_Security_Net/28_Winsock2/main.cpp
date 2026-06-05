// 28_Winsock2 — Sockets, Adapters & Network Info
// Modes: TCP/UDP Loopback, Adapter Enum, Socket Options, Raw HTTP GET, DNS Resolution
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")

// Winsock headers MUST come before windows.h
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <windowsx.h>
#include <mstcpip.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

enum {
    IDC_BTN_LOOPBACK  = 101,
    IDC_BTN_ADAPTERS  = 102,
    IDC_BTN_SOCKOPTS  = 103,
    IDC_BTN_HTTPGET   = 104,
    IDC_BTN_DNS       = 105,
    IDC_EDIT_HOST     = 106,
    IDC_LISTVIEW      = 107,
    IDC_EDIT_OUT      = 108,
    IDC_STATUS        = 109,
};

#define WM_THREAD_DONE (WM_APP + 1)

HWND      g_hwnd      = nullptr;
HWND      g_hList     = nullptr;
HWND      g_hEdit     = nullptr;
HWND      g_hEditHost = nullptr;
HWND      g_hStatus   = nullptr;
HINSTANCE g_hInst     = nullptr;
int       g_itemCount = 0;
BOOL      g_wsaInited = FALSE;

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

int ListAddRow(const wchar_t* c0, const wchar_t* c1 = L"", const wchar_t* c2 = L"",
               const wchar_t* c3 = L"", const wchar_t* c4 = L"", const wchar_t* c5 = L"") {
    LVITEM lvi = {};
    lvi.mask   = LVIF_TEXT;
    lvi.iItem  = g_itemCount++;
    lvi.pszText = const_cast<wchar_t*>(c0);
    int row = ListView_InsertItem(g_hList, &lvi);
    if (c1[0]) ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(c1));
    if (c2[0]) ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(c2));
    if (c3[0]) ListView_SetItemText(g_hList, row, 3, const_cast<wchar_t*>(c3));
    if (c4[0]) ListView_SetItemText(g_hList, row, 4, const_cast<wchar_t*>(c4));
    if (c5[0]) ListView_SetItemText(g_hList, row, 5, const_cast<wchar_t*>(c5));
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

// Ensure WSA is initialized
BOOL EnsureWSA() {
    if (g_wsaInited) return TRUE;
    WSADATA wsa = {};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
        g_wsaInited = TRUE;
        return TRUE;
    }
    return FALSE;
}

// ── Mode 1: TCP/UDP Loopback ──────────────────────────────────────────────────

void ModeTCPLoopback() {
    OutClear();
    OutLine(L"=== TCP/UDP Loopback Demo ===");
    ListClear();
    ListAddColumn(0, L"Property",    180);
    ListAddColumn(1, L"Value",       200);
    ListAddColumn(2, L"Notes",       260);

    if (!EnsureWSA()) { OutLine(L"WSAStartup failed"); return; }

    // ── TCP ──
    OutLine(L"--- TCP Loopback ---");
    SOCKET srvSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srvSock == INVALID_SOCKET) { OutLine(L"server socket() failed"); return; }

    sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = htons(0); // OS picks port

    bind(srvSock, (sockaddr*)&addr, sizeof(addr));
    listen(srvSock, 1);

    // Get actual port
    sockaddr_in boundAddr = {};
    int addrLen = sizeof(boundAddr);
    getsockname(srvSock, (sockaddr*)&boundAddr, &addrLen);
    USHORT port = ntohs(boundAddr.sin_port);
    OutFmt(L"Server listening on 127.0.0.1:%u", port);

    // Client connect
    SOCKET cliSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in srvAddr = {};
    srvAddr.sin_family      = AF_INET;
    srvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    srvAddr.sin_port        = boundAddr.sin_port;
    connect(cliSock, (sockaddr*)&srvAddr, sizeof(srvAddr));

    // Accept
    sockaddr_in peerAddr = {}; int peerLen = sizeof(peerAddr);
    SOCKET acceptSock = accept(srvSock, (sockaddr*)&peerAddr, &peerLen);
    OutLine(L"Connection accepted");

    // getsockopt on client
    struct { int level; int name; const wchar_t* label; const wchar_t* notes; } opts[] = {
        {SOL_SOCKET,   SO_RCVBUF,    L"SO_RCVBUF",    L"Receive buffer size"},
        {SOL_SOCKET,   SO_SNDBUF,    L"SO_SNDBUF",    L"Send buffer size"},
        {SOL_SOCKET,   SO_TYPE,      L"SO_TYPE",      L"1=SOCK_STREAM, 2=DGRAM"},
        {SOL_SOCKET,   SO_KEEPALIVE, L"SO_KEEPALIVE", L"Keep-alive enabled"},
        {IPPROTO_TCP,  TCP_NODELAY,  L"TCP_NODELAY",  L"Nagle algorithm disabled"},
    };
    for (auto& o : opts) {
        int val = 0; int valLen = sizeof(val);
        if (getsockopt(cliSock, o.level, o.name, (char*)&val, &valLen) == 0) {
            wchar_t v[32];
            StringCchPrintf(v, 32, L"%d", val);
            ListAddRow(o.label, v, o.notes);
        }
    }

    // Send / recv
    const char* msg = "Hello from TCP client!";
    send(cliSock, msg, (int)lstrlenA(msg), 0);
    char rbuf[256] = {};
    int n = recv(acceptSock, rbuf, sizeof(rbuf) - 1, 0);
    if (n > 0) {
        wchar_t wbuf[256];
        MultiByteToWideChar(CP_UTF8, 0, rbuf, n, wbuf, 256);
        wbuf[n] = L'\0';
        OutFmt(L"Server recv [%d bytes]: %s", n, wbuf);
        ListAddRow(L"TCP send/recv", L"OK", L"Loopback round-trip");
    }

    closesocket(cliSock);
    closesocket(acceptSock);
    closesocket(srvSock);

    // ── UDP ──
    OutLine(L"\r\n--- UDP Loopback ---");
    SOCKET udpSrv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKET udpCli = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in ua = {};
    ua.sin_family      = AF_INET;
    ua.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ua.sin_port        = htons(0);
    bind(udpSrv, (sockaddr*)&ua, sizeof(ua));
    getsockname(udpSrv, (sockaddr*)&ua, &addrLen);

    const char* udpMsg = "UDP datagram!";
    sockaddr_in udpDst = ua;
    sendto(udpCli, udpMsg, (int)lstrlenA(udpMsg), 0, (sockaddr*)&udpDst, sizeof(udpDst));
    char udpRbuf[256] = {};
    sockaddr_in fromAddr = {}; int fromLen = sizeof(fromAddr);
    n = recvfrom(udpSrv, udpRbuf, sizeof(udpRbuf)-1, 0, (sockaddr*)&fromAddr, &fromLen);
    if (n > 0) {
        wchar_t wbuf[256];
        MultiByteToWideChar(CP_UTF8, 0, udpRbuf, n, wbuf, 256);
        wbuf[n] = L'\0';
        OutFmt(L"UDP recv [%d bytes]: %s", n, wbuf);
        ListAddRow(L"UDP sendto/recvfrom", L"OK", L"UDP loopback datagram");
    }
    closesocket(udpSrv);
    closesocket(udpCli);

    StatusSet(L"TCP/UDP Loopback — socket/bind/listen/accept/connect/send/recv/getsockopt");
    StatusSetR(L"Done");
}

// ── Mode 2: Adapter Enumeration ───────────────────────────────────────────────

void ModeAdapters() {
    ListClear();
    ListAddColumn(0, L"Name",      160);
    ListAddColumn(1, L"Status",     80);
    ListAddColumn(2, L"IPv4",      140);
    ListAddColumn(3, L"IPv6",      160);
    ListAddColumn(4, L"MAC",       130);
    ListAddColumn(5, L"MTU",        60);
    OutClear();
    OutLine(L"=== Network Adapter Enumeration (GetAdaptersAddresses) ===");

    if (!EnsureWSA()) { OutLine(L"WSAStartup failed"); return; }

    ULONG bufLen = 16384;
    IP_ADAPTER_ADDRESSES* pAdapters = (IP_ADAPTER_ADDRESSES*)
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufLen);

    DWORD ret = GetAdaptersAddresses(AF_UNSPEC,
        GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_WINS_INFO,
        nullptr, pAdapters, &bufLen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        HeapFree(GetProcessHeap(), 0, pAdapters);
        pAdapters = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, bufLen);
        ret = GetAdaptersAddresses(AF_UNSPEC,
            GAA_FLAG_INCLUDE_PREFIX, nullptr, pAdapters, &bufLen);
    }

    if (ret != NO_ERROR) {
        OutFmt(L"GetAdaptersAddresses failed: %u", ret);
        HeapFree(GetProcessHeap(), 0, pAdapters);
        return;
    }

    for (IP_ADAPTER_ADDRESSES* p = pAdapters; p; p = p->Next) {
        // Status
        const wchar_t* status =
            p->OperStatus == IfOperStatusUp             ? L"Up" :
            p->OperStatus == IfOperStatusDown           ? L"Down" :
            p->OperStatus == IfOperStatusDormant        ? L"Dormant" :
            p->OperStatus == IfOperStatusNotPresent     ? L"NotPresent" :
            p->OperStatus == IfOperStatusLowerLayerDown ? L"LLDown" : L"?";

        // First IPv4
        wchar_t ipv4Str[64] = L"(none)";
        wchar_t ipv6Str[128] = L"(none)";
        for (IP_ADAPTER_UNICAST_ADDRESS* ua = p->FirstUnicastAddress; ua; ua = ua->Next) {
            wchar_t addrBuf[128] = {};
            DWORD addrLen = 128;
            WSAAddressToStringW(ua->Address.lpSockaddr,
                ua->Address.iSockaddrLength, nullptr, addrBuf, &addrLen);
            if (ua->Address.lpSockaddr->sa_family == AF_INET &&
                wcscmp(ipv4Str, L"(none)") == 0)
                StringCchCopyW(ipv4Str, 64, addrBuf);
            else if (ua->Address.lpSockaddr->sa_family == AF_INET6 &&
                     wcscmp(ipv6Str, L"(none)") == 0)
                StringCchCopyW(ipv6Str, 128, addrBuf);
        }

        // MAC address
        wchar_t macStr[64] = L"(n/a)";
        if (p->PhysicalAddressLength == 6) {
            StringCchPrintf(macStr, 64, L"%02X:%02X:%02X:%02X:%02X:%02X",
                p->PhysicalAddress[0], p->PhysicalAddress[1],
                p->PhysicalAddress[2], p->PhysicalAddress[3],
                p->PhysicalAddress[4], p->PhysicalAddress[5]);
        }

        // MTU
        wchar_t mtuStr[16];
        StringCchPrintf(mtuStr, 16, L"%u", p->Mtu);

        // Name (FriendlyName)
        ListAddRow(p->FriendlyName, status, ipv4Str, ipv6Str, macStr, mtuStr);

        // Detail in edit
        OutFmt(L"[%s] %s | IfIndex=%u | IfType=%u | MTU=%u",
            status, p->FriendlyName, p->IfIndex, p->IfType, p->Mtu);
        OutFmt(L"  AdapterName: %S", p->AdapterName);
        OutFmt(L"  Description: %s", p->Description);
        OutFmt(L"  MAC: %s", macStr);
        if (p->TransmitLinkSpeed != (ULONG64)-1)
            OutFmt(L"  TxSpeed: %I64u Mbps", p->TransmitLinkSpeed / 1000000);
        if (p->ReceiveLinkSpeed != (ULONG64)-1)
            OutFmt(L"  RxSpeed: %I64u Mbps", p->ReceiveLinkSpeed / 1000000);

        // All unicast addresses
        for (IP_ADAPTER_UNICAST_ADDRESS* ua = p->FirstUnicastAddress; ua; ua = ua->Next) {
            wchar_t abuf[128] = {}; DWORD alen = 128;
            WSAAddressToStringW(ua->Address.lpSockaddr,
                ua->Address.iSockaddrLength, nullptr, abuf, &alen);
            OutFmt(L"  UnicastAddr: %s (prefix/%u)",
                abuf, ua->OnLinkPrefixLength);
        }

        // DNS servers
        for (IP_ADAPTER_DNS_SERVER_ADDRESS* dns = p->FirstDnsServerAddress;
             dns; dns = dns->Next) {
            wchar_t dbuf[128] = {}; DWORD dlen = 128;
            WSAAddressToStringW(dns->Address.lpSockaddr,
                dns->Address.iSockaddrLength, nullptr, dbuf, &dlen);
            OutFmt(L"  DNS Server: %s", dbuf);
        }
        OutLine(L"");
    }

    HeapFree(GetProcessHeap(), 0, pAdapters);
    StatusSet(L"Adapters — GetAdaptersAddresses: Name/Status/IP/MAC/MTU/DNS");
    wchar_t cnt[32];
    StringCchPrintf(cnt, 32, L"Adapters: %d", g_itemCount);
    StatusSetR(cnt);
}

// ── Mode 3: Socket Options Inspector ─────────────────────────────────────────

void ModeSockOpts() {
    OutClear();
    OutLine(L"=== Socket Options Inspector + WSAIoctl ===");
    ListClear();
    ListAddColumn(0, L"Option",      160);
    ListAddColumn(1, L"Before",      120);
    ListAddColumn(2, L"After Set",   120);
    ListAddColumn(3, L"Notes",       280);

    if (!EnsureWSA()) { OutLine(L"WSAStartup failed"); return; }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { OutFmt(L"socket() failed: %d", WSAGetLastError()); return; }

    // Helper lambda via struct
    auto getSockOptInt = [&](int level, int name, int* out) -> BOOL {
        int len = sizeof(int);
        return getsockopt(s, level, name, (char*)out, &len) == 0;
    };

    struct { int lvl; int name; const wchar_t* label; const wchar_t* notes; int setVal; } opts[] = {
        {SOL_SOCKET,  SO_RCVBUF,    L"SO_RCVBUF",    L"Receive buffer",      131072},
        {SOL_SOCKET,  SO_SNDBUF,    L"SO_SNDBUF",    L"Send buffer",         131072},
        {SOL_SOCKET,  SO_KEEPALIVE, L"SO_KEEPALIVE", L"Keep-alive",          1},
        {SOL_SOCKET,  SO_REUSEADDR, L"SO_REUSEADDR", L"Reuse address",       1},
        {SOL_SOCKET,  SO_TYPE,      L"SO_TYPE",      L"1=Stream,2=Dgram",    -1},
        {IPPROTO_TCP, TCP_NODELAY,  L"TCP_NODELAY",  L"Disable Nagle algo",  1},
    };

    for (auto& o : opts) {
        int before = -1, after = -1;
        getSockOptInt(o.lvl, o.name, &before);

        if (o.setVal >= 0) {
            setsockopt(s, o.lvl, o.name, (char*)&o.setVal, sizeof(o.setVal));
            getSockOptInt(o.lvl, o.name, &after);
        }

        wchar_t bStr[32], aStr[32];
        StringCchPrintf(bStr, 32, L"%d", before);
        if (o.setVal >= 0) StringCchPrintf(aStr, 32, L"%d", after);
        else               StringCchCopy(aStr, 32, L"(not set)");

        ListAddRow(o.label, bStr, aStr, o.notes);
        OutFmt(L"%-20s  before=%-10d after=%d", o.label, before, after);
    }

    // SO_LINGER
    LINGER ling = {}; int lingLen = sizeof(ling);
    if (getsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&ling, &lingLen) == 0) {
        wchar_t v[64];
        StringCchPrintf(v, 64, L"onoff=%u timeout=%u", ling.l_onoff, ling.l_linger);
        ListAddRow(L"SO_LINGER", v, L"(unchanged)", L"Linger on close");
        OutFmt(L"SO_LINGER: onoff=%u timeout=%us", ling.l_onoff, ling.l_linger);
    }

    // WSAIoctl: FIONBIO — set non-blocking
    DWORD nonBlocking = 1, bytesRet = 0;
    int rc = WSAIoctl(s, FIONBIO, &nonBlocking, sizeof(nonBlocking),
        nullptr, 0, &bytesRet, nullptr, nullptr);
    wchar_t ioStr[64];
    StringCchPrintf(ioStr, 64, L"rc=%d (0=OK)", rc);
    ListAddRow(L"WSAIoctl(FIONBIO=1)", ioStr, L"(irreversible)", L"Set non-blocking mode");
    OutFmt(L"WSAIoctl(FIONBIO): rc=%d — socket is now non-blocking", rc);

    // Non-blocking connect (will fail immediately = WSAEWOULDBLOCK)
    sockaddr_in dst = {};
    dst.sin_family      = AF_INET;
    dst.sin_addr.s_addr = htonl(0x08080808); // 8.8.8.8
    dst.sin_port        = htons(80);
    int connectRc = connect(s, (sockaddr*)&dst, sizeof(dst));
    int connectErr = WSAGetLastError();
    OutFmt(L"Non-blocking connect to 8.8.8.8:80 => rc=%d err=%d (%s)",
        connectRc, connectErr,
        connectErr == WSAEWOULDBLOCK ? L"WSAEWOULDBLOCK (expected)" : L"other");

    // select() with 1s timeout
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(s, &writeSet);
    timeval tv = {1, 0};
    int selRc = select(0, nullptr, &writeSet, nullptr, &tv);
    OutFmt(L"select(write, 1s timeout): rc=%d (>0=writable, 0=timeout)", selRc);
    if (selRc > 0) ListAddRow(L"select() write-ready", L"YES", L"Connected", L"Non-blocking connect succeeded");

    closesocket(s);
    StatusSet(L"Socket Options — getsockopt/setsockopt/WSAIoctl(FIONBIO)/select");
    StatusSetR(L"Done");
}

// ── Mode 4: Raw HTTP GET (worker thread) ──────────────────────────────────────

struct HttpGetCtx {
    HWND  hEdit;
    HWND  hStatus;
    HWND  hMainWnd;
    wchar_t host[256];
};

DWORD WINAPI HttpGetWorker(LPVOID param) {
    HttpGetCtx* ctx = (HttpGetCtx*)param;

    auto AppendW = [&](const wchar_t* text) {
        int len = GetWindowTextLengthW(ctx->hEdit);
        SendMessage(ctx->hEdit, EM_SETSEL, len, len);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    };
    auto AppendFmt = [&](const wchar_t* fmt, ...) {
        wchar_t buf[2048];
        va_list va; va_start(va, fmt);
        StringCchVPrintfW(buf, 2048, fmt, va);
        va_end(va);
        AppendW(buf);
    };

    char hostA[256];
    WideCharToMultiByte(CP_UTF8, 0, ctx->host, -1, hostA, 256, nullptr, nullptr);

    // Resolve hostname
    addrinfo hints = {}, *res = nullptr;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);
    int gai = getaddrinfo(hostA, "80", &hints, &res);
    QueryPerformanceCounter(&t2);
    double dnsMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

    if (gai != 0) {
        AppendFmt(L"getaddrinfo(%S) failed: %d", hostA, gai);
        HeapFree(GetProcessHeap(), 0, ctx);
        return 1;
    }
    AppendFmt(L"DNS resolved in %.1f ms", dnsMs);

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    QueryPerformanceCounter(&t1);
    int connRc = connect(s, res->ai_addr, (int)res->ai_addrlen);
    QueryPerformanceCounter(&t2);
    double connMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
    freeaddrinfo(res);

    if (connRc != 0) {
        AppendFmt(L"connect() failed: %d", WSAGetLastError());
        closesocket(s);
        HeapFree(GetProcessHeap(), 0, ctx);
        return 1;
    }
    AppendFmt(L"TCP connect in %.1f ms", connMs);

    // Build HTTP/1.1 GET request
    char req[512];
    StringCchPrintfA(req, 512,
        "GET / HTTP/1.1\r\nHost: %s\r\nUser-Agent: Win32-Learn/1.0\r\n"
        "Connection: close\r\n\r\n", hostA);

    QueryPerformanceCounter(&t1);
    send(s, req, (int)lstrlenA(req), 0);

    // Receive response
    char recvBuf[8192]; int recvTotal = 0;
    wchar_t headerBuf[4096] = {};
    BOOL headersDone = FALSE;
    for (;;) {
        int n = recv(s, recvBuf, sizeof(recvBuf) - 1, 0);
        if (n <= 0) break;
        recvBuf[n] = '\0';
        recvTotal += n;
        if (!headersDone) {
            // Append to header buffer
            char* crlfcrlf = strstr(recvBuf, "\r\n\r\n");
            int headerBytes = crlfcrlf ? (int)(crlfcrlf - recvBuf + 4) : n;
            int wlen = MultiByteToWideChar(CP_ACP, 0, recvBuf,
                min(headerBytes, 2000), headerBuf, 4096);
            if (wlen > 0) headerBuf[wlen] = L'\0';
            if (crlfcrlf) headersDone = TRUE;
        }
    }
    QueryPerformanceCounter(&t2);
    double totalMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

    AppendFmt(L"Total bytes received: %d | Response time: %.1f ms", recvTotal, totalMs);
    AppendW(L"\r\n--- Response Headers ---");

    // Split header into lines for display
    wchar_t* line = headerBuf;
    int lineCount = 0;
    while (line && lineCount < 20) {
        wchar_t* nl = wcschr(line, L'\n');
        if (nl) *nl = L'\0';
        // Remove \r
        wchar_t* cr = wcschr(line, L'\r');
        if (cr) *cr = L'\0';
        if (line[0]) {
            AppendW(line);
            lineCount++;
        }
        if (!nl) break;
        line = nl + 1;
    }

    closesocket(s);
    HeapFree(GetProcessHeap(), 0, ctx);
    PostMessage(ctx->hMainWnd, WM_THREAD_DONE, 0, 0); // ctx already freed; use cached hwnd
    return 0;
}

void ModeHTTPGet() {
    OutClear();
    ListClear();
    OutLine(L"=== Raw HTTP GET (plain Winsock) ===");

    if (!EnsureWSA()) { OutLine(L"WSAStartup failed"); return; }

    wchar_t host[256];
    GetWindowTextW(g_hEditHost, host, 256);
    if (!host[0]) StringCchCopyW(host, 256, L"example.com");

    OutFmt(L"Connecting to %s:80 ...", host);

    HttpGetCtx* ctx = (HttpGetCtx*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HttpGetCtx));
    ctx->hEdit    = g_hEdit;
    ctx->hStatus  = g_hStatus;
    ctx->hMainWnd = g_hwnd;
    StringCchCopyW(ctx->host, 256, host);

    HANDLE hThread = CreateThread(nullptr, 0, HttpGetWorker, ctx, 0, nullptr);
    if (hThread) CloseHandle(hThread);

    StatusSet(L"HTTP GET — raw Winsock connect/send/recv (worker thread)");
    StatusSetR(L"Working...");
}

// ── Mode 5: DNS & Address Resolution ─────────────────────────────────────────

void ModeDNS() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Hostname",    200);
    ListAddColumn(1, L"Family",       70);
    ListAddColumn(2, L"Address",     180);
    ListAddColumn(3, L"CanonName",   200);
    OutLine(L"=== DNS & Address Resolution ===");

    if (!EnsureWSA()) { OutLine(L"WSAStartup failed"); return; }

    wchar_t hostW[256];
    GetWindowTextW(g_hEditHost, hostW, 256);
    if (!hostW[0]) StringCchCopyW(hostW, 256, L"example.com");
    char hostA[256];
    WideCharToMultiByte(CP_UTF8, 0, hostW, -1, hostA, 256, nullptr, nullptr);

    OutFmt(L"Resolving: %s", hostW);

    // ── getaddrinfo — AF_INET ──
    addrinfo hints = {}, *res4 = nullptr;
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    if (getaddrinfo(hostA, nullptr, &hints, &res4) == 0) {
        for (addrinfo* r = res4; r; r = r->ai_next) {
            wchar_t addrStr[128] = {}; DWORD al = 128;
            WSAAddressToStringW(r->ai_addr, (DWORD)r->ai_addrlen,
                nullptr, addrStr, &al);
            wchar_t canon[256] = {};
            if (r->ai_canonname)
                MultiByteToWideChar(CP_UTF8, 0, r->ai_canonname, -1, canon, 256);
            ListAddRow(hostW, L"IPv4", addrStr, canon);
            OutFmt(L"  IPv4: %s | canon: %s", addrStr, canon);
        }
        freeaddrinfo(res4);
    } else {
        OutFmt(L"  IPv4 getaddrinfo failed: %d", WSAGetLastError());
    }

    // ── getaddrinfo — AF_INET6 ──
    addrinfo hints6 = {};
    hints6.ai_flags    = AI_CANONNAME;
    hints6.ai_socktype = SOCK_STREAM;
    hints6.ai_family   = AF_INET6;
    addrinfo* res6 = nullptr;
    if (getaddrinfo(hostA, nullptr, &hints6, &res6) == 0) {
        for (addrinfo* r = res6; r; r = r->ai_next) {
            wchar_t addrStr[128] = {}; DWORD al = 128;
            WSAAddressToStringW(r->ai_addr, (DWORD)r->ai_addrlen,
                nullptr, addrStr, &al);
            ListAddRow(hostW, L"IPv6", addrStr, L"");
            OutFmt(L"  IPv6: %s", addrStr);
        }
        freeaddrinfo(res6);
    } else {
        OutFmt(L"  IPv6 getaddrinfo: %d (may be no AAAA record)", WSAGetLastError());
    }

    OutLine(L"");

    // ── getnameinfo — reverse lookup of loopback ──
    OutLine(L"--- getnameinfo (reverse lookup) ---");
    sockaddr_in lo = {};
    lo.sin_family = AF_INET;
    lo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    char nameOut[NI_MAXHOST] = {}, servOut[NI_MAXSERV] = {};
    if (getnameinfo((sockaddr*)&lo, sizeof(lo), nameOut, NI_MAXHOST,
            servOut, NI_MAXSERV, NI_NAMEREQD) == 0) {
        wchar_t wname[256], wserv[64];
        MultiByteToWideChar(CP_ACP, 0, nameOut, -1, wname, 256);
        MultiByteToWideChar(CP_ACP, 0, servOut, -1, wserv, 64);
        ListAddRow(L"127.0.0.1 (reverse)", L"PTR", wname, L"");
        OutFmt(L"getnameinfo(127.0.0.1): %s", wname);
    } else {
        OutFmt(L"getnameinfo(127.0.0.1) err: %d", WSAGetLastError());
        ListAddRow(L"127.0.0.1 (reverse)", L"PTR", L"(failed)", L"");
    }

    // ── legacy gethostbyname ──
    OutLine(L"");
    OutLine(L"--- Legacy gethostbyname ---");
    hostent* he = gethostbyname(hostA);
    if (he) {
        wchar_t whn[256];
        MultiByteToWideChar(CP_ACP, 0, he->h_name, -1, whn, 256);
        OutFmt(L"gethostbyname official name: %s", whn);
        for (int i = 0; he->h_addr_list[i]; i++) {
            in_addr a; a.s_addr = *(DWORD*)he->h_addr_list[i];
            char* ip = inet_ntoa(a);
            wchar_t wip[64];
            MultiByteToWideChar(CP_ACP, 0, ip, -1, wip, 64);
            OutFmt(L"  addr[%d]: %s", i, wip);
            ListAddRow(L"gethostbyname", L"IPv4", wip, whn);
        }
    } else {
        OutFmt(L"gethostbyname failed: %d", WSAGetLastError());
    }

    StatusSet(L"DNS — getaddrinfo(IPv4/IPv6) + getnameinfo + gethostbyname");
    wchar_t cnt[32];
    StringCchPrintf(cnt, 32, L"Records: %d", g_itemCount);
    StatusSetR(cnt);
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"TCP/UDP Loop",  IDC_BTN_LOOPBACK},
            {L"Adapters",      IDC_BTN_ADAPTERS},
            {L"Sock Options",  IDC_BTN_SOCKOPTS},
            {L"HTTP GET",      IDC_BTN_HTTPGET},
            {L"DNS / Resolve", IDC_BTN_DNS},
        };
        for (int i = 0; i < 5; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 152, 8, 142, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        HWND hLbl = CreateWindowW(L"STATIC", L"Host:",
            WS_CHILD | WS_VISIBLE, 10, 46, 40, 22, hwnd, nullptr, g_hInst, nullptr);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEditHost = CreateWindowW(L"EDIT", L"example.com",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            55, 45, 400, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_HOST, g_hInst, nullptr);
        SendMessage(g_hEditHost, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hList = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 76, 800, 220, hwnd,
            (HMENU)(UINT_PTR)IDC_LISTVIEW, g_hInst, nullptr);
        ListView_SetExtendedListViewStyle(g_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        SendMessage(g_hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 302, 800, 200, hwnd,
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
        int parts[] = {W - 180, -1};
        SendMessage(g_hStatus, SB_SETPARTS, 2, (LPARAM)parts);
        SetWindowPos(g_hEditHost, nullptr, 55, 45, W - 70, 22, SWP_NOZORDER);
        int listH = (H - 76 - sh) * 45 / 100;
        int editH = H - 76 - sh - listH;
        SetWindowPos(g_hList, nullptr, 0, 76, W, listH, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 76 + listH, W, editH, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_LOOPBACK: ModeTCPLoopback(); break;
        case IDC_BTN_ADAPTERS: ModeAdapters();    break;
        case IDC_BTN_SOCKOPTS: ModeSockOpts();    break;
        case IDC_BTN_HTTPGET:  ModeHTTPGet();     break;
        case IDC_BTN_DNS:      ModeDNS();         break;
        }
        return 0;

    case WM_THREAD_DONE:
        StatusSet(L"HTTP GET complete");
        StatusSetR(L"Done");
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) ModeTCPLoopback();
        return 0;

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lp;
        if (nm->hwndFrom == g_hList && nm->code == LVN_COLUMNCLICK) {}
        return 0;
    }

    case WM_DESTROY:
        if (g_wsaInited) WSACleanup();
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
    wc.lpszClassName = L"Ws28Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Ws28Class",
        L"28 — Winsock2: Sockets, Adapters & Network Info",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 940, 680,
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
