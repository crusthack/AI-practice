# Curriculum

ì´ ì»¤ë¦¬í˜ëŸ¼ì€ 8ê°œ í•µì‹¬ ë‹¨ê³„ì™€ 2ê°œ ì„ íƒ ë‹¨ê³„ë¡œ êµ¬ì„±ë©ë‹ˆë‹¤. ê° ë‹¨ê³„ëŠ” ê°œë… í•™ìŠµ, ì½”ë“œ ê´€ì°°, ì‹¤í–‰ ì‹¤ìŠµ, ì •ë¦¬ ê³¼ì œë¡œ ëë‚©ë‹ˆë‹¤.

## 1. Data Link: Ethernetê³¼ ARP

ê´€ë ¨ í´ë”: `01_DataLink_Ethernet_ARP`

í•™ìŠµ ëª©í‘œ:

- Ethernet II header 14ë°”ì´íŠ¸ë¥¼ destination MAC, source MAC, EtherTypeìœ¼ë¡œ ë‚˜ëˆˆë‹¤.
- broadcast, multicast, unicast MAC ì£¼ì†Œë¥¼ êµ¬ë¶„í•œë‹¤.
- ARP request/replyê°€ IPv4 ì£¼ì†Œì™€ MAC ì£¼ì†Œë¥¼ ì–´ë–»ê²Œ ì—°ê²°í•˜ëŠ”ì§€ ì„¤ëª…í•œë‹¤.

ì‹¤ìŠµ:

- `01_Ethernet_ARP_Frame`: ì •ì  byte arrayì—ì„œ Ethernet, ARP, IPv4 payloadë¥¼ íŒŒì‹±í•œë‹¤.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- ARP requestê°€ broadcastì´ê³  ARP replyê°€ ì¼ë°˜ì ìœ¼ë¡œ unicastì¸ ì´ìœ ë¥¼ ì„¤ëª…í•œ ë…¸íŠ¸.

## 2. Internet: IPv4ì™€ ICMP

ê´€ë ¨ í´ë”: `02_Network_IP_ICMP`

í•™ìŠµ ëª©í‘œ:

- IPv4 headerì˜ version, IHL, total length, TTL, protocol, checksum, source/destinationì„ ì½ëŠ”ë‹¤.
- ICMP Echo Request/Replyì˜ type, code, id, sequenceë¥¼ êµ¬ë¶„í•œë‹¤.
- raw socketì´ ê´€ë¦¬ìž ê¶Œí•œì„ ìš”êµ¬í•  ìˆ˜ ìžˆëŠ” ì´ìœ ë¥¼ ì´í•´í•œë‹¤.

ì‹¤ìŠµ:

- `01_RawSocket`: raw socketì˜ ì œì•½ê³¼ IP/ICMP ê´€ì°°.
- `02_Layered_RawICMP`: ICMP Echo êµ¬ì¡° íŒŒì•….
- `03_Layered_RawSocket`: IPv4 protocol fieldì— ë”°ë¥¸ TCP/UDP/ICMP ë¶„ê¸° íŒŒì‹±.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- IPv4 header field í‘œì™€ ICMP Echo Request/Reply ë¹„êµí‘œ.

## 3. Internet Extension: Multicast

ê´€ë ¨ í´ë”: `03_Network_Multicast`

í•™ìŠµ ëª©í‘œ:

- unicast, broadcast, multicast ëª©ì ì§€ì˜ ì°¨ì´ë¥¼ ì„¤ëª…í•œë‹¤.
- multicast group joinì˜ ì˜ë¯¸ë¥¼ ì´í•´í•œë‹¤.
- multicast TTLì´ ë²”ìœ„ ì œí•œì— ì–´ë–¤ ì—­í• ì„ í•˜ëŠ”ì§€ í™•ì¸í•œë‹¤.

ì‹¤ìŠµ:

- `01_Multicast`: UDP multicast receiverë¥¼ ë¨¼ì € ì‹¤í–‰í•˜ê³  senderë¡œ groupì— datagramì„ ë³´ë‚¸ë‹¤.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- groupì— ê°€ìž…í•œ receiverì™€ ê°€ìž…í•˜ì§€ ì•Šì€ receiverì˜ ì°¨ì´ ì •ë¦¬.

