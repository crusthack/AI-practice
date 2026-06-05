// 25_Security — Process Token & Object ACL Inspector
// Covers: OpenProcessToken, GetTokenInformation (all classes), SID functions,
//         Privilege management, Object DACL, AccessCheck, ACE types
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <sddl.h>
#include <aclapi.h>
#include <strsafe.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

enum {
    IDC_BTN_TOKEN_INFO   = 101,
    IDC_BTN_TOKEN_GROUPS = 102,
    IDC_BTN_PRIVILEGES   = 103,
    IDC_BTN_OBJECT_DACL  = 104,
    IDC_BTN_ACCESS_CHECK = 105,
    IDC_BTN_BROWSE       = 106,
    IDC_EDIT_PATH        = 107,
    IDC_LISTVIEW         = 108,
    IDC_STATUS           = 109,
};

HWND      g_hwnd       = nullptr;
HWND      g_hList      = nullptr;
HWND      g_hStatus    = nullptr;
HWND      g_hEditPath  = nullptr;
HINSTANCE g_hInst      = nullptr;
int       g_itemCount  = 0;

// ── Helpers ──────────────────────────────────────────────────────────────────

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

// Convert SID to "DOMAIN\Name" or SDDL string
void SidToDisplayName(PSID sid, wchar_t* out, DWORD outCch) {
    wchar_t name[128] = {}, domain[128] = {};
    DWORD nl = 128, dl = 128;
    SID_NAME_USE use;
    if (LookupAccountSidW(nullptr, sid, name, &nl, domain, &dl, &use)) {
        if (domain[0]) StringCchPrintf(out, outCch, L"%s\\%s", domain, name);
        else           StringCchCopy(out, outCch, name);
    } else {
        LPWSTR s = nullptr;
        if (ConvertSidToStringSidW(sid, &s)) {
            StringCchCopy(out, outCch, s);
            LocalFree(s);
        } else StringCchCopy(out, outCch, L"(unknown)");
    }
}

void SidToSddl(PSID sid, wchar_t* out, DWORD outCch) {
    LPWSTR s = nullptr;
    if (ConvertSidToStringSidW(sid, &s)) {
        StringCchCopy(out, outCch, s);
        LocalFree(s);
    } else StringCchCopy(out, outCch, L"?");
}

const wchar_t* SidUseStr(SID_NAME_USE u) {
    switch (u) {
    case SidTypeUser:           return L"User";
    case SidTypeGroup:          return L"Group";
    case SidTypeDomain:         return L"Domain";
    case SidTypeAlias:          return L"Alias";
    case SidTypeWellKnownGroup: return L"WellKnownGroup";
    case SidTypeDeletedAccount: return L"Deleted";
    case SidTypeInvalid:        return L"Invalid";
    case SidTypeUnknown:        return L"Unknown";
    case SidTypeComputer:       return L"Computer";
    default:                    return L"?";
    }
}

// ── Mode 1: Token Info ───────────────────────────────────────────────────────

