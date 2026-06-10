# Operating System 내부 통합 학습 교안

각 모듈: **이론 배경 → 핵심 개념 → 학습 목표 → 핵심 API → 실습 절차 → 관찰 포인트 → 예상 출력 → 함정 & 주의 → 확인 질문 → 확장 과제**

## 공통 실습 원칙

1. 이론 배경을 읽고 OS 내부 흐름을 머릿속에 그린 뒤 코드를 작성합니다.
2. 모든 Win32 API 반환값을 검사하고 `PrintWinError`로 오류 경로를 확인합니다.
3. Process Explorer / VMMap / ProcMon / WinDbg로 실행 결과를 **눈으로** 교차 검증합니다.
4. 성능 측정은 항상 **Release x64** 빌드, 최소 3회 반복 평균을 사용합니다.
5. 확인 질문에 스스로 답해보고 막히면 이론 배경을 다시 읽습니다.

---

## Phase 1: 프로세스 관리

---

### 01_Process_Creation

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 없음

#### 이론 배경

Windows에서 프로세스는 커널의 `EPROCESS` 구조체로 표현된다. `CreateProcess`는 내부적으로 `NtCreateUserProcess` 시스템 콜을 호출해 ① EPROCESS 및 초기 ETHREAD 커널 오브젝트 생성, ② 가상 주소 공간 초기화, ③ PEB(Process Environment Block) 설정, ④ 초기 스레드 시작의 네 단계를 수행한다.

`STARTUPINFO`는 생성될 프로세스의 콘솔·표준 I/O 핸들 초기 상태를 지정한다. `PROCESS_INFORMATION`에는 반환된 프로세스·스레드 핸들이 들어오며 사용 후 반드시 닫아야 한다.

| 생성 플래그 | 동작 |
|---|---|
| `CREATE_NEW_CONSOLE` | 새 콘솔 창에서 실행 |
| `DETACHED_PROCESS` | 콘솔 없이 실행 (서비스 유사) |
| `CREATE_SUSPENDED` | 초기 스레드 정지 상태로 생성 → `ResumeThread`로 시작 |
| `CREATE_NEW_PROCESS_GROUP` | Ctrl+C 신호 그룹 격리 |
| `INHERIT_PARENT_AFFINITY` | 부모의 CPU 친화성 마스크 상속 |

stdout 리다이렉션: `CreatePipe`로 익명 파이프를 만든 뒤 쓰기 끝을 자식 `STARTUPINFO.hStdOutput`으로, 읽기 끝을 부모에서 `ReadFile`로 소비한다.

#### 학습 목표

1. `CreateProcess` 주요 파라미터의 역할과 상호작용을 설명할 수 있다.
2. `CREATE_SUSPENDED`로 생성 후 재개하는 패턴을 구현할 수 있다.
3. 익명 파이프로 자식 stdout을 부모에서 캡처할 수 있다.

#### 핵심 API

`CreateProcess` · `WaitForSingleObject` · `GetExitCodeProcess` · `TerminateProcess`  
`CreatePipe` · `SetHandleInformation` · `ResumeThread`

#### 실습 절차

1. `CreateProcess`로 `cmd.exe /c dir` 실행 → `PROCESS_INFORMATION` 핸들을 누수 없이 닫기.
2. `CREATE_SUSPENDED` 플래그로 생성 후 `ResumeThread` 전후 Process Explorer 스레드 상태 비교.
3. `CreatePipe` + `STARTUPINFO.hStdOutput` 설정 → `ReadFile` 루프로 자식 출력 수집.
4. `WaitForSingleObject(pi.hProcess, INFINITE)` + `GetExitCodeProcess`로 종료 코드 출력.

#### 관찰 포인트

- Process Explorer → 자식 **Parent PID**가 현재 프로세스 PID와 일치하는지 확인.
- `CREATE_SUSPENDED` 상태에서 ProcMon 프로세스 생성 이벤트가 이미 기록되는지 확인.
- 파이프 쓰기 핸들을 부모가 닫지 않으면 `ReadFile`이 영원히 반환되지 않는 이유를 확인.

#### 예상 출력 예시

```
[Child] Volume in drive C has no label.
[Child]  Directory of C:\Windows\System32
[Exit code] 0
```

#### 함정 & 주의사항

- `hProcess`·`hThread` 누수: 반드시 `CloseHandle` — 커널 오브젝트 참조 카운트 유지.
- `bInheritHandles = TRUE`면 프로세스 핸들 테이블 전체의 상속 가능 핸들이 복사된다 — 불필요한 핸들은 미리 `HANDLE_FLAG_INHERIT = 0`으로 설정.
- 파이프 쓰기 끝을 부모가 먼저 닫지 않으면 자식 종료 후에도 `ReadFile`이 블로킹된다.

#### 확인 질문

1. `lpApplicationName`과 `lpCommandLine`을 동시에 지정할 때 어느 것이 실행 파일 경로로 우선되는가?
2. `CREATE_SUSPENDED` 상태의 프로세스는 커널 오브젝트가 이미 생성된 것인가?
3. 파이프 버퍼가 가득 찼을 때 자식 `WriteFile`은 어떻게 동작하는가?

#### 확장 과제

익명 파이프 대신 이름 있는 파이프로 동일한 리다이렉션을 구현하고 코드 복잡도를 비교하세요.

---

### 02_Process_Monitor

**난이도** ★★☆ | **예상 시간** 2h | **선행 모듈** 01

#### 이론 배경

`CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)`은 호출 시점 커널 프로세스 테이블의 **스냅샷**을 만든다. 스냅샷 이후 생성·종료된 프로세스는 반영되지 않는다.

`PROCESSENTRY32.th32ParentProcessID`는 **생성 시점**의 부모 PID다. 부모가 이미 종료되고 PID가 재사용되더라도 값은 변하지 않는다 — 따라서 현재 시점의 부모-자식 관계를 반영하지 않을 수 있다.

`QueryFullProcessImageName`은 32비트 프로세스에서 64비트 프로세스 경로를 정확히 조회한다. `GetModuleFileNameEx`는 WOW64 경계에서 경로가 왜곡될 수 있다.

#### 학습 목표

1. Toolhelp32 스냅샷으로 전체 프로세스 목록을 조회하고 트리를 재구성할 수 있다.
2. 스냅샷 재촬영으로 프로세스 생성·종료 이벤트를 감지하는 루프를 구현할 수 있다.
3. `GetProcessTimes`로 CPU 사용 시간을 수집할 수 있다.

#### 핵심 API

`CreateToolhelp32Snapshot` · `Process32First` · `Process32Next`  
`OpenProcess` · `QueryFullProcessImageName` · `GetProcessTimes`

#### 실습 절차

1. `TH32CS_SNAPPROCESS` 스냅샷으로 전체 프로세스(PID, PPID, 이름) 출력.
2. `std::map<DWORD, DWORD>` 테이블로 트리를 재구성, 들여쓰기로 출력.
3. 2초 간격 스냅샷 반복 → 이전 스냅샷 대비 증감 PID를 "CREATED"/"EXITED"로 출력.
4. `GetProcessTimes`로 특정 프로세스의 커널·사용자 모드 CPU 시간 출력.

#### 관찰 포인트

- 시스템 프로세스(PID 4)는 `OpenProcess`가 실패하는지 확인.
- `PROCESS_QUERY_LIMITED_INFORMATION` vs `PROCESS_QUERY_INFORMATION` 권한 차이 실험.
- Process Explorer 트리 뷰와 내 출력 비교.

#### 예상 출력 예시

```
PID=1234  PPID=5678  explorer.exe
  PID=9012  PPID=1234  cmd.exe
    PID=3456  PPID=9012  notepad.exe
[MONITOR] CREATED PID=7890 notepad.exe
[MONITOR] EXITED  PID=3456
```

#### 함정 & 주의사항

- `Process32First`/`Next` 실패 시 `ERROR_NO_MORE_FILES`는 정상 종료다.
- PID 재사용: 단순 PID 비교만으로는 오탐 가능 — 생성 시간까지 비교해야 신뢰성이 높아진다.
- 스냅샷 핸들 `CloseHandle` 누락 주의.

#### 확인 질문

1. ETW로 프로세스 이벤트를 감지하면 Toolhelp 스냅샷 방식과 어떤 차이가 있는가?
2. `th32ParentProcessID`가 실제 부모가 아닐 수 있는 두 가지 시나리오는?
3. 64비트 OS에서 32비트 프로세스가 64비트 프로세스 경로를 왜곡 없이 조회하려면?

#### 확장 과제

`TH32CS_SNAPTHREAD`로 특정 프로세스의 스레드 목록과 우선순위를 출력하는 기능을 추가하세요.

---

### 03_Job_Objects

**난이도** ★★★ | **예상 시간** 4h | **선행 모듈** 01

#### 이론 배경

Job Object는 프로세스 그룹에 **단일 제어 단위**를 부여하는 커널 오브젝트다. 메모리·CPU·프로세스 수·실행 시간 제한을 한 번에 적용할 수 있다.

`AssignProcessToJobObject`는 이미 **다른 Job에 속한** 프로세스에는 실패한다. Windows 8 이후에는 중첩 Job(Nested Job)이 가능하지만 제한은 OR로 합쳐진다.

Job 완료 알림: `JOBOBJECT_ASSOCIATE_COMPLETION_PORT`로 IOCP를 연결하면 `JOB_OBJECT_MSG_EXIT_PROCESS`, `JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT_EXCEEDED` 등을 비동기로 수신한다.

Windows 8 이후 자식 프로세스는 부모가 Job에 속하면 **자동으로 같은 Job에 배정**된다. `CREATE_BREAKAWAY_FROM_JOB` 플래그로 탈출할 수 있다 (Job이 허용한 경우).

#### 학습 목표

1. Job으로 자식 그룹에 메모리·프로세스 수 제한을 적용할 수 있다.
2. IOCP를 Job에 연결해 프로세스 종료 알림을 비동기로 수신할 수 있다.
3. `QueryInformationJobObject`로 그룹 CPU·메모리 회계 정보를 수집할 수 있다.

#### 핵심 API

`CreateJobObject` · `AssignProcessToJobObject` · `SetInformationJobObject`  
`QueryInformationJobObject` · `CreateIoCompletionPort` · `GetQueuedCompletionStatus`

#### 실습 절차

1. Job 생성 → `CreateProcess(CREATE_SUSPENDED)` → `Assign` → `Resume`.
2. `ActiveProcessLimit = 3` 설정 → 세 번째 이후 자식 생성 실패 확인.
3. `ProcessMemoryLimit = 50MB` 설정 → 초과 시 프로세스 강제 종료 확인.
4. IOCP 연결 → 별도 스레드 `GQCS` 루프 → `JOB_OBJECT_MSG_EXIT_PROCESS` 수신.
5. 모든 자식 종료 후 `JobObjectBasicAccountingInformation`으로 회계 정보 출력.

#### 관찰 포인트

- Process Explorer → 프로세스 속성의 **Job** 탭에서 Job 핸들·제한 확인.
- `TerminateJobObject`로 Job 전체를 종료했을 때 자식 일괄 제거 시간 측정.

#### 예상 출력 예시

```
[JOB] PID 1234 assigned.
[JOB] PID 5678 assigned.
[JOB] CreateProcess failed — ActiveProcessLimit exceeded.
[IOCP] EXIT_PROCESS PID=1234
[ACCOUNT] TotalUserTime=1250000 PageFaultCount=342
```

#### 함정 & 주의사항

- Windows 8+ 자동 배정: `AssignProcessToJobObject`가 중복 배정 오류를 낼 수 있다 — `IsProcessInJob`으로 먼저 확인.
- Job 핸들을 닫아도 Job 오브젝트는 모든 프로세스 종료 시까지 살아있다.
- `GQCS`에서 `lpOverlapped == NULL`이면 Job 메시지가 아닌 IOCP 자체 오류다.

#### 확인 질문

1. `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`를 설정하면 어떤 동작이 보장되는가?
2. 중첩 Job에서 자식 Job의 메모리 제한이 부모 Job보다 느슨할 수 있는가?
3. IOCP 없이 Job 내 프로세스 종료를 감지하는 대안적 방법은?

#### 확장 과제

`JOB_OBJECT_LIMIT_JOB_MEMORY`로 Job 전체 가상 메모리 한도를 설정하고, `JOB_OBJECT_MSG_JOB_MEMORY_LIMIT` 알림을 수신하는 시나리오를 구현하세요.

---

### 04_NT_Process_Internals

**난이도** ★★★★ | **예상 시간** 5h | **선행 모듈** 01~03 + Process Hacker 설치

#### 이론 배경

##### EPROCESS — 커널 프로세스 오브젝트

`EPROCESS`는 NT 커널이 프로세스를 표현하는 핵심 구조체로, 커널 풀(Nonpaged Pool)에 할당된다. 사용자 모드에서 직접 접근할 수 없으며 `NtQueryInformationProcess` 등의 시스템 콜을 통해 일부 필드를 간접 조회한다.

주요 필드 (WinDbg `dt nt!_EPROCESS`로 확인):

| 필드 | 의미 |
|---|---|
| `Pcb` (KPROCESS) | 스케줄러가 사용하는 커널 레벨 프로세스 블록. CR3(DirectoryTableBase) 포함 |
| `UniqueProcessId` | PID (핸들 테이블 인덱스가 아닌 순수 ID) |
| `ActiveProcessLinks` | 시스템 전체 EPROCESS 이중 연결 리스트 |
| `Peb` | 사용자 모드 PEB 포인터 |
| `Token` | EX_FAST_REF — 보안 토큰(ACCESS_TOKEN) 포인터 |
| `VirtualSize` | 현재 가상 주소 공간 사용량 |
| `Vm.WorkingSetSize` | 현재 Working Set 크기 |
| `ObjectTable` | 핸들 테이블(HANDLE_TABLE) 포인터 |

##### PEB — 사용자 모드 프로세스 환경 블록

PEB는 `ntdll`이 사용하는 프로세스 전역 데이터 구조체로, 사용자 모드에서 직접 읽기 가능하다 (`NtCurrentPeb()` 또는 `__readgsqword(0x60)` on x64).

```
PEB (+0x000)
├── InheritedAddressSpace       BOOLEAN
├── ReadImageFileExecOptions    BOOLEAN
├── BeingDebugged               BOOLEAN  ← 안티디버그 체크 포인트
├── Mutant                      HANDLE   (보통 -1)
├── ImageBaseAddress            PVOID    ← 로드된 EXE 베이스
├── Ldr                        → PEB_LDR_DATA
│     ├── InLoadOrderModuleList  (로드 순서)
│     ├── InMemoryOrderModuleList
│     └── InInitializationOrderModuleList  ← DLL 초기화 순서
├── ProcessParameters          → RTL_USER_PROCESS_PARAMETERS
│     ├── CommandLine           UNICODE_STRING
│     ├── ImagePathName         UNICODE_STRING
│     ├── CurrentDirectory      CURDIR
│     └── Environment           PVOID
├── ProcessHeap                 PVOID    ← GetProcessHeap() 반환값
├── FastPebLock                 RTL_CRITICAL_SECTION
└── TlsBitmap / TlsExpansionBitmap  ← TLS 슬롯 사용 현황
```

##### TEB — 스레드 환경 블록

각 스레드마다 TEB가 존재하며 `NtCurrentTeb()` 또는 `__readgsqword(0x30)` (x64)로 접근한다.

```
TEB (+0x000)
├── NtTib.ExceptionList    SEH 체인 헤드 (x86 전용)
├── NtTib.StackBase        스택 베이스 (높은 주소)
├── NtTib.StackLimit       스택 커밋 한계 (낮은 주소)
├── NtTib.Self             TEB 자기 포인터
├── ProcessEnvironmentBlock → PEB
├── ClientId               {UniqueProcessId, UniqueThreadId}
├── LastErrorValue         GetLastError() 값 저장소
├── TlsSlots[64]           TLS 슬롯 배열 (TlsAlloc/TlsSetValue)
└── TlsExpansionSlots      추가 TLS 슬롯 포인터
```

##### 핸들 테이블 (HANDLE_TABLE)

핸들 테이블은 프로세스별 커널 오브젝트 참조 테이블이다. 핸들 값은 4의 배수 인덱스로, 3단계 B-트리 구조로 구현된다.

```
핸들 값 = (Table Index) × 4
HANDLE_TABLE_ENTRY = {
    ObjectPointer : ULONG_PTR  // 커널 오브젝트 포인터 (하위 3비트는 플래그)
    GrantedAccess : ULONG      // 허용된 접근 권한 마스크
}
```

