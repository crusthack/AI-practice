// 03_Layered_CustomUDP: UDP S"  <   "    -? <   ?'   '  <o ?? S   ^ ~  + ACK +  z  " ?  "   ~" .  <^ < .
//  <  -?: 03_Layered_CustomUDP.exe server  /  03_Layered_CustomUDP.exe client
//  .  <   ? : SYN  ?' SYN_ACK 3-way  o  ,   .  "o .   ,
//            DATA + ACK   T~,  f? z" ." >f  <o  z  " ? , FIN  . O   "
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

static constexpr USHORT PORT       = 27016;
static constexpr DWORD  MAGIC      = 0xCAFEBABE;
static constexpr DWORD  TIMEOUT_MS = 2000;
static constexpr int    MAX_RETRY  = 3;
static constexpr int    MAX_PAYLOAD = 512;

//  "? "?   S  .? RUDP  O  ,   f? z.  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
enum class PktType : BYTE {
    SYN     = 0,    //  -    s" 
    SYN_ACK = 1,    //  -    ^~ 
    DATA    = 2,    //    " 
    ACK     = 3,    //  T.   ' < 
    FIN     = 4,    //  -    . O  s" 
    FIN_ACK = 5,    //  -    . O  T. 
};

//  "? "?   S  .?  -  " (16 bytes)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
#pragma pack(push, 1)
struct RudpHdr {
    DWORD magic;    //  O  ,   <  " z  (0xCAFEBABE)
    BYTE  type;     // PktType
    BYTE  flags;    //  ~^ . 
    DWORD seq;      //  <o ?? S   ^ ~  ( . -" "" - )
    DWORD ackSeq;   // ACK  ^ ~    ( . -" "" - )
    WORD  len;      //  Z~  o "o    ( . -" "" - )
};
#pragma pack(pop)

static constexpr int HDR = sizeof(RudpHdr);  // 16

static const char* typeName(BYTE t) {
    switch ((PktType)t) {
    case PktType::SYN:     return "SYN";
    case PktType::SYN_ACK: return "SYN_ACK";
    case PktType::DATA:    return "DATA";
    case PktType::ACK:     return "ACK";
    case PktType::FIN:     return "FIN";
    case PktType::FIN_ACK: return "FIN_ACK";
    }
    return "UNKNOWN";
}

static int makePacket(char* buf, PktType type, DWORD seq, DWORD ackSeq,
                      const void* payload = nullptr, int payLen = 0) {
    auto* h   = reinterpret_cast<RudpHdr*>(buf);
    h->magic  = htonl(MAGIC);
    h->type   = (BYTE)type;
    h->flags  = 0;
    h->seq    = htonl(seq);
    h->ackSeq = htonl(ackSeq);
    h->len    = htons((WORD)payLen);
    if (payload && payLen > 0) memcpy(buf + HDR, payload, payLen);
    return HDR + payLen;
}

static bool isValid(const char* buf, int len) {
    if (len < HDR) return false;
    return ntohl(reinterpret_cast<const RudpHdr*>(buf)->magic) == MAGIC;
}

