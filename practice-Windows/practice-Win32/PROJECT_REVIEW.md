# 프로젝트 검토 및 고도화 기록

## 요약

현재 `Win32.slnx`에는 44개 프로젝트가 등록되어 있습니다. 기존 프로젝트 4개와 Phase 1~7 로드맵 프로젝트 40개가 함께 구성되어 있으며, 전체 학습 흐름은 Win32 UI 기초에서 시작해 시스템 프로그래밍, COM, 보안/네트워크/IPC, 고급 시스템 실험, 캡스톤 도구로 이어집니다.

## 전체 검토 결과

| 구간 | 상태 | 판단 |
| --- | --- | --- |
| Legacy | 기존 예제 유지 | 초기 실습 자료로 가치가 있으나 로드맵 프로젝트와 중복되는 부분이 있습니다. |
| Phase 1 | 양호 | Win32 창과 메시지 모델을 실제 UI로 확인할 수 있습니다. |
| Phase 2 | 양호 | 고급 컨트롤과 Shell 통합의 관찰 가능성이 좋습니다. |
| Phase 3 | 양호 | 시스템 리소스 조작 흐름이 비교적 실습형으로 구성되어 있습니다. |
| Phase 4 | 보강 필요 | 일부 예제가 API 초기화 결과 확인 수준입니다. 실제 COM 객체 조작, 미디어/이미지 처리로 확장할 여지가 큽니다. |
| Phase 5 | 일부 고도화 완료 | 암호화, Winsock, WinHTTP는 실제 결과를 관찰하도록 개선했습니다. |
| Phase 6 | 일부 고도화 완료 | 위험도가 높은 주제는 안전한 로컬 실험 위주로 유지해야 합니다. Job Object는 실습성을 높였습니다. |
| Phase 7 | 확장 가능 | Process Explorer 형태의 캡스톤으로 적절하며, 모듈/토큰/메모리/핸들 탭 확장이 다음 단계입니다. |

## 공통 개선 기준

| 기준 | 설명 |
| --- | --- |
| 관찰 가능성 | 단순 성공/실패 메시지보다 실제 데이터, 상태 코드, 파일, 프로세스 결과를 보여줍니다. |
| 에러 처리 | `GetLastError`, `HRESULT`, `NTSTATUS`, Winsock error를 표시합니다. |
| 리소스 관리 | `CloseHandle`, `Release`, `WinHttpCloseHandle`, `closesocket`, `BCryptCloseAlgorithmProvider` 같은 정리 경로를 빠뜨리지 않습니다. |
| 실습 완결성 | 준비, 실행, 검증, 정리 단계가 하나의 예제 안에서 드러나야 합니다. |
| 안전성 | Injection, Hook, 커널, Minifilter 예제는 현재 프로세스나 로컬 테스트 대상 중심으로 제한합니다. |

## 이번 고도화 내역

| 파일 | 변경 내용 |
| --- | --- |
| `README.md` | 저장소 개요, 목차, 문서 구조, 학습 방식 재정리 |
| `PROJECT_REVIEW.md` | 전체 프로젝트 검토와 개선 기준 재정리 |
| `WIN32_LEARNING_GUIDE.md` | 44개 프로젝트 전체에 대한 통합 학습 교안 추가 |
| `Phase5_Security_Net/27_Cryptography/main.cpp` | CNG 상태 코드와 난수/해시 hex 출력 추가 |
| `Phase5_Security_Net/28_Winsock2/main.cpp` | 로컬 TCP echo 송수신 실습으로 확장 |
| `Phase5_Security_Net/31_WinHTTP/main.cpp` | 실제 HTTPS HEAD 요청, 응답 수신, 상태 코드 조회 추가 |
| `Phase6_Advanced/38_JobObject/main.cpp` | Job 제한, 자식 프로세스 할당, 회계 정보 조회 추가 |

## Phase별 개선 후보

### Phase 1

- DPI awareness 추가
- 창 크기 변경 시 컨트롤 레이아웃 재배치
- GDI 객체 선택/복구 패턴을 RAII로 정리

### Phase 2

- ListView 컬럼 정렬
- TreeView 동적 노드 로딩
- `IDropTarget` 기반 COM Drag and Drop 구현

### Phase 3

- `10_FileIO`: Overlapped I/O와 IOCP 기반 복사로 확장
- `12_Synchronization`: 생산자/소비자 큐와 종료 신호 처리
- `15_ServiceControl`: 서비스 시작, 중지, 상태 갱신 기능 추가

### Phase 4

- `19_Automation`: 실제 `IDispatch::Invoke` 호출 예제 추가
- `22_DirectShow`: 그래프 필터 열거 출력
- `23_MF_WIC`: WIC로 이미지 디코딩 정보 표시
- `24_D2D_DWrite`: 실제 HWND 렌더 타깃으로 텍스트와 도형 그리기

### Phase 5

- `25_Security`: 무결성 수준과 권한 목록 표시
- `29_NamedPipe_MM`: 서버/클라이언트 스레드 기반 왕복 RPC 패턴
- `32_ETW_Perf`: PDH 카운터 샘플링과 ETW provider 등록
- `33_Debugging`: 현재 프로세스 미니덤프 생성과 심볼 초기화

### Phase 6

- `34_DLL_Injection`: 현재 프로세스 대상의 안전한 메모리/모듈 실험으로 유지
- `35_Hook`: hook 설치 후 이벤트 카운터를 UI로 표시
- `36_KernelInterface`: 실패 경로에서 장치 이름, 오류 코드, 권한 조건을 명확히 표시
- `39_Minifilter`: VM/WDK 요구사항과 사용자 모드 통신 절차 문서화

### Phase 7

- 프로세스별 모듈 탭
- 토큰 사용자, 무결성 수준, 권한 탭
- 메모리 영역 탭
- 서비스와 프로세스 매핑
- 핸들 열거와 타입 표시

## 검증 기록

최근 전체 빌드 검증 명령:

```powershell
MSBuild Win32.slnx /p:Configuration=Debug /p:Platform=x64
```

최근 결과:

- Build succeeded
- 0 Warning(s)
- 0 Error(s)
