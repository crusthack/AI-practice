# PowerShell Labs

## Lab 1. 객체 파이프라인 확인

```powershell
Get-Process | Get-Member
Get-Process | Select-Object -First 5 Name, Id, CPU
```

## Lab 2. 파일 목록 CSV로 저장

```powershell
New-Item -ItemType Directory -Path .\sandbox\powershell -Force
Get-ChildItem -Recurse |
    Select-Object FullName, Length, LastWriteTime |
    Export-Csv .\sandbox\powershell\files.csv -NoTypeInformation -Encoding UTF8
```

## Lab 3. 도움말 읽기

```powershell
Get-Help Get-ChildItem -Examples
Get-Command *Service*
```

