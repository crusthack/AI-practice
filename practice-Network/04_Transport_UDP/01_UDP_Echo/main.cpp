#include "NetUtil.hpp"
static constexpr unsigned short PORT = 27016;
int server() {
    auto s = net::udp_socket();
    sockaddr_in addr = net::ipv4("0.0.0.0", PORT);
    if (bind(s.value, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) return net::print_error("bind"), 1;
    std::puts("UDP echo server on 0.0.0.0:27016");
    char buf[1500];
    while (true) {
        sockaddr_in from{}; int from_len = sizeof(from);
        int n = recvfrom(s.value, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &from_len);
        if (n == SOCKET_ERROR) continue;
        std::printf("%d bytes from %s\n", n, net::peer_name(from).c_str());
        sendto(s.value, buf, n, 0, reinterpret_cast<sockaddr*>(&from), from_len);
    }
}
int client() {
    auto s = net::udp_socket();
    sockaddr_in dst = net::ipv4("127.0.0.1", PORT);
    const char* msg = "udp datagram";
    sendto(s.value, msg, (int)std::strlen(msg), 0, reinterpret_cast<sockaddr*>(&dst), sizeof(dst));
    char buf[512]{}; int len = sizeof(dst);
    int n = recvfrom(s.value, buf, sizeof(buf)-1, 0, reinterpret_cast<sockaddr*>(&dst), &len);
    if (n > 0) std::printf("echo: %s\n", buf);
    return 0;
}
int main(int argc, char** argv) {
    net::WsaSession wsa;
    return argc > 1 && std::strcmp(argv[1], "client") == 0 ? client() : server();
}
