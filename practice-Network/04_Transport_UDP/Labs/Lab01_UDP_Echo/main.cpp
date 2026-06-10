//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Lab01_UDP_Echo  ?" UDP  -  "  "o " /     -  S 
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  <  -?   .:
//    "o ": Lab01_UDP_Echo.exe server
//       -  S : Lab01_UDP_Echo.exe client [ "o "IP]  (   ': 127.0.0.1)
//
//  .T S    'o:
//   1. UDP  ?O " ~  f  .   : socket  ?' bind( "o " O)  ?' sendto/recvfrom  ?' closesocket
//   2. TCP T? ~  .  <     :
//      -  -  (connect)   -? <   ?'  .  "o .     ~  " -  "o  -? O
//      -  " <o ?   " ?    o <   ?' sendto 1 ^ = recvfrom 1 ^
//      -  O  ,   ?  < ,  ^o "o  '  " ?o,  '   ^~ <   ? S  (  Lab -  "o S"  o   ""      ~  .^   -  , )
//      -  -  " ?  z' <  (TCP 20~60B vs UDP 8B)  ?'  ?O Y?  ^ ^  " ?  -   o  
//   3. recvfrom  o  o  ?  <  z    ?O   .O ." ,  S"   .
//   4.  O o "o  S  S / ? <   S  S  o  T. z  .~ S"   ~  o . 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 

#include "winsock_util.hpp"
#include <cstring>
#include <cstdio>