void ShowTokenInfo() {
    ListClear();
    ListAddColumn(0, L"Field",  220);
    ListAddColumn(1, L"Value",  340);
    ListAddColumn(2, L"Notes",  220);

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken)) {
        StatusSet(L"OpenProcessToken failed", 0);
        return;
    }

    wchar_t tmp[512];
    BYTE    buf[2048];
    DWORD   needed = 0;

    // --- TokenUser ---
    if (GetTokenInformation(hToken, TokenUser, buf, sizeof(buf), &needed)) {
        TOKEN_USER* tu = (TOKEN_USER*)buf;
        wchar_t name[256], sddl[128];
        SidToDisplayName(tu->User.Sid, name, 256);
        SidToSddl(tu->User.Sid, sddl, 128);
        ListAddRow(L"User Name",       name,  L"LookupAccountSid");
        ListAddRow(L"User SID",        sddl,  L"ConvertSidToStringSid");

        DWORD subCount = *GetSidSubAuthorityCount(tu->User.Sid);
        DWORD sidLen   = GetLengthSid(tu->User.Sid);
        StringCchPrintf(tmp, 512, L"%u subauthorities, %u bytes", subCount, sidLen);
        ListAddRow(L"SID Info", tmp, L"GetSidSubAuthorityCount / GetLengthSid");

        // EqualSid self-test
        PSID pSelfCopy = HeapAlloc(GetProcessHeap(), 0, sidLen);
        CopySid(sidLen, pSelfCopy, tu->User.Sid);
        ListAddRow(L"EqualSid (copy==orig)", EqualSid(tu->User.Sid, pSelfCopy) ? L"TRUE" : L"FALSE",
                   L"EqualSid");
        HeapFree(GetProcessHeap(), 0, pSelfCopy);

        // IsWellKnownSid checks
        const struct { WELL_KNOWN_SID_TYPE t; const wchar_t* n; } wks[] = {
            {WinLocalSystemSid,           L"LocalSystem"},
            {WinLocalServiceSid,          L"LocalService"},
            {WinNetworkServiceSid,        L"NetworkService"},
            {WinBuiltinAdministratorsSid, L"Administrators"},
            {WinAuthenticatedUserSid,     L"AuthenticatedUsers"},
        };
        for (auto& w : wks)
            if (IsWellKnownSid(tu->User.Sid, w.t))
                ListAddRow(L"IsWellKnownSid", w.n, L"Match!");
    }

    // --- TokenType ---
    TOKEN_TYPE tt;
    if (GetTokenInformation(hToken, TokenType, &tt, sizeof(tt), &needed))
        ListAddRow(L"Token Type",
            tt == TokenPrimary ? L"Primary" : L"Impersonation",
            L"TokenType enum");

    // --- TokenElevationType ---
    TOKEN_ELEVATION_TYPE tet;
    if (GetTokenInformation(hToken, TokenElevationType, &tet, sizeof(tet), &needed)) {
        const wchar_t* s =
            tet == TokenElevationTypeDefault ? L"Default (UAC disabled)" :
            tet == TokenElevationTypeFull    ? L"Full (elevated)"        :
                                               L"Limited (standard)";
        ListAddRow(L"Elevation Type", s, L"TokenElevationType");
    }

    // --- TokenElevation ---
    TOKEN_ELEVATION te;
    if (GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &needed))
        ListAddRow(L"Is Elevated", te.TokenIsElevated ? L"YES" : L"NO",
                   L"TokenElevation");

    // --- TokenStatistics ---
    TOKEN_STATISTICS ts;
    if (GetTokenInformation(hToken, TokenStatistics, &ts, sizeof(ts), &needed)) {
        StringCchPrintf(tmp, 512, L"%08X:%08X", ts.TokenId.HighPart, ts.TokenId.LowPart);
        ListAddRow(L"Token ID (LUID)", tmp, L"TokenStatistics.TokenId");
        StringCchPrintf(tmp, 512, L"%08X:%08X",
            ts.AuthenticationId.HighPart, ts.AuthenticationId.LowPart);
        ListAddRow(L"Authentication ID", tmp, L"LogonSession LUID");
        StringCchPrintf(tmp, 512, L"%08X:%08X", ts.ModifiedId.HighPart, ts.ModifiedId.LowPart);
        ListAddRow(L"Modified ID", tmp, L"Changes on token modify");
        StringCchPrintf(tmp, 512, L"%u privileges, %u groups",
            ts.PrivilegeCount, ts.GroupCount);
        ListAddRow(L"Counts", tmp);
        const wchar_t* imp =
            ts.ImpersonationLevel == SecurityAnonymous      ? L"Anonymous" :
            ts.ImpersonationLevel == SecurityIdentification ? L"Identification" :
            ts.ImpersonationLevel == SecurityImpersonation  ? L"Impersonation" :
            ts.ImpersonationLevel == SecurityDelegation     ? L"Delegation" : L"N/A";
        ListAddRow(L"Impersonation Level", imp, L"(Primary token = N/A)");
    }

    // --- TokenIntegrityLevel ---
    BYTE ibuf[256];
    if (GetTokenInformation(hToken, TokenIntegrityLevel, ibuf, sizeof(ibuf), &needed)) {
        TOKEN_MANDATORY_LABEL* tml = (TOKEN_MANDATORY_LABEL*)ibuf;
        DWORD ridCount = *GetSidSubAuthorityCount(tml->Label.Sid);
        DWORD rid      = *GetSidSubAuthority(tml->Label.Sid, ridCount - 1);
        const wchar_t* level =
            rid < 0x1000 ? L"Untrusted" :
            rid < 0x2000 ? L"Low" :
            rid < 0x2100 ? L"Medium" :
            rid < 0x3000 ? L"Medium+" :
            rid < 0x4000 ? L"High" : L"System";
        StringCchPrintf(tmp, 512, L"RID=0x%04X — %s", rid, level);
        ListAddRow(L"Integrity Level", tmp, L"TokenIntegrityLevel mandatory label");
    }

    // --- TokenDefaultDacl ---
    BYTE dbuf[4096];
    if (GetTokenInformation(hToken, TokenDefaultDacl, dbuf, sizeof(dbuf), &needed)) {
        TOKEN_DEFAULT_DACL* tdd = (TOKEN_DEFAULT_DACL*)dbuf;
        if (tdd->DefaultDacl) {
            ACL_SIZE_INFORMATION ai;
            GetAclInformation(tdd->DefaultDacl, &ai, sizeof(ai), AclSizeInformation);
            StringCchPrintf(tmp, 512, L"%u ACEs, %u bytes used",
                ai.AceCount, ai.AclBytesInUse);
            ListAddRow(L"Default DACL", tmp, L"TokenDefaultDacl");
        } else ListAddRow(L"Default DACL", L"NULL (no restrictions)", L"TokenDefaultDacl");
    }

    // --- TokenLinkedToken ---
    TOKEN_LINKED_TOKEN tlt;
    if (GetTokenInformation(hToken, TokenLinkedToken, &tlt, sizeof(tlt), &needed)) {
        // Compare user SIDs
        BYTE lb[512];
        DWORD ln = 0;
        if (GetTokenInformation(tlt.LinkedToken, TokenUser, lb, sizeof(lb), &ln)) {
            TOKEN_USER* lu = (TOKEN_USER*)lb;
            wchar_t lname[256];
            SidToDisplayName(lu->User.Sid, lname, 256);
            StringCchPrintf(tmp, 512, L"Linked to: %s", lname);
            ListAddRow(L"Linked Token User", tmp, L"TokenLinkedToken (split-token UAC)");
        }
        CloseHandle(tlt.LinkedToken);
    } else {
        ListAddRow(L"Linked Token", L"Not available", L"Single-token session or no UAC split");
    }

    CloseHandle(hToken);
    StatusSet(L"Token Info — OpenProcessToken + GetTokenInformation", g_itemCount);
}

