// 09_ShellIntegration — Comprehensive Shell API Showcase
// 5 modes:
//   1) Tray Icon — Shell_NotifyIcon v4, balloon, animated, context menu
//   2) File Operations — SHFileOperation (copy/move/delete), SHOpenFolderAndSelectItems
//   3) Known Folders — enumerate all FOLDERID_* paths via IKnownFolderManager
//   4) File Info — SHGetFileInfo, SHGetImageList, ExtractAssociatedIcon, AssocQueryString
//   5) Shortcuts — IShellLink + IPersistFile, SHAddToRecentDocs, SHGetPropertyStoreFromParsingName

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "propsys.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <Shellapi.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <ShlGuid.h>
#include <propsys.h>
#include <propvarutil.h>
#include <propkey.h>
#include <windowsx.h>
#include <strsafe.h>
#include <objbase.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

// ── IDs ──────────────────────────────────────────────────────────────────────
enum {
    IDC_BTN_M1 = 101, IDC_BTN_M2 = 102, IDC_BTN_M3 = 103,
    IDC_BTN_M4 = 104, IDC_BTN_M5 = 105,
    IDC_LIST   = 200,
    IDC_LOG    = 201,
    IDC_STATUS = 202,
    IDC_PATH   = 203, IDC_BROWSE = 204,
    // Tray sub-actions
    IDC_BTN_BALLOON   = 301,
    IDC_BTN_ANIMATE   = 302,
    IDC_BTN_HIDERESTORE = 303,
    // File ops sub-actions
    IDC_BTN_COPY   = 401, IDC_BTN_MOVE   = 402,
    IDC_BTN_DELETE = 403, IDC_BTN_OPEN   = 404,
    // File info sub-actions
    IDC_BTN_FILEINFO = 501,
    // Shortcuts
    IDC_BTN_MKLINK  = 601, IDC_BTN_RDLINK  = 602,

    WM_TRAYICON = WM_APP + 1,
    IDT_ANIMATE = 1,
    // Tray menu
    ID_TRAY_SHOW    = 9001,
    ID_TRAY_BALLOON = 9002,
    ID_TRAY_EXIT    = 9003,
};

// ── Globals ───────────────────────────────────────────────────────────────────
HWND g_hwnd{}, g_hList{}, g_hLog{}, g_hStatus{};
HWND g_hPath{}, g_hBrowse{};
HWND g_hBalloon{}, g_hAnimate{}, g_hHideRestore{};
HWND g_hBtnCopy{}, g_hBtnMove{}, g_hBtnDelete{}, g_hBtnOpen{};
HWND g_hBtnFileInfo{};
HWND g_hBtnMkLink{}, g_hBtnRdLink{};
HINSTANCE g_hInst{};
int  g_mode = 1;

// Tray state
bool g_trayAdded   = false;
bool g_animating   = false;
int  g_animFrame   = 0;
bool g_hidden      = false;
static const GUID kTrayGuid = {
    0x9fa6b6b3, 0x7f7a, 0x4a5e,
    {0xbc, 0xf3, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}
};

// Work paths
wchar_t g_workDir[MAX_PATH]{};
wchar_t g_srcFile[MAX_PATH]{};
wchar_t g_dstFile[MAX_PATH]{};

// ── Helpers ───────────────────────────────────────────────────────────────────
void Status(int part, const wchar_t* text)
{
    SendMessageW(g_hStatus, SB_SETTEXT, part, reinterpret_cast<LPARAM>(text));
}

void Log(const wchar_t* fmt, ...)
{
    wchar_t line[1024]{};
    va_list va; va_start(va, fmt);
    StringCchVPrintfW(line, 1024, fmt, va);
    va_end(va);
    int len = GetWindowTextLengthW(g_hLog);
    SendMessageW(g_hLog, EM_SETSEL, len, len);
    SendMessageW(g_hLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line));
    SendMessageW(g_hLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L"\r\n"));
    SendMessageW(g_hLog, EM_SCROLLCARET, 0, 0);
}

void LogClear() { SetWindowTextW(g_hLog, L""); }

void ListClear()
{
    ListView_DeleteAllItems(g_hList);
    while (ListView_DeleteColumn(g_hList, 0));
}

void ListAddCol(int idx, int w, const wchar_t* t)
{
    LVCOLUMNW c{ LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM };
    c.iSubItem=idx; c.cx=w; c.pszText=const_cast<wchar_t*>(t);
    ListView_InsertColumn(g_hList, idx, &c);
}

int ListAddRow(const wchar_t* c0, const wchar_t* c1=L"",
               const wchar_t* c2=L"", const wchar_t* c3=L"",
               int img=-1)
{
    static int rowCount = 0;
    LVITEMW lvi{ LVIF_TEXT };
    if (img >= 0) lvi.mask |= LVIF_IMAGE;
    lvi.iItem   = ListView_GetItemCount(g_hList);
    lvi.pszText = const_cast<wchar_t*>(c0);
    lvi.iImage  = img;
    int row = ListView_InsertItem(g_hList, &lvi);
    if (c1[0]) ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(c1));
    if (c2[0]) ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(c2));
    if (c3[0]) ListView_SetItemText(g_hList, row, 3, const_cast<wchar_t*>(c3));
    return row;
}

HFONT g_smallFont{};

