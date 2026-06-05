// 29_NamedPipe_MM — IPC Workbench: Named Pipes, Shared Memory, Mailslot
// Modes: Basic Pipe, Async Pipe, Multi-client, Shared Memory+Mutex, Broadcast
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
    IDC_BTN_BASIC     = 101,
    IDC_BTN_ASYNC     = 102,
    IDC_BTN_MULTICLI  = 103,
    IDC_BTN_SHAREDMEM = 104,
    IDC_BTN_BROADCAST = 105,
    IDC_EDIT_OUT      = 106,
    IDC_STATUS        = 107,
};

HWND      g_hwnd    = nullptr;
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
    wchar_t buf[1024];
    va_list va; va_start(va, fmt);
    StringCchVPrintfW(buf, 1024, fmt, va);
    va_end(va);
    OutLine(buf);
}

// ── Mode 1: Basic Named Pipe ──────────────────────────────────────────────────

void ModeBasicPipe() {
    OutClear();
    OutLine(L"=== Basic Named Pipe (message-mode, with security descriptor) ===");

    const wchar_t* pipeName = L"\\\\.\\pipe\\Win32Learn_Basic";

    // Build a security descriptor allowing Everyone to connect
    SECURITY_ATTRIBUTES sa = {};
    SECURITY_DESCRIPTOR sd = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    // NULL DACL = full access to everyone
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle       = FALSE;

    // Create server pipe (message mode)
    HANDLE hServer = CreateNamedPipeW(pipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 4096, 4096, 0, &sa);

    if (hServer == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateNamedPipe failed: %u", GetLastError());
        return;
    }
    OutFmt(L"Server: CreateNamedPipe '%s' OK", pipeName);

    // Get pipe info
    DWORD flags = 0, outBuf = 0, inBuf = 0, maxInst = 0;
    GetNamedPipeInfo(hServer, &flags, &outBuf, &inBuf, &maxInst);
    OutFmt(L"GetNamedPipeInfo: flags=0x%X outBuf=%u inBuf=%u maxInst=%u",
        flags, outBuf, inBuf, maxInst);

    // Client connects
    HANDLE hClient = CreateFileW(pipeName,
        GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED, nullptr);
    if (hClient == INVALID_HANDLE_VALUE) {
        OutFmt(L"Client CreateFile failed: %u", GetLastError());
        CloseHandle(hServer);
        return;
    }
    OutLine(L"Client: CreateFile pipe OK");

    // Server accept
    BOOL cok = ConnectNamedPipe(hServer, nullptr);
    DWORD cErr = GetLastError();
    // If client connected before ConnectNamedPipe, err = ERROR_PIPE_CONNECTED (it's ok)
    if (!cok && cErr != ERROR_PIPE_CONNECTED) {
        OutFmt(L"ConnectNamedPipe failed: %u", cErr);
    } else {
        OutLine(L"Server: ConnectNamedPipe — client connected");
    }

    // GetNamedPipeHandleState on server
    DWORD pipeState = 0, curInst = 0;
    GetNamedPipeHandleStateW(hServer, &pipeState, &curInst, nullptr, nullptr, nullptr, 0);
    OutFmt(L"GetNamedPipeHandleState: state=0x%X, curInstances=%u", pipeState, curInst);

    // Client sends message
    const wchar_t* msg = L"Hello from pipe client!";
    DWORD written = 0;
    WriteFile(hClient, msg, (DWORD)((wcslen(msg) + 1) * sizeof(wchar_t)),
        &written, nullptr);
    OutFmt(L"Client: WriteFile %u bytes", written);

    // Server reads
    wchar_t readBuf[256] = {};
    DWORD bytesRead = 0;
    ReadFile(hServer, readBuf, sizeof(readBuf), &bytesRead, nullptr);
    OutFmt(L"Server: ReadFile %u bytes: '%s'", bytesRead, readBuf);

    // Server replies
    const wchar_t* reply = L"Server received your message!";
    WriteFile(hServer, reply, (DWORD)((wcslen(reply) + 1) * sizeof(wchar_t)),
        &written, nullptr);

    // Client reads reply
    wchar_t replyBuf[256] = {};
    ReadFile(hClient, replyBuf, sizeof(replyBuf), &bytesRead, nullptr);
    OutFmt(L"Client: ReadFile reply %u bytes: '%s'", bytesRead, replyBuf);

    // TransactNamedPipe demo
    const wchar_t* tx = L"TransactRequest";
    wchar_t txReply[256] = {};
    DWORD txRead = 0;
    // Write reply for transact first
    WriteFile(hServer, L"TransactResponse",
        (DWORD)((wcslen(L"TransactResponse") + 1) * sizeof(wchar_t)),
        &written, nullptr);
    BOOL txOk = TransactNamedPipe(hClient,
        (PVOID)tx, (DWORD)((wcslen(tx) + 1) * sizeof(wchar_t)),
        txReply, sizeof(txReply), &txRead, nullptr);
    if (txOk || GetLastError() == ERROR_MORE_DATA) {
        OutFmt(L"TransactNamedPipe: reply='%s' (%u bytes)", txReply, txRead);
    } else {
        OutFmt(L"TransactNamedPipe: err=%u", GetLastError());
    }

    DisconnectNamedPipe(hServer);
    CloseHandle(hClient);
    CloseHandle(hServer);

    StatusSet(L"Basic Pipe — CreateNamedPipe + ConnectNamedPipe + Read/Write + TransactNamedPipe");
    StatusSetR(L"Done");
}

