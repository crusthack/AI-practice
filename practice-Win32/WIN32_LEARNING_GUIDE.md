# Win32 프로젝트 통합 학습 교안

이 문서는 `Win32.slnx`에 포함된 모든 프로젝트를 순서대로 학습하기 위한 통합 교안입니다. 각 항목은 목표, 실행 관찰 포인트, 핵심 API, 확장 과제로 구성되어 있습니다.

## 공통 실습 절차

1. 프로젝트를 단독 실행합니다.
2. 창, 메시지 박스, 파일, 네트워크 요청, 프로세스 생성 등 관찰 가능한 결과를 확인합니다.
3. `main.cpp`에서 API 호출 순서를 따라갑니다.
4. 실패 경로에서 어떤 오류 코드가 나오는지 확인합니다.
5. 리소스 정리 함수가 어디서 호출되는지 확인합니다.
6. 확장 과제를 작게 구현하고 전체 솔루션을 다시 빌드합니다.

## Legacy 프로젝트

### 1. Hello Window

- 목표: 기존 Win32 창 생성, 컨트롤 배치, `WM_COMMAND` 처리 흐름을 복습합니다.
- 관찰: 버튼 클릭, 텍스트 입력, 메시지 박스 표시 흐름을 확인합니다.
- 핵심 API: `CreateWindow`, `MessageBoxW`, `GetWindowText`, `SetWindowText`.
- 확장 과제: 컨트롤 크기를 창 크기 변경에 맞춰 재배치하고, 입력값 검증을 추가합니다.

### 2. COM

- 목표: COM 초기화와 파일 열기 대화상자 객체 사용을 익힙니다.
- 관찰: 파일 선택 후 경로가 표시되는지 확인합니다.
- 핵심 API: `CoInitializeEx`, `CoCreateInstance`, `IFileOpenDialog`, `IShellItem`, `Release`.
- 확장 과제: WRL `ComPtr`로 수동 `Release`를 제거하고 다중 파일 선택을 지원합니다.

### 3. Graphics

- 목표: 기존 그래픽 프로젝트의 빌드 설정과 그래픽 학습 위치를 확인합니다.
- 관찰: 프로젝트 구성, 링커 설정, 소스 포함 여부를 확인합니다.
- 핵심 API: 프로젝트 내용에 따라 GDI 또는 그래픽 라이브러리 호출을 추적합니다.
- 확장 과제: Phase 1의 `03_GDI_Painting` 또는 Phase 4의 `24_D2D_DWrite`와 역할을 비교합니다.

### 4. ProcessInput

- 목표: 자식 프로세스 실행과 파이프 기반 입출력 리다이렉션을 학습합니다.
- 관찰: 자식 프로세스의 출력이 부모 프로세스에서 읽히는지 확인합니다.
- 핵심 API: `CreatePipe`, `CreateProcessW`, `ReadFile`, `CloseHandle`.
- 확장 과제: stderr 리다이렉션과 timeout 기반 프로세스 종료를 추가합니다.

## Phase 1: Win32 UI 기초

### 01_WindowBasics

- 목표: Win32 프로그램의 최소 창 생성 구조를 이해합니다.
- 관찰: 창 클래스 등록, 창 생성, 메시지 루프가 어떤 순서로 실행되는지 확인합니다.
- 핵심 API: `WNDCLASSEX`, `RegisterClassEx`, `CreateWindowEx`, `ShowWindow`, `GetMessage`.
- 확장 과제: 창 스타일을 바꾸고 최소/최대 크기 제한을 추가합니다.

### 02_MessageLoop

- 목표: 메시지 루프와 `WndProc`의 역할을 익힙니다.
- 관찰: 키보드, 마우스, 타이머 이벤트가 화면 상태를 어떻게 바꾸는지 확인합니다.
- 핵심 API: `WM_KEYDOWN`, `WM_LBUTTONDOWN`, `SetTimer`, `KillTimer`, `InvalidateRect`.
- 확장 과제: 상태를 여러 개 추가하고 `WM_SIZE`에서 레이아웃을 갱신합니다.

### 03_GDI_Painting

- 목표: GDI 그리기와 페인트 메시지 처리 방식을 학습합니다.
- 관찰: `WM_PAINT`가 언제 발생하고 더블 버퍼링이 깜빡임을 어떻게 줄이는지 확인합니다.
- 핵심 API: `BeginPaint`, `EndPaint`, `CreateCompatibleDC`, `BitBlt`, `DeleteObject`.
- 확장 과제: 마우스로 도형을 추가하고 색상 선택 기능을 붙입니다.

