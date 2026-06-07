# Operating System 내부 통합 학습 교안

모든 모듈을 순서대로 학습하기 위한 통합 교안입니다. 각 항목은 목표, 관찰 포인트, 핵심 API, 확장 과제로 구성됩니다.

## 공통 실습 절차

1. `main.cpp`에서 API 호출 순서와 핸들 생명 주기를 파악합니다.
2. Process Explorer, VMMap, ProcMon으로 실행 결과를 교차 검증합니다.
3. 오류 경로를 의도적으로 유발하고 `GetLastError()` 코드를 분석합니다.
4. 성능 실험은 `Release` 빌드로 `QueryPerformanceCounter`를 사용해 측정합니다.
5. 확장 과제를 구현하고 전체 솔루션을 재빌드합니다.

---

## Phase 1: 프로세스 관리

### 01_Process_Creation

- **목표**: `CreateProcess`의 주요 파라미터(`STARTUPINFO`, `PROCESS_INFORMATION`, 생성 플래그)를 이해합니다.
- **관찰**: `CREATE_NEW_CONSOLE`, `CREATE_SUSPENDED`, `DETACHED_PROCESS` 플래그가 동작에 미치는 차이를 확인합니다.
- **핵심 API**: `CreateProcess`, `WaitForSingleObject`, `GetExitCodeProcess`, `TerminateProcess`
- **확장 과제**: stdout/stderr 파이프 리다이렉션을 추가하고 자식 프로세스 출력을 부모에서 읽습니다.

### 02_Process_Monitor

- **목표**: `Toolhelp32` 스냅샷으로 실행 중인 프로세스 목록과 정보를 조회합니다.
- **관찰**: `PROCESSENTRY32`의 `th32ParentProcessID`로 프로세스 트리를 재구성하는 과정을 확인합니다.
- **핵심 API**: `CreateToolhelp32Snapshot`, `Process32First`, `Process32Next`, `OpenProcess`, `QueryFullProcessImageName`
- **확장 과제**: 특정 프로세스 이름을 반복 모니터링하며 생성/종료 시 출력하는 감시 루프를 구현합니다.

### 03_Job_Objects

- **목표**: Job Object로 프로세스 그룹에 메모리·CPU·시간 제한을 적용합니다.
- **관찰**: `JOBOBJECT_BASIC_LIMIT_INFORMATION`의 `ActiveProcessLimit`으로 자식 프로세스 수를 제한하고 초과 시 `CreateProcess` 실패를 확인합니다.
- **핵심 API**: `CreateJobObject`, `AssignProcessToJobObject`, `SetInformationJobObject`, `QueryInformationJobObject`
- **확장 과제**: `IOCP` 연결로 `JOB_OBJECT_MSG_EXIT_PROCESS` 완료 알림을 수신합니다.

---

## Phase 2: 스레드 & 스케줄링

### 04_Thread_Basics

- **목표**: `CreateThread`로 스레드를 생성하고 우선순위와 CPU 친화성을 제어합니다.
- **관찰**: `SetThreadPriority(THREAD_PRIORITY_HIGHEST)`로 설정된 스레드가 낮은 우선순위 스레드보다 먼저 CPU를 점유하는 비율을 측정합니다.
- **핵심 API**: `CreateThread`, `SetThreadPriority`, `SetThreadAffinityMask`, `GetCurrentThread`
- **확장 과제**: `SuspendThread`/`ResumeThread`로 특정 스레드를 일시 중단하고 재개합니다.

### 05_ThreadPool

- **목표**: Windows 스레드 풀 API로 작업 항목을 큐잉하고 완료 콜백을 받습니다.
- **관찰**: `CreateThreadpool` 없이 기본 풀 `QueueUserWorkItem`을 쓸 때와 `CreateThreadpoolWork`로 전용 풀을 만들 때의 차이를 비교합니다.
- **핵심 API**: `CreateThreadpool`, `SetThreadpoolThreadMinimum/Maximum`, `CreateThreadpoolWork`, `SubmitThreadpoolWork`, `WaitForThreadpoolWorkCallbacks`
- **확장 과제**: `CreateThreadpoolTimer`로 주기적 콜백을 구현합니다.

### 06_Fiber

- **목표**: Fiber로 협력적(Cooperative) 컨텍스트 전환을 구현합니다.
- **관찰**: `SwitchToFiber`가 스케줄러 없이 호출자가 직접 전환 시점을 결정하는 구조를 확인합니다.
- **핵심 API**: `ConvertThreadToFiber`, `CreateFiber`, `SwitchToFiber`, `DeleteFiber`
- **확장 과제**: Fiber 기반 간단한 코루틴 생성기(Generator)를 구현합니다.