//  O  ,  "  " ?  .~   S  .  f? z. ~  ' <  "   <   <^ < .
//  f? z" ." >f  <o MAX_RETRY  O ?  z  " ?  .  <^ < .
static bool sendAndWait(SOCKET s, const char* pkt, int pktLen,
                        const sockaddr_in& dest,
                        PktType expected, RudpHdr& outHdr) {
    char recvBuf[HDR + MAX_PAYLOAD];
    sockaddr_in from{};
    int fromLen = sizeof(from);

    for (int attempt = 1; attempt <= MAX_RETRY; ++attempt) {
        sendto(s, pkt, pktLen, 0, (sockaddr*)&dest, sizeof(dest));
        auto* h = reinterpret_cast<const RudpHdr*>(pkt);
        printf("   ?' [%s] seq=%u  ( <o " %d/%d)\n",
            typeName(h->type), ntohl(h->seq), attempt, MAX_RETRY);

        int len = recvfrom(s, recvBuf, sizeof(recvBuf), 0, (sockaddr*)&from, &fromLen);
        if (len == SOCKET_ERROR) {
            printf("   ?   f? z" ." >f,  z  " ? \n");
            continue;
        }
        if (!isValid(recvBuf, len)) continue;

        auto* rh = reinterpret_cast<const RudpHdr*>(recvBuf);
        if ((PktType)rh->type == expected) {
            printf("   ?  [%s] seq=%u ack=%u\n",
                typeName(rh->type), ntohl(rh->seq), ntohl(rh->ackSeq));
            outHdr = *rh;
            return true;
        }
    }
    printf("   o-  " ?   <  O \n");
    return false;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o "
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runServer() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(s, (sockaddr*)&addr, sizeof(addr));

    printf("[ "o "] UDP   S  %d  O?   '...\n\n", PORT);

    char buf[HDR + MAX_PAYLOAD];
    sockaddr_in client{};
    int cLen = sizeof(client);

    //  "? "? 1 <  ": SYN  ^~ <   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    int len = recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&client, &cLen);
    if (!isValid(buf, len)) { closesocket(s); return 1; }
    auto* h = reinterpret_cast<RudpHdr*>(buf);
    if ((PktType)h->type != PktType::SYN) {
        printf("SYN "   O? -^ ? O [%s]  ^~ < \n", typeName(h->type));
        closesocket(s); return 1;
    }
    DWORD clientISN = ntohl(h->seq);
    printf("[ "o "]  ?  [SYN] seq=%u\n", clientISN);

    //  "? "? 2 <  ": SYN_ACK  ?  <   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    DWORD serverSeq = 1000;
    char reply[HDR];
    makePacket(reply, PktType::SYN_ACK, serverSeq, clientISN + 1);
    sendto(s, reply, HDR, 0, (sockaddr*)&client, cLen);
    printf("[ "o "]  ?' [SYN_ACK] seq=%u ack=%u\n", serverSeq, clientISN + 1);

    // DATA  ^~ <    "" -   f? z" ." >f   s  (    -  S  ?  ~^  ~  .S O  . O .  "  ^ " ?  .S " )
    DWORD to = 10000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));

    DWORD expectedSeq = clientISN + 1;
    printf("\n[ "o "]    "   ^~ <   O? ...\n");

    //  "? "? 3 <  ": DATA/FIN  ~    ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    while (true) {
        len = recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&client, &cLen);
        if (len == SOCKET_ERROR) { printf("[ "o "]  f? z" ." >f  ?"  . O\n"); break; }
        if (!isValid(buf, len)) continue;

        h = reinterpret_cast<RudpHdr*>(buf);
        DWORD  seq    = ntohl(h->seq);
        WORD   payLen = ntohs(h->len);
        PktType type  = (PktType)h->type;

        if (type == PktType::DATA) {
            if (seq == expectedSeq) {
                char payload[MAX_PAYLOAD + 1]{};
                if (payLen > 0) memcpy(payload, buf + HDR, payLen);
                printf("[ "o "]  ?  [DATA] seq=%u  \"%s\"\n", seq, payload);

                expectedSeq++;
                serverSeq++;
                char ack[HDR];
                makePacket(ack, PktType::ACK, serverSeq, expectedSeq);
                sendto(s, ack, HDR, 0, (sockaddr*)&client, cLen);
                printf("[ "o "]  ?' [ACK] ack=%u\n", expectedSeq);
            } else {
                //  '   O  , :  ^ ? ? ACK  z  " ?  (    -  S  ? ACK     > ?   s )
                printf("[ "o "]  ?  [DATA] seq=%u ( ' , expected=%u)  ?" ACK  z  " ? \n",
                    seq, expectedSeq);
                char ack[HDR];
                makePacket(ack, PktType::ACK, serverSeq, expectedSeq);
                sendto(s, ack, HDR, 0, (sockaddr*)&client, cLen);
            }
        } else if (type == PktType::FIN) {
            printf("[ "o "]  ?  [FIN] seq=%u\n", seq);
            serverSeq++;
            char finAck[HDR];
            makePacket(finAck, PktType::FIN_ACK, serverSeq, seq + 1);
            sendto(s, finAck, HDR, 0, (sockaddr*)&client, cLen);
            printf("[ "o "]  ?' [FIN_ACK]  ?"  -    . O\n");
            break;
        }
    }

    closesocket(s);
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runClient() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    DWORD to = TIMEOUT_MS;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port   = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    printf("[    -  S ] 127.0.0.1:%d  -   -    <o "\n\n", PORT);

    char pkt[HDR + MAX_PAYLOAD];
    int  pLen;
    RudpHdr respHdr{};
    DWORD mySeq = 100;

    //  "? "? 1 <  ": SYN  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("=== HANDSHAKE ===\n");
    pLen = makePacket(pkt, PktType::SYN, mySeq, 0);
    if (!sendAndWait(s, pkt, pLen, server, PktType::SYN_ACK, respHdr)) {
        printf(" -    <  O \n"); closesocket(s); return 1;
    }
    mySeq++;
    DWORD serverSeq = ntohl(respHdr.seq);
    printf("[    -  S ]  -    ^~  (serverISN=%u)\n\n", serverSeq);

    //  "? "? 2 <  ": DATA  " ?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    const char* messages[] = {
        "Hello, custom protocol!",
        "Sequence numbers make delivery reliable",
        "This is how TCP works under the hood",
    };

    printf("=== DATA TRANSFER ===\n");
    for (auto* msg : messages) {
        pLen = makePacket(pkt, PktType::DATA, mySeq, serverSeq + 1,
                          msg, (int)strlen(msg));
        if (!sendAndWait(s, pkt, pLen, server, PktType::ACK, respHdr)) break;
        mySeq++;
        Sleep(200);
    }

    //  "? "? 3 <  ": FIN  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n=== TEARDOWN ===\n");
    pLen = makePacket(pkt, PktType::FIN, mySeq, 0);
    sendAndWait(s, pkt, pLen, server, PktType::FIN_ACK, respHdr);
    printf("[    -  S ]  -    . O\n");

    closesocket(s);
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
int main(int argc, char* argv[]) {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup  <  O : %d\n", WSAGetLastError());
        return 1;
    }

    bool isServer = (argc < 2 || strcmp(argv[1], "server") == 0);
    int ret = isServer ? runServer() : runClient();

    WSACleanup();
    return ret;
}
