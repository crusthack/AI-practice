# Windows System Practice

Win32 API를 제외하고 Windows 운영체제를 다루는 실무 도구를 학습하기 위한 워크스페이스입니다.

다루는 범위:

- `cmd.exe` 기본 명령과 배치 파일
- PowerShell 명령, 파이프라인, 스크립트
- 파일 시스템, 프로세스, 서비스, 이벤트 로그
- 레지스트리 조회와 안전한 HKCU 기반 실습
- `winget`을 이용한 패키지 검색, 설치, 업그레이드 관리
- Windows 환경 변수, PATH, 시작 프로그램, 작업 스케줄러
- 기본 네트워크 진단 명령

제외 범위:

- Win32 API 직접 호출
- C/C++ Windows API 프로그래밍
- 커널 드라이버, 후킹, 우회, 권한 상승 기법

## 폴더 구조

```text
00-orientation/       학습 목표, 안전 규칙, 명령 실행 기준
01-cmd/               cmd.exe와 배치 파일
02-powershell/        PowerShell 기본기와 스크립팅
03-filesystem/        파일, 권한, 링크, 경로
04-registry/          레지스트리 조회/수정 실습
05-winget/            winget 패키지 관리
06-process-service/   프로세스, 서비스, 작업 스케줄러
07-network/           Windows 네트워크 진단
08-logging/           이벤트 로그와 진단 자료 수집
09-automation/        반복 작업 자동화 미니 프로젝트
scripts/              재사용 가능한 실습 스크립트
sandbox/              실습 중 생성되는 파일 보관
notes/                개인 학습 노트
```

## 시작 순서

1. [00-orientation/safety.md](00-orientation/safety.md)를 먼저 읽습니다.
2. [00-orientation/check-environment.ps1](00-orientation/check-environment.ps1)을 PowerShell에서 실행해 현재 환경을 기록합니다.
3. [roadmap.md](roadmap.md)의 순서대로 각 모듈의 `README.md`와 `labs.md`를 진행합니다.
4. 각 모듈의 `project.md`를 실행해 작은 결과물을 만듭니다.

## 실행 원칙

- 관리자 권한이 필요한 명령은 먼저 관리자 권한이 왜 필요한지 적습니다.
- 레지스트리 실습은 기본적으로 `HKCU:\Software\PracticeWindows` 아래에서만 합니다.
- 시스템 전체 변경 명령은 `-WhatIf`, 조회 명령, 백업 명령을 먼저 사용합니다.
- 실습 산출물은 `sandbox/` 아래에 둡니다.

## 프로젝트 실행 예시

```powershell
.\00-orientation\check-environment.ps1
.\02-powershell\Get-WorkspaceInventory.ps1
.\03-filesystem\Invoke-FileAudit.ps1
.\04-registry\Invoke-RegistryPractice.ps1 -Mode Create
.\09-automation\Build-WorkspaceReport.ps1
```
