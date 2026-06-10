#include "NetUtil.hpp"
#include <vector>
static constexpr unsigned short PORT = 27020;
int main() {
    net::WsaSession wsa;
    auto listen_sock = net::tcp_socket();
    if (!net::bind_listen(listen_sock.value, PORT)) return 1;
    std::vector<SOCKET> clients;
    std::puts("select echo server on 27020");
    while (true) {
        fd_set readset; FD_ZERO(&readset); FD_SET(listen_sock.value, &readset);
        for (SOCKET c : clients) FD_SET(c, &readset);
        timeval tv{1, 0};
        int ready = select(0, &readset, nullptr, nullptr, &tv);
        if (ready == SOCKET_ERROR) break;
        if (FD_ISSET(listen_sock.value, &readset)) {
            SOCKET c = accept(listen_sock.value, nullptr, nullptr);
            if (c != INVALID_SOCKET) clients.push_back(c);
        }
        for (size_t i = 0; i < clients.size();) {
            SOCKET c = clients[i];
            if (!FD_ISSET(c, &readset)) { ++i; continue; }
            char b[1024]; int n = recv(c, b, sizeof(b), 0);
            if (n <= 0) { closesocket(c); clients.erase(clients.begin() + i); continue; }
            send(c, b, n, 0); ++i;
        }
    }
}