## 4. Transport: UDP

ê´€ë ¨ í´ë”: `04_Transport_UDP`

í•™ìŠµ ëª©í‘œ:

- UDP datagram boundaryê°€ ìœ ì§€ëœë‹¤ëŠ” ì ì„ í™•ì¸í•œë‹¤.
- portê°€ host ë‚´ë¶€ application endpointë¥¼ ì‹ë³„í•œë‹¤ëŠ” ì ì„ ì´í•´í•œë‹¤.
- UDPê°€ ì œê³µí•˜ì§€ ì•ŠëŠ” ìˆœì„œ ë³´ìž¥, ìž¬ì „ì†¡, íë¦„ ì œì–´, í˜¼ìž¡ ì œì–´ë¥¼ ì„¤ëª…í•œë‹¤.

ì‹¤ìŠµ:

- `01_UDP_Echo`: UDP echo ê¸°ë³¸ êµ¬ì¡°.
- `02_Layered_UdpSock`: UDP ìœ„ custom headerì™€ fragmentation/reassembly.
- `03_Layered_CustomUDP`: UDP ìœ„ application protocol ì„¤ê³„.
- `Labs/Lab01_UDP_Echo`: ë‹¨ê³„í˜• UDP echo.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- UDP header 8ë°”ì´íŠ¸ ì„¤ëª…ê³¼ custom reliabilityì— í•„ìš”í•œ field ëª©ë¡.

## 5. Transport: TCP

ê´€ë ¨ í´ë”: `05_Transport_TCP`

í•™ìŠµ ëª©í‘œ:

- TCP 3-way handshakeì™€ graceful teardownì„ ì„¤ëª…í•œë‹¤.
- TCPê°€ messageê°€ ì•„ë‹ˆë¼ byte streamì„ ì œê³µí•œë‹¤ëŠ” ì ì„ ì´í•´í•œë‹¤.
- socket optionì´ protocol guarantee ìžì²´ë¥¼ ë°”ê¾¸ì§€ëŠ” ì•ŠëŠ”ë‹¤ëŠ” ì ì„ êµ¬ë¶„í•œë‹¤.

ì‹¤ìŠµ:

- `01_TCP_Server`: server lifecycle.
- `02_TCP_Client`: connect/send/recv/shutdown.
- `03_Socket_Options`: timeout, keepalive, TCP_NODELAY ë“±.
- `04_Layered_TcpSock`: stream wrapper.
- `05_TCP_Relay`: ì–‘ë°©í–¥ byte forwarding.
- `Labs/Lab01_TCP_Echo`, `Labs/Lab02_SockOpts`: ë‹¨ê³„í˜• lab.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- TCP lifecycle diagramê³¼ TCP streamì—ì„œ message boundaryê°€ ì—†ëŠ” ì´ìœ  ì„¤ëª….

## 6. Framing and Binary Protocol

ê´€ë ¨ í´ë”: `06_Framing_BinaryProtocol`

í•™ìŠµ ëª©í‘œ:

- TCP stream ìœ„ì—ì„œ application message boundaryë¥¼ ë§Œë“œëŠ” ë°©ë²•ì„ ì´í•´í•œë‹¤.
- magic, version, type, length, checksum fieldì˜ ëª©ì ì„ ì„¤ëª…í•œë‹¤.
- partial recvì™€ sticky packet ìƒí™©ì— ëŒ€ì‘í•œë‹¤.

ì‹¤ìŠµ:

- `01_Custom_Protocol`: ê³ ì • header + ê°€ë³€ payload.
- `02_Layered_TCPFraming`: length-prefix framingê³¼ `recvExact`.
- `Labs/Lab01_BinaryProto`: ë‹¨ê³„í˜• binary protocol.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- custom binary protocol header specification.

## 7. Application: HTTP

ê´€ë ¨ í´ë”: `07_Application_HTTP`

í•™ìŠµ ëª©í‘œ:

