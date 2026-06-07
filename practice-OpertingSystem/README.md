# Operating System 내부 학습 저장소

프로세스 관리부터 스레드 스케줄링, 가상 메모리, 동기화 프리미티브, I/O 서브시스템, IPC까지 Windows OS 내부 동작을 C++로 직접 실습하기 위한 Visual C++ 실습 모음입니다.

> **참고**: Win32 API 기초(`practice-Win32`의 Phase 3)를 먼저 이수한 뒤 이 저장소에서 심화 학습을 진행합니다.

## 빠른 시작

Visual Studio가 설치된 환경에서 각 Phase 폴더의 솔루션 또는 개별 프로젝트를 빌드합니다.

```powershell
MSBuild Phase1_Process\Phase1.sln /p:Configuration=Debug /p:Platform=x64
```

일부 모듈(Raw 소켓, 드라이버 통신, ETW 쓰기)은 관리자 권한으로 실행해야 합니다.

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 빌드 방법, 전체 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 학습 교안, 실습 절차, 확장 과제 |

## 전체 로드맵

| Phase | 폴더 | 학습 주제 |
| --- | --- | --- |
| Phase 1 | `Phase1_Process` | 프로세스 생성, 모니터링, Job Object |
| Phase 2 | `Phase2_Thread` | 스레드 생성, 우선순위, 스레드 풀, Fiber |
| Phase 3 | `Phase3_Memory` | 가상 메모리, 힙, 메모리 매핑 파일 |
| Phase 4 | `Phase4_Sync` | 동기화 프리미티브 성능 및 패턴 비교 |
| Phase 5 | `Phase5_IO` | Overlapped I/O, 파일 시스템 알림, DeviceIoControl |
| Phase 6 | `Phase6_IPC` | 파이프, 공유 메모리, Mailslot, LPC 개념 |

## 모듈 목차

### Phase 1: 프로세스 관리

- `01_Process_Creation` — `CreateProcess` 옵션, `STARTUPINFO`, stdout 리다이렉션
- `02_Process_Monitor` — `OpenProcess`, `QueryProcessInformation`, Toolhelp 스냅샷
- `03_Job_Objects` — Job 생성, 메모리/CPU 제한, 자식 프로세스 배정, 회계 정보

### Phase 2: 스레드 & 스케줄링

- `04_Thread_Basics` — `CreateThread`, 우선순위, `SetThreadAffinityMask`
- `05_ThreadPool` — `QueueUserWorkItem`, `CreateThreadpoolWork`, I/O 완료 콜백
- `06_Fiber` — `ConvertThreadToFiber`, `CreateFiber`, 협력적 컨텍스트 전환

### Phase 3: 메모리 관리

- `07_VirtualMemory` — `VirtualAlloc`, `VirtualProtect`, `VirtualQuery`, 페이지 상태 추적
- `08_HeapManagement` — `HeapCreate`, 사용자 정의 힙, 단편화 실험
- `09_MemoryMappedFile` — `CreateFileMapping`, `MapViewOfFile`, 프로세스 간 공유 메모리

### Phase 4: 동기화

- `10_CriticalSection` — CS 진입/해제 성능 측정, `CRITICAL_SECTION` 스핀카운트
- `11_SRWLock` — `AcquireSRWLockShared`/`Exclusive`, 읽기-쓰기 비율별 성능 비교
- `12_ConditionVariable` — `CONDITION_VARIABLE`, `SleepConditionVariableCS`, Producer-Consumer
- `13_EventSemaphore` — 수동/자동 리셋 이벤트, Semaphore 기반 카운팅 패턴
- `14_Interlocked` — `InterlockedIncrement`, `InterlockedCompareExchange`, Lock-Free 스택

### Phase 5: I/O 서브시스템

- `15_AsyncFileIO` — Overlapped 파일 읽기/쓰기, Completion Port 연동
- `16_FSNotify` — `ReadDirectoryChangesW`, 파일/폴더 변경 알림
- `17_DeviceIO` — `DeviceIoControl`로 드라이버 IOCTL 전송 개념

### Phase 6: IPC

- `18_Pipe_Advanced` — 비동기 Named Pipe 서버, 다중 인스턴스 연결
- `19_SharedMemory` — 이름 있는 공유 메모리 세그먼트, 프로세스 간 데이터 교환
- `20_Mailslot` — Mailslot 단방향 브로드캐스트 IPC

## 권장 학습 방식

1. `LEARNING_GUIDE.md`에서 해당 모듈의 목표와 관찰 포인트를 읽습니다.
2. Process Explorer, ProcMon, WinDbg, VMMap으로 실행 결과를 교차 검증합니다.
3. `HRESULT`, `NTSTATUS`, `GetLastError()` 오류 코드를 반드시 확인합니다.
4. 확장 과제를 구현하고 전체 솔루션을 재빌드합니다.
