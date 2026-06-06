# Project. winget State Export

목표:

- `winget` 사용 가능 여부를 확인합니다.
- 설치된 패키지 목록과 export 결과를 저장합니다.
- 실패 원인도 리포트에 남깁니다.

실행:

```powershell
.\05-winget\Export-WingetState.ps1
```

산출물:

- `sandbox\winget\winget-state.txt`
- `sandbox\winget\installed.txt`
- `sandbox\winget\packages.json`

관찰 포인트:

- `winget show`로 설치 전 publisher, source, installer 정보를 확인합니다.
- `winget export`는 환경 재현의 출발점이지만 모든 앱이 export되지 않을 수 있습니다.

