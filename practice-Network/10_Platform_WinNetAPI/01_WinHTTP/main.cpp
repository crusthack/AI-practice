#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#pragma comment(lib, "winhttp.lib")
int main() {
    HINTERNET h = WinHttpOpen(L"WinHTTP lab/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET c = WinHttpConnect(h, L"example.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET r = WinHttpOpenRequest(c, L"GET", L"/", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (WinHttpSendRequest(r, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(r, nullptr)) {
        DWORD status=0, size=sizeof(status); WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
        std::printf("HTTP status: %lu\n", status);
        char buf[1024]; DWORD read=0; while(WinHttpReadData(r, buf, sizeof(buf)-1, &read) && read){ buf[read]=0; std::printf("%s", buf); }
    }
    WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h); return 0;
}
