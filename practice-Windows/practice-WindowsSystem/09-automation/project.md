# Project. Build Workspace Report

목표:

- 앞 모듈의 조회 스크립트를 순서대로 실행합니다.
- `sandbox\report` 아래에 통합 리포트를 만듭니다.

실행:

```powershell
.\09-automation\Build-WorkspaceReport.ps1
```

산출물:

- `sandbox\report\index.md`
- 각 모듈별 산출물 링크와 실행 상태

관찰 포인트:

- 자동화 스크립트는 실패를 기록하고 다음 단계로 넘어갈 수 있어야 합니다.
- 시스템 조회 작업도 실행 환경에 따라 실패할 수 있으므로 로그가 필요합니다.

