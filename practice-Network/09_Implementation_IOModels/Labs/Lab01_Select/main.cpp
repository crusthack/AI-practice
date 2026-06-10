//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Lab01_Select  ?" select()   ~ I/O  <  ' T"  -  "  "o "
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//
// [ <  -?   .]
//    "o ": Lab01_Select.exe server
//       -  S : Lab01_Select.exe client   ( -  Y   o  T <o  <  -?  ? S )
//
// [ .T S    'o]
//   1. select() ~  T z'  >  :  -  Y   ?O " "  <    S  ^ "o -  "o   <o
//   2. fd_set      ,  s  .: FD_SET, FD_CLR, FD_ISSET, FD_ZERO
//   3. select() ~  .  <   o . : FD_SETSIZE(   64)  .o "
//   4. timeout   z  NULL vs timeval{0,0}  ~   
//   5. select()   >" fd_set  z   "    ." s" .o   o 
//
// [select()  T z'  >  ]
//    "O "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? " 
//    ",   "    ?O "  ' . (master_set)  ?' select()  " z.              ",
//    ",    " :    ?O " ~  ^~ <   "  -     "  ?  z^ S" ?  T.           ",
//    ",    " :  ? " o  ?O "  ^~, read_set ?  ? " o  f O  ,  ?         ",
//    ",   . : FD_ISSET() o  o  -  -   ?O "  ?  T.   >"  ~             ",
//    "" "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "~
//
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 

#include "winsock_util.hpp"
#include <cstring>
#include <cstdlib>

//  "? "? "?  f  ^~  . ~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static constexpr USHORT PORT       = 9006;
static constexpr int    MAX_CLIENTS = 5;    //  T <o  ' ?   -^ s   o O?  ^~
static constexpr int    BUFSIZE    = 512;

