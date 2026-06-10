#include "NetUtil.hpp"
static constexpr unsigned short PORT = 27015;
int main() {
    net::WsaSession wsa;
    net::Socket s;
    for (int attempt = 1; attempt <= 3; ++attempt) {
        s = net::tcp_socket();
        if (s && net::connect_loopback(s.value, PORT)) break;
        s.reset();
        std::printf("retry %d/3 after backoff\n", attempt);
        Sleep(250u << attempt);
    }
    if (!s) return 1;
    const char* msg = "hello from TCP client";
    net::send_all(s.value, msg, (int)std::strlen(msg));
    shutdown(s.value, SD_SEND);
    char buf[1024]{};
    int n = recv(s.value, buf, sizeof(buf)-1, 0);
    if (n > 0) std::printf("echo: %s\n", buf);
    return 0;
}
