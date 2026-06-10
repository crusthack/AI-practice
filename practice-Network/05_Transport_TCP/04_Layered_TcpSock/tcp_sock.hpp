#pragma once
#include "udp_sock.hpp"

//                                                                             
// TcpSock   UdpSock     TCP                .
//
//       :
//   UDP              TcpSock             :
//     1)        (3-way handshake: SYN   SYN+ACK   ACK)
//     2)         (stop-and-wait ARQ:           ACK         )
//     3)        (seq               )
//     4)        (window   ,      )
//     5)       (4-way teardown: FIN   ACK   FIN   ACK)
//
// API:
//     : passiveOpen(port)   send/recv   close
//        : activeOpen(ip, port)   send/recv   close
//                                                                             

static constexpr DWORD  CTCP_MAGIC     = 0x54435044u; // 'TCPD'
static constexpr int    CTCP_MSS       = 440;          // Max Segment Size (CUDP MTU -   )
static constexpr int    CTCP_MAX_DATA  = 65000;
static constexpr DWORD  CTCP_TIMEOUT   = 3000;         //          (ms)
static constexpr int    CTCP_MAX_RETRY = 5;

//    TCP                                                                     
enum : BYTE {
    CTCP_SYN = 0x01,
    CTCP_ACK = 0x02,
    CTCP_FIN = 0x04,
    CTCP_RST = 0x08,
    CTCP_PSH = 0x10,
};

//    TCP         (20 bytes)    UdpSock                          
#pragma pack(push, 1)
struct CtcpSeg {
    DWORD  magic;      // CTCP_MAGIC
    BYTE   flags;      // SYN/ACK/FIN/RST/PSH   
    BYTE   reserved;
    WORD   window;     //        (     :    1)
    DWORD  seq;        //        (           )
    DWORD  ackSeq;     //         
    DWORD  dataLen;    //               
    // Total: 4+1+1+2+4+4+4 = 20 bytes
};
#pragma pack(pop)

static constexpr int CTCP_SEG = sizeof(CtcpSeg);  // 20

//    TCP                                                                    
enum class TcpState : BYTE {
    CLOSED,
    LISTEN,        //         SYN   
    SYN_SENT,      //      : SYN      SYN_ACK   
    SYN_RCVD,      //   : SYN   , SYN_ACK      ACK   
    ESTABLISHED,   //          (         )
    FIN_WAIT_1,    // close()     : FIN      ACK   
    FIN_WAIT_2,    // FIN     ACK          FIN   
    CLOSE_WAIT,    //     FIN   :         close()           
    LAST_ACK,      //    close  :     FIN   , ACK   
    TIME_WAIT,     // 2*MSL      CLOSED (     :    CLOSED)
};

static const char* tcpStateName(TcpState s) {
    switch (s) {
    case TcpState::CLOSED:      return "CLOSED";
    case TcpState::LISTEN:      return "LISTEN";
    case TcpState::SYN_SENT:    return "SYN_SENT";
    case TcpState::SYN_RCVD:    return "SYN_RCVD";
    case TcpState::ESTABLISHED: return "ESTABLISHED";
    case TcpState::FIN_WAIT_1:  return "FIN_WAIT_1";
    case TcpState::FIN_WAIT_2:  return "FIN_WAIT_2";
    case TcpState::CLOSE_WAIT:  return "CLOSE_WAIT";
    case TcpState::LAST_ACK:    return "LAST_ACK";
    case TcpState::TIME_WAIT:   return "TIME_WAIT";
    }
    return "?";
}