// ── Mode 2: Token Groups ─────────────────────────────────────────────────────

void ShowTokenGroups() {
    ListClear();
    ListAddColumn(0, L"Group Name",  220);
    ListAddColumn(1, L"SID String",  200);
    ListAddColumn(2, L"Attributes",  200);
    ListAddColumn(3, L"SID Type",    110);

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        StatusSet(L"OpenProcessToken failed", 0);
        return;
    }

    DWORD needed = 0;
    GetTokenInformation(hToken, TokenGroups, nullptr, 0, &needed);
    BYTE* buf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, needed);
    if (buf && GetTokenInformation(hToken, TokenGroups, buf, needed, &needed)) {
        TOKEN_GROUPS* tg = (TOKEN_GROUPS*)buf;
        for (DWORD i = 0; i < tg->GroupCount; i++) {
            SID_AND_ATTRIBUTES& sa = tg->Groups[i];
            DWORD attr = sa.Attributes;
            wchar_t attrStr[256] = {};
            if (attr & SE_GROUP_MANDATORY)          StringCchCat(attrStr, 256, L"Mandatory ");
            if (attr & SE_GROUP_ENABLED_BY_DEFAULT) StringCchCat(attrStr, 256, L"ByDefault ");
            if (attr & SE_GROUP_ENABLED)            StringCchCat(attrStr, 256, L"Enabled ");
            if (attr & SE_GROUP_OWNER)              StringCchCat(attrStr, 256, L"Owner ");
            if (attr & SE_GROUP_USE_FOR_DENY_ONLY)  StringCchCat(attrStr, 256, L"DenyOnly ");
            if (attr & SE_GROUP_INTEGRITY)          StringCchCat(attrStr, 256, L"Integrity ");
            if (attr & SE_GROUP_INTEGRITY_ENABLED)  StringCchCat(attrStr, 256, L"IntegrityEnabled ");
            if (attr & SE_GROUP_LOGON_ID)           StringCchCat(attrStr, 256, L"LogonId ");

            wchar_t name[256], sddl[128];
            SidToDisplayName(sa.Sid, name, 256);
            SidToSddl(sa.Sid, sddl, 128);

            wchar_t uname[128] = {}, udomain[128] = {};
            DWORD nl = 128, dl = 128;
            SID_NAME_USE use = SidTypeUnknown;
            LookupAccountSidW(nullptr, sa.Sid, uname, &nl, udomain, &dl, &use);

            ListAddRow(name, sddl, attrStr, SidUseStr(use));
        }
        HeapFree(GetProcessHeap(), 0, buf);
    }
    CloseHandle(hToken);
    StatusSet(L"Token Groups — GetTokenInformation(TokenGroups)", g_itemCount);
}

