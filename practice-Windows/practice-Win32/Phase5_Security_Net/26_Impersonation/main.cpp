// 26_Impersonation — Token Manipulation & DPAPI
// Demos: Basic Flow, All Levels, Restricted Token, Thread Token, DPAPI, Token Statistics
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "crypt32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <sddl.h>
#include <wincrypt.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

enum {
    IDC_BTN_BASIC    = 101,
    IDC_BTN_LEVELS   = 102,
    IDC_BTN_RESTRICT = 103,
    IDC_BTN_THREAD   = 104,
    IDC_BTN_DPAPI    = 105,
    IDC_BTN_STATS    = 106,
    IDC_LISTVIEW     = 107,
    IDC_EDIT_OUT     = 108,
    IDC_STATUS       = 109,
};

HWND      g_hwnd      = nullptr;
HWND      g_hList     = nullptr;
HWND      g_hEdit     = nullptr;   // raw output edit
HWND      g_hStatus   = nullptr;
HINSTANCE g_hInst     = nullptr;
int       g_itemCount = 0;

// ── UI Helpers ───────────────────────────────────────────────────────────────

void StatusSet(const wchar_t* msg, int count = -1) {
    SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)msg);
    if (count >= 0) {
        wchar_t buf[64];
        StringCchPrintf(buf, 64, L"Items: %d", count);
        SendMessage(g_hStatus, SB_SETTEXT, 1, (LPARAM)buf);
    }
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

int ListAddRow(const wchar_t* f, const wchar_t* v, const wchar_t* n = L"", const wchar_t* c3 = L"") {
    LVITEM lvi = {};
    lvi.mask   = LVIF_TEXT;
    lvi.iItem  = g_itemCount++;
    lvi.pszText = const_cast<wchar_t*>(f);
    int row = ListView_InsertItem(g_hList, &lvi);
    if (v && v[0]) ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(v));
    if (n && n[0]) ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(n));
    if (c3 && c3[0]) ListView_SetItemText(g_hList, row, 3, const_cast<wchar_t*>(c3));
    return row;
}

