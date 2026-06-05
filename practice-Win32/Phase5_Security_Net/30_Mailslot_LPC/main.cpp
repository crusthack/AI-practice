// 30_Mailslot_LPC — Mailslot IPC Workbench
// Modes: Basic, Multi-slot, Sequenced Protocol, Non-blocking, Service Discovery
#pragma comment(lib, "comctl32.lib")

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <windowsx.h>
#include <process.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

enum {
    IDC_BTN_BASIC   = 101,
    IDC_BTN_MULTI   = 102,
    IDC_BTN_SEQD    = 103,
    IDC_BTN_NONBLK  = 104,
    IDC_BTN_DISCSVC = 105,
    IDC_LISTVIEW    = 106,
    IDC_EDIT_OUT    = 107,
    IDC_STATUS      = 108,
};

HWND      g_hwnd      = nullptr;
HWND      g_hList     = nullptr;
HWND      g_hEdit     = nullptr;
HWND      g_hStatus   = nullptr;
HINSTANCE g_hInst     = nullptr;
int       g_itemCount = 0;

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
               const wchar_t* c2 = L"", const wchar_t* c3 = L"") {
    LVITEM lvi = {};
    lvi.mask   = LVIF_TEXT;
    lvi.iItem  = g_itemCount++;
    lvi.pszText = const_cast<wchar_t*>(c0);
    int row = ListView_InsertItem(g_hList, &lvi);
    if (c1[0]) ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(c1));
    if (c2[0]) ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(c2));
    if (c3[0]) ListView_SetItemText(g_hList, row, 3, const_cast<wchar_t*>(c3));
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

// Helper: show mailslot queue state
void ShowMailslotInfo(HANDLE hSlot, const wchar_t* label) {
    DWORD maxMsgSize = 0, nextMsgSize = 0, msgCount = 0, readTimeout = 0;
    GetMailslotInfo(hSlot, &maxMsgSize, &nextMsgSize, &msgCount, &readTimeout);
    wchar_t buf[256];
    StringCchPrintf(buf, 256,
        L"[%s] maxMsg=%u nextMsg=%s count=%u timeout=%u",
        label, maxMsgSize,
        nextMsgSize == MAILSLOT_NO_MESSAGE ? L"(empty)" :
            (StringCchPrintf(buf, 256, L"%u", nextMsgSize), buf),
        msgCount,
        readTimeout == MAILSLOT_WAIT_FOREVER ? 0xFFFFFFFF : readTimeout);
    // Re-format properly
    wchar_t nextStr[32];
    if (nextMsgSize == MAILSLOT_NO_MESSAGE) StringCchCopy(nextStr, 32, L"(none)");
    else StringCchPrintf(nextStr, 32, L"%u", nextMsgSize);
    StringCchPrintf(buf, 256,
        L"[%s] maxMsg=%u nextMsg=%s queued=%u timeout=%u",
        label, maxMsgSize, nextStr, msgCount, readTimeout);
    OutLine(buf);
}

// ── Mode 1: Basic Mailslot ────────────────────────────────────────────────────