// ── Mode 3: Privileges ───────────────────────────────────────────────────────

void ShowPrivileges() {
    ListClear();
    ListAddColumn(0, L"Privilege Name",   210);
    ListAddColumn(1, L"Display Name",     260);
    ListAddColumn(2, L"State",            130);
    ListAddColumn(3, L"LUID",            120);

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        StatusSet(L"OpenProcessToken failed", 0);
        return;
    }

    DWORD needed = 0;
    GetTokenInformation(hToken, TokenPrivileges, nullptr, 0, &needed);
    BYTE* buf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, needed);
    if (buf && GetTokenInformation(hToken, TokenPrivileges, buf, needed, &needed)) {
        TOKEN_PRIVILEGES* tp = (TOKEN_PRIVILEGES*)buf;
        for (DWORD i = 0; i < tp->PrivilegeCount; i++) {
            LUID_AND_ATTRIBUTES& la = tp->Privileges[i];
            wchar_t privName[256] = {};
            DWORD   pnLen = 256;
            LookupPrivilegeNameW(nullptr, &la.Luid, privName, &pnLen);

            wchar_t dispName[256] = {};
            DWORD   dnLen = 256, langId = 0;
            LookupPrivilegeDisplayNameW(nullptr, privName, dispName, &dnLen, &langId);

            const wchar_t* state =
                (la.Attributes & SE_PRIVILEGE_REMOVED)            ? L"Removed" :
                (la.Attributes & SE_PRIVILEGE_ENABLED)            ? L"Enabled" :
                (la.Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT) ? L"ByDefault (off)" :
                                                                    L"Disabled";

            wchar_t luidStr[64];
            StringCchPrintf(luidStr, 64, L"%08X:%08X",
                la.Luid.HighPart, la.Luid.LowPart);

            ListAddRow(privName, dispName, state, luidStr);
        }
        HeapFree(GetProcessHeap(), 0, buf);
    }

    // Demo: enable then re-disable SeShutdownPrivilege
    TOKEN_PRIVILEGES tp2 = {};
    tp2.PrivilegeCount = 1;
    if (LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &tp2.Privileges[0].Luid)) {
        tp2.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        BOOL ok = AdjustTokenPrivileges(hToken, FALSE, &tp2, 0, nullptr, nullptr);
        DWORD e  = GetLastError();
        // Restore
        tp2.Privileges[0].Attributes = 0;
        AdjustTokenPrivileges(hToken, FALSE, &tp2, 0, nullptr, nullptr);

        wchar_t msg[256];
        StringCchPrintf(msg, 256, L"AdjustTokenPrivileges(SeShutdown,Enable): %s (err=%u)",
            ok ? L"OK" : L"FAIL", e);
        StatusSet(msg, g_itemCount);
    } else {
        StatusSet(L"Privileges loaded", g_itemCount);
    }
    CloseHandle(hToken);
}