void EditAppend(const wchar_t* text) {
    int len = GetWindowTextLengthW(g_hEdit);
    SendMessage(g_hEdit, EM_SETSEL, len, len);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(g_hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
}

void EditClear() {
    SetWindowTextW(g_hEdit, L"");
}

void SidToDisplayName(PSID sid, wchar_t* out, DWORD cch) {
    wchar_t name[128] = {}, domain[128] = {};
    DWORD nl = 128, dl = 128;
    SID_NAME_USE use;
    if (LookupAccountSidW(nullptr, sid, name, &nl, domain, &dl, &use)) {
        if (domain[0]) StringCchPrintf(out, cch, L"%s\\%s", domain, name);
        else           StringCchCopy(out, cch, name);
    } else {
        LPWSTR s = nullptr;
        if (ConvertSidToStringSidW(sid, &s)) {
            StringCchCopy(out, cch, s);
            LocalFree(s);
        } else StringCchCopy(out, cch, L"(unknown)");
    }
}

// ── Mode 1: Basic Impersonation Flow ─────────────────────────────────────────

void DemoBasicFlow() {
    ListClear();
    ListAddColumn(0, L"Field",  200);
    ListAddColumn(1, L"Value",  360);
    ListAddColumn(2, L"Notes",  180);

    EditClear();
    EditAppend(L"=== Basic Impersonation Flow ===");

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        StatusSet(L"OpenProcessToken failed", 0);
        return;
    }

    // Get original user
    BYTE buf[512]; DWORD needed;
    GetTokenInformation(hToken, TokenUser, buf, sizeof(buf), &needed);
    TOKEN_USER* tu = (TOKEN_USER*)buf;
    wchar_t origUser[256];
    SidToDisplayName(tu->User.Sid, origUser, 256);
    ListAddRow(L"Original User", origUser, L"Before impersonation");
    EditAppend(L"Original user obtained via OpenProcessToken + GetTokenInformation(TokenUser)");

    // Duplicate to impersonation token
    HANDLE hImpToken = nullptr;
    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, nullptr,
            SecurityImpersonation, TokenImpersonation, &hImpToken)) {
        ListAddRow(L"DuplicateTokenEx", L"FAILED", L"Cannot create imp token");
        CloseHandle(hToken);
        StatusSet(L"DuplicateTokenEx failed", g_itemCount);
        return;
    }
    ListAddRow(L"DuplicateTokenEx", L"Success", L"Created SecurityImpersonation token");
    EditAppend(L"DuplicateTokenEx(SecurityImpersonation, TokenImpersonation) succeeded");

    // Set thread token (impersonate)
    if (SetThreadToken(nullptr, hImpToken)) {
        ListAddRow(L"SetThreadToken(self)", L"Success", L"Now impersonating");
        EditAppend(L"SetThreadToken(nullptr, hImpToken) — thread now impersonates");

        // Verify: open own thread token
        HANDLE hThreadTok = nullptr;
        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadTok)) {
            BYTE tb[512]; DWORD tn;
            GetTokenInformation(hThreadTok, TokenUser, tb, sizeof(tb), &tn);
            TOKEN_USER* ttu = (TOKEN_USER*)tb;
            wchar_t impUser[256];
            SidToDisplayName(ttu->User.Sid, impUser, 256);
            ListAddRow(L"Thread Token User", impUser, L"OpenThreadToken confirmed");
            CloseHandle(hThreadTok);
        }

        // Revert
        RevertToSelf();
        ListAddRow(L"RevertToSelf()", L"Success", L"Reverted to process token");
        EditAppend(L"RevertToSelf() — thread token cleared");
    } else {
        wchar_t err[64];
        StringCchPrintf(err, 64, L"err=%u", GetLastError());
        ListAddRow(L"SetThreadToken", err, L"Failed");
    }

    // ImpersonateSelf demo
    if (ImpersonateSelf(SecurityIdentification)) {
        ListAddRow(L"ImpersonateSelf(Identification)", L"Success",
            L"Thread gets copy of process token");
        EditAppend(L"ImpersonateSelf(SecurityIdentification) succeeded");

        HANDLE hSelf = nullptr;
        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hSelf)) {
            TOKEN_TYPE ttype; DWORD tn;
            GetTokenInformation(hSelf, TokenType, &ttype, sizeof(ttype), &tn);
            ListAddRow(L"ImpersonateSelf Token Type",
                ttype == TokenImpersonation ? L"Impersonation" : L"Primary",
                L"Expected: Impersonation");
            CloseHandle(hSelf);
        }
        RevertToSelf();
        ListAddRow(L"RevertToSelf (2)", L"Success", L"After ImpersonateSelf");
    }

    CloseHandle(hImpToken);
    CloseHandle(hToken);
    StatusSet(L"Basic Flow — DuplicateTokenEx + SetThreadToken + ImpersonateSelf", g_itemCount);
}

// ── Mode 2: All Impersonation Levels ─────────────────────────────────────────