- HTTP request line, status line, header, bodyë¥¼ êµ¬ë¶„í•œë‹¤.
- `Content-Length`ê°€ body boundaryë¥¼ ì•Œë ¤ì£¼ëŠ” ë°©ì‹ì„ ì´í•´í•œë‹¤.
- keep-aliveì™€ TCP connection ìž¬ì‚¬ìš©ì˜ ê´€ê³„ë¥¼ ì„¤ëª…í•œë‹¤.

ì‹¤ìŠµ:

- `01_HTTP_Client`: raw TCP socketìœ¼ë¡œ HTTP GET ì „ì†¡.
- `02_HTTP_Server`: ê°„ë‹¨í•œ HTTP response ìƒì„±.
- `03_Layered_HTTP`: parser/router/server/client ë¶„ë¦¬.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- HTTP request/response sampleì„ field ë‹¨ìœ„ë¡œ ë¶„í•´í•œ ë¬¸ì„œ.

## 8. Security: TLS

ê´€ë ¨ í´ë”: `08_Security_TLS`

í•™ìŠµ ëª©í‘œ:

- TLSê°€ TCPì™€ HTTP ì‚¬ì´ì—ì„œ ì œê³µí•˜ëŠ” ê¸°ë°€ì„±, ë¬´ê²°ì„±, ì¸ì¦ì„ êµ¬ë¶„í•œë‹¤.
- TLS handshakeì™€ HTTP requestì˜ ìˆœì„œë¥¼ êµ¬ë¶„í•œë‹¤.
- HTTPSì—ì„œ HTTP header/bodyê°€ í‰ë¬¸ìœ¼ë¡œ ë³´ì´ì§€ ì•ŠëŠ” ì´ìœ ë¥¼ ì„¤ëª…í•œë‹¤.

ì‹¤ìŠµ:

- `01_TLS_Schannel`: Schannel ê¸°ë°˜ TLS client.

ì™„ë£Œ ì‚°ì¶œë¬¼:

- TCP connect, TLS handshake, HTTP request ìˆœì„œë„.

## 9. Optional: IO Models

ê´€ë ¨ í´ë”: `09_Implementation_IOModels`

í•™ìŠµ ëª©í‘œ:

- readiness ê¸°ë°˜ ëª¨ë¸ê³¼ completion ê¸°ë°˜ ëª¨ë¸ì„ êµ¬ë¶„í•œë‹¤.
- IO modelì´ protocol headerë‚˜ guaranteeë¥¼ ë°”ê¾¸ì§€ ì•ŠëŠ”ë‹¤ëŠ” ì ì„ ì´í•´í•œë‹¤.

ì‹¤ìŠµ:

- `01_Select_Model`, `02_WSAEventSelect`, `03_OverlappedIO`, `04_IOCP`
- `Labs/Lab01_Select`, `Labs/Lab02_EventSelect`, `Labs/Lab03_IOCP_Intro`

ì™„ë£Œ ì‚°ì¶œë¬¼:

- select/event/overlapped/IOCP ë¹„êµí‘œ.

## 10. Optional: Windows Platform APIs

ê´€ë ¨ í´ë”: `10_Platform_WinNetAPI`

í•™ìŠµ ëª©í‘œ:

- high-level APIê°€ protocol ì„¸ë¶€ êµ¬í˜„ì„ ê°ì‹¼ë‹¤ëŠ” ì ì„ ì´í•´í•œë‹¤.
- WebSocketì´ HTTP Upgrade ì´í›„ message frameìœ¼ë¡œ ë™ìž‘í•œë‹¤ëŠ” ì ì„ ì„¤ëª…í•œë‹¤.
- Named Pipeê°€ TCP/IP protocol ì‹¤ìŠµì´ ì•„ë‹ˆë¼ IPC ë¹„êµ ì‹¤ìŠµìž„ì„ êµ¬ë¶„í•œë‹¤.

ì‹¤ìŠµ:

- `01_WinHTTP`, `02_WebSocket`, `03_NamedPipe_Net`

ì™„ë£Œ ì‚°ì¶œë¬¼:

- raw socket êµ¬í˜„ê³¼ high-level API êµ¬í˜„ì˜ ìž¥ë‹¨ì  ë¹„êµ.