// ── Layout ────────────────────────────────────────────────────────────────────
void ShowModeControls(int mode)
{
    HWND allExtra[] = {
        g_hPath, g_hBrowse,
        g_hBalloon, g_hAnimate, g_hHideRestore,
        g_hBtnCopy, g_hBtnMove, g_hBtnDelete, g_hBtnOpen,
        g_hBtnFileInfo,
        g_hBtnMkLink, g_hBtnRdLink,
    };
    for (HWND h : allExtra) if (h) ShowWindow(h, SW_HIDE);

    switch (mode) {
    case 1:
        ShowWindow(g_hBalloon,    SW_SHOW);
        ShowWindow(g_hAnimate,    SW_SHOW);
        ShowWindow(g_hHideRestore,SW_SHOW);
        break;
    case 2:
        ShowWindow(g_hPath,      SW_SHOW);
        ShowWindow(g_hBrowse,    SW_SHOW);
        ShowWindow(g_hBtnCopy,   SW_SHOW);
        ShowWindow(g_hBtnMove,   SW_SHOW);
        ShowWindow(g_hBtnDelete, SW_SHOW);
        ShowWindow(g_hBtnOpen,   SW_SHOW);
        break;
    case 4:
        ShowWindow(g_hPath,        SW_SHOW);
        ShowWindow(g_hBrowse,      SW_SHOW);
        ShowWindow(g_hBtnFileInfo, SW_SHOW);
        break;
    case 5:
        ShowWindow(g_hBtnMkLink, SW_SHOW);
        ShowWindow(g_hBtnRdLink, SW_SHOW);
        break;
    }
    g_mode = mode;
}

void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    SendMessageW(g_hStatus, WM_SIZE, 0, 0);
    RECT sr{}; GetWindowRect(g_hStatus, &sr);
    int sbH = sr.bottom - sr.top;

    const int toolH  = 44;
    const int extraH = 44;
    const int pad    = 8;
    int cy  = toolH + pad;
    int cw  = rc.right - pad * 2;
    int ch  = rc.bottom - sbH - cy - extraH - pad * 2;
    int ey  = cy + ch + pad;

    // Mode buttons
    int btnW = (rc.right - pad * 6) / 5;
    for (int i = 0; i < 5; ++i)
        MoveWindow(GetDlgItem(hwnd, IDC_BTN_M1+i),
            pad + (btnW+pad)*i, 8, btnW, 30, TRUE);

    // Main list (top half) + log (bottom half)
    int listH = ch * 55 / 100;
    int logH  = ch - listH - pad;
    MoveWindow(g_hList, pad, cy,          cw, listH, TRUE);
    MoveWindow(g_hLog,  pad, cy+listH+pad, cw, logH, TRUE);

    // Extra controls row
    int ex = pad, ew = 160;
    MoveWindow(g_hPath,        ex, ey, cw-ew-pad, 28, TRUE); ex += cw-ew;
    MoveWindow(g_hBrowse,      ex, ey, ew,        28, TRUE);

    ex = pad;
    MoveWindow(g_hBalloon,     ex, ey, 150, 28, TRUE); ex += 158;
    MoveWindow(g_hAnimate,     ex, ey, 150, 28, TRUE); ex += 158;
    MoveWindow(g_hHideRestore, ex, ey, 160, 28, TRUE);

    ex = pad;
    MoveWindow(g_hBtnCopy,   ex, ey, 110, 28, TRUE); ex += 118;
    MoveWindow(g_hBtnMove,   ex, ey, 110, 28, TRUE); ex += 118;
    MoveWindow(g_hBtnDelete, ex, ey, 110, 28, TRUE); ex += 118;
    MoveWindow(g_hBtnOpen,   ex, ey, 180, 28, TRUE);

    MoveWindow(g_hBtnFileInfo, pad, ey, 200, 28, TRUE);

    ex = pad;
    MoveWindow(g_hBtnMkLink, ex, ey, 200, 28, TRUE); ex += 208;
    MoveWindow(g_hBtnRdLink, ex, ey, 200, 28, TRUE);
}

// ── Mode 1: Tray Icon ─────────────────────────────────────────────────────────
void TrayAdd()
{
    if (g_trayAdded) return;
    NOTIFYICONDATAW nid{ sizeof(nid) };
    nid.hWnd             = g_hwnd;
    nid.uID              = 1;
    nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_GUID | NIF_SHOWTIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon            = LoadIconW(nullptr, IDI_APPLICATION);
    nid.guidItem         = kTrayGuid;
    StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip),
        L"09 ShellIntegration — NIM_SETVERSION v4");
    if (Shell_NotifyIconW(NIM_ADD, &nid)) {
        // Set version to 4 (Vista+) for WM_MOUSEMOVE/button events in lParam
        nid.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &nid);
        g_trayAdded = true;
        Log(L"[Tray] NIM_ADD + NIM_SETVERSION (v4) OK — icon in taskbar notification area");
    } else {
        Log(L"[Tray] NIM_ADD failed: %lu", GetLastError());
    }
}

void TrayRemove()
{
    if (!g_trayAdded) return;
    NOTIFYICONDATAW nid{ sizeof(nid) };
    nid.hWnd     = g_hwnd;
    nid.uID      = 1;
    nid.uFlags   = NIF_GUID;
    nid.guidItem = kTrayGuid;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    g_trayAdded  = false;
    g_animating  = false;
    KillTimer(g_hwnd, IDT_ANIMATE);
    Log(L"[Tray] NIM_DELETE — icon removed");
}

