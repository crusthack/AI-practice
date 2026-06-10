# CLAUDE.md — practice-OpertingSystem 에이전트 지침

이 파일은 이 워크스페이스에서 작업하는 에이전트를 위한 지침입니다.

## 워크스페이스 개요

- **주제**: Windows NT 커널 내부 동작 — 프로세스, 스레드/스케줄러, 가상 메모리, 동시성, I/O, IPC
- **언어**: C++ (C++17 이상)
- **빌드 시스템**: Visual Studio 2022 / MSBuild (`.vcxproj`)
- **대상 플랫폼**: x64 Windows
- **선행 학습**: `../practice-Win32` Phase 3 (FileIO, ProcessThread, Synchronization, Memory)
- **목적**: Win32 기초 위에서 NT 커널 내부 구조를 직접 실험하는 심화 저장소

## 디렉토리 구조

```
practice-OpertingSystem/
├── Phase1_Process/         # 01~04: CreateProcess·Monitor·Job·NT Internals(EPROCESS·PEB)
├── Phase2_Thread/          # 05~09: Thread·ThreadPool·Fiber·Scheduler·NUMA
├── Phase3_Memory/          # 10~14: VirtualMemory·Heap·MMF·PageTable Walk·LargePages
├── Phase4_Sync/            # 15~21: CS·SRW·CV·Event·Interlocked·x86 MemModel·CacheCoherency
├── Phase5_IO/              # 22~24: AsyncFileIO(IOCP)·FSNotify·DeviceIO
├── Phase6_IPC/             # 25~27: Pipe·SharedMemory·Mailslot
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 전체 모듈 번호 체계

| Phase | 모듈 번호 | 폴더명 패턴 |
| --- | --- | --- |
| Phase 1 | 01~04 | `Phase1_Process/01_Process_Creation/` … `04_NT_Process_Internals/` |
| Phase 2 | 05~09 | `Phase2_Thread/05_Thread_Basics/` … `09_NUMA_ProcessorGroups/` |
| Phase 3 | 10~14 | `Phase3_Memory/10_VirtualMemory/` … `14_LargePages_MemProtection/` |
| Phase 4 | 15~21 | `Phase4_Sync/15_CriticalSection/` … `21_CacheCoherency/` |
| Phase 5 | 22~24 | `Phase5_IO/22_AsyncFileIO/` … `24_DeviceIO/` |
| Phase 6 | 25~27 | `Phase6_IPC/25_Pipe_Advanced/` … `27_Mailslot/` |

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 모듈 폴더를 만듭니다 (번호 체계 준수).  
   예: `Phase3_Memory/13_PageTable_Walk/`
2. Visual Studio에서 빈 C++ 콘솔 프로젝트를 생성합니다.
3. 아래 `main.cpp` 초기화 템플릿을 기반으로 작성합니다.
4. Phase 폴더의 솔루션(예: `Phase3_Memory/Phase3.sln`)에 프로젝트를 추가합니다.

## main.cpp 초기화 템플릿

```cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>

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
- 성능 측정에는 `QueryPerformanceCounter` / `QueryPerformanceFrequency` 또는 `__rdtsc()`를 사용합니다.
- 비교 실험(예: CS vs SRWLock, 배리어 있음 vs 없음)은 동일 파일 내 `#define` 분기로 처리합니다.
- 프로세스 간 데이터 교환 모듈은 서버/클라이언트 실행 파일을 별도 프로젝트로 분리합니다.
- 심화 모듈(04, 08, 13, 20, 21)의 이론 코드는 WinDbg 출력과 반드시 교차 검증합니다.

## 프로젝트 속성 기본값

```
구성 타입    : Application (.exe)
문자 집합    : Unicode
C++ 표준     : /std:c++17
경고 수준    : /W4
추가 정의    : WIN32_LEAN_AND_MEAN;NOMINMAX

추가 종속성 (Phase별):
  Phase 1 (04_NT_Internals)    : ntdll.lib (GetProcAddress로 NtQueryInformationProcess 사용 시 불필요)
  Phase 5 (22_AsyncFileIO)     : (없음 — Kernel32.lib 기본 포함)
  Phase 4 (21_CacheCoherency)  : /arch:AVX2 권장 (CLFLUSH 테스트)
```