### 04_Controls

- 목표: 기본 컨트롤도 모두 `HWND`라는 점을 이해합니다.
- 관찰: 버튼, 에디트, 리스트, 콤보박스 사이의 데이터 이동을 확인합니다.
- 핵심 API: `CreateWindow`, `SendMessage`, `GetWindowText`, `SetWindowText`, `WM_COMMAND`.
- 확장 과제: 입력 항목 삭제, 정렬, 선택 상태 표시를 추가합니다.

### 05_Dialogs

- 목표: 리소스 기반 대화상자와 공용 대화상자를 학습합니다.
- 관찰: modal dialog와 파일 선택 대화상자의 반환 흐름을 확인합니다.
- 핵심 API: `DialogBox`, `EndDialog`, `GetDlgItemText`, `GetOpenFileName`.
- 확장 과제: 저장 대화상자와 폴더 선택 대화상자를 추가합니다.

### 06_Menus_Accel

- 목표: 메뉴, 팝업 메뉴, 단축키 테이블의 관계를 익힙니다.
- 관찰: 메뉴 클릭과 accelerator 입력이 같은 명령으로 들어오는지 확인합니다.
- 핵심 API: `CreateMenu`, `AppendMenu`, `TrackPopupMenu`, `LoadAccelerators`, `TranslateAccelerator`.
- 확장 과제: 최근 파일 목록 메뉴와 체크 메뉴 상태를 추가합니다.

## Phase 2: 고급 UI와 Shell

### 07_CommonControls

- 목표: `comctl32` 기반 고급 컨트롤을 초기화하고 사용합니다.
- 관찰: ListView, TreeView, StatusBar가 `WM_NOTIFY`와 어떻게 연결되는지 확인합니다.
- 핵심 API: `InitCommonControlsEx`, `ListView_*`, `TreeView_*`, `StatusBar_*`.
- 확장 과제: ListView 컬럼 정렬과 TreeView 동적 로딩을 추가합니다.

### 08_OwnerDraw

- 목표: 컨트롤의 기본 렌더링을 직접 그리는 방식을 익힙니다.
- 관찰: 선택 상태, 포커스 상태, 배경색이 `WM_DRAWITEM`에서 어떻게 처리되는지 확인합니다.
- 핵심 API: `WM_DRAWITEM`, `WM_MEASUREITEM`, `DRAWITEMSTRUCT`, `ODS_SELECTED`.
- 확장 과제: 아이콘, 텍스트 정렬, hover 상태 렌더링을 추가합니다.

### 09_ShellIntegration

- 목표: Shell API를 이용해 데스크톱 환경과 통합합니다.
- 관찰: 트레이 아이콘, 파일 드롭, Shell 실행 결과를 확인합니다.
- 핵심 API: `Shell_NotifyIcon`, `DragAcceptFiles`, `WM_DROPFILES`, `ShellExecute`.
- 확장 과제: `IDropTarget` 기반 COM drag/drop과 컨텍스트 메뉴를 추가합니다.

## Phase 3: 시스템 프로그래밍

### 10_FileIO

- 목표: Win32 파일 생성, 쓰기, 복사, 읽기 흐름을 익힙니다.
- 관찰: 임시 파일 생성과 복사 결과를 확인합니다.
- 핵심 API: `CreateFileW`, `WriteFile`, `ReadFile`, `CopyFileW`, `CloseHandle`.
- 확장 과제: Overlapped I/O와 진행률 표시를 추가합니다.

### 11_ProcessThread

- 목표: 프로세스와 스레드의 생명주기를 학습합니다.
- 관찰: 자식 프로세스 stdout 리다이렉션과 스레드 종료 코드를 확인합니다.
- 핵심 API: `CreateProcessW`, `CreateThread`, `WaitForSingleObject`, `GetExitCodeProcess`.
- 확장 과제: timeout, 강제 종료, 스레드 풀 방식 작업 큐를 추가합니다.

### 12_Synchronization

- 목표: 동기화 객체별 사용 목적을 구분합니다.
- 관찰: 여러 스레드가 공유 상태를 어떻게 보호하는지 확인합니다.
- 핵심 API: `CRITICAL_SECTION`, `CreateEvent`, `CreateSemaphore`, `WaitForMultipleObjects`.
- 확장 과제: 생산자/소비자 큐와 종료 이벤트를 구현합니다.

### 13_Memory

