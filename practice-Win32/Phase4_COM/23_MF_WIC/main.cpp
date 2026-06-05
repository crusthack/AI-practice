#include <Windows.h>
#include <mfapi.h>
#include <wincodec.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"23_MF_WIC\r\n";
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT hr = MFStartup(MF_VERSION);
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"MFStartup: OK\r\n" : L"MFStartup: FAILED\r\n");

    IWICImagingFactory* factory{};
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"WIC ImagingFactory: OK\r\nMedia Foundation + WIC initialized." : L"WIC ImagingFactory: FAILED");
    if (factory) factory->Release();
    MFShutdown();
    CoUninitialize();
    MessageBoxW(nullptr, msg, L"MF WIC", MB_OK);
    return 0;
}
