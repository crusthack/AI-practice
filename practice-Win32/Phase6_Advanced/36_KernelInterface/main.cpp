#include <Windows.h>
#include <CommCtrl.h>
#include <winioctl.h>
#include <strsafe.h>
#pragma comment(lib, "comctl32.lib")

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_KernelInterface";
constexpr int IDC_VOLUME = 1001;
constexpr int IDC_NTFS   = 1002;
constexpr int IDC_DISK   = 1003;
constexpr int IDC_TEXT   = 1004;
constexpr int IDC_STATUS = 1005;

HWND gText{};
HWND gStatus{};

HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

void Append(wchar_t* buf, size_t count, const wchar_t* fmt, ...)
{
    wchar_t line[512]{};
    va_list args;
    va_start(args, fmt);
    StringCchVPrintfW(line, ARRAYSIZE(line), fmt, args);
    va_end(args);
    StringCchCatW(buf, count, line);
    StringCchCatW(buf, count, L"\r\n");
}

void VolumeInfoDemo()
{
    wchar_t out[4096] = L"=== 36_KernelInterface - Volume & Disk Info ===\r\n";

    // --- GetVolumeInformation ---
    wchar_t volName[MAX_PATH]{};
    wchar_t fsName[MAX_PATH]{};
    DWORD serialNum{}, maxComp{}, flags{};

    Append(out, ARRAYSIZE(out), L"[GetVolumeInformation] C:\\");
    BOOL ok = GetVolumeInformationW(L"C:\\", volName, ARRAYSIZE(volName),
        &serialNum, &maxComp, &flags, fsName, ARRAYSIZE(fsName));
    if (ok)
    {
        Append(out, ARRAYSIZE(out), L"  Volume name: \"%s\"",   volName);
        Append(out, ARRAYSIZE(out), L"  File system: %s",       fsName);
        Append(out, ARRAYSIZE(out), L"  Serial:      %08lX",    serialNum);
        Append(out, ARRAYSIZE(out), L"  MaxComp:     %lu",      maxComp);
        Append(out, ARRAYSIZE(out), L"  Flags:       0x%08lX", flags);
        if (flags & FILE_SUPPORTS_REPARSE_POINTS)
            Append(out, ARRAYSIZE(out), L"    - FILE_SUPPORTS_REPARSE_POINTS");
        if (flags & FILE_SUPPORTS_SPARSE_FILES)
            Append(out, ARRAYSIZE(out), L"    - FILE_SUPPORTS_SPARSE_FILES");
        if (flags & FILE_NAMED_STREAMS)
            Append(out, ARRAYSIZE(out), L"    - FILE_NAMED_STREAMS");
    }

    // --- GetDiskFreeSpaceEx ---
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[GetDiskFreeSpaceEx] C:\\");
    ULARGE_INTEGER freeBytesAvail{}, totalBytes{}, totalFree{};
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvail, &totalBytes, &totalFree))
    {
        Append(out, ARRAYSIZE(out), L"  Total:      %.1f GB",
            static_cast<double>(totalBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0));
        Append(out, ARRAYSIZE(out), L"  Free:       %.1f GB",
            static_cast<double>(totalFree.QuadPart) / (1024.0 * 1024.0 * 1024.0));
        Append(out, ARRAYSIZE(out), L"  Available:  %.1f GB",
            static_cast<double>(freeBytesAvail.QuadPart) / (1024.0 * 1024.0 * 1024.0));
    }

    // --- EnumVolumes ---
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"[FindFirstVolume] Enumerating volumes:");
    wchar_t volGuid[MAX_PATH]{};
    HANDLE hVol = FindFirstVolumeW(volGuid, ARRAYSIZE(volGuid));
    int volCount = 0;
    if (hVol != INVALID_HANDLE_VALUE)
    {
        do
        {
            wchar_t mountPts[MAX_PATH]{};
            DWORD needed{};
            GetVolumePathNamesForVolumeNameW(volGuid, mountPts, ARRAYSIZE(mountPts), &needed);
            Append(out, ARRAYSIZE(out), L"  [%d] %s  mount: %s",
                ++volCount, volGuid, mountPts[0] ? mountPts : L"(no mount point)");
        }
        while (FindNextVolumeW(hVol, volGuid, ARRAYSIZE(volGuid)) && volCount < 8);
        FindVolumeClose(hVol);
    }

    SetWindowTextW(gText, out);
    SendMessageW(gStatus, SB_SETTEXT, 0,
        reinterpret_cast<LPARAM>(L"Volume info retrieved."));
}

