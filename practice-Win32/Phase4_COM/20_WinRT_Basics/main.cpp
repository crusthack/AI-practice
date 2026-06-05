#include <Windows.h>
#include <roapi.h>
#include <winstring.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[512] = L"20_WinRT_Basics\r\n";
    HRESULT hr = RoInitialize(RO_INIT_MULTITHREADED);
    StringCchCatW(msg, ARRAYSIZE(msg), SUCCEEDED(hr) ? L"RoInitialize: OK\r\n" : L"RoInitialize: FAILED\r\n");

    HSTRING text{};
    hr = WindowsCreateString(L"WinRT HSTRING sample", 21, &text);
    UINT32 length{};
    const wchar_t* raw = WindowsGetStringRawBuffer(text, &length);
    wchar_t line[256]{};
    StringCchPrintfW(line, ARRAYSIZE(line), L"WindowsCreateString: %s\r\nLength: %u", raw, length);
    StringCchCatW(msg, ARRAYSIZE(msg), line);
    WindowsDeleteString(text);
    RoUninitialize();
    MessageBoxW(nullptr, msg, L"WinRT Basics", MB_OK);
    return 0;
}
