#include "NetUtil.hpp"
#include <map>
std::string recv_until_close(SOCKET s) { std::string r; char b[4096]; for (;;) { int n = recv(s,b,sizeof(b),0); if (n<=0) break; r.append(b,n); } return r; }
std::string decode_chunked(const std::string& body) {
    std::string out; size_t p = 0;
    while (p < body.size()) {
        size_t e = body.find("\r\n", p); if (e == std::string::npos) break;
        int n = std::strtol(body.substr(p, e-p).c_str(), nullptr, 16); p = e + 2;
        if (n == 0) break; if (p + n > body.size()) break;
        out.append(body, p, n); p += n + 2;
    }
    return out;
}
int main() {
    net::WsaSession wsa;
    addrinfo hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    addrinfo* ai = nullptr; getaddrinfo("example.com", "80", &hints, &ai);
    net::Socket s(socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol));
    connect(s.value, ai->ai_addr, (int)ai->ai_addrlen); freeaddrinfo(ai);
    const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: */*\r\nConnection: close\r\n\r\n";
    net::send_all(s.value, req, (int)std::strlen(req));
    std::string raw = recv_until_close(s.value);
    size_t split = raw.find("\r\n\r\n");
    std::printf("%.*s\n\n", (int)split, raw.c_str());
    std::string body = split == std::string::npos ? "" : raw.substr(split + 4);
    if (raw.find("Transfer-Encoding: chunked") != std::string::npos || raw.find("transfer-encoding: chunked") != std::string::npos) body = decode_chunked(body);
    size_t preview = body.size() < 500 ? body.size() : 500;
    std::printf("body bytes=%zu\n%.*s\n", body.size(), (int)preview, body.c_str());
}