static const char* ctcpFlagStr(BYTE f) {
    static char buf[32];
    int i = 0;
    if (f & CTCP_SYN) { buf[i++]='S'; buf[i++]='Y'; buf[i++]='N'; buf[i++]=' '; }
    if (f & CTCP_ACK) { buf[i++]='A'; buf[i++]='C'; buf[i++]='K'; buf[i++]=' '; }
    if (f & CTCP_FIN) { buf[i++]='F'; buf[i++]='I'; buf[i++]='N'; buf[i++]=' '; }
    if (f & CTCP_RST) { buf[i++]='R'; buf[i++]='S'; buf[i++]='T'; buf[i++]=' '; }
    if (f & CTCP_PSH) { buf[i++]='P'; buf[i++]='S'; buf[i++]='H'; buf[i++]=' '; }
    if (i == 0) { buf[0]='-'; i=1; }
    buf[i] = '\0';
    return buf;
}

//                                                                             
class TcpSock {
public:
    TcpSock() = default;
    ~TcpSock() { if (state_ != TcpState::CLOSED) close(); }

    TcpSock(const TcpSock&) = delete;
    TcpSock& operator=(const TcpSock&) = delete;

    TcpState state() const { return state_; }
    const char* stateName() const { return tcpStateName(state_); }

    //                                                         
    //            (passive open)
    //                                                         
    bool passiveOpen(USHORT port) {
        if (!udp_.bindPort(port)) return false;
        udp_.setRecvTimeout(0);   //       (SYN      )
        transition(TcpState::LISTEN);
        printf("[TcpSock]    %u        ...\n", port);

        //    SYN                                           
        CtcpSeg seg{};
        char body[CTCP_MSS + 4]{};
        int bodyLen = 0;
        while (true) {
            if (!recvSeg(seg, body, bodyLen)) continue;
            if (seg.flags & CTCP_SYN) break;
        }

        transition(TcpState::SYN_RCVD);
        hisSeq_ = ntohl(seg.seq) + 1;  // +1: SYN     1 seq   
        mySeq_  = 1000;                 //    ISN

        //    SYN+ACK                                       
        udp_.setRecvTimeout(CTCP_TIMEOUT);
        for (int attempt = 1; attempt <= CTCP_MAX_RETRY; ++attempt) {
            sendSeg(CTCP_SYN | CTCP_ACK);
            printf("[TcpSock]   [SYN+ACK] seq=%u ack=%u  (   %d)\n",
                   mySeq_, hisSeq_, attempt);

            bodyLen = 0;
            if (!recvSeg(seg, body, bodyLen)) {
                printf("[TcpSock] ACK     ,    \n");
                continue;
            }
            if (seg.flags & CTCP_ACK) break;
        }

        //    ACK      ESTABLISHED                          
        mySeq_++;   // SYN+ACK  1 seq   
        transition(TcpState::ESTABLISHED);
        return true;
    }

    //                                                         
    //            (active open)
    //                                                         
    bool activeOpen(const char* ip, USHORT port) {
        if (!udp_.open()) return false;
        udp_.setRecvTimeout(CTCP_TIMEOUT);

        strncpy_s(remoteIp_, sizeof(remoteIp_), ip, _TRUNCATE);
        remotePort_ = port;
        mySeq_ = 100;   //       ISN

        transition(TcpState::SYN_SENT);

        //    SYN                                           
        CtcpSeg resp{};
        char body[CTCP_MSS + 4]{};
        int bodyLen = 0;
        bool gotSynAck = false;

        for (int attempt = 1; attempt <= CTCP_MAX_RETRY && !gotSynAck; ++attempt) {
            sendSeg(CTCP_SYN);
            printf("[TcpSock]   [SYN] seq=%u  (   %d)\n", mySeq_, attempt);

            bodyLen = 0;
            if (!recvSeg(resp, body, bodyLen)) {
                printf("[TcpSock] SYN_ACK     ,    \n");
                continue;
            }
            if ((resp.flags & CTCP_SYN) && (resp.flags & CTCP_ACK))
                gotSynAck = true;
        }
        if (!gotSynAck) {
            transition(TcpState::CLOSED);
            return false;
        }

        //    ACK      ESTABLISHED                          
        hisSeq_ = ntohl(resp.seq) + 1;  //    ISN + SYN   
        mySeq_++;                         //       SYN   

        transition(TcpState::SYN_RCVD);
        sendSeg(CTCP_ACK);
        printf("[TcpSock]   [ACK] seq=%u ack=%u\n", mySeq_, hisSeq_);

        transition(TcpState::ESTABLISHED);
        return true;
    }

