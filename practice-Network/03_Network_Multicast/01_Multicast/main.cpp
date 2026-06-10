#include "NetUtil.hpp"
static constexpr unsigned short PORT = 27040; static constexpr const char* MULTICAST_GROUP = "239.255.0.1";
int recv_mode() {
    auto s = net::udp_socket(); sockaddr_in a = net::ipv4("0.0.0.0", PORT); bind(s.value,(sockaddr*)&a,sizeof(a));
    ip_mreq m{}; inet_pton(AF_INET, MULTICAST_GROUP, &m.imr_multiaddr); m.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(s.value, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&m, sizeof(m));
    char b[512]{}; sockaddr_in from{}; int fl=sizeof(from); int n=recvfrom(s.value,b,sizeof(b)-1,0,(sockaddr*)&from,&fl); if(n>0) std::printf("%s\n",b);
    setsockopt(s.value, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&m, sizeof(m)); return 0;
}
int send_mode() {
    auto s = net::udp_socket(); DWORD ttl = 1; setsockopt(s.value, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
    sockaddr_in g = net::ipv4(MULTICAST_GROUP, PORT); const char* msg="hello multicast"; sendto(s.value,msg,(int)strlen(msg),0,(sockaddr*)&g,sizeof(g)); return 0;
}
int main(int argc,char**argv){ net::WsaSession wsa; return argc>1 && strcmp(argv[1],"send")==0 ? send_mode() : recv_mode(); }