---

## Phase 3: 메모리 관리

### 07_VirtualMemory

- **목표**: `VirtualAlloc`/`VirtualFree`로 가상 주소 공간을 예약·커밋·해제하는 흐름을 이해합니다.
- **관찰**: `VirtualQuery`로 메모리 영역 상태(MEM_RESERVE/MEM_COMMIT, PAGE_*)를 출력합니다. VMMap으로 프로세스 주소 공간 레이아웃을 시각화합니다.
- **핵심 API**: `VirtualAlloc`, `VirtualFree`, `VirtualProtect`, `VirtualQuery`
- **확장 과제**: 가드 페이지(`PAGE_GUARD`)를 설정하고 접근 시 `EXCEPTION_GUARD_PAGE` 예외를 SEH로 처리합니다.

### 08_HeapManagement

- **목표**: `HeapCreate`로 전용 힙을 만들고 단편화 행태를 실험합니다.
- **관찰**: 기본 프로세스 힙(`GetProcessHeap`)과 사용자 정의 힙의 주소 범위를 비교합니다.
- **핵심 API**: `HeapCreate`, `HeapAlloc`, `HeapFree`, `HeapDestroy`, `HeapSize`
- **확장 과제**: `HEAP_NO_SERIALIZE` 플래그 힙에서 멀티스레드 접근 시 충돌을 실험합니다.

### 09_MemoryMappedFile

- **목표**: `CreateFileMapping` + `MapViewOfFile`로 파일 내용을 메모리에 직접 매핑하고 프로세스 간 공유 메모리를 실습합니다.
- **관찰**: 매핑된 메모리에 쓴 데이터가 파일에 반영되는 타이밍(`FlushViewOfFile`)을 확인합니다.
- **핵심 API**: `CreateFileMapping`, `MapViewOfFile`, `UnmapViewOfFile`, `FlushViewOfFile`
- **확장 과제**: 이름 있는 섹션(`INVALID_HANDLE_VALUE` + 이름)으로 두 프로세스가 동일 메모리를 공유합니다.

---

## Phase 4: 동기화

### 10_CriticalSection

- **목표**: `CRITICAL_SECTION`의 진입/해제 오버헤드를 측정하고 스핀카운트 효과를 실험합니다.
- **관찰**: `InitializeCriticalSectionAndSpinCount`로 스핀카운트를 높였을 때 짧은 임계 구역에서 컨텍스트 전환이 줄어드는지 확인합니다.
- **핵심 API**: `InitializeCriticalSection`, `EnterCriticalSection`, `TryEnterCriticalSection`, `LeaveCriticalSection`
- **확장 과제**: CS 없이 전역 카운터를 동시에 증가시켜 데이터 경쟁이 발생함을 측정합니다.

### 11_SRWLock

- **목표**: `SRWLOCK`으로 읽기-쓰기 잠금을 구현하고 CS와 성능을 비교합니다.
- **관찰**: 읽기 비율이 높을 때 SRWLock이 CS보다 처리량이 높음을 측정합니다.
- **핵심 API**: `InitializeSRWLock`, `AcquireSRWLockShared`, `ReleaseSRWLockShared`, `AcquireSRWLockExclusive`
- **확장 과제**: 업그레이드 불가 제약(Shared → Exclusive 직접 업그레이드 없음)을 실험하고 우회 패턴을 구현합니다.

### 12_ConditionVariable

- **목표**: `CONDITION_VARIABLE`과 `SleepConditionVariableCS`로 Producer-Consumer 패턴을 구현합니다.
- **관찰**: `WakeConditionVariable`이 정확히 하나의 대기 스레드를 깨우고 `WakeAllConditionVariable`이 전체를 깨우는 차이를 확인합니다.
- **핵심 API**: `InitializeConditionVariable`, `SleepConditionVariableCS`, `WakeConditionVariable`, `WakeAllConditionVariable`
- **확장 과제**: 유한 크기 큐(bounded buffer)를 구현하고 생산자/소비자 각 2개 스레드로 실험합니다.

### 13_EventSemaphore

- **목표**: 수동/자동 리셋 이벤트의 동작 차이와 Semaphore 기반 카운팅 패턴을 이해합니다.
- **관찰**: `CreateEvent(NULL, TRUE, ...)` 수동 리셋은 `ResetEvent` 없이 계속 시그널 상태임을 확인합니다.
- **핵심 API**: `CreateEvent`, `SetEvent`, `ResetEvent`, `CreateSemaphore`, `ReleaseSemaphore`
- **확장 과제**: Semaphore로 스레드 풀 크기를 제한하는 패턴을 구현합니다.

### 14_Interlocked