    //                                                         
    //        (ESTABLISHED   )
    //                                                         

    //      MSS                    .
    // stop-and-wait:             ACK                    .
    int send(const void* data, int len) {
        if (state_ != TcpState::ESTABLISHED) return -1;

        int sent = 0;
        while (sent < len) {
            int chunk = (len - sent > CTCP_MSS) ? CTCP_MSS : len - sent;
            const BYTE* ptr = (const BYTE*)data + sent;

            bool acked = false;
            for (int attempt = 1; attempt <= CTCP_MAX_RETRY && !acked; ++attempt) {
                // PSH:          (                    )
                sendSegWithData(CTCP_PSH | CTCP_ACK, ptr, chunk);
                printf("[TcpSock]   [PSH+ACK] seq=%u len=%d  (   %d)\n",
                       mySeq_, chunk, attempt);

                CtcpSeg ackSeg{};
                char tmp[4]{};
                int tmpLen = 0;
                if (!recvSeg(ackSeg, tmp, tmpLen)) {
                    printf("[TcpSock] ACK     ,    \n");
                    continue;
                }
                if ((ackSeg.flags & CTCP_ACK) &&
                    ntohl(ackSeg.ackSeq) == mySeq_ + (DWORD)chunk) {
                    mySeq_ += chunk;
                    acked = true;
                }
            }
            if (!acked) return sent;  //          
            sent += chunk;
        }
        return sent;
    }

    //                   .
    // ACK          .
    int recv(void* buf, int maxLen) {
        if (state_ != TcpState::ESTABLISHED &&
            state_ != TcpState::CLOSE_WAIT) return -1;

        udp_.setRecvTimeout(0);  //               
        while (true) {
            CtcpSeg seg{};
            char body[CTCP_MSS + 4]{};
            int bodyLen = 0;
            if (!recvSeg(seg, body, bodyLen)) return -1;

            BYTE flags = seg.flags;

            if (flags & CTCP_FIN) {
                //      FIN               
                hisSeq_++;
                sendSeg(CTCP_ACK);
                printf("[TcpSock]   [FIN]    [ACK]\n");
                transition(TcpState::CLOSE_WAIT);
                return 0;  // EOF
            }

            if (flags & CTCP_PSH) {
                int copyLen = bodyLen < maxLen ? bodyLen : maxLen;
                memcpy(buf, body, copyLen);
                hisSeq_ += copyLen;

                // ACK   
                udp_.setRecvTimeout(CTCP_TIMEOUT);
                sendSeg(CTCP_ACK);
                printf("[TcpSock]   [PSH] seq=%u len=%d    [ACK] ack=%u\n",
                       ntohl(seg.seq), bodyLen, hisSeq_);
                udp_.setRecvTimeout(0);
                return copyLen;
            }
        }
    }