- 목표: Heap, Virtual Memory, File Mapping의 차이를 이해합니다.
- 관찰: 할당된 주소, 보호 속성, 매핑된 메모리 내용을 확인합니다.
- 핵심 API: `HeapCreate`, `HeapAlloc`, `VirtualAlloc`, `VirtualQuery`, `CreateFileMapping`.
- 확장 과제: `VirtualProtect`로 페이지 보호를 바꾸고 예외를 관찰합니다.

### 14_Registry

- 목표: 레지스트리 키와 값의 CRUD 흐름을 익힙니다.
- 관찰: HKCU 아래 값이 쓰이고 다시 읽히는지 확인합니다.
- 핵심 API: `RegCreateKeyEx`, `RegSetValueEx`, `RegQueryValueEx`, `RegCloseKey`.
- 확장 과제: `RegNotifyChangeKeyValue`로 변경 감시를 추가합니다.

### 15_ServiceControl

- 목표: Service Control Manager와 서비스 열거 방식을 학습합니다.
- 관찰: 서비스 목록, 상태, 필요한 버퍼 크기 처리 방식을 확인합니다.
- 핵심 API: `OpenSCManager`, `EnumServicesStatusEx`, `CloseServiceHandle`.
- 확장 과제: 특정 서비스 시작, 중지, 상태 새로 고침 기능을 추가합니다.

## Phase 4: COM 및 Windows 런타임

### 16_COM_Basics

- 목표: COM 초기화, 객체 생성, 인터페이스 수명 관리를 익힙니다.
- 관찰: COM 초기화 결과와 Shell 객체 생성 결과를 확인합니다.
- 핵심 API: `CoInitializeEx`, `CoCreateInstance`, `IUnknown::Release`, `CoUninitialize`.
- 확장 과제: WRL `ComPtr`를 도입하고 `QueryInterface` 결과를 출력합니다.

### 17_COM_Advanced

- 목표: COM apartment와 마샬링 개념을 확인합니다.
- 관찰: 인터페이스를 stream에 marshal/unmarshal하는 흐름을 추적합니다.
- 핵심 API: `CoMarshalInterThreadInterfaceInStream`, `CoGetInterfaceAndReleaseStream`.
- 확장 과제: STA 스레드와 MTA 스레드를 분리해 호출 결과를 비교합니다.

### 18_ATL_WRL

- 목표: WRL `ComPtr`로 COM 포인터 수명을 관리합니다.
- 관찰: 수동 `Release` 없이 객체가 정리되는 흐름을 확인합니다.
- 핵심 API: `Microsoft::WRL::ComPtr`, `CoCreateInstance`.
- 확장 과제: 기존 `2. COM` 예제를 `ComPtr` 기반으로 다시 작성합니다.

### 19_Automation

- 목표: Automation 데이터 타입을 익힙니다.
- 관찰: `VARIANT`, `BSTR` 초기화와 정리 흐름을 확인합니다.
- 핵심 API: `VariantInit`, `VariantClear`, `SysAllocString`, `SysFreeString`.
- 확장 과제: 실제 `IDispatch::Invoke` 호출 예제를 추가합니다.

### 20_WinRT_Basics

- 목표: WinRT ABI 초기화와 문자열 타입을 이해합니다.
- 관찰: `RoInitialize`와 HSTRING 생성/해제 흐름을 확인합니다.
- 핵심 API: `RoInitialize`, `WindowsCreateString`, `WindowsDeleteString`.
- 확장 과제: WinRT activation factory 조회를 추가합니다.

### 21_DCOM_RPC

- 목표: COM 보안 초기화와 RPC/DCOM 개념을 학습합니다.
- 관찰: `CoInitializeSecurity` 호출 조건과 HRESULT를 확인합니다.
- 핵심 API: `CoInitializeEx`, `CoInitializeSecurity`, `CoCreateInstanceEx`.
- 확장 과제: 로컬 서버 COM 구성 또는 RPC binding 문자열 실습을 추가합니다.

### 22_DirectShow

- 목표: COM 기반 미디어 그래프의 기본 구조를 익힙니다.
- 관찰: Filter Graph 객체 생성과 `IMediaControl` 조회를 확인합니다.
- 핵심 API: `IGraphBuilder`, `IMediaControl`, `CoCreateInstance`.
- 확장 과제: 그래프 안의 필터 목록을 열거해 표시합니다.

### 23_MF_WIC

