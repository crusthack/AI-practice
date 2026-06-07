[CmdletBinding()]
param(
    [string]$Root,
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $Root) {
    $Root = $workspaceRoot
}
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\report"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$steps = @(
    @{ Name = "Environment"; Command = Join-Path $Root "00-orientation\check-environment.ps1" },
    @{ Name = "PowerShell Inventory"; Command = Join-Path $Root "02-powershell\Get-WorkspaceInventory.ps1" },
    @{ Name = "File Audit"; Command = Join-Path $Root "03-filesystem\Invoke-FileAudit.ps1" },
    @{ Name = "Registry Read"; Command = Join-Path $Root "04-registry\Invoke-RegistryPractice.ps1"; Parameters = @{ Mode = "Read" } },
    @{ Name = "Winget State"; Command = Join-Path $Root "05-winget\Export-WingetState.ps1" },
    @{ Name = "System Activity"; Command = Join-Path $Root "06-process-service\Get-SystemActivity.ps1" },
    @{ Name = "Network"; Command = Join-Path $Root "07-network\Test-NetworkBasics.ps1" },
    @{ Name = "Logging"; Command = Join-Path $Root "08-logging\Export-EventSummary.ps1" }
)

$results = foreach ($step in $steps) {
    $logFile = Join-Path $OutputDirectory (($step.Name -replace "[^a-zA-Z0-9]+", "-").Trim("-") + ".log")
    try {
        $parametersForStep = @{}
        if ($step.Parameters) {
            $parametersForStep = $step.Parameters
        }

        & $step.Command @parametersForStep *> $logFile

        [pscustomobject]@{
            Name = $step.Name
            Status = "ok"
            Log = $logFile
        }
    } catch {
        "ERROR: $($_.Exception.Message)" | Add-Content $logFile -Encoding UTF8
        [pscustomobject]@{
            Name = $step.Name
            Status = "failed"
            Log = $logFile
        }
    }
}

$index = @(
    "# Windows Practice Report"
    ""
    "Generated: $((Get-Date).ToString("s"))"
    ""
    "| Step | Status | Log |"
    "| --- | --- | --- |"
)

foreach ($result in $results) {
    $relativeLog = Resolve-Path -Path $result.Log -Relative
    $index += "| $($result.Name) | $($result.Status) | ``$relativeLog`` |"
}

$index | Set-Content (Join-Path $OutputDirectory "index.md") -Encoding UTF8
$results