void ModeBasic() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Property",   180);
    ListAddColumn(1, L"Value",      260);
    ListAddColumn(2, L"Notes",      260);
    OutLine(L"=== Basic Mailslot ===");

    const wchar_t* slotName = L"\\\\.\\mailslot\\Win32Learn_Basic";

    // Create mailslot
    HANDLE hSlot = CreateMailslotW(slotName,
        0,                      // max message size: any
        MAILSLOT_WAIT_FOREVER,  // read timeout
        nullptr);
    if (hSlot == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateMailslot failed: %u", GetLastError());
        return;
    }
    OutFmt(L"CreateMailslot '%s': OK", slotName);

    // Query info before write
    ShowMailslotInfo(hSlot, L"BEFORE WRITE");

    DWORD maxMsgSize = 0, nextMsgSize = 0, msgCount = 0, readTimeout = 0;
    GetMailslotInfo(hSlot, &maxMsgSize, &nextMsgSize, &msgCount, &readTimeout);
    wchar_t v[64];
    StringCchPrintf(v, 64, L"%u", maxMsgSize);
    ListAddRow(L"Max Message Size", v, L"0=unlimited");
    StringCchPrintf(v, 64, L"%u", msgCount);
    ListAddRow(L"Initial Queue Depth", v, L"Should be 0");
    StringCchPrintf(v, 64, L"%u", readTimeout);
    ListAddRow(L"Read Timeout", v, L"0xFFFFFFFF=WAIT_FOREVER");

    // Write 3 messages
    HANDLE hWriter = CreateFileW(slotName,
        GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hWriter != INVALID_HANDLE_VALUE) {
        const wchar_t* msgs[] = {
            L"First mailslot message",
            L"Second mailslot message",
            L"Third mailslot message",
        };
        LARGE_INTEGER t1, t2, freq;
        QueryPerformanceFrequency(&freq);
        for (int i = 0; i < 3; i++) {
            DWORD written = 0;
            QueryPerformanceCounter(&t1);
            WriteFile(hWriter, msgs[i],
                (DWORD)((wcslen(msgs[i]) + 1) * sizeof(wchar_t)), &written, nullptr);
            QueryPerformanceCounter(&t2);
            double ms = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
            wchar_t sz[64];
            StringCchPrintf(sz, 64, L"%u bytes | %.3f ms", written, ms);
            ListAddRow(msgs[i], sz, L"WriteFile");
            OutFmt(L"Wrote: '%s' (%u bytes, %.3f ms)", msgs[i], written, ms);
        }
        CloseHandle(hWriter);
    }

    // Query info after write
    ShowMailslotInfo(hSlot, L"AFTER WRITE");
    GetMailslotInfo(hSlot, &maxMsgSize, &nextMsgSize, &msgCount, &readTimeout);
    StringCchPrintf(v, 64, L"%u messages queued", msgCount);
    ListAddRow(L"Queue After Write", v, L"GetMailslotInfo");

    // SetMailslotInfo: change read timeout to 500ms
    SetMailslotInfo(hSlot, 500);
    GetMailslotInfo(hSlot, nullptr, nullptr, nullptr, &readTimeout);
    OutFmt(L"SetMailslotInfo(500ms): readTimeout now=%u", readTimeout);
    ListAddRow(L"SetMailslotInfo", L"500ms", L"Changed read timeout");

    // Read all messages
    OutLine(L"Reading queued messages:");
    GetMailslotInfo(hSlot, nullptr, &nextMsgSize, &msgCount, nullptr);
    while (msgCount > 0 && nextMsgSize != MAILSLOT_NO_MESSAGE) {
        wchar_t buf[512] = {};
        DWORD read = 0;
        ReadFile(hSlot, buf, sizeof(buf), &read, nullptr);
        OutFmt(L"  ReadFile %u bytes: '%s'", read, buf);
        GetMailslotInfo(hSlot, nullptr, &nextMsgSize, &msgCount, nullptr);
    }

    // Query info after read
    ShowMailslotInfo(hSlot, L"AFTER READ");
    CloseHandle(hSlot);

    StatusSet(L"Basic — CreateMailslot + GetMailslotInfo + SetMailslotInfo + Read/Write");
    StatusSetR(L"Done");
}

// ── Mode 2: Multi-slot (3 named slots, broadcast to all) ─────────────────────

