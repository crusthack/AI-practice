# Network Learning Roadmap

ì´ ë¡œë“œë§µì€ ì†Œì¼“ API ì‚¬ìš©ë²•ë³´ë‹¤ OSI/TCP/IP ê³„ì¸µê³¼ í”„ë¡œí† ì½œì˜ ì±…ìž„ì„ ë¨¼ì € ì´í•´í•˜ë„ë¡ êµ¬ì„±í•©ë‹ˆë‹¤. ê° ë‹¨ê³„ëŠ” ì´ì „ ë‹¨ê³„ì˜ PDUë¥¼ payloadë¡œ ë°›ì•„ ë‹¤ìŒ ê³„ì¸µì—ì„œ ì–´ë–¤ headerì™€ ê·œì¹™ì„ ì¶”ê°€í•˜ëŠ”ì§€ í™•ì¸í•©ë‹ˆë‹¤.

## ì „ì²´ íë¦„

| ìˆœì„œ | í´ë” | ê³„ì¸µ | í•µì‹¬ ì£¼ì œ | ì‚°ì¶œë¬¼ |
| --- | --- | --- | --- | --- |
| 1 | `01_DataLink_Ethernet_ARP` | Data Link / Link | Ethernet frame, MAC, EtherType, ARP | Ethernet/ARP frame field map |
| 2 | `02_Network_IP_ICMP` | Network / Internet | IPv4, ICMP, TTL, checksum | IPv4/ICMP packet field map |
| 3 | `03_Network_Multicast` | Network / Internet | multicast group, TTL scope | unicast vs multicast ë¹„êµ ë…¸íŠ¸ |
| 4 | `04_Transport_UDP` | Transport | UDP datagram, port, missing reliability | UDP headerì™€ custom reliability ì„¤ê³„ |
| 5 | `05_Transport_TCP` | Transport | TCP stream, handshake, ACK, teardown | TCP lifecycleì™€ stream íŠ¹ì„± ì •ë¦¬ |
| 6 | `06_Framing_BinaryProtocol` | Session/Presentation ê´€ì  | message boundary, binary header, byte order | custom binary protocol spec |
| 7 | `07_Application_HTTP` | Application | HTTP request/response, headers, body | HTTP message parser ê´€ì°° ê¸°ë¡ |
| 8 | `08_Security_TLS` | Presentation/Security ê´€ì  | TLS handshake, certificate, encrypted record | TLSê°€ ë³´í˜¸í•˜ëŠ” ë²”ìœ„ ì •ë¦¬ |
| 9 | `09_Implementation_IOModels` | Implementation | select/event/overlapped/IOCP | IO model ë¹„êµí‘œ |
| 10 | `10_Platform_WinNetAPI` | Platform API | WinHTTP, WebSocket, Named Pipe | high-level APIì™€ protocol ë¹„êµ |

## ê³„ì¸µë³„ í•µì‹¬ ì§ˆë¬¸

| ê³„ì¸µ | ì§ˆë¬¸ |
| --- | --- |
| Data Link | ê°™ì€ local linkì—ì„œ ëª©ì ì§€ëŠ” ì–´ë–¤ MAC ì£¼ì†Œë¡œ ì „ë‹¬ë˜ëŠ”ê°€? |
| Network | IP ì£¼ì†Œ, TTL, protocol fieldëŠ” packet forwardingê³¼ parsingì— ì–´ë–¤ ì—­í• ì„ í•˜ëŠ”ê°€? |
| Transport | UDPì™€ TCPëŠ” applicationì— ì–´ë–¤ ë³´ìž¥ ë˜ëŠ” ë¹„ë³´ìž¥ì„ ì œê³µí•˜ëŠ”ê°€? |
| Framing | TCP stream ìœ„ì—ì„œ application message ê²½ê³„ëŠ” ì–´ë–»ê²Œ ì •ì˜í•˜ëŠ”ê°€? |
| Application | HTTPëŠ” TCP byte stream ìœ„ì—ì„œ request/responseë¥¼ ì–´ë–¤ ê·œì¹™ìœ¼ë¡œ ë‚˜ëˆ„ëŠ”ê°€? |
| Security | TLSëŠ” TCPì™€ HTTP ì‚¬ì´ì—ì„œ ë¬´ì—‡ì„ ìˆ¨ê¸°ê³  ë¬´ì—‡ì„ ê²€ì¦í•˜ëŠ”ê°€? |
| Implementation | IO modelì€ protocol ë³´ìž¥ì„ ë°”ê¾¸ëŠ”ê°€, ì•„ë‹ˆë©´ ì²˜ë¦¬ ë°©ì‹ì„ ë°”ê¾¸ëŠ”ê°€? |

## ê¶Œìž¥ í•™ìŠµ ë¦¬ë“¬

1. ë¨¼ì € í•´ë‹¹ ë‹¨ê³„ READMEë¥¼ ì½ìŠµë‹ˆë‹¤.
2. í”„ë¡œì íŠ¸ READMEì˜ ì‹¤í–‰ ë°©ë²•ê³¼ ê´€ì°° í¬ì¸íŠ¸ë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
3. ì½”ë“œë¥¼ ë³´ê¸° ì „ì— header layoutì„ ì†ìœ¼ë¡œ ì ìŠµë‹ˆë‹¤.
4. ì½”ë“œë¥¼ ì‹¤í–‰í•˜ê³  ì¶œë ¥ì˜ fieldë¥¼ header layoutì— ëŒ€ì‘ì‹œí‚µë‹ˆë‹¤.
5. Wiresharkë¡œ ì‹¤ì œ packetì„ ë¹„êµí•©ë‹ˆë‹¤. ì˜¤í”„ë¼ì¸ parser ì‹¤ìŠµì€ ì˜ˆì œ byte arrayì™€ ë¹„êµí•©ë‹ˆë‹¤.
6. ì™„ë£Œ ê¸°ì¤€ì„ ë§ë¡œ ì„¤ëª…í•œ ë’¤ ë‹¤ìŒ ë‹¨ê³„ë¡œ ì´ë™í•©ë‹ˆë‹¤.

## ìµœì†Œ ì™„ë£Œ ê¸°ì¤€

- Ethernet frame, ARP packet, IPv4 packet, ICMP message, UDP header, TCP headerì˜ í•µì‹¬ í•„ë“œë¥¼ ì½ì„ ìˆ˜ ìžˆë‹¤.
- TCPì™€ UDPì˜ ì°¨ì´ë¥¼ API ì´ë¦„ì´ ì•„ë‹ˆë¼ protocol guarantee ê¸°ì¤€ìœ¼ë¡œ ì„¤ëª…í•  ìˆ˜ ìžˆë‹¤.
- TCP stream ìœ„ì— application framingì´ í•„ìš”í•œ ì´ìœ ë¥¼ ì˜ˆì œë¡œ ì„¤ëª…í•  ìˆ˜ ìžˆë‹¤.
- HTTP request/responseë¥¼ start line, headers, empty line, bodyë¡œ ë‚˜ëˆŒ ìˆ˜ ìžˆë‹¤.
- TLSê°€ HTTP messageë¥¼ ë³´í˜¸í•˜ëŠ” ìœ„ì¹˜ì™€ í•œê³„ë¥¼ ì„¤ëª…í•  ìˆ˜ ìžˆë‹¤.