void TrayBalloon()
{
    if (!g_trayAdded) TrayAdd();
    static int balloonType = 0;
    static const wchar_t* titles[] = { L"Information Balloon", L"Warning Balloon", L"Error Balloon" };
    static const wchar_t* texts[]  = {
        L"This is an NIF_INFO balloon (NIIF_INFO).\nClick to dismiss.",
        L"Warning: NIIF_WARNING balloon with NIIF_NOSOUND flag.",
        L"Error: NIIF_ERROR balloon — serious issue detected!",
    };
    static const DWORD types[] = { NIIF_INFO, NIIF_WARNING|NIIF_NOSOUND, NIIF_ERROR };

    NOTIFYICONDATAW nid{ sizeof(nid) };
    nid.hWnd       = g_hwnd;
    nid.uID        = 1;
    nid.uFlags     = NIF_INFO | NIF_GUID;
    nid.guidItem   = kTrayGuid;
    nid.dwInfoFlags= types[balloonType];
    nid.uTimeout   = 5000;
    StringCchCopyW(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), titles[balloonType]);
    StringCchCopyW(nid.szInfo,      ARRAYSIZE(nid.szInfo),      texts[balloonType]);
    Shell_NotifyIconW(NIM_MODIFY, &nid);

    Log(L"[Balloon] type=%s, title='%s'",
        balloonType==0?L"NIIF_INFO":balloonType==1?L"NIIF_WARNING":L"NIIF_ERROR",
        titles[balloonType]);
    balloonType = (balloonType + 1) % 3;
}

void TrayAnimate()
{
    if (!g_trayAdded) TrayAdd();
    if (g_animating) {
        KillTimer(g_hwnd, IDT_ANIMATE);
        g_animating = false;
        // Restore normal icon
        NOTIFYICONDATAW nid{ sizeof(nid) };
        nid.hWnd = g_hwnd; nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_GUID; nid.guidItem = kTrayGuid;
        nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        Shell_NotifyIconW(NIM_MODIFY, &nid);
        Log(L"[Tray] Animation stopped — restored IDI_APPLICATION");
    } else {
        g_animating = true;
        g_animFrame = 0;
        SetTimer(g_hwnd, IDT_ANIMATE, 400, nullptr);
        Log(L"[Tray] Animation started — cycling IDI_APPLICATION / IDI_INFORMATION / IDI_QUESTION");
    }
}

void TrayHideRestore()
{
    if (!g_trayAdded) TrayAdd();
    NOTIFYICONDATAW nid{ sizeof(nid) };
    nid.hWnd = g_hwnd; nid.uID = 1;
    nid.uFlags = NIF_STATE | NIF_GUID;
    nid.guidItem    = kTrayGuid;
    nid.dwStateMask = NIS_HIDDEN;
    nid.dwState     = g_hidden ? 0 : NIS_HIDDEN;
    if (Shell_NotifyIconW(NIM_MODIFY, &nid)) {
        g_hidden = !g_hidden;
        Log(L"[Tray] NIM_MODIFY NIS_HIDDEN=%s", g_hidden ? L"true" : L"false");
        SetWindowTextW(g_hHideRestore, g_hidden ? L"Restore Tray Icon" : L"Hide Tray Icon");
    }
}

void TrayContextMenu(POINT pt)
{
    SetForegroundWindow(g_hwnd);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_TRAY_SHOW,    L"Show Window");
    AppendMenuW(menu, MF_STRING, ID_TRAY_BALLOON, L"Show Balloon");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT,    L"Exit");
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, g_hwnd, nullptr);
    DestroyMenu(menu);
    switch (cmd) {
    case ID_TRAY_SHOW:    ShowWindow(g_hwnd, SW_RESTORE); SetForegroundWindow(g_hwnd); break;
    case ID_TRAY_BALLOON: TrayBalloon(); break;
    case ID_TRAY_EXIT:    DestroyWindow(g_hwnd); break;
    }
}

// ── Mode 2: File Operations ───────────────────────────────────────────────────
void EnsureWorkFiles()
{
    GetTempPathW(ARRAYSIZE(g_workDir), g_workDir);
    StringCchPrintfW(g_srcFile, ARRAYSIZE(g_srcFile), L"%sWin32Learn_ShellSrc.txt", g_workDir);
    StringCchPrintfW(g_dstFile, ARRAYSIZE(g_dstFile), L"%sWin32Learn_ShellDst.txt", g_workDir);

    // Create source file if needed
    if (GetFileAttributesW(g_srcFile) == INVALID_FILE_ATTRIBUTES) {
        HANDLE h = CreateFileW(g_srcFile, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) {
            const char data[] = "Win32 ShellIntegration demo file\r\nCreated by SHFileOperation example\r\n";
            DWORD w{};
            WriteFile(h, data, sizeof(data)-1, &w, nullptr);
            CloseHandle(h);
        }
    }
}

void SHFileOpDemo(UINT op, const wchar_t* from, const wchar_t* to,
                  FILEOP_FLAGS flags, const wchar_t* label)
{
    // Build double-null-terminated strings
    wchar_t fromBuf[MAX_PATH + 2]{};
    wchar_t toBuf[MAX_PATH + 2]{};
    StringCchCopyW(fromBuf, MAX_PATH, from);
    if (to) StringCchCopyW(toBuf, MAX_PATH, to);

    SHFILEOPSTRUCTW op2{ g_hwnd, op, fromBuf, to ? toBuf : nullptr,
        flags, FALSE, nullptr, L"Win32 SHFileOperation Demo" };

    LARGE_INTEGER t1, t2, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);
    int result = SHFileOperationW(&op2);
    QueryPerformanceCounter(&t2);
    double ms = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;

    bool anyErr = op2.fAnyOperationsAborted != 0;
    Log(L"[SHFileOp] %s: result=%d aborted=%s (%.1f ms)",
        label, result, anyErr ? L"YES" : L"no", ms);
    if (result != 0) {
        wchar_t buf[256]{};
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, result,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, ARRAYSIZE(buf), nullptr);
        Log(L"  Error: %s", buf);
    }
}

