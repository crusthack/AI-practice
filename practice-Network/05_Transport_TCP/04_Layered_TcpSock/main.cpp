// 04_Layered_TcpSock: TcpSock  f  fo   <      ?"  -  "  "o "/    -  S 
//  <  -?: 04_Layered_TcpSock.exe server  /  04_Layered_TcpSock.exe client
//
//  ?     S :
//   1)  f  fo  "   o : CLOSED ?'LISTEN ?'SYN_RCVD ?'ESTABLISHED ?'FIN_WAIT_1 ?'...
//   2) 3-way  .  "o .   : SYN / SYN+ACK / ACK
//   3)    "   " ? : PSH+ACK / ACK (stop-and-wait)
//   4) 4-way teardown: FIN+ACK / ACK / FIN+ACK / ACK
#include "tcp_sock.hpp"
#include <vector>

static constexpr USHORT PORT = 27019;

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o ": passiveOpen  ?' recv   ""  ?' close
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runServer() {
    printf(" .  .  .  TcpSock  "o " (  S  %u)  .  .  . \n\n", PORT);

    TcpSock srv;
    // passiveOpen  ,  ? -  "o LISTEN  ?' SYN_RCVD  ?' ESTABLISHED  O ?  " -? .  <^ < 
    if (!srv.passiveOpen(PORT)) {
        printf(" -    ^~   <  O \n");
        return 1;
    }
    printf("\n -    ^~   T" O!    "   ^~ <   O? ...\n\n");

    char buf[CTCP_MSS + 4]{};
    int  totalBytes = 0;

    while (true) {
        int r = srv.recv(buf, CTCP_MSS);
        if (r <= 0) {
            printf("\n[ "o "]  -    . O  <  ~   ^~ <  (recv=%d)\n", r);
            break;
        }
        buf[r] = '\0';
        totalBytes += r;
        printf("[ "o "]  ^~ <  %d bytes: \"%.*s\"\n", r, r < 60 ? r : 60, buf);
    }

    printf("\n[ "o "]    ^~ < : %d bytes\n", totalBytes);
    printf("[ "o "] close()  ~  o  ?'  ^~ T  . O\n");
    srv.close();
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S : activeOpen  ?'  " <o ? 3 o  " ?   ?' close
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
static int runClient() {
    printf(" .  .  .  TcpSock     -  S   ?' 127.0.0.1:%u  .  .  . \n\n", PORT);

    TcpSock cli;
    // activeOpen  ,  ?: SYN_SENT  ?' SYN_RCVD  ?' ESTABLISHED
    if (!cli.activeOpen("127.0.0.1", PORT)) {
        printf(" -    <  O \n");
        return 1;
    }
    printf("\n -    ^~   T" O!    "   " ?   <o z'...\n\n");

    //  "? "?  <    "    S   " <o ?  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    const char* small = "Hello from TcpSock!";
    printf("[ ]  " ?  (small): \"%s\"\n", small);
    cli.send(small, (int)strlen(small));

    Sleep(100);

    //  "? "? MSS  ^   " <o ? ( z  T  " . )  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    int bigLen = CTCP_MSS + 200;
    std::vector<char> big(bigLen + 1);
    for (int i = 0; i < bigLen; ++i) big[i] = (char)('a' + i % 26);
    big[bigLen] = '\0';
    printf("[ ]  " ?  (big %d bytes, MSS=%d  ?' %d  "    S ):\n",
           bigLen, CTCP_MSS, (bigLen + CTCP_MSS - 1) / CTCP_MSS);
    cli.send(big.data(), bigLen);

    Sleep(100);

    //  "? "?  "  "^     "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    const BYTE binary[] = { 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD };
    printf("[ ]  " ?  (binary %d bytes)\n", (int)sizeof(binary));
    cli.send(binary, sizeof(binary));

    Sleep(200);

    //  "? "?  S  T close  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n[ ] close()  ~  o  ?'  S  T  . O\n");
    cli.close();
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
