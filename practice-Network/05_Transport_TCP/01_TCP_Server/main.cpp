#include "NetUtil.hpp"
#include <thread>
#include <vector>
static constexpr unsigned short PORT = 27015;
void client_worker(SOCKET raw) {
    net::Socket client(raw);
    char buf[1024];
    while (true) {
        int n = recv(client.value, buf, sizeof(buf), 0);
        if (n <= 0) break;
        net::send_all(client.value, buf, n);
    }
}
int main() {
    net::WsaSession wsa;
    auto server = net::tcp_socket();
    if (!server || !net::bind_listen(server.value, PORT)) return 1;
    std::puts("TCP echo server on 0.0.0.0:27015");
    std::vector<std::thread> threads;
    while (true) {
        sockaddr_in peer{}; int len = sizeof(peer);
        SOCKET c = accept(server.value, reinterpret_cast<sockaddr*>(&peer), &len);
        if (c == INVALID_SOCKET) { net::print_error("accept"); continue; }
        std::printf("accepted %s\n", net::peer_name(peer).c_str());
        threads.emplace_back(client_worker, c);
        threads.back().detach();
    }
}