void DoCopy()
{
    EnsureWorkFiles();
    SHFileOpDemo(FO_COPY, g_srcFile, g_dstFile,
        FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, L"FO_COPY src→dst");
}
void DoMove()
{
    // If dst exists, move it back to src (swap)
    if (GetFileAttributesW(g_dstFile) != INVALID_FILE_ATTRIBUTES) {
        SHFileOpDemo(FO_MOVE, g_dstFile, g_srcFile,
            FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, L"FO_MOVE dst→src");
    } else {
        Log(L"[SHFileOp] FO_MOVE: destination doesn't exist — run Copy first");
    }
}
void DoDelete()
{
    EnsureWorkFiles();
    // Delete dst file to recycle bin (with undo)
    const wchar_t* target = (GetFileAttributesW(g_dstFile) != INVALID_FILE_ATTRIBUTES)
        ? g_dstFile : g_srcFile;
    SHFileOpDemo(FO_DELETE, target, nullptr,
        FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT, L"FO_DELETE (recycle bin)");
}
void DoOpenAndSelect()
{
    EnsureWorkFiles();
    // SHOpenFolderAndSelectItems: open Explorer highlighting the src file
    PIDLIST_ABSOLUTE pidl{};
    HRESULT hr = SHParseDisplayName(g_srcFile, nullptr, &pidl, 0, nullptr);
    if (SUCCEEDED(hr)) {
        hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        Log(L"[Shell] SHOpenFolderAndSelectItems('%s'): %s",
            g_srcFile, SUCCEEDED(hr) ? L"OK" : L"FAILED");
        CoTaskMemFree(pidl);
    } else {
        Log(L"[Shell] SHParseDisplayName failed: 0x%08X", hr);
    }
}

// ── Mode 3: Known Folders ─────────────────────────────────────────────────────
struct KnownFolderEntry { const KNOWNFOLDERID* id; const wchar_t* name; };
static const KnownFolderEntry kFolders[] = {
    { &FOLDERID_Desktop,            L"Desktop"              },
    { &FOLDERID_Documents,          L"Documents"            },
    { &FOLDERID_Downloads,          L"Downloads"            },
    { &FOLDERID_Music,              L"Music"                },
    { &FOLDERID_Pictures,           L"Pictures"             },
    { &FOLDERID_Videos,             L"Videos"               },
    { &FOLDERID_RoamingAppData,     L"AppData\\Roaming"     },
    { &FOLDERID_LocalAppData,       L"AppData\\Local"       },
    { &FOLDERID_ProgramFiles,       L"Program Files"        },
    { &FOLDERID_ProgramFilesX86,    L"Program Files (x86)"  },
    { &FOLDERID_Windows,            L"Windows"              },
    { &FOLDERID_System,             L"System32"             },
    { &FOLDERID_Fonts,              L"Fonts"                },
    { &FOLDERID_PublicDocuments,    L"Public Documents"     },
    { &FOLDERID_PublicDownloads,    L"Public Downloads"     },
    { &FOLDERID_PublicMusic,        L"Public Music"         },
    { &FOLDERID_StartMenu,          L"Start Menu"           },
    { &FOLDERID_Startup,            L"Startup"              },
    { &FOLDERID_SendTo,             L"SendTo"               },
    { &FOLDERID_Recent,             L"Recent"               },
    { &FOLDERID_NetworkFolder,      L"Network"              },
    { &FOLDERID_Profile,            L"User Profile"         },
    { &FOLDERID_UserPinned,         L"User Pinned"          },
};

void ShowKnownFolders()
{
    ListClear();
    ListAddCol(0, 180, L"Folder Name");
    ListAddCol(1, 380, L"Path");
    ListAddCol(2, 100, L"Exists");

    LogClear();
    Log(L"=== SHGetKnownFolderPath — Known Folder Paths ===");

    int found = 0;
    for (const auto& kf : kFolders) {
        PWSTR path{};
        HRESULT hr = SHGetKnownFolderPath(*kf.id, KF_FLAG_DEFAULT, nullptr, &path);
        if (SUCCEEDED(hr)) {
            bool exists = (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES);
            ListAddRow(kf.name, path, exists ? L"✓" : L"—");
            Log(L"  %-24s %s", kf.name, path);
            CoTaskMemFree(path);
            ++found;
        } else {
            ListAddRow(kf.name, L"(not available)", L"—");
        }
    }

    wchar_t buf[64]{};
    StringCchPrintfW(buf, 64, L"Known folders: %d resolved", found);
    Status(0, buf);
    Status(1, L"SHGetKnownFolderPath");
}

