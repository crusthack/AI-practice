// 02_Layered_RawICMP:  "" o ?  o  S  f   o .~ o"  ?" IP  -  "    '  OO <  .~ 
//             ICMP Echo  O  ,  "   '    .  ping "   ~" .  <^ < .
//  <  -?  s" :  ?  z   O .o (SOCK_RAW + IPPROTO_ICMP)
//  .  <   ? : sendto()  <o OS ? IP  -  "   T  ,
//            recvfrom()  o  > ?  "  S" IP  -  " ? "   <o z' .  <^ < .
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

static constexpr char TARGET_IP[]  = "8.8.8.8";
static constexpr int  PING_COUNT   = 4;
static constexpr int  PAYLOAD_SIZE = 32;
static constexpr DWORD RECV_TIMEOUT_MS = 2000;

//  "? "? IP  -  " (RFC 791, 20 bytes,  ~  .~  -? O)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
#pragma pack(push, 1)
struct IpHeader {
    BYTE  verIhl;       // [7:4]=version( .  f  4), [3:0]=IHL(4 "  S   <  o"  -  "   )
    BYTE  tos;
    WORD  totalLen;     //  "  S  >O   "  S   ~  "
    WORD  id;
    WORD  flagFrag;     // [15:13]=flags, [12:0]=fragment offset
    BYTE  ttl;
    BYTE  protocol;     // 1=ICMP, 6=TCP, 17=UDP
    WORD  checksum;
    DWORD srcAddr;
    DWORD dstAddr;
};

//  "? "? ICMP  -  " (RFC 792)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
struct IcmpHeader {
    BYTE type;          // 8=Echo Request, 0=Echo Reply
    BYTE code;
    WORD checksum;      // ICMP  -  " +  Z~  o "o  "  -   O? .o    " 
    WORD id;            //  "" o "  S  ID o  <  "
    WORD seq;
};
#pragma pack(pop)

// RFC 1071   "  "     " : 16 " S   >O "o  <  o" 1 ~   ^~  . 
static WORD internetChecksum(const void* data, int len) {
    const WORD* p = static_cast<const WORD*>(data);
    DWORD sum = 0;
    while (len > 1) { sum += *p++; len -= 2; }
    if (len == 1) sum += *(const BYTE*)p;
    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return static_cast<WORD>(~sum);
}

static void printIpHeader(const IpHeader* ip) {
    char src[16], dst[16];
    inet_ntop(AF_INET, &ip->srcAddr, src, sizeof(src));
    inet_ntop(AF_INET, &ip->dstAddr, dst, sizeof(dst));
    int ihl = (ip->verIhl & 0x0F) * 4;
    printf("    IP { ver=%d IHL=%d TTL=%d proto=%d len=%d  %s -> %s }\n",
        (ip->verIhl >> 4), ihl, ip->ttl, ip->protocol,
        ntohs(ip->totalLen), src, dst);
}

int main() {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // IPPROTO_ICMP  o raw socket  f  " 
    //  ?  <   <o ICMP  -  "+ Z~  o "o O  "~    ~ , OS ? IP  -  "   .z -   T  < .
    //  ^~ <   <o -  S" IP  -  "   .   "   O  ,    "  -  ~  < .
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        printf("socket()  <  O : %d   ?'   ?  z   O .o o  o  <  -? .~ "  s"\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    DWORD timeout = RECV_TIMEOUT_MS;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, TARGET_IP, &dest.sin_addr);

    WORD pid = (WORD)GetCurrentProcessId();

    printf("PING %s  payload=%d bytes\n", TARGET_IP, PAYLOAD_SIZE);
    printf("%-6s %-16s %-5s %-5s %s\n", "seq", "from", "type", "TTL", "RTT(ms)");
    printf("----------------------------------------------\n");

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    int received = 0;
    for (int seq = 1; seq <= PING_COUNT; ++seq) {
        // ICMP Echo Request  O  ,    
        char pkt[sizeof(IcmpHeader) + PAYLOAD_SIZE]{};
        auto* icmp    = reinterpret_cast<IcmpHeader*>(pkt);
        icmp->type    = 8;
        icmp->code    = 0;
        icmp->id      = htons(pid);
        icmp->seq     = htons((WORD)seq);
        icmp->checksum = 0;
        for (int i = 0; i < PAYLOAD_SIZE; ++i)
            pkt[sizeof(IcmpHeader) + i] = (char)('A' + i % 26);
        //    "  ?  -  "+ Z~  o "o  "    O? f  o  o  ^ ? ? -   " , 
        icmp->checksum = internetChecksum(pkt, sizeof(pkt));

        LARGE_INTEGER t0, t1;
        QueryPerformanceCounter(&t0);

        if (sendto(sock, pkt, sizeof(pkt), 0, (sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR) {
            printf("sendto  <  O : %d\n", WSAGetLastError());
            continue;
        }

        //  ^~ <   " : OS ? IP  -  " O ?   .  .  "o  O  ? < 
        char buf[1024]{};
        sockaddr_in from{};
        int fromLen = sizeof(from);
        int bytes = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
        QueryPerformanceCounter(&t1);

        if (bytes == SOCKET_ERROR) {
            printf("%-6d %-16s %s\n", seq, "-", " f? z" ." >f");
            continue;
        }

        // IP  -  "    O    "^ >  -  .  ICMP  -  " ?  ,~ ~  < 
        auto* ip  = reinterpret_cast<IpHeader*>(buf);
        int   ihl = (ip->verIhl & 0x0F) * 4;
        if (bytes < ihl + (int)sizeof(IcmpHeader)) continue;

        auto*  reply = reinterpret_cast<IcmpHeader*>(buf + ihl);
        //  s   ?   ,  ping -   O? .o reply  ?  T.  ( <    "" o "  S  ~ ICMP "  ^~ <    ^~  z^ O)
        if (ntohs(reply->id) != pid) { --seq; continue; }

        double rtt = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / freq.QuadPart;
        char fromStr[16];
        inet_ntop(AF_INET, &from.sin_addr, fromStr, sizeof(fromStr));

        printf("%-6d %-16s %-5d %-5d %.2f\n",
            seq, fromStr, reply->type, ip->ttl, rtt);
        printIpHeader(ip);
        ++received;
        Sleep(1000);
    }

    printf("\n--- %s ping  ?  " ---\n", TARGET_IP);
    printf(" " ? =%d   ^~ < =%d   ?  < =%d\n", PING_COUNT, received, PING_COUNT - received);

    closesocket(sock);
    WSACleanup();
    return 0;
}
