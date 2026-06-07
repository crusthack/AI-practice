// 07_CommonControls — Complete Common Controls Showcase
// Toolbar | TabControl (Explorer / Input / Misc) | StatusBar
// APIs: TreeView, ListView, Toolbar, ProgressBar, TrackBar, UpDown,
//       DateTimePicker, MonthCalendar, Tooltip, IP Address, ImageList, Rebar

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <Shellapi.h>
#include <windowsx.h>
#include <strsafe.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

// ── Control IDs ───────────────────────────────────────────────────────────────
enum {
    IDC_TOOLBAR  = 100, IDC_TAB      = 101,
    IDC_TREE     = 102, IDC_LIST     = 103,
    IDC_PROGRESS = 104, IDC_TRACKBAR = 105,
    IDC_SPIN     = 106, IDC_SPINBUD  = 107,
    IDC_DATETIME = 108, IDC_MONTHCAL = 109,
    IDC_IPADDR   = 110, IDC_STATUS   = 111,
    IDC_TRKVAL   = 112, IDC_PROGVAL  = 113,
    IDC_GRPPROG  = 114, IDC_GRPRNG   = 115,
    IDC_GRPDATE  = 116, IDC_GRPMISC  = 117,
    IDC_LBLIP    = 118, IDC_TOOLTIP_BTN = 119,
    // Toolbar buttons
    IDT_ADD = 200, IDT_REMOVE = 201, IDT_SORT = 202,
    IDT_VIEW = 203, IDT_EXPAND = 204, IDT_PROG = 205,
};

// ── Globals ───────────────────────────────────────────────────────────────────
HWND      g_hwnd{}, g_hToolbar{}, g_hTab{};
HWND      g_hTree{}, g_hList{};
HWND      g_hProgress{}, g_hTrack{}, g_hSpin{}, g_hSpinBud{};
HWND      g_hDateTime{}, g_hMonthCal{};
HWND      g_hIpAddr{}, g_hTooltipBtn{};
HWND      g_hTrkVal{}, g_hProgVal{};
HWND      g_hGrpProg{}, g_hGrpRng{}, g_hGrpDate{}, g_hGrpMisc{};
HWND      g_hLblIp{};
HWND      g_hTooltip{};
HWND      g_hStatus{};
HINSTANCE g_hInst{};
HIMAGELIST g_hSysImgList{};

int  g_currentTab    = 0;
int  g_sortCol       = 0;
bool g_sortAsc       = true;
int  g_viewMode      = 0;   // 0=Report 1=LargeIcon 2=SmallIcon 3=List
bool g_marquee       = false;
int  g_progVal       = 0;
UINT_PTR g_timerProg = 0;

// Which controls belong to each tab (for show/hide)
HWND g_tabCtrl[3][20]{};
int   g_tabCount[3]{};

