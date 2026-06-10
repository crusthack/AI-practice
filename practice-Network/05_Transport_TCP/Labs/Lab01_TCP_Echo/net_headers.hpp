#pragma once
//                                                                            
// net_headers.hpp                      (RFC 791/792/793/768)
//      Lab            .
//   #pragma pack(1)                     1:1      .
//                                                                            
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)

//     IPv4    (RFC 791)                                                      
//    IP          20    . raw socket recvfrom        .
struct IPv4Header {
    BYTE  verIhl;    // [7:4]=  (4), [3:0]=IHL(4           )
    BYTE  tos;       // DSCP+ECN
    WORD  totalLen;  // IP  +       (network byte order)
    WORD  id;        //        
    WORD  flagFrag;  // [15:13]=   (  /DF/MF), [12:0]=     (8B  )
    BYTE  ttl;
    BYTE  protocol;  // 1=ICMP  6=TCP  17=UDP
    WORD  checksum;
    DWORD srcAddr;   // network byte order
    DWORD dstAddr;

    int  hdrLen()    const { return (verIhl & 0x0F) * 4; }
    int  version()   const { return  verIhl >> 4; }
    bool dontFrag()  const { return (ntohs(flagFrag) & 0x4000) != 0; }
    bool moreFrags() const { return (ntohs(flagFrag) & 0x2000) != 0; }
    int  fragOff()   const { return (ntohs(flagFrag) & 0x1FFF) * 8; }
    int  dataLen()   const { return ntohs(totalLen) - hdrLen(); }
};

//     ICMP    (RFC 792)                                                       
struct IcmpHeader {
    BYTE type;       // 0=Echo Reply  8=Echo Req  3=Dest Unreach  11=TTL Exceed
    BYTE code;
    WORD checksum;   // ICMP  +       
    WORD id;
    WORD seq;

    static const char* typeName(BYTE t) {
        switch(t){
        case  0: return "Echo Reply";
        case  3: return "Dest Unreachable";
        case  8: return "Echo Request";
        case 11: return "TTL Exceeded";
        default: return "Other";
        }
    }
};

//     TCP    (RFC 793)                                                        
struct TcpHeader {
    WORD  srcPort, dstPort;
    DWORD seq, ackSeq;
    BYTE  dataOff;   // [7:4]=    (4B  )
    BYTE  flags;     // CWR URG ACK PSH RST SYN FIN (MSB LSB)
    WORD  window;
    WORD  checksum;
    WORD  urgPtr;

    int  hdrLen() const { return ((dataOff >> 4) & 0xF) * 4; }
    bool hasSYN() const { return (flags & 0x02) != 0; }
    bool hasACK() const { return (flags & 0x10) != 0; }
    bool hasFIN() const { return (flags & 0x01) != 0; }
    bool hasRST() const { return (flags & 0x04) != 0; }
    bool hasPSH() const { return (flags & 0x08) != 0; }

    const char* flagStr() const {
        static char buf[24];
        int i=0;
        if(hasSYN()){buf[i++]='S';buf[i++]='Y';buf[i++]='N';buf[i++]=' ';}
        if(hasACK()){buf[i++]='A';buf[i++]='C';buf[i++]='K';buf[i++]=' ';}
        if(hasFIN()){buf[i++]='F';buf[i++]='I';buf[i++]='N';buf[i++]=' ';}
        if(hasRST()){buf[i++]='R';buf[i++]='S';buf[i++]='T';buf[i++]=' ';}
        if(hasPSH()){buf[i++]='P';buf[i++]='S';buf[i++]='H';buf[i++]=' ';}
        if(i==0){buf[0]='-';i=1;}
        buf[i]='\0';
        return buf;
    }
};

//     UDP    (RFC 768)                                                        
struct UdpHeader {
    WORD srcPort, dstPort;
    WORD length;     // UDP  (8)+   
    WORD checksum;
};

#pragma pack(pop)

//                                                                          

// RFC 1071           IP/ICMP/TCP/UDP   
//         0        ,              
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
    switch(p){ case 1:return"ICMP"; case 6:return"TCP"; case 17:return"UDP"; }
    return "OTHER";
}

inline void ip4Str(DWORD addr, char* out16) { inet_ntop(AF_INET, &addr, out16, 16); }

//                 (raw recv    )
inline const IPv4Header* pktIP  (const void* p){ return reinterpret_cast<const IPv4Header*>(p); }
inline const IcmpHeader* pktICMP(const void* p){
    auto* ip=pktIP(p); return reinterpret_cast<const IcmpHeader*>((const BYTE*)p+ip->hdrLen()); }
inline const TcpHeader*  pktTCP (const void* p){
    auto* ip=pktIP(p); return reinterpret_cast<const TcpHeader*> ((const BYTE*)p+ip->hdrLen()); }
inline const UdpHeader*  pktUDP (const void* p){
    auto* ip=pktIP(p); return reinterpret_cast<const UdpHeader*> ((const BYTE*)p+ip->hdrLen()); }

// 16      (    )
inline void hexDump(const void* data, int len, const char* label="") {
    const BYTE* p = reinterpret_cast<const BYTE*>(data);
    if(label[0]) printf("[%s] %d bytes:\n", label, len);
    for(int i=0; i<len; ++i){
        if(i%16==0) printf("  %04X: ", i);
        printf("%02X ", p[i]);
        if(i%16==15||i==len-1) printf("\n");
    }
}