void ModeMultiSlot() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Slot",       200);
    ListAddColumn(1, L"Bytes Sent", 100);
    ListAddColumn(2, L"Received",   200);
    ListAddColumn(3, L"Status",     100);
    OutLine(L"=== Multi-instance Mailslots (3 slots, broadcast) ===");

    const wchar_t* slotNames[] = {
        L"\\\\.\\mailslot\\Win32Learn_Slot1",
        L"\\\\.\\mailslot\\Win32Learn_Slot2",
        L"\\\\.\\mailslot\\Win32Learn_Slot3",
    };
    HANDLE hSlots[3] = {};

    // Create 3 mailslots
    for (int i = 0; i < 3; i++) {
        hSlots[i] = CreateMailslotW(slotNames[i], 0, 500, nullptr);
        if (hSlots[i] == INVALID_HANDLE_VALUE) {
            OutFmt(L"CreateMailslot[%d] failed: %u", i, GetLastError());
            hSlots[i] = nullptr;
        } else {
            OutFmt(L"Created slot[%d]: %s", i, slotNames[i]);
        }
    }

    // Broadcast message to all 3
    const wchar_t* broadMsg = L"Broadcast: Hello all slots!";
    for (int i = 0; i < 3; i++) {
        if (!hSlots[i]) continue;
        HANDLE hW = CreateFileW(slotNames[i],
            GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hW != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hW, broadMsg,
                (DWORD)((wcslen(broadMsg) + 1) * sizeof(wchar_t)), &written, nullptr);
            CloseHandle(hW);
            // Read back
            wchar_t rbuf[512] = {};
            DWORD read = 0;
            ReadFile(hSlots[i], rbuf, sizeof(rbuf), &read, nullptr);
            wchar_t sz[32]; StringCchPrintf(sz, 32, L"%u", written);
            ListAddRow(slotNames[i] + 9, sz, rbuf, L"OK");
            OutFmt(L"Slot[%d]: sent %u bytes, got back '%s'", i, written, rbuf);
        }
    }

    // Write multiple messages to slot[0] and show queue stats
    OutLine(L"");
    OutLine(L"Writing 5 messages to slot[0]:");
    if (hSlots[0]) {
        HANDLE hW = CreateFileW(slotNames[0],
            GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        for (int i = 0; i < 5; i++) {
            wchar_t msg[64];
            StringCchPrintf(msg, 64, L"Message %d of 5", i + 1);
            DWORD written = 0;
            WriteFile(hW, msg, (DWORD)((wcslen(msg) + 1) * sizeof(wchar_t)), &written, nullptr);
        }
        CloseHandle(hW);

        DWORD cnt = 0, nxt = 0;
        GetMailslotInfo(hSlots[0], nullptr, &nxt, &cnt, nullptr);
        OutFmt(L"Queue: %u messages, next=%u bytes", cnt, nxt);
        ListAddRow(L"Queue depth", L"5 messages", L"After 5 writes to slot[0]", L"OK");

        // Drain
        while (cnt > 0) {
            wchar_t rbuf[512] = {};
            DWORD read = 0;
            ReadFile(hSlots[0], rbuf, sizeof(rbuf), &read, nullptr);
            OutFmt(L"  '%s'", rbuf);
            GetMailslotInfo(hSlots[0], nullptr, &nxt, &cnt, nullptr);
        }
    }

    for (int i = 0; i < 3; i++)
        if (hSlots[i]) CloseHandle(hSlots[i]);

    StatusSet(L"Multi-slot — 3 CreateMailslot instances + broadcast write + queue stats");
    StatusSetR(L"Done");
}

// ── Mode 3: Sequenced Protocol ────────────────────────────────────────────────

// Message frame with 4-byte sequence prefix
#pragma pack(push, 1)
struct SeqMessage {
    DWORD    seq;
    wchar_t  data[120];
};
#pragma pack(pop)

