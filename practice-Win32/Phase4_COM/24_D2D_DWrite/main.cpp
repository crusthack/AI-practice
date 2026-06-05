#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <strsafe.h>
#include <cmath>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_D2D_DWrite";
constexpr UINT_PTR kTimerId    = 1;

// Device-independent resources
ID2D1Factory*       gD2D{};
IDWriteFactory*     gDWrite{};
IDWriteTextFormat*  gFontTitle{};
IDWriteTextFormat*  gFontBody{};
IDWriteTextFormat*  gFontMono{};

// Device-dependent resource (recreated on resize/device-lost)
ID2D1HwndRenderTarget* gRT{};

// Animation state
float gAngle{};   // radians, orbit
float gPulse{};   // 0-1 pulse
int   gFrame{};

void DiscardDeviceResources()
{
    if (gRT) { gRT->Release(); gRT = nullptr; }
}

HRESULT CreateDeviceResources(HWND hwnd)
{
    if (gRT)
        return S_OK;

    RECT rc{};
    GetClientRect(hwnd, &rc);

    HRESULT hr = gD2D->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right, rc.bottom)),
        &gRT);
    return hr;
}

void RenderScene()
{
    D2D1_SIZE_F sz = gRT->GetSize();
    float cx = sz.width  / 2.0f;
    float cy = sz.height / 2.0f;

    gRT->BeginDraw();

    // Background
    gRT->Clear(D2D1::ColorF(0.06f, 0.07f, 0.12f));

    // --- Grid ---
    {
        ID2D1SolidColorBrush* gridBrush{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.13f, 0.14f, 0.22f), &gridBrush);
        for (float x = 0; x < sz.width;  x += 50)
            gRT->DrawLine(D2D1::Point2F(x, 0), D2D1::Point2F(x, sz.height), gridBrush, 0.5f);
        for (float y = 0; y < sz.height; y += 50)
            gRT->DrawLine(D2D1::Point2F(0, y), D2D1::Point2F(sz.width, y), gridBrush, 0.5f);
        gridBrush->Release();
    }

    // --- Outer ring (pulsating) ---
    {
        float r = 100.0f + 18.0f * gPulse;
        ID2D1SolidColorBrush* ringBrush{};
        gRT->CreateSolidColorBrush(
            D2D1::ColorF(0.2f, 0.55f + 0.15f * gPulse, 1.0f, 0.35f + 0.25f * gPulse),
            &ringBrush);
        gRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), r, r), ringBrush, 2.0f);
        ringBrush->Release();
    }

    // --- Orbiting dot ---
    {
        float ox = cx + 130.0f * std::cos(gAngle);
        float oy = cy + 130.0f * std::sin(gAngle);
        ID2D1SolidColorBrush* dotBrush{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.95f, 0.5f), &dotBrush);
        gRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(ox, oy), 14.0f, 14.0f), dotBrush);
        dotBrush->Release();
        // Orbit trail line
        ID2D1SolidColorBrush* lineBrush{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.95f, 0.5f, 0.3f), &lineBrush);
        gRT->DrawLine(D2D1::Point2F(cx, cy), D2D1::Point2F(ox, oy), lineBrush, 1.0f);
        lineBrush->Release();
    }

    // --- Center circle (filled + stroke) ---
    {
        ID2D1SolidColorBrush* fillBrush{};
        ID2D1SolidColorBrush* strokeBrush{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.55f, 0.1f), &fillBrush);
        gRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.8f, 0.4f), &strokeBrush);
        gRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 20.0f, 20.0f), fillBrush);
        gRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 20.0f, 20.0f), strokeBrush, 1.5f);
        fillBrush->Release();
        strokeBrush->Release();
    }

    // --- Decorative rounded rectangle (top-left) ---
    {
        D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(D2D1::RectF(20, 64, 340, 140), 10, 10);
        ID2D1SolidColorBrush* bg{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.45f), &bg);
        gRT->FillRoundedRectangle(rr, bg);
        bg->Release();
        ID2D1SolidColorBrush* border{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.25f, 0.55f, 1.0f, 0.6f), &border);
        gRT->DrawRoundedRectangle(rr, border, 1.0f);
        border->Release();
    }

    // --- DWrite text ---
    {
        ID2D1SolidColorBrush* white{};
        ID2D1SolidColorBrush* cyan{};
        ID2D1SolidColorBrush* gray{};
        gRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),       &white);
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.85f, 1.0f),        &cyan);
        gRT->CreateSolidColorBrush(D2D1::ColorF(0.55f, 0.6f, 0.7f),        &gray);

        // Title
        gRT->DrawText(L"24  Direct2D  +  DirectWrite",
            27, gFontTitle,
            D2D1::RectF(28, 16, sz.width - 20, 60),
            white);

        // Body
        gRT->DrawText(
            L"ID2D1HwndRenderTarget\nIDWriteFactory\nIDWriteTextFormat",
            54, gFontBody,
            D2D1::RectF(30, 70, 340, 138),
            cyan);

        // Frame counter
        wchar_t info[128]{};
        StringCchPrintfW(info, ARRAYSIZE(info),
            L"frame=%d  angle=%.2f rad  pulse=%.2f",
            gFrame, gAngle, gPulse);
        gRT->DrawText(info, static_cast<UINT32>(wcslen(info)),
            gFontMono,
            D2D1::RectF(12, sz.height - 30, sz.width - 12, sz.height - 4),
            gray);

        white->Release();
        cyan->Release();
        gray->Release();
    }

    HRESULT hr = gRT->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
        DiscardDeviceResources();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateDeviceResources(hwnd);
        SetTimer(hwnd, kTimerId, 16, nullptr); // ~60 fps
        return 0;

    case WM_TIMER:
        if (wParam == kTimerId)
        {
            constexpr float kTwoPi = 6.28318530718f;
            gAngle += 0.035f;
            if (gAngle > kTwoPi)
                gAngle -= kTwoPi;
            gPulse = (std::sin(gAngle * 2.3f) + 1.0f) * 0.5f;
            ++gFrame;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd, &ps);
        if (SUCCEEDED(CreateDeviceResources(hwnd)))
            RenderScene();
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE:
        if (gRT)
        {
            RECT rc{};
            GetClientRect(hwnd, &rc);
            gRT->Resize(D2D1::SizeU(rc.right, rc.bottom));
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, kTimerId);
        DiscardDeviceResources();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    // Device-independent setup
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gD2D);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&gDWrite));

    if (gDWrite)
    {
        gDWrite->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_BOLD,   DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            28.0f, L"en-us", &gFontTitle);
        gDWrite->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            14.0f, L"en-us", &gFontBody);
        gDWrite->CreateTextFormat(L"Consolas", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            12.0f, L"en-us", &gFontMono);
    }

    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kClassName, L"24  Direct2D + DirectWrite",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd)
        return static_cast<int>(GetLastError());

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup
    if (gFontMono)  { gFontMono->Release();  }
    if (gFontBody)  { gFontBody->Release();  }
    if (gFontTitle) { gFontTitle->Release(); }
    if (gDWrite)    { gDWrite->Release();    }
    if (gD2D)       { gD2D->Release();       }

    return static_cast<int>(msg.wParam);
}
