# Logging Labs

## Lab 1. 로그 목록 보기

```powershell
Get-WinEvent -ListLog * | Select-Object -First 20 LogName, RecordCount
```

## Lab 2. 최근 시스템 오류 확인

```powershell
Get-WinEvent -FilterHashtable @{
    LogName = "System"
    Level = 2
    StartTime = (Get-Date).AddDays(-1)
} | Select-Object -First 20 TimeCreated, ProviderName, Id, Message
```

## Lab 3. CSV로 저장

```powershell
New-Item -ItemType Directory -Path .\sandbox\logging -Force
Get-WinEvent -LogName System -MaxEvents 100 |
    Select-Object TimeCreated, ProviderName, Id, LevelDisplayName, Message |
    Export-Csv .\sandbox\logging\system-events.csv -NoTypeInformation -Encoding UTF8
```

