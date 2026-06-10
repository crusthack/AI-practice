#pragma once
//    1:                  +        
//          (UdpSock, TcpSock, HTTP)             .
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

//      :                       1:1                        
#pragma pack(push, 1)

//                                                         
// IPv4    (RFC 791,    20 bytes)
//                                                         
struct IpHeader {
    BYTE  verIhl;     // [7:4]=  (4), [3:0]=IHL(4            )
    BYTE  tos;        // DSCP(6b) + ECN(2b)
    WORD  totalLen;   // IP  +          (           )
    WORD  id;         //        
    WORD  flagFrag;   // [15:13]=   (  /DF/MF), [12:0]=      (8B   )
    BYTE  ttl;        // Time To Live
    BYTE  protocol;   // 1=ICMP  6=TCP  17=UDP
    WORD  checksum;   //           (RFC 1071)
    DWORD srcAddr;    //            
    DWORD dstAddr;

    int  version()    const { return verIhl >> 4; }
    int  headerLen()  const { return (verIhl & 0x0F) * 4; }
    bool dontFrag()   const { return (ntohs(flagFrag) & 0x4000) != 0; }
    bool moreFrags()  const { return (ntohs(flagFrag) & 0x2000) != 0; }
    int  fragOffset() const { return (ntohs(flagFrag) & 0x1FFF) * 8; }
    int  dataLen()    const { return ntohs(totalLen) - headerLen(); }
};

//                                                         
// ICMP    (RFC 792, 8 bytes)
//                                                         
struct IcmpHeader {
    BYTE  type;       // 0=Echo Reply  3=Dest Unreachable  8=Echo Request  11=TTL Exceeded
    BYTE  code;       //       (          )
    WORD  checksum;   // ICMP    +        
    WORD  id;         // Echo:     /     
    WORD  seq;        // Echo:      

    static const char* typeName(BYTE t) {
        switch (t) {
        case 0:  return "Echo Reply";
        case 3:  return "Dest Unreachable";
        case 8:  return "Echo Request";
        case 11: return "TTL Exceeded";
        default: return "Other";
        }
    }
};

//                                                         
// TCP    (RFC 793,    20 bytes)
//                                                         
struct TcpHeader {
    WORD  srcPort;
    WORD  dstPort;
    DWORD seq;        //       
    DWORD ackSeq;     //          (ACK           )
    BYTE  dataOffset; // [7:4]=     (4      ), [3:0]=  
    BYTE  flags;      // CWR URG ACK PSH RST SYN FIN (MSB   LSB)
    WORD  window;     //       
    WORD  checksum;
    WORD  urgPtr;

    int  headerLen() const { return ((dataOffset >> 4) & 0xF) * 4; }
    bool isSYN()     const { return (flags & 0x02) != 0; }
    bool isACK()     const { return (flags & 0x10) != 0; }
    bool isFIN()     const { return (flags & 0x01) != 0; }
    bool isRST()     const { return (flags & 0x04) != 0; }
    bool isPSH()     const { return (flags & 0x08) != 0; }

    void printFlags(char* out) const {
        int i = 0;
        if (isSYN()) { out[i++]='S'; out[i++]='Y'; out[i++]='N'; out[i++]=' '; }
        if (isACK()) { out[i++]='A'; out[i++]='C'; out[i++]='K'; out[i++]=' '; }
        if (isFIN()) { out[i++]='F'; out[i++]='I'; out[i++]='N'; out[i++]=' '; }
        if (isRST()) { out[i++]='R'; out[i++]='S'; out[i++]='T'; out[i++]=' '; }
        if (isPSH()) { out[i++]='P'; out[i++]='S'; out[i++]='H'; out[i++]=' '; }
        out[i] = '\0';
    }
};

//                                                         
// UDP    (RFC 768, 8 bytes)
//                                                         
struct UdpHeader {
    WORD srcPort;
    WORD dstPort;
    WORD length;    // UDP   (8) +    
    WORD checksum;  // 0           (IPv4      )
};

#pragma pack(pop)

//                                                         
//         (inline              )
//                                                         

// RFC 1071         (IP/ICMP/TCP/UDP   )
//         0          ,                   .
inline WORD calcChecksum(const void* data, int len) {
    const WORD* p = reinterpret_cast<const WORD*>(data);
    DWORD sum = 0;
    while (len > 1) { sum += *p++; len -= 2; }
    if (len == 1) sum += *(const BYTE*)p;
    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return static_cast<WORD>(~sum);
}

inline const char* protoName(BYTE p) {
    switch (p) { case 1: return "ICMP"; case 6: return "TCP"; case 17: return "UDP"; }
    return "OTHER";
}

inline void ipToStr(DWORD addr, char* out16) {
    inet_ntop(AF_INET, &addr, out16, 16);
}

//                          (raw recv    )
inline const IpHeader*   pktIp(const void* p)  { return reinterpret_cast<const IpHeader*>(p); }
inline const IcmpHeader* pktIcmp(const void* p) {
    const auto* ip = pktIp(p);
    return reinterpret_cast<const IcmpHeader*>((const BYTE*)p + ip->headerLen());
}
inline const TcpHeader*  pktTcp(const void* p) {
    const auto* ip = pktIp(p);
    return reinterpret_cast<const TcpHeader*>((const BYTE*)p + ip->headerLen());
}
inline const UdpHeader*  pktUdp(const void* p) {
    const auto* ip = pktIp(p);
    return reinterpret_cast<const UdpHeader*>((const BYTE*)p + ip->headerLen());
}

//           (    )
inline void hexDump(const void* data, int len, const char* label = "") {
    const BYTE* p = reinterpret_cast<const BYTE*>(data);
    if (label[0]) printf("[%s] %d bytes:\n", label, len);
    for (int i = 0; i < len; ++i) {
        if (i % 16 == 0) printf("  %04X: ", i);
        printf("%02X ", p[i]);
        if (i % 16 == 15 || i == len - 1) printf("\n");
    }
}