//  "? "?   o   f  ^~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static constexpr USHORT PORT     = 9002;
static constexpr int    BUF_SIZE = 1024;

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  TCP vs UDP  .  <   "  'o (  " )
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   .               ", TCP (SOCK_STREAM)       ", UDP (SOCK_DGRAM)
//   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   -    "  .        ", connect()  ." s"          ",  ^ ." s" (connectionless)
//   " <o ?   "      ",  -? O ( "  S   S  S  )    ",  z^ O (   "   z   <  o"   )
//   <   "            ",  z  " ? ,  ^o "o   z        ",   z   -? O ( ?  < / ^o "o '  " ?o  ? S )
//   -  "           ",  o ?O 20B               ", 8B
//   ~  z   o -         ",  z^ O                    ",  -? O
//   s  "             ", HTTP, FTP, SSH         ", DNS,  O z",  S  S   , VoIP

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// runServer()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static void runServer() {
    printf("=== UDP  -  "  "o "  <o z' (  S  %u) ===\n", PORT);
    printf("    UDP  S"  -    f  fo ?  -? o  ? o accept()  ?  -? < .\n");
    printf("    recvfrom()  o  o  ?  <  z    ?O    ^  OO .. .o < .\n\n");

    //  "? "? step 1: UDP  ?O "  f  "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // SOCK_DGRAM   :    "   z   ?O " (UDP)
    // IPPROTO_UDP  : UDP  "" o ?  o  . <o
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("[socket]  <  O : %s\n", errStr().c_str());
        return;
    }
    printf("[ "o "] UDP  ?O "  f  "   T" O\n");

    //  "? "? step 2: bind()  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    //  "o " S"     -  S  ?    "     ,   ^~  z^ "    .   S  -   "  "  .o < .
    //     -  S  S" bind  -?  sendto    "   OS  ?  z" <o   S    z  T  .  <  .o < .
    sockaddr_in serverAddr = makeAddr(nullptr, PORT);
    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[bind]  <  O : %s\n", errStr().c_str());
        closesocket(sock);
        return;
    }
    printf("[ "o "] bind  T" O  ?" 0.0.0.0:%u\n\n", PORT);

    //  "? "? step 3: recvfrom / sendto   ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // recvfrom():    "    > o   "o  T <o -   ?  <  z  ~ (IP,   S ) "  .O ." ,  < .
    //    ?'     ?O    O? o sendto  -   ,  s  .   -  " .o < .
    // UDP  S"  -     -? o  ? o  -  Y      -  S    T    "" -  "o  ~   ? S .
    char buf[BUF_SIZE];
    int  msgCount = 0;

    while (true) {
        sockaddr_in clientAddr{};
        int         addrLen = sizeof(clientAddr);

        // recvfrom:  " o ,   ~  o (   "  ?  ~   .O O ?  O? )
        //  ~ T~ ' > 0  :  ^~ <  .o    "   z  ~  "  S   ^~
        //  ~ T~ ' == 0 : UDP  -  "o S" 0 "  S     "   z  (  ~  -? O)
        //  ~ T~ ' < 0  :  ~  ~
        int recvBytes = recvfrom(sock, buf, BUF_SIZE - 1, 0,
                                 (sockaddr*)&clientAddr, &addrLen);
        if (recvBytes == SOCKET_ERROR) {
            printf("[recvfrom]  <  O : %s\n", errStr().c_str());
            break;
        }
        buf[recvBytes] = '\0';

        //  ?  <  z    ?O  o 
        char ipBuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
        ++msgCount;
        printf("[ "o "] #%d  ^~ <  %d bytes from %s:%u  ?' \"%s\"\n",
               msgCount, recvBytes, ipBuf, ntohs(clientAddr.sin_port), buf);

        // "exit"  " <o ?   > o    "o "  . O ( .O S  S   Z  ~)
        if (strncmp(buf, "exit", 4) == 0) {
            printf("[ "o "] exit  .   ^~ < .  . O .  <^ < .\n");
            break;
        }

        // sendto():  S  . (IP,   S ) o    "   z   " ? 
        // UDP  S"  -     -? o  ? o   ^    ?   ?O   ? . .  .   .o < .
        // TCP send()  T?  <      "   z   <  o" o  " <  o <   ?"   " ?    o < .
        int sentBytes = sendto(sock, buf, recvBytes, 0,
                               (sockaddr*)&clientAddr, addrLen);
        if (sentBytes == SOCKET_ERROR) {
            printf("[sendto]  <  O : %s\n", errStr().c_str());
            continue;  //  .o  O  ,   <  O  .  "  " ?   "o " S  (UDP  S  " )
        }
        printf("[ "o "]  -  "  T" O (%d bytes  ?' %s:%u)\n\n",
               sentBytes, ipBuf, ntohs(clientAddr.sin_port));
    }

    //  "? "? step 4:  ?O "  <    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // UDP  S"  -     -? o  ? o shutdown()  ?  ~   -? < . closesocket()  O  ~  o.
    closesocket(sock);
    printf("[ "o "]  . O\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// runClient()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static void runClient(const char* serverIp) {
    printf("=== UDP  -  "     -  S  ( "o ": %s:%u) ===\n", serverIp, PORT);
    printf("      z  -   z.   >" Enter. \"exit\"  z.   <o  "o " "  .  ~  . O.\n\n");

    //  "? "? step 1: UDP  ?O "  f  "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    //     -  S  S" bind()  -?   ?O " "  f  "  .~  O  .o < .
    // sendto    ~ O  ~  o .~ S"  ^o " OS  ?  z" <o   S (ephemeral port)   z  T   . .o < .
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("[socket]  <  O : %s\n", errStr().c_str());
        return;
    }
    printf("[    -  S ] UDP  ?O "  f  "   T" O\n\n");

    //  "o "   ?O  ? " (  ^ sendto  -   "~  < )
    sockaddr_in serverAddr = makeAddr(serverIp, PORT);
    int         addrLen    = sizeof(serverAddr);

    //  ^~ <   f? z" ." >f  "  .: recvfrom   3 ^  .^ -   ' <   -? o   WSAETIMEDOUT  ~ T~
    // UDP  S"  O  ,   ?  <   <o  ~  >  z^ block    ^~  z^ o  ? o  f? z" ." >f   ' s" .~ < .
    setRecvTimeout(sock, 3000);  // 3000ms

    //  "? "? step 2: sendto / recvfrom   ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    char sendBuf[BUF_SIZE];
    char recvBuf[BUF_SIZE];
    int  seqNo = 0;

    while (true) {
        printf("> ");
        fflush(stdout);

        if (!fgets(sendBuf, sizeof(sendBuf), stdin)) break;

        //  o -?  o 
        size_t len = strlen(sendBuf);
        if (len > 0 && sendBuf[len - 1] == '\n') sendBuf[--len] = '\0';
        if (len > 0 && sendBuf[len - 1] == '\r') sendBuf[--len] = '\0';
        if (len == 0) continue;

        ++seqNo;

        // sendto:    "   z  1 o   "o " o  " ? 
        //  ' s": UDP  S"  " <o ?   " ?    o < .
        //    ?' sendto(100B)  .~  recvfrom  "  . T. z^ 100B  o  O ." ~  < .
        //    ?' TCP  ~ Y   -  Y   ^ -       o ? ?  .S S" < .
        int sentBytes = sendto(sock, sendBuf, (int)len + 1, 0,
                               (sockaddr*)&serverAddr, addrLen);
        if (sentBytes == SOCKET_ERROR) {
            printf("[sendto]  <  O : %s\n", errStr().c_str());
            break;
        }
        printf("[    -  S ] #%d  " ?  %d bytes\n", seqNo, sentBytes);

        // recvfrom:  "o " ~  -  "  ' <   O? 
        //  f? z" ." >f(3s)  ,  -   ~  ?  .S o   WSAETIMEDOUT  ~  ~  ~ T~
        sockaddr_in fromAddr{};
        int         fromLen = sizeof(fromAddr);

        int recvBytes = recvfrom(sock, recvBuf, BUF_SIZE - 1, 0,
                                 (sockaddr*)&fromAddr, &fromLen);
        if (recvBytes == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {
                // UDP  S  " :  O  ,   ?  <   <o  ' <    .^  ~   ^~  z^ < .
                printf("[    -  S ] #%d  f? z" ." >f  ?"  O  ,    ?  <  ~ -^  ,~  "o " ?  ' <   -? O\n\n", seqNo);
                continue;  //  <  O  " <o ?  z.  o  o  " ? 
            }
            printf("[recvfrom]  <  O : %s\n", errStr(err).c_str());
            break;
        }
        recvBuf[recvBytes] = '\0';
        printf("[    -  S ] #%d  -  "  ^~ <  %d bytes: \"%s\"\n\n", seqNo, recvBytes, recvBuf);

        if (strncmp(sendBuf, "exit", 4) == 0) break;
    }

    //  "? "? step 3:  ?O "  <    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    closesocket(sock);
    printf("[    -  S ]  . O\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// main()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
int main(int argc, char* argv[]) {
    WsaInit wsa;  // WSAStartup(2.2) RAII

    if (argc < 2) {
        printf(" ,  s  .: %s server | client [ "o "IP]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        runServer();
    } else if (strcmp(argv[1], "client") == 0) {
        const char* ip = (argc >= 3) ? argv[2] : "127.0.0.1";
        runClient(ip);
    } else {
        printf("[ ~  ~]  .O  ^~  -? S"   "o: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