- 목표: Media Foundation과 WIC 초기화 흐름을 익힙니다.
- 관찰: MF startup과 WIC factory 생성 결과를 확인합니다.
- 핵심 API: `MFStartup`, `MFShutdown`, `CoCreateInstance`, `IWICImagingFactory`.
- 확장 과제: WIC로 실제 이미지 파일을 디코딩하고 크기/픽셀 포맷을 표시합니다.

### 24_D2D_DWrite

- 목표: Direct2D와 DirectWrite 팩터리 생성 방식을 학습합니다.
- 관찰: 그래픽 팩터리와 텍스트 포맷 생성 결과를 확인합니다.
- 핵심 API: `D2D1CreateFactory`, `DWriteCreateFactory`, `IDWriteTextFormat`.
- 확장 과제: HWND render target으로 텍스트와 도형을 실제 렌더링합니다.

## Phase 5: 보안, 네트워크, IPC, 진단

### 25_Security

- 목표: Windows 보안 토큰과 SID를 이해합니다.
- 관찰: 현재 프로세스 토큰에서 사용자 SID가 조회되는지 확인합니다.
- 핵심 API: `OpenProcessToken`, `GetTokenInformation`, `LookupAccountSid`.
- 확장 과제: 무결성 수준과 privilege 목록을 표시합니다.

### 26_Impersonation

- 목표: 토큰 복제와 가장의 기본 흐름을 학습합니다.
- 관찰: 현재 프로세스 토큰을 impersonation token으로 복제하는 과정을 확인합니다.
- 핵심 API: `DuplicateToken`, `ImpersonateLoggedOnUser`, `RevertToSelf`.
- 확장 과제: Named Pipe client 가장 예제를 추가합니다.

### 27_Cryptography

- 목표: CNG 기반 난수 생성과 SHA-256 해시 계산을 익힙니다.
- 관찰: NTSTATUS, 16바이트 난수, SHA-256 hex 결과를 확인합니다.
- 핵심 API: `BCryptGenRandom`, `BCryptOpenAlgorithmProvider`, `BCryptCreateHash`, `BCryptHashData`.
- 확장 과제: 파일을 읽어 SHA-256을 계산하고 기대 해시와 비교합니다.

### 28_Winsock2

- 목표: loopback TCP 소켓의 생성, 연결, 송수신 흐름을 익힙니다.
- 관찰: listener, client, accepted socket 간 echo 왕복 결과를 확인합니다.
- 핵심 API: `WSAStartup`, `socket`, `bind`, `listen`, `connect`, `accept`, `send`, `recv`.
- 확장 과제: 서버 스레드와 다중 클라이언트 처리를 추가합니다.

### 29_NamedPipe_MM

- 목표: Named Pipe와 메모리 매핑 IPC를 비교합니다.
- 관찰: pipe 생성, client 연결, mapping 생성과 view 접근 흐름을 확인합니다.
- 핵심 API: `CreateNamedPipe`, `CreateFileW`, `CreateFileMapping`, `MapViewOfFile`.
- 확장 과제: 서버/클라이언트 스레드 기반 request/reply 패턴을 구현합니다.

### 30_Mailslot_LPC

- 목표: 단방향 메시지 IPC인 Mailslot을 학습합니다.
- 관찰: mailslot 생성, writer open, message write 흐름을 확인합니다.
- 핵심 API: `CreateMailslot`, `CreateFileW`, `WriteFile`, `GetMailslotInfo`.
- 확장 과제: 여러 메시지를 읽는 loop와 timeout 설정을 추가합니다.

### 31_WinHTTP

- 목표: WinHTTP를 이용한 시스템 HTTP 클라이언트 흐름을 익힙니다.
- 관찰: `https://example.com/`에 HEAD 요청을 보내고 HTTP 상태 코드를 확인합니다.
- 핵심 API: `WinHttpOpen`, `WinHttpConnect`, `WinHttpOpenRequest`, `WinHttpSendRequest`, `WinHttpReceiveResponse`.
- 확장 과제: GET 요청으로 body 일부를 읽고 proxy/TLS 옵션을 표시합니다.

### 32_ETW_Perf

- 목표: ETW와 성능 카운터의 역할을 이해합니다.
- 관찰: provider/session/counter 개념이 어떤 API에 연결되는지 확인합니다.
- 핵심 API: `EventRegister`, `EventWrite`, `PdhOpenQuery`, `PdhCollectQueryData`.
- 확장 과제: PDH CPU 카운터를 1초 간격으로 샘플링합니다.

### 33_Debugging

