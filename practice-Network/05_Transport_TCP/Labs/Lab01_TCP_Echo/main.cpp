//                                                                            
// Lab01_TCP_Echo   TCP       /      
//                                                                            
//      :
//     : Lab01_TCP_Echo.exe server
//        : Lab01_TCP_Echo.exe client [  IP]  (   : 127.0.0.1)
//
//      :
//   1. TCP          : socket   bind   listen   accept   recv/send   close
//   2. 3-way handshake(SYN/SYN-ACK/ACK)  connect/accept         
//   3.           : recv      send                 
//   4. WSAStartup / WSACleanup       RAII   
//   5.      ,       (htons/ntohs), INADDR_ANY   
//                                                                            

#include "winsock_util.hpp"
#include <cstring>   // strncmp
#include <cstdio>
#include <string>

//                                                                        
static constexpr USHORT PORT     = 9001;   //   Lab      
static constexpr int    BACKLOG  = 5;      // listen()       
static constexpr int    BUF_SIZE = 1024;   //          (   )

//                                                                          
// runServer()                     
//                                                                          
static void runServer() {
    printf("=== TCP          (   %u) ===\n", PORT);

    //    step 1: socket()                                                   
    // AF_INET      : IPv4      
    // SOCK_STREAM  : TCP (     ,        )
    // IPPROTO_TCP  :         (0             )
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("[socket]   : %s\n", errStr().c_str());
        return;
    }
    printf("[  ]             (handle=%llu)\n", (UINT64)listenSock);

    //    SO_REUSEADDR:            TIME_WAIT                   
    //     Ctrl+C        OS           (TIME_WAIT)  .
    //               bind()   WSAEADDRINUSE       .
    int reuse = 1;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR,
                   (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        printf("[setsockopt SO_REUSEADDR]   : %s\n", errStr().c_str());
    }

    //    step 2: bind()                                                     
    //        IP +         .
    // INADDR_ANY(0.0.0.0) :        NIC            .
    // htons()             : Host TO Network Short               
    sockaddr_in serverAddr = makeAddr(nullptr, PORT);  // IP=INADDR_ANY
    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[bind]   : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    printf("[  ] bind      0.0.0.0:%u\n", PORT);

    //    step 3: listen()                                                   
    //     '     (passive open)'       .
    // BACKLOG:    accept()              OS              .
    //                             SYN    (drop)      .
    if (listen(listenSock, BACKLOG) == SOCKET_ERROR) {
        printf("[listen]   : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    printf("[  ] listen      backlog=%d,           ...\n", BACKLOG);

    //    step 4: accept                                                   
    // accept()         :                     .
    //    :              (clientSock). listenSock        .
    //                                   (     )
    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);

        printf("\n[  ]           (accept    )...\n");
        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            printf("[accept]   : %s\n", errStr().c_str());
            break;  //              
        }

        // inet_ntop:    IP             (    inet_ntoa          X)
        char ipBuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
        printf("[  ] accept:       %s:%u    \n",
               ipBuf, ntohs(clientAddr.sin_port));

        //    step 5: recv / send                                        
        // recv() :                        .
        //       > 0  :          (    BUF_SIZE           !)
        //       == 0 :                  (FIN   )
        //       < 0  :    (WSAGetLastError        )
        char buf[BUF_SIZE];
        while (true) {
            int recvBytes = recv(clientSock, buf, BUF_SIZE - 1, 0);
            if (recvBytes == 0) {
                printf("[  ]       %s       (FIN   )\n", ipBuf);
                break;
            }
            if (recvBytes == SOCKET_ERROR) {
                printf("[recv]   : %s\n", errStr().c_str());
                break;
            }
            buf[recvBytes] = '\0';  //         (   )
            printf("[  ]    %d bytes: \"%s\"\n", recvBytes, buf);

            // send() :                  OS  TCP         
            //    :                   (         )
            // SendAll() :      recvBytes                
            if (!SendAll(clientSock, buf, recvBytes)) {
                printf("[send]   : %s\n", errStr().c_str());
                break;
            }
            printf("[  ]       (%d bytes)\n", recvBytes);
        }

        //    step 6:                                              
        // shutdown(SD_BOTH):                  FIN        
        // closesocket()    :        OS     (      )
        //   : shutdown   closesocket (send               )
        shutdown(clientSock, SD_BOTH);
        closesocket(clientSock);
        printf("[  ]            .         ...\n");
    }

    //    step 7:                                                      
    closesocket(listenSock);
    printf("[  ]   \n");
}

