# CLAUDE.md — practice-OpertingSystem 에이전트 지침

이 파일은 이 워크스페이스에서 작업하는 에이전트를 위한 지침입니다.

## 워크스페이스 개요

- **주제**: Windows OS 내부 동작 — 프로세스, 스레드, 메모리, 동기화, I/O, IPC
- **언어**: C++ (C++17 이상)
- **빌드 시스템**: Visual Studio / MSBuild (`.vcxproj`)
- **대상 플랫폼**: x64 Windows
- **선행 학습**: `../practice-Win32` Phase 3 (FileIO, ProcessThread, Synchronization, Memory)
- **참고**: Win32에서 이미 다룬 내용의 심화·확장이 이 저장소의 목적입니다.

## 디렉토리 구조

```
practice-OpertingSystem/
├── Phase1_Process/         # 프로세스 생성·모니터링·Job Object
├── Phase2_Thread/          # 스레드·스레드 풀·Fiber
├── Phase3_Memory/          # 가상 메모리·힙·MMF
├── Phase4_Sync/            # 동기화 프리미티브 비교
├── Phase5_IO/              # 비동기 I/O·파일 알림·DeviceIoControl
├── Phase6_IPC/             # 파이프·공유 메모리·Mailslot
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 모듈 폴더를 만듭니다 (예: `Phase1_Process/01_Process_Creation/`).
2. Visual Studio에서 빈 C++ 콘솔 프로젝트를 생성합니다.
3. 아래 `main.cpp` 초기화 템플릿을 기반으로 작성합니다.
4. Phase 폴더의 솔루션에 프로젝트를 추가합니다.

## main.cpp 초기화 템플릿

```cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>

// 오류 출력 헬퍼
static void PrintWinError(const char* context) {
    DWORD err = GetLastError();
    char buf[256]{};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, sizeof(buf), nullptr);
    printf("[ERROR] %s: %lu — %s\n", context, err, buf);
}

int main() {
    // --- 여기에 구현 ---
    return 0;
}
```

## 코딩 컨벤션

- 모든 Win32 API 호출 후 반환값을 검사하고 실패 시 `PrintWinError`로 오류를 출력합니다.
- 핸들은 `RAII` 래퍼(`std::unique_ptr<HANDLE, ...>`) 또는 `goto cleanup` 패턴으로 반드시 닫습니다.
- 성능 측정에는 `QueryPerformanceCounter` / `QueryPerformanceFrequency`를 사용합니다.
- 비교 실험(예: CS vs SRWLock)은 동일 파일 내 `#define USE_CRITICAL_SECTION` 분기로 처리합니다.
- 프로세스 간 데이터 교환 모듈은 서버/클라이언트 실행 파일을 별도 프로젝트로 분리합니다.

## 프로젝트 속성 기본값

```
구성 타입: Application (.exe)
문자 집합: Unicode
C++ 표준: /std:c++17
추가 종속성: (Phase별로 추가)
  Phase 5 IO: ntdll.lib (선택적)
경고 수준: /W4
```

## Phase별 학습 진행 현황

| Phase | 상태 | 비고 |
| --- | --- | --- |
| Phase 1: Process | 미시작 | |
| Phase 2: Thread | 미시작 | |
| Phase 3: Memory | 미시작 | |
| Phase 4: Sync | 미시작 | |
| Phase 5: IO | 미시작 | |
| Phase 6: IPC | 미시작 | |

## 주의 사항

- Phase 5의 `17_DeviceIO`는 실제 드라이버 없이 개념 코드만 작성합니다. 드라이버 개발은 별도 WDK 환경이 필요합니다.
- `Phase6_IPC/19_SharedMemory`에서 서버·클라이언트를 별도 프로세스로 실행해야 합니다.
- Fiber(`Phase2_Thread/06_Fiber`)는 같은 스레드 내에서만 전환 가능합니다. 멀티스레드와 혼용 시 데드락 주의.
- 성능 측정 코드는 `Release` 구성으로 빌드해야 의미 있는 수치를 얻을 수 있습니다.
- ETW 이벤트 쓰기(향후 추가 시)는 관리자 권한이 필요합니다.
