# Project. HKCU Registry Practice

목표:

- `HKCU:\Software\PracticeWindows` 아래에 안전한 실습 키를 만들고 조회합니다.
- PowerShell registry provider와 `reg.exe` export를 같이 사용합니다.

실행:

```powershell
.\04-registry\Invoke-RegistryPractice.ps1 -Mode Create
.\04-registry\Invoke-RegistryPractice.ps1 -Mode Read
.\04-registry\Invoke-RegistryPractice.ps1 -Mode Export
.\04-registry\Invoke-RegistryPractice.ps1 -Mode Remove -WhatIf
```

산출물:

- `sandbox\registry\PracticeWindows.reg`
- `sandbox\registry\registry-read.json`

관찰 포인트:

- `HKCU`는 현재 사용자 범위입니다.
- `.reg` export는 수정 전 백업으로 유용합니다.
- 삭제는 먼저 `-WhatIf`로 확인합니다.

