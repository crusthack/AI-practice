# Learning Roadmap

## 0. Orientation

- Windows 명령 실행 환경 구분: `cmd.exe`, PowerShell, Windows Terminal
- 관리자 권한과 일반 권한 차이
- 실습 전후 상태 확인 습관

## 1. cmd.exe

- `dir`, `cd`, `copy`, `move`, `del`, `type`, `findstr`
- 환경 변수: `set`, `%PATH%`, `%USERPROFILE%`
- 리다이렉션과 파이프: `>`, `>>`, `<`, `|`
- 배치 파일: 인자, `if`, `for`, `errorlevel`
- 프로젝트: `01-cmd\workspace-map.cmd`

## 2. PowerShell

- cmdlet 명명 규칙: `Verb-Noun`
- 객체 파이프라인과 텍스트 파이프라인의 차이
- `Get-Help`, `Get-Command`, `Get-Member`
- 스크립트, 함수, 에러 처리, 실행 정책
- 프로젝트: `02-powershell\Get-WorkspaceInventory.ps1`

## 3. Filesystem

- 경로, 확장자, 숨김 파일, 시스템 파일
- ACL 조회: `Get-Acl`, `icacls`
- 심볼릭 링크, 하드 링크, junction
- 압축, 해시, 파일 검색
- 프로젝트: `03-filesystem\Invoke-FileAudit.ps1`

## 4. Registry

- registry hive와 key/value 구조
- `reg.exe`와 PowerShell registry provider
- `HKCU` 기반 안전한 생성/조회/삭제
- `.reg` export/import와 백업 습관
- 프로젝트: `04-registry\Invoke-RegistryPractice.ps1`

## 5. winget

- 패키지 검색과 정보 확인
- 설치, 제거, 업그레이드
- source 관리
- export/import로 개발 환경 재현
- 프로젝트: `05-winget\Export-WingetState.ps1`

## 6. Process, Service, Scheduler

- 프로세스 조회와 종료
- 서비스 상태 조회와 시작/중지
- 작업 스케줄러 기본 작업 만들기
- 시작 프로그램과 자동 실행 위치
- 프로젝트: `06-process-service\Get-SystemActivity.ps1`

## 7. Network

- `ipconfig`, `ping`, `tracert`, `nslookup`
- PowerShell `Test-NetConnection`
- 포트 사용 프로세스 찾기
- Windows 방화벽 조회
- 프로젝트: `07-network\Test-NetworkBasics.ps1`

## 8. Logging

- 이벤트 뷰어 구조
- `Get-WinEvent`
- 문제 재현 전후 로그 수집
- 진단 리포트 작성
- 프로젝트: `08-logging\Export-EventSummary.ps1`

## 9. Automation

- 환경 점검 스크립트
- winget 기반 앱 목록 백업
- 이벤트 로그 요약기
- 개발 PC 초기 세팅 점검표
- 프로젝트: `09-automation\Build-WorkspaceReport.ps1`
