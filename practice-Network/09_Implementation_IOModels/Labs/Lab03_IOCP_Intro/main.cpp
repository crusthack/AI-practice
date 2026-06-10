// ============================================================================
// Lab03_IOCP_Intro  ?" IOCP(I/O Completion Port)   ^  <  S 
// ============================================================================
//
// [ <  -?   .]
//   exe                   ?' Part 1 ( ^~ T Post/Get) + Part 2 (TCP accept+IOCP)  ^o   <  -?
//
// [ .T S    'o]
//   1. CreateIoCompletionPort / PostQueuedCompletionStatus / GetQueuedCompletionStatus
//       ~   " "  "  S  >O ,   -?   ^o ^~ .~ O   .  .o < .
//   2. Completion Key  T? OVERLAPPED    "  ?     -  -   .    <  S" ?   .  .o < .
//   3.  >O   S  ^ "o  o   o ^~(2 -CPU " - )      " "o o  T.  .o < .
//   4. Part 2 -  "o  <  o TCP  ?O " " IOCP -   -   .~   " T   ^~ <  "   -~ .o < .
//
// [  S ] 9010
// ============================================================================

#include "winsock_util.hpp"
#include <windows.h>
#include <process.h>   // _beginthreadex
#include <cstdio>
#include <vector>
#include <atomic>
#include <string>

//  "? "?   S   f  ^~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static constexpr USHORT PORT = 9010;

//  "? "?  >O   S  ^ "o  ^~ = 2  - CPU  " -   ^~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   o : IOCP  >O  S"  O? ? " ~  <o " " GetQueuedCompletionStatus() -  "o blocking  O? .
//       I/O  T" O ?  "  -  ~  S"  ^o "  .~ ,~ ?   -  ,~  ~  .o < .
//       CPU  " -   ^~ ~ 2    ,  s  .~ S"   o :
//        ?'   ?  >O  ?   "   ~  o( ~^: HeapAlloc, printf) o  z  <o  ^  "
//          <    >O  ? IOCP   " ?   "o ^  .   ^~  z^ -   ? -    " -  "  < .
static int workerCount() {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    return static_cast<int>(si.dwNumberOfProcessors) * 2;
}

// ============================================================================
//  "? "? PART 1:  ^~ T Post / Get  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// ============================================================================
// IOCP ~  .  <   o .  "  "  S  >O ,   -?    ?   .  .o < .
//
// Completion Key  :  -  S  "  ( ?O "/ OO )"  -   O? .o  T" O  ?  <  " .~ S"    " .
//                   CreateIoCompletionPort  ~  S" PostQueuedCompletionStatus -  "o  ? ..
// lpOverlapped    :  -  -  " z' -.(I/O  s" )"  -   O? .o  T" O  ?  <  " .~ S"    " .
//                    " T  WSARecv/WSASend -   "~  OVERLAPPED     ~   ?O.
//
//  ^~ T Post -  "o S"  ?O "   -? o  ? o  '   ' "  z" ~ o  To s  .o < .
// ============================================================================

//  z' -.  f? z.   " (Part 1 -  "o S" Completion Key  ' o  o  f? z. "   " " )
enum class JobType { COMPUTE = 1, LOG = 2, SHUTDOWN = 0 };

struct Job {
    JobType type;
    int     value;
};

static HANDLE g_iocpPart1 = INVALID_HANDLE_VALUE;

