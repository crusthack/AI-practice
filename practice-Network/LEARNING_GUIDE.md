# Network 프로그래밍 통합 학습 교안

모든 모듈을 순서대로 학습하기 위한 통합 교안입니다. 각 항목은 목표, 관찰 포인트, 핵심 API, 확장 과제로 구성됩니다.

## 공통 실습 절차

1. `main.cpp`에서 API 호출 순서를 먼저 파악합니다.
2. 서버를 먼저 실행하고 클라이언트를 연결합니다.
3. Wireshark에서 루프백 인터페이스를 캡처해 패킷 흐름을 확인합니다.
4. 오류 발생 시 `WSAGetLastError()` 코드를 MSDN에서 찾아 원인을 분석합니다.
5. 확장 과제를 구현하고 전체 솔루션을 재빌드합니다.

---

## Phase 1: 소켓 기초

### 01_TCP_Server

- **목표**: TCP 서버의 기본 생명 주기(소켓 생성 → bind → listen → accept → recv/send → close)를 이해합니다.
- **관찰**: `accept()`가 블로킹되는 시점, 클라이언트 연결 후 새 소켓 디스크립터가 발급되는 과정을 확인합니다.
- **핵심 API**: `socket`, `bind`, `listen`, `accept`, `recv`, `send`, `closesocket`
- **확장 과제**: 클라이언트를 스레드로 분리해 다수 동시 연결을 처리합니다.

### 02_TCP_Client

- **목표**: `connect()` 이후 서버와 데이터를 교환하고 `shutdown()`으로 우아하게 종료하는 흐름을 익힙니다.
- **관찰**: `connect()` 실패 시 오류 코드(`WSAECONNREFUSED`, `WSAETIMEDOUT`)를 확인합니다.
- **핵심 API**: `socket`, `connect`, `send`, `recv`, `shutdown`, `closesocket`
- **확장 과제**: 재연결 로직(최대 3회 재시도, 지수 백오프)을 추가합니다.

### 03_UDP_Echo

- **목표**: 비연결 UDP 소켓의 `sendto`/`recvfrom` 구조를 익힙니다.
- **관찰**: TCP와 달리 연결 설정 없이 바로 데이터그램을 전송하는 과정을 Wireshark로 비교합니다.
- **핵심 API**: `socket(AF_INET, SOCK_DGRAM)`, `sendto`, `recvfrom`
- **확장 과제**: 클라이언트 주소를 기반으로 유니캐스트 응답과 브로드캐스트 응답을 비교합니다.

### 04_Socket_Options

- **목표**: 소켓 옵션이 동작에 미치는 영향을 실험합니다.
- **관찰**: `TCP_NODELAY` 설정 전후 전송 지연(Nagle 알고리즘)을 측정합니다.
- **핵심 API**: `setsockopt`, `getsockopt`
- **대상 옵션**: `SO_KEEPALIVE`, `TCP_NODELAY`, `SO_RCVTIMEO`, `SO_SNDBUF`, `SO_REUSEADDR`
- **확장 과제**: `SO_LINGER`로 강제 닫기 시 `TIME_WAIT` 상태 변화를 `netstat`으로 관찰합니다.

---

## Phase 2: IO 모델

### 05_Select_Model

- **목표**: `select()`로 단일 스레드에서 여러 소켓을 모니터링하는 방법을 이해합니다.
- **관찰**: `fd_set` 크기 제한(`FD_SETSIZE = 64`)과 대규모 연결 시 성능 한계를 파악합니다.
- **핵심 API**: `select`, `FD_ZERO`, `FD_SET`, `FD_ISSET`, `FD_CLR`
- **확장 과제**: 타임아웃을 0으로 설정한 폴링 모드와 블로킹 모드 성능을 비교합니다.

### 06_WSAEventSelect

- **목표**: `WSAEventSelect`로 소켓 이벤트를 Windows 이벤트 핸들에 연결하는 방법을 학습합니다.
- **관찰**: `FD_READ`, `FD_WRITE`, `FD_ACCEPT`, `FD_CLOSE` 이벤트가 각각 언제 시그널되는지 확인합니다.
- **핵심 API**: `WSAEventSelect`, `WSAWaitForMultipleEvents`, `WSAEnumNetworkEvents`
- **확장 과제**: 최대 64개 이벤트 핸들 제한을 우회하는 멀티 스레드 패턴을 구현합니다.

### 07_OverlappedIO

- **목표**: `WSASend`/`WSARecv`의 Overlapped 구조체 기반 비동기 IO를 이해합니다.
- **관찰**: `WSAOVERLAPPED`의 완료 이벤트가 언제 시그널되고 `WSAGetOverlappedResult`로 바이트 수를 얻는 과정을 추적합니다.
- **핵심 API**: `WSASend`, `WSARecv`, `WSAOVERLAPPED`, `WSAGetOverlappedResult`
- **확장 과제**: Completion Routine(콜백) 기반 비동기와 이벤트 기반을 비교합니다.

### 08_IOCP

- **목표**: I/O Completion Port를 사용해 고성능 비동기 IO 서버를 구현합니다.
- **관찰**: `GetQueuedCompletionStatus`가 워커 스레드를 어떻게 깨우는지, Completion Key로 컨텍스트를 어떻게 전달하는지 확인합니다.
- **핵심 API**: `CreateIoCompletionPort`, `GetQueuedCompletionStatus`, `PostQueuedCompletionStatus`
- **확장 과제**: 워커 스레드 수를 1, 2, `2N`(N=코어 수)으로 바꾸며 처리량을 측정합니다.

---

## Phase 3: 프로토콜 구현

### 09_HTTP_Client

