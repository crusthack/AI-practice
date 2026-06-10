#include "NetUtil.hpp"
#include <vector>
static constexpr unsigned short PORT = 27021;
struct Entry { SOCKET s; WSAEVENT e; };
int main() {
    net::WsaSession wsa;
    auto listen_sock = net::tcp_socket();
    u_long nonblock = 1; ioctlsocket(listen_sock.value, FIONBIO, &nonblock);
    net::bind_listen(listen_sock.value, PORT);
    WSAEVENT listen_event = WSACreateEvent();
    WSAEventSelect(listen_sock.value, listen_event, FD_ACCEPT | FD_CLOSE);
    std::vector<Entry> entries{{listen_sock.value, listen_event}};
    std::puts("WSAEventSelect echo server on 27021");
    while (true) {
        std::vector<WSAEVENT> events;
        for (auto& e : entries) events.push_back(e.e);
        DWORD idx = WSAWaitForMultipleEvents((DWORD)events.size(), events.data(), FALSE, WSA_INFINITE, FALSE);
        if (idx == WSA_WAIT_FAILED) break;
        size_t pos = idx - WSA_WAIT_EVENT_0;
        WSANETWORKEVENTS ne{};
        WSAEnumNetworkEvents(entries[pos].s, entries[pos].e, &ne);
        if (entries[pos].s == listen_sock.value && (ne.lNetworkEvents & FD_ACCEPT)) {
            SOCKET c = accept(listen_sock.value, nullptr, nullptr);
            if (c != INVALID_SOCKET && entries.size() < WSA_MAXIMUM_WAIT_EVENTS) {
                WSAEVENT ev = WSACreateEvent();
                WSAEventSelect(c, ev, FD_READ | FD_CLOSE);
                entries.push_back({c, ev});
            }
        } else if (ne.lNetworkEvents & FD_READ) {
            char b[512]; int n = recv(entries[pos].s, b, sizeof(b), 0);
            if (n > 0) send(entries[pos].s, b, n, 0);
        }
        if (pos != 0 && (ne.lNetworkEvents & FD_CLOSE)) {
            closesocket(entries[pos].s); WSACloseEvent(entries[pos].e); entries.erase(entries.begin() + pos);
        }
    }
}
