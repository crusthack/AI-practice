# Operating System 내부 학습 저장소

프로세스 관리부터 스레드 스케줄링, 가상 메모리, 동기화 프리미티브, I/O 서브시스템, IPC까지  
Windows NT 커널 내부 동작을 C++로 직접 실습하기 위한 Visual C++ 실습 모음입니다.

## 선행 학습

`practice-Win32` Phase 3 (FileIO, ProcessThread, Synchronization, Memory)를 이수한 학습자를 대상으로 합니다.

## 필요 도구

| 도구 | 용도 | 필수 여부 |
|---|---|---|
| Visual Studio 2022 (C++ Desktop) | 빌드 환경 | **필수** |
| [Process Explorer](https://learn.microsoft.com/sysinternals/downloads/process-explorer) | 프로세스·핸들·Job·토큰 관찰 | **필수** |
| [Process Monitor](https://learn.microsoft.com/sysinternals/downloads/procmon) | 파일·레지스트리·프로세스 이벤트 | **필수** |
| [VMMap](https://learn.microsoft.com/sysinternals/downloads/vmmap) | 가상 주소 공간·PFN 상태 시각화 | Phase 3 필수 |
| WinDbg Preview (Store) | `!pte` `!vm` `!pfn` 커널 구조 탐색 | Phase 3~4 **필수** |
| [Process Hacker 2](https://processhacker.sourceforge.io/) | EPROCESS·PEB·토큰 필드 탐색 | Phase 1 심화 필수 |
| Intel VTune / AMD uProf | 하드웨어 PMC (LLC miss, IPC 측정) | Phase 4 심화 권장 |
| DebugView++ | `OutputDebugString` 캡처 | 권장 |

> Phase 5의 `24_DeviceIO`, Phase 6 일부, Phase 1의 `04_NT_Internals`는 **관리자 권한**이 필요합니다.

## 빠른 시작

각 Phase 폴더에 솔루션 파일(`Phase1.sln` 등)을 생성한 뒤 아래 명령으로 빌드합니다.

```powershell
# Phase 솔루션 전체 빌드
MSBuild Phase1_Process\Phase1.sln /p:Configuration=Release /p:Platform=x64

# 개별 프로젝트 빌드
MSBuild Phase1_Process\01_Process_Creation\01_Process_Creation.vcxproj /p:Configuration=Debug /p:Platform=x64
```

> **중요**: 성능 측정 모듈(Phase 4 전체 15~21, Phase 5의 22_AsyncFileIO)은 반드시 **Release x64** 빌드로 실행하세요. Debug 빌드는 최적화 미적용으로 의미 있는 수치를 얻을 수 없으며, 특히 20_x86_MemoryModel의 Store-Load 재정렬은 Release 빌드에서만 관측됩니다.

## 문서 구조

| 문서 | 용도 |
|---|---|
| `README.md` | 저장소 개요, 로드맵, 모듈 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 코딩 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 이론·실습·확인 질문·확장 과제 |

---

## 전체 로드맵

```
[선행: practice-Win32 Phase 3]
              │
              ▼
 ┌──── Phase 1: 프로세스 ────────────────────────────────────┐
 │  01 CreateProcess  02 Monitor  03 Job Objects             │
 │  04 NT Internals (EPROCESS·PEB·Handle·Token)              │
 └──────────────────────┬───────────────────────────────────┘
                        │
        ┌───────────────▼─────────────────────┐
        │        Phase 2: 스레드 & 스케줄러     │
        │  05 Thread Basics  06 ThreadPool      │
        │  07 Fiber                             │
        │  08 Scheduler Internals (NT 심화)     │
        │  09 NUMA & Processor Groups           │
        └──────┬──────────────────┬────────────┘
               │                  │
  ┌────────────▼───┐    ┌─────────▼──────────────────────┐
  │  Phase 3: 메모리 │    │       Phase 4: 동시성           │
  │  10 VirtualMem  │    │  15 CriticalSection            │
  │  11 Heap       │    │  16 SRWLock                    │
  │  12 MMF        │    │  17 ConditionVariable          │
  │  13 PageTable  │    │  18 Event & Semaphore          │
  │     Walk (심화) │    │  19 Interlocked (Lock-Free)    │
  │  14 LargePages │    │  20 x86 Memory Model (심화)    │
  │     ASLR·DEP   │    │  21 Cache Coherency (심화)     │
  └────────┬───────┘    └──────────────┬─────────────────┘
           │                           │
           └──────────┬────────────────┘
                      │
          ┌───────────▼───────────────┐
          │      Phase 5: I/O          │
          │  22 AsyncFileIO (IOCP)    │
          │  23 FSNotify              │
          │  24 DeviceIO              │
          └───────────┬───────────────┘
                      │
          ┌───────────▼───────────────┐
          │      Phase 6: IPC          │
          │  25 Pipe · 26 SharedMem   │
          │  27 Mailslot              │
          └───────────────────────────┘
```

**선행 관계 핵심**
- **04** (NT Internals) → Phase 3·4의 "왜"를 이해하는 기반 — 가상 주소, 핸들, 토큰 개념이 전제됨
- **08** (Scheduler) → 14 LargePages, 20 MemoryModel, 21 CacheCoherency의 이해에 필수
- **13** (PageTable Walk) → 14, 20, 21의 하드웨어 기반 이해에 필수
- **20·21** (Memory Model, Cache) → Lock-Free(19) 이전에 학습하면 훨씬 깊은 이해 가능

---

## 모듈 목차

난이도: ★☆☆ 입문 · ★★☆ 중급 · ★★★ 고급 · ★★★★ 전문  
예상 시간: 코딩 + 관찰 + 확장 과제 포함

---

### Phase 1: 프로세스 관리 (4 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 01 | `01_Process_Creation` | CreateProcess 플래그·핸들 상속·stdout 리다이렉션 | — | ★★☆ | 3h |
| 02 | `02_Process_Monitor` | Toolhelp 스냅샷·프로세스 트리·PID 감시 | 01 | ★★☆ | 2h |
| 03 | `03_Job_Objects` | Job 제한·IOCP 완료 알림·회계 정보 | 01 | ★★★ | 4h |
| **04** | **`04_NT_Process_Internals`** | **EPROCESS·PEB·핸들 테이블·액세스 토큰** | 01~03 | ★★★★ | 5h |

**Phase 1 수료 후**: NT 프로세스 오브젝트 내부 구조를 PEB 수준까지 탐색하고, 보안 토큰·핸들 테이블의 동작을 설명할 수 있다.

---

### Phase 2: 스레드 & 스케줄러 (5 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 05 | `05_Thread_Basics` | CreateThread·우선순위·CPU 친화성·TLS | Phase 1 완료 | ★★☆ | 3h |
| 06 | `06_ThreadPool` | TP_WORK/TP_TIMER/TP_IO·환경·정리 그룹 | 05 | ★★☆ | 4h |
| 07 | `07_Fiber` | ConvertThreadToFiber·SwitchToFiber·코루틴 패턴 | 05 | ★★★ | 4h |
| **08** | **`08_Scheduler_Internals`** | **KTHREAD 상태 머신·우선순위 부스트·IRQL·퀀텀** | 05 | ★★★★ | 5h |
| **09** | **`09_NUMA_ProcessorGroups`** | **NUMA 토폴로지·프로세서 그룹·NUMA 인식 할당** | 08 | ★★★ | 4h |

**Phase 2 수료 후**: NT 스케줄러가 우선순위·퀀텀·부스트를 어떻게 결합하는지 설명하고, NUMA-aware 스레드 풀을 구성할 수 있다.

---

### Phase 3: 메모리 관리 (5 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 10 | `10_VirtualMemory` | Reserve/Commit/Decommit·페이지 보호·가드 페이지 | Phase 1 완료 | ★★★ | 4h |
| 11 | `11_HeapManagement` | HeapCreate·LFH·단편화 실험·HEAP_NO_SERIALIZE | 10 | ★★☆ | 3h |
| 12 | `12_MemoryMappedFile` | 섹션 오브젝트·다중 프로세스 뷰·FlushViewOfFile | 10 | ★★☆ | 3h |
| **13** | **`13_PageTable_Walk`** | **x64 4단계 페이지 테이블·PFN 데이터베이스·페이지 폴트 분류** | 10, WinDbg | ★★★★ | 5h |
| **14** | **`14_LargePages_MemProtection`** | **Large Page·ASLR·DEP/NX·CFG** | 13 | ★★★ | 4h |

**Phase 3 수료 후**: x64 가상 주소가 물리 주소로 변환되는 전체 경로(CR3→PML4→PT)를 WinDbg로 추적하고, ASLR·DEP·CFG 보안 기법의 메모리 구조적 근거를 설명할 수 있다.

---

### Phase 4: 동시성 (7 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 15 | `15_CriticalSection` | 스핀카운트·CS 내부 구조·컨텍스트 전환 측정 | Phase 2 완료 | ★★☆ | 3h |
| 16 | `16_SRWLock` | 공유/독점 잠금·RTL_SRWLOCK 비트 구조·CS vs SRW 비교 | 15 | ★★☆ | 3h |
| 17 | `17_ConditionVariable` | Spurious Wakeup·Producer-Consumer·Bounded Buffer | 15 | ★★☆ | 4h |
| 18 | `18_EventSemaphore` | 수동/자동 리셋·WFMO·Semaphore 쓰로틀링 | 15 | ★★☆ | 3h |
| 19 | `19_Interlocked` | CAS 루프·ABA 문제·Lock-Free LIFO | 15 | ★★★ | 6h |
| **20** | **`20_x86_MemoryModel`** | **TSO 모델·Store Buffer·메모리 배리어·C++ memory_order** | 13, 19 | ★★★★ | 6h |
| **21** | **`21_CacheCoherency`** | **MESI 프로토콜·False Sharing·캐시 라인 패딩** | 20 | ★★★★ | 5h |

**Phase 4 수료 후**: x86 메모리 모델의 재정렬 규칙을 증명하고, MESI 프로토콜을 기반으로 False Sharing을 진단·제거할 수 있다. C++ `std::atomic` memory_order가 x86 명령어 수준에서 어떻게 구현되는지 설명할 수 있다.

---

### Phase 5: I/O 서브시스템 (3 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 22 | `22_AsyncFileIO` | Overlapped I/O·IOCP·per-op 컨텍스트 | Phase 2~4 완료 | ★★★ | 6h |
| 23 | `23_FSNotify` | ReadDirectoryChangesW·FILE_NOTIFY_INFORMATION | 22 | ★★☆ | 3h |
| 24 | `24_DeviceIO` | DeviceIoControl·IOCTL 인코딩·디스크 쿼리 | 22 | ★★☆ | 2h |

---

### Phase 6: IPC (3 모듈)

| # | 모듈 | 핵심 주제 | 선행 | 난이도 | 시간 |
|---|---|---|---|---|---|
| 25 | `25_Pipe_Advanced` | Overlapped Named Pipe·다중 클라이언트 상태 머신 | Phase 5 완료 | ★★★ | 5h |
| 26 | `26_SharedMemory` | 이름 있는 섹션·Mutex 보호·링 버퍼 | 12, 15 | ★★☆ | 4h |
| 27 | `27_Mailslot` | 단방향 브로드캐스트·3종 IPC 성능 비교 | 25, 26 | ★★☆ | 3h |

---

## 권장 학습 방식

1. `LEARNING_GUIDE.md`의 **이론 배경**을 읽고 OS 내부 동작 흐름을 먼저 머릿속에 그립니다.
2. 코드 작성 시 **모든 Win32 API 반환값**을 검사합니다 (`PrintWinError` 헬퍼 활용).
3. 실행 중 **Sysinternals / WinDbg**로 결과를 교차 검증합니다.
4. **확인 질문**에 직접 답해보고 막히면 이론 배경으로 돌아갑니다.
5. 여유가 있으면 **확장 과제**를 구현하고 솔루션을 재빌드합니다.