void ModeSequenced() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Seq#",   60);
    ListAddColumn(1, L"Message", 280);
    ListAddColumn(2, L"Size",    80);
    ListAddColumn(3, L"Time ms",  80);
    OutLine(L"=== Sequenced Protocol (4-byte seq prefix) ===");

    const wchar_t* slotName = L"\\\\.\\mailslot\\Win32Learn_Seqd";

    HANDLE hSlot = CreateMailslotW(slotName, sizeof(SeqMessage), 1000, nullptr);
    if (hSlot == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateMailslot failed: %u", GetLastError());
        return;
    }

    HANDLE hWriter = CreateFileW(slotName,
        GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hWriter == INVALID_HANDLE_VALUE) {
        OutFmt(L"Open writer failed: %u", GetLastError());
        CloseHandle(hSlot);
        return;
    }

    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);

    // Send 6 sequenced messages
    const wchar_t* payloads[] = {
        L"Init handshake",
        L"Data block A",
        L"Data block B",
        L"Data block C",
        L"Checksum: 0xDEADBEEF",
        L"Termination signal",
    };

    for (int i = 0; i < 6; i++) {
        SeqMessage smsg;
        smsg.seq = (DWORD)(i + 1);
        StringCchCopyW(smsg.data, 120, payloads[i]);
        DWORD written = 0;
        QueryPerformanceCounter(&t1);
        WriteFile(hWriter, &smsg, sizeof(smsg), &written, nullptr);
        QueryPerformanceCounter(&t2);
        double ms = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

        wchar_t seqStr[16], szStr[32], msStr[32];
        StringCchPrintf(seqStr, 16, L"%u", smsg.seq);
        StringCchPrintf(szStr, 32, L"%u", written);
        StringCchPrintf(msStr, 32, L"%.3f", ms);
        ListAddRow(seqStr, payloads[i], szStr, msStr);
        OutFmt(L"Sent seq=%u '%s' (%u bytes, %.3f ms)", smsg.seq, smsg.data, written, ms);
    }
    CloseHandle(hWriter);

    // Query queue
    DWORD cnt = 0, nxt = 0;
    GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);
    OutFmt(L"\nQueue: %u messages pending, next=%u bytes", cnt, nxt);

    // Read and validate sequence
    OutLine(L"\nReading & validating sequence:");
    DWORD expectedSeq = 1;
    while (cnt > 0 && nxt != MAILSLOT_NO_MESSAGE) {
        SeqMessage rmsg = {};
        DWORD read = 0;
        ReadFile(hSlot, &rmsg, sizeof(rmsg), &read, nullptr);
        BOOL seqOk = (rmsg.seq == expectedSeq);
        OutFmt(L"  seq=%u '%s' | Expected=%u %s",
            rmsg.seq, rmsg.data, expectedSeq, seqOk ? L"OK" : L"OUT-OF-ORDER!");
        expectedSeq++;
        GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);
    }

    CloseHandle(hSlot);
    StatusSet(L"Sequenced — SeqMessage{DWORD seq; wchar_t data[120]} protocol + queue validation");
    StatusSetR(L"Done");
}

// ── Mode 4: Non-blocking Read ─────────────────────────────────────────────────

