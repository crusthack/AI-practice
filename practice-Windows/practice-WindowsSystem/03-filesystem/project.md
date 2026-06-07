# Project. File Audit

목표:

- 파일 크기, 해시, ACL을 수집합니다.
- 중복 파일 후보를 SHA256 해시 기준으로 찾습니다.

실행:

```powershell
.\03-filesystem\Invoke-FileAudit.ps1
```

산출물:

- `sandbox\filesystem\file-audit.csv`
- `sandbox\filesystem\duplicate-candidates.csv`
- `sandbox\filesystem\root-acl.txt`

관찰 포인트:

- 해시는 파일 내용 기준 식별자입니다.
- ACL은 파일 접근 제어를 설명하며, 소유자와 ACE 목록으로 구성됩니다.