void TabRegister(int tab, HWND h)
{
    if (tab < 3 && g_tabCount[tab] < 20)
        g_tabCtrl[tab][g_tabCount[tab]++] = h;
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void Status(int part, const wchar_t* text)
{
    SendMessageW(g_hStatus, SB_SETTEXT, part, reinterpret_cast<LPARAM>(text));
}

void ListAddColumn(int idx, int width, const wchar_t* text)
{
    LVCOLUMNW c{};
    c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    c.iSubItem = idx; c.cx = width;
    c.pszText = const_cast<wchar_t*>(text);
    ListView_InsertColumn(g_hList, idx, &c);
}

// ── System Image List ─────────────────────────────────────────────────────────
int SysIconOf(const wchar_t* path, DWORD attrs = 0)
{
    SHFILEINFOW sfi{};
    DWORD flags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
    if (attrs) flags |= SHGFI_USEFILEATTRIBUTES;
    SHGetFileInfoW(path, attrs, &sfi, sizeof(sfi), flags);
    return sfi.iIcon;
}

void InitSysImageList()
{
    SHFILEINFOW sfi{};
    g_hSysImgList = reinterpret_cast<HIMAGELIST>(
        SHGetFileInfoW(L"C:\\", 0, &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
}

// ── TreeView ─────────────────────────────────────────────────────────────────
HTREEITEM TreeAddItem(HTREEITEM parent, const wchar_t* text, int icon,
                      LPARAM data = 0, bool checked = false)
{
    TVINSERTSTRUCTW tvi{};
    tvi.hParent      = parent;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask    = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
    tvi.item.pszText = const_cast<wchar_t*>(text);
    tvi.item.iImage  = icon;
    tvi.item.iSelectedImage = icon;
    tvi.item.lParam  = data;
    if (checked) {
        tvi.item.stateMask = TVIS_STATEIMAGEMASK;
        tvi.item.state     = INDEXTOSTATEIMAGEMASK(2); // checked
    }
    return TreeView_InsertItem(g_hTree, &tvi);
}

void FillTree()
{
    int iFolder = SysIconOf(L"C:\\Windows",         FILE_ATTRIBUTE_DIRECTORY);
    int iDll    = SysIconOf(L"x.dll",               FILE_ATTRIBUTE_NORMAL);
    int iExe    = SysIconOf(L"x.exe",               FILE_ATTRIBUTE_NORMAL);
    int iHdr    = SysIconOf(L"x.h",                 FILE_ATTRIBUTE_NORMAL);
    int iCpp    = SysIconOf(L"x.cpp",               FILE_ATTRIBUTE_NORMAL);

    HTREEITEM root = TreeAddItem(TVI_ROOT, L"Win32 Roadmap", iFolder);

    HTREEITEM p1 = TreeAddItem(root, L"Phase1_UI",          iFolder, 1);
    TreeAddItem(p1, L"01_WindowBasics\\main.cpp",   iCpp,  0, true);
    TreeAddItem(p1, L"02_MessageLoop\\main.cpp",    iCpp,  0, true);
    TreeAddItem(p1, L"03_GDI_Painting\\main.cpp",   iCpp);
    TreeAddItem(p1, L"04_Controls\\main.cpp",       iCpp);
    TreeAddItem(p1, L"05_Dialogs\\main.cpp",        iCpp);
    TreeAddItem(p1, L"06_Menus_Accel\\main.cpp",    iCpp);

    HTREEITEM p2 = TreeAddItem(root, L"Phase2_AdvancedUI",  iFolder, 2);
    TreeAddItem(p2, L"07_CommonControls\\main.cpp", iCpp);
    TreeAddItem(p2, L"08_OwnerDraw\\main.cpp",      iCpp);
    TreeAddItem(p2, L"09_ShellIntegration\\main.cpp", iCpp);

    HTREEITEM p3 = TreeAddItem(root, L"Phase3_System",      iFolder, 3);
    TreeAddItem(p3, L"10_FileIO\\main.cpp",         iCpp,  0, true);
    TreeAddItem(p3, L"11_ProcessThread\\main.cpp",  iCpp);

    HTREEITEM p4 = TreeAddItem(root, L"Phase4_COM",         iFolder, 4);
    TreeAddItem(p4, L"24_D2D_DWrite\\main.cpp",     iCpp);

    HTREEITEM p5 = TreeAddItem(root, L"Phase5_Security_Net",iFolder, 5);
    TreeAddItem(p5, L"25_Security\\main.cpp",       iCpp);

    HTREEITEM p6 = TreeAddItem(root, L"Phase6_Advanced",    iFolder, 6);
    TreeAddItem(p6, L"35_Hook\\main.cpp",           iCpp);

    HTREEITEM p7 = TreeAddItem(root, L"Phase7_Capstone",    iFolder, 7);
    TreeAddItem(p7, L"40_ProcessExplorer\\main.cpp",iCpp);

    HTREEITEM sln = TreeAddItem(root, L"Win32.slnx",       iHdr);

    TreeView_Expand(g_hTree, root, TVE_EXPAND);
    TreeView_Expand(g_hTree, p1,   TVE_EXPAND);
    TreeView_Expand(g_hTree, p2,   TVE_EXPAND);
    TreeView_SelectItem(g_hTree, p1);
}

// ── ListView ──────────────────────────────────────────────────────────────────
struct ListItem { const wchar_t* name; const wchar_t* size; const wchar_t* type; const wchar_t* date; int icon; };
static const ListItem kItems[] = {
    { L"01_WindowBasics\\main.cpp",    L"4.1 KB",  L"C++ Source", L"2025-01-10", 0 },
    { L"02_MessageLoop\\main.cpp",     L"5.2 KB",  L"C++ Source", L"2025-01-11", 0 },
    { L"03_GDI_Painting\\main.cpp",    L"3.8 KB",  L"C++ Source", L"2025-01-12", 0 },
    { L"04_Controls\\main.cpp",        L"4.6 KB",  L"C++ Source", L"2025-01-13", 0 },
    { L"05_Dialogs\\main.cpp",         L"4.9 KB",  L"C++ Source", L"2025-01-14", 0 },
    { L"06_Menus_Accel\\main.cpp",     L"5.1 KB",  L"C++ Source", L"2025-01-15", 0 },
    { L"07_CommonControls\\main.cpp",  L"9.2 KB",  L"C++ Source", L"2025-02-01", 0 },
    { L"08_OwnerDraw\\main.cpp",       L"8.7 KB",  L"C++ Source", L"2025-02-02", 0 },
    { L"09_ShellIntegration\\main.cpp",L"10.1 KB", L"C++ Source", L"2025-02-03", 0 },
    { L"Win32.slnx",                   L"2.3 KB",  L"Solution",   L"2025-01-05", 1 },
    { L"README.md",                    L"1.1 KB",  L"Markdown",   L"2025-01-01", 2 },
    { L"WIN32_LEARNING_GUIDE.md",      L"14.5 KB", L"Markdown",   L"2025-01-02", 2 },
};

void FillList()
{
    ListView_DeleteAllItems(g_hList);
    int iCpp = SysIconOf(L"x.cpp", FILE_ATTRIBUTE_NORMAL);
    int iSlnx= SysIconOf(L"x.slnx",FILE_ATTRIBUTE_NORMAL);
    int iMd  = SysIconOf(L"x.md",  FILE_ATTRIBUTE_NORMAL);
    int icons[] = { iCpp, iSlnx, iMd };

    for (int i = 0; i < ARRAYSIZE(kItems); ++i) {
        LVITEMW lvi{};
        lvi.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.iItem   = i;
        lvi.pszText = const_cast<wchar_t*>(kItems[i].name);
        lvi.iImage  = icons[kItems[i].icon];
        lvi.lParam  = i;
        ListView_InsertItem(g_hList, &lvi);
        ListView_SetItemText(g_hList, i, 1, const_cast<wchar_t*>(kItems[i].size));
        ListView_SetItemText(g_hList, i, 2, const_cast<wchar_t*>(kItems[i].type));
        ListView_SetItemText(g_hList, i, 3, const_cast<wchar_t*>(kItems[i].date));
    }

    wchar_t buf[64]{};
    StringCchPrintfW(buf, 64, L"Items: %d", ARRAYSIZE(kItems));
    Status(2, buf);
}

// ListView sort callback
int CALLBACK LVSort(LPARAM a, LPARAM b, LPARAM col)
{
    const ListItem& ia = kItems[a];
    const ListItem& ib = kItems[b];
    const wchar_t* sa = (col==0)?ia.name:(col==1)?ia.size:(col==2)?ia.type:ia.date;
    const wchar_t* sb = (col==0)?ib.name:(col==1)?ib.size:(col==2)?ib.type:ib.date;
    int cmp = lstrcmpiW(sa, sb);
    return g_sortAsc ? cmp : -cmp;
}

void SortList()
{
    SendMessageW(g_hList, LVM_SORTITEMSEX,
        static_cast<WPARAM>(g_sortCol),
        reinterpret_cast<LPARAM>(LVSort));

    // Draw sort arrow in column header
    HWND hHdr = ListView_GetHeader(g_hList);
    for (int i = 0; i < 4; ++i) {
        HDITEMW hdi{};
        hdi.mask = HDI_FORMAT;
        Header_GetItem(hHdr, i, &hdi);
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        if (i == g_sortCol)
            hdi.fmt |= g_sortAsc ? HDF_SORTUP : HDF_SORTDOWN;
        Header_SetItem(hHdr, i, &hdi);
    }
}

// Cycle ListView view modes
void CycleViewMode()
{
    static const DWORD modes[] = { LV_VIEW_DETAILS, LV_VIEW_ICON, LV_VIEW_SMALLICON, LV_VIEW_LIST };
    static const wchar_t* modeNames[] = { L"Details", L"Large Icons", L"Small Icons", L"List" };
    g_viewMode = (g_viewMode + 1) % 4;
    SendMessageW(g_hList, LVM_SETVIEW, modes[g_viewMode], 0);
    Status(0, modeNames[g_viewMode]);
}

// ── Toolbar ───────────────────────────────────────────────────────────────────
void CreateToolbar(HWND hwnd)
{
    g_hToolbar = CreateWindowExW(0, TOOLBARCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_TOP | CCS_NODIVIDER,
        0, 0, 0, 36, hwnd, reinterpret_cast<HMENU>(IDC_TOOLBAR), g_hInst, nullptr);

    SendMessageW(g_hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessageW(g_hToolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

    // Use standard bitmap set
    TBADDBITMAP tbab{ HINST_COMMCTRL, IDB_STD_SMALL_COLOR };
    SendMessageW(g_hToolbar, TB_ADDBITMAP, 15, reinterpret_cast<LPARAM>(&tbab));

    TBBUTTON tbb[] = {
        { STD_FILENEW,  IDT_ADD,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"Add Item")   },
        { STD_DELETE,   IDT_REMOVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"Remove")    },
        { 0, 0, 0, TBSTYLE_SEP, {}, 0, 0 },
        { STD_FILESAVE, IDT_SORT,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"Sort")      },
        { STD_FIND,     IDT_VIEW,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"View Mode") },
        { STD_PROPERTIES, IDT_EXPAND, TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"Expand/Collapse") },
        { 0, 0, 0, TBSTYLE_SEP, {}, 0, 0 },
        { STD_PASTE,    IDT_PROG,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {}, 0, reinterpret_cast<INT_PTR>(L"Start/Stop Progress") },
    };
    SendMessageW(g_hToolbar, TB_ADDBUTTONS, ARRAYSIZE(tbb), reinterpret_cast<LPARAM>(tbb));
    SendMessageW(g_hToolbar, TB_AUTOSIZE, 0, 0);
}

