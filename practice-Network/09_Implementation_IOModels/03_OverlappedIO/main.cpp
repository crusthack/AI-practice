#include "NetUtil.hpp"
static constexpr unsigned short PORT = 27022;
int main() {
    net::WsaSession wsa;
    auto server = net::tcp_socket(WSA_FLAG_OVERLAPPED);
    if (!net::bind_listen(server.value, PORT, 1)) return 1;
    std::puts("overlapped single-client echo server on 27022");
    net::Socket client(accept(server.value, nullptr, nullptr));
    char buffer[1024]{};
    WSABUF wsabuf{sizeof(buffer), buffer};
    WSAOVERLAPPED ov{}; ov.hEvent = WSACreateEvent();
    DWORD flags = 0, bytes = 0;
    int rc = WSARecv(client.value, &wsabuf, 1, &bytes, &flags, &ov, nullptr);
    if (rc == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) return net::print_error("WSARecv"), 1;
    WSAWaitForMultipleEvents(1, &ov.hEvent, TRUE, WSA_INFINITE, FALSE);
    WSAGetOverlappedResult(client.value, &ov, &bytes, FALSE, &flags);
    std::printf("completed recv: %lu bytes\n", bytes);
    WSASend(client.value, &wsabuf, 1, &bytes, 0, nullptr, nullptr);
    WSACloseEvent(ov.hEvent);
    return 0;
}