void ModeNonBlocking() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Attempt",    80);
    ListAddColumn(1, L"Result",    200);
    ListAddColumn(2, L"Queue Depth",100);
    ListAddColumn(3, L"Notes",     260);
    OutLine(L"=== Non-blocking Mailslot Reads (timeout=0) ===");

    const wchar_t* slotName = L"\\\\.\\mailslot\\Win32Learn_NonBlk";

    // Create with read timeout = 0 (non-blocking: returns immediately)
    HANDLE hSlot = CreateMailslotW(slotName, 0, 0, nullptr);
    if (hSlot == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateMailslot failed: %u", GetLastError());
        return;
    }
    OutLine(L"Mailslot created with timeout=0 (non-blocking)");

    // Attempt 1: read when empty — should fail immediately
    {
        wchar_t buf[256] = {};
        DWORD read = 0;
        BOOL ok = ReadFile(hSlot, buf, sizeof(buf), &read, nullptr);
        DWORD e  = GetLastError();
        wchar_t att[16], res[128], dep[32];
        StringCchCopy(att, 16, L"1 (empty)");
        if (!ok && e == ERROR_SEM_TIMEOUT) {
            StringCchCopy(res, 128, L"MAILSLOT_NO_MESSAGE (timeout=0)");
        } else if (!ok) {
            StringCchPrintf(res, 128, L"FAIL err=%u", e);
        } else {
            StringCchPrintf(res, 128, L"Read %u bytes: '%s'", read, buf);
        }
        StringCchCopy(dep, 32, L"0");
        ListAddRow(att, res, dep, L"Non-blocking read on empty slot");
        OutFmt(L"Attempt 1 (empty): %s", res);
    }

    // Write 3 messages
    HANDLE hW = CreateFileW(slotName,
        GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hW != INVALID_HANDLE_VALUE) {
        for (int i = 0; i < 3; i++) {
            wchar_t msg[64];
            StringCchPrintf(msg, 64, L"NonBlocking msg %d", i + 1);
            DWORD wr = 0;
            WriteFile(hW, msg, (DWORD)((wcslen(msg) + 1) * sizeof(wchar_t)), &wr, nullptr);
        }
        CloseHandle(hW);
    }

    // Read: should succeed immediately
    for (int attempt = 2; attempt <= 5; attempt++) {
        DWORD cnt = 0, nxt = 0;
        GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);

        wchar_t buf[256] = {};
        DWORD read = 0;
        wchar_t att[16]; StringCchPrintf(att, 16, L"%d", attempt);

        BOOL ok = FALSE;
        if (nxt == MAILSLOT_NO_MESSAGE) {
            // No message, try anyway
            ok = ReadFile(hSlot, buf, sizeof(buf), &read, nullptr);
        } else {
            ok = ReadFile(hSlot, buf, sizeof(buf), &read, nullptr);
        }
        DWORD e = GetLastError();

        wchar_t res[128], dep[32];
        if (ok) StringCchPrintf(res, 128, L"'%s' (%u bytes)", buf, read);
        else if (e == ERROR_SEM_TIMEOUT)
            StringCchCopy(res, 128, L"No message (timeout=0, MAILSLOT_NO_MESSAGE)");
        else
            StringCchPrintf(res, 128, L"FAIL err=%u", e);
        StringCchPrintf(dep, 32, L"%u", cnt);

        ListAddRow(att, res, dep, attempt <= 4 ? L"Message available" : L"Queue empty");
        OutFmt(L"Attempt %d (depth=%u): %s", attempt, cnt, res);

        // Rewrite one more for attempt 5
        if (attempt == 3) {
            // intentionally skip to show empty
        }
    }

    // Show message size stats
    OutLine(L"");
    OutLine(L"Testing variable message sizes:");
    HANDLE hW2 = CreateFileW(slotName,
        GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hW2 != INVALID_HANDLE_VALUE) {
        const struct { const wchar_t* m; } varMsgs[] = {
            {L"Small"},
            {L"A medium sized message fits here"},
            {L"This is a longer test message to verify that GetMailslotInfo shows correct sizes"},
        };
        for (auto& vm : varMsgs) {
            DWORD wr = 0;
            DWORD byteCount = (DWORD)((wcslen(vm.m) + 1) * sizeof(wchar_t));
            WriteFile(hW2, vm.m, byteCount, &wr, nullptr);
        }
        CloseHandle(hW2);

        // Drain and show sizes
        DWORD cnt = 0, nxt = 0;
        GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);
        while (cnt > 0 && nxt != MAILSLOT_NO_MESSAGE) {
            wchar_t rbuf[512] = {};
            DWORD rd = 0;
            ReadFile(hSlot, rbuf, sizeof(rbuf), &rd, nullptr);
            OutFmt(L"  Read %u bytes: '%s'", rd, rbuf);
            GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);
        }
    }

    CloseHandle(hSlot);
    StatusSet(L"Non-blocking — CreateMailslot(timeout=0) + MAILSLOT_NO_MESSAGE handling");
    StatusSetR(L"Done");
}

// ── Mode 5: Service Discovery Simulation ──────────────────────────────────────

struct DiscoverThread {
    HWND  hEdit;
    HWND  hMainWnd;
};

