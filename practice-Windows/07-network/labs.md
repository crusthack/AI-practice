# Network Labs

## Lab 1. IP 구성 확인

```cmd
ipconfig /all
```

## Lab 2. 연결 진단

```powershell
Test-NetConnection example.com -Port 443
ping example.com
tracert example.com
nslookup example.com
```

## Lab 3. 포트 사용 프로세스 찾기

```powershell
Get-NetTCPConnection |
    Where-Object State -eq Listen |
    Select-Object LocalAddress, LocalPort, OwningProcess |
    Sort-Object LocalPort
```

