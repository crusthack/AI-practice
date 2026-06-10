# Network Protocol Roadmap

The roadmap emphasizes protocol layers over socket API memorization.

## Layer Mapping

| OSI | TCP/IP | Main ideas | Repository focus |
| --- | --- | --- | --- |
| 7 Application | Application | HTTP, WebSocket, DNS concept | HTTP client/server, WebSocket API |
| 6 Presentation | Application | Encoding, byte order, TLS | Binary protocol, Schannel TLS |
| 5 Session | Application | Lifecycle, keep-alive | HTTP keep-alive, custom session state |
| 4 Transport | Transport | TCP, UDP, port, reliability | TCP/UDP echo, custom transport-like code |
| 3 Network | Internet | IPv4, ICMP, TTL, routing, multicast | Raw packet parsing, multicast |
| 2 Data Link | Link | Ethernet, MAC, frame, ARP | Ethernet/ARP frame parsing |
| 1 Physical | Link | Bit, signal, media | Concept only |

## Recommended Order

1. `01_DataLink_Ethernet_ARP`: read Ethernet and ARP before IP delivery.
2. `02_Network_IP_ICMP`: parse IPv4 and ICMP fields.
3. `03_Network_Multicast`: compare unicast and multicast delivery.
4. `04_Transport_UDP`: understand datagram delivery and missing guarantees.
5. `05_Transport_TCP`: understand stream delivery, handshake, and teardown.
6. `06_Framing_BinaryProtocol`: add application message boundaries over TCP.
7. `07_Application_HTTP`: analyze HTTP as an application protocol over TCP.
8. `08_Security_TLS`: place TLS between TCP and protected application data.
9. `09_Implementation_IOModels`: optional implementation model topics.
10. `10_Platform_WinNetAPI`: optional Windows platform API topics.

## Minimum Completion Criteria

- Decompose one HTTP request by OSI/TCP/IP layer.
- Read Ethernet, ARP, IPv4, ICMP, TCP, and UDP core fields.
- Explain TCP and UDP by protocol guarantees, not by API names.
- Explain why TCP applications need message framing.
- Explain what TLS protects and where it sits in the stack.