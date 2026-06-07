# Project. Workspace Map with cmd.exe

목표:

- `cmd.exe`만 사용해 워크스페이스 파일 목록, 환경 변수, 기본 명령 위치를 기록합니다.
- 리다이렉션, 파이프, `where`, `dir`, `findstr`, `errorlevel`을 실제로 사용합니다.

실행:

```cmd
01-cmd\workspace-map.cmd
```

산출물:

- `sandbox\cmd\workspace-map.txt`
- `sandbox\cmd\readme-files.txt`

관찰 포인트:

- `where`는 실행 파일 탐색에 `PATH`를 사용합니다.
- `dir /s /b`는 재귀 파일 목록을 기계가 읽기 쉬운 형식으로 출력합니다.
- 배치 파일에서 `%ERRORLEVEL%`은 직전 명령의 성공/실패를 판단하는 기본 재료입니다.

