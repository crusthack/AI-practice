# Detailed Practice Plan

ê° ì‹¤ìŠµì€ ê°™ì€ í˜•ì‹ìœ¼ë¡œ ì§„í–‰í•©ë‹ˆë‹¤: ì¤€ë¹„, ì‹¤í–‰, ê´€ì°°, ë³€í˜•, ì •ë¦¬.

## ê³µí†µ ì¤€ë¹„

- Visual Studio ë˜ëŠ” MSBuildê°€ ê°€ëŠ¥í•œ ê°œë°œìž ëª…ë ¹ í”„ë¡¬í”„íŠ¸.
- Wireshark. loopback captureê°€ í•„ìš”í•˜ë©´ Npcap loopback supportê°€ í•„ìš”í•  ìˆ˜ ìžˆìŠµë‹ˆë‹¤.
- raw socket ì‹¤ìŠµì€ ê´€ë¦¬ìž ê¶Œí•œì´ í•„ìš”í•  ìˆ˜ ìžˆìŠµë‹ˆë‹¤.
- ì™¸ë¶€ ë„¤íŠ¸ì›Œí¬ê°€ í•„ìš”í•œ ì‹¤ìŠµ: TLS, WinHTTP, WebSocket.

## 1. `01_DataLink_Ethernet_ARP`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_Ethernet_ARP_Frame`

ì§„í–‰:

1. `FRAME_LAYOUT.md`ì—ì„œ Ethernet IIì™€ ARP layoutì„ ì½ìŠµë‹ˆë‹¤.
2. `main.cpp`ì˜ ARP request, ARP reply, IPv4 frame byte arrayë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
3. destination MAC, source MAC, EtherType offsetì„ ì†ìœ¼ë¡œ í‘œì‹œí•©ë‹ˆë‹¤.
4. ì‹¤í–‰ ê²°ê³¼ì—ì„œ broadcast/unicast íŒì •ì„ í™•ì¸í•©ë‹ˆë‹¤.
5. EtherTypeì„ ë°”ê¿” parser ë¶„ê¸°ê°€ ë°”ë€ŒëŠ”ì§€ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- `ff:ff:ff:ff:ff:ff`ê°€ broadcastì¸ ì´ìœ .
- ARP requestì˜ target MACì´ ë¹„ì–´ ìžˆëŠ” ì´ìœ .
- `0x0806`ê³¼ `0x0800`ì˜ ì°¨ì´.

## 2. `02_Network_IP_ICMP`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_RawSocket`
- `02_Layered_RawICMP`
- `03_Layered_RawSocket`

ì§„í–‰:

1. ê´€ë¦¬ìž ê¶Œí•œ ì½˜ì†”ì„ ì¤€ë¹„í•©ë‹ˆë‹¤.
2. ICMP packetì„ ë³´ë‚´ê³  IPv4/ICMP headerë¥¼ ì¶œë ¥í•©ë‹ˆë‹¤.
3. Wiresharkì—ì„œ `icmp` í•„í„°ë¡œ Echo Request/Replyë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
4. `protocol` fieldê°€ ICMP/TCP/UDP parserë¥¼ ì„ íƒí•˜ëŠ” íë¦„ì„ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- IPv4 header length ê³„ì‚°.
- TTL ê°ì†Œì˜ ì˜ë¯¸.
- ICMP type 8ê³¼ type 0ì˜ ì°¨ì´.

## 3. `03_Network_Multicast`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_Multicast`

ì§„í–‰:

1. receiverë¥¼ ë¨¼ì € ì‹¤í–‰í•©ë‹ˆë‹¤.
2. senderë¥¼ ì‹¤í–‰í•´ multicast group addressë¡œ datagramì„ ë³´ëƒ…ë‹ˆë‹¤.
3. Wiresharkì—ì„œ multicast destination IPì™€ UDP portë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
4. receiverë¥¼ ë‘ ê°œ ì‹¤í–‰í•´ ê°™ì€ datagramì´ ìˆ˜ì‹ ë˜ëŠ”ì§€ ë´…ë‹ˆë‹¤.

