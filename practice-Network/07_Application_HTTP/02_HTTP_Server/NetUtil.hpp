#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

namespace net {

struct WsaSession {
    WsaSession() {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            std::printf("WSAStartup failed: %d\n", WSAGetLastError());
            std::exit(1);
        }
    }
    ~WsaSession() { WSACleanup(); }
    WsaSession(const WsaSession&) = delete;
    WsaSession& operator=(const WsaSession&) = delete;
};

struct Socket {
    SOCKET value = INVALID_SOCKET;
    Socket() = default;
    explicit Socket(SOCKET s) : value(s) {}
    ~Socket() { reset(); }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : value(other.value) { other.value = INVALID_SOCKET; }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            reset();
            value = other.value;
            other.value = INVALID_SOCKET;
        }
        return *this;
    }
    void reset(SOCKET s = INVALID_SOCKET) {
        if (value != INVALID_SOCKET) closesocket(value);
        value = s;
    }
    SOCKET release() {
        SOCKET s = value;
        value = INVALID_SOCKET;
        return s;
    }
    explicit operator bool() const { return value != INVALID_SOCKET; }
};

inline void print_error(const char* where, int code = WSAGetLastError()) {
    std::printf("%s failed: %d\n", where, code);
}

inline sockaddr_in ipv4(const char* ip, unsigned short port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip == nullptr || std::strcmp(ip, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        inet_pton(AF_INET, ip, &addr.sin_addr);
    }
    return addr;
}

inline Socket tcp_socket(DWORD flags = 0) {
    SOCKET s = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, flags);
    if (s == INVALID_SOCKET) print_error("WSASocket(TCP)");
    return Socket(s);
}

inline Socket udp_socket(DWORD flags = 0) {
    SOCKET s = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, flags);
    if (s == INVALID_SOCKET) print_error("WSASocket(UDP)");
    return Socket(s);
}

inline bool set_reuseaddr(SOCKET s) {
    BOOL yes = TRUE;
    return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == 0;
}

inline bool bind_listen(SOCKET s, unsigned short port, int backlog = SOMAXCONN) {
    set_reuseaddr(s);
    sockaddr_in addr = ipv4("0.0.0.0", port);
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        print_error("bind");
        return false;
    }
    if (listen(s, backlog) == SOCKET_ERROR) {
        print_error("listen");
        return false;
    }
    return true;
}

inline bool connect_loopback(SOCKET s, unsigned short port, const char* ip = "127.0.0.1") {
    sockaddr_in addr = ipv4(ip, port);
    if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        print_error("connect");
        return false;
    }
    return true;
}

inline int send_all(SOCKET s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        int n = send(s, data + sent, len - sent, 0);
        if (n == SOCKET_ERROR) return SOCKET_ERROR;
        if (n == 0) break;
        sent += n;
    }
    return sent;
}

inline int recv_exact(SOCKET s, char* data, int len) {
    int got = 0;
    while (got < len) {
        int n = recv(s, data + got, len - got, 0);
        if (n <= 0) return n == 0 ? 0 : SOCKET_ERROR;
        got += n;
    }
    return got;
}

inline std::string peer_name(const sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(ntohs(addr.sin_port));
}

inline uint32_t crc32(const void* data, size_t len) {
    static uint32_t table[256]{};
    static bool ready = false;
    if (!ready) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for (int j = 0; j < 8; ++j) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        ready = true;
    }
    uint32_t c = 0xFFFFFFFFu;
    const auto* p = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < len; ++i) c = table[(c ^ p[i]) & 0xFFu] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

} // namespace net
