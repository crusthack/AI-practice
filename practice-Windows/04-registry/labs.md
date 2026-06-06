# Registry Labs

## Lab 1. 안전한 실습 키 만들기

```powershell
New-Item -Path HKCU:\Software\PracticeWindows -Force
New-ItemProperty -Path HKCU:\Software\PracticeWindows -Name ExampleString -Value "hello" -PropertyType String -Force
Get-ItemProperty -Path HKCU:\Software\PracticeWindows
```

## Lab 2. reg.exe로 조회

```cmd
reg query HKCU\Software\PracticeWindows
```

## Lab 3. 백업과 삭제

```powershell
New-Item -ItemType Directory -Path .\sandbox\registry -Force
reg export HKCU\Software\PracticeWindows .\sandbox\registry\PracticeWindows.reg /y
Remove-Item -Path HKCU:\Software\PracticeWindows -Recurse -WhatIf
```

`-WhatIf` 결과를 확인한 뒤에만 실제 삭제 명령을 실행합니다.