    //                                                         
    //       (4-way teardown)
    //                                                         
    void close() {
        if (state_ == TcpState::ESTABLISHED) {
            //       close: FIN_WAIT_1                      
            transition(TcpState::FIN_WAIT_1);
            sendSeg(CTCP_FIN | CTCP_ACK);
            printf("[TcpSock]   [FIN+ACK]\n");

            udp_.setRecvTimeout(CTCP_TIMEOUT * 2);

            // ACK      FIN_WAIT_2
            CtcpSeg seg{};
            char tmp[4]{};
            int tmpLen = 0;
            if (recvSeg(seg, tmp, tmpLen) && (seg.flags & CTCP_ACK)) {
                transition(TcpState::FIN_WAIT_2);
                printf("[TcpSock]   [ACK]   FIN_WAIT_2\n");

                //     FIN      TIME_WAIT   CLOSED
                tmpLen = 0;
                if (recvSeg(seg, tmp, tmpLen) && (seg.flags & CTCP_FIN)) {
                    hisSeq_++;
                    sendSeg(CTCP_ACK);
                    printf("[TcpSock]   [FIN]    [ACK]    TIME_WAIT   CLOSED\n");
                }
            }
        } else if (state_ == TcpState::CLOSE_WAIT) {
            //       close: LAST_ACK                        
            transition(TcpState::LAST_ACK);
            sendSeg(CTCP_FIN | CTCP_ACK);
            printf("[TcpSock]   [FIN+ACK]\n");

            udp_.setRecvTimeout(CTCP_TIMEOUT * 2);
            CtcpSeg seg{};
            char tmp[4]{};
            int tmpLen = 0;
            if (recvSeg(seg, tmp, tmpLen) && (seg.flags & CTCP_ACK))
                printf("[TcpSock]   [ACK]   CLOSED\n");
        }

        transition(TcpState::CLOSED);
        udp_.close();
    }

    //            (passiveOpen             )                   
    void setRemote(const char* ip, USHORT port) {
        strncpy_s(remoteIp_, sizeof(remoteIp_), ip, _TRUNCATE);
        remotePort_ = port;
    }

private:
    UdpSock  udp_;
    TcpState state_      = TcpState::CLOSED;
    DWORD    mySeq_      = 0;
    DWORD    hisSeq_     = 0;
    char     remoteIp_[16]{};
    USHORT   remotePort_ = 0;

    void transition(TcpState next) {
        printf("[TcpSock] %s   %s\n", tcpStateName(state_), tcpStateName(next));
        state_ = next;
    }

    // CtcpSeg     (      )
    void sendSeg(BYTE flags) {
        CtcpSeg seg{};
        seg.magic   = htonl(CTCP_MAGIC);
        seg.flags   = flags;
        seg.window  = htons(1);
        seg.seq     = htonl(mySeq_);
        seg.ackSeq  = htonl(hisSeq_);
        seg.dataLen = 0;
        udp_.sendTo(remoteIp_, remotePort_, &seg, CTCP_SEG);
    }

    // CtcpSeg +       
    void sendSegWithData(BYTE flags, const void* data, int dataLen) {
        char buf[CTCP_SEG + CTCP_MSS];
        auto* seg   = reinterpret_cast<CtcpSeg*>(buf);
        seg->magic  = htonl(CTCP_MAGIC);
        seg->flags  = flags;
        seg->window = htons(1);
        seg->seq    = htonl(mySeq_);
        seg->ackSeq = htonl(hisSeq_);
        seg->dataLen= htonl((DWORD)dataLen);
        memcpy(buf + CTCP_SEG, data, dataLen);
        udp_.sendTo(remoteIp_, remotePort_, buf, CTCP_SEG + dataLen);
    }

    //         (UdpSock    )           remoteIp_/remotePort_    
    bool recvSeg(CtcpSeg& out, char* body, int& bodyLen) {
        char buf[CTCP_SEG + CTCP_MSS + 4]{};
        char srcIp[16]{};
        USHORT srcPort = 0;

        int r = udp_.recvFrom(srcIp, srcPort, buf, sizeof(buf));
        if (r < CTCP_SEG) return false;

        const auto* seg = reinterpret_cast<const CtcpSeg*>(buf);
        if (ntohl(seg->magic) != CTCP_MAGIC) return false;

        //                  (passiveOpen   SYN      )
        if (remoteIp_[0] == '\0') setRemote(srcIp, srcPort);

        out     = *seg;
        bodyLen = r - CTCP_SEG;
        if (bodyLen > 0) memcpy(body, buf + CTCP_SEG, bodyLen);

        printf("[TcpSock]   [%s] seq=%u ack=%u len=%d\n",
               ctcpFlagStr(seg->flags),
               ntohl(seg->seq), ntohl(seg->ackSeq), bodyLen);
        return true;
    }
};