// ── Layout ────────────────────────────────────────────────────────────────────
void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);

    // Toolbar
    SendMessageW(g_hToolbar, WM_SIZE, 0, 0);
    RECT tbRc{};
    GetWindowRect(g_hToolbar, &tbRc);
    int tbH = tbRc.bottom - tbRc.top;

    // StatusBar
    SendMessageW(g_hStatus, WM_SIZE, 0, 0);
    RECT sbRc{};
    GetWindowRect(g_hStatus, &sbRc);
    int sbH = sbRc.bottom - sbRc.top;

    // Tab control
    const int pad = 6;
    int tabTop  = tbH + pad;
    int tabH    = rc.bottom - sbH - tabTop;
    MoveWindow(g_hTab, pad, tabTop, rc.right - pad*2, tabH, TRUE);

    // Tab content area (inside tab control)
    RECT tabRc{ pad, tabTop, rc.right - pad*2, tabTop + tabH };
    TabCtrl_AdjustRect(g_hTab, FALSE, &tabRc);
    int cx = tabRc.left, cy = tabRc.top;
    int cw = tabRc.right - tabRc.left, ch = tabRc.bottom - tabRc.top;

    // --- Tab 0: Tree + List ---
    int treeW = 220;
    MoveWindow(g_hTree, cx,           cy, treeW,    ch, TRUE);
    MoveWindow(g_hList, cx+treeW+4,   cy, cw-treeW-4, ch, TRUE);

    // --- Tab 1: Input Controls ---
    int groupW = (cw - pad*3) / 2;
    // Progress group (left top)
    MoveWindow(g_hGrpProg,  cx,              cy,       groupW, 110, TRUE);
    MoveWindow(g_hProgress, cx+10,           cy+22,    groupW-20, 22, TRUE);
    MoveWindow(g_hProgVal,  cx+10,           cy+52,    groupW-20, 22, TRUE);
    // Range group (right top)
    MoveWindow(g_hGrpRng,   cx+groupW+pad,   cy,       groupW, 110, TRUE);
    MoveWindow(g_hTrack,    cx+groupW+pad+10,cy+22,    groupW-20, 30, TRUE);
    MoveWindow(g_hTrkVal,   cx+groupW+pad+10,cy+60,    60, 22, TRUE);
    MoveWindow(g_hSpinBud,  cx+groupW+pad+80,cy+60,    60, 22, TRUE);
    MoveWindow(g_hSpin,     cx+groupW+pad+140,cy+60,   24, 22, TRUE);
    // Date group (left bottom)
    MoveWindow(g_hGrpDate,  cx,              cy+120,   groupW, 130, TRUE);
    MoveWindow(g_hDateTime, cx+10,           cy+142,   groupW-20, 26, TRUE);
    MoveWindow(g_hMonthCal, cx+10,           cy+176,   groupW-20, 70, TRUE);
    // Misc group (right bottom)
    MoveWindow(g_hGrpMisc,  cx+groupW+pad,   cy+120,   groupW, 130, TRUE);
    MoveWindow(g_hLblIp,    cx+groupW+pad+10,cy+142,   80, 22, TRUE);
    MoveWindow(g_hIpAddr,   cx+groupW+pad+95,cy+142,   120, 22, TRUE);
    MoveWindow(g_hTooltipBtn,cx+groupW+pad+10,cy+176,  groupW-20, 30, TRUE);
}