DWORD WINAPI DiscoverWorker(LPVOID param) {
    DiscoverThread* ctx = (DiscoverThread*)param;

    auto AppW = [&](const wchar_t* text) {
        int len = GetWindowTextLengthW(ctx->hEdit);
        SendMessage(ctx->hEdit, EM_SETSEL, len, len);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
        SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    };
    auto AppF = [&](const wchar_t* fmt, ...) {
        wchar_t buf[512];
        va_list va; va_start(va, fmt);
        StringCchVPrintfW(buf, 512, fmt, va);
        va_end(va);
        AppW(buf);
    };

    const wchar_t* discoverSlot = L"\\\\.\\mailslot\\Win32Learn_Discovery";
    const wchar_t* responsePipe = L"\\\\.\\pipe\\Win32Learn_SvcResp";

    // Service thread: listen on discovery mailslot
    // Then when DISCOVER message arrives, create a named pipe to send service info

    AppW(L"[Discovery Worker] Waiting for DISCOVER on mailslot...");

    // Open the mailslot (created by main thread) with 2s timeout
    HANDLE hSlot = CreateFileW(discoverSlot,
        GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hSlot == INVALID_HANDLE_VALUE) {
        AppF(L"[Worker] Cannot open discovery slot: err=%u", GetLastError());
        HeapFree(GetProcessHeap(), 0, ctx);
        return 1;
    }

    // Send DISCOVER request
    const wchar_t* discMsg = L"DISCOVER Win32ServiceV1";
    DWORD written = 0;
    WriteFile(hSlot, discMsg,
        (DWORD)((wcslen(discMsg) + 1) * sizeof(wchar_t)), &written, nullptr);
    CloseHandle(hSlot);
    AppF(L"[Discoverer] Sent DISCOVER (%u bytes)", written);

    // Connect to response pipe that server created
    Sleep(100); // give server time to create pipe
    HANDLE hResp = INVALID_HANDLE_VALUE;
    for (int i = 0; i < 5 && hResp == INVALID_HANDLE_VALUE; i++) {
        hResp = CreateFileW(responsePipe,
            GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hResp == INVALID_HANDLE_VALUE) Sleep(100);
    }

    if (hResp != INVALID_HANDLE_VALUE) {
        wchar_t respBuf[512] = {};
        DWORD read = 0;
        ReadFile(hResp, respBuf, sizeof(respBuf), &read, nullptr);
        AppF(L"[Discoverer] Service response: '%s'", respBuf);
        CloseHandle(hResp);
    } else {
        AppW(L"[Discoverer] Could not connect to response pipe");
    }

    HeapFree(GetProcessHeap(), 0, ctx);
    return 0;
}

