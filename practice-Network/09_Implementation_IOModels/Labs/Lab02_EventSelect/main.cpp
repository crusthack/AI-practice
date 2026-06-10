//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// Lab02_EventSelect  ?" WSAEventSelect   ~ I/O  <  ' T"  -  "  "o "
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//
// [ <  -?   .]
//    "o ": Lab02_EventSelect.exe server
//       -  S : Lab02_EventSelect.exe client   ( -  Y   o  T <o  <  -?  ? S )
//
// [ .T S    'o]
//   1. WSAEventSelect() ~  T z'  >  :  ?O "    S     "     S     T?  -  
//   2. WSAWaitForMultipleEvents() o  -  Y     S   ' signaled    S   O? 
//   3. WSAEnumNetworkEvents() o  -  -   "  S  >O     S  ?  o f  -^ S" ?  T. 
//   4. select() T? WSAEventSelect() ~          . 
//   5. WSA_MAXIMUM_WAIT_EVENTS(64)  o .o      o 
//
// [select vs WSAEventSelect  " ]
//    "O "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? " 
//    ",    "           select()              WSAEventSelect()          ",
//    ",   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?  ",
//    ",    <o   <        fd_set                 S   .  "  wait          ",
//    ",   o O?  ?O "       FD_SETSIZE(64)        WSA_MAXIMUM_WAIT_EVENTS   ",
//    ",  fd_set   ,         ""  ." s"            ^ ." s"                    ",
//    ",     S   . ~     read/write/except     FD_READ/WRITE/ACCEPT  "    ",
//    ",  O(n)  S  "       z^ O                   -? O(   S    '  ' )     ",
//    "" "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "~
//
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 

#include "winsock_util.hpp"
#include <cstring>
#include <cstdlib>

//  "? "? "?  f  ^~  . ~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static constexpr USHORT PORT        = 9007;
static constexpr int    MAX_CLIENTS = 5;
static constexpr int    BUFSIZE     = 512;

