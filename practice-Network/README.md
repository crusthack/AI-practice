# Network Protocol Learning Lab

OSI 7ê³„ì¸µê³¼ TCP/IP 4ê³„ì¸µì„ ê¸°ì¤€ìœ¼ë¡œ ë„¤íŠ¸ì›Œí¬ í”„ë¡œí† ì½œì„ í•™ìŠµí•˜ëŠ” C++ ì‹¤ìŠµ ì €ìž¥ì†Œìž…ë‹ˆë‹¤. Winsock, select, IOCP, WinHTTP ê°™ì€ APIëŠ” í•™ìŠµì˜ ì¤‘ì‹¬ì´ ì•„ë‹ˆë¼ í”„ë¡œí† ì½œì„ ê´€ì°°í•˜ê³  êµ¬í˜„í•˜ê¸° ìœ„í•œ ë„êµ¬ë¡œë§Œ ë‹¤ë£¹ë‹ˆë‹¤.

## í•™ìŠµ ëª©í‘œ

- Ethernet, ARP, IPv4, ICMP, UDP, TCP, HTTP, TLSì˜ ì—­í• ì„ ê³„ì¸µë³„ë¡œ ì„¤ëª…í•©ë‹ˆë‹¤.
- ê° ê³„ì¸µì˜ headerì™€ payload ê²½ê³„ë¥¼ ì§ì ‘ ì½ìŠµë‹ˆë‹¤.
- MAC ì£¼ì†Œ, IP ì£¼ì†Œ, port, hostname/pathê°€ ì–´ëŠ ê³„ì¸µì˜ ì‹ë³„ìžì¸ì§€ êµ¬ë¶„í•©ë‹ˆë‹¤.
- TCP/UDPì˜ ì°¨ì´ë¥¼ APIê°€ ì•„ë‹ˆë¼ protocol guarantee ê¸°ì¤€ìœ¼ë¡œ ì„¤ëª…í•©ë‹ˆë‹¤.
- TCP stream ìœ„ì—ì„œ application message framingì´ í•„ìš”í•œ ì´ìœ ë¥¼ ì´í•´í•©ë‹ˆë‹¤.

## ë¬¸ì„œ

| ë¬¸ì„œ | ìš©ë„ |
| --- | --- |
| `docs/ROADMAP.md` | ì „ì²´ í•™ìŠµ ë¡œë“œë§µ |
| `docs/CURRICULUM.md` | ì±•í„°ë³„ ì»¤ë¦¬í˜ëŸ¼ |
| `docs/PRACTICE_PLAN.md` | ì„¸ë¶€ ì‹¤ìŠµ ê³„íš |
| `docs/ASSESSMENT.md` | ì™„ë£Œ ê¸°ì¤€ ì²´í¬ë¦¬ìŠ¤íŠ¸ |
| `LEARNING_GUIDE.md` | ì••ì¶• ìš”ì•½ ë¡œë“œë§µ |
| `AGENT_CONTEXT.md` | ì—ì´ì „íŠ¸ ìž‘ì—…ìš© ì €ìž¥ì†Œ ë§¥ë½ |
| `CLAUDE.md` | Claude/Codex ê³„ì—´ ì—ì´ì „íŠ¸ìš© ìµœì†Œ ê·œì¹™ |

## ë¡œë“œë§µ

| ìˆœì„œ | ê³„ì¸µ | ì£¼ì œ | í´ë” |
| --- | --- | --- | --- |
| 1 | Data Link / Link | Ethernet frame, MAC, EtherType, ARP | `01_DataLink_Ethernet_ARP` |
| 2 | Network / Internet | IPv4, ICMP, TTL, checksum | `02_Network_IP_ICMP` |
| 3 | Network / Internet | multicast address, group join | `03_Network_Multicast` |
| 4 | Transport | UDP datagram, port, missing reliability | `04_Transport_UDP` |
| 5 | Transport | TCP stream, handshake, ACK, teardown | `05_Transport_TCP` |
| 6 | Session/Presentation ê´€ì  | message framing, binary protocol, byte order | `06_Framing_BinaryProtocol` |
| 7 | Application | HTTP request/response, headers, body, keep-alive | `07_Application_HTTP` |
| 8 | Security/Presentation ê´€ì  | TLS handshake, certificate, encrypted record | `08_Security_TLS` |
| 9 | êµ¬í˜„ ë³´ì¡° | select, event, overlapped IO, IOCP | `09_Implementation_IOModels` |
| 10 | í”Œëž«í¼ API | WinHTTP, WebSocket, Named Pipe | `10_Platform_WinNetAPI` |

