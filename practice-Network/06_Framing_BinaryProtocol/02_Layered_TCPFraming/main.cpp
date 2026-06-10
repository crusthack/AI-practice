// 02_Layered_TCPFraming: TCP S"  "  S   S  S    ?"  " <o ?   " ?  -? < .
//   '  "  "^   "" ^    ^  -     ~" .   " <o ?  <  o"  ?  <  "  O "  <^ < .
//  <  -?: 02_Layered_TCPFraming.exe server  /  02_Layered_TCPFraming.exe client
//  .  <   ? : recv() S"  s"  .o  "  S   <    O  ~ T~ .   ^~  z^ <  (short read).
//             . T. z^ N "  S     S" recvExact()   "" ?  "" ^   ~  .  <  z. <^ < .
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

static constexpr USHORT PORT     = 27017;
static constexpr WORD   MAGIC    = 0xBEEF;
static constexpr int    MAX_BODY = 1024;

//  "? "?  "" ^ z"    "o  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
enum class Cmd : WORD {
    PING = 1,   // Heartbeat  s" 
    PONG = 2,   // Heartbeat  ' < 
    TEXT = 3,   //  .  S  S   " <o ?
    ACK  = 4,   //  ^~ <   T.  (+  -  "  " "")
    BYE  = 5,   //  -    . O
};

//  "? "?  "" ^ z"  -  " (12 bytes)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// [magic:2][cmd:2][flags:2][seq:2][bodyLen:4]
#pragma pack(push, 1)
struct FrameHdr {
    WORD  magic;    // 0xBEEF  ?"  z~   .  o  ^~ <   "       ? s 
    WORD  cmd;      // Cmd ( "  S  >O   "  S   ~  ")
    WORD  flags;    //  ~^ . 
    WORD  seq;      //  <o ?? S   ^ ~ 
    DWORD bodyLen;  //  " ""    ( "  S  >O   "  S   ~  ")
};
#pragma pack(pop)

static constexpr int HDR = sizeof(FrameHdr);  // 12

static const char* cmdName(WORD c) {
    switch ((Cmd)c) {
    case Cmd::PING: return "PING";
    case Cmd::PONG: return "PONG";
    case Cmd::TEXT: return "TEXT";
    case Cmd::ACK:  return "ACK";
    case Cmd::BYE:  return "BYE";
    }
    return "?";
}

// TCP S"  S  S    ? o  .  f   >  .~ S"  "  S   ^~ O    "" .  "o   -  .   .  <^ < .
// recv()  .o  ^ o  o n "  S  ?  " ?  ~  ?  .S "  ^~  z^ S  <^ < .
static int recvExact(SOCKET s, char* buf, int n) {
    int total = 0;
    while (total < n) {
        int r = recv(s, buf + total, n - total, 0);
        if (r <= 0) return r == 0 ? 0 : SOCKET_ERROR;
        total += r;
    }
    return total;
}

static int buildFrame(char* buf, Cmd cmd, WORD seq,
                      const void* body = nullptr, int bodyLen = 0) {
    auto* h   = reinterpret_cast<FrameHdr*>(buf);
    h->magic   = htons(MAGIC);
    h->cmd     = htons((WORD)cmd);
    h->flags   = 0;
    h->seq     = htons(seq);
    h->bodyLen = htonl(bodyLen);
    if (body && bodyLen > 0) memcpy(buf + HDR, body, bodyLen);
    return HDR + bodyLen;
}

