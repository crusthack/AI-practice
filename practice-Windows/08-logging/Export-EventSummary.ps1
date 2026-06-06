[CmdletBinding()]
param(
    [int]$Days = 1,
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\logging"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$start = (Get-Date).AddDays(-1 * $Days)
$logs = @("System", "Application")
$allEvents = @()

foreach ($logName in $logs) {
    try {
        $events = Get-WinEvent -FilterHashtable @{
            LogName = $logName
            Level = 2, 3
            StartTime = $start
        } -ErrorAction Stop
    } catch {
        "Get-WinEvent failed for ${logName}: $($_.Exception.Message)" |
            Set-Content (Join-Path $OutputDirectory "$logName-error.txt") -Encoding UTF8
        $events = @()
    }

    $rows = $events |
        Select-Object TimeCreated, LogName, ProviderName, Id, LevelDisplayName, Message

    $rows | Export-Csv (Join-Path $OutputDirectory "$($logName.ToLowerInvariant())-errors.csv") -NoTypeInformation -Encoding UTF8
    $allEvents += $rows
}

$allEvents |
    Group-Object LogName, ProviderName, Id, LevelDisplayName |
    Sort-Object Count -Descending |
    Select-Object Count,
        @{ Name = "LogProviderIdLevel"; Expression = { $_.Name } } |
    Export-Csv (Join-Path $OutputDirectory "provider-summary.csv") -NoTypeInformation -Encoding UTF8

"Event summary written to $OutputDirectory"
