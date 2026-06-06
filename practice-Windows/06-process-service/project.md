# Project. System Activity Snapshot

목표:

- 현재 프로세스, 서비스, 예약 작업 상태를 CSV로 저장합니다.
- CPU/메모리 사용량이 큰 프로세스를 찾습니다.
- 서비스는 조회만 수행합니다.

실행:

```powershell
.\06-process-service\Get-SystemActivity.ps1
```

산출물:

- `sandbox\process-service\top-processes.csv`
- `sandbox\process-service\services.csv`
- `sandbox\process-service\scheduled-tasks.csv`

관찰 포인트:

- 프로세스 종료와 서비스 중지는 별개의 작업입니다.
- 서비스 변경은 관리자 권한과 영향 범위 확인이 필요합니다.

