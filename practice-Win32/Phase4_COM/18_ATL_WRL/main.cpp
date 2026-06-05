#include <Windows.h>
#include <ShObjIdl.h>
#include <atlbase.h>
#include <wrl/client.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"18_ATL_WRL\r\n";
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    CComPtr<IFileOpenDialog> atlDialog;
    HRESULT hr = atlDialog.CoCreateInstance(CLSID_FileOpenDialog);
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"ATL CComPtr CoCreateInstance: OK\r\n" : L"ATL CComPtr: FAILED\r\n");

    Microsoft::WRL::ComPtr<IFileOpenDialog> wrlDialog;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wrlDialog));
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"WRL ComPtr CoCreateInstance: OK\r\nRAII COM pointer wrappers demonstrated." : L"WRL ComPtr: FAILED");

    CoUninitialize();
    MessageBoxW(nullptr, msg, L"ATL WRL", MB_OK);
    return 0;
}
