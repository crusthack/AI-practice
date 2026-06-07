# Process, Service, Scheduler Labs

## Lab 1. 프로세스 조회

```powershell
Get-Process | Sort-Object CPU -Descending | Select-Object -First 10 Name, Id, CPU
tasklist
```

## Lab 2. 서비스 조회

```powershell
Get-Service | Sort-Object Status, Name | Select-Object -First 20
sc.exe query
```

## Lab 3. 예약 작업 조회

```powershell
Get-ScheduledTask | Select-Object -First 20 TaskName, TaskPath, State
schtasks /query /fo table
```

