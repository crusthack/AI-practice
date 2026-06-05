#include <Windows.h>
#include <iostream>

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR,
    int)
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    HANDLE readPipe;
    HANDLE writePipe;

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    CreatePipe(&readPipe, &writePipe, &sa, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;

    PROCESS_INFORMATION pi = {};

    wchar_t cmd[] = L"cmd /c echo hello from child";

    CreateProcessW(
        NULL,
        cmd,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi);

    CloseHandle(writePipe);

    char buffer[128];
    DWORD read;

    while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &read, NULL))
    {
        if (read == 0)
            break;

        buffer[read] = 0;

        printf("%s", buffer);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(readPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    system("pause");

    return 0;
}