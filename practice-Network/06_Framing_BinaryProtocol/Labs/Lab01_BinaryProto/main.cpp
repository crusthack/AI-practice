// ============================================================================
// Lab01_BinaryProto  ?"   S  .?   "  "" o ?  o (TLV  O  " )
// ============================================================================
//  <  -?   .:
//    "o ": Lab01_BinaryProto.exe server
//       -  S : Lab01_BinaryProto.exe client
//
//  .T S    'o:
//   1.       "  S  >O   "" ^ z" o  o    T"/ -    T" .~ S"   .
//   2.  . -" "" - ( "  S  >O   "  S   ~  ")  ? T~   ." ^~    o 
//   3. magic  ." "o o  O  ,   T  T"     .~ S"   .
//   4. TLV(Type-Length-Value)  O  " :    " <o ? ?  f? z.     ' "   . 
//   5. RecvExact o TCP  S  S   -  "o  . T. .o       S"   .
// ============================================================================

#include "winsock_util.hpp"
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

static constexpr USHORT PORT = 9014;

//  "? "?  "" o ?  o  "  "  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// TLV(Type-Length-Value)  O  " :
//   - Type  :  " <o ? ~  . ~ (MSG_PING, MSG_TEXT  " )
//   - Length:  '  "   S" body ~  "  S   ^~
//   - Value :  <  o  Z~  o "o    " 
//
//  -  " ?   .   (16 bytes)  ? o  ^~ <    ?  .  f  16 "  S     ?   ,
//    .^ ~ bodyLen "     " ? o bodyLen  "  S     S" < .
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

//  "? "?  . -" "" - ( "  S  >O   "  S   ~  ")   ." s" .o   o   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//   ." x86/x64 S"   <? -" "" - : DWORD 0x12345678   "   -  78 56 34 12  ^o "o o  ? z .
//   Y  ,~   "  "   'o ?(RFC 1700) ?  . -" "" - : 12 34 56 78  ^o "o   ,  s  .o < .
//  "o o  <    ." ,  .  ~  "  ?  <   <o htonl/ntohl  ? T~   -? o    ^  z  ?  '  ' ~?  .  "  o < .
// hton = host-to-network, ntoh = network-to-host
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

//  " <o ?  f? z.  -   ~.
//  ' ? 1 ? "   <o z'  ?" 0 ? "  ^  T""    ? .~ S"    "    o" .   ~^ . 
enum MsgType : BYTE {
    MSG_PING       = 1,  //  -    ,  ." z^ S" ?  T.   s" 
    MSG_PONG       = 2,  // PING -   O? .o  ' < 
    MSG_TEXT       = 3,  //   ~  .  S  S   " <o ? (body = UTF-8   z  - )
    MSG_ACK        = 4,  // TEXT/FILE  ^~ <   T. 
    MSG_FILE_START = 5,  //  OO   " ?   <o z' (body =  OO  .)
    MSG_FILE_DATA  = 6,  //  OO     (body =  "  "^     " )
    MSG_FILE_END   = 7,  //  OO   " ?   T" O (body =    "  S   ^~ DWORD)
    MSG_BYE        = 8,  //  -    . O  s" 
};

//  "? "? magic  ." "o ~  -  .   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// 0xFACEFEED:  O  ,   <o z'  ^ ( T  T"  >O "o).
// TCP  S  S     ' " ? "    z^  ,~,   "  "  .~  z" ~    "  ?  ,  ."  z^ "  .O
// magic    ~ .~ ?  .S o    ?' " ^~ <   S  S     -  < ,  < "   O  <  .~   z  T  T".
//  <  o   ~": magic  ^  ~  <o 1 "  S  "   " " .~  0xFACEFEED   S  " .o < .
//  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
static constexpr DWORD MAGIC = 0xFACEFEEDu;

