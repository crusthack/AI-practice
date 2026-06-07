#include <Windows.h>
#include <strsafe.h>

namespace
{
constexpr wchar_t kClassName[] = L"Win32Roadmap_FileIO";
constexpr int IDC_RUN   = 1001;
constexpr int IDC_TEXT  = 1002;

HWND gText{};

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

void RunDemo()
{
    wchar_t out[4096] = L"=== 10_FileIO ===\r\n";

    wchar_t tempDir[MAX_PATH]{};
    wchar_t srcPath[MAX_PATH]{};
    wchar_t dstPath[MAX_PATH]{};
    wchar_t asyncPath[MAX_PATH]{};
    GetTempPathW(ARRAYSIZE(tempDir), tempDir);
    StringCchPrintfW(srcPath,   ARRAYSIZE(srcPath),   L"%sWin32_FileIO_src.txt",   tempDir);
    StringCchPrintfW(dstPath,   ARRAYSIZE(dstPath),   L"%sWin32_FileIO_dst.txt",   tempDir);
    StringCchPrintfW(asyncPath, ARRAYSIZE(asyncPath), L"%sWin32_FileIO_async.txt", tempDir);

    // --- CreateFile / WriteFile ---
    {
        HANDLE h = CreateFileW(srcPath, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        const char content[] = "Win32 FileIO demo\r\nSecond line of data.\r\nThird line.\r\n";
        DWORD written{};
        WriteFile(h, content, static_cast<DWORD>(sizeof(content) - 1), &written, nullptr);
        FlushFileBuffers(h);
        CloseHandle(h);
        Append(out, ARRAYSIZE(out), L"[WriteFile] %lu bytes written", written);
    }

    // --- GetFileAttributesEx / FileTimeToSystemTime ---
    {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (GetFileAttributesExW(srcPath, GetFileExInfoStandard, &attrs))
        {
            SYSTEMTIME st{};
            FileTimeToSystemTime(&attrs.ftLastWriteTime, &st);
            Append(out, ARRAYSIZE(out),
                L"[Attrs] size=%lu  LastWrite=%04d-%02d-%02d %02d:%02d:%02d",
                attrs.nFileSizeLow,
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
    }

    // --- CopyFile ---
    CopyFileW(srcPath, dstPath, FALSE);
    Append(out, ARRAYSIZE(out), L"[CopyFile] -> %s", dstPath);

    // --- ReadFile + SetFilePointerEx ---
    {
        HANDLE h = CreateFileW(dstPath, GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        char buf[128]{};
        DWORD readCount{};
        ReadFile(h, buf, sizeof(buf) - 1, &readCount, nullptr);
        buf[readCount] = '\0';

        LARGE_INTEGER fileSize{};
        GetFileSizeEx(h, &fileSize);

        // Seek back to start
        LARGE_INTEGER zero{};
        SetFilePointerEx(h, zero, nullptr, FILE_BEGIN);

        char buf2[16]{};
        DWORD readCount2{};
        ReadFile(h, buf2, 10, &readCount2, nullptr);
        buf2[readCount2] = '\0';
        CloseHandle(h);

        wchar_t wide[128]{};
        MultiByteToWideChar(CP_UTF8, 0, buf, -1, wide, ARRAYSIZE(wide));
        Append(out, ARRAYSIZE(out), L"[ReadFile] %lu bytes, filesize=%lld", readCount, fileSize.QuadPart);
        Append(out, ARRAYSIZE(out), L"  Content: %s", wide);

        wchar_t wide2[16]{};
        MultiByteToWideChar(CP_UTF8, 0, buf2, -1, wide2, ARRAYSIZE(wide2));
        Append(out, ARRAYSIZE(out), L"  Seek-to-0 re-read (10 bytes): \"%s\"", wide2);
    }

    // --- OVERLAPPED (async) write ---
    {
        HANDLE h = CreateFileW(asyncPath, GENERIC_WRITE | GENERIC_READ, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
        if (h != INVALID_HANDLE_VALUE)
        {
            HANDLE evt = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            OVERLAPPED ov{};
            ov.hEvent = evt;
            const char asyncData[] = "OVERLAPPED async write payload";
            DWORD asyncWritten{};
            BOOL ok = WriteFile(h, asyncData, sizeof(asyncData) - 1, &asyncWritten, &ov);
            if (!ok && GetLastError() == ERROR_IO_PENDING)
            {
                WaitForSingleObject(evt, INFINITE);
                GetOverlappedResult(h, &ov, &asyncWritten, FALSE);
                ok = TRUE;
            }
            Append(out, ARRAYSIZE(out), L"[OVERLAPPED] WriteFile: %s (%lu bytes)",
                ok ? L"OK" : L"FAILED", asyncWritten);
            CloseHandle(evt);
            CloseHandle(h);
            DeleteFileW(asyncPath);
        }
    }

    // --- FindFirstFile directory listing ---
    {
        wchar_t searchPath[MAX_PATH]{};
        StringCchPrintfW(searchPath, ARRAYSIZE(searchPath), L"%s*.txt", tempDir);
        WIN32_FIND_DATAW fd{};
        HANDLE hFind = FindFirstFileW(searchPath, &fd);
        int count = 0;
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do { ++count; } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }
        Append(out, ARRAYSIZE(out), L"[FindFirstFile] *.txt in temp dir: %d file(s)", count);
    }

    DeleteFileW(srcPath);
    DeleteFileW(dstPath);

    Append(out, ARRAYSIZE(out), L"");
    Append(out, ARRAYSIZE(out), L"APIs covered:");
    Append(out, ARRAYSIZE(out), L"  CreateFile, WriteFile, ReadFile, FlushFileBuffers, CloseHandle");
    Append(out, ARRAYSIZE(out), L"  CopyFile, GetFileSizeEx, SetFilePointerEx");
    Append(out, ARRAYSIZE(out), L"  GetFileAttributesEx, FileTimeToSystemTime");
    Append(out, ARRAYSIZE(out), L"  WriteFile(OVERLAPPED), WaitForSingleObject, GetOverlappedResult");
    Append(out, ARRAYSIZE(out), L"  FindFirstFile, FindNextFile, FindClose, DeleteFile");

    SetWindowTextW(gText, out);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HINSTANCE inst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
        CreateWindowW(L"BUTTON", L"Run File I/O Demo",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, 20, 200, 32,
            hwnd, ControlId(IDC_RUN), inst, nullptr);
        gText = CreateWindowW(L"EDIT", L"Click  Run File I/O Demo.",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            20, 68, 720, 380,
            hwnd, ControlId(IDC_TEXT), inst, nullptr);
        return 0;
    }

    case WM_SIZE:
    {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        MoveWindow(gText, 20, 68, rc.right - 40, rc.bottom - 84, TRUE);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RUN)
            RunDemo();
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

    HWND hwnd = CreateWindowExW(0, kClassName, L"10 FileIO",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 540,
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