`DuplicateHandle`은 소스 프로세스 핸들 테이블 엔트리를 대상 프로세스에 복사하고 동일 커널 오브젝트의 참조 카운트를 증가시킨다.

##### 액세스 토큰 (ACCESS_TOKEN)

토큰은 스레드/프로세스의 보안 컨텍스트를 정의하는 커널 오브젝트다.

```
TOKEN
├── AuthenticationId  (로그온 세션 LUID)
├── User SID          (계정 SID)
├── Groups[]          (그룹 SID 배열 + 속성)
├── Privileges[]      (SE_* 권한 배열 + 활성화 여부)
├── IntegrityLevel    (Untrusted=0, Low=1000, Medium=2000, High=3000, System=4000)
└── TokenType         (Primary | Impersonation)
```

`ImpersonationLevel`: `SecurityAnonymous → SecurityIdentification → SecurityImpersonation → SecurityDelegation` 순으로 권한이 커진다.

#### 핵심 개념

```
User Mode           Kernel Mode
─────────────────   ──────────────────────────────────
NtCurrentPeb()   →  EPROCESS.Peb
ReadProcessMemory→  EPROCESS.VirtualSize 등
OpenProcess()    →  EPROCESS 핸들 (참조 카운트 ++)
OpenThreadToken →   EPROCESS.Token (EX_FAST_REF)
CloseHandle()    →  핸들 테이블 엔트리 제거 + 오브젝트 참조 감소
```

#### 학습 목표

1. `NtQueryInformationProcess`로 PEB 주소를 구하고 `ReadProcessMemory`로 필드를 읽을 수 있다.
2. PEB Ldr 리스트를 순회해 로드된 DLL 목록을 나열할 수 있다 (EnumProcessModules 없이).
3. `OpenProcessToken` + `GetTokenInformation`으로 프로세스의 무결성 수준과 권한 목록을 출력할 수 있다.
4. `NtQueryInformationProcess(ProcessHandleCount)`로 핸들 수를 조회할 수 있다.

#### 핵심 API

`NtQueryInformationProcess` (ntdll) · `ReadProcessMemory`  
`OpenProcessToken` · `GetTokenInformation` · `LookupPrivilegeName`  
`GetCurrentProcess` · `IsWow64Process`

#### 실습 절차

1. `NtQueryInformationProcess(ProcessBasicInformation)` → `PEB*` 주소 획득 → `ReadProcessMemory`로 `PEB.ImageBaseAddress`, `PEB.ProcessHeap`, `PEB.BeingDebugged` 읽기.
2. `PEB.Ldr → InLoadOrderModuleList`를 `ReadProcessMemory`로 순회 → 각 `LDR_DATA_TABLE_ENTRY.BaseDllName` 출력 (EnumProcessModules와 비교).
3. `OpenProcessToken(hProcess, TOKEN_QUERY)` → `GetTokenInformation(TokenIntegrityLevel)` → 무결성 수준 문자열 출력.
4. `GetTokenInformation(TokenPrivileges)` → 전체 권한 목록을 `LookupPrivilegeName`으로 이름 변환하여 출력.
5. 타 프로세스(예: `notepad.exe`)를 대상으로 1~4를 반복하고 권한 차이 비교.

#### 관찰 포인트

- Process Hacker → 프로세스 속성 → Token 탭의 무결성 수준이 내 출력과 일치하는지 확인.
- Process Hacker → 프로세스 속성 → Modules 탭의 DLL 목록이 Ldr 순회 결과와 일치하는지 확인.
- WinDbg: `dt nt!_EPROCESS` → `dt nt!_PEB @$peb` → `dt nt!_LDR_DATA_TABLE_ENTRY` 체인으로 커널 뷰 대조.

#### 예상 출력 예시

```
[PEB] Address:       0x000000A1`34000000
[PEB] ImageBase:     0x00007FF7`12340000
[PEB] ProcessHeap:   0x000001A0`00000000
[PEB] BeingDebugged: 0 (not debugged)

[LDR] ntdll.dll         @ 0x00007FFF`E0000000
[LDR] KERNEL32.DLL      @ 0x00007FFF`D8000000
[LDR] KERNELBASE.dll    @ 0x00007FFF`D4000000

[TOKEN] IntegrityLevel:  Medium (0x2000)
[PRIV]  SeShutdownPrivilege       — Disabled
[PRIV]  SeDebugPrivilege          — Disabled
[PRIV]  SeChangeNotifyPrivilege   — Enabled
```

#### 함정 & 주의사항

- `NtQueryInformationProcess`는 `ntdll.lib`에서 직접 링크하거나 `GetProcAddress`로 가져와야 한다 — 공개 헤더에 선언이 없다.
- `ReadProcessMemory`로 타 프로세스 PEB를 읽으려면 `PROCESS_VM_READ` 권한이 필요하다. 시스템 프로세스는 거부된다.
- x64 프로세스에서 `__readgsqword(0x60)`은 PEB, `__readgsqword(0x30)`은 TEB를 가리킨다. x86에서는 FS 세그먼트 레지스터를 사용한다.
- `EX_FAST_REF`: `EPROCESS.Token`의 하위 4비트는 참조 카운트 힌트로 사용된다 — 실제 포인터를 얻으려면 하위 비트를 마스킹해야 한다 (`& ~0xF`).

#### 확인 질문

1. `PEB.BeingDebugged` 필드가 안티디버깅 기법에서 자주 체크되는 이유는? 우회 방법은?
2. `DuplicateHandle`로 복사한 핸들은 원본 핸들과 동일한 커널 오브젝트를 가리키는가? 두 핸들을 독립적으로 닫을 수 있는가?
3. Medium(0x2000) 무결성 프로세스가 High(0x3000) 무결성 프로세스의 핸들을 `OpenProcess(PROCESS_ALL_ACCESS)`로 열려고 하면 어떻게 되는가?
4. 64비트 Windows에서 32비트(WOW64) 프로세스가 가진 PEB는 몇 개인가? 각각 어떤 역할인가?

#### 확장 과제

`NtQueryInformationProcess(ProcessHandleInformation)`으로 프로세스의 전체 핸들 목록(타입·권한·오브젝트 주소 포함)을 출력하는 도구를 구현하세요. Process Explorer의 하단 핸들 탭과 비교합니다.

---

## Phase 2: 스레드 & 스케줄러

---

### 05_Thread_Basics

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** Phase 1 완료

#### 이론 배경

스레드는 OS 스케줄러가 CPU에 배정하는 기본 단위다. 스레드 컨텍스트는 **레지스터 집합 + 스택 포인터 + TEB**로 구성된다. 컨텍스트 전환 시 커널은 현재 스레드의 레지스터를 KTHREAD의 KernelStack에 저장하고 대상 스레드의 상태를 복원한다.

**우선순위 계산**: 최종 기본 우선순위 = 프로세스 우선순위 클래스 + 스레드 상대 우선순위

| 프로세스 클래스 | 기본값 |
|---|---|
| IDLE_PRIORITY_CLASS | 4 |
| BELOW_NORMAL_PRIORITY_CLASS | 6 |
| NORMAL_PRIORITY_CLASS | 8 |
| ABOVE_NORMAL_PRIORITY_CLASS | 10 |
| HIGH_PRIORITY_CLASS | 13 |
| REALTIME_PRIORITY_CLASS | 24 |

스레드 상대 우선순위 (`THREAD_PRIORITY_*`)는 −2 ~ +2 (±1 등) 오프셋을 더한다. 결과 범위는 1~31 (31은 REALTIME 전용).

**CPU 친화성**: `SetThreadAffinityMask`는 스레드가 실행될 논리 프로세서를 비트마스크로 제한한다. 잘못된 설정은 실제로 성능을 낮출 수 있다 (NUMA 원격 접근 증가).

#### 학습 목표

1. `CreateThread`로 스레드를 생성하고 올바르게 종료 대기·핸들 해제할 수 있다.
2. 우선순위를 변경하고 높은 우선순위 스레드가 CPU를 더 많이 점유함을 측정할 수 있다.
3. CPU 친화성 마스크로 스레드를 특정 코어에 고정할 수 있다.

#### 핵심 API

`CreateThread` · `SetThreadPriority` · `GetThreadPriority`  
`SetThreadAffinityMask` · `SuspendThread` · `ResumeThread`  
`GetCurrentProcessorNumber` · `TlsAlloc` · `TlsSetValue` · `TlsGetValue`

#### 실습 절차

1. 카운터 증가 스레드 2개: `THREAD_PRIORITY_HIGHEST` vs `THREAD_PRIORITY_LOWEST` → 1초 후 카운터 비율 출력.
2. `SetThreadAffinityMask(hThread, 1 << 0)` → `GetCurrentProcessorNumber()`로 코어 고정 확인.
3. `SuspendThread`/`ResumeThread`로 정지·재개, 정지 중 카운터 멈춤 확인.
4. `TlsAlloc`으로 TLS 슬롯 할당 → 스레드마다 독립 카운터 유지 → 메인에서 합산.

#### 관찰 포인트

- Process Explorer 스레드 탭: 기본 우선순위 vs 현재 우선순위 비교.
- 코어 수가 많을수록 우선순위 차이 효과가 줄어드는지 확인 (코어가 충분하면 둘 다 실행됨).
- `SuspendThread` 재귀 호출 시 `SuspendCount`가 누적되어 같은 횟수 `ResumeThread`가 필요함을 확인.

#### 예상 출력 예시

```
HIGH counter : 48,293,710
LOW  counter :    312,884   ratio ≈ 154:1
Thread core  : 0 (affinity pinned to Core 0)
TLS total    : 40,000,000 (4 threads × 10M each)
```

#### 함정 & 주의사항

- `TerminateThread`는 스택 언와인드 없이 종료 → RAII 소멸자 미실행. 정상 종료 플래그를 사용하라.
- `SuspendThread`를 임의 시점에 호출하면 힙 락 보유 채 정지 가능 → 데드락.
- 우선순위 역전(Priority Inversion): 낮은 우선순위 스레드가 높은 우선순위 스레드에게 필요한 뮤텍스를 보유하면 시스템 전체 교착.

#### 확인 질문

1. 단일 코어에서 `HIGHEST`와 `LOWEST` 스레드를 동시에 실행하면 `LOWEST`는 절대 실행되지 않는가?
2. `GetCurrentProcessorNumber()`가 같은 스레드에서 연속 호출해도 다른 값을 반환할 수 있는 이유는?
3. TLS(`TlsAlloc`)와 `__declspec(thread)` 정적 TLS의 내부 구현 차이는?

#### 확장 과제

`SetThreadDescription`(Win10 1607+)으로 스레드에 이름을 붙이고 WinDbg `~` 명령으로 이름이 표시되는지 확인하세요.

---

### 06_ThreadPool

**난이도** ★★☆ | **예상 시간** 4h | **선행 모듈** 05

#### 이론 배경

Windows 스레드 풀은 프로세스당 하나의 **기본 풀**(Default Pool)을 제공한다. `QueueUserWorkItem`은 이 기본 풀을 사용한다. `CreateThreadpool`로 전용 풀을 만들면 `TP_CALLBACK_ENVIRON`으로 Work·Timer·IO 콜백을 해당 풀에 바인딩할 수 있다.

**Cleanup Group**: `CreateThreadpoolCleanupGroup` + `CloseThreadpoolCleanupGroupMembers`로 그룹 내 모든 콜백 완료를 한 번에 대기한다.

**타이머 정밀도**: `SetThreadpoolTimerEx`는 Windows 10의 고정밀도 타이머를 지원하며 최소 `timeBeginPeriod(1)`를 호출하지 않아도 1ms 이하 정밀도가 가능하다.

#### 학습 목표

1. 전용 풀 + Cleanup Group으로 여러 작업의 완료를 일괄 대기할 수 있다.
2. `TP_TIMER`로 주기적 콜백을 구현하고 실제 간격 정확도를 측정할 수 있다.
3. `TP_IO`를 파이프에 연결해 비동기 I/O 완료를 콜백으로 처리할 수 있다.

#### 핵심 API

`CreateThreadpool` · `SetThreadpoolThreadMinimum/Maximum`  
`CreateThreadpoolWork` · `SubmitThreadpoolWork` · `WaitForThreadpoolWorkCallbacks`  
`CreateThreadpoolTimer` · `SetThreadpoolTimer`  
`CreateThreadpoolCleanupGroup` · `CloseThreadpoolCleanupGroupMembers`

#### 실습 절차

1. 기본 풀 `QueueUserWorkItem` 100회 → 완료를 `CONDITION_VARIABLE`로 수집.
2. 전용 풀(2~8 스레드) + Cleanup Group → 100 작업 제출 → `CloseThreadpoolCleanupGroupMembers` 대기.
3. `CreateThreadpoolTimer` 500ms 주기 → `QueryPerformanceCounter`로 실제 간격 측정.
4. `WaitForThreadpoolWorkCallbacks(work, TRUE)`: fCancelPending이 "대기 중인 것만" 취소함을 실험.

#### 관찰 포인트

- 전용 풀 스레드 수를 1·2·4·8로 바꾸면서 100 작업 완료 시간 변화 측정.
- 타이머 콜백 지연이 부하 증가 시 어떻게 변하는지 확인.

#### 예상 출력 예시

```
[DefaultPool] 100 items: 45ms
[CustomPool 2t] 100 items: 38ms
[CustomPool 8t] 100 items: 19ms
[Timer] 500.12ms, 1000.08ms, 1500.19ms
```

#### 함정 & 주의사항

- `WaitForThreadpoolWorkCallbacks(work, TRUE)`: 실행 **중인** 콜백은 취소되지 않는다.
- 풀 소멸 순서: `CloseThreadpoolCleanupGroupMembers` → `CloseThreadpoolCleanupGroup` → `DestroyThreadpoolEnvironment` → `CloseThreadpool`.
- `SetThreadpoolTimer(timer, NULL, 0, 0)` = 타이머 취소. `NULL` 전달이 해제가 아님에 주의.

#### 확인 질문

1. 기본 풀 스레드가 무한 루프 작업을 실행 중이면 나머지 항목은 어떻게 처리되는가?
2. Cleanup Group 없이 동적으로 생성된 여러 `TP_WORK`의 완료를 안전하게 대기하는 패턴은?
3. `SetThreadpoolCallbackRunsLong`을 설정하지 않은 장기 실행 콜백이 풀에 미치는 영향은?

#### 확장 과제

`CreateThreadpoolIo`와 익명 파이프를 연결해 비동기 읽기 완료 시 스레드 풀 콜백이 호출되는 패턴을 구현하세요.

---

### 07_Fiber

**난이도** ★★★ | **예상 시간** 4h | **선행 모듈** 05

#### 이론 배경

Fiber는 OS 스케줄러가 아닌 **사용자 코드**가 전환 시점을 결정하는 경량 실행 단위다. 동일 스레드 내에서만 전환되므로 **협력적 멀티태스킹**을 구현한다.

`SwitchToFiber`는 현재 Fiber의 레지스터와 스택 포인터를 저장하고 대상 Fiber의 컨텍스트를 복원한다. 내부적으로 `setjmp`/`longjmp`와 유사한 메커니즘으로 동작한다.

Fiber Local Storage(FLS)는 TLS와 달리 Fiber 전환 시 자동으로 교체되고, Fiber 종료 시 `FlsAlloc`에 등록한 소멸자가 호출된다.

#### 학습 목표

1. 스레드를 Fiber로 변환하고 여러 Fiber를 수동 전환할 수 있다.
2. Fiber 기반 라운드-로빈 스케줄러를 구현할 수 있다.
3. Fiber로 Generator(yield) 패턴을 구현할 수 있다.

#### 핵심 API

`ConvertThreadToFiber` · `CreateFiber` · `SwitchToFiber` · `DeleteFiber`  
`GetCurrentFiber` · `FlsAlloc` · `FlsSetValue` · `FlsGetValue`

#### 실습 절차

1. 메인을 Fiber로 변환 → A, B, C Fiber 생성 → A→B→C→메인 순으로 수동 전환.
2. Fiber 배열 + 라운드-로빈 인덱스로 스케줄러 구현 → 각 Fiber가 10회 작업 후 양보.
3. Generator: `produce(value)` 호출 시 메인으로 값 전달 후 일시 중단 → Fibonacci 수열 생성.
4. `DeleteFiber`를 현재 실행 중인 Fiber 자신에 호출 시 동작 확인.

#### 관찰 포인트

- 전환 중 `GetCurrentThreadId()`는 항상 동일한 값을 반환하는지 확인.
- 스택 크기 기본값(0 → 64KB) 대비 재귀 깊이 제한 실험.

#### 예상 출력 예시