// ── Mode 4: File Info ─────────────────────────────────────────────────────────
void ShowFileInfo()
{
    wchar_t path[MAX_PATH]{};
    GetWindowTextW(g_hPath, path, ARRAYSIZE(path));
    if (path[0] == L'\0')
        StringCchCopyW(path, ARRAYSIZE(path), L"C:\\Windows\\System32\\notepad.exe");

    ListClear();
    ListAddCol(0, 180, L"Property");
    ListAddCol(1, 320, L"Value");
    ListAddCol(2, 100, L"Source API");
    LogClear();
    Log(L"=== File Info: %s ===", path);

    // SHGetFileInfo — icon index, type, display name, exe type
    SHFILEINFOW sfi{};
    DWORD_PTR imgList = SHGetFileInfoW(path, 0, &sfi, sizeof(sfi),
        SHGFI_ICON | SHGFI_TYPENAME | SHGFI_DISPLAYNAME | SHGFI_EXETYPE |
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ATTRIBUTES);

    if (imgList) {
        ListView_SetImageList(g_hList,
            reinterpret_cast<HIMAGELIST>(imgList), LVSIL_SMALL);
        ListAddRow(sfi.szDisplayName, L"", L"", L"", sfi.iIcon);  // first row shows icon
    }
    ListAddRow(L"Display Name",   sfi.szDisplayName,   L"SHGFI_DISPLAYNAME");
    ListAddRow(L"Type Name",      sfi.szTypeName,      L"SHGFI_TYPENAME");
    wchar_t idxBuf[16]{}; StringCchPrintfW(idxBuf, 16, L"%d", sfi.iIcon);
    ListAddRow(L"System Icon Index", idxBuf, L"SHGFI_SYSICONINDEX");

    // exe type
    if (sfi.dwAttributes != 0) {
        const wchar_t* exeType = L"Non-executable";
        if (HIWORD(sfi.hIcon) == 0 && LOWORD(sfi.hIcon) == SCS_32BIT_BINARY) exeType = L"Win32 EXE";
        else if (LOWORD(sfi.hIcon) == SCS_64BIT_BINARY) exeType = L"Win64 EXE";
        ListAddRow(L"Exe Type", exeType, L"SHGFI_EXETYPE");
    }
    if (sfi.hIcon) DestroyIcon(sfi.hIcon);

    // AssocQueryString — file extension associations
    wchar_t ext[64]{};
    {
        wchar_t* e = PathFindExtensionW(path);
        if (e && e[0]) StringCchCopyW(ext, ARRAYSIZE(ext), e);
    }

    if (ext[0]) {
        wchar_t friendlyDoc[256]{};
        DWORD len = ARRAYSIZE(friendlyDoc);
        if (SUCCEEDED(AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_FRIENDLYDOCNAME,
                ext, nullptr, friendlyDoc, &len)))
            ListAddRow(L"Friendly Doc Name", friendlyDoc, L"AssocQueryString");

        wchar_t exePath[MAX_PATH]{};
        len = ARRAYSIZE(exePath);
        if (SUCCEEDED(AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_EXECUTABLE,
                ext, nullptr, exePath, &len)))
            ListAddRow(L"Default Executable", exePath, L"AssocQueryString");

        wchar_t contentType[128]{};
        len = ARRAYSIZE(contentType);
        if (SUCCEEDED(AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_CONTENTTYPE,
                ext, nullptr, contentType, &len)))
            ListAddRow(L"Content Type (MIME)", contentType, L"AssocQueryString");
    }

    // File size + dates via GetFileAttributesEx
    WIN32_FILE_ATTRIBUTE_DATA fad{};
    if (GetFileAttributesExW(path, GetFileExInfoStandard, &fad)) {
        ULARGE_INTEGER sz{ fad.nFileSizeLow, fad.nFileSizeHigh };
        wchar_t szBuf[32]{};
        if (sz.QuadPart < 1024)
            StringCchPrintfW(szBuf, 32, L"%llu B", sz.QuadPart);
        else if (sz.QuadPart < 1024*1024)
            StringCchPrintfW(szBuf, 32, L"%.1f KB", sz.QuadPart / 1024.0);
        else
            StringCchPrintfW(szBuf, 32, L"%.2f MB", sz.QuadPart / (1024.0*1024.0));
        ListAddRow(L"File Size", szBuf, L"GetFileAttributesEx");

        SYSTEMTIME st{};
        FileTimeToLocalFileTime(&fad.ftLastWriteTime, &fad.ftLastWriteTime);
        FileTimeToSystemTime(&fad.ftLastWriteTime, &st);
        wchar_t dateBuf[64]{};
        StringCchPrintfW(dateBuf, 64, L"%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        ListAddRow(L"Last Modified", dateBuf, L"GetFileAttributesEx");

        // Attribute flags
        wchar_t attrs[64]{};
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  StringCchCatW(attrs, 64, L"R ");
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    StringCchCatW(attrs, 64, L"H ");
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    StringCchCatW(attrs, 64, L"S ");
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)StringCchCatW(attrs, 64, L"C ");
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) StringCchCatW(attrs, 64, L"E ");
        if (attrs[0] == L'\0') StringCchCopyW(attrs, 64, L"Normal");
        ListAddRow(L"Attributes", attrs, L"GetFileAttributesEx");
    }

    // IPropertyStore — file metadata properties
    IPropertyStore* ps{};
    if (SUCCEEDED(SHGetPropertyStoreFromParsingName(path, nullptr,
            GPS_DEFAULT, IID_PPV_ARGS(&ps)))) {
        DWORD propCount{};
        ps->GetCount(&propCount);
        wchar_t countBuf[32]{};
        StringCchPrintfW(countBuf, 32, L"%u properties", propCount);
        ListAddRow(L"Property Store", countBuf, L"IPropertyStore::GetCount");

        // Try to read a few common properties
        auto ReadProp = [&](const PROPERTYKEY& key, const wchar_t* name) {
            PROPVARIANT pv{};
            if (SUCCEEDED(ps->GetValue(key, &pv)) && pv.vt != VT_EMPTY) {
                wchar_t val[256]{};
                PropVariantToString(pv, val, ARRAYSIZE(val));
                if (val[0]) ListAddRow(name, val, L"IPropertyStore::GetValue");
            }
            PropVariantClear(&pv);
        };
        ReadProp(PKEY_Title,            L"Title");
        ReadProp(PKEY_Author,           L"Author");
        ReadProp(PKEY_Comment,          L"Comment");
        ReadProp(PKEY_FileDescription,  L"Description");
        ReadProp(PKEY_FileVersion,      L"File Version");
        ReadProp(PKEY_Software_ProductName, L"Product Name");
        ReadProp(PKEY_Company,          L"Company");
        ps->Release();
    }

    wchar_t status[128]{};
    StringCchPrintfW(status, 128, L"Items: %d", ListView_GetItemCount(g_hList));
    Status(0, status);
    Status(1, L"SHGetFileInfo + AssocQueryString + IPropertyStore");
    Log(L"File info complete — %d properties shown", ListView_GetItemCount(g_hList));
}

