#include <Windows.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t msg[1024] = L"34_DLL_Injection\r\nSafe self-process API flow demo.\r\n\r\n";
    HANDLE self = GetCurrentProcess();
    const wchar_t dllName[] = L"user32.dll";
    void* remoteBuffer = VirtualAllocEx(self, nullptr, sizeof(dllName), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    SIZE_T written{};
    BOOL wrote = WriteProcessMemory(self, remoteBuffer, dllName, sizeof(dllName), &written);
    HMODULE user32 = LoadLibraryW(dllName);
    FARPROC messageBox = GetProcAddress(user32, "MessageBoxW");

    wchar_t line[512]{};
    StringCchPrintfW(line, ARRAYSIZE(line),
        L"VirtualAllocEx(current process): %p\r\nWriteProcessMemory: %s (%zu bytes)\r\nLoadLibraryW: %p\r\nGetProcAddress(MessageBoxW): %p\r\n\r\nThis intentionally does not inject into another process.",
        remoteBuffer, wrote ? L"OK" : L"FAILED", written, user32, messageBox);
    StringCchCatW(msg, ARRAYSIZE(msg), line);
    if (remoteBuffer) VirtualFreeEx(self, remoteBuffer, 0, MEM_RELEASE);
    MessageBoxW(nullptr, msg, L"DLL Injection Concepts", MB_OK);
    return 0;
}