```
[A] step 1  [B] step 1  [C] step 1
[A] step 2  [B] step 2  ...
[GEN] 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
```

#### 함정 & 주의사항

- `ConvertThreadToFiber` 없이 `SwitchToFiber` 호출 → 크래시.
- 종료된 Fiber 핸들로 `SwitchToFiber` 호출 → 접근 위반. Fiber 상태를 직접 관리해야 한다.
- Fiber는 멀티스레드와 혼용 금지: 같은 Fiber를 두 스레드가 동시 실행 → 정의되지 않은 동작.

#### 확인 질문

1. Fiber 전환과 스레드 컨텍스트 전환의 비용 차이는 어디서 오는가?
2. `SwitchToFiber`가 반환되는 시점은 언제인가?
3. C++20 코루틴(`co_yield`)과 Fiber 기반 코루틴의 핵심 구현 차이는?

#### 확장 과제

Fiber 스케줄러에 **우선순위 큐**를 추가해 높은 우선순위 Fiber가 먼저 재개되도록 구현하세요.

---

### 08_Scheduler_Internals

**난이도** ★★★★ | **예상 시간** 5h | **선행 모듈** 05

#### 이론 배경

##### KTHREAD 상태 머신

NT 스케줄러는 소프트웨어 우선순위 기반 선점형 스케줄러다. 각 스레드는 `KTHREAD` 구조체(커널 풀)로 표현되며 다음 상태를 순환한다.

```
               Preempted
         ┌────────────────┐
         │                ▼
[Initialized] → [Ready] → [Standby] → [Running]
                  ▲                      │  │
                  │                      │  │
              [Waiting] ←────────────────┘  │
                  │    wait satisfied        │
                  └──────────────────────────┘ (wake)
                                             [Terminated]
```

- **Ready**: 실행 가능하나 CPU 없음 — 우선순위별 Ready Queue에 대기.
- **Standby**: 특정 CPU에 배정됨 — 다음 디스패치에서 Running으로 전환.
- **Running**: 현재 CPU를 점유 중.
- **Waiting**: 커널 오브젝트(이벤트·뮤텍스 등) 대기 또는 `Sleep` 호출 중.
- **Transition**: 커널 스택이 페이지 아웃된 Waiting 상태.

##### 우선순위 부스트 메커니즘

스케줄러는 특정 조건에서 스레드의 **현재(Dynamic) 우선순위**를 일시적으로 높인다:

| 이벤트 | 부스트 크기 | 소멸 방식 |
|---|---|---|
| I/O 완료 (디스크) | +1 | 스케줄 시마다 −1 감소 |
| I/O 완료 (직렬 포트) | +2 | 스케줄 시마다 −1 감소 |
| I/O 완료 (키보드) | +6 | 스케줄 시마다 −1 감소 |
| I/O 완료 (비디오) | +2 | 스케줄 시마다 −1 감소 |
| 포그라운드 프로세스 | 퀀텀 1.5~3× 증가 | 백그라운드 전환 시 소멸 |
| 기아 방지 (Starvation) | 우선순위 15로 즉시 상승 | 2 퀀텀 후 기본 우선순위로 복귀 |
| 뮤텍스 대기 후 깨어남 | +1 (Boost flag) | 즉시 소멸 |

기본 우선순위(Base) 아래로는 Dynamic 우선순위가 내려가지 않는다. 단, `SetThreadPriorityBoost(TRUE)`로 모든 동적 부스트를 비활성화할 수 있다.

##### 스레드 퀀텀

퀀텀(Quantum)은 스레드가 선점 없이 실행되는 최대 시간이다.

- **타이머 인터럽트**: 기본 15.6ms (64Hz) — `timeBeginPeriod(1)`로 1ms로 낮출 수 있으나 CPU 부하 증가.
- 워크스테이션: 기본 퀀텀 2 tick (약 31ms), 포그라운드 3× 적용 시 최대 6 tick.
- 서버: 고정 퀀텀 12 tick (약 187ms) — `HKLM\SYSTEM\CurrentControlSet\Control\PriorityControl\Win32PrioritySeparation` 레지스트리로 조정.

퀀텀 만료 시 같은 우선순위의 다른 Ready 스레드가 있으면 선점된다. 없으면 퀀텀을 다시 받아 계속 실행된다.

##### IRQL (Interrupt Request Level)

IRQL은 현재 프로세서가 처리할 수 있는 인터럽트 수준을 정의한다:

| IRQL | 값 | 컨텍스트 |
|---|---|---|
| PASSIVE_LEVEL | 0 | 일반 사용자/커널 코드 |
| APC_LEVEL | 1 | APC 처리 (커널·사용자 모드) |
| DISPATCH_LEVEL | 2 | 스케줄러, DPC 루틴 — **페이지 메모리 접근 금지** |
| DEVICE_IRQL | 3~11 | 디바이스 인터럽트 서비스 루틴 |
| HIGH_LEVEL | 15 | NMI, 머신 체크 |

DISPATCH_LEVEL 이상에서는 스케줄러가 실행되지 않으므로 **대기 함수 호출 불가**, **페이지 가능 메모리 접근 불가**.

DPC(Deferred Procedure Call)는 ISR이 처리를 지연하기 위해 DISPATCH_LEVEL 큐에 등록하는 루틴이다. 네트워크 수신·타이머 만료가 DPC로 처리된다.

#### 핵심 개념 — 퀀텀 및 부스트 타임라인

```
Priority
  31 │
  24 │ REALTIME
  16 │
  15 │──── Starvation boost plateau (2 quanta)
   8 │──── Base priority (NORMAL class, NORMAL thread)
   7 │   ↑ Dynamic priority after I/O boost (+1)
   6 │
   1 │
   0 │ (idle thread only)

Time:  [Q1: base=8]──[Q2: boost→9 after disk I/O]──[Q3: 8 (−1 decay)]──[Q4: 8]
```

#### 학습 목표

1. KTHREAD 상태 전환을 코드 실험으로 관찰하고 WinDbg `!thread` 출력과 연결할 수 있다.
2. 스레드 퀀텀 길이를 `QueryPerformanceCounter`로 간접 측정할 수 있다.
3. I/O 완료 후 우선순위 부스트가 실제로 측정 가능한지 실험할 수 있다.
4. `SetThreadPriorityBoost`로 동적 부스트를 비활성화하고 성능 차이를 측정할 수 있다.

#### 핵심 API

`SetThreadPriorityBoost` · `GetThreadPriorityBoost`  
`SetProcessPriorityClass` · `GetProcessPriorityClass`  
`NtQuerySystemInformation(SystemProcessorPerformanceInformation)` (ntdll)  
`QueryPerformanceCounter` · `QueryPerformanceFrequency`

#### 실습 절차

1. 루프에서 `QueryPerformanceCounter`로 루프 반복 시간을 기록 → 퀀텀 만료 시 비정상적으로 큰 간격이 발생하는 순간 탐지 → 퀀텀 길이 추정.
2. 파일 I/O 완료 직후 스레드 우선순위를 반복 샘플링 (`GetThreadPriority`) → 부스트 후 감소 패턴 관찰.
3. `SetThreadPriorityBoost(FALSE)` (부스트 비활성) 상태에서 I/O 완료 후 동일 측정 → 부스트 없는 플랫 우선순위 확인.
4. 고부하 스레드 하나로 다른 스레드를 기아 상태에 빠뜨린 뒤 약 4초 후 기아 방지 부스트가 적용되는지 확인.

#### 관찰 포인트

- WinDbg: `!thread <addr> 1f` → KTHREAD 필드 중 `BasePriority`, `Priority`, `State` 확인.
- Process Explorer: 스레드 탭에서 **Base/Current Priority** 두 컬럼 비교.
- `Win32PrioritySeparation` 레지스트리 값을 0x24(포그라운드 3× 퀀텀)에서 0x18(균등 퀀텀)으로 변경 후 포그라운드 프로세스의 응답성 변화 비교.

#### 예상 출력 예시

```
[QUANTUM] Detected scheduling gap at iteration 8,472,391: 15.8ms  ← quantum expiry
[QUANTUM] Average gap: 0.0003ms  (normal iteration)
[BOOST]   Before I/O: priority=8   After I/O: priority=9  After 1 sched: priority=8
[BOOST DISABLED] Before I/O: priority=8   After I/O: priority=8  (no change)
[STARVATION] Low-prio thread blocked 4.2s → boosted to 15 → 2 quanta → back to 8
```

#### 함정 & 주의사항

- 퀀텀 측정은 단일 코어 환경에서 더 명확하다. 멀티코어에서는 다른 코어로 이동해 측정 노이즈가 커진다.
- `SetThreadPriorityBoost(TRUE)`는 부스트를 **비활성화**한다 (TRUE = Disable). 인자 의미가 직관과 반대다.
- REALTIME 우선순위 클래스는 마우스·키보드 처리 스레드보다 높을 수 있어 시스템 반응성이 크게 저하될 수 있다.

#### 확인 질문

1. 우선순위 15 스레드에게 I/O 부스트가 적용되어 16이 될 수 있는가? (힌트: 16은 REALTIME 영역 경계)
2. DISPATCH_LEVEL에서 `WaitForSingleObject`를 호출하면 왜 시스템이 멈추거나 크래시가 발생하는가?
3. 서버 Windows에서 퀀텀이 워크스테이션보다 긴 이유는 무엇이며, 이것이 처리량(throughput)에 어떤 영향을 미치는가?
4. DPC 루틴이 너무 오래 실행되면 어떤 증상이 발생하는가? (힌트: DPC Watchdog)

#### 확장 과제

`timeBeginPeriod(1)`로 타이머 해상도를 1ms로 낮춘 뒤 동일한 퀀텀 측정을 반복하고, 기본 해상도(15.6ms)와 비교하세요. 전력 소비 차이를 `powercfg /energy`로 측정합니다.

---

### 09_NUMA_ProcessorGroups

**난이도** ★★★ | **예상 시간** 4h | **선행 모듈** 08

#### 이론 배경

##### NUMA 토폴로지

NUMA(Non-Uniform Memory Access)는 멀티소켓 시스템에서 각 CPU 소켓이 **로컬 메모리 컨트롤러**를 갖는 구조다. 원격 노드 메모리 접근은 로컬보다 2~4배 느리다.

```
Socket 0 (Node 0)          Socket 1 (Node 1)
┌────────────────┐          ┌────────────────┐
│  Core0  Core1  │          │  Core2  Core3  │
│    L1/L2/L3    │          │    L1/L2/L3    │
│  Memory: 16GB  │◄──QPI───►│  Memory: 16GB  │
└────────────────┘  (slow)  └────────────────┘
  Local: ~50ns               Remote: ~100ns
```

단일 소켓 시스템에서도 NUMA API는 지원되며 노드 수는 1이다. `GetNumaHighestNodeNumber`로 최대 노드 번호를 확인한다.

##### 프로세서 그룹 (Processor Groups)

Windows는 단일 **프로세서 그룹**에 최대 64개의 논리 프로세서를 지원한다. 128개 이상의 논리 프로세서는 여러 그룹으로 나뉘며, `SetThreadGroupAffinity`로 특정 그룹에 스레드를 배정해야 한다.

기본적으로 새 스레드는 **그룹 0**에 배정된다. 그룹 1 이상의 CPU를 활용하려면 명시적 `SetThreadGroupAffinity`가 필요하다.

##### NUMA 인식 메모리 할당

`VirtualAllocExNuma`로 특정 NUMA 노드의 물리 메모리에 우선 할당한다. 노드에 여유 메모리가 없으면 원격 노드로 폴백된다.

```cpp
// NUMA 노드 0에 우선 할당
VirtualAllocExNuma(hProcess, NULL, size,
    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE,
    0  // NUMA node 0
);
```

스레드 풀의 NUMA 인식: `SetThreadpoolCallbackRunsLong` + `SetThreadpoolCallbackPool`로 특정 노드의 코어에서만 실행되는 풀을 구성할 수 있다.

##### 로컬 vs 원격 메모리 접근 지연 측정

NUMA 효과를 측정하려면:
1. 스레드를 Node 0 코어에 고정 (`SetThreadGroupAffinity`)
2. Node 0 메모리와 Node 1 메모리에 각각 대규모 배열 할당
3. 동일 접근 패턴으로 반복 읽기 → `QueryPerformanceCounter`로 시간 비교

#### 학습 목표

1. 시스템의 NUMA 토폴로지를 쿼리하고 노드별 프로세서 마스크를 출력할 수 있다.
2. `VirtualAllocExNuma`로 NUMA 로컬 메모리를 할당하고 원격 접근 대비 지연을 측정할 수 있다.
3. 64개 이상 논리 프로세서를 가진 시스템에서 프로세서 그룹을 인식하는 코드를 작성할 수 있다.

#### 핵심 API

`GetNumaHighestNodeNumber` · `GetNumaNodeProcessorMaskEx`  
`GetNumaProcessorNodeEx` · `VirtualAllocExNuma`  
`SetThreadGroupAffinity` · `GetThreadGroupAffinity`  
`GetActiveProcessorGroupCount` · `GetActiveProcessorCount`

#### 실습 절차

1. `GetNumaHighestNodeNumber` → 노드 수 출력 → 각 노드의 `GetNumaNodeProcessorMaskEx`로 프로세서 마스크 출력.
2. `GetActiveProcessorGroupCount` + `GetActiveProcessorCount`로 전체 논리 프로세서 수 출력.
3. 스레드를 Node 0 Core 0에 고정 → Node 0 메모리(4GB) 순차 읽기 → 시간 측정.
4. 동일 스레드로 Node 1 메모리(4GB) 순차 읽기 → 시간 비교 (단일 소켓 환경에서는 차이 없음을 확인).
5. NUMA 인식 메모리 할당자: `VirtualAllocExNuma` 래퍼 클래스 구현 → 노드 간 차이 측정.

#### 관찰 포인트

- 단일 소켓 환경에서는 `GetNumaHighestNodeNumber`가 0을 반환 — 대신 두 코어에 각각 `SetThreadGroupAffinity`해서 L3 캐시 경계 효과를 측정.
- 대형 배열 접근 시 `_mm_prefetch` 힌트가 NUMA 원격 접근 페널티를 얼마나 완화하는지 측정.

#### 예상 출력 예시

```
NUMA Nodes: 2
  Node 0: Processors 0-15 (mask=0x000000000000FFFF)
  Node 1: Processors 16-31 (mask=0x00000000FFFF0000)
Processor Groups: 1 (total 32 logical processors)

[LATENCY] Node 0 → Node 0 memory read 4GB: 1,240ms
[LATENCY] Node 0 → Node 1 memory read 4GB: 2,891ms  (2.33× slower)
```

#### 함정 & 주의사항

- `GetNumaNodeProcessorMask` (구버전)는 그룹 0만 반환한다. 64개 이상 CPU 시스템에서는 `GetNumaNodeProcessorMaskEx`를 사용해야 한다.
- `VirtualAllocExNuma`는 `dwNUMANode = NUMA_NO_PREFERRED_NODE`(0xFFFFFFFF)를 지정하면 NUMA 힌트 없는 일반 할당과 동일하다.
- NUMA 측정은 소켓 2개 이상 물리 서버에서만 명확한 차이가 나온다 — 단일 소켓에서는 개념 이해 중심으로 진행.

#### 확인 질문

1. 스레드가 Node 0에 고정되어 있지만 Node 1 메모리를 접근할 때 어떤 하드웨어 경로로 데이터가 전달되는가?
2. `GetSystemInfo`가 반환하는 `dwNumberOfProcessors`가 64개를 넘을 수 없는 이유는?
3. NUMA-aware 스레드 풀을 구성할 때 각 노드에 별도 풀을 만드는 것과 단일 풀에서 친화성만 설정하는 것의 차이는?

#### 확장 과제

NUMA 노드 수에 따라 자동으로 스레드 풀을 분할하는 `NumaAwareThreadPool` 클래스를 구현하세요. 각 노드 전용 풀의 스레드는 해당 노드 코어에 고정되고, 작업 제출 시 NUMA-local 메모리 버퍼를 사용합니다.

---

# Phase 3~4 (임시) — LEARNING_GUIDE.md에 병합 예정

## Phase 3: 메모리 관리

---

### 10_VirtualMemory

**난이도** ★★★ | **예상 시간** 4h | **선행 모듈** Phase 1 완료

#### 이론 배경

##### 가상 주소 공간 레이아웃 (x64)

x64 Windows 프로세스는 **128TB** 사용자 모드 가상 주소 공간을 가진다 (48비트 가상 주소 중 하위 절반).

```
0x00000000`00000000  ┬─ 사용자 모드 가상 주소 공간 (128TB)
                     │  스택, 힙, 모듈, 매핑 영역