// Show controls for selected tab, hide others
void ApplyTab(int tab)
{
    for (int t = 0; t < 3; ++t) {
        int sw = (t == tab) ? SW_SHOW : SW_HIDE;
        for (int i = 0; i < g_tabCount[t]; ++i)
            ShowWindow(g_tabCtrl[t][i], sw);
    }
}

// ── Controls Creation ─────────────────────────────────────────────────────────
void CreateAllControls(HWND hwnd)
{
    InitSysImageList();

    // --- Toolbar ---
    CreateToolbar(hwnd);

    // --- Tab control ---
    g_hTab = CreateWindowExW(0, WC_TABCONTROLW, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_HOTTRACK,
        0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(IDC_TAB), g_hInst, nullptr);

    TCITEMW tci{ TCIF_TEXT };
    tci.pszText = const_cast<wchar_t*>(L"  Tree & List  ");
    TabCtrl_InsertItem(g_hTab, 0, &tci);
    tci.pszText = const_cast<wchar_t*>(L"  Input Controls  ");
    TabCtrl_InsertItem(g_hTab, 1, &tci);
    tci.pszText = const_cast<wchar_t*>(L"  Misc Controls  ");
    TabCtrl_InsertItem(g_hTab, 2, &tci);

    // --- Status bar (3 parts) ---
    g_hStatus = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(IDC_STATUS), g_hInst, nullptr);
    {
        int parts[] = { 260, 520, -1 };
        SendMessageW(g_hStatus, SB_SETPARTS, 3, reinterpret_cast<LPARAM>(parts));
        Status(0, L"Tree & List"); Status(1, L"(nothing selected)"); Status(2, L"Items: 0");
    }

    // === TAB 0: TreeView + ListView ===
    // TreeView
    g_hTree = CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE |
        TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |
        TVS_SHOWSELALWAYS | TVS_CHECKBOXES | TVS_EDITLABELS | TVS_INFOTIP,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_TREE), g_hInst, nullptr);
    TreeView_SetImageList(g_hTree, g_hSysImgList, TVSIL_NORMAL);
    FillTree();
    TabRegister(0, g_hTree);

    // ListView
    g_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_LIST), g_hInst, nullptr);
    ListView_SetExtendedListViewStyle(g_hList,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER |
        LVS_EX_CHECKBOXES | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);
    ListView_SetImageList(g_hList, g_hSysImgList, LVSIL_SMALL);
    ListView_SetImageList(g_hList, g_hSysImgList, LVSIL_NORMAL);
    ListAddColumn(0, 260, L"Name");
    ListAddColumn(1, 70,  L"Size");
    ListAddColumn(2, 100, L"Type");
    ListAddColumn(3, 130, L"Modified");
    FillList();
    TabRegister(0, g_hList);

    // === TAB 1: Input Controls ===
    // Progress group
    g_hGrpProg = CreateWindowW(L"BUTTON", L"ProgressBar",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_GRPPROG), g_hInst, nullptr);
    g_hProgress = CreateWindowExW(0, PROGRESS_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_PROGRESS), g_hInst, nullptr);
    SendMessageW(g_hProgress, PBM_SETRANGE32, 0, 100);
    SendMessageW(g_hProgress, PBM_SETSTEP, 1, 0);
    SendMessageW(g_hProgress, PBM_SETPOS, 35, 0);
    g_hProgVal = CreateWindowW(L"STATIC", L"Progress: 35%",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_PROGVAL), g_hInst, nullptr);
    TabRegister(1, g_hGrpProg);
    TabRegister(1, g_hProgress);
    TabRegister(1, g_hProgVal);

    // Range group (TrackBar + Spin)
    g_hGrpRng = CreateWindowW(L"BUTTON", L"TrackBar & SpinControl",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_GRPRNG), g_hInst, nullptr);
    g_hTrack = CreateWindowExW(0, TRACKBAR_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS | TBS_BOTH,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_TRACKBAR), g_hInst, nullptr);
    SendMessageW(g_hTrack, TBM_SETRANGE,    TRUE, MAKELONG(0, 100));
    SendMessageW(g_hTrack, TBM_SETPOS,      TRUE, 50);
    SendMessageW(g_hTrack, TBM_SETTICFREQ,  10,   0);
    SendMessageW(g_hTrack, TBM_SETPAGESIZE, 0,    10);
    g_hTrkVal = CreateWindowW(L"STATIC", L"50",
        WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_TRKVAL), g_hInst, nullptr);
    g_hSpinBud = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"5",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_SPINBUD), g_hInst, nullptr);
    g_hSpin = CreateWindowExW(0, UPDOWN_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_HOTTRACK,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_SPIN), g_hInst, nullptr);
    SendMessageW(g_hSpin, UDM_SETRANGE32, 1, 100);
    SendMessageW(g_hSpin, UDM_SETPOS32,  0, 5);
    SendMessageW(g_hSpin, UDM_SETBUDDY,  reinterpret_cast<WPARAM>(g_hSpinBud), 0);
    TabRegister(1, g_hGrpRng);
    TabRegister(1, g_hTrack);
    TabRegister(1, g_hTrkVal);
    TabRegister(1, g_hSpinBud);
    TabRegister(1, g_hSpin);

    // Date group
    g_hGrpDate = CreateWindowW(L"BUTTON", L"DateTimePicker & MonthCalendar",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_GRPDATE), g_hInst, nullptr);
    g_hDateTime = CreateWindowExW(0, DATETIMEPICK_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT | DTS_SHOWNONE,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_DATETIME), g_hInst, nullptr);
    g_hMonthCal = CreateWindowExW(0, MONTHCAL_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | MCS_WEEKNUMBERS,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_MONTHCAL), g_hInst, nullptr);
    TabRegister(1, g_hGrpDate);
    TabRegister(1, g_hDateTime);
    TabRegister(1, g_hMonthCal);

    // Misc group (IP Address + Tooltip demo)
    g_hGrpMisc = CreateWindowW(L"BUTTON", L"IP Address & Tooltip",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_GRPMISC), g_hInst, nullptr);
    g_hLblIp = CreateWindowW(L"STATIC", L"IP Address:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_LBLIP), g_hInst, nullptr);
    g_hIpAddr = CreateWindowExW(WS_EX_CLIENTEDGE, WC_IPADDRESSW, nullptr,
        WS_CHILD | WS_VISIBLE,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_IPADDR), g_hInst, nullptr);
    SendMessageW(g_hIpAddr, IPM_SETADDRESS, 0, MAKEIPADDRESS(192,168,1,1));
    g_hTooltipBtn = CreateWindowW(L"BUTTON", L"Hover me for tooltip",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_TOOLTIP_BTN), g_hInst, nullptr);
    // Tooltip control
    g_hTooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOANIMATE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, nullptr, g_hInst, nullptr);
    SendMessageW(g_hTooltip, TTM_SETMAXTIPWIDTH, 0, 260);
    // Register tooltip tool for button
    TOOLINFOW ti{ sizeof(TOOLINFOW) };
    ti.uFlags   = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd     = hwnd;
    ti.uId      = reinterpret_cast<UINT_PTR>(g_hTooltipBtn);
    ti.lpszText = const_cast<wchar_t*>(
        L"This is a balloon tooltip!\r\n"
        L"Created with TOOLTIPS_CLASS.\r\n"
        L"Supports multi-line text.");
    SendMessageW(g_hTooltip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
    SendMessageW(g_hTooltip, TTM_ACTIVATE, TRUE, 0);
    TabRegister(1, g_hGrpMisc);
    TabRegister(1, g_hLblIp);
    TabRegister(1, g_hIpAddr);
    TabRegister(1, g_hTooltipBtn);

    // === TAB 2: Extra controls demo ===
    // Nothing shown in tab 2 for now (content is handled via WM_PAINT of a child)
    // We'll re-use the list with different content for tab 2

    // Initially show tab 0
    ApplyTab(0);
}