// ── Mode 5: Shortcuts (IShellLink) ───────────────────────────────────────────
void CreateShortcut()
{
    wchar_t lnkPath[MAX_PATH]{};
    GetTempPathW(ARRAYSIZE(lnkPath), lnkPath);
    StringCchCatW(lnkPath, ARRAYSIZE(lnkPath), L"Win32Learn_Notepad.lnk");

    // Target: notepad.exe in System32
    wchar_t targetPath[MAX_PATH]{};
    GetSystemDirectoryW(targetPath, ARRAYSIZE(targetPath));
    StringCchCatW(targetPath, ARRAYSIZE(targetPath), L"\\notepad.exe");

    ListClear();
    ListAddCol(0, 200, L"Property");
    ListAddCol(1, 400, L"Value");
    LogClear();
    Log(L"=== IShellLink — Create Shortcut ===");

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    IShellLinkW* psl{};
    hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
        IID_IShellLinkW, reinterpret_cast<void**>(&psl));
    if (FAILED(hr)) { Log(L"CoCreateInstance(CLSID_ShellLink) FAILED: 0x%08X", hr); return; }

    // Set shortcut properties
    psl->SetPath(targetPath);
    psl->SetDescription(L"Opens Notepad — Win32Roadmap IShellLink demo");
    psl->SetArguments(L"");
    psl->SetWorkingDirectory(targetPath);
    psl->SetShowCmd(SW_SHOWNORMAL);
    psl->SetHotkey(MAKEWORD('N', HOTKEYF_ALT | HOTKEYF_CONTROL));
    psl->SetIconLocation(targetPath, 0);

    Log(L"[IShellLink] SetPath: %s", targetPath);
    Log(L"[IShellLink] SetDescription: 'Opens Notepad — Win32Roadmap IShellLink demo'");
    Log(L"[IShellLink] SetHotkey: Ctrl+Alt+N");

    // Save via IPersistFile
    IPersistFile* ppf{};
    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));
    if (SUCCEEDED(hr)) {
        hr = ppf->Save(lnkPath, TRUE);
        Log(L"[IPersistFile] Save to '%s': %s", lnkPath, SUCCEEDED(hr)?L"OK":L"FAILED");
        ppf->Release();
    }
    psl->Release();

    if (SUCCEEDED(hr)) {
        ListAddRow(L"Shortcut Path",    lnkPath,    L"");
        ListAddRow(L"Target",           targetPath, L"SetPath");
        ListAddRow(L"Description", L"Opens Notepad — Win32Roadmap IShellLink demo", L"SetDescription");
        ListAddRow(L"Hot Key",     L"Ctrl+Alt+N",  L"SetHotkey");
        ListAddRow(L"Show Command",L"SW_SHOWNORMAL",L"SetShowCmd");

        // SHAddToRecentDocs
        SHAddToRecentDocs(SHARD_PATHW, lnkPath);
        Log(L"[Shell] SHAddToRecentDocs: added '%s' to Recent", lnkPath);
        ListAddRow(L"Recent Docs", L"Added to Recent Documents", L"SHAddToRecentDocs");

        Status(0, L"Shortcut created"); Status(1, lnkPath);
    }
    CoUninitialize();
}