void NtfsStreamsDemo()
{
    wchar_t out[4096] = L"=== 36_KernelInterface - NTFS Streams & Compression ===\r\n";

    wchar_t tempDir[MAX_PATH]{};
    wchar_t filePath[MAX_PATH]{};
    GetTempPathW(ARRAYSIZE(tempDir), tempDir);
    StringCchPrintfW(filePath, ARRAYSIZE(filePath),
        L"%sWin32Roadmap_NtfsDemo.txt", tempDir);

    // Create file with default stream
    HANDLE h = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (h != INVALID_HANDLE_VALUE)
    {
        const char mainData[] = "Main stream data\r\n";
        DWORD written{};
        WriteFile(h, mainData, sizeof(mainData) - 1, &written, nullptr);
        Append(out, ARRAYSIZE(out), L"[CreateFile] %s", filePath);
        Append(out, ARRAYSIZE(out), L"  Default stream: %lu bytes written", written);

        // FSCTL_GET_COMPRESSION
        USHORT compression{};
        DWORD bytes{};
        BOOL cmpOk = DeviceIoControl(h, FSCTL_GET_COMPRESSION,
            nullptr, 0, &compression, sizeof(compression), &bytes, nullptr);
        Append(out, ARRAYSIZE(out), L"");
        Append(out, ARRAYSIZE(out), L"[DeviceIoControl] FSCTL_GET_COMPRESSION: %s",
            cmpOk ? L"OK" : L"FAILED");
        if (cmpOk)
        {
            const wchar_t* cmpStr =
                compression == COMPRESSION_FORMAT_NONE    ? L"NONE" :
                compression == COMPRESSION_FORMAT_DEFAULT ? L"DEFAULT" :
                compression == COMPRESSION_FORMAT_LZNT1  ? L"LZNT1" : L"OTHER";
            Append(out, ARRAYSIZE(out), L"  Compression: %s (value=%hu)", cmpStr, compression);
        }

        // FSCTL_IS_VOLUME_DIRTY
        DWORD dirtyFlags{};
        BOOL dirtyOk = DeviceIoControl(h, FSCTL_IS_VOLUME_DIRTY,
            nullptr, 0, &dirtyFlags, sizeof(dirtyFlags), &bytes, nullptr);
        Append(out, ARRAYSIZE(out), L"[DeviceIoControl] FSCTL_IS_VOLUME_DIRTY: %s",
            dirtyOk ? (dirtyFlags & VOLUME_IS_DIRTY ? L"DIRTY" : L"CLEAN") : L"FAILED/NA");

        CloseHandle(h);
    }

    // Alternate Data Stream
    wchar_t adsPath[MAX_PATH]{};
    StringCchPrintfW(adsPath, ARRAYSIZE(adsPath),
        L"%s:Win32Roadmap_ADS", filePath);
    HANDLE hAds = CreateFileW(adsPath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hAds != INVALID_HANDLE_VALUE)
    {
        const char adsData[] = "NTFS Alternate Data Stream content";
        DWORD adsWritten{};
        WriteFile(hAds, adsData, sizeof(adsData) - 1, &adsWritten, nullptr);
        CloseHandle(hAds);
        Append(out, ARRAYSIZE(out), L"");
        Append(out, ARRAYSIZE(out), L"[ADS] %s", adsPath);
        Append(out, ARRAYSIZE(out), L"  Written %lu bytes to alternate data stream", adsWritten);
        Append(out, ARRAYSIZE(out), L"  (visible via: dir /r  or  streams.exe)");
    }

    // Get file info
    h = CreateFileW(filePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE)
    {
        BY_HANDLE_FILE_INFORMATION info{};
        if (GetFileInformationByHandle(h, &info))
        {
            Append(out, ARRAYSIZE(out), L"");
            Append(out, ARRAYSIZE(out), L"[GetFileInformationByHandle]");
            Append(out, ARRAYSIZE(out), L"  nNumberOfLinks: %lu", info.nNumberOfLinks);
            Append(out, ARRAYSIZE(out), L"  dwVolumeSerialNumber: %08lX", info.dwVolumeSerialNumber);
            ULARGE_INTEGER fileId{};
            fileId.HighPart = info.nFileIndexHigh;
            fileId.LowPart  = info.nFileIndexLow;
            Append(out, ARRAYSIZE(out), L"  FileIndex: %llu", fileId.QuadPart);
        }
        CloseHandle(h);
    }

    DeleteFileW(filePath);
    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs: DeviceIoControl (FSCTL_GET_COMPRESSION, FSCTL_IS_VOLUME_DIRTY)");
    Append(out, ARRAYSIZE(out), L"      CreateFile (Alternate Data Stream), GetFileInformationByHandle");

    SetWindowTextW(gText, out);
    SendMessageW(gStatus, SB_SETTEXT, 0,
        reinterpret_cast<LPARAM>(L"NTFS demo complete."));
}

