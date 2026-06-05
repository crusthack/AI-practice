// 08_OwnerDraw — Custom Drawing: Owner-Draw, NM_CUSTOMDRAW, WM_CTLCOLOR, Subclassing
// 4 modes:
//   1) WM_DRAWITEM / WM_MEASUREITEM  — ListBox (variable heights), ComboBox, Button
//   2) NM_CUSTOMDRAW on ListView     — alternating rows, hot-track, bold header
//   3) WM_CTLCOLOR*                  — Edit, Static, ListBox custom colors
//   4) SetWindowSubclass             — custom-painted Button + Edit with placeholder

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <strsafe.h>
#include <uxtheme.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

// ── IDs ──────────────────────────────────────────────────────────────────────
enum {
    IDC_BTN_M1 = 101, IDC_BTN_M2 = 102, IDC_BTN_M3 = 103, IDC_BTN_M4 = 104,
    IDC_STATUS  = 105,

    // Mode 1
    IDC_OD_LIST = 201, IDC_OD_COMBO = 202, IDC_OD_BTN = 203,
    IDC_LBL_LIST = 204, IDC_LBL_COMBO = 205,

    // Mode 2
    IDC_CD_LIST = 301,

    // Mode 3
    IDC_CC_EDIT1 = 401, IDC_CC_EDIT2 = 402, IDC_CC_STATIC = 403,
    IDC_CC_LISTBOX = 404, IDC_CC_TOGGLE = 405,
    IDC_LBL_EDIT1 = 406, IDC_LBL_EDIT2 = 407, IDC_LBL_LISTBOX = 408,

    // Mode 4
    IDC_SC_BTN = 501, IDC_SC_EDIT = 502, IDC_LBL_SC = 503,
};

// ── Globals ───────────────────────────────────────────────────────────────────
HWND      g_hwnd{}, g_hStatus{};
HINSTANCE g_hInst{};
int       g_mode    = 1;
bool      g_ctlColor = false; // toggle for mode 3

// Mode 1
HWND g_hOdList{}, g_hOdCombo{}, g_hOdBtn{}, g_hLblList{}, g_hLblCombo{};
// Mode 2
HWND g_hCdList{};
// Mode 3
HWND g_hCcEdit1{}, g_hCcEdit2{}, g_hCcStatic{}, g_hCcListBox{}, g_hCcToggle{};
HWND g_hLblEdit1{}, g_hLblEdit2{}, g_hLblListbox{};
// Mode 4
HWND g_hScBtn{}, g_hScEdit{}, g_hLblSc{};

// Colors for custom draw
static const COLORREF kRowColors[] = {
    RGB(255,255,255), RGB(240,247,255) // alternating rows
};
static const COLORREF kPalette[] = {
    RGB(37,99,235), RGB(22,163,74), RGB(217,119,6),
    RGB(220,38,38), RGB(124,58,237), RGB(14,165,233),
    RGB(236,72,153), RGB(132,204,22)
};

// Brushes cached for WM_CTLCOLOR
HBRUSH g_brEditNormal{}, g_brEditReadOnly{}, g_brStatic{}, g_brListBox{};

// Subclass data
BOOL   g_scBtnHover   = FALSE;
BOOL   g_scBtnPressed = FALSE;
BOOL   g_scEditFocus  = FALSE;
HFONT  g_boldFont{}, g_normalFont{};

// ── Helpers ───────────────────────────────────────────────────────────────────
void Status(const wchar_t* msg, int part = 0)
{
    SendMessageW(g_hStatus, SB_SETTEXT, part, reinterpret_cast<LPARAM>(msg));
}

void ShowMode(int mode)
{
    // Hide all mode controls
    auto hide = [](HWND h) { if (h) ShowWindow(h, SW_HIDE); };
    HWND m1[] = {g_hOdList,g_hOdCombo,g_hOdBtn,g_hLblList,g_hLblCombo};
    HWND m2[] = {g_hCdList};
    HWND m3[] = {g_hCcEdit1,g_hCcEdit2,g_hCcStatic,g_hCcListBox,g_hCcToggle,
                 g_hLblEdit1,g_hLblEdit2,g_hLblListbox};
    HWND m4[] = {g_hScBtn,g_hScEdit,g_hLblSc};
    for (HWND h : m1) hide(h);
    for (HWND h : m2) hide(h);
    for (HWND h : m3) hide(h);
    for (HWND h : m4) hide(h);

    // Show selected
    auto show = [](HWND h) { if (h) ShowWindow(h, SW_SHOW); };
    switch (mode) {
    case 1: for (HWND h : m1) show(h); break;
    case 2: for (HWND h : m2) show(h); break;
    case 3: for (HWND h : m3) show(h); break;
    case 4: for (HWND h : m4) show(h); break;
    }
    g_mode = mode;

    static const wchar_t* statuses[] = {
        L"", L"WM_DRAWITEM + WM_MEASUREITEM  —  owner-draw ListBox, ComboBox, Button",
        L"NM_CUSTOMDRAW on ListView  —  row colors, hot track, column formatting",
        L"WM_CTLCOLOR*  —  custom background/text colors for Edit, Static, ListBox",
        L"SetWindowSubclass  —  custom-painted Button (hover/press) + Edit (placeholder)"
    };
    Status(statuses[mode]);
}

