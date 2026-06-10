#include "NetUtil.hpp"
#include <thread>
static constexpr unsigned short PORT = 27023;
struct IoData { WSAOVERLAPPED ov{}; WSABUF buf{}; char data[1024]{}; };
int main() {
    net::WsaSession wsa;
    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    std::thread worker([&] {
        while (true) {
            DWORD bytes = 0; ULONG_PTR key = 0; LPOVERLAPPED pov = nullptr;
            if (!GetQueuedCompletionStatus(iocp, &bytes, &key, &pov, INFINITE)) continue;
            auto* io = reinterpret_cast<IoData*>(pov); SOCKET s = (SOCKET)key;
            if (bytes == 0) { closesocket(s); delete io; continue; }
            send(s, io->data, bytes, 0);
            ZeroMemory(&io->ov, sizeof(io->ov)); io->buf.len = sizeof(io->data); io->buf.buf = io->data;
            DWORD flags = 0, recvd = 0; WSARecv(s, &io->buf, 1, &recvd, &flags, &io->ov, nullptr);
        }
    });
    worker.detach();
    auto ls = net::tcp_socket(WSA_FLAG_OVERLAPPED);
    if (!net::bind_listen(ls.value, PORT)) return 1;
    std::puts("IOCP echo server on 27023");
    while (true) {
        SOCKET c = accept(ls.value, nullptr, nullptr);
        if (c == INVALID_SOCKET) continue;
        CreateIoCompletionPort((HANDLE)c, iocp, (ULONG_PTR)c, 0);
        auto* io = new IoData(); io->buf.buf = io->data; io->buf.len = sizeof(io->data);
        DWORD flags = 0, recvd = 0; WSARecv(c, &io->buf, 1, &recvd, &flags, &io->ov, nullptr);
    }
}
