//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Lab02_SockOpts  ?"  ?O "  ~  .~  <  -~
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  <  -?   .:
//    "o ": Lab02_SockOpts.exe server
//       -  S : Lab02_SockOpts.exe client [ "o "IP]  (   ': 127.0.0.1)
//
//  .T S    'o:
//   1. getsockopt / setsockopt  ~  ,  s  .   ^ (SOL_SOCKET, IPPROTO_TCP)
//   2. SO_SNDBUF / SO_RCVBUF:   "   ,   ?  ^~ <   "       .
//   3. TCP_NODELAY: Nagle  .O   ~  " To "  T"  ?"  z' ?  O  ,   ? -   o 
//   4. SO_KEEPALIVE:  o  o   -   ~  f    -  ?  T. 
//   5. SO_RCVTIMEO:  " o ,  recv  -   f? z" ." >f  "  .
//   6. SO_REUSEADDR: TIME_WAIT  f  fo   S   ? <o  z  ,  s 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 

#include "winsock_util.hpp"
#include <cstdio>
#include <cstring>

static constexpr USHORT PORT     = 9003;
static constexpr int    BUF_SIZE = 1024;

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  getsockopt / setsockopt   ?   -  
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

// int  ~.  ?O "  ~  .~   
static int getSockOptInt(SOCKET s, int level, int optname, const char* label) {
    int val = 0;
    int len = sizeof(val);
    if (getsockopt(s, level, optname, (char*)&val, &len) == SOCKET_ERROR) {
        printf("  [getsockopt %s]  <  O : %s\n", label, errStr().c_str());
        return -1;
    }
    return val;
}

