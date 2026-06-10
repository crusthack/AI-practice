#pragma once
#include "net_headers.hpp"
#include <string>
#include <windows.h>

//                                                                            
// winsock_util.hpp      Lab             RAII +     
//                                                                            

//     RAII: WSAStartup / WSACleanup                                        
//    main()         .           WSACleanup      .
struct WsaInit {
    WsaInit() {
        WSADATA wsa{};
        int r = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (r != 0) { printf("[WsaInit] WSAStartup   : %d\n", r); exit(1); }
    }
    ~WsaInit() { WSACleanup(); }
    WsaInit(const WsaInit&) = delete;
};

//         n                                                            
// TCP            recv()      n               .
//       n                     .
inline bool RecvExact(SOCKET s, void* buf, int n) {
    int total = 0;
    while (total < n) {
        int r = recv(s, (char*)buf + total, n - total, 0);
        if (r <= 0) return false;  //       or   
        total += r;
    }
    return true;
}

//         n                                                            
inline bool SendAll(SOCKET s, const void* buf, int n) {
    int total = 0;
    while (total < n) {
        int r = send(s, (const char*)buf + total, n - total, 0);
        if (r == SOCKET_ERROR) return false;
        total += r;
    }
    return true;
}

//              (RTT,        )                                     
struct HiResTimer {
    LARGE_INTEGER freq, start;
    HiResTimer() { QueryPerformanceFrequency(&freq); reset(); }
    void   reset()       { QueryPerformanceCounter(&start); }
    double elapsedMs()   {
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        return (double)(now.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    }
    double elapsedUs()   { return elapsedMs() * 1000.0; }
};

//     WSAGetLastError                                              
inline std::string errStr(int code = -1) {
    if (code == -1) code = WSAGetLastError();
    char buf[256]{};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, code, 0, buf, sizeof(buf), nullptr);
    //      
    for (char* p = buf; *p; ++p) if (*p == '\r' || *p == '\n') *p = ' ';
    return std::string("[") + std::to_string(code) + "] " + buf;
}

//                                                                     
inline std::string addrStr(const sockaddr_in& a) {
    char ip[16];
    inet_ntop(AF_INET, &a.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(a.sin_port));
}

//              (        )                                         
inline SOCKET makeTcpSocket() {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
        printf("[  ] TCP      : %s\n", errStr().c_str());
    return s;
}

inline SOCKET makeUdpSocket() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
        printf("[  ] UDP      : %s\n", errStr().c_str());
    return s;
}

//     IPv4 sockaddr_in                                                       
inline sockaddr_in makeAddr(const char* ip, USHORT port) {
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = htons(port);
    if (ip) inet_pton(AF_INET, ip, &a.sin_addr);
    else    a.sin_addr.s_addr = INADDR_ANY;
    return a;
}

//                                                                      
inline void setRecvTimeout(SOCKET s, DWORD ms) {
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms));
}

//               (   )                                                
inline std::string nowStr() {
    SYSTEMTIME st; GetLocalTime(&st);
    char buf[24];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d",
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}