void DemoAllLevels() {
    ListClear();
    ListAddColumn(0, L"Level",      180);
    ListAddColumn(1, L"SetThread?", 100);
    ListAddColumn(2, L"OpenThread?",110);
    ListAddColumn(3, L"Notes",      260);
    EditClear();
    EditAppend(L"=== All Impersonation Levels ===");

    HANDLE hProcToken = nullptr;
    OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, &hProcToken);

    const struct { SECURITY_IMPERSONATION_LEVEL lvl; const wchar_t* name; } levels[] = {
        {SecurityAnonymous,      L"SecurityAnonymous"},
        {SecurityIdentification, L"SecurityIdentification"},
        {SecurityImpersonation,  L"SecurityImpersonation"},
        {SecurityDelegation,     L"SecurityDelegation"},
    };

    for (auto& lv : levels) {
        HANDLE hImp = nullptr;
        BOOL dupOk = DuplicateTokenEx(hProcToken, TOKEN_ALL_ACCESS, nullptr,
            lv.lvl, TokenImpersonation, &hImp);

        if (!dupOk) {
            wchar_t ev[64];
            StringCchPrintf(ev, 64, L"DupFail err=%u", GetLastError());
            ListAddRow(lv.name, L"N/A", L"N/A", ev);
            continue;
        }

        BOOL setOk = SetThreadToken(nullptr, hImp);
        DWORD setErr = GetLastError();

        HANDLE hThr = nullptr;
        BOOL openOk = FALSE;
        if (setOk) {
            openOk = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThr);
            if (hThr) CloseHandle(hThr);
            RevertToSelf();
        }

        wchar_t note[128];
        if (lv.lvl == SecurityAnonymous)
            StringCchCopy(note, 128, L"Can't open own thread token");
        else if (lv.lvl == SecurityIdentification)
            StringCchCopy(note, 128, L"Can query, cannot impersonate resources");
        else if (lv.lvl == SecurityImpersonation)
            StringCchCopy(note, 128, L"Full local impersonation");
        else
            StringCchCopy(note, 128, L"Network impersonation (Kerberos)");

        wchar_t setStr[32], openStr[32];
        if (!setOk) StringCchPrintf(setStr, 32, L"NO(err=%u)", setErr);
        else        StringCchCopy(setStr, 32, L"YES");
        StringCchCopy(openStr, 32, openOk ? L"YES" : L"NO");

        ListAddRow(lv.name, setStr, openStr, note);
        CloseHandle(hImp);

        wchar_t editLine[256];
        StringCchPrintf(editLine, 256, L"Level %s: set=%s open=%s",
            lv.name, setStr, openStr);
        EditAppend(editLine);
    }

    CloseHandle(hProcToken);
    StatusSet(L"All Levels — DuplicateTokenEx with each SECURITY_IMPERSONATION_LEVEL", g_itemCount);
}

// ── Mode 3: Restricted Token ─────────────────────────────────────────────────

void DemoRestrictedToken() {
    ListClear();
    ListAddColumn(0, L"Property",  200);
    ListAddColumn(1, L"Value",     360);
    ListAddColumn(2, L"Notes",     190);
    EditClear();
    EditAppend(L"=== Restricted Token ===");

    HANDLE hToken = nullptr;
    OpenProcessToken(GetCurrentProcess(),
        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ADJUST_GROUPS, &hToken);

    // CreateRestrictedToken: remove all privileges, add restricting SID
    // Restricting SID = Everyone (S-1-1-0)
    PSID pEveryoneSid = nullptr;
    SID_IDENTIFIER_AUTHORITY worldAuth = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID,
        0,0,0,0,0,0,0, &pEveryoneSid);

    SID_AND_ATTRIBUTES restricting[1];
    restricting[0].Sid        = pEveryoneSid;
    restricting[0].Attributes = 0;

    HANDLE hRestricted = nullptr;
    BOOL ok = CreateRestrictedToken(hToken,
        DISABLE_MAX_PRIVILEGE,  // remove most privileges
        0, nullptr,             // no SIDs to deny
        0, nullptr,             // no privileges to delete
        1, restricting,         // one restricting SID
        &hRestricted);

    wchar_t tmp[256];
    if (ok) {
        ListAddRow(L"CreateRestrictedToken", L"SUCCESS",
            L"DISABLE_MAX_PRIVILEGE + restricting SID");
        EditAppend(L"CreateRestrictedToken: DISABLE_MAX_PRIVILEGE + Everyone restricting SID");

        // Check IsTokenRestricted
        BOOL isRestr = IsTokenRestricted(hRestricted);
        ListAddRow(L"IsTokenRestricted", isRestr ? L"TRUE" : L"FALSE",
            L"Should be TRUE");

        // Compare privilege counts
        DWORD needed;
        BYTE b1[4096], b2[4096];
        GetTokenInformation(hToken, TokenPrivileges, b1, sizeof(b1), &needed);
        GetTokenInformation(hRestricted, TokenPrivileges, b2, sizeof(b2), &needed);
        DWORD origCount = ((TOKEN_PRIVILEGES*)b1)->PrivilegeCount;
        DWORD restrCount= ((TOKEN_PRIVILEGES*)b2)->PrivilegeCount;
        StringCchPrintf(tmp, 256, L"Original=%u, Restricted=%u", origCount, restrCount);
        ListAddRow(L"Privilege Count", tmp, L"Should be fewer");
        EditAppend(tmp);

        // Impersonate restricted token
        HANDLE hImpRestr = nullptr;
        DuplicateToken(hRestricted, SecurityImpersonation, &hImpRestr);
        if (SetThreadToken(nullptr, hImpRestr)) {
            ListAddRow(L"Impersonate restricted", L"OK", L"SetThreadToken with restricted");
            RevertToSelf();
        }
        CloseHandle(hImpRestr);
        CloseHandle(hRestricted);
    } else {
        StringCchPrintf(tmp, 256, L"FAILED err=%u", GetLastError());
        ListAddRow(L"CreateRestrictedToken", tmp, L"");
    }

    // NtCreateLowBoxToken analogue: demonstrate privilege removal manually
    ListAddRow(L"Note", L"LowBox tokens require NT6.2+ undocumented APIs",
        L"CreateAppContainerToken");

    FreeSid(pEveryoneSid);
    CloseHandle(hToken);
    StatusSet(L"Restricted Token — CreateRestrictedToken + IsTokenRestricted", g_itemCount);
}