// int  ~.  ?O "  ~  .~  "  
static bool setSockOptInt(SOCKET s, int level, int optname, int val, const char* label) {
    if (setsockopt(s, level, optname, (char*)&val, sizeof(val)) == SOCKET_ERROR) {
        printf("  [setsockopt %s]  <  O : %s\n", label, errStr().c_str());
        return false;
    }
    return true;
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   <  -~ 1  ?" SO_SNDBUF / SO_RCVBUF
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// TCP  ?O " ?   "   "    .^ -   ?  <   " (SNDBUF) T?  ^~ <   " (RCVBUF)   - S" < .
//   SNDBUF: send()  o  "~     "   ACK  >   " O ?  z" <o   ?
//   RCVBUF:  f  O? ?   ,     "  ? recv()  -   ~ .    z   .O O ?  O?  .~ S"   "
//  "  ?  z' o    O? s  Y?  " ?   <o send/recv  ?  z    " o ,  o < .
//  "  ?     "     "  "  ? O  ~  Y?(throughput)   -  f  o < .
static void experiment1_Buffers(SOCKET s) {
    printf("\n "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("[ <  -~1] SO_SNDBUF / SO_RCVBUF\n");
    printf(" "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");

    //    '  T. 
    int sndBefore = getSockOptInt(s, SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF");
    int rcvBefore = getSockOptInt(s, SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF");
    printf("     ': SO_SNDBUF=%d B,  SO_RCVBUF=%d B\n", sndBefore, rcvBefore);

    //  "      64KB  o  ? 
    const int NEW_BUF = 65536;
    setSockOptInt(s, SOL_SOCKET, SO_SNDBUF, NEW_BUF, "SO_SNDBUF");
    setSockOptInt(s, SOL_SOCKET, SO_RCVBUF, NEW_BUF, "SO_RCVBUF");

    //  ?   >"  z  T.  (OS  ?  s"  ' "   O? o  "  ?  .S  2   "  o  o  ~    ~  .   ^~  z^ O)
    int sndAfter = getSockOptInt(s, SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF");
    int rcvAfter = getSockOptInt(s, SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF");
    printf("   ?  >": SO_SNDBUF=%d B,  SO_RCVBUF=%d B\n", sndAfter, rcvAfter);
    printf("   ?  Windows  S"  s"  ' " 2  o  ~    ~  .~ S"   s  ?  z^ S  <^ < .\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   <  -~ 2  ?" TCP_NODELAY (Nagle  .O   ~)
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// Nagle  .O   ~(RFC 896):
//    z' ?  O  ,  "  "   ."  .~ ,~ ~    "    S  o  .   "o   ,  S"  o  T".
//    "  S  >O   s  o   -  f   ?"  " ?  ? - ( o O? 200ms)  o f .
// TCP_NODELAY=1 (Nagle OFF):
//   send()  ? <o  O  ,  "  " ?   ?'  ? -   -? O  ?'  O z",  >    o - , RPC  -   ,  s 
// TCP_NODELAY=0 (Nagle ON,    '):
//    z' ?  O  ,  "   ."  " ?   ?'  O? -    ^ . ,  ~  Y?  -  f 
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//     -  S    -  "o 1 "  S   O  ,  64 o    ,  S"  <o " " Nagle ON/OFF  o  "  .o < .
static void experiment2_NoDelay_Measure(SOCKET s, bool enableNoDelay) {
    const char label[] = "TCP_NODELAY";
    int setting = enableNoDelay ? 1 : 0;
    setSockOptInt(s, IPPROTO_TCP, TCP_NODELAY, setting, label);

    int current = getSockOptInt(s, IPPROTO_TCP, TCP_NODELAY, label);
    printf("\n  TCP_NODELAY=%d (%s)\n", current,
           current ? "Nagle OFF  ?"  ? <o  " ? " : "Nagle ON   ?"  "    >"  " ? ");

    // 1 "  S   O  ,  64 o  " ? ,     <o "   .
    HiResTimer timer;
    timer.reset();
    const char data = 'x';
    for (int i = 0; i < 64; ++i) {
        if (send(s, &data, 1, 0) == SOCKET_ERROR) {
            printf("  [send]  <  O : %s\n", errStr().c_str());
            break;
        }
    }
    double ms = timer.elapsedMs();
    printf("  1 "  S   O  ,  64 o  " ?   ?' %.3f ms\n", ms);
    printf("  (Nagle ON     .  200ms  ,  T , Nagle OFF      ~ 0ms)\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   <  -~ 3  ?" SO_KEEPALIVE
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// TCP  -   ?  z  <o "    "   " ?    -? -  "  o  ? o < .
//  f  O?    " . f   . O( " >    < ,  "  S  >O   <  ^) .~     ?    < .
// SO_KEEPALIVE=1  "  .  <o:
//   OS  ?   .  <o "(   2 <o ")   f   o  o   f  fo   keepalive  f  (probe) "  z  T  " ? .
//    f   -   ' <   -? o    -   "  S  recv/send  -  WSAECONNRESET  ~ T~.
//       f?   S"  ^ ? S  S   KeepAliveTime/KeepAliveInterval  -  "o   ..
static void experiment3_KeepAlive(SOCKET s) {
    printf("\n "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("[ <  -~3] SO_KEEPALIVE\n");
    printf(" "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");

    int before = getSockOptInt(s, SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE");
    printf("   "  .  ": SO_KEEPALIVE=%d\n", before);

    setSockOptInt(s, SOL_SOCKET, SO_KEEPALIVE, 1, "SO_KEEPALIVE");

    int after = getSockOptInt(s, SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE");
    printf("   "  .  >": SO_KEEPALIVE=%d\n", after);
    printf("   ?     keepalive    S"  .  2 <o ".  <  <o "   ? ?  ." s" .~ \n");
    printf("     WSAIoctl(SIO_KEEPALIVE_VALS)  o ms  <  o"  "  .   ? S  .  <^ < .\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   <  -~ 4  ?" SO_RCVTIMEO
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//     " o ,  recv  S"    "  ?  ~   .O O ?   .o  O?  .o < .
// SO_RCVTIMEO    "  . .~   ? . .o  ?  ^  >" WSAETIMEDOUT  ~  ~ o  ~ T~ o < .
//  To s :  "o "  ' <  SLA   z ,     -  S  hang   ?,  z  -    o    ~"
static void experiment4_RecvTimeout(SOCKET s) {
    printf("\n "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("[ <  -~4] SO_RCVTIMEO  ?"  " o ,  recv  f? z" ." >f\n");
    printf(" "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");

    DWORD timeout = 2000;  // 2 ^
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
                   (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        printf("  [setsockopt SO_RCVTIMEO]  <  O : %s\n", errStr().c_str());
        return;
    }
    printf("  SO_RCVTIMEO = %u ms  "  .  T" O\n", timeout);
    printf("  2 ^  .^ -     "  ?  ~  ?  .S o   recv  ? WSAETIMEDOUT  ~ T~\n");

    //  <  o  f? z" ." >f  o  o ( "o "/    -  S    '    ,  ?  .S ?  f  fo)
    printf("  recv()  ~  o  ?"  f? z" ." >f  O?   '...\n");
    HiResTimer timer;
    timer.reset();

    char dummy[16];
    int r = recv(s, dummy, sizeof(dummy), 0);
    double elapsed = timer.elapsedMs();

    if (r == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
        printf("    : WSAETIMEDOUT (%.0f ms   )  ?"  f? z" ." >f  . f   T z'!\n", elapsed);
    } else if (r > 0) {
        printf("    :    "   ^~ <   (%d bytes, %.0f ms)\n", r, elapsed);
    } else {
        printf("    :  ~  ~ %s\n", errStr().c_str());
    }

    //  f? z" ." >f  .  o (  >"  <  -~ "  o" . )
    timeout = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   <  -~ 5  ?" SO_REUSEADDR
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// TCP  -   " closesocket  .~  OS  S"  z  <o TIME_WAIT  f  fo   o  ? .o < .
//   TIME_WAIT   z    o :
//     1)  ? -   "   O  ,    f^  -     ~  z  ~ ?  .S "  (2*MSL  O? )
//     2)  f  O?   ACK      > .~ "  .O  z  " ?  -   ' <  .~   o" . 
// SO_REUSEADDR=1  "  .  <o: TIME_WAIT  f  fo    S    ? <o  z  ,  s   ? S .
//  "o "  o o/ .O S  S   <o  ." ^~ (Ctrl+C  >"  " o  z  <o z'  ? S )
static void experiment5_ReuseAddr() {
    printf("\n "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("[ <  -~5] SO_REUSEADDR  ?" TIME_WAIT   S   z  ,  s \n");
    printf(" "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");

    //  ?O " A: SO_REUSEADDR  -?  bind
    SOCKET sockA = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr = makeAddr(nullptr, PORT);
    if (bind(sockA, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("  [bind A]  <  O : %s\n", errStr().c_str());
        closesocket(sockA);
        return;
    }
    printf("   ?O "A bind  "   (  S  %u)\n", PORT);

    //  ?O " A  <   (TIME_WAIT  " z.  ? S  " )
    closesocket(sockA);
    printf("   ?O "A closesocket  T" O\n");

    //  ?O " B: SO_REUSEADDR  o  T ?   S  -   ? <o  z  "  " 
    SOCKET sockB = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int reuse = 1;
    setSockOptInt(sockB, SOL_SOCKET, SO_REUSEADDR, reuse, "SO_REUSEADDR");

    if (bind(sockB, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("  [bind B]  <  O : %s (TIME_WAIT  '   ^~  z^ O)\n", errStr().c_str());
    } else {
        printf("   ?O "B bind  "    ?" SO_REUSEADDR  o   S  %u  ? <o  z  ,  s !\n", PORT);
    }
    closesocket(sockB);

    printf("   ?  netstat -an | findstr %u  o TIME_WAIT  f  fo   T.  .   "  s".\n", PORT);
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  runServer()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static void runServer() {
    printf("=== Lab03  ?O "  ~  .~  "o " (  S  %u) ===\n\n", PORT);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("[socket]  <  O : %s\n", errStr().c_str());
        return;
    }

    // SO_REUSEADDR  S" bind  " -   "  . .  .   .o < 
    setSockOptInt(listenSock, SOL_SOCKET, SO_REUSEADDR, 1, "SO_REUSEADDR");

    sockaddr_in serverAddr = makeAddr(nullptr, PORT);
    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[bind]  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    if (listen(listenSock, 5) == SOCKET_ERROR) {
        printf("[listen]  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    printf("[ "o "]     -  S   O?   '...\n");

    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);
    SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
    if (clientSock == INVALID_SOCKET) {
        printf("[accept]  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    char ipBuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
    printf("[ "o "]     -  S  %s:%u  -   \n\n", ipBuf, ntohs(clientAddr.sin_port));

    //  "o "    <  -~
    experiment1_Buffers(clientSock);
    experiment3_KeepAlive(clientSock);
    //  <  -~4:  "o " S" recv  f? z" ." >f     ? <o WSAETIMEDOUT  "  o  o
    experiment4_RecvTimeout(clientSock);
    experiment5_ReuseAddr();  //  " "  ?O " o  o  <  -~

    //     -  S  ?   ,  64*2 o  O  , (Nagle ON/OFF)  "  <  ^o  ^~ < 
    printf("\n[ "o "]     -  S  ~ TCP_NODELAY  O  ,   ^~ <   O?   '...\n");
    char buf[BUF_SIZE];
    int total = 0;
    //  "blocking  -?   " <  z^  "   ^~ < 
    setRecvTimeout(clientSock, 3000);
    while (true) {
        int n = recv(clientSock, buf, sizeof(buf), 0);
        if (n <= 0) break;
        total += n;
    }
    printf("[ "o "]  ^~ <    %d bytes (Nagle  <  -~  O  , )\n", total);

    shutdown(clientSock, SD_BOTH);
    closesocket(clientSock);
    closesocket(listenSock);
    printf("\n[ "o "]  . O\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  runClient()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static void runClient(const char* serverIp) {
    printf("=== Lab03  ?O "  ~  .~     -  S  ( "o ": %s:%u) ===\n\n", serverIp, PORT);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("[socket]  <  O : %s\n", errStr().c_str());
        return;
    }

    sockaddr_in serverAddr = makeAddr(serverIp, PORT);
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[connect]  <  O : %s\n", errStr().c_str());
        closesocket(sock);
        return;
    }
    printf("[    -  S ]  "o "  -    "  \n");

    //     -  S     <  -~ 1, 3
    experiment1_Buffers(sock);
    experiment3_KeepAlive(sock);

    //  <  -~ 2: TCP_NODELAY  s    ?" Nagle ON/OFF  o 1 "  S   O  ,  64 o  " ? 
    printf("\n "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("[ <  -~2] TCP_NODELAY  ?" Nagle  .O   ~ ON vs OFF\n");
    printf(" "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  "  " \n");
    printf("  [Nagle ON  ?"    ']\n");
    experiment2_NoDelay_Measure(sock, false);  // TCP_NODELAY=0 (Nagle ON)

    printf("  [Nagle OFF  ?" TCP_NODELAY=1]\n");
    experiment2_NoDelay_Measure(sock, true);   // TCP_NODELAY=1 (Nagle OFF)

    //  -    . 
    shutdown(sock, SD_SEND);
    closesocket(sock);
    printf("\n[    -  S ]  . O\n");
}

//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  main()
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
int main(int argc, char* argv[]) {
    WsaInit wsa;

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
