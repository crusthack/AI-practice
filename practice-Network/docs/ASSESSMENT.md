# Assessment Checklist

이 문서는 각 단계 완료 여부를 확인하기 위한 체크리스트입니다.

## Layer Understanding

- [ ] OSI 7계층과 TCP/IP 4계층을 1:1 암기가 아니라 책임 기준으로 설명할 수 있다.
- [ ] message, segment/datagram, packet, frame의 차이를 말할 수 있다.
- [ ] MAC, IP, port, hostname/path가 어느 계층의 식별자인지 구분할 수 있다.

## Data Link

- [ ] Ethernet II header 14바이트를 나눌 수 있다.
- [ ] broadcast, multicast, unicast MAC을 구분할 수 있다.
- [ ] ARP request와 reply의 sender/target field를 설명할 수 있다.

## Internet

- [ ] IPv4 IHL로 header length를 계산할 수 있다.
- [ ] TTL의 목적을 설명할 수 있다.
- [ ] protocol number 1, 6, 17을 구분할 수 있다.
- [ ] ICMP Echo Request/Reply type을 구분할 수 있다.

## Transport

- [ ] UDP header 8바이트 field를 설명할 수 있다.
- [ ] UDP가 제공하지 않는 guarantee를 말할 수 있다.
- [ ] TCP 3-way handshake를 packet 흐름으로 설명할 수 있다.
- [ ] TCP stream에 message boundary가 없다는 점을 예로 설명할 수 있다.
- [ ] FIN/RST 종료 차이를 설명할 수 있다.

## Framing and Application

- [ ] length-prefix framing이 필요한 이유를 설명할 수 있다.
- [ ] partial recv와 sticky packet을 구분할 수 있다.
- [ ] binary protocol의 magic/version/type/length/checksum 목적을 설명할 수 있다.
- [ ] HTTP request line, status line, headers, body를 구분할 수 있다.
- [ ] `Content-Length`가 body parsing에 필요한 이유를 설명할 수 있다.

## Security

- [ ] TLS handshake가 HTTP request보다 먼저 수행됨을 설명할 수 있다.
- [ ] TLS가 제공하는 confidentiality, integrity, authentication을 구분할 수 있다.
- [ ] HTTPS capture에서 HTTP plaintext가 보이지 않는 이유를 설명할 수 있다.

## Implementation

- [ ] select/event 모델이 readiness 기반임을 설명할 수 있다.
- [ ] overlapped/IOCP 모델이 completion 기반임을 설명할 수 있다.
- [ ] IO model이 TCP/UDP/HTTP protocol semantics를 바꾸지 않는다는 점을 말할 수 있다.
- [ ] WinHTTP/WebSocket 같은 high-level API가 protocol 개념을 숨기지만 제거하지는 않는다는 점을 설명할 수 있다.

## Final Capstone

다음 내용을 한 문서로 작성하면 전체 커리큘럼을 완료한 것으로 봅니다.

1. HTTP GET 요청 하나가 application data에서 TCP segment, IPv4 packet, Ethernet frame으로 캡슐화되는 과정.
2. local link에서 ARP가 필요한 이유.
3. TCP와 UDP의 보장 차이.
4. TCP 위 HTTP가 message boundary를 만드는 방식.
5. TLS가 HTTP message를 보호하는 위치.