// Part 1  >O   S  ^ "o  .  ^~
static unsigned __stdcall workerPart1(void* /*param*/) {
    DWORD       bytes = 0;
    ULONG_PTR   key   = 0;
    OVERLAPPED* ovl   = nullptr;

    while (true) {
        //  "? "? GetQueuedCompletionStatus  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        //  "   ^ (lpCompletionKey): Completion Key ( ^" ?  T" O  ,~)
        //  "   ^ (lpOverlapped)   : OVERLAPPED    "  ( -  -  I/O ?  T" O  ,~)
        // INFINITE:  T" O  O  ,    ~   .O O ?  ~  >  z^  O?  (CPU  ,  s  0%)
        BOOL ok = GetQueuedCompletionStatus(
                      g_iocpPart1,  // IOCP  .  " 
                      &bytes,       //  " ?   "  S   ^~
                      &key,         // Completion Key
                      &ovl,         // OVERLAPPED    " 
                      INFINITE);    //  f? z" ." >f

        if (!ok && ovl == nullptr) {
            //  f? z" ." >f  ~  S" IOCP  .  "   <  z~  ?'  . O
            break;
        }

        // key==0 + bytes==0  ?'  . O  <  ~ 
        if (key == 0 && bytes == 0) {
            printf("  [Worker tid=%lu]  . O  <  ~   ^~ < \n", GetCurrentThreadId());
            break;
        }

        // key  Job    "  o  .  " 
        Job* job = reinterpret_cast<Job*>(key);
        switch (job->type) {
        case JobType::COMPUTE:
            printf("  [Worker tid=%lu] COMPUTE job: value=%d, result=%d\n",
                   GetCurrentThreadId(), job->value, job->value * job->value);
            break;
        case JobType::LOG:
            printf("  [Worker tid=%lu] LOG    job: message=item_%d\n",
                   GetCurrentThreadId(), job->value);
            break;
        default:
            break;
        }
    }
    return 0;
}

static void runPart1() {
    printf("\n");
    printf("============================================================\n");
    printf(" PART 1: IOCP  ^~ T Post / Get ( "  S  >O ,   -? O)\n");
    printf("============================================================\n");

    //  "? "? IOCP  f  "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, numWorkers)
    //      ^    z  INVALID_HANDLE_VALUE  ?'  f^ IOCP  f  "  ( OO / ?O "  -    -? O)
    //    "   ^    z  numWorkers            ?'  T <o  <  -?  -^ s   S  ^ "o  ^~
    //                                       0 = CPU  " -   ^~  z  T  ,  s 
    int N = workerCount();
    printf("[Part1]  >O   S  ^ "o  ^~ = %d (CPU  - 2)\n", N);

    g_iocpPart1 = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, (DWORD)N);
    if (g_iocpPart1 == NULL) {
        printf("[CreateIoCompletionPort]  <  O : %s\n", errStr().c_str());
        return;
    }

    //  >O   S  ^ "o  <o z'
    std::vector<HANDLE> threads(N);
    for (int i = 0; i < N; ++i)
        threads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerPart1, nullptr, 0, nullptr);

    //  "? "? Post: 10 o  z' -.   -   ,  z.  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // PostQueuedCompletionStatus:  ?O " I/O  -?   ^~ T o  o  T" O  O  ,  " IOCP -   "  S" < .
    //   dwNumberOfBytesTransferred :  >  .~ S"  '  z  o   ? . ( >O  -  "o   f")
    //   dwCompletionKey            : ULONG_PTR  ?'   ?        "    "~  < 
    //   lpOverlapped               : OVERLAPPED    "  ( ^~ T   .O S" nullptr  ? S )
    static Job jobs[10];
    for (int i = 0; i < 10; ++i) {
        jobs[i].type  = (i % 2 == 0) ? JobType::COMPUTE : JobType::LOG;
        jobs[i].value = i + 1;

        BOOL ok = PostQueuedCompletionStatus(
                      g_iocpPart1,
                      (DWORD)(i + 1),                // bytes  ." "o ( z" ~  ,  s )
                      reinterpret_cast<ULONG_PTR>(&jobs[i]),  // Completion Key
                      nullptr);                       // OVERLAPPED ( ^~ T  ? o  ^ ." s")
        if (!ok)
            printf("[PostQueuedCompletionStatus]  <  O : %s\n", errStr().c_str());
    }
    printf("[Part1] 10 o  z' -. Post  T" O.  >O  "    ~   '...\n");

    //  ~   <o "  O? 
    Sleep(500);

    //  "? "?  . O  <  ~  Post ( >O   ^~ O )  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // key=0, bytes=0, ovl=nullptr  ?'  >O  ?     . O  <  ~  o   < 
    for (int i = 0; i < N; ++i)
        PostQueuedCompletionStatus(g_iocpPart1, 0, 0, nullptr);

    //   "   >O   . O  O? 
    WaitForMultipleObjects((DWORD)N, threads.data(), TRUE, 3000);
    for (auto h : threads) CloseHandle(h);
    CloseHandle(g_iocpPart1);
    g_iocpPart1 = INVALID_HANDLE_VALUE;

    printf("[Part1]  T" O\n");
}