0x00007FFF`FFFFFFFF  ┴
0x00008000`00000000  ─ 비정규(Non-canonical) 주소 홀 (64KB)
0xFFFF8000`00000000  ┬─ 커널 모드 가상 주소 공간 (128TB)
                     │  시스템 PTE, 페이지 풀, 드라이버
0xFFFFFFFF`FFFFFFFF  ┴
```

##### 페이지 상태 3단계

```
[Free] ──Reserve──► [Reserved] ──Commit──► [Committed]
                                                │
                              Decommit ◄────────┘
                     Release  ◄── Reserved ──┘
```

- **Free**: 가상 주소 범위에 매핑 없음.
- **Reserved**: VA 범위 예약, 물리 메모리·페이지 파일 미할당. 접근 시 `EXCEPTION_ACCESS_VIOLATION`.
- **Committed**: 페이지 파일 공간 확보. 첫 접근 시 **페이지 폴트** → 물리 페이지 할당.

##### 페이지 보호 플래그

| 플래그 | 실행 | 쓰기 | 읽기 |
|---|---|---|---|
| `PAGE_NOACCESS` | ✗ | ✗ | ✗ |
| `PAGE_READONLY` | ✗ | ✗ | ✓ |
| `PAGE_READWRITE` | ✗ | ✓ | ✓ |
| `PAGE_EXECUTE_READ` | ✓ | ✗ | ✓ |
| `PAGE_EXECUTE_READWRITE` | ✓ | ✓ | ✓ |
| `PAGE_GUARD` | — | — | 첫 접근 시 예외 |
| `PAGE_NOCACHE` | — | — | 캐시 비활성 |

`PAGE_GUARD`는 **스택 성장 메커니즘**에 사용된다: 스택 확장 영역의 맨 앞 페이지에 설정, 첫 접근 시 `EXCEPTION_GUARD_PAGE` 발생 → 커널이 페이지를 커밋하고 플래그 제거 → 스택이 점진적으로 커진다.

##### Working Set vs Virtual Size

- **Virtual Size**: 전체 예약+커밋 VA 크기. 실제 메모리와 무관.
- **Committed Size**: 페이지 파일에 공간이 예약된 크기.
- **Working Set**: 현재 물리 메모리에 상주하는 페이지 집합. `QueryWorkingSet`으로 조회.

#### 학습 목표

1. `VirtualAlloc` Reserve → Commit → Decommit → Release 전체 흐름을 구현하고 VMMap으로 시각화할 수 있다.
2. `VirtualQuery`로 임의 주소의 페이지 상태(State, Protect, Type)를 출력할 수 있다.
3. 가드 페이지를 설정하고 SEH로 `EXCEPTION_GUARD_PAGE`를 처리할 수 있다.
4. 지연 커밋 배열을 SEH 기반으로 구현할 수 있다.

#### 핵심 API

`VirtualAlloc` · `VirtualFree` · `VirtualProtect` · `VirtualQuery`  
`GetSystemInfo` · `QueryWorkingSet` · `__try` / `__except`

#### 실습 절차

1. `GetSystemInfo`로 페이지 크기 출력 → `MEM_RESERVE`만으로 10페이지 예약 → `VirtualQuery`로 상태 확인.
2. `MEM_COMMIT`으로 3페이지 커밋 → VMMap에서 Reserved→Committed 전환 캡처.
3. `VirtualProtect`로 첫 페이지를 `PAGE_READONLY` → 쓰기 시도 SEH 캐치.
4. 마지막 커밋 페이지를 `PAGE_GUARD` → 첫 접근 `EXCEPTION_GUARD_PAGE` 처리 → 두 번째 접근 예외 없음 확인.
5. SEH 기반 지연 커밋 배열: `MEM_RESERVE`만 한 뒤 `EXCEPTION_ACCESS_VIOLATION` 핸들러에서 해당 페이지를 커밋.

#### 관찰 포인트

- VMMap: Reserved(황색) → Committed(녹색) 전환. Commit 직후 Working Set은 아직 증가 안 함 — 첫 쓰기 후 증가.
- `MEMORY_BASIC_INFORMATION.RegionSize`가 페이지 크기의 배수임을 확인.
- `VirtualAlloc`의 반환 주소가 `dwAllocationGranularity`(보통 64KB)의 배수임을 확인.

#### 예상 출력 예시

```
Page size: 4096  Allocation granularity: 65536
Reserved  @ 0x200`00000000  size=40960  State=MEM_RESERVE
Committed @ 0x200`00000000  size=12288  State=MEM_COMMIT  Protect=PAGE_READWRITE
[SEH] EXCEPTION_ACCESS_VIOLATION on PAGE_READONLY  (write blocked, expected)
[SEH] EXCEPTION_GUARD_PAGE triggered — page committed, flag cleared
Second access: no exception
[Lazy] Access at offset 8MB → commit triggered, value stored: 42
```

#### 함정 & 주의사항

- `VirtualFree`에서 `MEM_DECOMMIT`은 `dwSize` 지정 필요, `MEM_RELEASE`는 `dwSize = 0`만 허용. 혼동하면 오류.
- `MEM_RESERVE | MEM_COMMIT` 동시 지정 시 이후 범위 확장이 불가 (Reserve 주소 추적 필요).
- 페이지 크기를 하드코딩하지 말 것 — `GetSystemInfo`로 항상 확인.

#### 확인 질문

1. Commit된 페이지가 물리 메모리를 즉시 소비하지 않는 이유는? (Demand paging 관점에서)
2. `PAGE_GUARD` 플래그는 한 번 트리거되면 제거된다 — 이것이 스택 성장에 어떻게 활용되는가?
3. 프로세스 A가 1GB를 Reserve했을 때 다른 프로세스의 메모리 가용량에 영향을 주는가? Commit했을 때는?

#### 확장 과제

대용량 배열을 Reserve한 뒤 접근 시에만 페이지를 Commit하는 `LazyArray<T>` 템플릿 클래스를 구현하세요. `operator[]` 접근 시 SEH 폴트 핸들러가 해당 페이지를 커밋합니다.

---

### 11_HeapManagement

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 10

#### 이론 배경

##### NT 힙 구조

기본 프로세스 힙(`GetProcessHeap()`)은 `ntdll`이 관리하는 `NT_HEAP` 구조체로, 내부적으로 `VirtualAlloc`으로 세그먼트를 확장한다.

```
NT Heap
├── Segment 0  (초기 ~ 1MB)
│     ├── HEAP_ENTRY: busy block (size, flags)
│     ├── HEAP_ENTRY: free block → free list
│     └── ...
├── Segment 1  (확장 시 추가)
│     └── ...
└── FreeList[128]  (크기별 여유 블록 리스트)
```

##### LFH (Low-Fragmentation Heap)

Windows Vista 이후, 동일 크기 할당이 **17회** 이상 발생하면 해당 크기 버킷에 LFH가 자동 활성화된다.

- LFH는 고정 크기 **SubSegment** 단위로 관리 → 단편화 없음.
- LFH 적용 크기 범위: 1~16,368 바이트 (이상은 일반 힙).
- `HeapQueryInformation(HeapCompatibilityInformation)`으로 LFH 활성화 여부 확인.

##### Segment Heap (Windows 10 1507+)

일부 프로세스(현대 앱)는 `SEGMENT_HEAP`을 사용한다:
- Backend(대용량), VS(Variable Size), LFH, Large block 4가지 할당자를 상황에 따라 선택.
- 단편화가 더 낮고 보안 기능(guard page, randomization)이 강화됨.
- `HeapCreate`로 만든 사용자 힙은 여전히 NT Heap을 사용한다.

#### 학습 목표

1. 전용 힙을 생성하고 `HeapAlloc`/`HeapFree`로 관리할 수 있다.
2. `HeapWalk`로 힙 블록 레이아웃을 출력하고 단편화 패턴을 관찰할 수 있다.
3. `HEAP_NO_SERIALIZE` 힙에 멀티스레드가 접근 시 손상이 발생함을 실험할 수 있다.

#### 핵심 API

`HeapCreate` · `HeapAlloc` · `HeapReAlloc` · `HeapFree` · `HeapDestroy`  
`HeapSize` · `HeapWalk` · `HeapQueryInformation`

#### 실습 절차

1. 전용 힙 생성 → 크기 다른 블록 20개 할당 → 짝수 인덱스 해제 → `HeapWalk`로 단편화 관찰.
2. 기본 힙과 전용 힙의 가상 주소 범위 비교 → 서로 다른 VA 영역에 위치함 확인.
3. `HEAP_NO_SERIALIZE` 힙을 4 스레드가 동시 `HeapAlloc/Free` → 손상 또는 크래시 확인.
4. `HeapQueryInformation(HeapCompatibilityInformation)` → LFH 활성화 여부 출력.

#### 관찰 포인트

- `HeapWalk` 출력에서 `PROCESS_HEAP_ENTRY_BUSY` (할당됨) vs 0 (여유) 비율.
- LFH 활성화 임계값: 동일 크기로 18회 할당 후 상태 변화.

#### 예상 출력 예시

```
Custom heap  @ 0x000001B2`00000000
Process heap @ 0x000001A0`00000000
[Walk] Busy:10  Free:10  (alternating free)
[LFH] Not active → 18x same-size alloc → LFH active
[RACE] Heap corruption: HeapAlloc returned NULL or crash
```

#### 함정 & 주의사항

- `HeapFree`로 이미 해제된 포인터를 다시 해제하면 힙 손상.
- `HeapDestroy` 이후 해당 힙의 포인터를 역참조하면 조용한 메모리 손상 가능.
- `HeapWalk`는 힙 락을 점유하며 순회 — 긴 순회 중 다른 스레드 `HeapAlloc`은 블로킹.

#### 확인 질문

1. LFH가 활성화된 버킷과 일반 힙의 내부 구조 차이는?
2. `HEAP_NO_SERIALIZE` 힙의 보안 이점은? (힙 스프레이 공격 관점)
3. `HeapCreate(0, 0, 0)`와 `GetProcessHeap()`으로 얻은 힙의 차이는?

#### 확장 과제

고정 블록 크기(64바이트) 전용 풀 할당자를 `HeapCreate`로 구현하세요. 내부 여유 목록을 유지하여 `HeapFree` 비용 없이 재사용하는 slab 패턴을 구현합니다.

---

### 12_MemoryMappedFile

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 10

#### 이론 배경

`CreateFileMapping`은 파일(또는 페이지 파일)에 대한 커널 **섹션 오브젝트(Section Object)**를 생성한다. `MapViewOfFile`은 이 섹션을 프로세스 VA에 매핑하는 뷰를 만든다.

두 프로세스가 같은 섹션을 각자 `MapViewOfFile`하면 **같은 물리 페이지를 서로 다른 가상 주소**로 공유한다.

```
Process A               Physical Memory       Process B
VA: 0x200`00000000 ──► [Page Frame 1234] ◄── VA: 0x300`00000000
VA: 0x200`00001000 ──► [Page Frame 1235] ◄── VA: 0x300`00001000
```

`INVALID_HANDLE_VALUE`를 파일 핸들로 주면 **페이지 파일 기반 익명 공유 메모리**가 된다. 이름을 부여하면 다른 프로세스가 `OpenFileMapping`으로 접근 가능하다.

#### 학습 목표

1. 파일을 메모리에 매핑하고 슬라이딩 윈도로 대용량 파일을 처리할 수 있다.
2. 이름 있는 섹션으로 두 프로세스 간 메모리를 공유할 수 있다.
3. `FlushViewOfFile`과 `FlushFileBuffers`의 관계를 이해하고 적용할 수 있다.

#### 핵심 API

`CreateFileMapping` · `OpenFileMapping` · `MapViewOfFile`  
`UnmapViewOfFile` · `FlushViewOfFile`

#### 실습 절차

1. 100MB 이진 파일 → `PAGE_READONLY` + `MapViewOfFile(FILE_MAP_READ)` → `memcmp`로 내용 검증.
2. `INVALID_HANDLE_VALUE` + `"Local\\OSSharedMem"` → 서버 쓰기, 클라이언트 `OpenFileMapping`으로 읽기.
3. 큰 파일에 4MB 슬라이딩 윈도(`dwFileOffsetLow`를 1MB씩 증가)로 전체 파일 순회 → 속도 비교.
4. `FlushViewOfFile` + `FlushFileBuffers`로 변경 내용 디스크 반영 확인.

#### 관찰 포인트

- VMMap: `MappedFile` 타입 영역이 Reserved 없이 바로 Committed로 표시됨.
- 두 프로세스의 매핑 가상 주소가 달라도 내용이 공유됨을 확인.
- 파일 오프셋이 `dwAllocationGranularity`(65536) 배수여야 함을 오류로 확인.

#### 예상 출력 예시

```
[SERVER] Wrote "tick=1234567" to shared memory
[CLIENT] Read  "tick=1234567" from shared memory
[SLIDING] 100MB in 128ms vs ReadFile: 312ms
```

#### 함정 & 주의사항

- `UnmapViewOfFile` 없이 `CloseHandle(hMapping)`만 하면 뷰는 살아있지만 섹션 핸들이 닫힘.
- 두 프로세스 동시 쓰기 → 경쟁 조건. 이름 있는 Mutex로 직렬화 필요.
- 공유 메모리에 **절대 포인터 저장 금지** — VA가 프로세스마다 다르다. 오프셋 사용.

#### 확인 질문

1. `File_MAP_COPY` 매핑의 사용 사례는?
2. `MEM_MAPPED` vs `MEM_PRIVATE` 타입의 차이는 (`VirtualQuery` 기준)?
3. 섹션 오브젝트에 `SEC_IMAGE` 플래그가 필요한 경우는?

#### 확장 과제

공유 메모리 위에 링 버퍼(header: write_idx, read_idx, capacity)를 구현해 서버→클라이언트 단방향 스트림을 만드세요. Mutex로 인덱스 업데이트를 보호합니다.

---

### 13_PageTable_Walk

**난이도** ★★★★ | **예상 시간** 5h | **선행 모듈** 10 + WinDbg 설치

#### 이론 배경

##### x64 4단계 페이지 테이블

x64 Windows는 **4단계 페이지 테이블** 구조로 48비트 가상 주소를 변환한다.

```
Virtual Address (48-bit)
┌──────┬──────┬──────┬──────┬────────────┐
│ PML4 │ PDPT │  PD  │  PT  │   Offset   │
│[8:0] │[8:0] │[8:0] │[8:0] │  [11:0]    │
│ 9bit │ 9bit │ 9bit │ 9bit │   12bit    │
└──────┴──────┴──────┴──────┴────────────┘
   ↓       ↓      ↓      ↓
  512    512    512    512   = 2^36 pages × 4KB = 256TB 가상 공간
```

**변환 경로**:
1. CR3 레지스터 → PML4 테이블의 물리 주소 (프로세스별, 컨텍스트 전환 시 갱신)
2. PML4 테이블 + VA[47:39] 인덱스 → PDPT 물리 주소
3. PDPT + VA[38:30] → PD 물리 주소 (또는 1GB Large Page)
4. PD + VA[29:21] → PT 물리 주소 (또는 2MB Large Page)
5. PT + VA[20:12] → 물리 페이지 프레임 번호 (PFN)
6. PFN × 4096 + VA[11:0] → **물리 주소**

##### 페이지 테이블 엔트리 (PTE) 구조

```
PTE (8 bytes)
Bit  0:   P   (Present) — 페이지가 물리 메모리에 있음
Bit  1:   RW  (Read/Write) — 쓰기 허용
Bit  2:   US  (User/Supervisor) — 사용자 모드 접근 허용
Bit  3:   PWT (Page Write Through)
Bit  4:   PCD (Page Cache Disable)
Bit  5:   A   (Accessed) — CPU가 페이지 접근 시 자동 설정
Bit  6:   D   (Dirty) — 쓰기 발생 시 자동 설정
Bit  7:   PS  (Page Size) — 1이면 Large Page (PD 레벨에서)
Bit 11:8: (available for OS use)
Bit 51:12: Physical Page Frame Number
Bit 63:   NX  (No-Execute) — DEP의 하드웨어 구현
```

TLB는 최근 VA→PA 변환을 캐시한다. `INVLPG` 명령으로 특정 VA의 TLB 엔트리를 무효화하고, 페이지 테이블 수정 시 **TLB 샷다운 IPI**(Inter-Processor Interrupt)가 전체 CPU에 전송된다.

##### PFN 데이터베이스

커널은 `MmPfnDatabase`에 각 물리 페이지의 상태를 추적한다:

| PFN 상태 | 의미 |
|---|---|
| Active | 프로세스 working set에 포함 (PTE.P = 1) |
| Standby | Evicted됐지만 디스크 기록 완료. 재접근 시 소프트 폴트로 빠르게 복구 |
| Modified | Dirty 상태로 Evict됨. 페이지 파일에 아직 기록 안 됨 → 재접근 또는 기록 대기 |
| ModifiedNoWrite | Modified이지만 기록 억제 중 |
| Free | 물리 메모리로 반환됨, 아직 Zero화 안 됨 |
| Zero | `MmZeroPageThread`가 초기화 완료. 새 할당에 즉시 사용 |
| Bad | 하드웨어 오류로 사용 불가 |

##### 페이지 폴트 분류

```
접근 → P=0 → 페이지 폴트 핸들러
                 │
          ┌──────┴──────┐
          │ 어디 있는가? │
          └──────┬──────┘
       Standby/  │  Transition  │  페이지 파일 or 디스크
       Modified  ↓              ↓
      [Soft Fault]         [Hard Fault]
      ~수 마이크로초         ~수 밀리초 (I/O 포함)
