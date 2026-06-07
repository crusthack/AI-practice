# Batch File Examples

Windows에서 자주 쓰는 셸 명령을 `.cmd` 배치 파일로 묶는 예제입니다.

## 실행 방법

```cmd
01-cmd\examples\hello.cmd
01-cmd\examples\backup-folder.cmd notes sandbox\cmd-backup
01-cmd\examples\find-text.cmd README .
01-cmd\examples\system-report.cmd
```

## 예제별 목표

- `hello.cmd`: 변수, 인자, 현재 경로 출력
- `backup-folder.cmd`: 폴더 존재 확인, `xcopy`, 에러 처리
- `find-text.cmd`: `findstr`로 파일 내용 검색
- `system-report.cmd`: 여러 Windows 명령 결과를 하나의 로그 파일로 저장
- `call-powershell.cmd`: 배치 파일에서 PowerShell 스크립트 호출

## 배치 파일 기본 문법

```cmd
@echo off
setlocal

set "NAME=value"
echo %NAME%

if not exist "sandbox" mkdir "sandbox"

command
if errorlevel 1 (
    echo command failed
    exit /b 1
)

endlocal
```

주의:

- 변수 대입은 `set "NAME=value"` 형태가 안전합니다.
- 경로는 항상 큰따옴표로 감쌉니다.
- 삭제 명령은 먼저 `echo`로 대상 경로를 출력해 확인합니다.