// ── Mode 4: Object DACL ──────────────────────────────────────────────────────

void ShowObjectDacl() {
    ListClear();
    ListAddColumn(0, L"#",           36);
    ListAddColumn(1, L"ACE Type",   100);
    ListAddColumn(2, L"Account",    230);
    ListAddColumn(3, L"Access Mask",140);

    wchar_t path[MAX_PATH];
    GetWindowTextW(g_hEditPath, path, MAX_PATH);
    if (!path[0]) StringCchCopyW(path, MAX_PATH, L"C:\\Windows\\System32\\notepad.exe");

    PSECURITY_DESCRIPTOR pSD = nullptr;
    PACL pDacl = nullptr;
    DWORD err = GetNamedSecurityInfoW(path, SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
        nullptr, nullptr, &pDacl, nullptr, &pSD);

    if (err != ERROR_SUCCESS) {
        wchar_t msg[256];
        StringCchPrintf(msg, 256, L"GetNamedSecurityInfo failed: err=%u", err);
        StatusSet(msg, 0);
        return;
    }

    // Show SDDL string
    LPWSTR sddl = nullptr;
    if (ConvertSecurityDescriptorToStringSecurityDescriptorW(
            pSD, SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &sddl, nullptr)) {
        ListAddRow(L"SDDL", sddl, L"ConvertSecDescToStringSec", L"");
        LocalFree(sddl);
    }

    if (pDacl) {
        ACL_SIZE_INFORMATION ai = {};
        GetAclInformation(pDacl, &ai, sizeof(ai), AclSizeInformation);

        wchar_t aclInfo[128];
        StringCchPrintf(aclInfo, 128, L"ACL: %u ACEs, %u bytes used / %u free",
            ai.AceCount, ai.AclBytesInUse, ai.AclBytesFree);
        ListAddRow(L"ACL Info", aclInfo, L"GetAclInformation(AclSizeInformation)", L"");

        for (DWORD i = 0; i < ai.AceCount; i++) {
            ACE_HEADER* ace = nullptr;
            if (!GetAce(pDacl, i, (PVOID*)&ace)) continue;

            PSID        sid     = nullptr;
            ACCESS_MASK mask    = 0;
            const wchar_t* aceType = L"Unknown";

            // Properly cast based on AceType
            switch (ace->AceType) {
            case ACCESS_ALLOWED_ACE_TYPE: {
                auto* p = (ACCESS_ALLOWED_ACE*)ace;
                sid = (PSID)&p->SidStart; mask = p->Mask;
                aceType = L"Allow";       break;
            }
            case ACCESS_DENIED_ACE_TYPE: {
                auto* p = (ACCESS_DENIED_ACE*)ace;
                sid = (PSID)&p->SidStart; mask = p->Mask;
                aceType = L"Deny";        break;
            }
            case SYSTEM_AUDIT_ACE_TYPE: {
                auto* p = (SYSTEM_AUDIT_ACE*)ace;
                sid = (PSID)&p->SidStart; mask = p->Mask;
                aceType = L"Audit";       break;
            }
            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                aceType = L"AllowObj";    break;
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
                aceType = L"DenyObj";     break;
            default:
                aceType = L"Other";
            }

            wchar_t idxStr[8], maskStr[32], acctName[256];
            StringCchPrintf(idxStr, 8, L"%u", i);
            StringCchPrintf(maskStr, 32, L"0x%08X", mask);
            if (sid) SidToDisplayName(sid, acctName, 256);
            else     StringCchCopy(acctName, 256, L"(object ACE, no SID)");

            ListAddRow(idxStr, aceType, acctName, maskStr);
        }
    } else {
        ListAddRow(L"DACL", L"NULL DACL", L"Full access for everyone", L"");
    }

    LocalFree(pSD);
    StatusSet(L"Object DACL — GetNamedSecurityInfo + GetAce", g_itemCount);
}

