#include "NetUtil.hpp"
#pragma pack(push,1)
struct Header { uint32_t magic, seq, len, crc; uint16_t version, type; };
#pragma pack(pop)
static constexpr uint32_t MAGIC = 0x4E455431; static constexpr unsigned short PORT = 27031;
bool send_msg(SOCKET s, uint16_t type, uint32_t seq, const std::string& payload) {
    Header h{htonl(MAGIC), htonl(seq), htonl((uint32_t)payload.size()), htonl(net::crc32(payload.data(), payload.size())), htons(1), htons(type)};
    return net::send_all(s, (char*)&h, sizeof(h)) == sizeof(h) && net::send_all(s, payload.data(), (int)payload.size()) == (int)payload.size();
}
bool recv_msg(SOCKET s, Header& h, std::string& payload) {
    if (net::recv_exact(s, (char*)&h, sizeof(h)) <= 0) return false;
    uint32_t len = ntohl(h.len); payload.assign(len, '\0');
    if (len && net::recv_exact(s, payload.data(), (int)len) <= 0) return false;
    return ntohl(h.magic) == MAGIC && ntohl(h.crc) == net::crc32(payload.data(), payload.size());
}
int server() {
    auto ls = net::tcp_socket(); net::bind_listen(ls.value, PORT, 1); net::Socket c(accept(ls.value,nullptr,nullptr));
    Header h{}; std::string p; while (recv_msg(c.value,h,p)) { std::printf("type=%u seq=%u payload=%s\n", ntohs(h.type), ntohl(h.seq), p.c_str()); send_msg(c.value, 2, ntohl(h.seq), "ack:" + p); }
    return 0;
}
int client() {
    auto s = net::tcp_socket(); if (!net::connect_loopback(s.value, PORT)) return 1;
    send_msg(s.value, 1, 1, "framed payload with crc32"); Header h{}; std::string p; recv_msg(s.value,h,p); std::puts(p.c_str()); return 0;
}
int main(int argc, char** argv) { net::WsaSession wsa; return argc > 1 && std::strcmp(argv[1],"client")==0 ? client() : server(); }