void ModeServiceDiscovery() {
    OutClear();
    ListClear();
    ListAddColumn(0, L"Step",      180);
    ListAddColumn(1, L"Details",   400);
    ListAddColumn(2, L"Status",    100);
    OutLine(L"=== Service Discovery via Mailslot ===");
    OutLine(L"Server listens on mailslot, client sends DISCOVER, server responds via named pipe");
    OutLine(L"");

    const wchar_t* discoverSlot = L"\\\\.\\mailslot\\Win32Learn_Discovery";
    const wchar_t* responsePipe = L"\\\\.\\pipe\\Win32Learn_SvcResp";

    // Create discovery mailslot (server side)
    HANDLE hSlot = CreateMailslotW(discoverSlot, 0, 2000, nullptr);
    if (hSlot == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateMailslot failed: %u", GetLastError());
        return;
    }
    OutLine(L"Server: Created discovery mailslot");
    ListAddRow(L"1. CreateMailslot", discoverSlot, L"OK");
    ShowMailslotInfo(hSlot, L"Server BEFORE");

    // Launch discoverer thread
    DiscoverThread* ctx = (DiscoverThread*)HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY, sizeof(DiscoverThread));
    ctx->hEdit    = g_hEdit;
    ctx->hMainWnd = g_hwnd;
    HANDLE hThr = CreateThread(nullptr, 0, DiscoverWorker, ctx, 0, nullptr);

    // Server: wait for DISCOVER message (2s timeout)
    Sleep(200); // Give client time to send

    DWORD cnt = 0, nxt = 0;
    GetMailslotInfo(hSlot, nullptr, &nxt, &cnt, nullptr);
    OutFmt(L"Server: GetMailslotInfo after delay — %u messages, next=%u",
        cnt, nxt);
    ShowMailslotInfo(hSlot, L"Server AFTER SEND");

    wchar_t discBuf[256] = {};
    DWORD read = 0;
    BOOL gotDiscover = FALSE;
    if (cnt > 0 && nxt != MAILSLOT_NO_MESSAGE) {
        ReadFile(hSlot, discBuf, sizeof(discBuf), &read, nullptr);
        gotDiscover = TRUE;
        OutFmt(L"Server: Received DISCOVER: '%s'", discBuf);
        ListAddRow(L"2. DISCOVER received", discBuf, L"OK");
    } else {
        OutLine(L"Server: No message yet, trying ReadFile with timeout...");
        SetMailslotInfo(hSlot, 1500);
        ReadFile(hSlot, discBuf, sizeof(discBuf), &read, nullptr);
        gotDiscover = (read > 0);
        if (gotDiscover) {
            OutFmt(L"Server: Received DISCOVER: '%s'", discBuf);
            ListAddRow(L"2. DISCOVER received", discBuf, L"OK");
        } else {
            OutLine(L"Server: No DISCOVER received in time");
            ListAddRow(L"2. DISCOVER received", L"(timeout)", L"FAIL");
        }
    }

    if (gotDiscover) {
        // Parse DISCOVER and create response pipe
        OutLine(L"Server: Creating response pipe...");
        HANDLE hRespPipe = CreateNamedPipeW(responsePipe,
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_WAIT,
            1, 4096, 0, 1000, nullptr);
        ListAddRow(L"3. Response pipe created", responsePipe, L"OK");
        OutLine(L"Server: Waiting for discoverer to connect...");

        ConnectNamedPipe(hRespPipe, nullptr); // blocks until client connects

        // Send service info
        wchar_t svcInfo[256];
        StringCchPrintf(svcInfo, 256,
            L"SERVICE=Win32LearnSvc|VERSION=1.0|PORT=8080|UPTIME=%ums",
            GetTickCount());
        DWORD wr = 0;
        WriteFile(hRespPipe, svcInfo,
            (DWORD)((wcslen(svcInfo) + 1) * sizeof(wchar_t)), &wr, nullptr);
        OutFmt(L"Server: Sent service info (%u bytes)", wr);
        ListAddRow(L"4. Service info sent", svcInfo, L"OK");

        DisconnectNamedPipe(hRespPipe);
        CloseHandle(hRespPipe);
    }

    // Wait for discoverer thread
    if (hThr) {
        WaitForSingleObject(hThr, 3000);
        CloseHandle(hThr);
    }

    CloseHandle(hSlot);
    OutLine(L"");
    OutLine(L"Discovery flow complete: Mailslot → Named Pipe response");
    StatusSet(L"Service Discovery — Mailslot DISCOVER + Named Pipe response");
    StatusSetR(L"Done");
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"Basic",          IDC_BTN_BASIC},
            {L"Multi-slot",     IDC_BTN_MULTI},
            {L"Sequenced",      IDC_BTN_SEQD},
            {L"Non-blocking",   IDC_BTN_NONBLK},
            {L"Svc Discovery",  IDC_BTN_DISCSVC},
        };
        for (int i = 0; i < 5; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 152, 8, 142, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        g_hList = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 45, 800, 200, hwnd,
            (HMENU)(UINT_PTR)IDC_LISTVIEW, g_hInst, nullptr);
        ListView_SetExtendedListViewStyle(g_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        SendMessage(g_hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 250, 800, 260, hwnd,
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
        int listH = (H - 45 - sh) * 42 / 100;
        int editH = H - 45 - sh - listH;
        SetWindowPos(g_hList, nullptr, 0, 45, W, listH, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 45 + listH, W, editH, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_BASIC:   ModeBasic();            break;
        case IDC_BTN_MULTI:   ModeMultiSlot();        break;
        case IDC_BTN_SEQD:    ModeSequenced();        break;
        case IDC_BTN_NONBLK:  ModeNonBlocking();      break;
        case IDC_BTN_DISCSVC: ModeServiceDiscovery(); break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) ModeBasic();
        return 0;

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lp;
        if (nm->hwndFrom == g_hList && nm->code == LVN_COLUMNCLICK) {}
        return 0;
    }

    case WM_DESTROY:
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
    wc.lpszClassName = L"Mslot30Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Mslot30Class",
        L"30 — Mailslot & LPC: IPC Workbench",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 880, 620,
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