// ============================================================================
//  "? "? PART 2: TCP accept + IOCP  -    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// ============================================================================
//  <  o  ?O " " IOCP -   -   .~   " T   ^~ < (WSARecv) "   -~ .o < .
//
// IOCP   ~  "o " ~  .  <    ":
//   1) CreateIoCompletionPort o IOCP  f  " 
//   2) accept() o     -  S   ?O "  s  " 
//   3) CreateIoCompletionPort(clientSock, iocp, key, 0)  ?'  ?O " " IOCP -   -  
//   4) WSARecv(ovl)  ?'  " T   ^~ <   <o z' ( ? <o  ~ T~)
//   5)  >O   S  ^ "o: GetQueuedCompletionStatus  ?'    "   ~   ?' WSARecv  z  o -?
//
//  "? "? select vs IOCP  s  o   "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// select:     "" ^ <  " ? " o  ?O "   z^ ,~?"   polling  ?' O(n) fd  ? , 
//             -  S  1000 o  999 o ?  ."  f "  .^  .  "  ? ,   ." s"
// IOCP  : I/O  T" O  <o OS ?   '  T" O  O  ,  "   -   "  O  ?' O(1) dequeue
//          O?   '   >O  ?  ? <o   -  ,~  ~   ?'  ^ ." s" .o CPU  ,  "  -? O
// ============================================================================

// Per-I/O    "  (OVERLAPPED   ~    " T  I/O  s"  ^ <   .~ ,~ "   .  < )
struct PerIoData {
    WSAOVERLAPPED ovl;      //  ~ "o <o    ^   ." "o!
                            // GetQueuedCompletionStatus ~ lpOverlapped  T?  T    ?O
    WSABUF        wsaBuf;   // WSARecv/WSASend -   "~  S"  "   "" S    " 
    char          buf[4096];
    int           opType;   // 0=OP_RECV, 1=OP_SEND ( -  >"  T. z  s )

    static const int OP_RECV = 0;
    static const int OP_SEND = 1;
};

// Per-Connection    "  (Completion Key o  " < :  ?O "  <  ")
struct PerConnData {
    SOCKET sock;
    char   remoteIp[16];
    USHORT remotePort;
};

static HANDLE g_iocpPart2 = INVALID_HANDLE_VALUE;
static std::atomic<bool> g_stopPart2{false};

//  f^ PerIoData   zT -   .  <  .~  WSARecv   o -? .~ S"  -  
// HeapAlloc/HeapFree  O? <  new/delete   ,  s  .~ S"  f "  ? S  .~ ? O
//   "  S   "o " -  "o S"  " "  '?(Pool)  .  <  z    "  S"   s  ?  Z < .
static bool postRecv(PerConnData* conn) {
    PerIoData* io = new PerIoData{};   // zero-init
    io->opType        = PerIoData::OP_RECV;
    io->wsaBuf.buf    = io->buf;
    io->wsaBuf.len    = sizeof(io->buf);

    DWORD flags = 0, recvBytes = 0;
    int r = WSARecv(
                conn->sock,
                &io->wsaBuf, 1,   //  "    -  (WSABUF*)
                &recvBytes,       //  T   T" O  <o  ^~ <   "  S  ( " T     <o)
                &flags,
                &io->ovl,         // OVERLAPPED  ?'  T" O  <o IOCP   -     "   "  -  ~ 
                nullptr);         //  T" O   <  (IOCP   <  -  "o S" NULL)

    if (r == SOCKET_ERROR) {
        int e = WSAGetLastError();
        if (e != WSA_IO_PENDING) {
            // WSA_IO_PENDING =  " T   o -?  "   ( ? z   . f      s )
            //    T   ~  ~   <  O 
            printf("[WSARecv]  <  O : %s\n", errStr(e).c_str());
            delete io;
            return false;
        }
    }
    // WSA_IO_PENDING:  " T  I/O ?   -   "  -  " < .  T" O ~   >O  ?   -  ,o < .
    return true;
}