// ── Layout ────────────────────────────────────────────────────────────────────
void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    SendMessageW(g_hStatus, WM_SIZE, 0, 0);
    RECT sr{};
    GetWindowRect(g_hStatus, &sr);
    int sbH = sr.bottom - sr.top;

    const int toolH = 44;
    const int pad   = 12;
    int cx  = pad;
    int cy  = toolH + pad;
    int cw  = rc.right - pad * 2;
    int ch  = rc.bottom - sbH - cy - pad;

    // Mode buttons (top bar)
    int btnW = (rc.right - pad * 5) / 4;
    MoveWindow(GetDlgItem(hwnd, IDC_BTN_M1), pad + (btnW+pad)*0, 8, btnW, 30, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_BTN_M2), pad + (btnW+pad)*1, 8, btnW, 30, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_BTN_M3), pad + (btnW+pad)*2, 8, btnW, 30, TRUE);
    MoveWindow(GetDlgItem(hwnd, IDC_BTN_M4), pad + (btnW+pad)*3, 8, btnW, 30, TRUE);

    // Mode 1
    int halfW = (cw - pad) / 2;
    MoveWindow(g_hLblList,  cx,            cy,      120, 22, TRUE);
    MoveWindow(g_hOdList,   cx,            cy+24,   halfW, ch-24, TRUE);
    MoveWindow(g_hLblCombo, cx+halfW+pad,  cy,      120, 22, TRUE);
    MoveWindow(g_hOdCombo,  cx+halfW+pad,  cy+24,   halfW, 180, TRUE);
    MoveWindow(g_hOdBtn,    cx+halfW+pad,  cy+210,  halfW, 48, TRUE);

    // Mode 2
    MoveWindow(g_hCdList, cx, cy, cw, ch, TRUE);

    // Mode 3
    int colW = (cw - pad * 2) / 3;
    MoveWindow(g_hLblEdit1,  cx,              cy,     colW, 22, TRUE);
    MoveWindow(g_hCcEdit1,   cx,              cy+24,  colW, 28, TRUE);
    MoveWindow(g_hLblEdit2,  cx,              cy+60,  colW, 22, TRUE);
    MoveWindow(g_hCcEdit2,   cx,              cy+82,  colW, 60, TRUE);
    MoveWindow(g_hCcStatic,  cx,              cy+150, colW, 50, TRUE);
    MoveWindow(g_hLblListbox,cx+colW+pad,     cy,     colW, 22, TRUE);
    MoveWindow(g_hCcListBox, cx+colW+pad,     cy+24,  colW, ch-80, TRUE);
    MoveWindow(g_hCcToggle,  cx+colW+pad,     cy+ch-50, colW, 32, TRUE);

    // Mode 4
    MoveWindow(g_hLblSc,  cx,      cy,        cw, 22, TRUE);
    MoveWindow(g_hScBtn,  cx,      cy+30,     240, 56, TRUE);
    MoveWindow(g_hScEdit, cx+260,  cy+30,     cw-260, 56, TRUE);
}

// ── Owner-Draw: DrawItem ──────────────────────────────────────────────────────

// Item data for variable-height listbox
struct OdItem { int type; const wchar_t* text; }; // type: 0=normal, 1=header, 2=icon
static const OdItem kOdItems[] = {
    {1, L"─── View Types ───────"},
    {0, L"Report (columns)"},
    {0, L"Large Icon"},
    {0, L"Small Icon"},
    {0, L"List"},
    {1, L"─── Styles ───────────"},
    {0, L"Full Row Select"},
    {0, L"Grid Lines"},
    {0, L"Check Boxes"},
    {0, L"Hot Track"},
    {1, L"─── Colors ───────────"},
    {2, L"Blue"},
    {2, L"Green"},
    {2, L"Amber"},
    {2, L"Red"},
    {2, L"Purple"},
};

