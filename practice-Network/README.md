# Network 프로그래밍 학습 저장소

Winsock2를 출발점으로 Windows 소켓 모델, IO 다중화, 프로토콜 구현, TLS, 고급 네트워킹 API까지 단계적으로 학습하기 위한 Visual C++ 실습 모음입니다.

## 빠른 시작

Visual Studio가 설치된 환경에서 각 Phase 폴더의 `.sln` 또는 개별 `.vcxproj`를 열어 빌드합니다.

```powershell
# 예시: Phase 1 전체 빌드
MSBuild Phase1_SocketBasics\Phase1.sln /p:Configuration=Debug /p:Platform=x64
```

링커에 `ws2_32.lib`가 포함되어야 합니다. 각 프로젝트 속성 → 링커 → 추가 종속성에 `ws2_32.lib`를 추가하거나 소스 상단에 `#pragma comment(lib, "ws2_32.lib")`를 사용합니다.

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 빌드 방법, 전체 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 학습 교안, 실습 절차, 확장 과제 |

## 전체 로드맵

| Phase | 폴더 | 학습 주제 |
| --- | --- | --- |
| Phase 1 | `Phase1_SocketBasics` | TCP/UDP 소켓 생성, 바인드, 연결, 에코 서버/클라이언트 |
| Phase 2 | `Phase2_IOModels` | select, WSAEventSelect, Overlapped I/O, IOCP |
| Phase 3 | `Phase3_Protocols` | HTTP/1.1 파싱, 커스텀 바이너리 프로토콜, TLS(Schannel) |
| Phase 4 | `Phase4_Advanced` | 멀티캐스트, Raw 소켓, TCP 프록시/릴레이 |
| Phase 5 | `Phase5_WinNetAPI` | WinHTTP, WebSocket, Named Pipe 네트워크 |

## 모듈 목차

### Phase 1: 소켓 기초

- `01_TCP_Server` — `bind` → `listen` → `accept` → `recv`/`send` 루프
- `02_TCP_Client` — `connect` 후 메시지 송수신, graceful shutdown
- `03_UDP_Echo` — 비연결 UDP 에코 서버/클라이언트
- `04_Socket_Options` — `SO_KEEPALIVE`, `TCP_NODELAY`, `SO_RCVTIMEO`, `SO_REUSEADDR`

### Phase 2: IO 모델

- `05_Select_Model` — `select()`로 다수 소켓 모니터링
- `06_WSAEventSelect` — `WSAEventSelect` + `WaitForMultipleObjects` 패턴
- `07_OverlappedIO` — `WSASend`/`WSARecv` Overlapped 구조체 기반 비동기 IO
- `08_IOCP` — I/O Completion Port, 워커 스레드 풀, Completion Key 패턴

### Phase 3: 프로토콜 구현

- `09_HTTP_Client` — Raw 소켓으로 HTTP/1.1 GET/POST 요청 파싱
- `10_HTTP_Server` — 멀티스레드 HTTP 서버, Content-Type 분기
- `11_Custom_Protocol` — 고정 헤더 + 가변 페이로드 바이너리 프로토콜
- `12_TLS_Schannel` — Schannel SSPI로 TLS 핸드셰이크 및 암호화 통신

### Phase 4: 고급 네트워킹

- `13_Multicast` — IP 멀티캐스트 그룹 참가/탈퇴, TTL 설정
- `14_RawSocket` — Raw 소켓으로 IP/ICMP 헤더 접근 (관리자 권한 필요)
- `15_TCP_Relay` — 클라이언트 ↔ 릴레이 ↔ 백엔드 TCP 프록시 구현

### Phase 5: Windows 네트워크 API

- `16_WinHTTP` — `WinHttpOpen` / `WinHttpSendRequest` / 응답 스트리밍
- `17_WebSocket` — WinHTTP WebSocket 업그레이드, 메시지 프레임 송수신
- `18_NamedPipe_Net` — Named Pipe 서버/클라이언트 IPC (네트워크 비교 실습)

## 권장 학습 방식

1. `LEARNING_GUIDE.md`에서 해당 모듈의 목표와 관찰 포인트를 읽습니다.
2. `main.cpp`에서 API 호출 순서를 따라갑니다.
3. Wireshark 또는 네트워크 모니터로 실제 패킷을 캡처해 비교합니다.
4. 오류 발생 시 `WSAGetLastError()` 코드를 확인합니다.
5. 확장 과제를 구현하고 전체 빌드 후 동작을 검증합니다.