void DiskGeometryDemo()
{
    wchar_t out[4096] = L"=== 36_KernelInterface - Disk Geometry ===\r\n";

    // Open physical disk 0
    HANDLE disk = CreateFileW(L"\\\\.\\PhysicalDrive0",
        0, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);

    if (disk == INVALID_HANDLE_VALUE)
    {
        Append(out, ARRAYSIZE(out), L"[PhysicalDrive0] Open failed (error=%lu)", GetLastError());
        Append(out, ARRAYSIZE(out), L"  (May need admin rights for PhysicalDrive0)");
    }
    else
    {
        DISK_GEOMETRY_EX geo{};
        DWORD bytes{};
        BOOL ok = DeviceIoControl(disk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
            nullptr, 0, &geo, sizeof(geo), &bytes, nullptr);
        Append(out, ARRAYSIZE(out), L"[IOCTL_DISK_GET_DRIVE_GEOMETRY_EX] %s",
            ok ? L"OK" : L"FAILED");
        if (ok)
        {
            Append(out, ARRAYSIZE(out), L"  DiskSize:         %.1f GB",
                static_cast<double>(geo.DiskSize.QuadPart) / (1024.0 * 1024.0 * 1024.0));
            Append(out, ARRAYSIZE(out), L"  BytesPerSector:   %lu", geo.Geometry.BytesPerSector);
            Append(out, ARRAYSIZE(out), L"  SectorsPerTrack: %lu", geo.Geometry.SectorsPerTrack);
            Append(out, ARRAYSIZE(out), L"  TracksPerCylinder: %lu", geo.Geometry.TracksPerCylinder);
            Append(out, ARRAYSIZE(out), L"  Cylinders:        %lld", geo.Geometry.Cylinders.QuadPart);
        }
        CloseHandle(disk);
    }

    // Also query the C: volume
    Append(out, ARRAYSIZE(out), L"");
    HANDLE vol = CreateFileW(L"\\\\.\\C:",
        0, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (vol != INVALID_HANDLE_VALUE)
    {
        DISK_GEOMETRY geo{};
        DWORD bytes{};
        BOOL ok = DeviceIoControl(vol, IOCTL_DISK_GET_DRIVE_GEOMETRY,
            nullptr, 0, &geo, sizeof(geo), &bytes, nullptr);
        Append(out, ARRAYSIZE(out), L"[IOCTL_DISK_GET_DRIVE_GEOMETRY] on \\\\.\\C: %s",
            ok ? L"OK" : L"FAILED (may need admin)");
        if (ok)
            Append(out, ARRAYSIZE(out), L"  BytesPerSector: %lu", geo.BytesPerSector);
        CloseHandle(vol);
    }

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs: CreateFile(\"\\\\\\\\.\\\\PhysicalDriveN\")");
    Append(out, ARRAYSIZE(out), L"      DeviceIoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY_EX)");
    Append(out, ARRAYSIZE(out), L"      DISK_GEOMETRY_EX, DISK_GEOMETRY");

    SetWindowTextW(gText, out);
    SendMessageW(gStatus, SB_SETTEXT, 0,
        reinterpret_cast<LPARAM>(L"Disk geometry demo complete."));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_BAR_CLASSES };
        InitCommonControlsEx(&icc);

        CreateWindowW(L"BUTTON", L"Volume Info",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            12, 12, 130, 32, hwnd, ControlId(IDC_VOLUME), inst, nullptr);
        CreateWindowW(L"BUTTON", L"NTFS / ADS",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            154, 12, 130, 32, hwnd, ControlId(IDC_NTFS), inst, nullptr);
        CreateWindowW(L"BUTTON", L"Disk Geometry",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            296, 12, 130, 32, hwnd, ControlId(IDC_DISK), inst, nullptr);

        gText = CreateWindowW(L"EDIT", L"Click Volume Info, NTFS / ADS, or Disk Geometry.",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            12, 56, 760, 400,
            hwnd, ControlId(IDC_TEXT), inst, nullptr);
        gStatus = CreateWindowExW(0, STATUSCLASSNAMEW, L"Ready",
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd, ControlId(IDC_STATUS), inst, nullptr);
        return 0;
    }

    case WM_SIZE:
    {
        SendMessageW(gStatus, WM_SIZE, 0, 0);
        RECT rc{};
        GetClientRect(hwnd, &rc);
        RECT sRc{};
        GetWindowRect(gStatus, &sRc);
        int sH = sRc.bottom - sRc.top;
        MoveWindow(gText, 12, 56, rc.right - 24, rc.bottom - sH - 68, TRUE);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_VOLUME) VolumeInfoDemo();
        if (LOWORD(wParam) == IDC_NTFS)   NtfsStreamsDemo();
        if (LOWORD(wParam) == IDC_DISK)   DiskGeometryDemo();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kClassName,
        L"36 KernelInterface - Volume / NTFS / Disk",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 560,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd)
        return static_cast<int>(GetLastError());

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