// ── TreeView Context Menu ─────────────────────────────────────────────────────
void ShowTreeContextMenu(HWND hwnd, POINT screenPt)
{
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"Add Child Node");
    AppendMenuW(menu, MF_STRING, 2, L"Delete Node");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 3, L"Expand All");
    AppendMenuW(menu, MF_STRING, 4, L"Collapse All");
    AppendMenuW(menu, MF_STRING, 5, L"Rename (F2)");
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
        screenPt.x, screenPt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);

    HTREEITEM sel = TreeView_GetSelection(g_hTree);
    switch (cmd) {
    case 1: {
        if (sel) {
            int iFolder = SysIconOf(L"C:\\Windows", FILE_ATTRIBUTE_DIRECTORY);
            HTREEITEM newItem = TreeAddItem(sel, L"New Node", iFolder);
            TreeView_SelectItem(g_hTree, newItem);
            TreeView_EnsureVisible(g_hTree, newItem);
            TreeView_EditLabel(g_hTree, newItem);
        }
        break;
    }
    case 2:
        if (sel) TreeView_DeleteItem(g_hTree, sel);
        break;
    case 3:
        TreeView_Expand(g_hTree, TreeView_GetRoot(g_hTree), TVE_EXPAND);
        break;
    case 4:
        TreeView_Expand(g_hTree, TreeView_GetRoot(g_hTree), TVE_COLLAPSE);
        break;
    case 5:
        if (sel) TreeView_EditLabel(g_hTree, sel);
        break;
    }
}