void DrawOdListItem(const DRAWITEMSTRUCT* dis)
{
    if (dis->itemID == static_cast<UINT>(-1)) return;
    const OdItem& item = kOdItems[dis->itemID];

    bool sel = (dis->itemState & ODS_SELECTED) != 0;
    COLORREF bg, fg;

    if (item.type == 1) {
        // Section header: dark background, light text
        bg = RGB(30, 41, 59); fg = RGB(148, 163, 184);
    } else if (sel) {
        bg = RGB(37, 99, 235); fg = RGB(255, 255, 255);
    } else {
        bg = (dis->itemID % 2 == 0) ? RGB(255,255,255) : RGB(248,250,252);
        fg = RGB(15, 23, 42);
    }

    HBRUSH br = CreateSolidBrush(bg);
    FillRect(dis->hDC, &dis->rcItem, br);
    DeleteObject(br);

    if (item.type == 2) {
        // Color swatch on left
        int colorIdx = dis->itemID - 11; // offset to color items
        if (colorIdx >= 0 && colorIdx < 8) {
            RECT swatch = dis->rcItem;
            swatch.left += 8; swatch.right = swatch.left + 16;
            swatch.top += 6; swatch.bottom -= 6;
            HBRUSH sb = CreateSolidBrush(kPalette[colorIdx]);
            FillRect(dis->hDC, &swatch, sb);
            DeleteObject(sb);
            FrameRect(dis->hDC, &swatch, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        }
    }

    // Left accent bar for headers
    if (item.type == 1) {
        RECT accent = { dis->rcItem.left, dis->rcItem.top,
                        dis->rcItem.left + 3, dis->rcItem.bottom };
        HBRUSH ab = CreateSolidBrush(RGB(56,189,248));
        FillRect(dis->hDC, &accent, ab);
        DeleteObject(ab);
    }

    // Text
    RECT textRc = dis->rcItem;
    textRc.left += (item.type == 2) ? 32 : 10;
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, fg);
    HFONT hFont = (item.type == 1) ? g_boldFont : g_normalFont;
    HGDIOBJ oldFont = SelectObject(dis->hDC, hFont);
    DrawTextW(dis->hDC, item.text, -1, &textRc,
        DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(dis->hDC, oldFont);

    if (dis->itemState & ODS_FOCUS)
        DrawFocusRect(dis->hDC, &dis->rcItem);
}

void DrawOdComboItem(const DRAWITEMSTRUCT* dis)
{
    if (dis->itemID == static_cast<UINT>(-1)) return;
    bool sel = (dis->itemState & ODS_SELECTED) != 0;

    HBRUSH bg = CreateSolidBrush(sel ? RGB(226, 232, 240) : RGB(255,255,255));
    FillRect(dis->hDC, &dis->rcItem, bg);
    DeleteObject(bg);

    COLORREF accent = kPalette[dis->itemID % ARRAYSIZE(kPalette)];
    RECT swatch = dis->rcItem;
    swatch.left += 6; swatch.right = swatch.left + 20;
    swatch.top += 4; swatch.bottom -= 4;
    HBRUSH sb = CreateSolidBrush(accent);
    FillRect(dis->hDC, &dis->rcItem, sb);
    DeleteObject(sb);

    // Draw pill shape border
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255,255,255));
    HGDIOBJ oldPen = SelectObject(dis->hDC, pen);
    HGDIOBJ oldBr  = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
    RoundRect(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+2,
        dis->rcItem.right-2, dis->rcItem.bottom-2, 6, 6);
    SelectObject(dis->hDC, oldPen); SelectObject(dis->hDC, oldBr);
    DeleteObject(pen);

    wchar_t text[128]{};
    SendMessageW(dis->hwndItem, CB_GETLBTEXT, dis->itemID,
        reinterpret_cast<LPARAM>(text));
    RECT textRc = dis->rcItem;
    textRc.left += 10;
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, RGB(255,255,255));
    SelectObject(dis->hDC, g_boldFont);
    DrawTextW(dis->hDC, text, -1, &textRc,
        DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    if (dis->itemState & ODS_FOCUS)
        DrawFocusRect(dis->hDC, &dis->rcItem);
}