// ── Mode 4: Thread Token ─────────────────────────────────────────────────────

DWORD WINAPI ThreadTokenWorker(LPVOID param) {
    HWND hEdit = (HWND)param;
    wchar_t buf[512];

    // Try to open thread token (should fail — no impersonation)
    HANDLE hThr = nullptr;
    BOOL got = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hThr);
    DWORD e   = GetLastError();

    StringCchPrintf(buf, 512, L"Worker: OpenThreadToken (no imp): %s (err=%u)",
        got ? L"OK" : L"FAIL", e);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    if (got) CloseHandle(hThr);

    // Now impersonate self
    ImpersonateSelf(SecurityImpersonation);
    got = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThr);
    StringCchPrintf(buf, 512, L"Worker: OpenThreadToken (after ImpersonateSelf): %s",
        got ? L"OK" : L"FAIL");
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");

    if (got) {
        BYTE tb[512]; DWORD tn;
        GetTokenInformation(hThr, TokenUser, tb, sizeof(tb), &tn);
        TOKEN_USER* tu = (TOKEN_USER*)tb;
        wchar_t uname[256] = {}, udomain[256] = {};
        DWORD un = 256, ud = 256; SID_NAME_USE use;
        LookupAccountSidW(nullptr, tu->User.Sid, uname, &un, udomain, &ud, &use);
        StringCchPrintf(buf, 512, L"Worker thread token user: %s\\%s", udomain, uname);
        SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
        SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
        CloseHandle(hThr);
    }

    RevertToSelf();
    StringCchCopy(buf, 512, L"Worker: RevertToSelf() — done");
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    return 0;
}

