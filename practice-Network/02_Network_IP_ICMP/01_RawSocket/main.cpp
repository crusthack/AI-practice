#include "NetUtil.hpp"
#pragma pack(push,1)
struct IpHeader { unsigned char ver_ihl,tos; unsigned short len,id,frag; unsigned char ttl,proto; unsigned short csum; unsigned int src,dst; };
#pragma pack(pop)
int main() {
    net::WsaSession wsa;
    net::Socket s(socket(AF_INET, SOCK_RAW, IPPROTO_ICMP));
    if (!s) { net::print_error("raw socket - run as Administrator"); return 1; }
    DWORD timeout = 10000; setsockopt(s.value, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    std::puts("waiting for one ICMP packet; run ping 127.0.0.1 in another terminal if needed");
    char b[2048]; sockaddr_in from{}; int fl=sizeof(from); int n=recvfrom(s.value,b,sizeof(b),0,(sockaddr*)&from,&fl);
    if(n>=(int)sizeof(IpHeader)){ auto* ip=(IpHeader*)b; char src[16],dst[16]; inet_ntop(AF_INET,&ip->src,src,sizeof(src)); inet_ntop(AF_INET,&ip->dst,dst,sizeof(dst)); std::printf("IP ttl=%u proto=%u %s -> %s bytes=%d\n",ip->ttl,ip->proto,src,dst,n); }
    return 0;
}