// ── Mode 5: Access Check ─────────────────────────────────────────────────────

void ShowAccessCheck() {
    ListClear();
    ListAddColumn(0, L"Access Right",  200);
    ListAddColumn(1, L"Granted?",       90);
    ListAddColumn(2, L"Mask",          110);
    ListAddColumn(3, L"Notes",         240);

    wchar_t path[MAX_PATH];
    GetWindowTextW(g_hEditPath, path, MAX_PATH);
    if (!path[0]) StringCchCopyW(path, MAX_PATH, L"C:\\Windows\\System32\\notepad.exe");

    PSECURITY_DESCRIPTOR pSD = nullptr;
    DWORD err = GetNamedSecurityInfoW(path, SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION,
        nullptr, nullptr, nullptr, nullptr, &pSD);
    if (err != ERROR_SUCCESS) {
        wchar_t msg[256];
        StringCchPrintf(msg, 256, L"GetNamedSecurityInfo failed: %u", err);
        StatusSet(msg, 0);
        return;
    }

    HANDLE hToken = nullptr, hImpToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        LocalFree(pSD);
        StatusSet(L"OpenProcessToken failed", 0);
        return;
    }
    DuplicateToken(hToken, SecurityImpersonation, &hImpToken);
    CloseHandle(hToken);

    GENERIC_MAPPING gm = {};
    gm.GenericRead    = FILE_GENERIC_READ;
    gm.GenericWrite   = FILE_GENERIC_WRITE;
    gm.GenericExecute = FILE_GENERIC_EXECUTE;
    gm.GenericAll     = FILE_ALL_ACCESS;

    const struct { const wchar_t* name; ACCESS_MASK mask; const wchar_t* notes; } rights[] = {
        {L"FILE_GENERIC_READ",    FILE_GENERIC_READ,    L"Read file contents"},
        {L"FILE_GENERIC_WRITE",   FILE_GENERIC_WRITE,   L"Write file contents"},
        {L"FILE_GENERIC_EXECUTE", FILE_GENERIC_EXECUTE, L"Execute the file"},
        {L"FILE_READ_DATA",       FILE_READ_DATA,       L"Read raw data"},
        {L"FILE_WRITE_DATA",      FILE_WRITE_DATA,      L"Write raw data"},
        {L"FILE_APPEND_DATA",     FILE_APPEND_DATA,     L"Append to file"},
        {L"FILE_READ_ATTRIBUTES", FILE_READ_ATTRIBUTES, L"Read file attributes"},
        {L"FILE_WRITE_ATTRIBUTES",FILE_WRITE_ATTRIBUTES,L"Write file attributes"},
        {L"FILE_READ_EA",         FILE_READ_EA,         L"Read extended attributes"},
        {L"READ_CONTROL",         READ_CONTROL,         L"Read security descriptor"},
        {L"DELETE",               DELETE,               L"Delete file"},
        {L"WRITE_DAC",            WRITE_DAC,            L"Modify DACL"},
        {L"WRITE_OWNER",          WRITE_OWNER,          L"Change owner"},
        {L"SYNCHRONIZE",          SYNCHRONIZE,          L"Wait on file handle"},
    };

    for (auto& r : rights) {
        ACCESS_MASK desired = r.mask;
        MapGenericMask(&desired, &gm);

        PRIVILEGE_SET ps   = {};
        DWORD         psLen = sizeof(ps);
        DWORD         granted = 0;
        BOOL          result  = FALSE;
        AccessCheck(pSD, hImpToken, desired, &gm, &ps, &psLen, &granted, &result);

        wchar_t maskStr[32];
        StringCchPrintf(maskStr, 32, L"0x%08X", r.mask);
        ListAddRow(r.name, result ? L"YES" : L"NO", maskStr, r.notes);
    }

    CloseHandle(hImpToken);
    LocalFree(pSD);
    StatusSet(L"Access Check — AccessCheck() with impersonation token", g_itemCount);
}