- 목표: Windows 디버깅 API와 미니덤프 개념을 학습합니다.
- 관찰: 디버깅 API, 예외 처리, 덤프 생성 API의 역할을 구분합니다.
- 핵심 API: `DebugActiveProcess`, `WaitForDebugEvent`, `MiniDumpWriteDump`, `SymInitialize`.
- 확장 과제: 현재 프로세스 미니덤프 생성 기능을 추가합니다.

## Phase 6: 고급 심화

### 34_DLL_Injection

- 목표: 원격 프로세스 메모리 조작과 DLL 로딩 개념을 안전하게 학습합니다.
- 관찰: 현재 프로세스 대상 메모리 할당과 API 주소 조회 결과를 확인합니다.
- 핵심 API: `VirtualAllocEx`, `WriteProcessMemory`, `LoadLibraryW`, `GetProcAddress`.
- 확장 과제: 외부 프로세스 대상 동작은 VM에서만 실험하고, 이 저장소 예제는 안전한 개념 검증으로 유지합니다.

### 35_Hook

- 목표: low-level hook 설치 개념과 콜백 흐름을 이해합니다.
- 관찰: hook handle 생성, 이벤트 콜백, unhook 정리 경로를 확인합니다.
- 핵심 API: `SetWindowsHookEx`, `CallNextHookEx`, `UnhookWindowsHookEx`.
- 확장 과제: 키 입력 내용을 저장하지 말고 이벤트 카운터만 UI에 표시합니다.

### 36_KernelInterface

- 목표: 사용자 모드에서 장치 파일을 열고 IOCTL을 보내는 형태를 익힙니다.
- 관찰: 장치 경로 실패 시 오류 코드와 권한 조건을 확인합니다.
- 핵심 API: `CreateFileW`, `DeviceIoControl`, `CloseHandle`.
- 확장 과제: WDK 샘플 드라이버와 연결되는 IOCTL 상수를 문서화합니다.

### 37_Fiber_UMS

- 목표: Fiber 기반 협력적 실행 모델을 이해합니다.
- 관찰: thread가 fiber로 변환되고 fiber 간 전환되는 흐름을 확인합니다.
- 핵심 API: `ConvertThreadToFiber`, `CreateFiber`, `SwitchToFiber`, `DeleteFiber`.
- 확장 과제: 작은 fiber scheduler와 작업 큐를 구현합니다.

### 38_JobObject

- 목표: Job Object로 프로세스 그룹 제한과 회계 정보를 관리합니다.
- 관찰: kill-on-close, process memory limit, 자식 프로세스 할당, accounting 결과를 확인합니다.
- 핵심 API: `CreateJobObject`, `SetInformationJobObject`, `AssignProcessToJobObject`, `QueryInformationJobObject`.
- 확장 과제: CPU rate limit과 UI 기반 종료 버튼을 추가합니다.

### 39_Minifilter

- 목표: Minifilter 사용자 모드 통신 포트 연결 개념을 이해합니다.
- 관찰: Filter Manager DLL 로딩, 통신 포트 연결 실패 경로, HRESULT를 확인합니다.
- 핵심 API: `LoadLibrary`, `GetProcAddress`, `FilterConnectCommunicationPort`.
- 확장 과제: WDK/VM 환경에서 실제 minifilter 샘플과 통신하도록 확장합니다.

## Phase 7: Capstone

### 40_ProcessExplorer

- 목표: 앞선 시스템 API 주제를 통합해 간이 Process Explorer를 구현합니다.
- 관찰: 프로세스 목록, PID, 부모 PID, 스레드 수, 미니덤프 생성 결과를 확인합니다.
- 핵심 API: `CreateToolhelp32Snapshot`, `Process32First`, `Process32Next`, `OpenProcess`, `MiniDumpWriteDump`.
- 확장 과제: 모듈 탭, 토큰 탭, 메모리 맵 탭, 서비스 매핑, 핸들 열거를 추가합니다.

## 최종 캡스톤 확장 순서

1. `40_ProcessExplorer`에 선택 프로세스의 모듈 목록을 추가합니다.
2. `25_Security` 내용을 가져와 토큰 사용자와 권한 탭을 추가합니다.
3. `13_Memory` 내용을 가져와 `VirtualQueryEx` 기반 메모리 맵을 추가합니다.
4. `15_ServiceControl` 내용을 가져와 서비스와 프로세스 ID를 매핑합니다.
5. `33_Debugging` 내용을 확장해 미니덤프 옵션을 선택할 수 있게 합니다.
6. `32_ETW_Perf` 내용을 붙여 프로세스 시작/종료 이벤트를 실시간으로 표시합니다.
