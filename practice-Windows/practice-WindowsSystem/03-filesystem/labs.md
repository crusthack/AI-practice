# Filesystem Labs

## Lab 1. 파일 속성 보기

```powershell
Get-ChildItem -Force
Get-Item .\README.md | Format-List *
```

## Lab 2. ACL 조회

```powershell
Get-Acl . | Format-List
icacls .
```

## Lab 3. 해시 계산

```powershell
Get-FileHash .\README.md -Algorithm SHA256
```