// 16 "  S    .  -  "  ?" #pragma pack(1)  o  O  "   -?   "  S  >O      1:1   .'
#pragma pack(push, 1)
struct MsgHeader {
    DWORD magic;    //  O  ,   <o z'  ^ :  .  f  0xFACEFEED
    BYTE  version;  //  "" o ?  o  " " ( ~" z  1)
    BYTE  type;     // MsgType  -   '
    WORD  flags;    //  ~^ .  ( " >"  .. .  ." ~  T"  "O z~  s )
    DWORD seq;      //  ?  <     " <o ?  <o ?? S   ^ ~  ( ^o "o  " / z    s )
    DWORD bodyLen;  //  '  "   S" body ~  "  S   ^~ ( "  S  >O   "  S   ~  ")
    //    -  "   >" -  bodyLen  "  S  ~  Z~  o "o ?  ~  < 
};
#pragma pack(pop)

static_assert(sizeof(MsgHeader) == 16, "MsgHeader S"  . T. z^ 16 "  S  -  .   .  <^ < ");

//  "? "?  f? z.   "  ?'   z  -   ? T~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
const char* msgTypeName(BYTE t) {
    switch (t) {
    case MSG_PING:       return "MSG_PING";
    case MSG_PONG:       return "MSG_PONG";
    case MSG_TEXT:       return "MSG_TEXT";
    case MSG_ACK:        return "MSG_ACK";
    case MSG_FILE_START: return "MSG_FILE_START";
    case MSG_FILE_DATA:  return "MSG_FILE_DATA";
    case MSG_FILE_END:   return "MSG_FILE_END";
    case MSG_BYE:        return "MSG_BYE";
    default:             return "UNKNOWN";
    }
}

//  "? "?   " ":  ~  S  S   "  S   ~  "  ?'  "  S  >O   "  S   ~  "  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  ~  o  " hdr ~  ." "o   ~  S  S   "  S   ~  " o  " s   '     .  ^~ -   " <  .o < .
//  ,  ? -  "o htonl/htons    s  .   " ?    "  f  fo o  ? T~ .o < .
MsgHeader encode(BYTE type, DWORD seq, DWORD bodyLen, BYTE version = 1) {
    MsgHeader h{};
    // magic ?     . -" "" -  o  o  ? z  ~ -  .   .~ ? o htonl   s 
    h.magic   = htonl(MAGIC);
    h.version = version;
    h.type    = type;
    h.flags   = 0;
    h.seq     = htonl(seq);     // DWORD: 4 "  S    '   ? T~
    h.bodyLen = htonl(bodyLen); // bodyLen "  T 
    return h;
}

//  "? "?  "" " ":  "  S  >O   "  S   ~  "  ?'  ~  S  S   "  S   ~  "  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// recv o   ?  >  <o  "  S       o  .  "  .o  '     .  ^~ o  . o T" .o < .
bool decode(const MsgHeader& raw, MsgHeader& out) {
    out.magic   = ntohl(raw.magic);
    out.version = raw.version;
    out.type    = raw.type;
    out.flags   = ntohs(raw.flags);
    out.seq     = ntohl(raw.seq);
    out.bodyLen = ntohl(raw.bodyLen);

    // magic  ^  ~  ?'  S  S     -  < ,   ,~  z~  o  O  , 
    if (out.magic != MAGIC) {
        printf("[decode] magic  ^  ~: 0x%08X (  O?: 0x%08X)\n", out.magic, MAGIC);
        return false;
    }
    return true;
}