## ë²ˆí˜¸ ê·œì¹™

- ìµœìƒìœ„ í´ë”ëŠ” ë¡œë“œë§µ ìˆœì„œë¥¼ ë”°ë¦…ë‹ˆë‹¤.
- ê° ì±•í„° ë‚´ë¶€ í”„ë¡œì íŠ¸ëŠ” `01`, `02`, `03`ì²˜ëŸ¼ ë‹¤ì‹œ ì‹œìž‘í•©ë‹ˆë‹¤.
- `Labs` ë‚´ë¶€ ì‹¤ìŠµë„ `Lab01`, `Lab02`, `Lab03`ì²˜ëŸ¼ ë‹¤ì‹œ ì‹œìž‘í•©ë‹ˆë‹¤.
- ì˜ˆ: `05_Transport_TCP/01_TCP_Server`, `05_Transport_TCP/02_TCP_Client`, `05_Transport_TCP/Labs/Lab01_TCP_Echo`

## í”„ë¡œì íŠ¸ êµ¬ì¡°

```text
practice-Network/
  01_DataLink_Ethernet_ARP/
  02_Network_IP_ICMP/
  03_Network_Multicast/
  04_Transport_UDP/
  05_Transport_TCP/
  06_Framing_BinaryProtocol/
  07_Application_HTTP/
  08_Security_TLS/
  09_Implementation_IOModels/
  10_Platform_WinNetAPI/
  docs/
```

## í•™ìŠµ ë°©ì‹

1. `docs/ROADMAP.md`ë¡œ ì „ì²´ ìˆœì„œë¥¼ í™•ì¸í•©ë‹ˆë‹¤.
2. `docs/CURRICULUM.md`ì—ì„œ í•´ë‹¹ ì±•í„°ì˜ ëª©í‘œë¥¼ ì½ìŠµë‹ˆë‹¤.
3. ê° í”„ë¡œì íŠ¸ í´ë”ì˜ `README.md`ë¥¼ ë³´ê³  ì‹¤í–‰í•©ë‹ˆë‹¤.
4. header fieldì™€ payload boundaryë¥¼ ì§ì ‘ í‘œì‹œí•©ë‹ˆë‹¤.
5. ê°€ëŠ¥í•˜ë©´ Wireshark ìº¡ì²˜ì™€ í”„ë¡œê·¸ëž¨ ì¶œë ¥ì„ ë¹„êµí•©ë‹ˆë‹¤.
6. `docs/ASSESSMENT.md`ì˜ ì²´í¬ë¦¬ìŠ¤íŠ¸ë¥¼ ì±„ì›ë‹ˆë‹¤.

## ë¹Œë“œ

ê° ì˜ˆì œëŠ” ë…ë¦½ì ì¸ Visual Studio C++ í”„ë¡œì íŠ¸ìž…ë‹ˆë‹¤. Visual Studioì—ì„œ `.vcxproj`ë¥¼ ì—´ê±°ë‚˜, MSBuildê°€ PATHì— ìž¡í˜€ ìžˆìœ¼ë©´ ë‹¤ìŒì²˜ëŸ¼ ë¹Œë“œí•©ë‹ˆë‹¤.

```powershell
MSBuild 01_DataLink_Ethernet_ARP\01_Ethernet_ARP_Frame\01_Ethernet_ARP_Frame.vcxproj /p:Configuration=Debug /p:Platform=x64
```

## ì£¼ì˜ì‚¬í•­

- Raw socket ì‹¤ìŠµì€ ê´€ë¦¬ìž ê¶Œí•œì´ í•„ìš”í•  ìˆ˜ ìžˆìŠµë‹ˆë‹¤.
- TLS/Schannel ì‹¤ìŠµì€ `secur32.lib`ê°€ í•„ìš”í•©ë‹ˆë‹¤.
- WinHTTP/WebSocket ì‹¤ìŠµì€ `winhttp.lib`ê°€ í•„ìš”í•©ë‹ˆë‹¤.
- ì™¸ë¶€ ë„¤íŠ¸ì›Œí¬ê°€ í•„ìš”í•œ ì‹¤ìŠµì€ í™˜ê²½ì— ë”°ë¼ ì‹¤íŒ¨í•  ìˆ˜ ìžˆìŠµë‹ˆë‹¤.
- ë¹Œë“œ ì‚°ì¶œë¬¼ì€ ì €ìž¥ì†Œì— í¬í•¨í•˜ì§€ ì•ŠìŠµë‹ˆë‹¤.