// ── WndProc ──────────────────────────────────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Five mode buttons
        struct { const wchar_t* label; int id; } btns[] = {
            {L"Token Info",    IDC_BTN_TOKEN_INFO},
            {L"Token Groups",  IDC_BTN_TOKEN_GROUPS},
            {L"Privileges",    IDC_BTN_PRIVILEGES},
            {L"Object DACL",   IDC_BTN_OBJECT_DACL},
            {L"Access Check",  IDC_BTN_ACCESS_CHECK},
        };
        for (int i = 0; i < 5; i++) {
            HWND hb = CreateWindowW(L"BUTTON", btns[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + i * 148, 8, 138, 28, hwnd,
                (HMENU)(UINT_PTR)btns[i].id, g_hInst, nullptr);
            SendMessage(hb, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // Path input row
        HWND hLbl = CreateWindowW(L"STATIC", L"Path:",
            WS_CHILD | WS_VISIBLE, 10, 46, 40, 22, hwnd, nullptr, g_hInst, nullptr);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_hEditPath = CreateWindowW(L"EDIT",
            L"C:\\Windows\\System32\\notepad.exe",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            55, 45, 580, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_EDIT_PATH, g_hInst, nullptr);
        SendMessage(g_hEditPath, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hBrowse = CreateWindowW(L"BUTTON", L"Browse...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            640, 45, 80, 22, hwnd,
            (HMENU)(UINT_PTR)IDC_BTN_BROWSE, g_hInst, nullptr);
        SendMessage(hBrowse, WM_SETFONT, (WPARAM)hFont, TRUE);

        // ListView
        g_hList = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
            0, 76, 800, 400, hwnd,
            (HMENU)(UINT_PTR)IDC_LISTVIEW, g_hInst, nullptr);
        ListView_SetExtendedListViewStyle(g_hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        SendMessage(g_hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        // StatusBar
        g_hStatus = CreateWindowW(STATUSCLASSNAMEW, L"Ready — select a mode",
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
        SetWindowPos(g_hList, nullptr, 0, 76, W, H - 76 - sh, SWP_NOZORDER);
        // Resize path edit
        SetWindowPos(g_hEditPath, nullptr, 55, 45, W - 190, 22, SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hwnd, IDC_BTN_BROWSE), nullptr,
            W - 130, 45, 80, 22, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wp)) {
        case IDC_BTN_TOKEN_INFO:    ShowTokenInfo();    break;
        case IDC_BTN_TOKEN_GROUPS:  ShowTokenGroups();  break;
        case IDC_BTN_PRIVILEGES:    ShowPrivileges();   break;
        case IDC_BTN_OBJECT_DACL:   ShowObjectDacl();   break;
        case IDC_BTN_ACCESS_CHECK:  ShowAccessCheck();  break;
        case IDC_BTN_BROWSE: {
            OPENFILENAMEW ofn = {};
            wchar_t file[MAX_PATH] = {};
            GetWindowTextW(g_hEditPath, file, MAX_PATH);
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner   = hwnd;
            ofn.lpstrFile   = file;
            ofn.nMaxFile    = MAX_PATH;
            ofn.lpstrFilter = L"All Files\0*.*\0Executables\0*.exe\0";
            ofn.Flags       = OFN_FILEMUSTEXIST;
            if (GetOpenFileNameW(&ofn))
                SetWindowTextW(g_hEditPath, file);
            break;
        }
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (wp == VK_F5) ShowTokenInfo();
        return 0;

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lp;
        if (nm->hwndFrom == g_hList && nm->code == LVN_COLUMNCLICK) {
            // Column click — future sort
        }
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

    WNDCLASSEXW wc    = {};
    wc.cbSize         = sizeof(wc);
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInst;
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName  = L"Sec25Class";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(0, L"Sec25Class",
        L"25 — Security: Process Token & Object ACL Inspector",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 640,
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
