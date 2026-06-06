# winget Labs

## Lab 1. winget 상태 확인

```powershell
winget --version
winget source list
```

## Lab 2. 패키지 검색과 상세 정보

```powershell
winget search PowerShell
winget show Microsoft.PowerShell
```

## Lab 3. 설치된 패키지 목록 저장

```powershell
New-Item -ItemType Directory -Path .\sandbox\winget -Force
winget list > .\sandbox\winget\installed.txt
winget export -o .\sandbox\winget\packages.json
```