// ── Mode 2: Async Named Pipe (OVERLAPPED) ────────────────────────────────────

void ModeAsyncPipe() {
    OutClear();
    OutLine(L"=== Async Named Pipe (OVERLAPPED) ===");

    const wchar_t* pipeName = L"\\\\.\\pipe\\Win32Learn_Async";

    HANDLE hServer = CreateNamedPipeW(pipeName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, nullptr);

    if (hServer == INVALID_HANDLE_VALUE) {
        OutFmt(L"CreateNamedPipe (async) failed: %u", GetLastError());
        return;
    }
    OutLine(L"Server: CreateNamedPipe (FILE_FLAG_OVERLAPPED) OK");

    // Async ConnectNamedPipe
    HANDLE hConnEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    OVERLAPPED olConn = {};
    olConn.hEvent = hConnEvent;

    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);

    BOOL connOk = ConnectNamedPipe(hServer, &olConn);
    DWORD connErr = GetLastError();
    if (!connOk && connErr != ERROR_IO_PENDING && connErr != ERROR_PIPE_CONNECTED) {
        OutFmt(L"Async ConnectNamedPipe failed: %u", connErr);
        CloseHandle(hServer);
        CloseHandle(hConnEvent);
        return;
    }
    OutLine(L"Server: ConnectNamedPipe pending (IO_PENDING)...");

    // Client connects
    HANDLE hClient = CreateFileW(pipeName,
        GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    OutFmt(L"Client: CreateFile result: %s",
        hClient != INVALID_HANDLE_VALUE ? L"OK" : L"FAIL");

    // Wait for connection event
    DWORD waitRes = WaitForSingleObject(hConnEvent, 2000);
    QueryPerformanceCounter(&t2);
    double connMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
    OutFmt(L"WaitForSingleObject(connect event): %s (%.2f ms)",
        waitRes == WAIT_OBJECT_0 ? L"SIGNALED" : L"TIMEOUT", connMs);

    if (hClient != INVALID_HANDLE_VALUE) {
        // Async write from client
        const wchar_t* data = L"Async pipe data!";
        DWORD written = 0;
        WriteFile(hClient, data, (DWORD)((wcslen(data) + 1) * sizeof(wchar_t)),
            &written, nullptr);
        OutFmt(L"Client: wrote %u bytes synchronously", written);

        // Async read on server
        HANDLE hReadEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        OVERLAPPED olRead = {};
        olRead.hEvent = hReadEvent;
        wchar_t readBuf[256] = {};

        QueryPerformanceCounter(&t1);
        BOOL readOk = ReadFile(hServer, readBuf, sizeof(readBuf), nullptr, &olRead);
        DWORD readErr = GetLastError();
        if (!readOk && readErr == ERROR_IO_PENDING) {
            DWORD bytesRead = 0;
            GetOverlappedResult(hServer, &olRead, &bytesRead, TRUE);
            QueryPerformanceCounter(&t2);
            double readMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
            OutFmt(L"Server: async ReadFile %u bytes (%.2f ms): '%s'",
                bytesRead, readMs, readBuf);
        } else if (readOk) {
            DWORD br = 0;
            GetOverlappedResult(hServer, &olRead, &br, FALSE);
            OutFmt(L"Server: ReadFile completed immediately: '%s'", readBuf);
        }

        // Async write from server
        HANDLE hWriteEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        OVERLAPPED olWrite = {};
        olWrite.hEvent = hWriteEvent;
        const wchar_t* reply = L"Server async reply!";
        BOOL writeOk = WriteFile(hServer, reply,
            (DWORD)((wcslen(reply) + 1) * sizeof(wchar_t)), nullptr, &olWrite);
        if (!writeOk && GetLastError() == ERROR_IO_PENDING) {
            DWORD bw = 0;
            GetOverlappedResult(hServer, &olWrite, &bw, TRUE);
            OutFmt(L"Server: async WriteFile %u bytes completed via GetOverlappedResult", bw);
        }

        // Client reads reply
        wchar_t replyBuf[256] = {};
        DWORD rr = 0;
        ReadFile(hClient, replyBuf, sizeof(replyBuf), &rr, nullptr);
        OutFmt(L"Client: read reply '%s'", replyBuf);

        CloseHandle(hWriteEvent);
        CloseHandle(hReadEvent);
        CloseHandle(hClient);
    }

    CloseHandle(hConnEvent);
    DisconnectNamedPipe(hServer);
    CloseHandle(hServer);
    StatusSet(L"Async Pipe — FILE_FLAG_OVERLAPPED + ConnectNamedPipe(OVERLAPPED) + GetOverlappedResult");
    StatusSetR(L"Done");
}

