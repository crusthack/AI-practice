#include <Windows.h>
#include <strsafe.h>

using FilterConnectCommunicationPortFn = HRESULT (WINAPI*)(LPCWSTR, DWORD, LPCVOID, WORD, LPSECURITY_ATTRIBUTES, HANDLE*);

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HMODULE fltLib = LoadLibraryW(L"FltLib.dll");
    auto connect = reinterpret_cast<FilterConnectCommunicationPortFn>(
        fltLib ? GetProcAddress(fltLib, "FilterConnectCommunicationPort") : nullptr);

    HANDLE port{};
    HRESULT hr = connect ? connect(L"\\Win32RoadmapMissingPort", 0, nullptr, 0, nullptr, &port) : HRESULT_FROM_WIN32(GetLastError());
    wchar_t msg[512]{};
    StringCchPrintfW(msg, ARRAYSIZE(msg),
        L"39_Minifilter\r\nFltLib.dll: %s\r\nFilterConnectCommunicationPort export: %s\r\nCall result against a missing demo port: 0x%08lX\r\n\r\nA real minifilter requires WDK, driver signing, and a VM.",
        fltLib ? L"loaded" : L"not loaded", connect ? L"found" : L"not found", hr);
    if (port) CloseHandle(port);
    if (fltLib) FreeLibrary(fltLib);
    MessageBoxW(nullptr, msg, L"Minifilter", MB_OK);
    return 0;
}