//  "? "? "?  "   "  -   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
void runServer();
void runClient(int id);

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// main
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
int main(int argc, char* argv[]) {
    WsaInit wsa;   // RAII: WSAStartup / WSACleanup  z  T  ~ 

    if (argc < 2) {
        printf(" ,  s  .: %s server|client [    -  S  ^ ~ ]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        runServer();
    } else if (strcmp(argv[1], "client") == 0) {
        int id = (argc >= 3) ? atoi(argv[2]) : 1;
        runClient(id);
    } else {
        printf("  z   ~  ~: 'server'  ~  S" 'client'   ? . .~ "  s".\n");
        return 1;
    }
    return 0;
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//  "o "   ~"
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
void runServer() {
    printf("[ "o "] Lab06 select()  -  "  "o "  <o z'  ?"   S  %d\n", PORT);
    printf("[ "o "]  o O? %d     -  S   T <o  ~ \n", MAX_CLIENTS);

    //  "? "?   S   ?O "  f  "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("[ "o "]  ?O "  f  "   <  O : %s\n", errStr().c_str());
        return;
    }

    // SO_REUSEADDR:  "o "  z  <o z'  <o "  ?O  ,  s   '"  ~  ~   ?
    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

    //  "? "?  "  "o +   S   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    sockaddr_in addr = makeAddr(nullptr, PORT);   // INADDR_ANY
    if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[ "o "] bind  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        printf("[ "o "] listen  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    printf("[ "o "]  O?   ' ... (%s)\n", nowStr().c_str());

    //  "? "?     -  S   ?O "   -   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // clients[0..MAX_CLIENTS-1]:  -   o     -  S   ?O ", INVALID_SOCKET= ^  S  
    SOCKET clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; ++i) clients[i] = INVALID_SOCKET;
    int clientCount = 0;

    //  "? "? select()  "    ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    //
    //  ~.  .  <   o . : fd_set   ,    o 
    //   select() S"  ~  o  <o  " <  .o fd_set "   '  ^~ . .  <^ < .
    //     "   >" read_set -  S" " ? " o  ?O " O"  ,    ,~  ? S"  ? >O ' <^ < .
    //    "   "o   <o .   ?O "   (master_set) "  "  o   ? .~ ,
    //       "" ^ <  read_set = master_set  o  o   ,  .  "o select() -   "~  .   .  <^ < .
    //
    fd_set master_set;   //  .  f   o <   ?O "    "  o  ? .~ S"  ^ S  "   ' . 
    fd_set read_set;     // select() -   " <  .~ S"  z' -. s    ,  

    FD_ZERO(&master_set);
    FD_SET(listenSock, &master_set);  //   S   ?O " "   <o  O? f  -   " ?

    //  ~. FD_SETSIZE  .o "   " 
    //   Windows Winsock ~ FD_SETSIZE    ' ? 64 z. <^ < .
    //    ?, fd_set  .~ ,~ -   o O? 64 o  ?O " O  "    ? S  .  <^ < .
    //   (Unix/Linux "    1024  ? O  ?O "  ^ ~    ~    ~  ?  <  ")
    //    "  Z ?  -   "  ~  .~   WSAEventSelect, IOCP  "  "  ,  s  .  .   .  <^ < .
    printf("[ "o "]  ?  FD_SETSIZE = %d (fd_set  .~ ,~ <   o O?  ?O "  ^~)\n", FD_SETSIZE);

    while (true) {
        //  "? "? read_set "     "" ^ <  master_set o  o  z   "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        // select() ? read_set "   -  "    .O  -    ,  ?  ." ^~
        read_set = master_set;

        //  "? "? select()  ~  o  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        //
        //  ~. timeout   z  NULL vs timeval{0,0}   
        //   NULL         ?'  ? " o  ?O "   f    .O O ?   .o  O?  ( " o , )
        //   {0, 0}       ?'  ? <o   "  (     "o, CPU 100%   ~)
        //   {tv_sec,  s} ?'  ? .  <o " O   O?   >"  f? z" ." >f   " (0)
        //
        //  -   "o S" NULL o  "  . .   ^ ." s" .o CPU  ,  "   ? S  <^ < .
        timeval tv{};
        tv.tv_sec  = 5;   // 5 ^ ^ <  " O?   '..."  " <o ?  o  ( f    T.  s )
        tv.tv_usec = 0;

        // nfds: Unix -  "o S"  o O? fd  ^ ~ +1, Windows -  "o S"   <o ~ ? O  ? ? f  0  ,  s 
        int ready = select(0, &read_set, nullptr, nullptr, &tv);

        if (ready == SOCKET_ERROR) {
            printf("[ "o "] select  <  O : %s\n", errStr().c_str());
            break;
        }
        if (ready == 0) {
            //  f? z" ." >f:  ."   ?O " "  ? " ~ ?  .S O
            printf("[ "o "] [%s]  O?   ' ( -    ^~: %d)...\n", nowStr().c_str(), clientCount);
            continue;
        }

        //  "? "?   S   ?O "  ~ :  f^  -    ^~   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        if (FD_ISSET(listenSock, &read_set)) {
            sockaddr_in cAddr{};
            int cLen = sizeof(cAddr);
            SOCKET newSock = accept(listenSock, (sockaddr*)&cAddr, &cLen);

            if (newSock == INVALID_SOCKET) {
                printf("[ "o "] accept  <  O : %s\n", errStr().c_str());
            } else if (clientCount >= MAX_CLIENTS) {
                //  o O?     -  S   ^~  ^ :   ^
                printf("[ "o "]  o O?  -    ^~  ^ ,   ^: %s\n", addrStr(cAddr).c_str());
                const char* msg = " "o "   T"  f  fo z. <^ < .  z  <o  >"  <  <o  <o " .~ "  s".\r\n";
                send(newSock, msg, (int)strlen(msg), 0);
                closesocket(newSock);
            } else {
                //  ^  S   -   f^     -  S   "  
                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i] == INVALID_SOCKET) {
                        clients[i] = newSock;
                        ++clientCount;
                        FD_SET(newSock, &master_set);  //  ^ S  "   ' .  -   " ?
                        printf("[ "o "] [%s]     -  S   -  : %s ( S   %d,  ~" z   -    ^~: %d)\n",
                               nowStr().c_str(), addrStr(cAddr).c_str(), i, clientCount);
                        break;
                    }
                }
            }
        }

        //  "? "?     -  S   ?O "  ~ :  ^~ <     "  or  -    . O  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == INVALID_SOCKET) continue;

            // FD_ISSET:    ?O " -    "    "  ?  z^ S" ?  T. 
            if (!FD_ISSET(clients[i], &read_set)) continue;

            char buf[BUFSIZE]{};
            int n = recv(clients[i], buf, BUFSIZE - 1, 0);

            if (n <= 0) {
                // n == 0:     -  S  ?  . f   . O (graceful close)
                // n < 0:   ?O "  ~  ~
                if (n == 0) {
                    printf("[ "o "] [%s]     -  S   . f   . O ( S   %d,  ~" z   -    ^~: %d)\n",
                           nowStr().c_str(), i, clientCount - 1);
                } else {
                    printf("[ "o "] recv  ~  ~ ( S   %d): %s\n", i, errStr().c_str());
                }
                //  ?O "  . 
                FD_CLR(clients[i], &master_set);  //  ^ S  "   ' .  -  "o  o 
                closesocket(clients[i]);
                clients[i] = INVALID_SOCKET;
                --clientCount;
            } else {
                //  -  ":  > ?    "    O? o  O   f"
                buf[n] = '\0';
                printf("[ "o "] [%s]  S   %d  ?' \"%s\" (%d "  S )\n",
                       nowStr().c_str(), i, buf, n);

                //  -  "  " ?  (SendAll:  ? "  ?  <    ?)
                if (!SendAll(clients[i], buf, n)) {
                    printf("[ "o "] send  ~  ~ ( S   %d): %s\n", i, errStr().c_str());
                    FD_CLR(clients[i], &master_set);
                    closesocket(clients[i]);
                    clients[i] = INVALID_SOCKET;
                    --clientCount;
                }
            }
        }
    }

    //  "? "?  .   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    for (int i = 0; i < MAX_CLIENTS; ++i)
        if (clients[i] != INVALID_SOCKET) closesocket(clients[i]);
    closesocket(listenSock);
    printf("[ "o "]  . O\n");
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S    ~"
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
void runClient(int id) {
    printf("[    -  S -%d]  "o " %s:%d  -   -    <o "\n", id, "127.0.0.1", PORT);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("[    -  S -%d]  ?O "  f  "   <  O : %s\n", id, errStr().c_str());
        return;
    }

    sockaddr_in srvAddr = makeAddr("127.0.0.1", PORT);
    if (connect(sock, (sockaddr*)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR) {
        printf("[    -  S -%d] connect  <  O : %s\n", id, errStr().c_str());
        closesocket(sock);
        return;
    }
    printf("[    -  S -%d]  -    "  \n", id);

    // 5 o  " <o ?  0.5 ^  "  o  o  " ? 
    for (int m = 1; m <= 5; ++m) {
        char buf[BUFSIZE]{};
        int len = snprintf(buf, BUFSIZE, "Client-%d: msg#%d", id, m);

        printf("[    -  S -%d]  " ? : \"%s\"\n", id, buf);
        if (!SendAll(sock, buf, len)) {
            printf("[    -  S -%d] send  <  O : %s\n", id, errStr().c_str());
            break;
        }

        //  -  "  ^~ <  (  ,   "  S   ^~ O   . T. z^  > )
        char echo[BUFSIZE]{};
        if (!RecvExact(sock, echo, len)) {
            printf("[    -  S -%d] echo recv  <  O \n", id);
            break;
        }
        printf("[    -  S -%d]  -  "  ^~ < : \"%.*s\"\n", id, len, echo);

        // 0.5 ^  O?  ( -  Y      -  S   T <o  <  -?  <o  "o "  ? .~  T.   s  )
        Sleep(500);
    }

    closesocket(sock);
    printf("[    -  S -%d]  . O\n", id);
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// [ .T S   . ] select() vs  <   I/O   
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//
//                   o O?  ?O "    S  ^ "o  ." s"     "   .O    < 
//   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  select()          64 o         ^ ." s"          (fd_set  S  ")
//  WSAEventSelect    64 o         ^ ." s"           S   .  " 
//  IOCP               o .o  -? O    ." s"( >O )     T" O  O  ,   
//
// select() ~  ,  ?  T z':
//   1.  .   fd_set -   ?O "  "    >" select()  ~  o
//   2. OS   "    "   o   "   ?O " ~ I/O  ? "  f  fo    <o
//   3.  .~ ,~  "  ? " ~  select()   " , fd_set -   ? " o  ?O " O  ,  ?
//   4.  .  ? FD_ISSET o  o  -  -   ?O "  ? O(n) o  o  ^o sO .~   ~ 
//
//  <  :
//   -   ^ fd_set  z   "   ." s" (  "  ?' .    "    "    ,   o f )
//   - FD_SETSIZE=64  o .o
//   - O(n)  S  " o  o  ?O "  ^~ ?  S~ ^~   "  S   ? .~
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
