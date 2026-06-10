#pragma once
#include "ip_headers.hpp"
#include <string>

//                                                                             
// RawSocket   SOCK_RAW  RAII       .
//
//   :
//   1)         (SOCK_RAW + IPPROTO_IP + SIO_RCVALL)
//   2) ICMP      (SOCK_RAW + IPPROTO_ICMP)
//   3) TCP/UDP             (              )
//
//   :            socket()          .
//                                                                             
class RawSocket {
public:
    enum class Proto { ICMP = IPPROTO_ICMP, TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

    explicit RawSocket(Proto proto)
        : proto_(proto)
        , sock_(socket(AF_INET, SOCK_RAW, static_cast<int>(proto)))
    {
        if (sock_ == INVALID_SOCKET)
            printf("[RawSocket]      : %d                  \n",
                   WSAGetLastError());
    }

    ~RawSocket() { destroy(); }

    RawSocket(RawSocket&& o) noexcept : sock_(o.sock_), proto_(o.proto_),
        promiscEnabled_(o.promiscEnabled_) {
        o.sock_ = INVALID_SOCKET;
    }
    RawSocket(const RawSocket&) = delete;
    RawSocket& operator=(const RawSocket&) = delete;

    bool valid() const { return sock_ != INVALID_SOCKET; }

    //                                                                    

    // SIO_RCVALL                    IP           .
    // loopback(127.0.0.1)        NIC IP       .
    bool bindTo(const char* localIp) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, localIp, &addr.sin_addr);
        if (bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("[RawSocket] bind   : %d\n", WSAGetLastError());
            return false;
        }
        boundIp_ = localIp;
        return true;
    }

    //      (Promiscuous):                IP      
    // SIO_RCVALL       + NIC bind   
    bool enablePromiscuous() {
        if (promiscEnabled_) return true;
        DWORD flag = 1, ret = 0;
        if (WSAIoctl(sock_, SIO_RCVALL, &flag, sizeof(flag),
                     nullptr, 0, &ret, nullptr, nullptr) == SOCKET_ERROR) {
            printf("[RawSocket] SIO_RCVALL       : %d\n", WSAGetLastError());
            return false;
        }
        promiscEnabled_ = true;
        printf("[RawSocket]           (NIC: %s)\n", boundIp_.c_str());
        return true;
    }

    void setRecvTimeout(DWORD ms) {
        setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms));
    }

    //                                                                   

    // ICMP      OS  IP             .
    // icmpData   IcmpHeader +               .
    int sendIcmp(const char* dstIp, const void* icmpData, int len) {
        sockaddr_in dst{};
        dst.sin_family = AF_INET;
        inet_pton(AF_INET, dstIp, &dst.sin_addr);
        int r = sendto(sock_, (const char*)icmpData, len, 0,
                       (sockaddr*)&dst, sizeof(dst));
        if (r == SOCKET_ERROR)
            printf("[RawSocket] sendIcmp   : %d\n", WSAGetLastError());
        return r;
    }

    //                                                                   

    //         : buf  IP             .
    // fromIp(  ):     IP       (16   )
    //    :         , 0=     , SOCKET_ERROR=  
    int recvPacket(void* buf, int bufLen, char* fromIp = nullptr) {
        sockaddr_in from{};
        int fromLen = sizeof(from);
        int r = recvfrom(sock_, (char*)buf, bufLen, 0,
                         (sockaddr*)&from, &fromLen);
        if (r > 0 && fromIp)
            inet_ntop(AF_INET, &from.sin_addr, fromIp, 16);
        return r;
    }

    //                                                                  

    static void printIpHdr(const IpHeader* ip) {
        char src[16], dst[16];
        ipToStr(ip->srcAddr, src);
        ipToStr(ip->dstAddr, dst);
        printf("  IP  v=%d IHL=%d TOS=0x%02X len=%d id=0x%04X TTL=%d proto=%s(%d)\n",
               ip->version(), ip->headerLen(), ip->tos,
               ntohs(ip->totalLen), ntohs(ip->id),
               ip->ttl, protoName(ip->protocol), ip->protocol);
        printf("      %s   %s  DF=%d MF=%d fragOff=%d\n",
               src, dst, ip->dontFrag(), ip->moreFrags(), ip->fragOffset());
    }

    static void printIcmpHdr(const IcmpHeader* icmp) {
        printf("  ICMP type=%d(%s) code=%d id=0x%04X seq=%d\n",
               icmp->type, IcmpHeader::typeName(icmp->type),
               icmp->code, ntohs(icmp->id), ntohs(icmp->seq));
    }

    static void printTcpHdr(const TcpHeader* tcp) {
        char flags[32]{};
        tcp->printFlags(flags);
        printf("  TCP  %u   %u  seq=%u ack=%u  win=%u  [%s]\n",
               ntohs(tcp->srcPort), ntohs(tcp->dstPort),
               ntohl(tcp->seq), ntohl(tcp->ackSeq),
               ntohs(tcp->window), flags);
    }

    static void printUdpHdr(const UdpHeader* udp) {
        printf("  UDP  %u   %u  len=%u\n",
               ntohs(udp->srcPort), ntohs(udp->dstPort),
               ntohs(udp->length));
    }

    //                         +   
    static const BYTE* getPayload(const void* pkt, int pktLen, int& outLen) {
        const auto* ip = pktIp(pkt);
        int offset = ip->headerLen();
        switch (ip->protocol) {
        case IPPROTO_TCP: offset += pktTcp(pkt)->headerLen(); break;
        case IPPROTO_UDP: offset += (int)sizeof(UdpHeader);   break;
        case IPPROTO_ICMP:offset += (int)sizeof(IcmpHeader);  break;
        }
        outLen = ntohs(ip->totalLen) - offset;
        if (outLen < 0) outLen = 0;
        return reinterpret_cast<const BYTE*>(pkt) + offset;
    }

private:
    SOCKET     sock_           = INVALID_SOCKET;
    Proto      proto_;
    bool       promiscEnabled_ = false;
    std::string boundIp_;

    void destroy() {
        if (sock_ == INVALID_SOCKET) return;
        if (promiscEnabled_) {
            DWORD flag = 0, ret = 0;
            WSAIoctl(sock_, SIO_RCVALL, &flag, sizeof(flag),
                     nullptr, 0, &ret, nullptr, nullptr);
        }
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
};

//    ICMP Echo Request                                                     
inline void buildIcmpEcho(char* buf, int payloadSize, WORD id, WORD seq) {
    auto* h = reinterpret_cast<IcmpHeader*>(buf);
    h->type     = 8;    // Echo Request
    h->code     = 0;
    h->id       = htons(id);
    h->seq      = htons(seq);
    h->checksum = 0;
    for (int i = 0; i < payloadSize; ++i)
        buf[sizeof(IcmpHeader) + i] = (char)('A' + i % 26);
    //        +                   
    h->checksum = calcChecksum(buf, (int)sizeof(IcmpHeader) + payloadSize);
}