// Part 2  >O   S  ^ "o
static unsigned __stdcall workerPart2(void* /*param*/) {
    DWORD       bytes = 0;
    ULONG_PTR   key   = 0;
    OVERLAPPED* ovl   = nullptr;

    while (!g_stopPart2) {
        BOOL ok = GetQueuedCompletionStatus(
                      g_iocpPart2,
                      &bytes, &key, &ovl,
                      1000);   // 1 ^  f? z" ." >f ( . O  "O z~       T. )

        //  . O  <  ~ : bytes=0, key=0, ovl=nullptr
        if (ok && key == 0 && bytes == 0 && ovl == nullptr) break;

        if (!ok) {
            if (ovl == nullptr) continue;   //  f? z" ." >f  ?'   ""  z  <o z'
            // I/O  ~  ~  ?'  -    .  o  ~ 
            bytes = 0;
        }

        // PerIoData  ovl -  "o  -  ,  (ovl     ^   ." "o  ? o   ?O == PerIoData*)
        PerIoData*   io   = reinterpret_cast<PerIoData*>(ovl);
        PerConnData* conn = reinterpret_cast<PerConnData*>(key);

        if (bytes == 0) {
            //     -  S   . f   . O  ~  S"  -  Y 
            printf("[Part2] %s:%d  -    .  o\n", conn->remoteIp, conn->remotePort);
            closesocket(conn->sock);
            delete conn;
            delete io;
            continue;
        }

        io->buf[bytes] = '\0';
        printf("[Part2] %s:%d  ?' %u bytes: \"%s\"\n",
               conn->remoteIp, conn->remotePort, bytes, io->buf);

        //  ' <   " ?  ( " <  z^  T  send)
        SendAll(conn->sock, io->buf, (int)bytes);

        //  <  O  ^~ <  "  o" .  PerIoData  z  o -?
        // ( <  o   "  S   "o " -  "o S" io   z  To s  .~ ? O,  -   "o S" delete  >"  z  .  < )
        delete io;
        postRecv(conn);
    }
    return 0;
}