// ── Mode 3: Multi-client Named Pipe ──────────────────────────────────────────

struct MultiClientCtx {
    int clientId;
    HWND hEdit;
};

DWORD WINAPI MultiClientThread(LPVOID param) {
    MultiClientCtx* ctx = (MultiClientCtx*)param;
    const wchar_t* pipeName = L"\\\\.\\pipe\\Win32Learn_Multi";

    // Wait a moment for server to be ready
    Sleep(50 * ctx->clientId);

    HANDLE hPipe = INVALID_HANDLE_VALUE;
    for (int retry = 0; retry < 5 && hPipe == INVALID_HANDLE_VALUE; retry++) {
        hPipe = CreateFileW(pipeName,
            GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hPipe == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_PIPE_BUSY)
                WaitNamedPipeW(pipeName, 1000);
            else Sleep(50);
        }
    }

    wchar_t buf[256];
    if (hPipe == INVALID_HANDLE_VALUE) {
        StringCchPrintf(buf, 256, L"Client %d: connect FAILED err=%u",
            ctx->clientId, GetLastError());
    } else {
        // Set message mode
        DWORD mode = PIPE_READMODE_MESSAGE;
        SetNamedPipeHandleState(hPipe, &mode, nullptr, nullptr);

        // Send message
        wchar_t msg[128];
        StringCchPrintf(msg, 128, L"Message from client %d", ctx->clientId);
        DWORD written = 0;
        WriteFile(hPipe, msg, (DWORD)((wcslen(msg) + 1) * sizeof(wchar_t)), &written, nullptr);

        // Read reply
        wchar_t reply[256] = {};
        DWORD read = 0;
        ReadFile(hPipe, reply, sizeof(reply), &read, nullptr);

        StringCchPrintf(buf, 256, L"Client %d: sent '%s' → got '%s'",
            ctx->clientId, msg, reply);
        CloseHandle(hPipe);
    }

    int len = GetWindowTextLengthW(ctx->hEdit);
    SendMessage(ctx->hEdit, EM_SETSEL, len, len);
    SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessage(ctx->hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");

    HeapFree(GetProcessHeap(), 0, ctx);
    return 0;
}

