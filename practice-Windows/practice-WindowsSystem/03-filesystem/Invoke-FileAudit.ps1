[CmdletBinding()]
param(
    [string]$Root,
    [string]$OutputDirectory,
    [int64]$MaxHashBytes = 10485760
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $Root) {
    $Root = $workspaceRoot
}
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\filesystem"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$files = Get-ChildItem -Path $Root -Recurse -File -Force |
    Where-Object { $_.FullName -notlike "*\sandbox\*" } |
    Sort-Object FullName

$rows = foreach ($file in $files) {
    $hash = if ($file.Length -le $MaxHashBytes) {
        (Get-FileHash -Path $file.FullName -Algorithm SHA256).Hash
    } else {
        "[skipped: larger than $MaxHashBytes bytes]"
    }

    [pscustomobject]@{
        FullName = $file.FullName
        Length = $file.Length
        Extension = $file.Extension
        LastWriteTime = $file.LastWriteTime
        Attributes = $file.Attributes
        Sha256 = $hash
    }
}

$rows | Export-Csv (Join-Path $OutputDirectory "file-audit.csv") -NoTypeInformation -Encoding UTF8

$rows |
    Where-Object { $_.Sha256 -and -not $_.Sha256.StartsWith("[skipped:") } |
    Group-Object Sha256 |
    Where-Object Count -gt 1 |
    ForEach-Object {
        foreach ($item in $_.Group) {
            [pscustomobject]@{
                Sha256 = $_.Name
                FullName = $item.FullName
                Length = $item.Length
            }
        }
    } |
    Export-Csv (Join-Path $OutputDirectory "duplicate-candidates.csv") -NoTypeInformation -Encoding UTF8

Get-Acl -Path $Root | Format-List * | Out-File (Join-Path $OutputDirectory "root-acl.txt") -Encoding UTF8

"File audit written to $OutputDirectory"
