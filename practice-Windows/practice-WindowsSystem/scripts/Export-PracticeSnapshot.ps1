[CmdletBinding()]
param(
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\snapshot"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

Get-ChildItem -Force |
    Select-Object Name, Mode, Length, LastWriteTime |
    Export-Csv (Join-Path $OutputDirectory "workspace-files.csv") -NoTypeInformation -Encoding UTF8

Get-Process |
    Sort-Object ProcessName |
    Select-Object ProcessName, Id, CPU, WorkingSet64 |
    Export-Csv (Join-Path $OutputDirectory "processes.csv") -NoTypeInformation -Encoding UTF8

Get-Service |
    Sort-Object Name |
    Select-Object Name, DisplayName, Status, StartType |
    Export-Csv (Join-Path $OutputDirectory "services.csv") -NoTypeInformation -Encoding UTF8

if (Get-Command winget -ErrorAction SilentlyContinue) {
    winget list | Out-File (Join-Path $OutputDirectory "winget-list.txt") -Encoding UTF8
}

"Snapshot written to $OutputDirectory"
