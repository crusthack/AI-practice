// 02_Layered_UdpSock: UdpSock   z~ S    
//  <  -?: 02_Layered_UdpSock.exe server  /  02_Layered_UdpSock.exe client
//
//  ?     S :
//   1)  z' ?  " <o ?  ?'  <  Z  T"  -?  1 o CUDP  O  ,  o  o  " < 
//   2)    " <o ?    ?' CUDP_MTU_BODY(512B)  <  o" o  z  T  <  Z  T"  >"  z   
//   3)    "   ~  ~  O  ,   ?'     o   o 
//   4)  <   "   -? O  T.   ?'  "o "   ' " -   . O .  "     -  S  S"  " ?   " ? 
#include "udp_sock.hpp"
#include <cstdio>
#include <cstring>

static constexpr USHORT PORT = 27018;

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o ":  ^~ <    ""
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runServer() {
    UdpSock s;
    if (!s.bindPort(PORT)) return 1;
    s.setRecvTimeout(15000);

    printf("[ "o "] CUDP   S  %d  O?  (15 ^  f? z" ." >f)\n\n", PORT);
    printf("%-20s %-6s %-10s %s\n", "From IP", "Port", "Bytes", "Preview");
    printf("%-20s %-6s %-10s %s\n", "--------", "----", "-----", "-------");

    char buf[CUDP_MAX_MSG + 1]{};
    char srcIp[16]{};
    USHORT srcPort = 0;
    int round = 0;

    while (true) {
        int r = s.recvFrom(srcIp, srcPort, buf, CUDP_MAX_MSG);
        if (r <= 0) {
            printf("\n[ "o "]  f? z" ." >f  ~  S"  ~  ~  ?"  . O\n");
            break;
        }
        buf[r] = '\0';
        ++round;

        //     :  o O? 40 z 
        char preview[41]{};
        int preLen = r < 40 ? r : 40;
        memcpy(preview, buf, preLen);
        for (int i = 0; i < preLen; ++i)
            if ((unsigned char)preview[i] < 32) preview[i] = '.';
        preview[preLen] = '\0';

        printf("[%d] %-20s %-6u %-10d \"%s%s\"\n",
               round, srcIp, srcPort, r,
               preview, r > 40 ? "..." : "");
    }
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S :  ?O ~./ O? ~./PING  " ? 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runClient() {
    UdpSock c;
    if (!c.open()) return 1;
    c.setRecvTimeout(3000);

    //  "? "? 1)  ?O ~.  " <o ? ( <  Z  T"  -? O)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("=== [1]  ?O ~.  " <o ? ( <  Z  T"  -? O) ===\n");
    {
        const char* msg = "Hello, custom UDP!";
        printf("[ ]  " ? : \"%s\" (%zu bytes)\n", msg, strlen(msg));
        c.sendTo("127.0.0.1", PORT, msg, (int)strlen(msg));
    }
    Sleep(200);

    //  "? "? 2)  O? ~.  " <o ? ( <  Z  T"  o f )  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n=== [2]  O? ~.  " <o ? (%d bytes,  <  Z  T"  o f ) ===\n",
           CUDP_MTU_BODY * 3 + 50);
    {
        int bigLen = CUDP_MTU_BODY * 3 + 50;  // 1586 bytes  ?' 4 o  <  Z 
        std::vector<char> big(bigLen);
        for (int i = 0; i < bigLen; ++i)
            big[i] = (char)('a' + i % 26);
        printf("[ ]  " ? : %d bytes  ?' CUDP_MTU_BODY=%d  ?' %d o  <  Z   ~^ f \n",
               bigLen, CUDP_MTU_BODY,
               (bigLen + CUDP_MTU_BODY - 1) / CUDP_MTU_BODY);
        c.sendTo("127.0.0.1", PORT, big.data(), bigLen);
    }
    Sleep(200);

    //  "? "? 3)  -  ?  10 o  " ?  ( <   "   -? O  T. )  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n=== [3]  -  ?  10 o  " ?  ( ^o "o/ ?  <   ? ) ===\n");
    for (int i = 1; i <= 10; ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Packet #%d  ?" unreliable UDP has no guarantee", i);
        printf("[ ]  " ?  #%d\n", i);
        c.sendTo("127.0.0.1", PORT, msg, (int)strlen(msg));
        Sleep(50);
    }

    //  "? "? 4) PING  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n=== [4] PING  " ?  ===\n");
    c.sendTo("127.0.0.1", PORT, "PING", 4, CudpType::PING);
    printf("[ ] PING  " ?   T" O\n");

    printf("\n[ ]   "   " ?   T" O\n");
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
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