- **목표**: `InterlockedIncrement`, `InterlockedCompareExchange`로 Lock-Free 자료 구조를 구현합니다.
- **관찰**: 비잠금 스택의 ABA 문제를 실험하고 `InterlockedCompareExchange128`(또는 태그 포인터)으로 해결합니다.
- **핵심 API**: `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedCompareExchange`, `InterlockedExchange`
- **확장 과제**: Lock-Free LIFO 스택을 구현하고 멀티스레드 환경에서 정확성을 검증합니다.

---

## Phase 5: I/O 서브시스템

### 15_AsyncFileIO

- **목표**: Overlapped I/O로 파일을 비동기로 읽고 IOCP와 연동합니다.
- **관찰**: `ReadFile`에 `OVERLAPPED` 구조체를 전달했을 때 즉시 반환되고 `GetQueuedCompletionStatus`에서 완료를 수신함을 확인합니다.
- **핵심 API**: `CreateFile(FILE_FLAG_OVERLAPPED)`, `ReadFile`, `WriteFile`, `OVERLAPPED`, `CreateIoCompletionPort`
- **확장 과제**: 여러 파일을 동시에 읽고 완료 순서가 발행 순서와 다를 수 있음을 관찰합니다.

### 16_FSNotify

- **목표**: `ReadDirectoryChangesW`로 디렉토리 변경(파일 생성/삭제/수정)을 비동기로 감지합니다.
- **관찰**: `FILE_NOTIFY_INFORMATION`의 `Action` 필드(`FILE_ACTION_ADDED`, `FILE_ACTION_MODIFIED` 등)를 파싱합니다.
- **핵심 API**: `ReadDirectoryChangesW`, `FILE_NOTIFY_INFORMATION`, `WaitForSingleObject`
- **확장 과제**: 재귀 감시(`bWatchSubtree = TRUE`)와 비재귀 감시의 이벤트 수를 비교합니다.

### 17_DeviceIO

- **목표**: `DeviceIoControl`로 커널 드라이버에 IOCTL을 전송하는 패턴을 이해합니다. (개념 실습)
- **관찰**: `\\.\PhysicalDrive0` 또는 `\\.\C:`를 열어 `IOCTL_DISK_GET_DRIVE_GEOMETRY`를 호출하고 디스크 정보를 읽습니다.
- **핵심 API**: `CreateFile("\\.\...")`, `DeviceIoControl`, `IOCTL_DISK_GET_DRIVE_GEOMETRY`
- **확장 과제**: `IOCTL_STORAGE_QUERY_PROPERTY`로 드라이브의 `StorageDeviceProperty`를 읽습니다.

---

## Phase 6: IPC

### 18_Pipe_Advanced

- **목표**: Overlapped Named Pipe로 서버가 여러 클라이언트를 동시에 처리합니다.
- **관찰**: `ConnectNamedPipe(OVERLAPPED)`로 각 파이프 인스턴스가 독립적으로 연결을 기다리는 구조를 확인합니다.
- **핵심 API**: `CreateNamedPipe(FILE_FLAG_OVERLAPPED)`, `ConnectNamedPipe`, `DisconnectNamedPipe`, `PIPE_UNLIMITED_INSTANCES`
- **확장 과제**: 파이프 인스턴스에 보안 기술자(`SECURITY_ATTRIBUTES`)를 적용해 특정 사용자만 연결 가능하게 합니다.

### 19_SharedMemory

- **목표**: 이름 있는 파일 매핑으로 두 프로세스가 동일 메모리 페이지를 공유합니다.
- **관찰**: 서버 프로세스에서 쓴 데이터가 클라이언트 프로세스에서 즉시 보임을 확인합니다. 접근 순서를 Mutex로 보호합니다.
- **핵심 API**: `CreateFileMapping(INVALID_HANDLE_VALUE, ...)`, `OpenFileMapping`, `MapViewOfFile`
- **확장 과제**: 헤더(시퀀스 번호, 길이)를 포함한 간단한 링 버퍼를 공유 메모리에 구현합니다.

### 20_Mailslot

- **목표**: Mailslot으로 단방향 브로드캐스트 메시지를 전송합니다.
- **관찰**: `CreateMailslot`로 수신 측을 만들고 `CreateFile("\\.\mailslot\...")`로 로컬 클라이언트가 메시지를 씁니다.
- **핵심 API**: `CreateMailslot`, `GetMailslotInfo`, `CreateFile("\\.\mailslot\...")`, `WriteFile`, `ReadFile`
- **확장 과제**: Named Pipe, Mailslot, 공유 메모리 세 방식의 메시지 전달 지연과 복잡도를 비교 정리합니다.
