# CLAUDE.md

Repository rules for agents working in `practice-Network`.

## Scope

- Topic: OSI/TCP-IP based network protocol practice.
- Language: C++17 or later.
- Platform: x64 Windows.
- Build: Visual Studio / MSBuild / `.vcxproj`.
- Default socket library: `ws2_32.lib`.

## Structure

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

## Rules

- Prefer protocol explanations over API trivia.
- Keep examples independent; do not add shared/common folders.
- Keep project numbering local to each chapter: `01`, `02`, `03`.
- Keep lab numbering local to each chapter: `Lab01`, `Lab02`, `Lab03`.
- Do not reintroduce old numbering such as `14_RawSocket`, `15_Layered_HTTP`, or `18_NamedPipe_Net`.
- Raw socket examples may require administrator privileges.
- TLS examples may require `secur32.lib`.
- WinHTTP/WebSocket examples may require `winhttp.lib`.
- Do not commit build outputs.

## Primary Docs

- `README.md`: repository overview.
- `AGENT_CONTEXT.md`: working context for agents.
- `docs/ROADMAP.md`: learning roadmap.
- `docs/CURRICULUM.md`: curriculum.
- `docs/PRACTICE_PLAN.md`: practice plan.
- `docs/ASSESSMENT.md`: completion checklist.