```

**소프트 폴트**: 페이지가 Standby/Modified 목록에 있어 디스크 I/O 없이 PTE만 수정. **하드 폴트**: 페이지 파일이나 실행 파일에서 실제 디스크 읽기 필요.

**Copy-on-Write(CoW)**: `FILE_MAP_COPY` 매핑이나 fork 유사 패턴에서, 최초 쓰기 시 커널이 새 물리 페이지를 할당하고 내용을 복사한 뒤 PTE를 업데이트한다.

#### 학습 목표

1. 48비트 가상 주소를 수동으로 PML4→PT 인덱스로 분해할 수 있다.
2. WinDbg `!pte`, `!pfn`, `!vm` 명령으로 페이지 테이블 체인을 추적할 수 있다.
3. 소프트 폴트와 하드 폴트를 `GetProcessMemoryInfo`로 측정하고 차이를 설명할 수 있다.
4. PTE 플래그 (`Accessed`, `Dirty`, `NX`)를 실험으로 관찰할 수 있다.

#### 핵심 API

`VirtualQuery` · `GetProcessMemoryInfo` · `QueryWorkingSetEx`  
`VirtualLock` · `VirtualUnlock`  
WinDbg: `!pte`, `!pfn`, `!vm`, `!address`

#### 실습 절차

1. 가상 주소 하나를 선택해 수동으로 PML4[va>>39&0x1FF], PDPT[va>>30&0x1FF], PD[va>>21&0x1FF], PT[va>>12&0x1FF] 인덱스를 계산하고 출력.
2. WinDbg: 같은 주소에 `!pte <va>` 실행 → 출력에서 PFN, Dirty, Accessed, NX 비트 확인.
3. 배열 할당 직후 vs 전체 쓰기 후 vs `SetProcessWorkingSetSize`로 트리밍 후 `GetProcessMemoryInfo`의 `WorkingSetSize`·`PageFaultCount` 비교.
4. 소프트 폴트 시뮬레이션: 배열 접근 → `SetProcessWorkingSetSize(min, min)` 강제 트리밍 → 재접근 → Soft Fault 카운트 증가 확인.
5. `VirtualLock`으로 페이지를 메모리에 고정 → Working Set에 항상 포함됨 확인.

#### 관찰 포인트

- WinDbg `!address <va>` → `Type`, `State`, `Protect`, `Usage` 필드 분석.
- VMMap에서 "Private Data" / "Mapped File" / "Image" 타입 분류 확인.
- `!pfn <pfn번호>` → PFN 상태(Active/Standby/Modified) 확인.
- WinDbg `!vm` → 커밋 전체 크기, 사용 가능 커밋 한도, 페이지 파일 크기 확인.

#### 예상 출력 예시

```
VA = 0x00007FF7`12345678
  PML4 index: 255  (bits 47:39)
  PDPT index: 388  (bits 38:30)
  PD   index: 145  (bits 29:21)
  PT   index:  52  (bits 20:12)
  Offset:   0x678  (bits 11:0)

WinDbg !pte 0x7ff712345678:
  PML4E: 0a000001`2c345863  pfn=0x12c345  W U A
  PDPTE: 0a000001`3d456863  pfn=0x13d456  W U A
  PDE:   0a000001`4e567863  pfn=0x14e567  W U A
  PTE:   81000001`5f678025  pfn=0x15f678  R U A D  NX  (R=Readonly, NX=no-execute)

[WorkingSet] Before: 4MB  After alloc+write: 20MB  After trim: 4MB
[PageFaults] After re-access: +1 soft fault per trimmed page
```

#### 함정 & 주의사항

- WinDbg `!pte`는 커널 디버거(KD) 또는 PROCESS 컨텍스트 지정 후(`!process -1 0`)에 정확한 결과를 반환한다.
- `VirtualLock`은 `SE_LOCK_MEMORY_PRIVILEGE`가 활성화되어 있어야 대용량 잠금이 가능하다.
- 페이지 폴트 수 측정에서 시스템 DLL 로딩 등의 노이즈를 고려해야 한다.

#### 확인 질문

1. CR3 레지스터를 변경하면 어떤 일이 일어나는가? 컨텍스트 전환 시 CR3는 어떻게 처리되는가?
2. Large Page(2MB)를 사용하면 페이지 테이블 단계가 하나 줄어든다 — 어느 단계가 생략되는가?
3. `PAGE_GUARD` 페이지를 접근한 뒤 PTE의 어떤 비트가 어떻게 변경되는가?
4. TLB 샷다운 IPI가 너무 자주 발생하면 성능에 어떤 영향을 주는가? 최소화하는 방법은?

#### 확장 과제

`QueryWorkingSetEx(PSAPI_WORKING_SET_EX_INFORMATION)`으로 Working Set의 각 페이지별 물리 주소·공유 여부·잠금 여부를 출력하는 도구를 구현하세요. VMMap의 "Physical Pages" 탭과 비교합니다.

---

### 14_LargePages_MemProtection

**난이도** ★★★ | **예상 시간** 4h | **선행 모듈** 13

#### 이론 배경

##### Large Pages (2MB)

표준 페이지 크기는 4KB다. x64 CPU는 PD 레벨에서 `PS` 비트를 설정해 **2MB Large Page**를 지원한다 (PT 단계를 건너뜀).

```
4KB 페이지: PML4 → PDPT → PD → PT → 물리 주소  (4 단계 + offset)
2MB 페이지: PML4 → PDPT → PD[PS=1]   → 물리 주소  (3 단계 + 21-bit offset)
```

**Large Page 이점**:
- TLB 엔트리 하나가 4KB 대신 2MB를 커버 → TLB miss rate 대폭 감소.
- 데이터베이스 버퍼 풀, 게임 에셋 캐시 등 대용량 연속 접근에 유리.

**Large Page 제약**:
- `SeLockMemoryPrivilege` 필요 (기본 비활성, Local Security Policy에서 활성화).
- 항상 물리 메모리에 잠김(non-pageable) → 과도한 사용은 메모리 압박.
- 부분 Decommit 불가 — 전체 해제만 가능.
- 할당 크기는 `GetLargePageMinimum()`의 배수여야 함 (보통 2MB).

##### ASLR (Address Space Layout Randomization)

ASLR은 익스플로잇의 절대 주소 의존성을 제거하는 방어 기법이다.

```
일반 실행 (ASLR 없음):   ntdll 항상 0x77000000에 로드
ASLR 적용:               ntdll base = f(boot entropy + RDRAND)
```

엔트로피 소스:
- 이미지 베이스: 8비트 엔트로피 (256가지)
- 스택: 17비트 (64KB 단위 × 128가지)
- 힙: 5비트 (32가지)

`PROCESS_MITIGATION_ASLR_POLICY`로 프로세스별 ASLR 정책을 쿼리/변경할 수 있다.

##### DEP / NX (Data Execution Prevention)

DEP는 데이터 영역의 코드 실행을 막는 보안 기법이다.

**하드웨어 DEP**: CPU PTE의 NX 비트(`Bit 63`)가 설정된 페이지에서 코드 실행 시 `EXCEPTION_ACCESS_VIOLATION`.

```
Stack/Heap PTE: NX=1  → 쉘코드 실행 불가
Code segment:   NX=0  → 실행 가능
```

`GetProcessDEPPolicy`로 현재 프로세스의 DEP 정책 확인. `SetProcessDEPPolicy`로 변경 가능 (제약 있음).

##### CFG (Control Flow Guard)

CFG는 간접 호출(`call rax`, `jmp rcx`)의 대상이 컴파일러가 표시한 유효한 함수 시작점인지 런타임에 검증한다.

```
[CFG 없는 익스플로잇]     [CFG 적용 후]
jmp [overwritten ptr]  →   _guard_check_icall() → 유효 주소? 예: 실행 / 아니오: 종료
```

컴파일러 `/guard:cf` 플래그 + 커널의 유효 함수 비트맵으로 구현.

#### 학습 목표

1. `SeLockMemoryPrivilege`를 활성화하고 Large Page를 할당해 TLB 성능 차이를 측정할 수 있다.
2. 동일 프로그램을 여러 번 실행해 ASLR 엔트로피를 측정할 수 있다.
3. `GetProcessDEPPolicy`로 DEP 정책을 쿼리하고 NX 비트 동작을 실험할 수 있다.

#### 핵심 API

`GetLargePageMinimum` · `VirtualAlloc(MEM_LARGE_PAGES)`  
`AdjustTokenPrivileges(SeLockMemoryPrivilege)`  
`GetProcessDEPPolicy` · `SetProcessDEPPolicy`  
`GetProcessMitigationPolicy` · `SetProcessMitigationPolicy`  
`GetModuleInformation`

#### 실습 절차

1. `AdjustTokenPrivileges`로 `SeLockMemoryPrivilege` 활성화 → `GetLargePageMinimum()` 출력 → `MEM_LARGE_PAGES`로 2MB 할당.
2. 4KB 페이지 vs 2MB Large Page로 1GB 배열 순차 읽기 벤치마크 → `QueryPerformanceCounter`로 시간 비교.
3. 동일 exe를 10회 실행 → 각 실행의 `GetModuleHandle("ntdll.dll")` 베이스 주소 비교 → ASLR 엔트로피 측정.
4. `GetProcessDEPPolicy(GetCurrentProcess())` → 현재 DEP 정책 출력.
5. `SetProcessMitigationPolicy(ProcessDynamicCodePolicy)` → 동적 코드 생성 차단 후 `VirtualAlloc(PAGE_EXECUTE_READWRITE)` 시도 → 실패 확인.

#### 관찰 포인트

- Large Page 할당 후 VMMap에서 해당 영역이 `MEM_LARGE_PAGES`로 표시되는지 확인.
- ASLR 비활성화(`HKLM\...\MoveImages = 0`) 후 같은 dll 주소가 고정되는지 확인 (테스트 VM에서만).
- `GetSystemInfo`의 `dwPageSize`(4096)와 `GetLargePageMinimum()`(2MB) 비교.

#### 예상 출력 예시

```
Large page minimum: 2,097,152 bytes (2MB)
[BENCH] 4KB pages 1GB read: 312ms
[BENCH] 2MB pages 1GB read: 198ms  (1.58× faster)
[ASLR] ntdll base across 10 runs: 0x7FFA`12340000 → 0x7FFF`87650000 (varies)
[DEP] Policy: enabled-permanent, ATL thunk emulation: disabled
[DynCode] VirtualAlloc PAGE_EXECUTE_READWRITE: ERROR_ACCESS_DENIED (policy blocked)
```

#### 함정 & 주의사항

- `SeLockMemoryPrivilege`는 기본적으로 관리자도 비활성화 상태다 — 로컬 보안 정책(`secpol.msc`)에서 "Lock pages in memory"에 계정을 추가해야 한다.
- Large Page는 페이지 파일에 스왑 불가 — 물리 메모리가 부족한 환경에서 대량 사용하면 OOM.
- CFG를 우회하는 고급 익스플로잇 기법(JOP, COOP)이 존재한다 — CFG는 만능이 아니다.

#### 확인 질문

1. Large Page 할당이 `MEM_COMMIT`과 `MEM_RESERVE`를 동시에 요구하는 이유는?
2. ASLR이 활성화되어 있어도 같은 프로세스 내에서 `GetModuleHandle`은 항상 같은 값을 반환한다 — 왜인가?
3. DEP가 활성화된 상태에서 JIT 컴파일러(예: JavaScript 엔진)는 어떻게 실행 가능한 메모리를 생성하는가?
4. `SetProcessMitigationPolicy(ProcessSignaturePolicy)`가 어떤 공격을 막는가?

#### 확장 과제

`QueryWorkingSetEx`를 이용해 현재 프로세스 Working Set에서 Large Page(`PSAPI_WORKING_SET_EX_BLOCK.LargePage`)로 표시된 페이지와 일반 페이지의 비율을 출력하세요.

---

## Phase 4: 동시성

---

### 15_CriticalSection

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** Phase 2 완료

#### 이론 배경

##### CRITICAL_SECTION 내부 구조

`CRITICAL_SECTION`은 **사용자 모드 스핀 + 커널 이벤트** 하이브리드다.

```cpp
typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    LONG    LockCount;       // -1: 잠금 없음, ≥0: 대기 스레드 수 − 1
    LONG    RecursionCount;  // 재진입 횟수 (0이면 잠금 해제)
    HANDLE  OwningThread;    // 현재 소유 스레드 ID
    HANDLE  LockSemaphore;   // 커널 이벤트 핸들 (경합 시 생성)
    ULONG_PTR SpinCount;     // 커널 전환 전 사용자 모드 스핀 횟수
} CRITICAL_SECTION;
```

**동작 흐름**:
1. `EnterCriticalSection`: 먼저 `SpinCount`만큼 `LockCount`의 원자 증가 시도.
2. 스핀 중 잠금 해제되면 커널 전환 없이 획득.
3. 스핀 실패 → `LockSemaphore` 커널 이벤트 생성 → `WaitForSingleObject`로 블로킹.
4. `LeaveCriticalSection`: `RecursionCount--` → 0이 되면 `LockCount`를 −1로 재설정, 대기자가 있으면 이벤트 신호.

**스핀카운트 효과**: 짧은 임계 구역(수백 사이클 이하)에서는 스핀 중 잠금이 해제되어 커널 전환(~수 마이크로초)을 피한다. 멀티코어에서만 의미있다.

#### 학습 목표

1. CS로 공유 카운터를 보호하고 데이터 경쟁이 없는 결과를 얻을 수 있다.
2. 스핀카운트가 성능에 미치는 영향을 벤치마크로 측정할 수 있다.
3. `TryEnterCriticalSection`으로 비블로킹 획득 패턴을 구현할 수 있다.

#### 핵심 API

`InitializeCriticalSection` · `InitializeCriticalSectionAndSpinCount`  
`EnterCriticalSection` · `TryEnterCriticalSection` · `LeaveCriticalSection`  
`DeleteCriticalSection`

#### 실습 절차

1. 보호 없이 4 스레드가 전역 카운터 1천만 회 증가 → 최종값이 4천만과 다름 확인(데이터 경쟁).
2. CS로 보호 추가 → 정확한 4천만 확인 + 소요 시간 측정.
3. `InitializeCriticalSectionAndSpinCount` 0 / 400 / 4000으로 성능 비교.
4. 재진입: 같은 스레드에서 `Enter` 3회 → `Leave` 3회 → 정상 해제 확인. `Leave` 2회만 호출 후 다른 스레드 블로킹 확인.

#### 관찰 포인트

- `CRITICAL_SECTION.RecursionCount`가 `Enter` 횟수를 반영하는지 디버거에서 확인.
- 단일 코어에서 스핀카운트 효과가 없음을 확인 (스핀 중 다른 스레드가 실행될 수 없음).

#### 예상 출력 예시

```
[No Lock]   result=37,841,293  (expected 40,000,000) RACE
[With CS]   result=40,000,000  time=245ms
[Spin=400]  result=40,000,000  time=198ms
[Spin=4000] result=40,000,000  time=191ms
```

#### 함정 & 주의사항

- `DeleteCriticalSection`을 스레드가 보유 중인 CS에 호출 → 정의되지 않은 동작.
- `LeaveCriticalSection` 누락 → 데드락. RAII 래퍼 필수.
- 단일 스레드 테스트에서는 스핀카운트 효과가 측정되지 않는다.

#### 확인 질문

1. `CRITICAL_SECTION`이 순수 사용자 모드로 구현되지 않고 커널 이벤트를 사용하는 이유는?
2. 스핀카운트를 너무 높게 설정하면 오히려 성능이 저하될 수 있는 이유는?
3. `LockCount`가 −1일 때와 0일 때의 차이는?

