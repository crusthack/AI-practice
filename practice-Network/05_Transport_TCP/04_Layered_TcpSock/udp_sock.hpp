#pragma once
#include "ip_headers.hpp"
#include <unordered_map>
#include <vector>
#include <string>

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
// UdpSock  ?" "  S  .? UDP"  SOCK_DGRAM  o" -  "o   '   ~" .  <^ < .
//
//  ^  -    :
//    <  o UDP(RFC 768) ? IP  o" -  "o  .~ S"   "  s   O ~  -  " o  z  ~" .  <^ < .
//   -   S    ~   ?O  ? .  (srcPort / dstPort)
//   -  O  ,     "     "    (calcChecksum  z  ,  s )
//   -  <  Z  T" &  z         (MTU  <     " <o ?   -  Y     o  o  " . )
//   -  " -   ~.  ?  ^~ <        (sendTo / recvFrom)
//
//    ^  -  S"  <   " ( z  " ? ,  ^o "o   z ) "  o  .~ ?  .S S  <^ < .
//  ?'  <   "  ? 04_Layered_TcpSock -  "o    ^  -   o" -    ~"  <^ < .
//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 

static constexpr DWORD  CUDP_MAGIC    = 0xC0FFEE42;
static constexpr int    CUDP_MTU_BODY = 512;   //  <  Z  <   o O?  Z~  o "o ( "  S )
static constexpr int    CUDP_MAX_MSG  = 65000; //  o O?  " <o ?   

//  "? "? CUDP  -  " (24 bytes)  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
//  <  o UDP  -  "(8B)  <    ? O,   o     o  o  <  Z  T"  ." "o    .  .  <^ < .
#pragma pack(push, 1)
struct CudpHdr {
    DWORD  magic;      // 0xC0FFEE42  ?"  O  ,   <  "
    BYTE   type;       // CudpType
    BYTE   flags;      // CUDP_FLAG_FRAG(0x01) | CUDP_FLAG_LAST(0x02)
    WORD   srcPort;
    WORD   dstPort;
    WORD   fragId;     //  <  Z  T"    ID ( " <  Z  T"  <o 0)
    WORD   fragOff;    //  <  Z   ~  "" .< ( "  S   <  o")
    WORD   payLen;     //    O  ,  ~  Z~  o "o   
    WORD   totalLen;   //  >    " <o ?  "     ( z    s )
    WORD   checksum;   // calcChecksum( -  "+ Z~  o "o),  " ,   "    ." "o S" 0
    WORD   reserved;
    // Total: 4+1+1+2+2+2+2+2+2+2+2 = 24 bytes
};
#pragma pack(pop)

static constexpr int CUDP_HDR = sizeof(CudpHdr);  // 24

enum class CudpType : BYTE {
    DATA  = 1,    //   ~    " 
    PING  = 2,    //  f    T. 
    PONG  = 3,    // PING  ' < 
    ERROR = 0xFF, //  ~  ~
};

static constexpr BYTE CUDP_FLAG_FRAG = 0x01;  //  <  Z  T" o  O  , 
static constexpr BYTE CUDP_FLAG_LAST = 0x02;  //  ^ ? ?  <  Z 

//  "? "?  <  Z   z     "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
struct FragGroup {
    std::vector<BYTE> data;        //  z     '   "  (totalLen    o  ^  T")
    std::vector<bool> received;    //    <  Z   ^~ <   -  ?
    int  totalLen   = 0;
    int  fragCount  = 0;           //  ^~ <  .o  <  Z   ^~
    int  fragExpect = 0;           //  "   <  Z   ^~ ( ^ ? ?  <  Z  "  > o    .O  ^~  z^ O)
    DWORD arrivedAt = 0;           //    <  Z   "   <o  ( f? z" ." >f  ~ )
};

//  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . 
class UdpSock {
public:
    UdpSock() : sock_(INVALID_SOCKET) {}
    ~UdpSock() { close(); }

    UdpSock(UdpSock&& o) noexcept : sock_(o.sock_), localPort_(o.localPort_),
        fragIdCounter_(o.fragIdCounter_) {
        o.sock_ = INVALID_SOCKET;
        fragBufs_ = std::move(o.fragBufs_);
    }
    UdpSock(const UdpSock&) = delete;
    UdpSock& operator=(const UdpSock&) = delete;