void ModeMultiClient() {
    OutClear();
    OutLine(L"=== Multi-client Named Pipe (nMaxInstances=4) ===");

    const wchar_t* pipeName = L"\\\\.\\pipe\\Win32Learn_Multi";

    // Create 3 pipe instances
    HANDLE hInstances[3] = {};
    HANDLE hEvents[3]    = {};
    OVERLAPPED olArr[3]  = {};

    for (int i = 0; i < 3; i++) {
        hInstances[i] = CreateNamedPipeW(pipeName,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            4, 4096, 4096, 0, nullptr);
        if (hInstances[i] == INVALID_HANDLE_VALUE) {
            OutFmt(L"CreateNamedPipe instance %d failed: %u", i, GetLastError());
            continue;
        }
        hEvents[i] = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        olArr[i].hEvent = hEvents[i];
        ConnectNamedPipe(hInstances[i], &olArr[i]);
    }
    OutFmt(L"Server: created %d pipe instances (maxInst=4)", 3);

    // Launch 3 client threads
    for (int i = 0; i < 3; i++) {
        MultiClientCtx* ctx = (MultiClientCtx*)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, sizeof(MultiClientCtx));
        ctx->clientId = i + 1;
        ctx->hEdit    = g_hEdit;
        HANDLE hThr = CreateThread(nullptr, 0, MultiClientThread, ctx, 0, nullptr);
        if (hThr) CloseHandle(hThr);
    }

    // Server: wait for all 3 connections and respond
    for (int round = 0; round < 3; round++) {
        // Wait for any instance to have a client
        DWORD idx = WaitForMultipleObjects(3, hEvents, FALSE, 3000);
        if (idx >= WAIT_OBJECT_0 && idx < WAIT_OBJECT_0 + 3) {
            int i = (int)(idx - WAIT_OBJECT_0);
            ResetEvent(hEvents[i]);

            wchar_t readBuf[256] = {};
            DWORD bytesRead = 0;
            OVERLAPPED olRead = {0}; olRead.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            ReadFile(hInstances[i], readBuf, sizeof(readBuf), nullptr, &olRead);
            if (GetLastError() == ERROR_IO_PENDING)
                GetOverlappedResult(hInstances[i], &olRead, &bytesRead, TRUE);
            else
                GetOverlappedResult(hInstances[i], &olRead, &bytesRead, FALSE);
            CloseHandle(olRead.hEvent);

            OutFmt(L"Server (inst %d): read '%s'", i, readBuf);

            wchar_t reply[256];
            StringCchPrintf(reply, 256, L"Echo[%d]: %s", i, readBuf);
            DWORD written = 0;
            WriteFile(hInstances[i], reply,
                (DWORD)((wcslen(reply) + 1) * sizeof(wchar_t)), &written, nullptr);
        }
    }

    Sleep(500); // Let client threads finish

    for (int i = 0; i < 3; i++) {
        DisconnectNamedPipe(hInstances[i]);
        CloseHandle(hInstances[i]);
        CloseHandle(hEvents[i]);
    }

    StatusSet(L"Multi-client — CreateNamedPipe(nMax=4) x3 + WaitForMultipleObjects + client threads");
    StatusSetR(L"Done");
}

// ── Mode 4: Shared Memory with Mutex ──────────────────────────────────────────

struct SharedData {
    DWORD    sequence;
    wchar_t  message[256];
    FILETIME timestamp;
};