ì²´í¬:

- group joinì´ receiver ì¸¡ ë™ìž‘ì¸ ì´ìœ .
- multicast senderê°€ receiver ëª©ë¡ì„ ëª°ë¼ë„ ë˜ëŠ” ì´ìœ .
- TTLì´ multicast ë²”ìœ„ì— ì£¼ëŠ” ì˜í–¥.

## 4. `04_Transport_UDP`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_UDP_Echo`
- `02_Layered_UdpSock`
- `03_Layered_CustomUDP`
- `Labs/Lab01_UDP_Echo`

ì§„í–‰:

1. UDP echo server/clientë¥¼ ì‹¤í–‰í•©ë‹ˆë‹¤.
2. `sendto`ì™€ `recvfrom`ì˜ ì£¼ì†Œ ì¸ìžë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
3. í° messageë¥¼ custom UDP ì‹¤ìŠµì—ì„œ ë³´ë‚´ fragmentation/reassembly fieldë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
4. ì¼ë¶€ packetì„ ë¬´ì‹œí•˜ëŠ” ë³€í˜•ì„ ë§Œë“¤ì–´ timeout/retry í•„ìš”ì„±ì„ ì •ë¦¬í•©ë‹ˆë‹¤.

ì²´í¬:

- UDP header 8ë°”ì´íŠ¸.
- datagram boundary ìœ ì§€.
- UDPê°€ ì œê³µí•˜ì§€ ì•ŠëŠ” guarantee ëª©ë¡.

## 5. `05_Transport_TCP`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_TCP_Server`
- `02_TCP_Client`
- `03_Socket_Options`
- `04_Layered_TcpSock`
- `05_TCP_Relay`
- `Labs/Lab01_TCP_Echo`
- `Labs/Lab02_SockOpts`

ì§„í–‰:

1. TCP serverë¥¼ ì‹¤í–‰í•˜ê³  clientë¡œ ì ‘ì†í•©ë‹ˆë‹¤.
2. Wiresharkì—ì„œ SYN, SYN/ACK, ACKë¥¼ ì°¾ìŠµë‹ˆë‹¤.
3. `recv`ê°€ application message ì „ì²´ë¥¼ ë³´ìž¥í•˜ì§€ ì•ŠëŠ”ë‹¤ëŠ” ì ì„ í™•ì¸í•©ë‹ˆë‹¤.
4. socket optionì„ í•˜ë‚˜ì”© ë°”ê¿” ê´€ì°° ê°€ëŠ¥í•œ ì°¨ì´ë¥¼ ê¸°ë¡í•©ë‹ˆë‹¤.
5. TCP relayë¥¼ í†µí•´ byte streamì´ ê·¸ëŒ€ë¡œ ì „ë‹¬ë˜ëŠ”ì§€ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- listening socketê³¼ accepted socketì˜ ì°¨ì´.
- streamê³¼ messageì˜ ì°¨ì´.
- FINê³¼ RSTì˜ ì°¨ì´.

## 6. `06_Framing_BinaryProtocol`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_Custom_Protocol`
- `02_Layered_TCPFraming`
- `Labs/Lab01_BinaryProto`

ì§„í–‰:

1. binary header fieldë¥¼ í‘œë¡œ ì ìŠµë‹ˆë‹¤.
2. senderê°€ headerì™€ payloadë¥¼ ì§ë ¬í™”í•˜ëŠ” ë¶€ë¶„ì„ í™•ì¸í•©ë‹ˆë‹¤.
3. receiverê°€ headerë¥¼ ë¨¼ì € ì½ê³  lengthë§Œí¼ payloadë¥¼ ì½ëŠ” íë¦„ì„ í™•ì¸í•©ë‹ˆë‹¤.
4. malformed lengthë‚˜ wrong magicì„ ë„£ì–´ parser ë°©ì–´ ë¡œì§ì„ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- fixed headerì™€ variable payload.
- network byte order.
- partial recv ëŒ€ì‘ ë°©ì‹.