#### 확장 과제

`CRITICAL_SECTION` 대신 `std::mutex`를 사용해 동일 벤치마크를 실행하고 성능을 비교하세요.

---

### 16_SRWLock

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 15

#### 이론 배경

##### RTL_SRWLOCK 비트 구조

`SRWLOCK`은 **순수 사용자 모드** — 커널 오브젝트가 없다. 한 개의 `ULONG_PTR`(포인터 크기) 안에 모든 상태를 비트로 인코딩한다.

```
SRWLOCK (8 bytes on x64)
┌──────────────────────────────────────────────────────────────────┐
│  Waiter Queue Pointer (62 bits)  │  Exclusive (1) │  Locked (1) │
└──────────────────────────────────────────────────────────────────┘
- Locked=0, Exclusive=0: 잠금 없음
- Locked=1, Exclusive=0: 공유 잠금 보유 (독자 수를 별도 카운팅)
- Locked=1, Exclusive=1: 독점 잠금 보유
- Waiter Pointer ≠ 0: 대기 스레드 링크드 리스트 존재
```

내부적으로 `RtlAcquireSRWLockExclusive`는 CAS 루프를 통해 비트를 원자적으로 설정한다. 경합이 있으면 `NtWaitForKeyedEvent`를 통해 커널로 전환한다.

**업그레이드 불가 제약**: 공유 → 독점 업그레이드를 직접 시도하면 데드락. 반드시 공유 해제 → 독점 획득 → 조건 재확인 패턴을 사용해야 한다.

#### 학습 목표

1. 읽기-쓰기 비율에 따라 SRW의 처리량 이점을 CS와 비교해 측정할 수 있다.
2. `TryAcquireSRWLockShared/Exclusive`로 비블로킹 획득을 구현할 수 있다.
3. 업그레이드 패턴(공유 해제 → 독점 획득 → 재확인)을 안전하게 구현할 수 있다.

#### 핵심 API

`InitializeSRWLock` · `AcquireSRWLockShared` · `ReleaseSRWLockShared`  
`AcquireSRWLockExclusive` · `ReleaseSRWLockExclusive`  
`TryAcquireSRWLockShared` · `TryAcquireSRWLockExclusive`

#### 실습 절차

1. 공유 정수 + 4 독자 + 1 기록자 → SRW 공유/독점 잠금으로 보호.
2. 독자 비율 90% / 50% / 10%로 바꾸면서 CS vs SRW 처리량 비교.
3. 공유 → 독점 직접 업그레이드 시도 → 데드락 발생 확인 (타임아웃으로 회수).
4. 안전한 업그레이드 패턴 구현: `ReleaseSRWLockShared` → `AcquireSRWLockExclusive` → 조건 재확인.

#### 관찰 포인트

- 독자 비율 90% 이상에서 SRW 처리량이 CS 대비 유의미하게 높은지 확인.
- WinDbg `dt ntdll!_RTL_SRWLOCK <addr>`로 비트 필드 관찰.

#### 예상 출력 예시

```
[Read 90%]  CS:  4.2M ops/s   SRW: 11.8M ops/s   (+181%)
[Read 50%]  CS:  3.9M ops/s   SRW:  5.1M ops/s    (+31%)
[Read 10%]  CS:  4.1M ops/s   SRW:  3.8M ops/s     (−7%)
```

#### 함정 & 주의사항

- `Release*Shared`와 `Release*Exclusive`를 반대로 호출 → 락 상태 손상.
- `SRWLOCK`은 프로세스 간 공유 불가 — 공유 메모리에 복사해도 동작하지 않는다.
- 기아(Starvation): 독자가 계속 유입되면 기록자가 독점 잠금을 획득하지 못할 수 있다.

#### 확인 질문

1. `SRWLOCK`이 프로세스 간 공유가 불가능한 근본 이유는?
2. 독자 비율 10%에서 CS가 SRW보다 빠를 수 있는 이유는?
3. `std::shared_mutex`와 Windows `SRWLOCK`의 구현 관계는?

#### 확장 과제

`TryAcquireSRWLockExclusive`를 이용한 비블로킹 스핀 기록자를 구현하고 블로킹 기록자와 처리량을 비교하세요.

---

### 17_ConditionVariable

**난이도** ★★☆ | **예상 시간** 4h | **선행 모듈** 15

#### 이론 배경

`CONDITION_VARIABLE`은 `CRITICAL_SECTION` 또는 `SRWLOCK`과 함께 사용한다. `SleepConditionVariableCS`는 CS를 **원자적으로 해제**하고 대기한다 — 이 두 동작 사이에 신호가 와도 놓치지 않는다.

**Spurious Wakeup**: `SleepConditionVariableCS`는 신호 없이 깨어날 수 있다. Windows 구현에서는 드물지만 스펙상 허용되므로 대기 루프에서 **조건 재확인**(`while`, not `if`)이 필수다.

```cpp
// 올바른 패턴
EnterCriticalSection(&cs);
while (!condition_is_true())          // 반드시 while — if는 Spurious Wakeup에 취약
    SleepConditionVariableCS(&cv, &cs, INFINITE);
// 조건이 참임이 보장된 상태
LeaveCriticalSection(&cs);
```

#### 학습 목표

1. `CONDITION_VARIABLE` + CS로 안전한 Producer-Consumer를 구현할 수 있다.
2. Bounded Buffer에서 두 조건 변수(notFull, notEmpty)를 관리할 수 있다.
3. `WakeConditionVariable` vs `WakeAllConditionVariable`의 동작 차이를 실험으로 확인할 수 있다.

#### 핵심 API

`InitializeConditionVariable` · `SleepConditionVariableCS` · `SleepConditionVariableSRW`  
`WakeConditionVariable` · `WakeAllConditionVariable`

#### 실습 절차

1. 무한 큐 + 1 생산자 + 2 소비자: 100개 항목 삽입 후 소비자들이 처리.
2. 유한 큐(용량 10) + 2 생산자 + 2 소비자: `notFull`·`notEmpty` 두 조건 변수로 흐름 제어.
3. `WakeConditionVariable` vs `WakeAllConditionVariable`로 소비자 2개를 깨울 때 차이 측정.
4. `if` → `while` 변경 실험: Spurious Wakeup 시뮬레이션 (시간 초과 `SleepConditionVariableCS` 사용).

#### 함정 & 주의사항

- `SleepConditionVariableCS`를 CS 없이 호출 → 정의되지 않은 동작.
- 조건 변수를 신호하기 **전** CS 해제 → 타이밍 이슈. CS 보유 중 신호가 안전.
- `WakeAllConditionVariable` 후 N개가 깨어나도 실제 진행 가능한 것은 1개뿐 — 나머지는 다시 대기.

#### 확인 질문

1. `SleepConditionVariableCS`가 CS를 원자적으로 해제해야 하는 이유는? (비원자적이면 어떤 문제가 생기나)
2. Spurious Wakeup이 실제로 발생하는 원인은 무엇인가?
3. `CONDITION_VARIABLE`을 `SRWLOCK`과 함께 쓸 때 `SleepConditionVariableSRW`의 두 번째 플래그 파라미터는 무슨 역할인가?

#### 확장 과제

`WakeAllConditionVariable`을 사용하되 정확히 K개 소비자만 진행하도록 허용하는 세마포 기반 패턴으로 리팩터링하세요.

---

### 18_EventSemaphore

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 15

#### 이론 배경

이벤트와 세마포는 커널 오브젝트로 프로세스 간 공유 가능하다.

**자동 리셋 이벤트**: 한 대기자가 `WaitForSingleObject`로 소비하면 자동으로 비신호. 한 번에 하나만 통과.  
**수동 리셋 이벤트**: `ResetEvent` 호출 전까지 모든 대기자 동시 통과.

**세마포**: 0~MaxCount 범위 카운터. `ReleaseSemaphore(+N)` → 최대 N개 대기자 해제.

`WaitForMultipleObjects(bWaitAll=FALSE)`: 가장 먼저 신호된 하나의 핸들 인덱스 반환.  
`WaitForMultipleObjects(bWaitAll=TRUE)`: 모든 핸들이 신호될 때까지 대기.

#### 학습 목표

1. 자동/수동 리셋 이벤트의 의미론 차이를 실험으로 확인할 수 있다.
2. `WFMO`로 여러 이벤트를 동시에 대기할 수 있다.
3. 세마포로 동시 실행 스레드 수를 제한할 수 있다.

#### 핵심 API

`CreateEvent` · `SetEvent` · `ResetEvent`  
`CreateSemaphore` · `ReleaseSemaphore` · `WaitForMultipleObjects`

#### 실습 절차

1. 자동 리셋 이벤트 + 3 대기 스레드 → `SetEvent` 1회 → 1개만 통과 확인.
2. 수동 리셋 이벤트 + 3 대기 스레드 → `SetEvent` → 3개 동시 통과 → `ResetEvent` 후 재대기.
3. `WFMO(FALSE)` + 5개 이벤트 → 먼저 신호된 인덱스 출력.
4. 세마포(MaxCount=3) + 10 스레드 → 동시 최대 3개 실행 중임을 `InterlockedIncrement`로 검증.

#### 함정 & 주의사항

- `PulseEvent`는 대기자가 없으면 신호가 소실 — 신뢰성 있는 통지에는 사용하지 말 것.
- `WFMO` 최대 핸들 수: `MAXIMUM_WAIT_OBJECTS(64)`.
- `ReleaseSemaphore`로 MaxCount 초과 시 `ERROR_TOO_MANY_POSTS` 오류.

#### 확인 질문

1. 수동 리셋 이벤트가 신호 상태일 때 `WaitForSingleObject`는 즉시 반환하는가?
2. 세마포와 뮤텍스의 근본적인 차이는? (소유권 관점)
3. `WaitForMultipleObjects(bWaitAll=TRUE)` 호출 중 하나의 핸들이 소멸되면 어떻게 되는가?

#### 확장 과제

`RegisterWaitForSingleObject`를 사용해 이벤트 신호를 스레드 풀 콜백으로 처리하는 비동기 리스너를 구현하세요.

---

### 19_Interlocked

**난이도** ★★★ | **예상 시간** 6h | **선행 모듈** 15

#### 이론 배경

##### x86 LOCK 접두사

`Interlocked*` 함수들은 x86/x64의 `LOCK` 접두사 명령어로 구현된다.

```asm
; InterlockedIncrement 내부
lock xadd [dest], 1    ; 원자적 add + 이전 값 반환
; lock 접두사: 버스 락 또는 캐시 라인 락으로 원자성 보장
; 암시적 full memory barrier (acquire + release)
```

`LOCK XCHG`는 접두사 없이도 암묵적 LOCK 의미를 가진다 (x86 아키텍처 명세상).

`InterlockedCompareExchange`는 `CMPXCHG` + `LOCK` 접두사로 구현:

```asm
lock cmpxchg [dest], newVal
; if [dest] == EAX: [dest] = newVal (ZF=1)
; else:            EAX = [dest]    (ZF=0)
```

##### CAS 루프 패턴

```cpp
LONG old_val, new_val;
do {
    old_val = *target;                         // 현재 값 읽기
    new_val = compute(old_val);                // 새 값 계산
} while (InterlockedCompareExchange(target, new_val, old_val) != old_val);
// 경합 발생 시 재시도 → 결국 성공
```

##### ABA 문제

```
스레드 A: ptr = head (A 노드 읽음)
스레드 B: pop(A), push(X), push(A)  → head = A 지만 next = X (A가 재사용됨)
스레드 A: CAS(head, B, A) 성공 → head = B 이지만 B의 next는 이미 소멸됨
```

해결: 64비트 시스템에서 `LONGLONG[2]` + `InterlockedCompareExchange128`으로 포인터와 버전 카운터를 함께 원자적으로 교체.

#### 학습 목표

1. `InterlockedCompareExchange` CAS 루프로 Lock-Free LIFO 스택을 구현할 수 있다.
2. ABA 문제를 재현하고 버전 카운터로 해결할 수 있다.
3. Lock-Free와 Mutex 기반의 성능을 경쟁 수준별로 비교할 수 있다.

#### 핵심 API

`InterlockedIncrement` · `InterlockedDecrement` · `InterlockedExchange`  
`InterlockedCompareExchange` · `InterlockedCompareExchange64`  
`InterlockedCompareExchange128` · `MemoryBarrier`

#### 실습 절차

1. 4 스레드 `InterlockedIncrement` 1천만 회 → CS 방식과 성능 비교.
2. CAS 루프로 Lock-Free LIFO 스택(`push`/`pop`) 구현 → 4 스레드 정확성 검증.
3. ABA 재현: 스레드 A가 pop 직전 일시 정지 → B가 push/pop 반복해 같은 주소 재사용 → A의 CAS가 잘못 성공.
4. `InterlockedCompareExchange128`으로 `{pointer, version}` 쌍을 원자 교체 → ABA 해결.

#### 관찰 포인트

- `InterlockedIncrement`가 단순 `++` 대비 캐시 라인 경쟁 시 몇 배 느린지 측정.
- 경쟁이 낮을 때(1 스레드) vs 높을 때(8 스레드) Lock-Free vs Mutex 성능 역전 여부.

#### 예상 출력 예시

```
[Interlocked] 4 threads×10M: 312ms  result=40,000,000 ✓
[CAS Stack]   push/pop 1M×4t: 89ms  remaining=0 ✓
[ABA Demo]    stale pointer accepted — incorrect pop!
[ABA Fixed]   tag-pointer: no false success in 1M iterations ✓
```

#### 함정 & 주의사항

- 64비트 포인터에 32비트 `InterlockedCompareExchange`를 사용하면 상위 비트 손상.
- pop 후 즉시 `delete`하면 다른 스레드가 해당 노드를 통해 `next`를 읽고 있을 수 있다 — Hazard Pointer나 지연 해제 필요.
- `_ReadWriteBarrier()`는 컴파일러 재정렬 방지만 한다. 하드웨어 배리어는 `MemoryBarrier()` 또는 `_mm_mfence()`.

#### 확인 질문

1. CAS 루프에서 경쟁이 심할 때 지수 백오프(exponential backoff)가 왜 성능을 개선하는가?
2. Lock-Free가 Deadlock-Free는 보장하지만 Starvation-Free는 보장하지 않는 이유는?
3. `InterlockedCompareExchange128`이 `__int128` 대신 두 개의 `LONGLONG`을 사용하는 이유는?

#### 확장 과제

Lock-Free LIFO를 **Michael-Scott Lock-Free FIFO 큐**로 확장하세요. head/tail 두 포인터를 각각 CAS로 관리하고 멀티스레드 정확성을 검증합니다.

---

### 20_x86_MemoryModel

**난이도** ★★★★ | **예상 시간** 6h | **선행 모듈** 13, 19

#### 이론 배경

##### x86/x64 TSO 메모리 모델

x86/x64는 **Total Store Order(TSO)** 메모리 모델을 구현한다. 순서 보장 규칙:

| 재정렬 | 허용 여부 | 이유 |
|---|---|---|
| Load → Load | **금지** | 같은 스레드 읽기는 순서 보장 |
| Store → Store | **금지** | 쓰기 버퍼는 FIFO |
| Load → Store | **금지** | 읽기는 이미 완료 후 쓰기 |
| **Store → Load** | **허용** | 쓰기 버퍼에 있는 동안 다른 스레드의 최신 값을 읽을 수 있음 |

이 마지막 규칙이 핵심이다: **스레드 A의 쓰기가 스토어 버퍼에 있는 동안, 스레드 B는 A의 쓰기가 반영되지 않은 이전 값을 읽을 수 있다.**

```
Thread A               Thread B
x = 1;                 y = 1;
r1 = y;                r2 = x;

// TSO 허용 결과: r1=0, r2=0
// (각 스레드의 쓰기가 상대방에게 보이기 전에 읽기가 실행됨)
```

##### 스토어 버퍼 (Store Buffer)

각 CPU 코어는 쓰기를 즉시 캐시/메모리에 반영하지 않고 **스토어 버퍼**에 쌓아둔다. 스토어 버퍼는 코어-local이며 다른 코어에는 보이지 않는다.

```
Core 0                      Core 1
[Store Buffer]              [Store Buffer]
  x=1 (pending)               y=1 (pending)
  ↓ (not yet in cache)         ↓ (not yet in cache)
[L1 Cache]                  [L1 Cache]
[L2/L3 Cache (shared)]
[Main Memory]
```

스토어 버퍼가 비워져(flush) 캐시에 쓰여지면 다른 코어에 MESI 프로토콜로 전파된다.

##### 메모리 배리어 명령어