//  " <      -  S  ( " "  S  ^ "o)
static unsigned __stdcall clientThread2(void* /*param*/) {
    Sleep(300);   //  "o "   S   O? 
    WsaInit wsa2;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr = makeAddr("127.0.0.1", PORT);
    if (connect(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[Client] connect  <  O : %s\n", errStr().c_str());
        closesocket(s);
        return 1;
    }
    printf("[Client]  "o "  -    "  \n");
    for (int i = 1; i <= 5; ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Hello-from-client #%d", i);
        SendAll(s, msg, (int)strlen(msg));
        char echo[256]{};
        int r = recv(s, echo, sizeof(echo) - 1, 0);
        if (r > 0) { echo[r] = '\0'; printf("[Client]  -  ": %s\n", echo); }
        Sleep(200);
    }
    closesocket(s);
    return 0;
}

static void runPart2() {
    printf("\n");
    printf("============================================================\n");
    printf(" PART 2: TCP accept + IOCP  -   (  S  %d)\n", PORT);
    printf("============================================================\n");

    int N = workerCount();

    // IOCP  f  " 
    g_iocpPart2 = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, (DWORD)N);
    if (!g_iocpPart2) {
        printf("[CreateIoCompletionPort]  <  O : %s\n", errStr().c_str());
        return;
    }

    //   S   ?O "  f  " 
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("[socket]  <  O : %s\n", errStr().c_str());
        CloseHandle(g_iocpPart2); return;
    }

    // SO_REUSEADDR:   "  <  -? ~ TIME_WAIT  ?O "    S     o  .  "  z  ,  s   ? S 
    BOOL reuse = TRUE;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    sockaddr_in srv = makeAddr(nullptr, PORT);
    if (bind(listenSock, (SOCKADDR*)&srv, sizeof(srv)) == SOCKET_ERROR) {
        printf("[bind]  <  O : %s\n", errStr().c_str());
        closesocket(listenSock); CloseHandle(g_iocpPart2); return;
    }
    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        printf("[listen]  <  O : %s\n", errStr().c_str());
        closesocket(listenSock); CloseHandle(g_iocpPart2); return;
    }
    printf("[Part2]   S   <o z' (127.0.0.1:%d)\n", PORT);

    //   S   ?O " " IOCP -   -   .   ^~  z^ ? O, Lab 10 -  "o S" AcceptEx   "  ?  .S 
    //  <  ^o blocking accept   "" o   ~" .o < . (AcceptEx S" Lab11 -  "o  o "   ? S )

    //  >O   S  ^ "o  <o z'
    std::vector<HANDLE> workers(N);
    for (int i = 0; i < N; ++i)
        workers[i] = (HANDLE)_beginthreadex(nullptr, 0, workerPart2, nullptr, 0, nullptr);

    //     -  S   S  ^ "o  <o z' ( z  T  .O S  S  s )
    HANDLE hClient = (HANDLE)_beginthreadex(nullptr, 0, clientThread2, nullptr, 0, nullptr);

    // accept   "" ( -   "o S" 5 ^  ~  S"     -  S   S  ^ "o  . O O ?)
    setRecvTimeout(listenSock, 5000);  // accept  f? z" ." >f 5 ^
    int accepted = 0;
    while (accepted < 3) {
        sockaddr_in cli{}; int cliLen = sizeof(cli);
        SOCKET cliSock = accept(listenSock, (SOCKADDR*)&cli, &cliLen);
        if (cliSock == INVALID_SOCKET) break;

        //  "? "?  ?O " " IOCP -   -    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
        // CreateIoCompletionPort(   OO  .  " ,   IOCP, CompletionKey, 0)
        //   CompletionKey -  PerConnData    "    "~  < .
        //     >"    ?O " ~   "  I/O  T" O  <o GetQueuedCompletionStatus -  "o
        //    .  <  PerConnData    "     ,   ^~  z^ < .
        PerConnData* conn = new PerConnData{};
        conn->sock = cliSock;
        inet_ntop(AF_INET, &cli.sin_addr, conn->remoteIp, sizeof(conn->remoteIp));
        conn->remotePort = ntohs(cli.sin_port);

        HANDLE h = CreateIoCompletionPort(
                       (HANDLE)cliSock,          //  -   .   ?O "
                       g_iocpPart2,              //    IOCP -   -  
                       (ULONG_PTR)conn,          // Completion Key = PerConnData*
                       0);                       // 0 = IOCP  ^~ .  -? O
        if (h == NULL) {
            printf("[CreateIoCompletionPort]  ?O "  -    <  O : %s\n", errStr().c_str());
            closesocket(cliSock); delete conn; continue;
        }

        printf("[Part2]     -  S  %s:%d  -   ( ?O " ?'IOCP  -    T" O)\n",
               conn->remoteIp, conn->remotePort);

        //    ^   " T   ^~ <   o -?
        postRecv(conn);
        ++accepted;
    }

    //     -  S   S  ^ "o  . O  O? 
    WaitForSingleObject(hClient, 5000);
    CloseHandle(hClient);

    //  >O   . O  <  ~ 
    g_stopPart2 = true;
    for (int i = 0; i < N; ++i)
        PostQueuedCompletionStatus(g_iocpPart2, 0, 0, nullptr);
    WaitForMultipleObjects((DWORD)N, workers.data(), TRUE, 5000);
    for (auto h : workers) CloseHandle(h);

    closesocket(listenSock);
    CloseHandle(g_iocpPart2);
    printf("[Part2]  T" O\n");
}

// ============================================================================
int main() {
    WsaInit wsa;

    printf("=== Lab10: IOCP   ^  <  S  ===\n");
    printf("CPU  " -   ^~: %d  ?'  >O   S  ^ "o   'o: %d\n",
           workerCount() / 2, workerCount());

    runPart1();   //  ^~ T Post/Get ( "  S  >O ,   -? O)
    runPart2();   // TCP accept + IOCP  -  

    printf("\n[Lab10]   "   OO S   T" O.\n");
    return 0;
}
