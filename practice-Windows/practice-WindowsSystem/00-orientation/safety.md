# Safety Rules

## 기본 원칙

- 모르는 삭제 명령은 실행하지 않습니다.
- `HKLM`, `HKCR`, `HKU`, `C:\Windows`, `C:\Program Files` 변경은 별도 기록 없이 하지 않습니다.
- 레지스트리 수정 실습은 `HKCU:\Software\PracticeWindows`만 사용합니다.
- 서비스 중지, 방화벽 변경, PATH 변경은 현재 목적과 되돌리는 방법을 먼저 적습니다.

## 먼저 써볼 옵션

PowerShell:

```powershell
Remove-Item .\sandbox\example.txt -WhatIf
Get-Command Remove-Item -Syntax
Get-Help Remove-Item -Online
```

cmd:

```cmd
where command
command /?
```

winget:

```powershell
winget search PowerShell
winget show Microsoft.PowerShell
```

## 레지스트리 백업 예시

```powershell
reg export HKCU\Software\PracticeWindows .\sandbox\PracticeWindows.reg /y
```