//                                                                          
// runClient()         ,                
//                                                                          
static void runClient(const char* serverIp) {
    printf("=== TCP          (  : %s:%u) ===\n", serverIp, PORT);
    printf("              Enter. \"quit\"        .\n\n");

    //    step 1:                                                  
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("[socket]   : %s\n", errStr().c_str());
        return;
    }

    //    step 2: connect()                                                  
    //     (IP,   )   TCP         3-way handshake   
    //   1)         SYN          (connect           )
    //   2)            SYN + ACK   (    listen/accept      )
    //   3)         ACK          (connect            )
    // connect    = handshake    =              
    sockaddr_in serverAddr = makeAddr(serverIp, PORT);
    printf("[     ]    %s:%u   connect()     ...\n", serverIp, PORT);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[connect]   : %s\n", errStr().c_str());
        printf("              ,         %u           \n", PORT);
        closesocket(sock);
        return;
    }
    printf("[     ]      !\n\n");

    //    step 3:                                                  
    char sendBuf[BUF_SIZE];
    char recvBuf[BUF_SIZE];

    while (true) {
        printf("> ");
        fflush(stdout);

        // fgets:           .           
        if (!fgets(sendBuf, sizeof(sendBuf), stdin)) break;

        //         
        size_t len = strlen(sendBuf);
        if (len > 0 && sendBuf[len - 1] == '\n') sendBuf[--len] = '\0';
        if (len > 0 && sendBuf[len - 1] == '\r') sendBuf[--len] = '\0';

        if (len == 0) continue;  //       

        // "quit"        
        if (strncmp(sendBuf, "quit", 4) == 0 && len == 4) {
            printf("[     ]         .         .\n");
            break;
        }

        // send(): len+1   '\0'       (            )
        //                                (        )
        if (!SendAll(sock, sendBuf, (int)len + 1)) {
            printf("[send]   : %s\n", errStr().c_str());
            break;
        }
        printf("[     ]    %zu bytes\n", len + 1);

        // recv():           .
        // TCP                len+1                     .
        // RecvExact()       len+1          .
        if (!RecvExact(sock, recvBuf, (int)len + 1)) {
            printf("[recv]            : %s\n", errStr().c_str());
            break;
        }
        recvBuf[len] = '\0';
        printf("[     ]      : \"%s\"\n\n", recvBuf);
    }

    //    step 4:                                                        
    // shutdown(SD_SEND): "                 "       FIN   
    //                          recv        (half-close)
    shutdown(sock, SD_SEND);
    closesocket(sock);
    printf("[     ]      .   .\n");
}

//                                                                          
// main()
//                                                                          
int main(int argc, char* argv[]) {
    //    WsaInit: WSAStartup(2.2)                                      
    // Winsock               WSAStartup          .
    //    MAKEWORD(2,2) =     Winsock   (2.2).
    // WsaInit               WSACleanup          .
    WsaInit wsa;

    if (argc < 2) {
        printf("   : %s server | client [  IP]\n", argv[0]);
        printf("  server : TCP         \n");
        printf("  client : TCP             (   IP: 127.0.0.1)\n");
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        runServer();
    } else if (strcmp(argv[1], "client") == 0) {
        const char* ip = (argc >= 3) ? argv[2] : "127.0.0.1";
        runClient(ip);
    } else {
        printf("[  ]          : %s\n", argv[1]);
        return 1;
    }

    return 0;
}
