# cmd Labs

## Lab 1. 도움말과 명령 위치 찾기

```cmd
where cmd
where powershell
dir /?
for /?
```

기록할 것:

- `where`가 찾은 실행 파일 경로
- `dir /a`, `dir /s`, `dir /b`의 차이

## Lab 2. sandbox 파일 만들기

```cmd
mkdir sandbox\cmd
echo hello cmd > sandbox\cmd\hello.txt
type sandbox\cmd\hello.txt
```

## Lab 3. findstr 검색

```cmd
dir /s /b > sandbox\cmd\files.txt
findstr /i "readme" sandbox\cmd\files.txt
```