//  ~. WSA_MAXIMUM_WAIT_EVENTS  .o "   " 
//   WSAWaitForMultipleEvents() S"  o O? WSA_MAXIMUM_WAIT_EVENTS(=64) o ~
//      S   .  "  O  T <o -    <o .   ^~  z^ S  <^ < .
//     S" Windows   "  ~ WaitForMultipleObjects()  o .o(MAXIMUM_WAIT_OBJECTS=64) -  "o
//    "  o  f o  o,  "  Z ?  -   "  ~  .~   IOCP   ,  s  .  .   .  <^ < .
//      S  0:   S   ?O "    S ,    S  1~:     -  S     S 
static constexpr int    MAX_SOCKETS = 1 + MAX_CLIENTS;  //   S  +     -  S 

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
    printf("[ "o "] Lab07 WSAEventSelect  -  "  "o "  <o z'  ?"   S  %d\n", PORT);

    //  "? "?   S   ?O "  f  "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("[ "o "]  ?O "  f  "   <  O : %s\n", errStr().c_str());
        return;
    }

    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

    //  "? "?  "  "o +   S   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    sockaddr_in addr = makeAddr(nullptr, PORT);
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

    //  "? "? events[] + sockets[]   -   '   ?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    //
    //  ~.  .  <   "  ":    S    ~ 1:1   .'
    //   events[i]  ?" sockets[i]  S"  .  f   T ?    S     o  .  <^ < .
    //   WSAWaitForMultipleEvents()   "  ' -  "o    S    " o . 
    //   events[idx] -   O? ' .~ S" sockets[idx]   ? <o   "  ^~  z^ S  <^ < .
    //
    //      S  0:  .  f    S   ?O " (FD_ACCEPT   <o)
    //      S  1~MAX_CLIENTS:     -  S   ?O " (FD_READ | FD_CLOSE   <o)
    //
    WSAEVENT events [MAX_SOCKETS];
    SOCKET   sockets[MAX_SOCKETS];

    for (int i = 0; i < MAX_SOCKETS; ++i) {
        events [i] = WSA_INVALID_EVENT;
        sockets[i] = INVALID_SOCKET;
    }

    //  "? "?   S   ?O "    S   "    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    //
    // WSACreateEvent():  ^~ T   .<(manual-reset)    S   .  "   f  " 
    //  ^   f  fo: non-signaled
    events[0] = WSACreateEvent();
    if (events[0] == WSA_INVALID_EVENT) {
        printf("[ "o "] WSACreateEvent  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    sockets[0] = listenSock;

    // WSAEventSelect():  ?O "    S     "     S     T?  -  
    //   FD_ACCEPT:  f^  -    ^~   ? S  (  S   ?O " s )
    //   FD_READ  :    "   ^~ <   ? S 
    //   FD_CLOSE :  -    . O   ?
    //
    //  ~.  ' s": WSAEventSelect   ~  o .~   ?O "   z  T o  o  .  " o ,    "o ?   <^ < !
    //     >" recv(), accept()  "    ? <o   "  .~ ? o WSAEWOULDBLOCK  ~  ?  ." s" .  <^ < .
    if (WSAEventSelect(listenSock, events[0], FD_ACCEPT) == SOCKET_ERROR) {
        printf("[ "o "] WSAEventSelect(  S )  <  O : %s\n", errStr().c_str());
        WSACloseEvent(events[0]);
        closesocket(listenSock);
        return;
    }

    int sockCount = 1;   //  ~" z    <o  '   ?O "  ^~ (  S  1 o ? "   <o z')
    printf("[ "o "]  O?   '... (WSA_MAXIMUM_WAIT_EVENTS = %d)\n", WSA_MAXIMUM_WAIT_EVENTS);

    //  "? "? WSAEventSelect  "    ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    while (true) {
        //  "? "?  <o  "  o    S   O?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        //
        // WSAWaitForMultipleEvents(count, events, bWaitAll, timeout, bAlertable)
        //   count     :   <o .     S   .  "   ^~
        //   events    :    S   .  "    - 
        //   bWaitAll  : FALSE =  .~ ,~  "  <o  "  ~    "  (OR   )
        //               TRUE  =   '   <o  "    .O O ?  O?  (AND   ,   ~  .^  "?)
        //   timeout   : INFINITE =   .o  O? 
        //   bAlertable: FALSE = APC(Completion Routine)   <o (Lab09 -  "o TRUE  ,  s )
        //
        DWORD idx = WSAWaitForMultipleEvents(
            sockCount, events, FALSE, INFINITE, FALSE);

        if (idx == WSA_WAIT_FAILED) {
            printf("[ "o "] WSAWaitForMultipleEvents  <  O : %s\n", errStr().c_str());
            break;
        }
        if (idx == WSA_WAIT_TIMEOUT) {
            // INFINITE  ? o  -   S"  " <  .~ ?  .S ? O   -   " "o o  o  ?
            continue;
        }

        //  "? "?    S     S   " o  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        //
        // WSAWaitForMultipleEvents()   "  ': WSA_WAIT_EVENT_0 +    S 
        //  <  o    S  =   "  ' - WSA_WAIT_EVENT_0
        DWORD i = idx - WSA_WAIT_EVENT_0;

        //  "? "?  -  -   "  S  >O     S   ?  T.   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        //
        // WSAEnumNetworkEvents():  <o  "  o    S   .  "  "   sO .~    .<
        //      S   .  "  "  z  T o  o non-signaled  f  fo o   .< .  <^ < .
        //
        //  ~. WSAResetEvent ?  ^ ." s" .o   o :
        //   WSAEnumNetworkEvents()  ~  o  <o  ,  ?  o  o    S     .< .  <^ < .
        //    "   "o  " " o WSAResetEvent()   ~  o .   ." s" ?  -? S  <^ < .
        //   ( ^~ T o  o WSACreateEvent() .o    S    WSAResetEvent ?  ." s" .~ ? O,
        //    WSAEventSelect -   -   o    S  S"  z  T  ?   <^ < .)
        //
        WSANETWORKEVENTS netEvents{};
        if (WSAEnumNetworkEvents(sockets[i], events[i], &netEvents) == SOCKET_ERROR) {
            printf("[ "o "] WSAEnumNetworkEvents  <  O  (   S  %lu): %s\n",
                   i, errStr().c_str());
            continue;
        }

        //  "? "? FD_ACCEPT:  f^  -    ^~   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        if (netEvents.lNetworkEvents & FD_ACCEPT) {
            // FD_ACCEPT -   ~  ~ ?  z^ S" ?   ?  T. 
            if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                printf("[ "o "] FD_ACCEPT  ~  ~: %s\n",
                       errStr(netEvents.iErrorCode[FD_ACCEPT_BIT]).c_str());
            } else {
                sockaddr_in cAddr{};
                int cLen = sizeof(cAddr);
                SOCKET newSock = accept(sockets[i], (sockaddr*)&cAddr, &cLen);

                if (newSock == INVALID_SOCKET) {
                    printf("[ "o "] accept  <  O : %s\n", errStr().c_str());
                } else if (sockCount >= MAX_SOCKETS) {
                    printf("[ "o "]  o O?  -    ^~  ^ ,   ^: %s\n",
                           addrStr(cAddr).c_str());
                    const char* msg = " "o " ?   T"  f  fo z. <^ < .\r\n";
                    send(newSock, msg, (int)strlen(msg), 0);
                    closesocket(newSock);
                } else {
                    //  f^     -  S   ?O " -     S   .  "   f  "     -  
                    WSAEVENT ev = WSACreateEvent();
                    if (ev == WSA_INVALID_EVENT) {
                        printf("[ "o "] WSACreateEvent(    -  S )  <  O : %s\n",
                               errStr().c_str());
                        closesocket(newSock);
                    } else if (WSAEventSelect(newSock, ev,
                                              FD_READ | FD_CLOSE) == SOCKET_ERROR) {
                        printf("[ "o "] WSAEventSelect(    -  S )  <  O : %s\n",
                               errStr().c_str());
                        WSACloseEvent(ev);
                        closesocket(newSock);
                    } else {
                        events [sockCount] = ev;
                        sockets[sockCount] = newSock;
                        ++sockCount;
                        printf("[ "o "] [%s]  -    ^~ : %s (   S  %d,  ~" z  %d o  -  )\n",
                               nowStr().c_str(), addrStr(cAddr).c_str(),
                               sockCount - 1, sockCount - 1);
                    }
                }
            }
        }

        //  "? "? FD_READ:  ^~ <     "   ~   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        if (netEvents.lNetworkEvents & FD_READ) {
            if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
                printf("[ "o "] FD_READ  ~  ~ (   S  %lu): %s\n",
                       i, errStr(netEvents.iErrorCode[FD_READ_BIT]).c_str());
            } else {
                char buf[BUFSIZE]{};
                // WSAEventSelect  ,  s   '  ? o  ?O " ?  .  " o ,    "o
                // recv() S"    "  ?  -? o   WSAEWOULDBLOCK   "  ( .~ ? O FD_READ
                //    S   >"  ? o    "  ?  z^ O    z  )
                int n = recv(sockets[i], buf, BUFSIZE - 1, 0);
                if (n > 0) {
                    buf[n] = '\0';
                    printf("[ "o "] [%s]  ^~ <  (   S  %lu): \"%s\" (%d "  S )\n",
                           nowStr().c_str(), i, buf, n);
                    //  -  "
                    if (send(sockets[i], buf, n, 0) == SOCKET_ERROR) {
                        printf("[ "o "] send  <  O  (   S  %lu): %s\n",
                               i, errStr().c_str());
                    }
                } else if (n == SOCKET_ERROR) {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK) {
                        printf("[ "o "] recv  ~  ~ (   S  %lu): %s\n",
                               i, errStr(err).c_str());
                    }
                }
            }
        }

        //  "? "? FD_CLOSE:  -    . O  ~   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        if (netEvents.lNetworkEvents & FD_CLOSE) {
            printf("[ "o "] [%s]  -    . O (   S  %lu)\n", nowStr().c_str(), i);

            //    S   .  "    ?O "  . 
            WSACloseEvent(events[i]);
            closesocket(sockets[i]);

            //  "? "?   -  -  "o  .  <   S    o :  ^ ? ?  s" ?O   ^  z   o   T  "? "? "? "? "? "? "? "? "? "?
            //   -   ' " -       f       S  ?  z ?  .S o  ? o
            //  ^ ? ?  >  ?O   .  <   o" ~ o   ,  .    -  "  -  ?   o  o  o  ? .  <^ < .
            --sockCount;
            if ((int)i < sockCount) {
                events [i] = events [sockCount];
                sockets[i] = sockets[sockCount];
            }
            events [sockCount] = WSA_INVALID_EVENT;
            sockets[sockCount] = INVALID_SOCKET;

            printf("[ "o "]  ~" z   -    ^~: %d\n", sockCount - 1);
        }
    }

    //  "? "?  .   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    for (int i = 0; i < sockCount; ++i) {
        if (events[i]  != WSA_INVALID_EVENT) WSACloseEvent(events[i]);
        if (sockets[i] != INVALID_SOCKET)    closesocket(sockets[i]);
    }
    printf("[ "o "]  . O\n");
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//     -  S    ~" (Lab06   T    )
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
void runClient(int id) {
    printf("[    -  S -%d]  "o " 127.0.0.1:%d  -   -    <o "\n", id, PORT);

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

    for (int m = 1; m <= 5; ++m) {
        char buf[BUFSIZE]{};
        int len = snprintf(buf, BUFSIZE, "Client-%d: msg#%d", id, m);

        printf("[    -  S -%d]  " ? : \"%s\"\n", id, buf);
        if (!SendAll(sock, buf, len)) {
            printf("[    -  S -%d] send  <  O : %s\n", id, errStr().c_str());
            break;
        }

        char echo[BUFSIZE]{};
        if (!RecvExact(sock, echo, len)) {
            printf("[    -  S -%d] echo recv  <  O \n", id);
            break;
        }
        printf("[    -  S -%d]  -  "  ^~ < : \"%.*s\"\n", id, len, echo);

        Sleep(500);
    }

    closesocket(sock);
    printf("[    -  S -%d]  . O\n", id);
}

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// [ .T S   . ] WSAEventSelect  ~    "
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
//
//  1. WSACreateEvent()     ?'    S   .  " (WSAEVENT)  f  " 
//  2. WSAEventSelect()     ?'  ?O "     S   .  "  "  -  ,   <o .     S   ^ S    "  .
//                           ( T <o -   ?O " "  .  " o ,    "o o  " T~!)
//  3. WSAWaitForMultipleEvents()  ?'  "   o    S   '  .~ ,~  "  <o  "    .O O ?  O? 
//  4. WSAEnumNetworkEvents()      ?'  -  -   "  S  >O     S   ?  T.  +    S   z  T   .<
//  5. lNetworkEvents  ." "o  AND  ^ S   o  T.  (FD_READ, FD_ACCEPT  " )
//  6. iErrorCode[]   -  -  "o      S  "  ~  ~  " "o  T. 
//
//    S   . ~ (FD_  f  ^~ " ):
//   FD_READ    : recv()  ? S  ( ^~ <   "  -     "   z^ O)
//   FD_WRITE   : send()  ? S  ( ?  <   "  -    "  z^ O),  ^   -     >"  .o  ^  o f 
//   FD_ACCEPT  : accept()  ? S  ( O?   '   -    z^ O)
//   FD_CONNECT : connect()  T" O
//   FD_CLOSE   :  f  O?    -    . O
//   FD_OOB     : Out-Of-Band(  ?)    "   ^~ < 
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
