#include <Windows.h>
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HANDLE job = CreateJobObjectW(nullptr, L"Local\\Win32RoadmapJobDemo");
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    limits.ProcessMemoryLimit = 64ull * 1024ull * 1024ull;
    limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    BOOL set = SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));

    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    wchar_t command[] = L"cmd.exe /c echo JobObject child process && exit 7";
    BOOL created = CreateProcessW(nullptr, command, nullptr, nullptr, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    BOOL assigned = FALSE;
    DWORD exitCode = 0;
    if (created)
    {
        assigned = AssignProcessToJobObject(job, pi.hProcess);
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting{};
    BOOL queriedAccounting = job ? QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &accounting, sizeof(accounting), nullptr) : FALSE;

    wchar_t msg[512]{};
    StringCchPrintfW(msg, ARRAYSIZE(msg),
        L"38_JobObject\r\n"
        L"CreateJobObject: %s\r\n"
        L"Set limit: kill-on-close + 64 MiB process memory: %s\r\n"
        L"CreateProcess suspended: %s\r\n"
        L"AssignProcessToJobObject: %s\r\n"
        L"Child exit code: %lu\r\n"
        L"Query accounting: %s\r\n"
        L"Total processes seen by job: %lu\r\n"
        L"Active processes now: %lu",
        job ? L"OK" : L"FAILED",
        set ? L"OK" : L"FAILED",
        created ? L"OK" : L"FAILED",
        assigned ? L"OK" : L"FAILED",
        exitCode,
        queriedAccounting ? L"OK" : L"FAILED",
        accounting.TotalProcesses,
        accounting.ActiveProcesses);
    if (job) CloseHandle(job);
    MessageBoxW(nullptr, msg, L"Job Object", MB_OK);
    return 0;
}