void DemoThreadToken() {
    ListClear();
    ListAddColumn(0, L"Step",       200);
    ListAddColumn(1, L"Result",     340);
    ListAddColumn(2, L"Notes",      200);
    EditClear();
    EditAppend(L"=== Thread Token (per-thread impersonation) ===");

    // Main thread: no impersonation initially
    HANDLE hThr = nullptr;
    BOOL got = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hThr);
    wchar_t tmp[256];
    StringCchPrintf(tmp, 256, L"%s (err=%u)", got ? L"OK" : L"FAIL (expected)", GetLastError());
    ListAddRow(L"OpenThreadToken (no imp)", tmp, L"Threads start with no token");
    if (got) CloseHandle(hThr);

    // ImpersonateSelf
    ImpersonateSelf(SecurityImpersonation);
    got = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        TRUE, &hThr);
    StringCchPrintf(tmp, 256, L"%s", got ? L"OK" : L"FAIL");
    ListAddRow(L"OpenThreadToken (after ImpersonateSelf)", tmp, L"Now has thread token");

    if (got) {
        // Adjust a privilege on the thread token
        TOKEN_PRIVILEGES tp = {};
        tp.PrivilegeCount = 1;
        LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        BOOL adjOk = AdjustTokenPrivileges(hThr, FALSE, &tp, 0, nullptr, nullptr);
        StringCchPrintf(tmp, 256, L"%s (err=%u)", adjOk ? L"OK" : L"FAIL", GetLastError());
        ListAddRow(L"AdjustTokenPrivileges(SeDebug) on thread", tmp,
            L"Thread-level privilege adjustment");
        CloseHandle(hThr);
    }
    RevertToSelf();
    ListAddRow(L"RevertToSelf()", L"OK", L"Cleared thread token");

    // Spawn worker thread
    ListAddRow(L"Worker thread", L"Launching...", L"See raw output below");
    EditAppend(L"--- Worker Thread Output ---");

    int editLen = GetWindowTextLengthW(g_hEdit);
    SendMessage(g_hEdit, EM_SETSEL, editLen, editLen);

    HANDLE hWorker = CreateThread(nullptr, 0, ThreadTokenWorker, g_hEdit, 0, nullptr);
    if (hWorker) {
        WaitForSingleObject(hWorker, 5000);
        CloseHandle(hWorker);
        ListAddRow(L"Worker thread", L"Done", L"Thread-level impersonation demo");
    }

    StatusSet(L"Thread Token — OpenThreadToken + per-thread ImpersonateSelf", g_itemCount);
}

// ── Mode 5: DPAPI ────────────────────────────────────────────────────────────