void DrawOdButton(const DRAWITEMSTRUCT* dis)
{
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool focused = (dis->itemState & ODS_FOCUS) != 0;

    RECT rc = dis->rcItem;

    // Gradient-like fill: two solid fills top/bottom
    COLORREF top    = pressed ? RGB(29,78,216)  : RGB(59,130,246);
    COLORREF bottom = pressed ? RGB(37,99,235)  : RGB(37,99,235);

    int mid = (rc.top + rc.bottom) / 2;
    RECT topRc    = { rc.left, rc.top, rc.right, mid };
    RECT bottomRc = { rc.left, mid, rc.right, rc.bottom };
    HBRUSH brTop    = CreateSolidBrush(top);
    HBRUSH brBottom = CreateSolidBrush(bottom);
    FillRect(dis->hDC, &topRc,    brTop);
    FillRect(dis->hDC, &bottomRc, brBottom);
    DeleteObject(brTop); DeleteObject(brBottom);

    // Rounded border
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(29, 78, 216));
    HGDIOBJ oldPen = SelectObject(dis->hDC, pen);
    HGDIOBJ oldBr  = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
    RoundRect(dis->hDC, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
    SelectObject(dis->hDC, oldPen); SelectObject(dis->hDC, oldBr);
    DeleteObject(pen);

    // Shadow at bottom (if not pressed)
    if (!pressed) {
        RECT shadow = { rc.left+2, rc.bottom-4, rc.right-2, rc.bottom };
        HBRUSH brShadow = CreateSolidBrush(RGB(29,78,200));
        FillRect(dis->hDC, &shadow, brShadow);
        DeleteObject(brShadow);
    }

    // Text
    wchar_t text[128]{};
    GetWindowTextW(dis->hwndItem, text, 128);
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, RGB(255,255,255));
    SelectObject(dis->hDC, g_boldFont);
    RECT textRc = rc;
    if (pressed) OffsetRect(&textRc, 1, 1);
    DrawTextW(dis->hDC, text, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (focused) DrawFocusRect(dis->hDC, &rc);
}

// ── NM_CUSTOMDRAW ListView ────────────────────────────────────────────────────
LRESULT HandleCustomDraw(LPARAM lp)
{
    auto* nm = reinterpret_cast<NMLVCUSTOMDRAW*>(lp);
    switch (nm->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
        return CDRF_NOTIFYSUBITEMDRAW | CDRF_NEWFONT;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
        int row = static_cast<int>(nm->nmcd.dwItemSpec);
        int col = nm->iSubItem;
        bool hot = (nm->nmcd.uItemState & CDIS_HOT) != 0;
        bool sel = (nm->nmcd.uItemState & CDIS_SELECTED) != 0;

        // Background
        if (sel) {
            nm->clrTextBk = RGB(37, 99, 235);
            nm->clrText   = RGB(255, 255, 255);
        } else if (hot) {
            nm->clrTextBk = RGB(219, 234, 254);
            nm->clrText   = RGB(30, 58, 138);
        } else {
            nm->clrTextBk = (row % 2 == 0) ? RGB(255,255,255) : RGB(241,245,249);
            nm->clrText   = RGB(15, 23, 42);
        }

        // Column 0: bold
        if (col == 0) {
            SelectObject(nm->nmcd.hdc, g_boldFont);
        }
        // Column 2: color-code the "Status" column
        if (col == 2 && !sel) {
            wchar_t text[64]{};
            ListView_GetItemText(g_hCdList, row, 2, text, 64);
            if (wcsstr(text, L"High") || wcsstr(text, L"Critical"))
                nm->clrText = RGB(220, 38, 38);
            else if (wcsstr(text, L"Medium") || wcsstr(text, L"Warning"))
                nm->clrText = RGB(217, 119, 6);
            else if (wcsstr(text, L"Low") || wcsstr(text, L"OK"))
                nm->clrText = RGB(22, 163, 74);
        }
        return CDRF_NEWFONT;
    }
    }
    return CDRF_DODEFAULT;
}