## 7. `07_Application_HTTP`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_HTTP_Client`
- `02_HTTP_Server`
- `03_Layered_HTTP`

ì§„í–‰:

1. HTTP clientê°€ ë³´ë‚´ëŠ” request ë¬¸ìžì—´ì„ ì†ìœ¼ë¡œ ë¶„í•´í•©ë‹ˆë‹¤.
2. HTTP server responseì˜ status line, headers, bodyë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
3. `Content-Length`ì™€ ì‹¤ì œ body byte ìˆ˜ë¥¼ ë¹„êµí•©ë‹ˆë‹¤.
4. keep-alive ì—°ê²°ì—ì„œ request ì—¬ëŸ¬ ê°œë¥¼ ì²˜ë¦¬í•˜ëŠ” ë°©ì‹ì„ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- request lineê³¼ status line.
- header/body boundary.
- HTTPì™€ TCP connectionì˜ ì°¨ì´.

## 8. `08_Security_TLS`

ì‹¤ìŠµ í”„ë¡œì íŠ¸:

- `01_TLS_Schannel`

ì§„í–‰:

1. TCP connect í›„ TLS handshakeê°€ ì§„í–‰ë˜ëŠ” ìˆœì„œë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
2. Schannel contextì™€ token êµí™˜ íë¦„ì„ ì½ìŠµë‹ˆë‹¤.
3. HTTPS requestê°€ TLS handshake ì´í›„ ì „ì†¡ë˜ëŠ”ì§€ í™•ì¸í•©ë‹ˆë‹¤.
4. Wiresharkì—ì„œ TLS recordë¥¼ í™•ì¸í•©ë‹ˆë‹¤.

ì²´í¬:

- TLSê°€ ë³´í˜¸í•˜ëŠ” ê²ƒ: confidentiality, integrity, authentication.
- ì¸ì¦ì„œ ê²€ì¦ì˜ ëª©ì .
- HTTP plaintextê°€ captureì— ë³´ì´ì§€ ì•ŠëŠ” ì´ìœ .

## 9. `09_Implementation_IOModels`

ì§„í–‰:

1. blocking TCP echoë¥¼ ë¨¼ì € ê¸°ì¤€ìœ¼ë¡œ ë‘¡ë‹ˆë‹¤.
2. selectì™€ WSAEventSelectë¥¼ readiness modelë¡œ ë¹„êµí•©ë‹ˆë‹¤.
3. overlapped IOì™€ IOCPë¥¼ completion modelë¡œ ë¹„êµí•©ë‹ˆë‹¤.
4. IO modelì´ protocol semanticsë¥¼ ë°”ê¾¸ì§€ ì•ŠëŠ”ë‹¤ëŠ” ì ì„ ì •ë¦¬í•©ë‹ˆë‹¤.

ì²´í¬:

- readiness vs completion.
- fd/event/socket/context ìˆ˜ëª….
- scalabilityì™€ protocol correctnessì˜ ì°¨ì´.

## 10. `10_Platform_WinNetAPI`

ì§„í–‰:

1. WinHTTPë¡œ HTTP request/responseë¥¼ ê³ ìˆ˜ì¤€ APIì—ì„œ í™•ì¸í•©ë‹ˆë‹¤.
2. WebSocket Upgradeì™€ message frame ì†¡ìˆ˜ì‹ ì„ í™•ì¸í•©ë‹ˆë‹¤.
3. Named Pipeë¥¼ TCP/IPê°€ ì•„ë‹Œ IPC ë¹„êµ ëŒ€ìƒìœ¼ë¡œ ì •ë¦¬í•©ë‹ˆë‹¤.

ì²´í¬:

- high-level APIì™€ protocol conceptì˜ ì°¨ì´.
- WebSocket message frameê³¼ TCP byte streamì˜ ì°¨ì´.
- Named Pipe endpointì™€ TCP endpointì˜ ì°¨ì´.
