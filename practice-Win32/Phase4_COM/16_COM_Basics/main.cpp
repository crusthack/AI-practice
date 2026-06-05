#include <Windows.h>
#include <ShObjIdl.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"16_COM_Basics\r\n";
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"CoInitializeEx: OK\r\n" : L"CoInitializeEx: FAILED\r\n");

    IFileOpenDialog* dialog{};
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(hr))
    {
        IShellItem* item{};
        hr = dialog->QueryInterface(IID_PPV_ARGS(&item));
        StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"QueryInterface: OK\r\nAddRef/Release through COM interfaces demonstrated." : L"QueryInterface: FAILED");
        if (item) item->Release();
        dialog->Release();
    }
    else
    {
        StringCchCatW(msg, ARRAYSIZE(msg), L"CoCreateInstance(CLSID_FileOpenDialog): FAILED");
    }

    if (SUCCEEDED(hr)) {}
    CoUninitialize();
    MessageBoxW(nullptr, msg, L"COM Basics", MB_OK);
    return 0;
}
