# CLAUDE.md — practice-Network 에이전트 지침

이 파일은 이 워크스페이스에서 작업하는 에이전트를 위한 지침입니다.

## 워크스페이스 개요

- **주제**: Windows 소켓 프로그래밍 (Winsock2)
- **언어**: C++ (C++17 이상)
- **빌드 시스템**: Visual Studio / MSBuild (`.vcxproj`)
- **대상 플랫폼**: x64 Windows
- **참고 저장소**: `../practice-Win32` (Phase 5의 Winsock 기초 내용 참고)

## 디렉토리 구조

```
practice-Network/
├── Phase1_SocketBasics/    # TCP/UDP 기초 소켓
├── Phase2_IOModels/        # IO 다중화 모델
├── Phase3_Protocols/       # 프로토콜 구현
├── Phase4_Advanced/        # 고급 네트워킹
├── Phase5_WinNetAPI/       # Windows 전용 네트워크 API
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 모듈 폴더를 만듭니다 (예: `Phase1_SocketBasics/01_TCP_Server/`).
2. Visual Studio에서 빈 C++ 프로젝트(`.vcxproj`)를 생성하고 폴더에 저장합니다.
3. 아래 `main.cpp` 초기화 템플릿을 기반으로 작성합니다.
4. Phase 폴더의 솔루션(`.sln`)에 프로젝트를 추가합니다 (없으면 새 솔루션 생성).

## main.cpp 초기화 템플릿

```cpp
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // --- 여기에 구현 ---

    WSACleanup();
    return 0;
}
```

## 코딩 컨벤션

- `WSAStartup` / `WSACleanup`은 항상 쌍으로 사용합니다.
- 소켓은 `INVALID_SOCKET` 체크 후 사용하고, 사용 완료 시 반드시 `closesocket()`을 호출합니다.
- 오류 경로에서 `WSAGetLastError()`를 출력합니다.
- 블로킹 소켓과 논블로킹 소켓의 동작 차이를 주석으로 명시합니다.
- 서버와 클라이언트가 한 파일에 있을 때는 `#define SERVER_MODE` / `CLIENT_MODE`로 분기합니다.
- 포트 번호는 `static constexpr USHORT PORT = 27015;`로 상수화합니다.

## 프로젝트 속성 기본값

```
구성 타입: Application (.exe)
문자 집합: Unicode
C++ 표준: /std:c++17
추가 종속성: ws2_32.lib (Phase 3 TLS: secur32.lib 추가)
경고 수준: /W4
```

## Phase별 학습 진행 현황

| Phase | 상태 | 비고 |
| --- | --- | --- |
| Phase 1: Socket Basics | 미시작 | |
| Phase 2: IO Models | 미시작 | |
| Phase 3: Protocols | 미시작 | |
| Phase 4: Advanced | 미시작 | |
| Phase 5: WinNetAPI | 미시작 | |

## 주의 사항

- Phase 4의 `14_RawSocket`은 관리자 권한 실행이 필요합니다 (`SOCK_RAW`).
- Phase 3의 `12_TLS_Schannel`은 `secur32.lib`를 링커에 추가해야 합니다.
- 방화벽이 포트를 차단할 수 있습니다. 테스트 시 루프백(`127.0.0.1`)을 우선 사용합니다.
- IOCP 실습 시 워커 스레드 수는 `2 * CPU 코어 수`를 기본값으로 사용합니다.