| 명령어 | 의미 | Win32 매핑 |
|---|---|---|
| `MFENCE` | Full barrier: 이전 load/store가 모두 완료된 후 다음 명령 실행 | `MemoryBarrier()` |
| `SFENCE` | Store barrier: 이전 store가 모두 완료 | `_WriteBarrier()` + `_mm_sfence()` |
| `LFENCE` | Load barrier: 이전 load가 모두 완료 | `_ReadBarrier()` + `_mm_lfence()` |
| `LOCK XCHG` | 암시적 full barrier | `InterlockedExchange` |
| `XCHG` | (LOCK 없어도) 암시적 full barrier | — |

##### C++ std::atomic ↔ x86 명령어 매핑

| C++ memory_order | Load (x86) | Store (x86) |
|---|---|---|
| `relaxed` | MOV | MOV |
| `acquire` | MOV (x86은 load가 항상 acquire) | — |
| `release` | — | MOV (x86은 store가 항상 release) |
| `seq_cst` | MOV + MFENCE 또는 LOCK ADD 0 | XCHG 또는 MOV + MFENCE |

x86에서는 acquire load와 release store가 추가 명령어 없이 구현되므로 `memory_order_acquire/release`의 비용이 `relaxed`와 동일하다. `seq_cst`만 추가 비용이 발생한다.

##### Dekker's Algorithm — 배리어 없을 때의 실패

```cpp
// Thread 0               Thread 1
flag0 = true;             flag1 = true;
if (!flag1) {             if (!flag0) {
    // 임계 구역           //  임계 구역
}                         }

// TSO 허용: flag0=flag1=true 쓰기가 각자 스토어 버퍼에 있는 동안
// 두 스레드 모두 상대방 flag를 false로 읽어 동시에 진입 가능
```

MFENCE를 `if` 조건 전에 삽입하면 스토어 버퍼를 플러시하여 정확히 동작한다.

#### 핵심 개념

```
컴파일러 재정렬 (compiler reordering)
    └── _ReadWriteBarrier() / std::atomic_thread_fence() 로 방지

하드웨어 재정렬 (CPU reordering, store buffer)
    └── MFENCE / LOCK 접두사 / std::atomic seq_cst 로 방지

두 층 모두 방지해야 올바른 동시 코드
```

#### 학습 목표

1. Store-Load 재정렬을 실험으로 증명할 수 있다 (배리어 없이 `r1=r2=0` 결과 재현).
2. `MFENCE`/`LOCK XCHG`를 추가해 재정렬이 제거됨을 확인할 수 있다.
3. C++ `std::atomic`의 `memory_order_seq_cst` vs `relaxed` 성능 비용을 측정할 수 있다.
4. Dekker's Algorithm을 배리어 없이 / 있이 구현하고 정확성을 비교할 수 있다.

#### 핵심 API / 헤더

`<atomic>` : `std::atomic`, `std::atomic_thread_fence`, `std::memory_order_*`  
`<intrin.h>` : `_mm_mfence()`, `_mm_sfence()`, `_mm_lfence()`  
`_ReadWriteBarrier()`, `MemoryBarrier()` (winnt.h)  
`__rdtsc()` (타이밍)

#### 실습 절차

1. **Store-Load 재정렬 증명**: `x=1; MFENCE 없음; r1=y` 패턴 + 반복 실험 → `r1=0, r2=0` 결과 발생 빈도 측정.
2. 동일 실험에 `_mm_mfence()` 삽입 → `r1=r2=0` 발생 빈도 = 0 확인.
3. Dekker's Algorithm: 배리어 없는 버전 → 동시 진입 발생. 배리어 추가 → 정확성 확인.
4. `std::atomic<int>` 카운터로 `memory_order_relaxed` vs `memory_order_seq_cst` 처리량 벤치마크 (멀티스레드 환경).
5. `std::atomic_thread_fence(memory_order_seq_cst)` 독립 배리어 비용 측정 (`__rdtsc()`로 사이클 수 측정).

#### 관찰 포인트

- 코어 수가 많을수록 Store-Load 재정렬 관측 빈도가 높아지는지 확인.
- Release 빌드에서 컴파일러가 `volatile` 변수도 재정렬할 수 있음을 어셈블리 뷰에서 확인.
- `memory_order_seq_cst` store가 실제로 `XCHG` 명령어로 컴파일되는지 어셈블리 확인 (`/FAcs` 컴파일 옵션).

#### 예상 출력 예시

```
[No Barrier]   r1=0, r2=0 occurred: 347 / 10,000,000 iterations (0.003%)
[With MFENCE]  r1=0, r2=0 occurred: 0   / 10,000,000 iterations ✓

[Dekker No Barrier]  Mutual exclusion VIOLATED: 2 threads in CS simultaneously
[Dekker With MFENCE] Mutual exclusion maintained: 0 violations ✓

[Bench] relaxed:   128M ops/s
[Bench] seq_cst:    89M ops/s  (1.44× slower — MFENCE cost)

[Fence cost] MFENCE: ~18 cycles  (vs MOV: ~4 cycles)
```

#### 함정 & 주의사항

- `volatile`은 C++에서 **컴파일러 재정렬만** 방지한다 — CPU 하드웨어 재정렬(스토어 버퍼)은 막지 않는다. `std::atomic`이나 명시적 배리어가 필요하다.
- ARM 프로세서는 TSO보다 약한 메모리 모델(Weak Ordering)을 가진다 — x86에서 통과하는 테스트가 ARM에서는 실패할 수 있다.
- `memory_order_relaxed`와 `acquire/release`는 x86에서 동일한 명령어를 생성할 수 있지만 컴파일러 재정렬 방지 여부가 다르다.

#### 확인 질문

1. x86 TSO에서 Store-Load 재정렬이 허용되는 하드웨어 메커니즘(스토어 버퍼)을 설명하라.
2. `std::atomic<int> x; x.store(1, relaxed); x.load(relaxed);`가 x86에서 `MOV` 명령어로 컴파일될 수 있는 이유는?
3. ARM에서 `std::atomic`의 `acquire` load가 x86과 달리 추가 명령어(`DMB ISH`)를 필요로 하는 이유는?
4. `seq_cst` 스레드 펜스와 `seq_cst` atomic operation의 차이는?

#### 확장 과제

Peterson's Algorithm을 `std::atomic`의 `relaxed`, `acquire/release`, `seq_cst` 세 버전으로 각각 구현하고, 각 버전에서 상호 배제 위반이 발생하는지 10억 회 반복 테스트로 검증하세요. ARM 환경(WSL2 또는 실제 ARM 장치)에서도 동일 테스트를 실행합니다.

---

### 21_CacheCoherency

**난이도** ★★★★ | **예상 시간** 5h | **선행 모듈** 20

#### 이론 배경

##### 캐시 계층 구조

```
Core 0                    Core 1
┌────────────────┐        ┌────────────────┐
│  L1-I  (32KB)  │        │  L1-I  (32KB)  │
│  L1-D  (32KB)  │        │  L1-D  (32KB)  │
│  L2    (256KB) │        │  L2    (256KB) │
└───────┬────────┘        └───────┬────────┘
        └─────────┐  ┌───────────┘
               ┌──▼──▼──┐
               │   L3   │  (8~32MB, 모든 코어 공유)
               └────────┘
               │  Memory │  (~100ns latency)
```

캐시 계층별 대략적 지연:
- L1: 4 사이클 (~1.4ns)
- L2: 12 사이클 (~4ns)
- L3: 40 사이클 (~14ns)
- DRAM: 200+ 사이클 (~70ns)

**캐시 라인**: 캐시와 메모리 간 데이터 전송 단위, x86/x64는 **64바이트**.

##### MESI 프로토콜

MESI는 캐시 일관성을 유지하는 프로토콜이다. 각 캐시 라인은 네 가지 상태 중 하나를 가진다:

| 상태 | 의미 | 읽기 | 쓰기 |
|---|---|---|---|
| **M**odified | 이 코어만 보유, 메모리와 다름(dirty) | 가능 | 가능 |
| **E**xclusive | 이 코어만 보유, 메모리와 동일(clean) | 가능 | M으로 전환 (silent) |
| **S**hared | 여러 코어가 보유, clean | 가능 | 다른 코어에 Invalidate 전송 후 M |
| **I**nvalid | 이 캐시에 없음 | 캐시 미스 (다른 코어에서 가져옴) | 다른 코어에서 가져온 후 M |

**쓰기 비용**: Shared 상태 캐시 라인에 쓰려면 다른 모든 코어에 **Invalidate 메시지**를 전송해야 한다 → 캐시 라인이 자주 공유되면 코어 간 통신 비용이 증가한다.

##### False Sharing (거짓 공유)

두 스레드가 **논리적으로 독립적인 변수**를 사용하지만 **물리적으로 같은 캐시 라인**에 위치할 때 발생한다.

```
캐시 라인 (64바이트):
┌────────────────────────────────────────────────────────────────┐
│  Thread 0 counter (8B)  │  Thread 1 counter (8B)  │ padding... │
└────────────────────────────────────────────────────────────────┘
         ^                          ^
   Thread 0이 씀              Thread 1이 씀
   → 전체 캐시 라인 Invalidated → 서로 계속 캐시 미스 유발
```

**해결**: `alignas(64)`로 각 변수를 별도 캐시 라인에 배치.

```cpp
struct alignas(64) PaddedCounter {
    std::atomic<long long> value;
    char pad[64 - sizeof(std::atomic<long long>)];  // 캐시 라인 채우기
};
```

##### True Sharing과의 차이

- **False Sharing**: 두 변수가 같은 캐시 라인이지만 논리적으로 독립 → 패딩으로 해결.
- **True Sharing**: 두 스레드가 실제로 같은 데이터를 공유 → 동기화가 필요하며 패딩으로 해결 불가.

##### 하드웨어 Prefetcher

CPU 하드웨어 프리페처는 메모리 접근 패턴을 감지해 미리 캐시 라인을 가져온다:
- **순차 패턴**: 매우 효과적으로 프리페치
- **Stride 패턴**: 일정 간격 접근도 감지
- **임의 패턴**: 프리페치 불가 → 캐시 미스 다수

`_mm_prefetch(addr, _MM_HINT_T0)` 명시적 소프트웨어 프리페치로 보완 가능.

#### 핵심 개념

```
False Sharing 진단:
1. 성능이 스레드 수에 비례하지 않고 오히려 저하됨
2. L3/LLC 캐시 미스 수가 예상보다 매우 많음 (VTune/uProf로 측정)
3. 핫 캐시 라인을 공유하는 변수 발견

해결 순서:
1. alignas(64) 패딩으로 캐시 라인 분리
2. 스레드 로컬 카운터 사용 후 최종 합산
3. NUMA 로컬 메모리 사용 (09 모듈)
```

#### 학습 목표

1. False Sharing 시나리오에서 스레드 수 증가가 오히려 성능을 저하시킴을 실험으로 재현할 수 있다.
2. `alignas(64)` 패딩으로 False Sharing을 제거하고 성능을 회복시킬 수 있다.
3. 스레드 로컬 카운터 + 최종 합산 패턴으로 True Sharing 없이 구현할 수 있다.
4. `__rdtsc()` 또는 `QueryPerformanceCounter`로 캐시 라인 경쟁 비용을 정량화할 수 있다.

#### 핵심 헤더 / 매크로

`<immintrin.h>` : `_mm_prefetch`, `_mm_clflush`  
`<intrin.h>` : `__rdtsc()`  
`alignas(64)` (C++11)  
`std::hardware_destructive_interference_size` (C++17, 캐시 라인 크기 상수)

#### 실습 절차

1. **False Sharing 재현**: 인접한 카운터 배열 `long long counters[N]` → N개 스레드가 각자 `counters[i]++` × 1억 회 → 1, 2, 4, 8 스레드 처리량 측정.
2. **패딩 적용**: `struct alignas(64) PaddedCounter { atomic<long long> v; }; PaddedCounter counters[N]` → 동일 측정.
3. **스레드 로컬 합산**: 각 스레드가 지역 변수로 카운트 후 최종에 `InterlockedAdd` 한 번 → 성능 비교.
4. `std::hardware_destructive_interference_size`를 출력하고 `alignas`에 활용.
5. (선택) Intel VTune에서 "Memory Access" 분석 → LLC Miss 수 비교.

#### 관찰 포인트

- 스레드 수 1→2에서 처리량이 2배가 아닌 비율 측정 (False Sharing 심각도).
- 패딩 후 캐시 라인 크기(64) 배수인지 확인: `sizeof(PaddedCounter) % 64 == 0`.
- 패딩 적용 시 메모리 사용량 N배 증가 → 대용량 배열에는 패딩이 부담될 수 있음.

#### 예상 출력 예시

```
Cache line size: 64 bytes (hardware_destructive_interference_size)

[False Sharing]
  1 thread:  1,200M ops/s
  2 threads: 1,180M ops/s  (−1.7%  — WORSE than 1 thread!)
  4 threads:   980M ops/s  (−18%   — even worse)
  8 threads:   720M ops/s  (−40%   — severe degradation)

[Padded]
  1 thread:  1,200M ops/s
  2 threads: 2,380M ops/s  (+98%  — near-linear)
  4 threads: 4,710M ops/s  (+292%)
  8 threads: 9,250M ops/s  (+671%)

[TLS Sum]
  8 threads: 9,300M ops/s  (+675% — slightly better due to no atomic)
```

#### 함정 & 주의사항

- `std::hardware_destructive_interference_size`는 컴파일 시 상수지만 실제 캐시 라인 크기와 다를 수 있다 (예: L1과 L2가 다른 캐시 라인 크기를 가지는 아키텍처). `GetLogicalProcessorInformationEx`로 런타임 확인 가능.
- 패딩이 메모리 사용량을 캐시 라인 배수로 늘린다 — 대규모 객체 배열에서는 메모리 vs 성능 트레이드오프를 검토해야 한다.
- `_mm_clflush(addr)`로 특정 캐시 라인을 강제로 Flush할 수 있다 — 테스트에서 캐시 워밍 효과를 제거하는 데 유용.

#### 확인 질문

1. MESI 프로토콜에서 Exclusive 상태와 Modified 상태의 차이는? Exclusive → Modified 전환은 어떤 동작을 유발하는가?
2. False Sharing을 패딩으로 해결할 때 구조체 크기가 캐시 라인 크기의 **배수**여야 하는 이유는?
3. CPU 하드웨어 프리페처가 False Sharing을 악화시킬 수 있는 시나리오는?
4. `_mm_clflush`가 동기화 없이 캐시를 무효화할 수 있는 보안 취약점(Flush+Reload 사이드 채널)의 원리는?

#### 확장 과제

Intel VTune(또는 AMD uProf)을 사용해 False Sharing과 Padded 버전의 **L3 Cache Misses Per Instruction (MPKI)**를 비교 측정하세요. 두 버전의 MPKI 차이가 처리량 차이와 어떻게 상관관계를 가지는지 분석합니다.

---


## Phase 5: I/O 서브시스템

---

### 22_AsyncFileIO

**난이도** ★★★ | **예상 시간** 6h | **선행 모듈** Phase 2·3·4 완료

#### 이론 배경

Windows 비동기 I/O의 핵심은 `OVERLAPPED` 구조체다. `ReadFile`에 `OVERLAPPED`를 전달하면 즉시 반환되고 완료는 세 방식으로 통지된다: ① `OVERLAPPED.hEvent` 시그널, ② APC(Alertable I/O), ③ IOCP 완료 패킷.

**IOCP**: `CreateIoCompletionPort`로 파일 핸들을 IOCP에 연결하면, I/O 완료 시 `GetQueuedCompletionStatus`에서 패킷을 수신한다. 워커 스레드 수를 CPU 코어 수와 맞추면 최적 처리량을 달성한다.

**per-op 컨텍스트**: `OVERLAPPED`를 구조체 첫 번째 멤버로 배치 → `CONTAINING_RECORD` 매크로로 전체 컨텍스트 복원.

```cpp
struct MyOverlapped {
    OVERLAPPED  ov;        // 반드시 첫 번째
    DWORD       offset;
    BYTE*       buffer;
    DWORD       bufSize;
};
// GQCS에서 lpOverlapped를 MyOverlapped*로 캐스팅:
MyOverlapped* ctx = CONTAINING_RECORD(lpOverlapped, MyOverlapped, ov);
```

#### 학습 목표

1. `FILE_FLAG_OVERLAPPED` + IOCP 기반 비동기 파일 읽기 파이프라인을 구현할 수 있다.
2. per-op 컨텍스트 패턴으로 여러 비동기 작업을 추적할 수 있다.
3. `PostQueuedCompletionStatus`로 워커 스레드 종료 신호를 구현할 수 있다.