//  "? "?  " <o ?  ?  <   -    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  -  "    " "  .~  body T?  .  ~  " ?  .o < .
// SendAll:  ,  ?   "" o n "  S     z   " ?  (partial send  O? ')
bool sendMsg(SOCKET s, BYTE type, DWORD seq,
             const void* body = nullptr, DWORD bodyLen = 0) {
    MsgHeader h = encode(type, seq, bodyLen);
    if (!SendAll(s, &h, sizeof(h))) {
        printf("[sendMsg]  -  "  " ?   <  O : %s\n", errStr().c_str());
        return false;
    }
    if (bodyLen > 0 && body) {
        if (!SendAll(s, body, (int)bodyLen)) {
            printf("[sendMsg] body  " ?   <  O : %s\n", errStr().c_str());
            return false;
        }
    }
    printf("[->] %-16s seq=%-4u bodyLen=%u\n",
           msgTypeName(type), seq, bodyLen);
    return true;
}

//  "? "?  " <o ?  ^~ <   -    "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
// RecvExact o  -  " 16 "  S    . T. z^   ,
// bodyLen -   "    " ? o body    S" < .
bool recvMsg(SOCKET s, MsgHeader& hdr, std::vector<char>& body) {
    // 1 <  ":   .     -  "  ^~ <  ( .  f  16 "  S )
    MsgHeader raw{};
    if (!RecvExact(s, &raw, sizeof(raw))) return false;

    // 2 <  ":  "  S   ~  "  ? T~   magic  ? 
    if (!decode(raw, hdr)) return false;

    // 3 <  ": body  ^~ <  (bodyLen > 0     s )
    body.resize(hdr.bodyLen);
    if (hdr.bodyLen > 0) {
        if (!RecvExact(s, body.data(), (int)hdr.bodyLen)) return false;
    }

    printf("[<-] %-16s seq=%-4u bodyLen=%u\n",
           msgTypeName(hdr.type), hdr.seq, hdr.bodyLen);
    return true;
}

// ============================================================================
//  "o "    o 
// ============================================================================
void runServer() {
    printf("=== Lab14   "  "" o ?  o  "o " (  S  %d) ===\n\n", PORT);

    SOCKET listenSock = makeTcpSocket();
    if (listenSock == INVALID_SOCKET) return;

    // SO_REUSEADDR:  "" o "  S   . O  >" TIME_WAIT  f  fo -  "o  T ?   S   z  ,  s   -^ s 
    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

    sockaddr_in addr = makeAddr(nullptr, PORT);
    if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[ "o "] bind  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    listen(listenSock, 1);
    printf("[ "o "]  -    O?   '...\n");

    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);
    SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
    if (clientSock == INVALID_SOCKET) {
        printf("[ "o "] accept  <  O : %s\n", errStr().c_str());
        closesocket(listenSock);
        return;
    }
    printf("[ "o "]     -  S   -  : %s\n\n", addrStr(clientAddr).c_str());

    DWORD svrSeq = 0; //  "o "    ?  <   <o ?? S   ^ ~ 

    //  "? "?  " <o ?  ~    ""  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    bool running = true;
    while (running) {
        MsgHeader hdr{};
        std::vector<char> body;

        if (!recvMsg(clientSock, hdr, body)) {
            printf("[ "o "]  -    . O  ~  S"  ^~ <   ~  ~\n");
            break;
        }

        switch (hdr.type) {
        case MSG_PING:
            // PING  ^~ <   ?' PONG  ' <  (ping-pong  -    T. )
            sendMsg(clientSock, MSG_PONG, svrSeq++);
            break;

        case MSG_TEXT: {
            // TEXT  ^~ <   ?' body    z  -  o  o   >" ACK +  -  "
            std::string txt(body.data(), body.size());
            printf("  [ "o "]  ^~ <   .  S  S : \"%s\"\n", txt.c_str());

            // ACK  " ?  ( ^~ <   T. )
            sendMsg(clientSock, MSG_ACK, svrSeq++);

            //  -  ":  > ?  .  S  S     O? o  O   f"
            sendMsg(clientSock, MSG_TEXT, svrSeq++,
                    body.data(), (DWORD)body.size());
            break;
        }

        case MSG_FILE_START: {
            std::string filename(body.data(), body.size());
            printf("  [ "o "]  OO   ^~ <   <o z': \"%s\"\n", filename.c_str());
            sendMsg(clientSock, MSG_ACK, svrSeq++);
            break;
        }

        case MSG_FILE_DATA:
            //  <  o   ~" -  "o S"  OO  -   "   ? O,  -   "o S"    O  o 
            printf("  [ "o "]  OO      ^~ < : %u bytes\n", hdr.bodyLen);
            break;

        case MSG_FILE_END: {
            DWORD total = 0;
            if (body.size() >= 4)
                total = ntohl(*reinterpret_cast<DWORD*>(body.data()));
            printf("  [ "o "]  OO   ^~ <   T" O:   %u bytes\n", total);
            sendMsg(clientSock, MSG_ACK, svrSeq++);
            break;
        }

        case MSG_BYE:
            // BYE  ^~ <   ?' ACK  >"   ""  . O
            printf("  [ "o "]  . O  s"   ^~ < \n");
            sendMsg(clientSock, MSG_ACK, svrSeq++);
            running = false;
            break;

        default:
            printf("  [ "o "]  .O  ^~  -? S"  f? z.: %d\n", hdr.type);
            break;
        }
    }

    closesocket(clientSock);
    closesocket(listenSock);
    printf("\n[ "o "]  . O\n");
}

// ============================================================================
//     -  S     o 
// ============================================================================
void runClient() {
    printf("=== Lab14   "  "" o ?  o     -  S  ===\n\n");

    SOCKET sock = makeTcpSocket();
    if (sock == INVALID_SOCKET) return;

    sockaddr_in addr = makeAddr("127.0.0.1", PORT);
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[    -  S ] connect  <  O : %s\n", errStr().c_str());
        closesocket(sock);
        return;
    }
    printf("[    -  S ]  "o "  -    "  \n\n");

    DWORD seq = 0; //     -  S     ?  <   <o ?? S   ^ ~ 

    //  "? "?  -  :  "o "  ' <   .o    ^~ <   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    auto recvOne = [&]() {
        MsgHeader hdr{};
        std::vector<char> body;
        if (!recvMsg(sock, hdr, body)) {
            printf("[    -  S ]  ^~ <   <  O \n");
            return false;
        }
        if (hdr.type == MSG_TEXT && !body.empty()) {
            std::string txt(body.data(), body.size());
            printf("  [    -  S ]  -  "  ^~ < : \"%s\"\n", txt.c_str());
        }
        return true;
    };

    //  "? "? 1 <  ": PING  " ?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("--- 1 <  ": PING ---\n");
    sendMsg(sock, MSG_PING, seq++);
    recvOne(); // PONG   O?

    //  "? "? 2 <  ": TEXT  " <o ? 3 o  " ?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n--- 2 <  ": TEXT x3 ---\n");
    const char* texts[] = { " .^ .. .~ "  s"!", "  "  "" o ?  o  .O S  S ", "TLV  O  "   <  S " };
    for (int i = 0; i < 3; ++i) {
        sendMsg(sock, MSG_TEXT, seq++, texts[i], (DWORD)strlen(texts[i]));
        recvOne(); // ACK   O?
        recvOne(); //  -  " TEXT   O?
    }

    //  "? "? 3 <  ":  OO   " ?   <o  ^  .~  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    // FILE_START  ?' FILE_DATA ( -  Y    )  ?' FILE_END  ^o "o
    //  <  o  OO   " ?   "" o ?  o   T  .o   "
    printf("\n--- 3 <  ":  OO   " ?   <o  ^  .~ ---\n");
    const char* filename = "test_data.bin";
    sendMsg(sock, MSG_FILE_START, seq++, filename, (DWORD)strlen(filename));
    recvOne(); // ACK   O?

    //  ? f   OO     " : 3     - 64 bytes
    std::vector<char> chunk(64, 0xAB);
    DWORD totalBytes = 0;
    for (int i = 0; i < 3; ++i) {
        sendMsg(sock, MSG_FILE_DATA, seq++, chunk.data(), (DWORD)chunk.size());
        totalBytes += (DWORD)chunk.size();
    }

    // FILE_END: body -     "  S   ^~  DWORD( "  S  >O   "  S   ~  ") o  <  O
    DWORD netTotal = htonl(totalBytes);
    sendMsg(sock, MSG_FILE_END, seq++, &netTotal, sizeof(netTotal));
    recvOne(); // ACK   O?

    //  "? "? 4 <  ": BYE  " ?   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    printf("\n--- 4 <  ": BYE ---\n");
    sendMsg(sock, MSG_BYE, seq++);
    recvOne(); // ACK   O?

    closesocket(sock);
    printf("\n[    -  S ]  . O\n");
}

// ============================================================================
int main(int argc, char* argv[]) {
    // RAII:  S  " ""  . O  <o WSACleanup  z  T  ~  o
    WsaInit wsa;

    if (argc < 2) {
        printf(" ,  s  .: %s server|client\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        runServer();
    } else if (strcmp(argv[1], "client") == 0) {
        runClient();
    } else {
        printf(" .O  ^~  -? S"   "o: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
