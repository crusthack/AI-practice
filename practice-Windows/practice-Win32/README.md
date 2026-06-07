# Win32 시스템 프로그래밍 학습 저장소

이 저장소는 Win32 API를 출발점으로 Windows 데스크톱, 시스템 프로그래밍, COM, 보안, 네트워크, IPC, 디버깅, 고급 시스템 실험까지 단계적으로 학습하기 위한 Visual C++ 실습 모음입니다.

전체 솔루션은 `Win32.slnx`이며, 현재 44개 프로젝트가 등록되어 있습니다. 초기 예제 4개와 Phase 1~7의 로드맵 예제 40개로 구성되어 있습니다.

## 빠른 시작

Visual Studio 또는 MSBuild가 설치된 환경에서 다음 명령으로 전체 프로젝트를 빌드합니다.

```powershell
MSBuild Win32.slnx /p:Configuration=Debug /p:Platform=x64
```

빌드 산출물은 기본적으로 `x64/Debug` 아래에 생성됩니다.

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 빌드 방법, 전체 목차 |
| `PROJECT_REVIEW.md` | 전체 프로젝트 검토 결과와 고도화 기록 |
| `WIN32_LEARNING_GUIDE.md` | 각 프로젝트별 학습 교안, 실습 절차, 확장 과제 |

## 전체 로드맵

| 구간 | 프로젝트 | 학습 주제 |
| --- | --- | --- |
| Legacy | `1. Hello Window` ~ `4. ProcessInput` | 기존 Win32, COM, 그래픽, 프로세스 입출력 기초 |
| Phase 1 | `01_WindowBasics` ~ `06_Menus_Accel` | 창, 메시지 루프, GDI, 컨트롤, 대화상자, 메뉴 |
| Phase 2 | `07_CommonControls` ~ `09_ShellIntegration` | 고급 컨트롤, Owner Draw, Shell 통합 |
| Phase 3 | `10_FileIO` ~ `15_ServiceControl` | 파일, 프로세스, 스레드, 동기화, 메모리, 레지스트리, 서비스 |
| Phase 4 | `16_COM_Basics` ~ `24_D2D_DWrite` | COM, WRL, Automation, WinRT, DirectShow, WIC, Direct2D |
| Phase 5 | `25_Security` ~ `33_Debugging` | 보안, 가장, 암호화, Winsock, IPC, WinHTTP, ETW, 디버깅 |
| Phase 6 | `34_DLL_Injection` ~ `39_Minifilter` | Injection 개념, Hook, 커널 인터페이스, Fiber, Job Object, Minifilter |
| Phase 7 | `40_ProcessExplorer` | 통합 시스템 도구 캡스톤 |

## 프로젝트 목차

### Legacy

- `1. Hello Window`: 기존 Win32 창, 컨트롤, 메시지 박스 실습
- `2. COM`: 기존 COM 파일 열기 대화상자 실습
- `3. Graphics`: 기존 그래픽 프로젝트
- `4. ProcessInput`: 기존 프로세스 실행과 파이프 입출력 실습

### Phase 1: Win32 UI 기초

- `01_WindowBasics`: 창 클래스 등록, 창 생성, 기본 메시지 루프
- `02_MessageLoop`: 키보드, 마우스, 타이머 메시지 처리
- `03_GDI_Painting`: GDI 그리기와 더블 버퍼링
- `04_Controls`: Button, Edit, ListBox, ComboBox 연동
- `05_Dialogs`: 리소스 기반 대화상자와 공용 파일 대화상자
- `06_Menus_Accel`: 메뉴바, 팝업 메뉴, 단축키 테이블

### Phase 2: 고급 UI와 Shell

- `07_CommonControls`: ListView, TreeView, StatusBar
- `08_OwnerDraw`: Owner-draw 렌더링
- `09_ShellIntegration`: 트레이 아이콘, 파일 드롭, Shell 실행

### Phase 3: 시스템 프로그래밍

- `10_FileIO`: Win32 파일 생성, 복사, 읽기
- `11_ProcessThread`: 자식 프로세스, stdout 리다이렉션, 스레드
- `12_Synchronization`: Critical Section, Event, Semaphore
- `13_Memory`: Heap, VirtualAlloc, File Mapping
- `14_Registry`: HKCU 기반 설정 저장과 조회
- `15_ServiceControl`: Service Control Manager 조회

### Phase 4: COM 및 Windows 런타임

- `16_COM_Basics`: COM 초기화와 Shell COM 객체
- `17_COM_Advanced`: COM 마샬링 개념
- `18_ATL_WRL`: WRL `ComPtr` 기반 COM 포인터 관리
- `19_Automation`: `VARIANT`, `BSTR`, Automation 기초
- `20_WinRT_Basics`: WinRT 초기화와 HSTRING
- `21_DCOM_RPC`: COM 보안 초기화와 RPC 개념
- `22_DirectShow`: Filter Graph 생성
- `23_MF_WIC`: Media Foundation 및 WIC 초기화
- `24_D2D_DWrite`: Direct2D, DirectWrite 팩터리 생성

### Phase 5: 보안, 네트워크, IPC, 진단

- `25_Security`: 프로세스 토큰과 SID 조회
- `26_Impersonation`: 토큰 복제와 가장 흐름
- `27_Cryptography`: CNG 난수와 SHA-256 해시
- `28_Winsock2`: 로컬 TCP echo 송수신
- `29_NamedPipe_MM`: Named Pipe와 파일 매핑
- `30_Mailslot_LPC`: Mailslot 단방향 IPC
- `31_WinHTTP`: HTTPS HEAD 요청과 상태 코드 조회
- `32_ETW_Perf`: ETW/PDH 진단 API 개요
- `33_Debugging`: Debug API와 MiniDump 개요

### Phase 6: 고급 심화

- `34_DLL_Injection`: 원격 메모리 쓰기와 LoadLibrary 개념
- `35_Hook`: Low-level hook 설치 개념
- `36_KernelInterface`: `DeviceIoControl` 호출 형태
- `37_Fiber_UMS`: Fiber 기반 협력적 실행
- `38_JobObject`: Job Object 제한, 프로세스 할당, 회계 정보 조회
- `39_Minifilter`: Filter Manager 통신 포트 연결 개념

### Phase 7: Capstone

- `40_ProcessExplorer`: Toolhelp 기반 프로세스 목록, ListView UI, 미니덤프 생성

## 권장 학습 방식

1. `WIN32_LEARNING_GUIDE.md`에서 해당 프로젝트의 목표와 관찰 포인트를 읽습니다.
2. Visual Studio에서 프로젝트를 단독 실행합니다.
3. `main.cpp`의 API 호출 순서와 리소스 정리 경로를 추적합니다.
4. `GetLastError`, `HRESULT`, `NTSTATUS`, Winsock error를 확인합니다.
5. Process Explorer, ProcMon, WinDbg, Event Viewer 같은 외부 도구로 실행 결과를 교차 검증합니다.
6. 각 프로젝트의 확장 과제를 하나씩 구현합니다.

## 현재 고도화 상태

최근 고도화에서 다음 항목이 개선되었습니다.

- `27_Cryptography`: CNG 상태 코드와 16진수 결과 표시
- `28_Winsock2`: 로컬 loopback TCP echo 실습
- `31_WinHTTP`: 실제 HTTPS 요청과 HTTP 상태 코드 조회
- `38_JobObject`: Job 제한, 자식 프로세스 할당, 회계 정보 조회

자세한 검토 내용은 `PROJECT_REVIEW.md`, 프로젝트별 교안은 `WIN32_LEARNING_GUIDE.md`를 참고합니다.