void ReadShortcut()
{
    wchar_t lnkPath[MAX_PATH]{};
    GetTempPathW(ARRAYSIZE(lnkPath), lnkPath);
    StringCchCatW(lnkPath, ARRAYSIZE(lnkPath), L"Win32Learn_Notepad.lnk");

    if (GetFileAttributesW(lnkPath) == INVALID_FILE_ATTRIBUTES) {
        Log(L"Shortcut not found — run 'Create Shortcut' first");
        return;
    }

    ListClear();
    ListAddCol(0, 200, L"Property");
    ListAddCol(1, 400, L"Value");
    LogClear();
    Log(L"=== IShellLink — Read Shortcut: %s ===", lnkPath);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    IShellLinkW* psl{};
    hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
        IID_IShellLinkW, reinterpret_cast<void**>(&psl));
    if (FAILED(hr)) return;

    IPersistFile* ppf{};
    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));
    if (SUCCEEDED(hr)) {
        hr = ppf->Load(lnkPath, STGM_READ);
        Log(L"[IPersistFile] Load: %s", SUCCEEDED(hr) ? L"OK" : L"FAILED");
        ppf->Release();
    }

    if (SUCCEEDED(hr)) {
        // Resolve (needed to find target even if moved)
        psl->Resolve(g_hwnd, SLR_NO_UI | SLR_NOSEARCH);

        wchar_t buf[MAX_PATH]{};

        psl->GetPath(buf, ARRAYSIZE(buf), nullptr, SLGP_RAWPATH);
        ListAddRow(L"Target Path", buf, L"GetPath");
        Log(L"  Target:  %s", buf);

        psl->GetDescription(buf, ARRAYSIZE(buf));
        ListAddRow(L"Description", buf, L"GetDescription");
        Log(L"  Desc:    %s", buf);

        psl->GetWorkingDirectory(buf, ARRAYSIZE(buf));
        ListAddRow(L"Working Dir", buf, L"GetWorkingDirectory");

        psl->GetArguments(buf, ARRAYSIZE(buf));
        if (buf[0]) ListAddRow(L"Arguments", buf, L"GetArguments");

        int showCmd{};
        psl->GetShowCmd(&showCmd);
        ListAddRow(L"Show Command",
            showCmd == SW_SHOWNORMAL ? L"SW_SHOWNORMAL" : L"other", L"GetShowCmd");

        WORD hotKey{};
        psl->GetHotkey(&hotKey);
        if (hotKey) {
            wchar_t hkBuf[64]{};
            StringCchPrintfW(hkBuf, 64, L"vk=0x%02X mods=0x%02X",
                LOBYTE(hotKey), HIBYTE(hotKey));
            ListAddRow(L"Hot Key", hkBuf, L"GetHotkey");
            Log(L"  HotKey: %s", hkBuf);
        }

        int iconIdx{};
        psl->GetIconLocation(buf, ARRAYSIZE(buf), &iconIdx);
        wchar_t iconBuf[MAX_PATH + 8]{};
        StringCchPrintfW(iconBuf, ARRAYSIZE(iconBuf), L"%s, %d", buf, iconIdx);
        ListAddRow(L"Icon Location", iconBuf, L"GetIconLocation");

        // IShellLinkDataList for extra flags
        IShellLinkDataList* psdl{};
        if (SUCCEEDED(psl->QueryInterface(IID_IShellLinkDataList,
                reinterpret_cast<void**>(&psdl)))) {
            DWORD flags{};
            psdl->GetFlags(&flags);
            wchar_t fBuf[64]{};
            StringCchPrintfW(fBuf, 64, L"0x%08X", flags);
            ListAddRow(L"DataList Flags", fBuf, L"IShellLinkDataList");
            psdl->Release();
        }
    }
    psl->Release();
    CoUninitialize();

    Status(0, L"Shortcut read"); Status(1, lnkPath);
}

// ── Browse for folder ─────────────────────────────────────────────────────────
void BrowseForFile()
{
    OPENFILENAMEW ofn{};
    wchar_t path[MAX_PATH]{};
    GetWindowTextW(g_hPath, path, ARRAYSIZE(path));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = g_hwnd;
    ofn.lpstrFilter  = L"All Files\0*.*\0Executable\0*.exe\0DLL\0*.dll\0";
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = ARRAYSIZE(path);
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrInitialDir = L"C:\\Windows\\System32";
    if (GetOpenFileNameW(&ofn))
        SetWindowTextW(g_hPath, path);
}

// ── Create All Controls ───────────────────────────────────────────────────────
void CreateAll(HWND hwnd)
{
    g_smallFont = CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");

    // Mode buttons
    static const wchar_t* modeLabels[] = {
        L"1 · Tray Icon", L"2 · File Operations",
        L"3 · Known Folders", L"4 · File Info", L"5 · Shortcuts"
    };
    for (int i = 0; i < 5; ++i)
        CreateWindowW(L"BUTTON", modeLabels[i],
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            0,0,0,0, hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_M1+i)),
            g_hInst, nullptr);

    // ListView (main output)
    g_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_LIST), g_hInst, nullptr);
    ListView_SetExtendedListViewStyle(g_hList,
        LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
    ListView_SetImageList(g_hList, nullptr, LVSIL_SMALL);

    // Log edit (secondary output)
    g_hLog = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL|ES_AUTOVSCROLL,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_LOG), g_hInst, nullptr);
    SendMessageW(g_hLog, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);

    // Status bar (2 parts)
    g_hStatus = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_STATUS), g_hInst, nullptr);
    int sbParts[] = { 320, -1 };
    SendMessageW(g_hStatus, SB_SETPARTS, 2, reinterpret_cast<LPARAM>(sbParts));

    // Path + Browse (shared across modes)
    g_hPath = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
        L"C:\\Windows\\System32\\notepad.exe",
        WS_CHILD|ES_AUTOHSCROLL, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_PATH), g_hInst, nullptr);
    SendMessageW(g_hPath, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);
    g_hBrowse = CreateWindowW(L"BUTTON", L"Browse…",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BROWSE), g_hInst, nullptr);

    // Mode 1 extra buttons
    g_hBalloon     = CreateWindowW(L"BUTTON", L"Show Balloon",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_BALLOON), g_hInst, nullptr);
    g_hAnimate     = CreateWindowW(L"BUTTON", L"Start Animate",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_ANIMATE), g_hInst, nullptr);
    g_hHideRestore = CreateWindowW(L"BUTTON", L"Hide Tray Icon",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_HIDERESTORE), g_hInst, nullptr);

    // Mode 2 extra buttons
    g_hBtnCopy   = CreateWindowW(L"BUTTON", L"FO_COPY", WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_BTN_COPY),   g_hInst, nullptr);
    g_hBtnMove   = CreateWindowW(L"BUTTON", L"FO_MOVE", WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_BTN_MOVE),   g_hInst, nullptr);
    g_hBtnDelete = CreateWindowW(L"BUTTON", L"FO_DELETE (Recycle)", WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_BTN_DELETE), g_hInst, nullptr);
    g_hBtnOpen   = CreateWindowW(L"BUTTON", L"Open Folder & Select", WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_BTN_OPEN),   g_hInst, nullptr);

    // Mode 4 extra
    g_hBtnFileInfo = CreateWindowW(L"BUTTON", L"Get File Info",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_FILEINFO), g_hInst, nullptr);

    // Mode 5 extra
    g_hBtnMkLink = CreateWindowW(L"BUTTON", L"Create Shortcut (.lnk)",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_MKLINK), g_hInst, nullptr);
    g_hBtnRdLink = CreateWindowW(L"BUTTON", L"Read Shortcut Properties",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_RDLINK), g_hInst, nullptr);

    // Init mode 1
    ShowModeControls(1);
    TrayAdd();
    Log(L"=== 09 ShellIntegration ===");
    Log(L"Tray icon added. Right-click the tray icon for context menu.");
    Log(L"Balloon notifications: NIF_INFO with NIIF_INFO/WARNING/ERROR types.");
    Log(L"Use the mode buttons above to explore other Shell APIs.");
}