#### 핵심 API

`CreateFile(FILE_FLAG_OVERLAPPED)` · `ReadFile` · `WriteFile`  
`CreateIoCompletionPort` · `GetQueuedCompletionStatus` · `PostQueuedCompletionStatus`

#### 실습 절차

1. 100MB 파일 → `FILE_FLAG_OVERLAPPED`로 열기 → `ReadFile(4MB×25회)` → `WaitForSingleObject(ov.hEvent)` 방식 완료 대기.
2. IOCP 생성 + 파일 연결 → per-op 컨텍스트 → 워커 2개 `GQCS` 루프.
3. 파일 5개를 동시 비동기 읽기 → 완료 순서 로깅 (발행 순서와 다름 확인).
4. `PostQueuedCompletionStatus(iocp, 0, SHUTDOWN_KEY, NULL)`로 워커 종료 신호.

#### 관찰 포인트

- `FILE_FLAG_NO_BUFFERING` 추가 후 캐시 우회 측정 (페이지 정렬 버퍼 필요).
- IOCP 워커 수 1→2→4로 증가 시 처리 속도 변화.

#### 예상 출력 예시

```
[IOCP] Worker0: op#7 offset=28MB completed
[IOCP] Worker1: op#3 offset=12MB completed
[IOCP] Worker0: op#1 offset=0MB  completed  (order differs from submission)
Total: 100MB in 48ms (2 workers)
```

#### 함정 & 주의사항

- `ReadFile` 반환 `FALSE` + `ERROR_IO_PENDING` → 정상 비동기 시작.
- `OVERLAPPED.Offset/OffsetHigh`에 파일 오프셋을 반드시 설정.
- 비동기 I/O 완료 전 `OVERLAPPED` 구조체나 버퍼 해제 → 메모리 손상.

#### 확인 질문

1. `FILE_FLAG_NO_BUFFERING`으로 파일을 열 때 버퍼 정렬 요건이 필요한 이유는?
2. IOCP 워커 수를 CPU 코어 수에 맞추는 것이 일반 권장 사항인 이유는?
3. `GQCS`가 `FALSE` 반환 + `lpOverlapped != NULL`인 경우 무엇을 의미하는가?

#### 확장 과제

`ReadFileScatter`로 비연속 메모리 페이지에 분산 읽기(Scatter Read)를 구현하고 단순 `ReadFile` 루프와 성능을 비교하세요.

---

### 23_FSNotify

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 22

#### 이론 배경

`ReadDirectoryChangesW`는 디렉토리 변경(파일 생성·삭제·수정·이름 변경)을 `FILE_NOTIFY_INFORMATION` 구조체 배열로 전달한다. 동기/비동기 모드와 IOCP 연결을 지원한다.

버퍼 오버플로: 커널 버퍼가 넘치면 마지막 항목으로 `ReturnLength == 0`을 받고 이후 이벤트는 **소실**된다 — 감지 시 전체 디렉토리 재스캔이 필요하다.

#### 학습 목표

1. `ReadDirectoryChangesW`로 디렉토리 변경을 감지하고 `FILE_NOTIFY_INFORMATION`을 파싱할 수 있다.
2. 재귀/비재귀 감시의 이벤트 수 차이를 측정할 수 있다.
3. 버퍼 오버플로를 탐지하고 복구 전략을 구현할 수 있다.

#### 핵심 API

`CreateFile(FILE_FLAG_BACKUP_SEMANTICS)` · `ReadDirectoryChangesW`  
`FILE_NOTIFY_INFORMATION` · `FILE_ACTION_*` 상수

#### 실습 절차

1. 감시 디렉토리를 `FILE_FLAG_BACKUP_SEMANTICS`로 열기 → 동기 모드 → 파일 생성/수정/삭제 이벤트 출력.
2. `FILE_NOTIFY_INFORMATION` 연결 리스트 파싱 (`NextEntryOffset == 0` = 마지막).
3. `bWatchSubtree TRUE` vs `FALSE` 이벤트 수 비교.
4. 작은 버퍼(512바이트) + 대량 이벤트 → `ReturnLength == 0` 탐지.

#### 예상 출력 예시

```
[NOTIFY] ADDED    test.txt
[NOTIFY] MODIFIED test.txt
[NOTIFY] RENAMED  test.txt → renamed.txt
[NOTIFY] REMOVED  renamed.txt
[OVERFLOW] Buffer overflow — full rescan triggered
```

#### 함정 & 주의사항

- 감시 핸들은 `FILE_FLAG_BACKUP_SEMANTICS`로 열어야 한다.
- `FileName`은 NULL 종료 없는 `FileName[1]` — `FileNameLength`(바이트)로 직접 wstring 구성.
- 드라이브 루트 직접 감시 불가.

#### 확인 질문

1. 오버플로 시 취할 수 있는 복구 전략은?
2. `ReadDirectoryChangesW` 대신 ETW를 사용할 때의 장단점은?
3. 이름 변경은 왜 이벤트가 두 개(OLD_NAME + NEW_NAME)로 분리되는가?

#### 확장 과제

동일 파일에 100ms 내 발생한 이벤트를 디바운스해 마지막 하나만 콜백으로 전달하는 필터를 구현하세요.

---

### 24_DeviceIO

**난이도** ★★☆ | **예상 시간** 2h | **선행 모듈** 22

#### 이론 배경

`DeviceIoControl`은 사용자 모드에서 커널 드라이버에 **IOCTL**을 전달하는 인터페이스다. IOCTL 코드는 32비트 값으로 인코딩된다:

```
CTL_CODE(DeviceType, Function, Method, Access)
  DeviceType: FILE_DEVICE_DISK (7) 등
  Method:     METHOD_BUFFERED / METHOD_IN_DIRECT / METHOD_OUT_DIRECT / METHOD_NEITHER
  Access:     FILE_ANY_ACCESS / FILE_READ_DATA / FILE_WRITE_DATA
```

`METHOD_BUFFERED`: 입출력 버퍼가 커널이 중간 버퍼를 통해 복사된다. 소용량 데이터에 적합.  
`METHOD_IN_DIRECT`: 입력 버퍼는 유저 모드에서 직접 접근, 출력은 복사. 대용량 입력에 적합.

#### 학습 목표

1. `DeviceIoControl`로 `IOCTL_DISK_GET_DRIVE_GEOMETRY`를 호출하고 결과를 파싱할 수 있다.
2. IOCTL 코드 구조를 이해하고 `CTL_CODE`로 코드를 생성할 수 있다.

#### 핵심 API

`CreateFile("\\\\.\\\\.\\...")` · `DeviceIoControl`  
`IOCTL_DISK_GET_DRIVE_GEOMETRY` · `IOCTL_STORAGE_QUERY_PROPERTY`

#### 실습 절차

1. `\\.\PhysicalDrive0`을 `GENERIC_READ | FILE_SHARE_READ | FILE_SHARE_WRITE`로 열기 (관리자 권한 필요).
2. `IOCTL_DISK_GET_DRIVE_GEOMETRY` → `DISK_GEOMETRY` 수신 → 섹터 크기·전체 용량 출력.
3. `IOCTL_STORAGE_QUERY_PROPERTY(StorageDeviceProperty)` → 드라이브 제조사·모델 출력.
4. `CTL_CODE(FILE_DEVICE_DISK, 0x0007, METHOD_BUFFERED, FILE_ANY_ACCESS)` 값 수동 계산 확인.

#### 예상 출력 예시

```
PhysicalDrive0:
  Sectors/Track: 63   Bytes/Sector: 512
  Total: ~500GB
  Model: Samsung SSD 860 EVO 500GB
```

#### 함정 & 주의사항

- 관리자 권한 필요. UAC 승격 확인.
- `STORAGE_DEVICE_DESCRIPTOR` 가변 길이 — 크기 먼저 쿼리 후 재할당.
- `ERROR_INSUFFICIENT_BUFFER` = 출력 버퍼 부족.

#### 확인 질문

1. `METHOD_BUFFERED`와 `METHOD_IN_DIRECT`의 차이와 적합한 사용 사례는?
2. `DeviceIoControl`을 `OVERLAPPED`와 함께 사용하는 이점은?
3. 드라이버 없이 `DeviceIoControl`로 접근 가능한 다른 내장 인터페이스 두 가지는?

#### 확장 과제

`IOCTL_DISK_GET_PARTITION_INFO_EX`로 파티션 정보를 조회하고 `diskpart list partition` 출력과 비교하세요.

---

## Phase 6: IPC

---

### 25_Pipe_Advanced

**난이도** ★★★ | **예상 시간** 5h | **선행 모듈** Phase 5 완료

#### 이론 배경

Named Pipe는 서버-클라이언트 모델이다. `PIPE_UNLIMITED_INSTANCES`로 다중 인스턴스를 지원하고, `FILE_FLAG_OVERLAPPED`로 비동기 모드에서 IOCP와 통합된다.

파이프 인스턴스 상태 머신:
```
[CONNECTING] → ConnectNamedPipe 완료 → [READING] → 데이터 처리 → [WRITING]
     ↑                                                               │
     └──── DisconnectNamedPipe + 새 인스턴스 생성 ──────────────────┘
```

#### 학습 목표

1. Overlapped Named Pipe 서버로 3개 이상 클라이언트를 동시 처리할 수 있다.
2. IOCP 기반 완료 디스패치를 파이프 서버에 통합할 수 있다.
3. 클라이언트 연결 해제 후 인스턴스를 재사용할 수 있다.

#### 핵심 API

`CreateNamedPipe` · `ConnectNamedPipe` · `DisconnectNamedPipe`  
`ImpersonateNamedPipeClient` · `TransactNamedPipe`

#### 실습 절차

1. `\\.\pipe\OsPractice` 서버 → 동기 클라이언트 3개 순차 연결·에코 테스트.
2. `FILE_FLAG_OVERLAPPED` + IOCP → 3개 클라이언트 동시 처리.
3. 연결 해제 후 `DisconnectNamedPipe` + `ConnectNamedPipe`로 인스턴스 재사용.
4. `ImpersonateNamedPipeClient` → 클라이언트 사용자 이름 출력.

#### 예상 출력 예시

```
[Server] Client 1 connected (instance 0)
[Server] Client 2 connected (instance 1)
[Server] Echoed 'Hello' to client 1
[Server] Client 1 disconnected — instance 0 recycled
```

#### 함정 & 주의사항

- `ConnectNamedPipe` 반환 `ERROR_PIPE_CONNECTED` = 클라이언트가 이미 연결됨. 오류 아님.
- `DisconnectNamedPipe` 없이 핸들만 닫으면 클라이언트가 `ReadFile`에서 멈출 수 있다.
- 비동기 `ConnectNamedPipe`에서 `ERROR_IO_PENDING`이 아닌 경우 처리 필요.

#### 확인 질문

1. `PIPE_READMODE_MESSAGE` vs `PIPE_READMODE_BYTE`의 차이는?
2. Named Pipe와 Anonymous Pipe의 방향성·접근 범위 차이는?
3. `ImpersonateNamedPipeClient`를 사용하는 보안 이유는?

#### 확장 과제

`SECURITY_ATTRIBUTES`로 DACL을 적용해 특정 SID만 연결 가능하도록 제한하고 `ERROR_ACCESS_DENIED` 반환을 확인하세요.

---

### 26_SharedMemory

**난이도** ★★☆ | **예상 시간** 4h | **선행 모듈** 12, 15

#### 이론 배경

이름 있는 공유 메모리는 `CreateFileMapping(INVALID_HANDLE_VALUE, ..., "Local\\MySharedSeg")`으로 페이지 파일 기반 섹션을 생성한다. 두 프로세스가 각자 `MapViewOfFile`하면 같은 물리 페이지를 공유한다. 공유 메모리 자체에는 동기화가 없으므로 이름 있는 Mutex로 접근을 보호해야 한다.

포인터를 공유 메모리 안에 저장하면 VA가 프로세스마다 다르므로 **반드시 오프셋**을 사용해야 한다.

#### 학습 목표

1. 서버·클라이언트 두 프로세스가 공유 메모리로 데이터를 교환할 수 있다.
2. 이름 있는 Mutex로 공유 메모리 접근을 보호할 수 있다.
3. 공유 메모리 위에 링 버퍼를 구현할 수 있다.

#### 핵심 API

`CreateFileMapping(INVALID_HANDLE_VALUE)` · `OpenFileMapping` · `MapViewOfFile`  
`CreateMutex` · `OpenMutex` · `WaitForSingleObject` · `ReleaseMutex`

#### 실습 절차

1. 서버: `CreateFileMapping` + `MapViewOfFile` → 1초마다 타임스탬프 쓰기.
2. 클라이언트: `OpenFileMapping` + `MapViewOfFile` → 읽어 출력.
3. 이름 있는 Mutex로 쓰기/읽기 직렬화.
4. 링 버퍼(write_idx, read_idx, capacity) 서버 push + 클라이언트 pop.

#### 예상 출력 예시

```
[Server]  view @ 0x1A0`00001000  wrote tick=1234567
[Client]  view @ 0x2B1`00003000  read  tick=1234567
[RingBuf] 1000 pushed / 1000 popped — no loss
```

#### 함정 & 주의사항

- 포인터를 공유 메모리에 저장 금지 — VA가 프로세스마다 다르다. 오프셋 사용.
- `"Global\\"` 네임스페이스는 세션 0(서비스)과 다른 세션 간 공유 시 사용.
- `OpenFileMapping` 실패 = 서버가 아직 섹션 미생성 — 재시도 로직 필요.

#### 확인 질문

1. 공유 메모리에 절대 포인터 대신 오프셋을 저장해야 하는 이유는?
2. `"Global\\"` vs `"Local\\"` 네임스페이스의 차이는?
3. 공유 메모리 vs Named Pipe의 대용량 데이터 전달 장단점은?

#### 확장 과제

링 버퍼를 MPMC(다중 생산자·다중 소비자) 지원으로 확장하세요. 이름 있는 세마포 두 개(emptySlots, fullSlots)로 Mutex 없이 동기화합니다.

---

### 27_Mailslot

**난이도** ★★☆ | **예상 시간** 3h | **선행 모듈** 25, 26

#### 이론 배경

Mailslot은 단방향 브로드캐스트 IPC다. ACK·흐름 제어가 없으며 버퍼 초과 시 메시지가 소실된다. 로컬 브로드캐스트: `\\*\mailslot\name`으로 쓰면 동일 도메인의 모든 수신자에게 전달된다.

#### 학습 목표

1. Mailslot 수신 서버와 송신 클라이언트를 별도 프로세스로 구현할 수 있다.
2. Named Pipe·공유 메모리·Mailslot 세 IPC 방식의 특성을 정량적으로 비교할 수 있다.

#### 핵심 API

`CreateMailslot` · `GetMailslotInfo` · `SetMailslotInfo`  
`CreateFile("\\\\.\\\\ mailslot\\...")` · `WriteFile` · `ReadFile`

#### 실습 절차

1. 수신: `CreateMailslot("\\\\.\\mailslot\\OsPractice", 512, MAILSLOT_WAIT_FOREVER, NULL)` → `ReadFile` 루프.
2. 송신: `CreateFile("\\\\.\\mailslot\\OsPractice", GENERIC_WRITE, ...)` → 10개 메시지 `WriteFile`.
3. `GetMailslotInfo`로 대기 메시지 수·최대 크기 출력.
4. 수신자 없는 상태에서 `WriteFile` 동작 확인.

#### 예상 출력 예시

```
[Sender]   Sent: "msg-001"
[Receiver] Received: "msg-001"
[Info] Pending: 3  MaxSize: 512
```

#### 함정 & 주의사항

- 이름 접두사: `\\.\mailslot\` — Named Pipe의 `\\.\pipe\`와 혼동 금지.
- TCP/IP 네트워크에서 작동 안 함 — 동일 호스트 또는 NetBIOS 전용.
- 송신 측 `CloseHandle` 후에도 수신 측 `ReadFile`은 블로킹 유지 (Pipe와 다름).

#### 확인 질문

1. Mailslot이 Named Pipe보다 브로드캐스트 알림에 적합한 이유는?
2. 메시지가 소실될 수 있는 두 가지 시나리오는?
3. Named Pipe·공유 메모리·Mailslot 중 가장 낮은 지연을 제공하는 방식과 그 이유는?

#### 확장 과제

세 IPC 방식으로 각각 1KB 메시지를 1000회 왕복 전송하고 **평균 지연·최대 지연·CPU 오버헤드**를 표로 비교하세요. 사용 사례별 권장 방식을 정리합니다.