    bool valid() const { return sock_ != INVALID_SOCKET; }
    USHORT localPort() const { return localPort_; }

    //  "? "?  ^  T"  "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

    //  ^~ <  s :  S  .   S  -   "  "  ( "o "/ ^~ <   O? )
    bool bindPort(USHORT port) {
        if (!createSocket()) return false;
        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("[UdpSock] bind  <  O : %d\n", WSAGetLastError());
            return false;
        }
        localPort_ = port;
        return true;
    }

    //  ?  <  s :  "  "   -?   f  "  (OS ?  z" <o   S   .  < )
    bool open() { return createSocket(); }

    void setRecvTimeout(DWORD ms) {
        setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms));
    }

    void close() {
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
    }

    //  "? "?  ?  <   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

    // data ? MTU   ^  .~   z  T o  o  <  Z  T" .  <^ < .
    bool sendTo(const char* dstIp, USHORT dstPort,
                const void* data, int dataLen,
                CudpType type = CudpType::DATA) {
        if (dataLen > CUDP_MAX_MSG) {
            printf("[UdpSock]  " <o ?  "^   : %d\n", dataLen);
            return false;
        }

        if (dataLen <= CUDP_MTU_BODY) {
            //  <  Z  T"  ^ ." s"
            return sendFragment(dstIp, dstPort, type,
                                /*flags*/0, /*fragId*/0, /*fragOff*/0,
                                dataLen, data, dataLen);
        }

        //  <  Z  T"  ." s"
        WORD fragId = ++fragIdCounter_;
        int  offset = 0;
        while (offset < dataLen) {
            int chunk = (dataLen - offset > CUDP_MTU_BODY)
                        ? CUDP_MTU_BODY : (dataLen - offset);
            bool isLast = (offset + chunk >= dataLen);
            BYTE flags  = CUDP_FLAG_FRAG | (isLast ? CUDP_FLAG_LAST : 0);

            if (!sendFragment(dstIp, dstPort, type,
                              flags, fragId, (WORD)offset,
                              dataLen, (const BYTE*)data + offset, chunk))
                return false;
            offset += chunk;
        }
        return true;
    }

    //  "? "?  ^~ <   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?

    //  <  Z  T" o  O  ,  ?  ,  ? -  "o  z    .o  '   T" "  o  " <o ?   ~ T~ .  <^ < .
    // srcIpOut:  ?  <  z  IP (16 "  S   " )
    // srcPortOut:  ?  <  z    S 
    //  ~ T~ ':  ^~ <  o    "    ,  ? 0     ~  ~/ f? z" ." >f
    int recvFrom(char* srcIpOut, USHORT& srcPortOut,
                 void* buf, int bufLen) {
        while (true) {
            char raw[CUDP_HDR + CUDP_MTU_BODY + 64]{};
            sockaddr_in from{};
            int fromLen = sizeof(from);

            int r = recvfrom(sock_, raw, sizeof(raw), 0,
                             (sockaddr*)&from, &fromLen);
            if (r <= 0) return r == 0 ? 0 : SOCKET_ERROR;
            if (r < CUDP_HDR) continue;

            auto* h = reinterpret_cast<CudpHdr*>(raw);
            if (ntohl(h->magic) != CUDP_MAGIC) continue;

            //    "   ? 
            if (!verifyChecksum(raw, r)) {
                printf("[UdpSock]    "   ~  ~,  O  ,    \n");
                continue;
            }

            inet_ntop(AF_INET, &from.sin_addr, srcIpOut, 16);
            srcPortOut = ntohs(h->srcPort);

            int payLen = ntohs(h->payLen);

            //  "? "?  " <  Z   O  ,   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
            if (!(h->flags & CUDP_FLAG_FRAG)) {
                if (payLen > bufLen) payLen = bufLen;
                memcpy(buf, raw + CUDP_HDR, payLen);
                return payLen;
            }

            //  "? "?  <  Z   z     "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
            WORD fragId   = ntohs(h->fragId);
            WORD fragOff  = ntohs(h->fragOff);
            int  totalLen = ntohs(h->totalLen);

            auto& grp = fragBufs_[fragId];
            if (grp.totalLen == 0) {
                grp.totalLen  = totalLen;
                grp.data.resize(totalLen, 0);
                grp.received.assign(
                    (totalLen + CUDP_MTU_BODY - 1) / CUDP_MTU_BODY, false);
                grp.arrivedAt = GetTickCount();
            }

            int idx = fragOff / CUDP_MTU_BODY;
            if (idx < (int)grp.received.size() && !grp.received[idx]) {
                memcpy(grp.data.data() + fragOff, raw + CUDP_HDR, payLen);
                grp.received[idx] = true;
                grp.fragCount++;
            }

            if (h->flags & CUDP_FLAG_LAST) {
                grp.fragExpect = idx + 1;
            }

            //   "   <  Z   "   T. 
            if (grp.fragExpect > 0 && grp.fragCount == grp.fragExpect) {
                int copyLen = totalLen < bufLen ? totalLen : bufLen;
                memcpy(buf, grp.data.data(), copyLen);
                fragBufs_.erase(fragId);
                printf("[UdpSock]  z     T" O: %d bytes (%d  <  Z )\n",
                       totalLen, grp.fragCount);
                return copyLen;
            }
        }
    }

    //  "? "?  "" "   "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "? "?
    static void printHdr(const CudpHdr* h) {
        printf("  CUDP src=%u dst=%u fragId=%u fragOff=%u "
               "payLen=%u totalLen=%u flags=0x%02X\n",
               ntohs(h->srcPort), ntohs(h->dstPort),
               ntohs(h->fragId),  ntohs(h->fragOff),
               ntohs(h->payLen),  ntohs(h->totalLen),
               h->flags);
    }

private:
    SOCKET   sock_          = INVALID_SOCKET;
    USHORT   localPort_     = 0;
    WORD     fragIdCounter_ = 0;
    std::unordered_map<WORD, FragGroup> fragBufs_;

    bool createSocket() {
        if (sock_ != INVALID_SOCKET) return true;
        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) {
            printf("[UdpSock] socket  <  O : %d\n", WSAGetLastError());
            return false;
        }
        return true;
    }

    bool sendFragment(const char* dstIp, USHORT dstPort,
                      CudpType type, BYTE flags,
                      WORD fragId, WORD fragOff,
                      int totalLen,
                      const void* payload, int payLen) {
        char pkt[CUDP_HDR + CUDP_MTU_BODY];
        auto* h       = reinterpret_cast<CudpHdr*>(pkt);
        h->magic      = htonl(CUDP_MAGIC);
        h->type       = (BYTE)type;
        h->flags      = flags;
        h->srcPort    = htons(localPort_);
        h->dstPort    = htons(dstPort);
        h->fragId     = htons(fragId);
        h->fragOff    = htons(fragOff);
        h->payLen     = htons((WORD)payLen);
        h->totalLen   = htons((WORD)totalLen);
        h->checksum   = 0;
        h->reserved   = 0;
        memcpy(pkt + CUDP_HDR, payload, payLen);
        h->checksum   = calcChecksum(pkt, CUDP_HDR + payLen);

        sockaddr_in dst{};
        dst.sin_family = AF_INET;
        dst.sin_port   = htons(dstPort);
        inet_pton(AF_INET, dstIp, &dst.sin_addr);

        int r = sendto(sock_, pkt, CUDP_HDR + payLen, 0,
                       (sockaddr*)&dst, sizeof(dst));
        return r != SOCKET_ERROR;
    }

    static bool verifyChecksum(const char* raw, int totalBytes) {
        auto* h  = reinterpret_cast<const CudpHdr*>(raw);
        WORD  ck = ntohs(h->checksum);
        //    "   ." "o  0 o  o  ?"   z  " , 
        char tmp[CUDP_HDR + CUDP_MTU_BODY + 64];
        memcpy(tmp, raw, totalBytes);
        reinterpret_cast<CudpHdr*>(tmp)->checksum = 0;
        return calcChecksum(tmp, totalBytes) == ck;
    }
};