void DemoDPAPI() {
    ListClear();
    ListAddColumn(0, L"Operation",  200);
    ListAddColumn(1, L"Result",     300);
    ListAddColumn(2, L"Notes",      250);
    EditClear();
    EditAppend(L"=== DPAPI — Data Protection API ===");

    // Plaintext to encrypt
    const wchar_t* plainW = L"Secret Win32 learning data!";
    DATA_BLOB input = {};
    input.pbData = (BYTE*)plainW;
    input.cbData = (DWORD)((wcslen(plainW) + 1) * sizeof(wchar_t));

    // Optional entropy
    const wchar_t* entropyW = L"MyEntropy42";
    DATA_BLOB entropy = {};
    entropy.pbData = (BYTE*)entropyW;
    entropy.cbData = (DWORD)((wcslen(entropyW) + 1) * sizeof(wchar_t));

    // ── CryptProtectData (user scope) ──
    DATA_BLOB encrypted = {};
    BOOL ok = CryptProtectData(&input, L"Win32 Demo",
        &entropy, nullptr, nullptr,
        CRYPTPROTECT_UI_FORBIDDEN,   // no UI
        &encrypted);

    wchar_t tmp[512];
    if (ok) {
        StringCchPrintf(tmp, 512, L"Blob size: %u bytes", encrypted.cbData);
        ListAddRow(L"CryptProtectData (user)", tmp, L"User-scope DPAPI encryption");
        EditAppend(L"CryptProtectData (user scope) succeeded");

        // Show first 32 bytes as hex
        wchar_t hex[128] = {};
        DWORD showBytes = min(encrypted.cbData, (DWORD)32);
        for (DWORD i = 0; i < showBytes; i++) {
            wchar_t hb[4];
            StringCchPrintf(hb, 4, L"%02X", encrypted.pbData[i]);
            StringCchCat(hex, 128, hb);
            if (i < showBytes - 1) StringCchCat(hex, 128, L" ");
        }
        ListAddRow(L"Encrypted blob (hex, first 32B)", hex, L"");
        EditAppend(hex);

        // ── CryptUnprotectData ──
        DATA_BLOB decrypted = {};
        LPWSTR description = nullptr;
        BOOL decOk = CryptUnprotectData(&encrypted, &description,
            &entropy, nullptr, nullptr,
            CRYPTPROTECT_UI_FORBIDDEN,
            &decrypted);

        if (decOk) {
            wchar_t* decStr = (wchar_t*)decrypted.pbData;
            StringCchPrintf(tmp, 512, L"\"%s\"", decStr);
            ListAddRow(L"CryptUnprotectData", tmp, L"Decrypted text matches original");
            if (description) {
                ListAddRow(L"Description", description, L"Label set during encrypt");
                LocalFree(description);
            }
            BOOL match = (wcscmp(decStr, plainW) == 0);
            ListAddRow(L"Round-trip match", match ? L"YES" : L"NO",
                L"strcmp(decrypted, original)");
            LocalFree(decrypted.pbData);
            EditAppend(L"CryptUnprotectData succeeded, text matches.");
        } else {
            StringCchPrintf(tmp, 512, L"FAILED err=%u", GetLastError());
            ListAddRow(L"CryptUnprotectData", tmp, L"");
        }
        LocalFree(encrypted.pbData);
    } else {
        StringCchPrintf(tmp, 512, L"FAILED err=%u", GetLastError());
        ListAddRow(L"CryptProtectData (user)", tmp, L"");
    }

    // ── CryptProtectData (machine scope) ──
    DATA_BLOB encMachine = {};
    ok = CryptProtectData(&input, L"Win32 MachineScope",
        nullptr, nullptr, nullptr,
        CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_LOCAL_MACHINE,
        &encMachine);
    if (ok) {
        StringCchPrintf(tmp, 512, L"Blob size: %u bytes (machine scope)", encMachine.cbData);
        ListAddRow(L"CryptProtectData (machine)", tmp,
            L"CRYPTPROTECT_LOCAL_MACHINE — any user on this machine can decrypt");
        LocalFree(encMachine.pbData);
        EditAppend(L"CryptProtectData (CRYPTPROTECT_LOCAL_MACHINE) succeeded");
    } else {
        StringCchPrintf(tmp, 512, L"Machine scope FAILED err=%u", GetLastError());
        ListAddRow(L"CryptProtectData (machine)", tmp, L"");
    }

    // ── CryptProtectMemory ──
    wchar_t memBuf[128];
    StringCchCopy(memBuf, 128, L"In-memory secret");
    DWORD memSize = 128 * sizeof(wchar_t);
    // Round up to CRYPTPROTECTMEMORY_BLOCK_SIZE
    ok = CryptProtectMemory(memBuf, memSize, CRYPTPROTECTMEMORY_SAME_PROCESS);
    if (ok) {
        ListAddRow(L"CryptProtectMemory", L"OK", L"CRYPTPROTECTMEMORY_SAME_PROCESS");
        EditAppend(L"CryptProtectMemory encrypted in-place (buffer is now opaque)");
        // Unprotect
        BOOL unOk = CryptUnprotectMemory(memBuf, memSize, CRYPTPROTECTMEMORY_SAME_PROCESS);
        if (unOk) {
            StringCchPrintf(tmp, 512, L"\"%s\"", memBuf);
            ListAddRow(L"CryptUnprotectMemory", tmp, L"Data restored in-place");
            EditAppend(L"CryptUnprotectMemory restored data in-place");
        }
    } else {
        StringCchPrintf(tmp, 512, L"CryptProtectMemory FAILED err=%u", GetLastError());
        ListAddRow(L"CryptProtectMemory", tmp, L"");
    }

    StatusSet(L"DPAPI — CryptProtectData + CryptProtectMemory", g_itemCount);
}

// ── Mode 6: Token Statistics & Linked Token ───────────────────────────────────