// ── Subclass Procs ────────────────────────────────────────────────────────────
LRESULT CALLBACK SubclassBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                  UINT_PTR, DWORD_PTR)
{
    switch (msg) {
    case WM_MOUSEMOVE:
        if (!g_scBtnHover) {
            g_scBtnHover = TRUE;
            TRACKMOUSEEVENT tme{ sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    case WM_MOUSELEAVE:
        g_scBtnHover   = FALSE;
        g_scBtnPressed = FALSE;
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_LBUTTONDOWN:
        g_scBtnPressed = TRUE;
        SetCapture(hwnd);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_LBUTTONUP:
        if (g_scBtnPressed) {
            g_scBtnPressed = FALSE;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            // Simulate click
            SendMessageW(GetParent(hwnd), WM_COMMAND,
                MAKEWPARAM(IDC_SC_BTN, BN_CLICKED),
                reinterpret_cast<LPARAM>(hwnd));
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc{}; GetClientRect(hwnd, &rc);

        // Background
        COLORREF bg = g_scBtnPressed ? RGB(30,58,138)
                    : g_scBtnHover   ? RGB(96,165,250)
                    :                  RGB(59,130,246);
        HBRUSH br = CreateSolidBrush(bg);
        HPEN pen  = CreatePen(PS_SOLID, 2, RGB(29,78,216));
        HGDIOBJ oldBr  = SelectObject(hdc, br);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 12, 12);
        SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
        DeleteObject(br); DeleteObject(pen);

        // Hover glow border
        if (g_scBtnHover && !g_scBtnPressed) {
            HPEN glow = CreatePen(PS_SOLID, 3, RGB(147,197,253));
            HGDIOBJ op = SelectObject(hdc, glow);
            HGDIOBJ ob = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            RoundRect(hdc, rc.left+1, rc.top+1, rc.right-1, rc.bottom-1, 11, 11);
            SelectObject(hdc, op); SelectObject(hdc, ob);
            DeleteObject(glow);
        }

        // Text
        wchar_t text[128]{};
        GetWindowTextW(hwnd, text, 128);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255,255,255));
        SelectObject(hdc, g_boldFont);
        if (g_scBtnPressed) OffsetRect(&rc, 1, 1);
        DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK SubclassEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                   UINT_PTR, DWORD_PTR)
{
    switch (msg) {
    case WM_SETFOCUS:
        g_scEditFocus = TRUE;
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_KILLFOCUS:
        g_scEditFocus = FALSE;
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_PAINT: {
        LRESULT r = DefSubclassProc(hwnd, msg, wp, lp);
        // Draw left accent bar after default paint
        HDC hdc = GetDC(hwnd);
        RECT rc{}; GetClientRect(hwnd, &rc);
        COLORREF accent = g_scEditFocus ? RGB(59,130,246) : RGB(203,213,225);
        HBRUSH br = CreateSolidBrush(accent);
        RECT bar = { rc.left, rc.top, rc.left + 4, rc.bottom };
        FillRect(hdc, &bar, br);
        DeleteObject(br);
        // Draw placeholder if empty
        wchar_t txt[2]{};
        GetWindowTextW(hwnd, txt, 2);
        if (txt[0] == L'\0') {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(148,163,184));
            RECT textRc = { rc.left + 10, rc.top, rc.right - 4, rc.bottom };
            SelectObject(hdc, g_normalFont);
            DrawTextW(hdc, L"Type something here…", -1, &textRc,
                DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        }
        ReleaseDC(hwnd, hdc);
        return r;
    }
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

// ── Fill controls with data ───────────────────────────────────────────────────
void FillOdControls()
{
    for (const auto& item : kOdItems)
        SendMessageW(g_hOdList, LB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(item.text));

    static const wchar_t* comboItems[] = {
        L"Sapphire Blue", L"Emerald Green", L"Amber Gold",
        L"Crimson Red", L"Amethyst Purple", L"Sky Blue",
        L"Rose Pink", L"Lime Green"
    };
    for (const wchar_t* s : comboItems)
        SendMessageW(g_hOdCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
    SendMessageW(g_hOdCombo, CB_SETCURSEL, 0, 0);
}

void FillCdList()
{
    // Add columns
    auto addCol = [](int idx, int w, const wchar_t* t) {
        LVCOLUMNW c{ LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM };
        c.iSubItem=idx; c.cx=w; c.pszText=const_cast<wchar_t*>(t);
        ListView_InsertColumn(g_hCdList, idx, &c);
    };
    addCol(0, 200, L"Component");
    addCol(1, 120, L"Value");
    addCol(2, 110, L"Status");
    addCol(3, 130, L"Category");

    struct Row { const wchar_t* a,*b,*c,*d; };
    static const Row rows[] = {
        {L"CPU Usage",         L"87%",      L"High",     L"Performance"},
        {L"Memory",            L"6.2 GB",   L"Medium",   L"Resources"},
        {L"Disk Read",         L"120 MB/s", L"OK",       L"Storage"},
        {L"Disk Write",        L"45 MB/s",  L"OK",       L"Storage"},
        {L"Network In",        L"1.2 Gbps", L"OK",       L"Network"},
        {L"Network Out",       L"850 Mbps", L"Warning",  L"Network"},
        {L"Process Count",     L"342",      L"OK",       L"System"},
        {L"Handle Count",      L"18,422",   L"Warning",  L"System"},
        {L"GDI Objects",       L"2,100",    L"Medium",   L"GDI"},
        {L"Uptime",            L"3d 14h",   L"OK",       L"System"},
        {L"Page Faults/sec",   L"1,240",    L"Warning",  L"Memory"},
        {L"Committed Memory",  L"12.4 GB",  L"High",     L"Memory"},
        {L"Cache Hit Rate",    L"94.2%",    L"OK",       L"Performance"},
        {L"Context Switches",  L"34,200/s", L"Medium",   L"CPU"},
        {L"Kernel Time",       L"8.3%",     L"OK",       L"CPU"},
        {L"Interrupt/sec",     L"22,000",   L"Medium",   L"CPU"},
        {L"Disk Queue",        L"0.0",      L"OK",       L"Storage"},
        {L"Active Connections",L"48",       L"OK",       L"Network"},
        {L"ETW Sessions",      L"4",        L"OK",       L"Tracing"},
        {L"Critical Events",   L"2",        L"Critical", L"Diagnostics"},
    };
    for (int i = 0; i < ARRAYSIZE(rows); ++i) {
        LVITEMW lvi{ LVIF_TEXT };
        lvi.iItem = i;
        lvi.pszText = const_cast<wchar_t*>(rows[i].a);
        ListView_InsertItem(g_hCdList, &lvi);
        ListView_SetItemText(g_hCdList, i, 1, const_cast<wchar_t*>(rows[i].b));
        ListView_SetItemText(g_hCdList, i, 2, const_cast<wchar_t*>(rows[i].c));
        ListView_SetItemText(g_hCdList, i, 3, const_cast<wchar_t*>(rows[i].d));
    }
}

void FillCcControls()
{
    static const wchar_t* lbItems[] = {
        L"Alice",   L"Bob",   L"Charlie", L"Dave",   L"Eve",
        L"Frank",   L"Grace", L"Heidi",   L"Ivan",   L"Judy",
        L"Karl",    L"Laura", L"Mallory", L"Niaj",   L"Oscar",
    };
    for (const wchar_t* s : lbItems)
        SendMessageW(g_hCcListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));

    SetWindowTextW(g_hCcEdit1, L"Normal Edit — colored background");
    SetWindowTextW(g_hCcEdit2,
        L"Read-only multi-line edit\r\nwith custom background color.\r\nLines 3...");
    SetWindowTextW(g_hCcStatic,
        L"WM_CTLCOLORSTATIC: custom text + background");
}

// ── Create All Controls ───────────────────────────────────────────────────────
void CreateAll(HWND hwnd)
{
    // Fonts
    g_boldFont   = CreateFontW(14,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
    g_normalFont = CreateFontW(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");

    // Brushes for WM_CTLCOLOR
    g_brEditNormal  = CreateSolidBrush(RGB(254,252,232)); // yellow tint
    g_brEditReadOnly= CreateSolidBrush(RGB(240,253,244)); // green tint
    g_brStatic      = CreateSolidBrush(RGB(239,246,255)); // blue tint
    g_brListBox     = CreateSolidBrush(RGB(250,245,255)); // purple tint

    // Mode buttons
    for (int i = 1; i <= 4; ++i) {
        static const wchar_t* labels[] = {
            L"1 · WM_DRAWITEM", L"2 · NM_CUSTOMDRAW",
            L"3 · WM_CTLCOLOR", L"4 · SetWindowSubclass"
        };
        CreateWindowW(L"BUTTON", labels[i-1],
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            0,0,0,0, hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_M1+i-1)),
            g_hInst, nullptr);
    }

    // Status bar
    g_hStatus = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_STATUS), g_hInst, nullptr);

    // ── Mode 1: Owner-Draw ListBox + ComboBox + Button ──
    g_hLblList = CreateWindowW(L"STATIC", L"Variable-height Owner-Draw ListBox:",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_LIST), g_hInst, nullptr);
    g_hOdList = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD|WS_BORDER|WS_VSCROLL|
        LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_NOTIFY,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_OD_LIST), g_hInst, nullptr);
    SendMessageW(g_hOdList, WM_SETFONT, reinterpret_cast<WPARAM>(g_normalFont), TRUE);

    g_hLblCombo = CreateWindowW(L"STATIC", L"Owner-Draw ComboBox (filled pill style):",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_COMBO), g_hInst, nullptr);
    g_hOdCombo = CreateWindowW(L"COMBOBOX", nullptr,
        WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_OD_COMBO), g_hInst, nullptr);

    g_hOdBtn = CreateWindowW(L"BUTTON", L"Owner-Draw Button  —  click me",
        WS_CHILD|BS_OWNERDRAW,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_OD_BTN), g_hInst, nullptr);

    FillOdControls();

    // ── Mode 2: NM_CUSTOMDRAW ListView ──
    g_hCdList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_CD_LIST), g_hInst, nullptr);
    ListView_SetExtendedListViewStyle(g_hCdList,
        LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP);
    FillCdList();

    // ── Mode 3: WM_CTLCOLOR* ──
    g_hLblEdit1  = CreateWindowW(L"STATIC", L"Single-line Edit (yellow BG):",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_EDIT1), g_hInst, nullptr);
    g_hCcEdit1   = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD|ES_AUTOHSCROLL,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_CC_EDIT1), g_hInst, nullptr);

    g_hLblEdit2  = CreateWindowW(L"STATIC", L"Multi-line ReadOnly Edit (green BG):",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_EDIT2), g_hInst, nullptr);
    g_hCcEdit2   = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_CC_EDIT2), g_hInst, nullptr);

    g_hCcStatic  = CreateWindowW(L"STATIC", nullptr,
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_CC_STATIC), g_hInst, nullptr);

    g_hLblListbox= CreateWindowW(L"STATIC", L"ListBox (purple BG, custom selection):",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_LISTBOX), g_hInst, nullptr);
    g_hCcListBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD|WS_VSCROLL|LBS_NOTIFY,
        0,0,0,0, hwnd, reinterpret_cast<HMENU>(IDC_CC_LISTBOX), g_hInst, nullptr);

    g_hCcToggle  = CreateWindowW(L"BUTTON", L"Toggle Custom Colors ON/OFF",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_CC_TOGGLE), g_hInst, nullptr);

    FillCcControls();

    // ── Mode 4: SetWindowSubclass ──
    g_hLblSc = CreateWindowW(L"STATIC",
        L"SetWindowSubclass — custom Button (hover/press painting) + Edit (left-accent + placeholder):",
        WS_CHILD|SS_LEFT, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_LBL_SC), g_hInst, nullptr);
    g_hScBtn = CreateWindowW(L"BUTTON", L"Subclassed  Button",
        WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_SC_BTN), g_hInst, nullptr);
    g_hScEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD|ES_AUTOHSCROLL, 0,0,0,0, hwnd,
        reinterpret_cast<HMENU>(IDC_SC_EDIT), g_hInst, nullptr);

    SetWindowSubclass(g_hScBtn,  SubclassBtnProc,  IDC_SC_BTN,  0);
    SetWindowSubclass(g_hScEdit, SubclassEditProc, IDC_SC_EDIT, 0);

    // Apply fonts
    SendMessageW(g_hCcEdit1,  WM_SETFONT, reinterpret_cast<WPARAM>(g_normalFont), TRUE);
    SendMessageW(g_hCcEdit2,  WM_SETFONT, reinterpret_cast<WPARAM>(g_normalFont), TRUE);
    SendMessageW(g_hCcStatic, WM_SETFONT, reinterpret_cast<WPARAM>(g_boldFont),   TRUE);
    SendMessageW(g_hScEdit,   WM_SETFONT, reinterpret_cast<WPARAM>(g_normalFont), TRUE);
    SendMessageW(g_hScBtn,    WM_SETFONT, reinterpret_cast<WPARAM>(g_boldFont),   TRUE);

    ShowMode(1); // Start on mode 1
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

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_M1: ShowMode(1); Layout(hwnd); break;
        case IDC_BTN_M2: ShowMode(2); Layout(hwnd); break;
        case IDC_BTN_M3: ShowMode(3); Layout(hwnd); break;
        case IDC_BTN_M4: ShowMode(4); Layout(hwnd); break;
        case IDC_OD_BTN:
            MessageBoxW(hwnd, L"Owner-drawn button clicked!", L"08 OwnerDraw", MB_OK | MB_ICONINFORMATION);
            break;
        case IDC_SC_BTN:
            MessageBoxW(hwnd, L"Subclassed button clicked!\nHover, press, and release effects are painted manually.",
                L"08 OwnerDraw", MB_OK | MB_ICONINFORMATION);
            break;
        case IDC_CC_TOGGLE:
            g_ctlColor = !g_ctlColor;
            // Redraw all controls in mode 3
            InvalidateRect(g_hCcEdit1,  nullptr, TRUE); UpdateWindow(g_hCcEdit1);
            InvalidateRect(g_hCcEdit2,  nullptr, TRUE); UpdateWindow(g_hCcEdit2);
            InvalidateRect(g_hCcStatic, nullptr, TRUE); UpdateWindow(g_hCcStatic);
            InvalidateRect(g_hCcListBox,nullptr, TRUE); UpdateWindow(g_hCcListBox);
            SetWindowTextW(g_hCcToggle,
                g_ctlColor ? L"Disable Custom Colors" : L"Enable Custom Colors");
            Status(g_ctlColor ? L"WM_CTLCOLOR* active — custom brushes returned" :
                                L"WM_CTLCOLOR* disabled — using default system colors");
            break;
        case IDC_OD_LIST:
            if (HIWORD(wp) == LBN_SELCHANGE) {
                int sel = static_cast<int>(SendMessageW(g_hOdList, LB_GETCURSEL, 0, 0));
                if (sel >= 0 && sel < ARRAYSIZE(kOdItems)) {
                    wchar_t buf[64]{};
                    StringCchPrintfW(buf, 64, L"ListBox: selected item %d — %s",
                        sel, kOdItems[sel].text);
                    Status(buf);
                }
            }
            break;
        case IDC_OD_COMBO:
            if (HIWORD(wp) == CBN_SELCHANGE) {
                int sel = static_cast<int>(SendMessageW(g_hOdCombo, CB_GETCURSEL, 0, 0));
                wchar_t buf[64]{}; wchar_t item[64]{};
                SendMessageW(g_hOdCombo, CB_GETLBTEXT, sel,
                    reinterpret_cast<LPARAM>(item));
                StringCchPrintfW(buf, 64, L"ComboBox: %s", item);
                Status(buf);
            }
            break;
        }
        return 0;

    case WM_MEASUREITEM: {
        auto* mis = reinterpret_cast<MEASUREITEMSTRUCT*>(lp);
        if (mis->CtlID == IDC_OD_LIST) {
            if (mis->itemID < ARRAYSIZE(kOdItems)) {
                switch (kOdItems[mis->itemID].type) {
                case 1: mis->itemHeight = 28; break; // section header
                case 2: mis->itemHeight = 36; break; // icon item
                default: mis->itemHeight = 26; break; // normal
                }
            }
        }
        if (mis->CtlID == IDC_OD_COMBO)
            mis->itemHeight = 34;
        return TRUE;
    }

    case WM_DRAWITEM: {
        auto* dis = reinterpret_cast<DRAWITEMSTRUCT*>(lp);
        if (dis->CtlID == IDC_OD_LIST)  { DrawOdListItem(dis);  return TRUE; }
        if (dis->CtlID == IDC_OD_COMBO) { DrawOdComboItem(dis); return TRUE; }
        if (dis->CtlID == IDC_OD_BTN)   { DrawOdButton(dis);    return TRUE; }
        return FALSE;
    }

    case WM_NOTIFY: {
        auto* hdr = reinterpret_cast<NMHDR*>(lp);
        if (hdr->idFrom == IDC_CD_LIST && hdr->code == NM_CUSTOMDRAW)
            return HandleCustomDraw(lp);
        return 0;
    }

    // WM_CTLCOLOR* messages — only active when g_ctlColor is true
    case WM_CTLCOLOREDIT: {
        HWND ctrl = reinterpret_cast<HWND>(lp);
        if (!g_ctlColor) break;
        if (ctrl == g_hCcEdit1) {
            SetTextColor(reinterpret_cast<HDC>(wp), RGB(120, 53, 15));
            SetBkColor(reinterpret_cast<HDC>(wp), RGB(254,252,232));
            return reinterpret_cast<LRESULT>(g_brEditNormal);
        }
        if (ctrl == g_hCcEdit2) {
            SetTextColor(reinterpret_cast<HDC>(wp), RGB(21, 128, 61));
            SetBkColor(reinterpret_cast<HDC>(wp), RGB(240,253,244));
            return reinterpret_cast<LRESULT>(g_brEditReadOnly);
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HWND ctrl = reinterpret_cast<HWND>(lp);
        if (!g_ctlColor) break;
        if (ctrl == g_hCcStatic) {
            SetTextColor(reinterpret_cast<HDC>(wp), RGB(29, 78, 216));
            SetBkColor(reinterpret_cast<HDC>(wp), RGB(239,246,255));
            return reinterpret_cast<LRESULT>(g_brStatic);
        }
        break;
    }
    case WM_CTLCOLORLISTBOX: {
        HWND ctrl = reinterpret_cast<HWND>(lp);
        if (!g_ctlColor) break;
        if (ctrl == g_hCcListBox) {
            SetTextColor(reinterpret_cast<HDC>(wp), RGB(88, 28, 135));
            SetBkColor(reinterpret_cast<HDC>(wp), RGB(250,245,255));
            return reinterpret_cast<LRESULT>(g_brListBox);
        }
        break;
    }

    case WM_DESTROY:
        if (g_boldFont)        DeleteObject(g_boldFont);
        if (g_normalFont)      DeleteObject(g_normalFont);
        if (g_brEditNormal)    DeleteObject(g_brEditNormal);
        if (g_brEditReadOnly)  DeleteObject(g_brEditReadOnly);
        if (g_brStatic)        DeleteObject(g_brStatic);
        if (g_brListBox)       DeleteObject(g_brListBox);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
    wc.lpszClassName = L"Win32Roadmap_OwnerDraw";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, L"Win32Roadmap_OwnerDraw",
        L"08  Owner Draw — WM_DRAWITEM · NM_CUSTOMDRAW · WM_CTLCOLOR · SetWindowSubclass",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1020, 680,
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