- **목표**: Raw TCP 소켓으로 HTTP/1.1 GET 요청을 직접 조립하고 응답을 파싱합니다.
- **관찰**: 상태 라인, 헤더, 빈 줄, 본문 구조를 `recv` 버퍼에서 직접 분리합니다.
- **핵심 API**: `connect`, `send`, `recv` + 문자열 파싱
- **확장 과제**: `Transfer-Encoding: chunked` 응답을 디코딩합니다.

### 10_HTTP_Server

- **목표**: 간단한 멀티스레드 HTTP 서버를 구현합니다.
- **관찰**: 요청 메서드(GET/POST) 분기, Content-Type 헤더 설정, Connection: keep-alive 처리를 확인합니다.
- **핵심 API**: `accept`, 스레드 생성, HTTP 요청/응답 문자열 조립
- **확장 과제**: 정적 파일 서빙(파일 읽기 → Content-Length 계산)을 추가합니다.

### 11_Custom_Protocol

- **목표**: 고정 헤더(Magic, Version, MessageType, PayloadLength) + 가변 페이로드 바이너리 프로토콜을 설계하고 구현합니다.
- **관찰**: `recv`가 페이로드를 잘라서 받을 때(short recv) 재조립하는 로직을 확인합니다.
- **핵심 API**: `recv` 루프, 바이트 오더 변환(`htonl`, `ntohl`)
- **확장 과제**: 프로토콜에 체크섬(CRC32)과 시퀀스 번호를 추가합니다.

### 12_TLS_Schannel

- **목표**: Windows Schannel SSPI로 TLS 핸드셰이크를 수행하고 암호화 채널을 통해 데이터를 송수신합니다.
- **관찰**: `InitializeSecurityContext`가 여러 차례 호출되며 핸드셰이크 토큰을 교환하는 과정을 추적합니다.
- **핵심 API**: `AcquireCredentialsHandle`, `InitializeSecurityContext`, `EncryptMessage`, `DecryptMessage`
- **확장 과제**: 서버 인증서 체인 검증(`CERT_CHAIN_POLICY_SSL`)을 추가합니다.

---

## Phase 4: 고급 네트워킹

### 13_Multicast

- **목표**: IP 멀티캐스트 그룹에 참가하고 UDP 패킷을 그룹에 전송합니다.
- **관찰**: `IP_ADD_MEMBERSHIP`으로 그룹에 참가한 소켓만 패킷을 수신함을 Wireshark로 확인합니다.
- **핵심 API**: `setsockopt(IP_ADD_MEMBERSHIP)`, `setsockopt(IP_MULTICAST_TTL)`, `sendto`
- **확장 과제**: 멀티캐스트 소스 필터(`IP_ADD_SOURCE_MEMBERSHIP`)를 실험합니다.

### 14_RawSocket

- **목표**: `SOCK_RAW`로 IP/ICMP 헤더를 직접 읽어 ping 패킷을 관찰합니다. (관리자 권한 필요)
- **관찰**: IP 헤더 필드(TTL, Protocol, Source/Dest IP)를 직접 파싱합니다.
- **핵심 API**: `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`, `recvfrom`, IP/ICMP 헤더 구조체
- **확장 과제**: ICMP Echo Request를 직접 조립해 전송하고 RTT를 측정합니다.

### 15_TCP_Relay

- **목표**: 클라이언트 ↔ 릴레이 ↔ 백엔드 구조의 TCP 프록시를 구현합니다.
- **관찰**: 릴레이가 양방향 바이트 스트림을 어떻게 투명하게 전달하는지 확인합니다.
- **핵심 API**: `select` 또는 IOCP로 양방향 전달, 소켓 쌍 관리
- **확장 과제**: 릴레이에 간단한 트래픽 로깅(타임스탬프 + 바이트 수)을 추가합니다.

---

## Phase 5: Windows 네트워크 API

### 16_WinHTTP

- **목표**: `WinHttpOpen` → `WinHttpConnect` → `WinHttpOpenRequest` → `WinHttpSendRequest` → 응답 읽기 전체 흐름을 익힙니다.
- **관찰**: `WinHttpQueryHeaders`로 HTTP 상태 코드와 응답 헤더를 읽는 과정을 확인합니다.
- **핵심 API**: `WinHttpOpen`, `WinHttpConnect`, `WinHttpSendRequest`, `WinHttpReadData`
- **확장 과제**: HTTPS 요청 시 인증서 오류 무시 플래그(`WINHTTP_FLAG_SECURE_DEFAULTS`)를 비교합니다.

### 17_WebSocket

- **목표**: WinHTTP WebSocket API로 HTTP Upgrade를 수행하고 텍스트/바이너리 프레임을 송수신합니다.
- **관찰**: `WinHttpWebSocketUpgrade` 이후 소켓이 WebSocket 프레임 단위로 동작함을 확인합니다.
- **핵심 API**: `WinHttpWebSocketUpgrade`, `WinHttpWebSocketSend`, `WinHttpWebSocketReceive`
- **확장 과제**: Ping/Pong 프레임 처리 루프를 구현합니다.

### 18_NamedPipe_Net

- **목표**: Named Pipe를 네트워크 IPC 대안으로 실습하고 소켓 기반 통신과 성능 및 구조를 비교합니다.
- **관찰**: 로컬 Named Pipe(`\\.\pipe\...`)와 원격 Named Pipe(`\\server\pipe\...`)의 차이를 이해합니다.
- **핵심 API**: `CreateNamedPipe`, `ConnectNamedPipe`, `WriteFile`, `ReadFile`
- **확장 과제**: 비동기 Named Pipe(`FILE_FLAG_OVERLAPPED`)와 동기 방식을 비교합니다.
