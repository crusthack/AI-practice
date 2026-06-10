# Agent Context

ì´ ë¬¸ì„œëŠ” ì´ ì €ìž¥ì†Œì—ì„œ ìž‘ì—…í•˜ëŠ” ì—ì´ì „íŠ¸ê°€ ë¹ ë¥´ê²Œ ë§¥ë½ì„ íŒŒì•…í•˜ê¸° ìœ„í•œ ìž‘ì—… ì§€ì¹¨ìž…ë‹ˆë‹¤.

## ì €ìž¥ì†Œ ëª©ì 

- ëª©ì : OSI/TCP-IP ê³„ì¸µ ê¸°ë°˜ ë„¤íŠ¸ì›Œí¬ í”„ë¡œí† ì½œ í•™ìŠµ ì‹¤ìŠµ.
- ì–¸ì–´: C++17.
- í”Œëž«í¼: Windows x64.
- ë¹Œë“œ ë°©ì‹: Visual Studio / MSBuild / ê°œë³„ `.vcxproj`.
- í•µì‹¬ ë°©í–¥: API ì‚¬ìš©ë²•ë³´ë‹¤ protocol layer, header, encapsulation, addressing, reliability, framing ì„¤ëª…ì„ ìš°ì„ í•œë‹¤.

## í˜„ìž¬ êµ¬ì¡°

```text
practice-Network/
  01_DataLink_Ethernet_ARP/      Link: Ethernet, MAC, ARP
  02_Network_IP_ICMP/            Internet: IPv4, ICMP, raw packet
  03_Network_Multicast/          Internet: multicast
  04_Transport_UDP/              Transport: UDP
  05_Transport_TCP/              Transport: TCP
  06_Framing_BinaryProtocol/     TCP application framing and binary protocol
  07_Application_HTTP/           Application: HTTP
  08_Security_TLS/               TLS security layer
  09_Implementation_IOModels/    Optional IO models
  10_Platform_WinNetAPI/         Optional Windows platform APIs
  docs/                          Roadmap, curriculum, practice plan, assessment
```

## ë²ˆí˜¸ ê·œì¹™

- Top-level chapter numbers are global roadmap numbers.
- Project numbers restart inside each chapter: `01_*`, `02_*`, `03_*`.
- Lab numbers restart inside each chapter: `Lab01_*`, `Lab02_*`.
- Do not reintroduce old project numbers such as `14_RawSocket`, `15_Layered_HTTP`, or `18_NamedPipe_Net`.

## ë¬¸ì„œ ì²´ê³„

- `README.md`: human-facing repository entry point.
- `docs/ROADMAP.md`: layer-based roadmap.
- `docs/CURRICULUM.md`: chapter curriculum.
- `docs/PRACTICE_PLAN.md`: detailed hands-on plan.
- `docs/ASSESSMENT.md`: checklist and capstone.
- Each chapter folder has its own `README.md`.
- Each project folder with `main.cpp` should have its own `README.md`.

## ìž‘ì—… ì›ì¹™

- í”„ë¡œì íŠ¸ ê°„ `common`, `shared` í´ë”ë¥¼ ë§Œë“¤ì§€ ì•ŠëŠ”ë‹¤.
- ê° í”„ë¡œì íŠ¸ëŠ” í•„ìš”í•œ helper/headerë¥¼ ìžê¸° í´ë” ì•ˆì— ë‘”ë‹¤.
- Winsock/select/IOCPëŠ” protocol ìžì²´ê°€ ì•„ë‹ˆë¼ êµ¬í˜„ ìˆ˜ë‹¨ìœ¼ë¡œ ì„¤ëª…í•œë‹¤.
- Data Link, Internet, Transport, Application, TLS ê°œë…ì„ ìš°ì„ í•œë‹¤.
- ì†ŒìŠ¤ì™€ ë¬¸ì„œëŠ” ASCII ìš°ì„ ìœ¼ë¡œ ìž‘ì„±í•œë‹¤. í•œê¸€ ë¬¸ì„œë¥¼ ì“¸ ë•ŒëŠ” UTF-8ì´ ê¹¨ì§€ì§€ ì•Šë„ë¡ ì£¼ì˜í•œë‹¤.
- ë¹Œë“œ ì‚°ì¶œë¬¼, `.vs`, `x64`, `Debug`, `Release` í´ë”ëŠ” ì»¤ë°‹ ëŒ€ìƒì´ ì•„ë‹ˆë‹¤.

## ë¹Œë“œ ì°¸ê³ 

ì¼ë°˜ socket í”„ë¡œì íŠ¸ëŠ” `ws2_32.lib`ê°€ í•„ìš”í•˜ë‹¤.

TLS/Schannel:

- `secur32.lib`

WinHTTP/WebSocket:

- `winhttp.lib`

ì˜ˆì‹œ:

```powershell
MSBuild 07_Application_HTTP\03_Layered_HTTP\03_Layered_HTTP.vcxproj /p:Configuration=Debug /p:Platform=x64
```

## ê²€ì¦ ì²´í¬

ìž‘ì—… í›„ ë‹¤ìŒì„ í™•ì¸í•œë‹¤.

```powershell
rg --files -g *.vcxproj
rg --files -g README.md
rg "14_RawSocket|15_Layered_HTTP|18_NamedPipe_Net"
```

ë§ˆì§€ë§‰ ê²€ìƒ‰ì€ ê²°ê³¼ê°€ ì—†ì–´ì•¼ í•œë‹¤.