//  -  "  ?'  " ""  ^o "o o  . T. z^   S  <^ < .
//  ~ T~ ': true= "  , false= -    . O  ~  S"  ~  ~
static bool readFrame(SOCKET s, FrameHdr& hdr, char* bodyBuf, int& bodyLen) {
    if (recvExact(s, (char*)&hdr, HDR) <= 0) return false;
    if (ntohs(hdr.magic) != MAGIC) {
        printf("  !  z~  o magic: 0x%04X  ?"  -    S \n", ntohs(hdr.magic));
        return false;
    }
    bodyLen = (int)ntohl(hdr.bodyLen);
    if (bodyLen > MAX_BODY) {
        printf("  !  " ""  "^   : %d\n", bodyLen);
        return false;
    }
    if (bodyLen > 0 && recvExact(s, bodyBuf, bodyLen) <= 0) return false;
    return true;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o "
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runServer() {
    SOCKET ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(ls, (sockaddr*)&addr, sizeof(addr));
    listen(ls, 1);
    printf("[ "o "] TCP   S  %d  O?   '...\n", PORT);

    sockaddr_in cli{};
    int cLen = sizeof(cli);
    SOCKET conn = accept(ls, (sockaddr*)&cli, &cLen);
    char cIp[16];
    inet_ntop(AF_INET, &cli.sin_addr, cIp, sizeof(cIp));
    printf("[ "o "]  -  : %s\n\n", cIp);

    char buf[HDR + MAX_BODY];
    char body[MAX_BODY + 1]{};
    FrameHdr hdr{};
    int bodyLen = 0;
    WORD srvSeq = 0;

    while (readFrame(conn, hdr, body, bodyLen)) {
        Cmd  cmd = (Cmd)ntohs(hdr.cmd);
        WORD seq = ntohs(hdr.seq);
        body[bodyLen] = '\0';
        printf("[ "o "]  ?  [%s] seq=%u bodyLen=%d\n", cmdName((WORD)cmd), seq, bodyLen);

        int outLen;
        switch (cmd) {
        case Cmd::PING:
            outLen = buildFrame(buf, Cmd::PONG, ++srvSeq);
            send(conn, buf, outLen, 0);
            printf("[ "o "]  ?' [PONG] seq=%u\n", srvSeq);
            break;

        case Cmd::TEXT:
            printf("[ "o "]    ,  s : \"%s\"\n", body);
            outLen = buildFrame(buf, Cmd::ACK, ++srvSeq, body, bodyLen);
            send(conn, buf, outLen, 0);
            printf("[ "o "]  ?' [ACK+ -  "] seq=%u\n", srvSeq);
            break;

        case Cmd::BYE:
            printf("[ "o "]     -  S   . O  s"   ^~ <   ?"  -    <  \n");
            goto done;

        default:
            break;
        }
    }
done:
    closesocket(conn);
    closesocket(ls);
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runClient() {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sv{};
    sv.sin_family = AF_INET;
    sv.sin_port   = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &sv.sin_addr);

    if (connect(s, (sockaddr*)&sv, sizeof(sv)) == SOCKET_ERROR) {
        printf("connect  <  O : %d\n", WSAGetLastError());
        closesocket(s); return 1;
    }
    printf("[    -  S ]  "o "  -  \n\n");

    char buf[HDR + MAX_BODY];
    char body[MAX_BODY + 1]{};
    FrameHdr hdr{};
    int bodyLen = 0;
    WORD seq = 0;
    int outLen;

    //  "? "? PING / PONG  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("=== HEARTBEAT ===\n");
    outLen = buildFrame(buf, Cmd::PING, ++seq);
    send(s, buf, outLen, 0);
    printf("[    -  S ]  ?' [PING] seq=%u\n", seq);
    if (readFrame(s, hdr, body, bodyLen))
        printf("[    -  S ]  ?  [%s] seq=%u\n", cmdName(ntohs(hdr.cmd)), ntohs(hdr.seq));

    //  "? "? TEXT  " <o ?  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n=== MESSAGES ===\n");
    const char* msgs[] = {
        "Hello, binary framing!",
        "TCP delivers bytes, not messages",
        "recvExact() makes it reliable",
    };
    for (auto* m : msgs) {
        outLen = buildFrame(buf, Cmd::TEXT, ++seq, m, (int)strlen(m));
        send(s, buf, outLen, 0);
        printf("[    -  S ]  ?' [TEXT] \"%s\"\n", m);
        if (readFrame(s, hdr, body, bodyLen)) {
            body[bodyLen] = '\0';
            printf("[    -  S ]  ?  [%s] \"%s\"\n\n",
                cmdName(ntohs(hdr.cmd)), body);
        }
        Sleep(100);
    }

    //  "? "? BYE  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("=== DISCONNECT ===\n");
    outLen = buildFrame(buf, Cmd::BYE, ++seq);
    send(s, buf, outLen, 0);
    printf("[    -  S ]  ?' [BYE]\n");

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
