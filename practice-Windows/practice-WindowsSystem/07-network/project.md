# Project. Network Basics Diagnostic

목표:

- DNS, ICMP, TCP 연결, listen 포트를 한 번에 점검합니다.
- 네트워크 문제 리포트의 기본 재료를 만듭니다.

실행:

```powershell
.\07-network\Test-NetworkBasics.ps1 -Target example.com -Port 443
```

산출물:

- `sandbox\network\network-summary.txt`
- `sandbox\network\tcp-listeners.csv`
- `sandbox\network\ipconfig.txt`

관찰 포인트:

- DNS 확인과 TCP 연결 성공은 다른 문제입니다.
- listen 포트의 `OwningProcess`는 `Get-Process -Id`로 프로세스 이름과 연결할 수 있습니다.