void DemoTokenStats() {
    ListClear();
    ListAddColumn(0, L"Field",   220);
    ListAddColumn(1, L"Value",   300);
    ListAddColumn(2, L"Notes",   230);
    EditClear();
    EditAppend(L"=== Token Statistics & Linked Token ===");

    HANDLE hToken = nullptr;
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken);

    wchar_t tmp[512];

    // TokenStatistics
    TOKEN_STATISTICS ts;
    DWORD needed;
    if (GetTokenInformation(hToken, TokenStatistics, &ts, sizeof(ts), &needed)) {
        StringCchPrintf(tmp, 512, L"%08X:%08X", ts.TokenId.HighPart, ts.TokenId.LowPart);
        ListAddRow(L"TokenId (LUID)", tmp, L"Unique ID for this token instance");

        StringCchPrintf(tmp, 512, L"%08X:%08X",
            ts.AuthenticationId.HighPart, ts.AuthenticationId.LowPart);
        ListAddRow(L"AuthenticationId", tmp, L"Logon session LUID");

        StringCchPrintf(tmp, 512, L"%08X:%08X",
            ts.ModifiedId.HighPart, ts.ModifiedId.LowPart);
        ListAddRow(L"ModifiedId", tmp, L"Changes on each token modification");

        StringCchPrintf(tmp, 512, L"%u", ts.PrivilegeCount);
        ListAddRow(L"PrivilegeCount", tmp, L"From TOKEN_STATISTICS");

        StringCchPrintf(tmp, 512, L"%u", ts.GroupCount);
        ListAddRow(L"GroupCount", tmp, L"From TOKEN_STATISTICS");

        LARGE_INTEGER expiry;
        expiry.HighPart = ts.ExpirationTime.HighPart;
        expiry.LowPart  = ts.ExpirationTime.LowPart;
        if (expiry.QuadPart == 0x7FFFFFFFFFFFFFFF) {
            StringCchCopy(tmp, 512, L"Never (infinite)");
        } else {
            FILETIME ft;
            ft.dwLowDateTime  = ts.ExpirationTime.LowPart;
            ft.dwHighDateTime = ts.ExpirationTime.HighPart;
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft, &st);
            StringCchPrintf(tmp, 512, L"%04d-%02d-%02d %02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
        ListAddRow(L"ExpirationTime", tmp, L"TOKEN_STATISTICS.ExpirationTime");

        const wchar_t* imp =
            ts.ImpersonationLevel == SecurityAnonymous      ? L"Anonymous" :
            ts.ImpersonationLevel == SecurityIdentification ? L"Identification" :
            ts.ImpersonationLevel == SecurityImpersonation  ? L"Impersonation" :
            ts.ImpersonationLevel == SecurityDelegation     ? L"Delegation" : L"N/A (Primary)";
        ListAddRow(L"ImpersonationLevel", imp, L"N/A for primary tokens");

        EditAppend(L"TokenStatistics retrieved: TokenId, AuthId, ExpirationTime, ModifiedId");
    }

    // TokenElevationType
    TOKEN_ELEVATION_TYPE tet;
    if (GetTokenInformation(hToken, TokenElevationType, &tet, sizeof(tet), &needed)) {
        const wchar_t* s =
            tet == TokenElevationTypeDefault ? L"Default (no UAC split)" :
            tet == TokenElevationTypeFull    ? L"Full (elevated, split token)" :
                                               L"Limited (standard, split token)";
        ListAddRow(L"ElevationType", s, L"TokenElevationType");
    }

    // TokenLinkedToken
    TOKEN_LINKED_TOKEN tlt;
    if (GetTokenInformation(hToken, TokenLinkedToken, &tlt, sizeof(tlt), &needed)) {
        EditAppend(L"TokenLinkedToken: found linked token (UAC split detected)");

        // Current token user
        BYTE b1[512]; DWORD n1;
        GetTokenInformation(hToken, TokenUser, b1, sizeof(b1), &n1);
        wchar_t curUser[256];
        SidToDisplayName(((TOKEN_USER*)b1)->User.Sid, curUser, 256);
        ListAddRow(L"Current Token User", curUser, L"Current (this) token");

        // Linked token user
        BYTE b2[512]; DWORD n2;
        GetTokenInformation(tlt.LinkedToken, TokenUser, b2, sizeof(b2), &n2);
        wchar_t lnkUser[256];
        SidToDisplayName(((TOKEN_USER*)b2)->User.Sid, lnkUser, 256);
        ListAddRow(L"Linked Token User", lnkUser, L"The paired elevated/limited token");

        // SID comparison
        BOOL same = EqualSid(((TOKEN_USER*)b1)->User.Sid,
                             ((TOKEN_USER*)b2)->User.Sid);
        ListAddRow(L"Same SID (current vs linked)", same ? L"YES" : L"NO",
            L"EqualSid — should be same user");

        // Linked token statistics
        TOKEN_STATISTICS lts;
        if (GetTokenInformation(tlt.LinkedToken, TokenStatistics, &lts, sizeof(lts), &needed)) {
            StringCchPrintf(tmp, 512, L"%08X:%08X",
                lts.TokenId.HighPart, lts.TokenId.LowPart);
            ListAddRow(L"Linked Token ID", tmp, L"Different from current token");
            StringCchPrintf(tmp, 512, L"%u privs", lts.PrivilegeCount);
            ListAddRow(L"Linked Token Privileges", tmp, L"May differ from current");
        }

        CloseHandle(tlt.LinkedToken);
    } else {
        ListAddRow(L"TokenLinkedToken", L"Not available",
            L"No UAC split (single token session)");
        EditAppend(L"TokenLinkedToken: not available — likely no UAC split");
    }

    CloseHandle(hToken);
    StatusSet(L"Token Statistics — GetTokenInformation(TokenStatistics + TokenLinkedToken)", g_itemCount);
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        struct { const wchar_t* label; int id; } btns[] = {
            {L"Basic Flow",    IDC_BTN_BASIC},
            {L"All Levels",    IDC_BTN_LEVELS},
            {L"Restricted",    IDC_BTN_RESTRICT},
            {L"Thread Token",  IDC_BTN_THREAD},
            {L"DPAPI",         IDC_BTN_DPAPI},
            {L"Token Stats",   IDC_BTN_STATS},
        };
        for (int i = 0; i < 6; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 133, 8, 123, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        g_hList = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS,
            0, 45, 800, 260, hwnd,
            (HMENU)(UINT_PTR)IDC_LISTVIEW, g_hInst, nullptr);
        ListView_SetExtendedListViewStyle(g_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        SendMessage(g_hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            0, 310, 800, 200, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_OUT, g_hInst, nullptr);
        SendMessage(g_hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hStatus = CreateWindowW(STATUSCLASSNAMEW, L"Ready",
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd,
            (HMENU)(UINT_PTR)IDC_STATUS, g_hInst, nullptr);
        int parts[] = {500, -1};
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
        int listH = (H - 45 - sh) * 55 / 100;
        int editH = (H - 45 - sh) - listH;
        SetWindowPos(g_hList, nullptr, 0, 45, W, listH, SWP_NOZORDER);
        SetWindowPos(g_hEdit, nullptr, 0, 45 + listH, W, editH, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_BASIC:    DemoBasicFlow();       break;
        case IDC_BTN_LEVELS:   DemoAllLevels();       break;
        case IDC_BTN_RESTRICT: DemoRestrictedToken(); break;
        case IDC_BTN_THREAD:   DemoThreadToken();     break;
        case IDC_BTN_DPAPI:    DemoDPAPI();           break;
        case IDC_BTN_STATS:    DemoTokenStats();      break;
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_F5) DemoBasicFlow();
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
    wc.lpszClassName = L"Imp26Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Imp26Class",
        L"26 — Impersonation: Token Manipulation & DPAPI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 870, 640,
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
