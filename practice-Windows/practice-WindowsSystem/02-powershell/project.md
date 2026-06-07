# Project. Workspace Inventory

목표:

- PowerShell 객체 파이프라인으로 워크스페이스 파일을 분석합니다.
- 파일 목록을 CSV/JSON으로 저장하고, 확장자별 개수를 요약합니다.

실행:

```powershell
.\02-powershell\Get-WorkspaceInventory.ps1
```

산출물:

- `sandbox\powershell\workspace-files.csv`
- `sandbox\powershell\extension-summary.csv`
- `sandbox\powershell\workspace-summary.json`

관찰 포인트:

- `Get-ChildItem`은 문자열이 아니라 `FileInfo`/`DirectoryInfo` 객체를 반환합니다.
- `Group-Object`, `Measure-Object`, `ConvertTo-Json`으로 구조화된 리포트를 만들 수 있습니다.