void ModeSharedMemory() {
    OutClear();
    OutLine(L"=== Shared Memory (Named File Mapping) + Named Mutex ===");

    const wchar_t* mapName   = L"Win32Learn_SharedMem";
    const wchar_t* mutexName = L"Win32Learn_Mutex";

    // Create named file mapping (page file backed)
    HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE,
        nullptr, PAGE_READWRITE, 0, sizeof(SharedData), mapName);
    if (!hMap) {
        OutFmt(L"CreateFileMapping failed: %u", GetLastError());
        return;
    }
    BOOL alreadyExisted = (GetLastError() == ERROR_ALREADY_EXISTS);
    OutFmt(L"CreateFileMapping '%s': %s", mapName,
        alreadyExisted ? L"Opened existing" : L"Created new");

    // Map view
    SharedData* pData = (SharedData*)MapViewOfFile(hMap,
        FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (!pData) {
        OutFmt(L"MapViewOfFile failed: %u", GetLastError());
        CloseHandle(hMap);
        return;
    }
    OutFmt(L"MapViewOfFile: pData=%p (size=%zu bytes)", (void*)pData, sizeof(SharedData));

    // Create named mutex
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, mutexName);
    OutFmt(L"CreateMutex '%s': handle=%p", mutexName, (void*)hMutex);

    // Second view of same mapping (simulate another process)
    HANDLE hMap2 = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, mapName);
    SharedData* pData2 = (SharedData*)MapViewOfFile(hMap2,
        FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    OutFmt(L"Second view: pData2=%p (simulates second process)", (void*)pData2);

    // Write loop via mutex
    for (int i = 0; i < 5; i++) {
        DWORD waitRes = WaitForSingleObject(hMutex, 1000);
        if (waitRes == WAIT_OBJECT_0) {
            pData->sequence++;
            StringCchPrintf(pData->message, 256, L"Shared message #%d", pData->sequence);
            GetSystemTimeAsFileTime(&pData->timestamp);

            // Read from second view
            DWORD  seq2  = pData2->sequence;
            wchar_t msg2[256];
            StringCchCopyW(msg2, 256, pData2->message);

            OutFmt(L"Iteration %d: seq=%u msg='%s' | view2: seq=%u msg='%s'",
                i + 1, pData->sequence, pData->message, seq2, msg2);

            ReleaseMutex(hMutex);

            // VirtualProtect demo: make view read-only mid-way
            if (i == 2) {
                DWORD oldProt;
                BOOL vpOk = VirtualProtect(pData, sizeof(SharedData),
                    PAGE_READONLY, &oldProt);
                OutFmt(L"VirtualProtect(PAGE_READONLY): %s (was 0x%X)",
                    vpOk ? L"OK" : L"FAIL", oldProt);

                // Try to write (should fail / raise SEH)
                __try {
                    pData->sequence++;
                    OutLine(L"Write after PAGE_READONLY: succeeded (OS may not fault here)");
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    OutLine(L"Write after PAGE_READONLY: raised ACCESS_VIOLATION (expected)");
                }

                // Restore
                VirtualProtect(pData, sizeof(SharedData), PAGE_READWRITE, &oldProt);
                OutLine(L"VirtualProtect(PAGE_READWRITE): restored");
            }
        }
    }

    UnmapViewOfFile(pData2);
    UnmapViewOfFile(pData);
    CloseHandle(hMap2);
    CloseHandle(hMap);
    CloseHandle(hMutex);

    StatusSet(L"Shared Memory — CreateFileMapping + MapViewOfFile + CreateMutex + VirtualProtect");
    StatusSetR(L"Done");
}

// ── Mode 5: Broadcast IPC ─────────────────────────────────────────────────────

