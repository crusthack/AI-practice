[CmdletBinding()]
param(
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\winget"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$stateFile = Join-Path $OutputDirectory "winget-state.txt"
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("Generated: $((Get-Date).ToString("s"))")

$winget = Get-Command winget -ErrorAction SilentlyContinue
if (-not $winget) {
    $lines.Add("winget: not found")
    $lines | Set-Content $stateFile -Encoding UTF8
    Get-Content $stateFile
    return
}

$lines.Add("winget path: $($winget.Source)")

try {
    $version = & winget --version 2>&1
    $lines.Add("winget version: $version")
} catch {
    $lines.Add("winget version failed: $($_.Exception.Message)")
}

try {
    & winget source list 2>&1 | Out-File (Join-Path $OutputDirectory "sources.txt") -Encoding UTF8
    $lines.Add("source list: written")
} catch {
    $lines.Add("source list failed: $($_.Exception.Message)")
}

try {
    & winget list 2>&1 | Out-File (Join-Path $OutputDirectory "installed.txt") -Encoding UTF8
    $lines.Add("installed list: written")
} catch {
    $lines.Add("installed list failed: $($_.Exception.Message)")
}

try {
    & winget export -o (Join-Path $OutputDirectory "packages.json") 2>&1 |
        Out-File (Join-Path $OutputDirectory "export-output.txt") -Encoding UTF8
    $lines.Add("export: attempted")
} catch {
    $lines.Add("export failed: $($_.Exception.Message)")
}

$lines | Set-Content $stateFile -Encoding UTF8
Get-Content $stateFile
