#include <Windows.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_GDIPainting";

void PaintScene(HDC dc, const RECT& rc)
{
    HBRUSH background = CreateSolidBrush(RGB(248, 250, 252));
    FillRect(dc, &rc, background);
    DeleteObject(background);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(220, 225, 232));
    HGDIOBJ oldPen = SelectObject(dc, gridPen);
    for (int x = 0; x < rc.right; x += 40)
        MoveToEx(dc, x, 0, nullptr), LineTo(dc, x, rc.bottom);
    for (int y = 0; y < rc.bottom; y += 40)
        MoveToEx(dc, 0, y, nullptr), LineTo(dc, rc.right, y);
    SelectObject(dc, oldPen);
    DeleteObject(gridPen);

    HBRUSH blue = CreateSolidBrush(RGB(37, 99, 235));
    HBRUSH green = CreateSolidBrush(RGB(22, 163, 74));
    HPEN thick = CreatePen(PS_SOLID, 3, RGB(15, 23, 42));

    oldPen = SelectObject(dc, thick);
    HGDIOBJ oldBrush = SelectObject(dc, blue);
    RoundRect(dc, 60, 80, 260, 190, 16, 16);
    SelectObject(dc, green);
    Ellipse(dc, 320, 80, 520, 190);

    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(blue);
    DeleteObject(green);
    DeleteObject(thick);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(15, 23, 42));
    TextOutW(dc, 60, 32, L"03 GDI Painting - double buffered WM_PAINT", 43);
    TextOutW(dc, 60, 225, L"CreateCompatibleDC + compatible bitmap prevent flicker.", 55);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC screen = BeginPaint(hwnd, &ps);
        RECT client{};
        GetClientRect(hwnd, &client);

        HDC memory = CreateCompatibleDC(screen);
        HBITMAP bitmap = CreateCompatibleBitmap(screen, client.right, client.bottom);
        HGDIOBJ oldBitmap = SelectObject(memory, bitmap);

        PaintScene(memory, client);
        BitBlt(screen, 0, 0, client.right, client.bottom, memory, 0, 0, SRCCOPY);

        SelectObject(memory, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memory);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kClassName, L"03 GDI Painting", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 760, 480, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return static_cast<int>(GetLastError());

    ShowWindow(hwnd, showCommand);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
