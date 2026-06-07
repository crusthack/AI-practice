# Project. Event Log Summary

목표:

- 최근 Windows 이벤트 로그 중 오류/경고를 요약합니다.
- Provider별 발생 빈도를 CSV로 저장합니다.

실행:

```powershell
.\08-logging\Export-EventSummary.ps1 -Days 1
```

산출물:

- `sandbox\logging\system-errors.csv`
- `sandbox\logging\application-errors.csv`
- `sandbox\logging\provider-summary.csv`

관찰 포인트:

- `Level=2`는 오류, `Level=3`은 경고입니다.
- 같은 이벤트 ID라도 Provider에 따라 의미가 다릅니다.

