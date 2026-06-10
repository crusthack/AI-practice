#include "NetUtil.hpp"
#include <fstream>
#include <sstream>
#include <thread>
static constexpr unsigned short PORT = 8080;
std::string read_file(const char* path) { std::ifstream f(path, std::ios::binary); std::ostringstream ss; ss << f.rdbuf(); return ss.str(); }
void serve(SOCKET raw) {
    net::Socket s(raw); char b[4096]{}; int n = recv(s.value, b, sizeof(b)-1, 0); if (n <= 0) return;
    std::string req(b, n), body, type = "text/plain";
    if (req.rfind("GET /static ", 0) == 0) { body = read_file("static.txt"); if (body.empty()) body = "create static.txt beside the exe\n"; }
    else if (req.rfind("POST ", 0) == 0) { size_t p = req.find("\r\n\r\n"); body = p == std::string::npos ? "" : req.substr(p+4); }
    else { type = "text/html"; body = "<h1>Winsock HTTP server</h1><p>GET /static or POST /echo</p>"; }
    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: " + type + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
    net::send_all(s.value, resp.c_str(), (int)resp.size());
}
int main() {
    net::WsaSession wsa; auto ls = net::tcp_socket(); if (!net::bind_listen(ls.value, PORT)) return 1;
    std::puts("HTTP server on http://127.0.0.1:8080");
    while (true) { SOCKET c = accept(ls.value, nullptr, nullptr); if (c != INVALID_SOCKET) std::thread(serve, c).detach(); }
}