## 도구 요구사항 (모듈별)

| 모듈 | 필요 도구 | 권한 |
| --- | --- | --- |
| 04_NT_Internals | Process Hacker 2, WinDbg | 관리자 권장 |
| 08_Scheduler | WinDbg (`!thread`), Process Explorer | 일반 |
| 13_PageTable_Walk | WinDbg (`!pte !pfn !vm`), VMMap | 커널 디버거 권장 |
| 14_LargePages | secpol.msc (SeLockMemoryPrivilege 설정) | 관리자 필수 |
| 20_x86_MemoryModel | 어셈블리 뷰 (`/FAcs`), MFENCE 테스트 | 일반 |
| 21_CacheCoherency | Intel VTune 또는 AMD uProf (LLC miss 측정) | 일반 |
| 24_DeviceIO | 관리자 권한 (PhysicalDrive0 접근) | **관리자 필수** |

## Phase별 학습 진행 현황

총 27개 모듈 (기존 20 + 심화 7 — 04, 08, 09, 13, 14, 20, 21)

| Phase | 상태 | 일반 모듈 | 심화 모듈 | 비고 |
| --- | --- | --- | --- | --- |
| Phase 1: Process | 미시작 | 01, 02, 03 | **04** NT Internals | 04는 관리자 권한 권장 |
| Phase 2: Thread | 미시작 | 05, 06, 07 | **08** Scheduler, **09** NUMA | WinDbg 필요 (08) |
| Phase 3: Memory | 미시작 | 10, 11, 12 | **13** PageTable, **14** LargePages | WinDbg 필수 (13) |
| Phase 4: Sync | 미시작 | 15, 16, 17, 18, 19 | **20** MemoryModel, **21** CacheCoherency | Release 빌드 필수 |
| Phase 5: IO | 미시작 | 22, 23, 24 | — | 24는 관리자 권한 필요 |
| Phase 6: IPC | 미시작 | 25, 26, 27 | — | 26은 서버·클라이언트 별도 프로젝트 |

## 주의 사항

- **04_NT_Process_Internals**: `NtQueryInformationProcess`는 ntdll 미공개 API — `GetProcAddress`로 런타임 로딩하거나 `ntdll.lib`를 수동 링크합니다.
- **07_Fiber**: 같은 스레드 내에서만 전환 가능합니다. 멀티스레드와 혼용 시 크래시 발생.
- **08_Scheduler_Internals**: REALTIME 우선순위 클래스 설정 시 마우스·키보드가 멈출 수 있습니다 — 반드시 타임아웃을 걸고 테스트하세요.
- **13_PageTable_Walk**: WinDbg `!pte` 명령은 커널 디버거(KD) 또는 로컬 커널 디버깅 활성화 환경에서만 정확합니다.
- **14_LargePages_MemProtection**: `VirtualAlloc(MEM_LARGE_PAGES)`는 `SeLockMemoryPrivilege` 활성화 필수 — `secpol.msc → 로컬 정책 → 사용자 권한 할당 → 메모리의 페이지 잠금`에서 계정 추가.
- **20_x86_MemoryModel**: Store-Load 재정렬 실험은 **Release x64 빌드** + 멀티코어 환경에서만 관측 가능합니다. Debug 빌드는 최적화 부재로 재정렬이 발생하지 않을 수 있습니다.
- **24_DeviceIO**: `\\.\PhysicalDrive0` 접근은 관리자 권한이 필요합니다. 실제 드라이버 개발은 별도 WDK 환경이 필요합니다.
- **26_SharedMemory**: 서버·클라이언트를 반드시 별도 프로세스로 실행해야 합니다.
- ETW 이벤트 쓰기(향후 추가 시)는 관리자 권한이 필요합니다.
- 모든 성능 측정 모듈(15~21)은 **Release x64** 구성으로 빌드해야 의미 있는 수치를 얻을 수 있습니다.