// ── Window Proc ───────────────────────────────────────────────────────────────
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        g_hInst = reinterpret_cast<LPCREATESTRUCTW>(lp)->hInstance;
        g_hwnd  = hwnd;
        CreateAllControls(hwnd);
        Layout(hwnd);
        return 0;

    case WM_SIZE:
        Layout(hwnd);
        return 0;

    case WM_TIMER:
        if (wp == 1 && !g_marquee) {
            g_progVal = (g_progVal + 2) % 101;
            SendMessageW(g_hProgress, PBM_SETPOS, g_progVal, 0);
            wchar_t buf[32]{};
            StringCchPrintfW(buf, 32, L"Progress: %d%%", g_progVal);
            SetWindowTextW(g_hProgVal, buf);
            // Toggle state for demo: error at >80, paused at >60
            if (g_progVal > 80)
                SendMessageW(g_hProgress, PBM_SETSTATE, PBST_ERROR, 0);
            else if (g_progVal > 60)
                SendMessageW(g_hProgress, PBM_SETSTATE, PBST_PAUSED, 0);
            else
                SendMessageW(g_hProgress, PBM_SETSTATE, PBST_NORMAL, 0);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDT_ADD: {
            int iCpp = SysIconOf(L"x.cpp", FILE_ATTRIBUTE_NORMAL);
            HTREEITEM sel = TreeView_GetSelection(g_hTree);
            HTREEITEM newItem = TreeAddItem(sel ? sel : TVI_ROOT, L"NewFile.cpp", iCpp);
            TreeView_SelectItem(g_hTree, newItem);
            TreeView_EnsureVisible(g_hTree, newItem);
            TreeView_EditLabel(g_hTree, newItem);

            int n = ListView_GetItemCount(g_hList);
            LVITEMW lvi{ LVIF_TEXT }; lvi.iItem = n;
            lvi.pszText = const_cast<wchar_t*>(L"NewFile.cpp");
            ListView_InsertItem(g_hList, &lvi);
            ListView_SetItemText(g_hList, n, 1, const_cast<wchar_t*>(L"0 KB"));
            ListView_SetItemText(g_hList, n, 2, const_cast<wchar_t*>(L"C++ Source"));
            break;
        }
        case IDT_REMOVE: {
            int sel = ListView_GetNextItem(g_hList, -1, LVNI_SELECTED);
            if (sel >= 0) ListView_DeleteItem(g_hList, sel);
            HTREEITEM tsel = TreeView_GetSelection(g_hTree);
            if (tsel) TreeView_DeleteItem(g_hTree, tsel);
            break;
        }
        case IDT_SORT:
            g_sortAsc = (g_sortCol == 0) ? !g_sortAsc : true;
            g_sortCol = 0;
            SortList();
            break;
        case IDT_VIEW:
            CycleViewMode();
            break;
        case IDT_EXPAND: {
            static bool expanded = true;
            HTREEITEM root = TreeView_GetRoot(g_hTree);
            TreeView_Expand(g_hTree, root, expanded ? TVE_EXPAND : TVE_COLLAPSE);
            expanded = !expanded;
            break;
        }
        case IDT_PROG:
            if (g_timerProg) {
                KillTimer(hwnd, 1);
                g_timerProg = 0;
                // Switch to marquee mode demo
                if (!g_marquee) {
                    g_marquee = true;
                    LONG style = GetWindowLongW(g_hProgress, GWL_STYLE);
                    SetWindowLongW(g_hProgress, GWL_STYLE, style | PBS_MARQUEE);
                    SendMessageW(g_hProgress, PBM_SETMARQUEE, TRUE, 40);
                    SetWindowTextW(g_hProgVal, L"Marquee mode");
                } else {
                    g_marquee = false;
                    LONG style = GetWindowLongW(g_hProgress, GWL_STYLE);
                    SetWindowLongW(g_hProgress, GWL_STYLE, style & ~PBS_MARQUEE);
                    SendMessageW(g_hProgress, PBM_SETMARQUEE, FALSE, 0);
                }
            } else {
                g_timerProg = SetTimer(hwnd, 1, 80, nullptr);
            }
            break;
        }
        return 0;

    case WM_NOTIFY: {
        const NMHDR* hdr = reinterpret_cast<const NMHDR*>(lp);

        // Toolbar tooltip
        if (hdr->code == TTN_GETDISPINFOW) {
            auto* tti = reinterpret_cast<NMTTDISPINFOW*>(lp);
            static const wchar_t* tips[] = {
                L"Add item (Ins)", L"Remove selected (Del)",
                L"", // sep
                L"Sort by Name (Ctrl+S)", L"Cycle view mode (Ctrl+V)",
                L"Expand / Collapse All (Ctrl+E)",
                L"", // sep
                L"Start/Stop/Marquee progress"
            };
            // Use lParam as button index
            UINT cmdId = static_cast<UINT>(tti->hdr.idFrom);
            switch (cmdId) {
            case IDT_ADD:    tti->lpszText = const_cast<wchar_t*>(tips[0]); break;
            case IDT_REMOVE: tti->lpszText = const_cast<wchar_t*>(tips[1]); break;
            case IDT_SORT:   tti->lpszText = const_cast<wchar_t*>(tips[3]); break;
            case IDT_VIEW:   tti->lpszText = const_cast<wchar_t*>(tips[4]); break;
            case IDT_EXPAND: tti->lpszText = const_cast<wchar_t*>(tips[5]); break;
            case IDT_PROG:   tti->lpszText = const_cast<wchar_t*>(tips[7]); break;
            }
            return 0;
        }

        // Tab selection
        if (hdr->idFrom == IDC_TAB && hdr->code == TCN_SELCHANGE) {
            g_currentTab = TabCtrl_GetCurSel(g_hTab);
            ApplyTab(g_currentTab);
            static const wchar_t* names[] = { L"Tree & List", L"Input Controls", L"Misc Controls" };
            Status(0, names[g_currentTab]);
            Layout(hwnd);
            return 0;
        }

        // TreeView
        if (hdr->idFrom == IDC_TREE) {
            if (hdr->code == TVN_SELCHANGEDW) {
                auto* nm = reinterpret_cast<const NMTREEVIEWW*>(lp);
                wchar_t text[128]{};
                TVITEMW tvi{ TVIF_TEXT, nm->itemNew.hItem, 0, 0, text, 128 };
                TreeView_GetItem(g_hTree, &tvi);
                Status(1, text);
            }
            if (hdr->code == NM_RCLICK) {
                POINT pt{};
                GetCursorPos(&pt);
                ShowTreeContextMenu(hwnd, pt);
                return TRUE;
            }
            if (hdr->code == TVN_BEGINLABELEDITW) {
                // Allow edit
                return FALSE;
            }
            if (hdr->code == TVN_ENDLABELEDITW) {
                auto* nm = reinterpret_cast<const NMTVDISPINFOW*>(lp);
                if (nm->item.pszText) {
                    TVITEMW tvi{ TVIF_TEXT, nm->item.hItem, 0, 0,
                        nm->item.pszText, static_cast<int>(wcslen(nm->item.pszText)) };
                    TreeView_SetItem(g_hTree, &tvi);
                }
                return TRUE;
            }
            if (hdr->code == TVN_ITEMCHANGEDW) {
                auto* nm = reinterpret_cast<const NMTVITEMCHANGE*>(lp);
                if (nm->uChanged & TVIF_STATE) {
                    UINT checkState = (nm->uStateNew & TVIS_STATEIMAGEMASK) >> 12;
                    if (checkState == 1 || checkState == 2) {
                        wchar_t buf[64]{};
                        StringCchPrintfW(buf, 64, L"Checkbox: %s",
                            checkState == 2 ? L"checked" : L"unchecked");
                        Status(1, buf);
                    }
                }
            }
        }

        // ListView
        if (hdr->idFrom == IDC_LIST) {
            if (hdr->code == LVN_COLUMNCLICK) {
                auto* nm = reinterpret_cast<const NMLISTVIEW*>(lp);
                if (nm->iSubItem == g_sortCol)
                    g_sortAsc = !g_sortAsc;
                else { g_sortCol = nm->iSubItem; g_sortAsc = true; }
                SortList();
                return 0;
            }
            if (hdr->code == LVN_ITEMCHANGED) {
                auto* nm = reinterpret_cast<const NMLISTVIEW*>(lp);
                if (nm->iItem >= 0 && (nm->uNewState & LVIS_SELECTED)) {
                    wchar_t name[128]{};
                    ListView_GetItemText(g_hList, nm->iItem, 0, name, 128);
                    Status(1, name);
                }
            }
        }

        // TrackBar
        if (hdr->idFrom == IDC_TRACKBAR && hdr->code == TRBN_THUMBPOSCHANGING) {
            auto* nm = reinterpret_cast<const NMTRBTHUMBPOSCHANGING*>(lp);
            wchar_t buf[32]{};
            StringCchPrintfW(buf, 32, L"%d", nm->dwPos);
            SetWindowTextW(g_hTrkVal, buf);
        }

        // DateTimePicker
        if (hdr->idFrom == IDC_DATETIME && hdr->code == DTN_DATETIMECHANGE) {
            auto* nm = reinterpret_cast<const NMDATETIMECHANGE*>(lp);
            wchar_t buf[64]{};
            if (nm->dwFlags == GDT_VALID) {
                StringCchPrintfW(buf, 64, L"Date: %04d-%02d-%02d",
                    nm->st.wYear, nm->st.wMonth, nm->st.wDay);
            } else {
                StringCchCopyW(buf, 64, L"Date: (none selected)");
            }
            Status(1, buf);
        }

        // IP Address
        if (hdr->idFrom == IDC_IPADDR && hdr->code == IPN_FIELDCHANGED) {
            DWORD addr{};
            SendMessageW(g_hIpAddr, IPM_GETADDRESS, 0, reinterpret_cast<LPARAM>(&addr));
            wchar_t buf[64]{};
            StringCchPrintfW(buf, 64, L"IP: %d.%d.%d.%d",
                FIRST_IPADDRESS(addr), SECOND_IPADDRESS(addr),
                THIRD_IPADDRESS(addr), FOURTH_IPADDRESS(addr));
            Status(1, buf);
        }

        // Spin
        if (hdr->idFrom == IDC_SPIN && hdr->code == UDN_DELTAPOS) {
            auto* nm = reinterpret_cast<const NMUPDOWN*>(lp);
            int newVal = nm->iPos + nm->iDelta;
            wchar_t buf[32]{};
            StringCchPrintfW(buf, 32, L"Spin: %d", newVal);
            Status(1, buf);
        }

        // TrackBar WM_HSCROLL
        return 0;
    }

    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lp) == g_hTrack) {
            int pos = static_cast<int>(SendMessageW(g_hTrack, TBM_GETPOS, 0, 0));
            wchar_t buf[16]{};
            StringCchPrintfW(buf, 16, L"%d", pos);
            SetWindowTextW(g_hTrkVal, buf);
            Status(1, buf);
        }
        return 0;

    case WM_CONTEXTMENU:
        if (reinterpret_cast<HWND>(wp) == g_hTree) {
            POINT pt{ GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
            ShowTreeContextMenu(hwnd, pt);
        }
        return 0;

    case WM_DESTROY:
        if (g_timerProg) KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc),
        ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES |
        ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_UPDOWN_CLASS |
        ICC_DATE_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
    wc.lpszClassName = L"Win32Roadmap_CommonControls";
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, L"Win32Roadmap_CommonControls",
        L"07  Common Controls — Toolbar · TreeView · ListView · ProgressBar · TrackBar · DateTimePicker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1060, 680,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) return static_cast<int>(GetLastError());

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}