// ── Window Proc ───────────────────────────────────────────────────────────────
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        g_hInst = reinterpret_cast<LPCREATESTRUCTW>(lp)->hInstance;
        g_hwnd  = hwnd;
        CreateAll(hwnd);
        Layout(hwnd);
        return 0;

    case WM_SIZE:
        Layout(hwnd);
        return 0;

    case WM_TIMER:
        if (wp == IDT_ANIMATE && g_animating) {
            static const wchar_t* icons[] = {
                nullptr, // IDI_APPLICATION
                (const wchar_t*)IDI_INFORMATION,
                (const wchar_t*)IDI_QUESTION,
                (const wchar_t*)IDI_ASTERISK,
            };
            ++g_animFrame;
            NOTIFYICONDATAW nid{ sizeof(nid) };
            nid.hWnd = hwnd; nid.uID = 1;
            nid.uFlags = NIF_ICON | NIF_GUID; nid.guidItem = kTrayGuid;
            nid.hIcon = LoadIconW(nullptr,
                g_animFrame % 2 == 0 ? IDI_APPLICATION : IDI_INFORMATION);
            Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        // Mode switch
        case IDC_BTN_M1:
            ShowModeControls(1); Layout(hwnd);
            Log(L"--- Mode 1: Tray Icon ---");
            Status(0, L"Tray Icon"); Status(1, L"Shell_NotifyIcon v4");
            break;
        case IDC_BTN_M2:
            ShowModeControls(2); Layout(hwnd);
            LogClear(); Log(L"--- Mode 2: File Operations ---");
            Status(0, L"File Operations"); Status(1, L"SHFileOperation");
            break;
        case IDC_BTN_M3:
            ShowModeControls(3); Layout(hwnd);
            ShowKnownFolders();
            break;
        case IDC_BTN_M4:
            ShowModeControls(4); Layout(hwnd);
            LogClear(); Log(L"--- Mode 4: File Info — enter path then click Get File Info ---");
            Status(0, L"File Info"); Status(1, L"SHGetFileInfo + AssocQueryString");
            break;
        case IDC_BTN_M5:
            ShowModeControls(5); Layout(hwnd);
            LogClear(); Log(L"--- Mode 5: IShellLink Shortcuts ---");
            Status(0, L"Shortcuts"); Status(1, L"IShellLink + IPersistFile");
            break;
        // Mode 1
        case IDC_BTN_BALLOON:    TrayBalloon(); break;
        case IDC_BTN_ANIMATE:    TrayAnimate(); break;
        case IDC_BTN_HIDERESTORE:TrayHideRestore(); break;
        // Mode 2
        case IDC_BTN_COPY:   DoCopy(); break;
        case IDC_BTN_MOVE:   DoMove(); break;
        case IDC_BTN_DELETE: DoDelete(); break;
        case IDC_BTN_OPEN:   DoOpenAndSelect(); break;
        case IDC_BROWSE:     BrowseForFile(); break;
        // Mode 4
        case IDC_BTN_FILEINFO: ShowFileInfo(); break;
        // Mode 5
        case IDC_BTN_MKLINK: CreateShortcut(); break;
        case IDC_BTN_RDLINK: ReadShortcut();   break;
        // Tray menu
        case ID_TRAY_SHOW:    ShowWindow(hwnd, SW_RESTORE); SetForegroundWindow(hwnd); break;
        case ID_TRAY_BALLOON: TrayBalloon(); break;
        case ID_TRAY_EXIT:    DestroyWindow(hwnd); break;
        }
        return 0;

    case WM_TRAYICON: {
        // NOTIFYICON_VERSION_4: LOWORD(lp)=event, HIWORD(lp)=icon ID
        UINT event = LOWORD(lp);
        if (event == WM_RBUTTONUP || event == WM_CONTEXTMENU) {
            POINT pt{};
            GetCursorPos(&pt);
            TrayContextMenu(pt);
        }
        if (event == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
        }
        if (event == NIN_BALLOONSHOW)
            Log(L"[Tray] NIN_BALLOONSHOW — balloon appeared");
        if (event == NIN_BALLOONTIMEOUT)
            Log(L"[Tray] NIN_BALLOONTIMEOUT — balloon dismissed by timeout");
        if (event == NIN_BALLOONUSERCLICK)
            Log(L"[Tray] NIN_BALLOONUSERCLICK — user clicked the balloon");
        return 0;
    }

    case WM_DESTROY:
        TrayRemove();
        KillTimer(hwnd, IDT_ANIMATE);
        if (g_smallFont) DeleteObject(g_smallFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc),
        ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
    wc.lpszClassName = L"Win32Roadmap_ShellIntegration";
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, L"Win32Roadmap_ShellIntegration",
        L"09  Shell Integration — Tray · SHFileOperation · Known Folders · File Info · IShellLink",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 760,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) return static_cast<int>(GetLastError());

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
