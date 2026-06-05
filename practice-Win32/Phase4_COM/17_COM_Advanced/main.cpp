#include <Windows.h>
#include <ShObjIdl.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"17_COM_Advanced\r\n";
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    IFileOpenDialog* dialog{};
    IStream* stream{};
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(hr))
    {
        hr = CoMarshalInterThreadInterfaceInStream(IID_IFileOpenDialog, dialog, &stream);
        StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"CoMarshalInterThreadInterfaceInStream: OK\r\n" : L"Marshal: FAILED\r\n");
        IFileOpenDialog* unmarshaled{};
        if (stream)
        {
            hr = CoGetInterfaceAndReleaseStream(stream, IID_PPV_ARGS(&unmarshaled));
            StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"CoGetInterfaceAndReleaseStream: OK\r\nApartment marshaling concept demonstrated." : L"Unmarshal: FAILED");
            if (unmarshaled) unmarshaled->Release();
        }
        dialog->Release();
    }
    CoUninitialize();
    MessageBoxW(nullptr, msg, L"COM Advanced", MB_OK);
    return 0;
}
