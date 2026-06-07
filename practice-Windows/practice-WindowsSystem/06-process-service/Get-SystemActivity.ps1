[CmdletBinding()]
param(
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\process-service"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

Get-Process |
    Sort-Object WorkingSet64 -Descending |
    Select-Object -First 50 ProcessName, Id, CPU, WorkingSet64, StartTime |
    Export-Csv (Join-Path $OutputDirectory "top-processes.csv") -NoTypeInformation -Encoding UTF8

Get-Service |
    Sort-Object Status, Name |
    Select-Object Name, DisplayName, Status, StartType |
    Export-Csv (Join-Path $OutputDirectory "services.csv") -NoTypeInformation -Encoding UTF8

try {
    Get-ScheduledTask |
        Select-Object TaskName, TaskPath, State |
        Export-Csv (Join-Path $OutputDirectory "scheduled-tasks.csv") -NoTypeInformation -Encoding UTF8
} catch {
    "Get-ScheduledTask failed: $($_.Exception.Message)" |
        Set-Content (Join-Path $OutputDirectory "scheduled-tasks-error.txt") -Encoding UTF8
}

"System activity snapshot written to $OutputDirectory"
