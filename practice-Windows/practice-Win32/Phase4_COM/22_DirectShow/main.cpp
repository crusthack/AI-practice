#include <Windows.h>
#include <dshow.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"22_DirectShow\r\n";
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IGraphBuilder* graph{};
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&graph));
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"CLSID_FilterGraph created.\r\n" : L"FilterGraph creation failed.\r\n");
    if (graph)
    {
        IMediaControl* control{};
        hr = graph->QueryInterface(IID_PPV_ARGS(&control));
        StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"IMediaControl QueryInterface: OK" : L"IMediaControl QueryInterface: FAILED");
        if (control) control->Release();
        graph->Release();
    }
    CoUninitialize();
    MessageBoxW(nullptr, msg, L"DirectShow", MB_OK);
    return 0;
}
