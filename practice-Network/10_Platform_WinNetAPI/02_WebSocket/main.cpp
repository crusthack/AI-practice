#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#pragma comment(lib, "winhttp.lib")
int main() {
    HINTERNET h=WinHttpOpen(L"WebSocket lab/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET c=WinHttpConnect(h, L"echo.websocket.events", INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET r=WinHttpOpenRequest(c, L"GET", L"/", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    WinHttpSetOption(r, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0);
    if (WinHttpSendRequest(r, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) && WinHttpReceiveResponse(r, nullptr)) {
        HINTERNET ws=WinHttpWebSocketCompleteUpgrade(r, 0); r=nullptr;
        const char* msg="hello websocket"; WinHttpWebSocketSend(ws, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, (void*)msg, (DWORD)strlen(msg));
        char buf[256]{}; DWORD read=0; WINHTTP_WEB_SOCKET_BUFFER_TYPE type{};
        if (WinHttpWebSocketReceive(ws, buf, sizeof(buf)-1, &read, &type)==NO_ERROR) std::printf("received: %s\n", buf);
        WinHttpWebSocketClose(ws, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0); WinHttpCloseHandle(ws);
    }
    if(r)WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h); return 0;
}