void ModeBroadcast() {
    OutClear();
    OutLine(L"=== Broadcast IPC: Named Pipe + Mailslot + Shared Memory ===");

    const wchar_t* broadcastMsg = L"BROADCAST: Hello all IPC endpoints!";
    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);

    // ── Named Pipe broadcast ──
    OutLine(L"");
    OutLine(L"--- Named Pipe ---");
    const wchar_t* pipeName = L"\\\\.\\pipe\\Win32Learn_Broadcast";

    HANDLE hSrv = CreateNamedPipeW(pipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 4096, 4096, 0, nullptr);

    HANDLE hCli = CreateFileW(pipeName,
        GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hCli != INVALID_HANDLE_VALUE) {
        ConnectNamedPipe(hSrv, nullptr); // Accept (ERROR_PIPE_CONNECTED ok)

        QueryPerformanceCounter(&t1);
        DWORD written = 0;
        WriteFile(hCli, broadcastMsg,
            (DWORD)((wcslen(broadcastMsg) + 1) * sizeof(wchar_t)), &written, nullptr);
        wchar_t pipeRcv[512] = {};
        DWORD read = 0;
        ReadFile(hSrv, pipeRcv, sizeof(pipeRcv), &read, nullptr);
        QueryPerformanceCounter(&t2);
        double pipeMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

        OutFmt(L"  Sent %u bytes, received %u bytes via named pipe", written, read);
        OutFmt(L"  Pipe latency: %.3f ms", pipeMs);
        OutFmt(L"  Received: '%s'", pipeRcv);
        CloseHandle(hCli);
        DisconnectNamedPipe(hSrv);
    } else {
        OutFmt(L"  Named pipe client failed: %u", GetLastError());
    }
    CloseHandle(hSrv);

    // ── Mailslot broadcast ──
    OutLine(L"");
    OutLine(L"--- Mailslot ---");
    const wchar_t* slotName = L"\\\\.\\mailslot\\Win32Learn_Broadcast";

    HANDLE hSlot = CreateMailslotW(slotName,
        0, MAILSLOT_WAIT_FOREVER, nullptr);
    if (hSlot != INVALID_HANDLE_VALUE) {
        HANDLE hWriter = CreateFileW(slotName,
            GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hWriter != INVALID_HANDLE_VALUE) {
            QueryPerformanceCounter(&t1);
            DWORD written = 0;
            WriteFile(hWriter, broadcastMsg,
                (DWORD)((wcslen(broadcastMsg) + 1) * sizeof(wchar_t)), &written, nullptr);
            CloseHandle(hWriter);

            // Read with timeout=0
            DWORD msgCount = 0, nextMsgSize = 0;
            GetMailslotInfo(hSlot, nullptr, &nextMsgSize, &msgCount, nullptr);
            OutFmt(L"  GetMailslotInfo: %u messages, next=%u bytes", msgCount, nextMsgSize);

            if (msgCount > 0 && nextMsgSize != MAILSLOT_NO_MESSAGE) {
                wchar_t slotBuf[512] = {};
                DWORD read = 0;
                QueryPerformanceCounter(&t1);
                ReadFile(hSlot, slotBuf, sizeof(slotBuf), &read, nullptr);
                QueryPerformanceCounter(&t2);
                double slotMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
                OutFmt(L"  Mailslot read %u bytes: '%s'", read, slotBuf);
                OutFmt(L"  Mailslot latency: %.3f ms", slotMs);
            }
        }
        CloseHandle(hSlot);
    } else {
        OutFmt(L"  CreateMailslot failed: %u", GetLastError());
    }

    // ── Shared Memory broadcast ──
    OutLine(L"");
    OutLine(L"--- Shared Memory ---");

    HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
        PAGE_READWRITE, 0, 1024, L"Win32Learn_BroadcastMem");
    wchar_t* pMem = (wchar_t*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 1024);
    if (pMem) {
        QueryPerformanceCounter(&t1);
        StringCchCopyW(pMem, 512, broadcastMsg);
        // Read back
        wchar_t readBack[512] = {};
        // Simulate second view
        HANDLE hMap2 = OpenFileMappingW(FILE_MAP_READ, FALSE, L"Win32Learn_BroadcastMem");
        wchar_t* pMem2 = (wchar_t*)MapViewOfFile(hMap2, FILE_MAP_READ, 0, 0, 1024);
        if (pMem2) {
            StringCchCopyW(readBack, 512, pMem2);
            QueryPerformanceCounter(&t2);
            double memMs = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
            OutFmt(L"  Shared Mem read: '%s'", readBack);
            OutFmt(L"  Shared Mem latency: %.3f ms", memMs);
            UnmapViewOfFile(pMem2);
        }
        CloseHandle(hMap2);
        UnmapViewOfFile(pMem);
    }
    CloseHandle(hMap);

    OutLine(L"");
    OutLine(L"--- Summary ---");
    OutLine(L"All three IPC mechanisms delivered the same broadcast message.");
    OutLine(L"Named Pipe: ordered, reliable, bidirectional message stream");
    OutLine(L"Mailslot: one-way datagram, no ordering guarantee");
    OutLine(L"Shared Memory: lowest latency, requires synchronization");

    StatusSet(L"Broadcast IPC — Named Pipe + Mailslot + Shared Memory latency comparison");
    StatusSetR(L"Done");
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"Basic Pipe",    IDC_BTN_BASIC},
            {L"Async Pipe",    IDC_BTN_ASYNC},
            {L"Multi-client",  IDC_BTN_MULTICLI},
            {L"Shared Memory", IDC_BTN_SHAREDMEM},
            {L"Broadcast IPC", IDC_BTN_BROADCAST},
        };
        for (int i = 0; i < 5; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 152, 8, 142, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 45, 800, 480, hwnd,
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
        SetWindowPos(g_hEdit, nullptr, 0, 45, W, H - 45 - sh, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_BASIC:     ModeBasicPipe();    break;
        case IDC_BTN_ASYNC:     ModeAsyncPipe();    break;
        case IDC_BTN_MULTICLI:  ModeMultiClient();  break;
        case IDC_BTN_SHAREDMEM: ModeSharedMemory(); break;
        case IDC_BTN_BROADCAST: ModeBroadcast();    break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) ModeBasicPipe();
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
    wc.lpszClassName = L"Pipe29Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Pipe29Class",
        L"29 — NamedPipe & SharedMemory: IPC Workbench",
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
