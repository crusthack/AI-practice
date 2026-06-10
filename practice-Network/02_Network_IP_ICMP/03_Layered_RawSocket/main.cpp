// 03_Layered_RawSocket: RawSocket   z~ S    
//  '   ? ?   "o:
//   ping <IP>         ?" ICMP Echo Request 4 sO  " ?  + IP/ICMP  -  "  OO <   o 
//   sniff <NIC_IP>    ?"  ? . NIC "  ~  z    "o o  O  ,  50 o   ~ ( ?  z   ." ^~)
//
//  <  -?  ~^:
//   03_Layered_RawSocket.exe ping 8.8.8.8
//   03_Layered_RawSocket.exe sniff 192.168.1.100
#include "raw_socket.hpp"
#include <windows.h>

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Mode 1: ICMP Ping
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runPing(const char* targetIp) {
    RawSocket raw(RawSocket::Proto::ICMP);
    if (!raw.valid()) return 1;

    raw.setRecvTimeout(2000);

    WORD pid    = (WORD)GetCurrentProcessId();
    int  payLen = 32;
    int  count  = 4;

    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);

    printf("PING %s  %d bytes payload  id=0x%04X\n\n", targetIp, payLen, pid);
    printf("%-5s %-16s %-5s %-5s %s\n", "seq", "from", "type", "TTL", "RTT(ms)");
    printf("%-5s %-16s %-5s %-5s %s\n", "---", "----", "----", "---", "-------");

    int received = 0;

    for (int seq = 1; seq <= count; ++seq) {
        char pkt[sizeof(IcmpHeader) + 32]{};
        buildIcmpEcho(pkt, payLen, pid, (WORD)seq);

        QueryPerformanceCounter(&t0);
        raw.sendIcmp(targetIp, pkt, sizeof(pkt));

        char recvBuf[2048]{};
        char fromIp[16]{};

        // ICMP raw socket ?   "  ICMP   ^~ <  .~ ? o  s   Echo Reply    "  .O O ?   ""
        bool got = false;
        while (!got) {
            int r = raw.recvPacket(recvBuf, sizeof(recvBuf), fromIp);
            if (r == SOCKET_ERROR) break;   //  f? z" ." >f

            const IpHeader*   ip   = pktIp(recvBuf);
            if (ip->protocol != IPPROTO_ICMP) continue;

            const IcmpHeader* icmp = pktIcmp(recvBuf);
            // type=0(Echo Reply) + id   ~  T. 
            if (icmp->type != 0 || ntohs(icmp->id) != pid) continue;

            QueryPerformanceCounter(&t1);
            double rtt = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / freq.QuadPart;

            printf("%-5d %-16s %-5d %-5d %.2f\n",
                   seq, fromIp, icmp->type, ip->ttl, rtt);

            // IP + ICMP  -  "  f  "   o 
            RawSocket::printIpHdr(ip);
            RawSocket::printIcmpHdr(icmp);
            printf("\n");

            ++received;
            got = true;
        }
        if (!got)
            printf("%-5d %-16s %s\n", seq, "-", " f? z" ." >f");

        Sleep(1000);
    }

    printf(" "? "?  ?  ":  " ? =%d  ^~ < =%d  ?  < =%d  "? "?\n",
           count, received, count - received);
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Mode 2: Packet Sniffer
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runSniffer(const char* nicIp) {
    // IPPROTO_IP + SIO_RCVALL:   "  Z~  S    ? ,~ S"   "  IP  O  ,   ^~ < 
    RawSocket raw(RawSocket::Proto::ICMP);  // IPPROTO_IP  ?  -? o  ? o ICMP o  <o z'
    //  <  o o SIO_RCVALL ? IPPROTO_IP(0) raw socket  -  "o O  ~  " z^  T z' .  <^ < .
    // Windows -  "o IPPROTO_IP raw socket: socket(AF_INET, SOCK_RAW, IPPROTO_IP)
    //  -   "o S"   ' SOCKET "  f  "  .  <^ < .
    SOCKET sniffSock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if (sniffSock == INVALID_SOCKET) {
        printf(" S  <^   ?O "  f  "   <  O : %d  ( ?  z   O .o  ." s")\n", WSAGetLastError());
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = 0;
    inet_pton(AF_INET, nicIp, &addr.sin_addr);
    if (bind(sniffSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind  <  O : %d  (NIC IP ?  z S" ?  T.  .~ "  s")\n", WSAGetLastError());
        closesocket(sniffSock);
        return 1;
    }

    DWORD flag = 1, ret = 0;
    if (WSAIoctl(sniffSock, SIO_RCVALL, &flag, sizeof(flag),
                 nullptr, 0, &ret, nullptr, nullptr) == SOCKET_ERROR) {
        printf("SIO_RCVALL  <  O : %d\n", WSAGetLastError());
        closesocket(sniffSock);
        return 1;
    }
    printf("[ S  <^ ] NIC %s  ~  z    "o  <o z', 50 o   ~...\n\n", nicIp);

    //  "" o ?  o "   s  " 
    int cntTcp = 0, cntUdp = 0, cntIcmp = 0, cntOther = 0;

    //  -  "  o 
    printf("%-3s %-6s %-16s %-16s %-5s %-5s\n",
           "#", "Proto", "Src IP", "Dst IP", "Src P", "Dst P");
    printf("%s\n", "--- ------ ---------------- ---------------- ----- -----");

    char buf[65535];
    for (int i = 1; i <= 50; ++i) {
        int r = recv(sniffSock, buf, sizeof(buf), 0);
        if (r < (int)sizeof(IpHeader)) continue;

        const IpHeader* ip = pktIp(buf);
        if (ip->version() != 4) { --i; continue; }

        char src[16], dst[16];
        ipToStr(ip->srcAddr, src);
        ipToStr(ip->dstAddr, dst);

        WORD sp = 0, dp = 0;
        switch (ip->protocol) {
        case IPPROTO_TCP: {
            const TcpHeader* tcp = pktTcp(buf);
            sp = ntohs(tcp->srcPort); dp = ntohs(tcp->dstPort);
            ++cntTcp; break;
        }
        case IPPROTO_UDP: {
            const UdpHeader* udp = pktUdp(buf);
            sp = ntohs(udp->srcPort); dp = ntohs(udp->dstPort);
            ++cntUdp; break;
        }
        case IPPROTO_ICMP: ++cntIcmp; break;
        default:           ++cntOther; break;
        }

        printf("%-3d %-6s %-16s %-16s %-5u %-5u\n",
               i, protoName(ip->protocol), src, dst, sp, dp);

        // TCP/ICMP S"  -  "  f  "  "  o 
        if (ip->protocol == IPPROTO_TCP)
            RawSocket::printTcpHdr(pktTcp(buf));
        else if (ip->protocol == IPPROTO_ICMP)
            RawSocket::printIcmpHdr(pktIcmp(buf));
    }

    printf("\n "? "?  ' ": TCP=%d UDP=%d ICMP=%d OTHER=%d  "? "?\n",
           cntTcp, cntUdp, cntIcmp, cntOther);

    flag = 0;
    WSAIoctl(sniffSock, SIO_RCVALL, &flag, sizeof(flag),
             nullptr, 0, &ret, nullptr, nullptr);
    closesocket(sniffSock);
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
int main(int argc, char* argv[]) {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup  <  O : %d\n", WSAGetLastError());
        return 1;
    }

    int ret = 0;
    if (argc >= 3 && strcmp(argv[1], "ping") == 0) {
        ret = runPing(argv[2]);
    } else if (argc >= 3 && strcmp(argv[1], "sniff") == 0) {
        ret = runSniffer(argv[2]);
    } else {
        printf(" ,  s  .:\n");
        printf("  %s ping  <target_ip>    ( ~^: ping 8.8.8.8)\n", argv[0]);
        printf("  %s sniff <nic_ip>       ( ~^: sniff 192.168.1.5)\n", argv[0]);
        printf("\n '    "o   '   ?  z   O .o   ." s" .  <^ < .\n");
        ret = 1;
    }

    WSACleanup();
    return ret;
}
