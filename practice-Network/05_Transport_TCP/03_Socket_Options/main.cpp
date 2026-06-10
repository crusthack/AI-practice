#include "NetUtil.hpp"
#include <mstcpip.h>
int main() {
    net::WsaSession wsa;
    auto s = net::tcp_socket();
    BOOL keepalive = TRUE, reuse = TRUE, nodelay = TRUE;
    DWORD timeout = 3000;
    int sndbuf = 64 * 1024;
    setsockopt(s.value, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive));
    setsockopt(s.value, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    setsockopt(s.value, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(s.value, SOL_SOCKET, SO_SNDBUF, (char*)&sndbuf, sizeof(sndbuf));
    setsockopt(s.value, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    linger lg{1, 0};
    setsockopt(s.value, SOL_SOCKET, SO_LINGER, (char*)&lg, sizeof(lg));
    int got = 0; int len = sizeof(got);
    getsockopt(s.value, SOL_SOCKET, SO_SNDBUF, (char*)&got, &len);
    std::printf("SO_SNDBUF=%d, SO_RCVTIMEO=%lu ms, TCP_NODELAY=on, SO_LINGER={on,0}\n", got, timeout);
    return 0;
}
