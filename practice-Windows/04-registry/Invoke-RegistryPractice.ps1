[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [ValidateSet("Create", "Read", "Export", "Remove")]
    [string]$Mode = "Read",
    [string]$KeyPath = "HKCU:\Software\PracticeWindows",
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\registry"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

switch ($Mode) {
    "Create" {
        if ($PSCmdlet.ShouldProcess($KeyPath, "Create practice registry key and values")) {
            New-Item -Path $KeyPath -Force | Out-Null
            New-ItemProperty -Path $KeyPath -Name "ExampleString" -Value "hello registry" -PropertyType String -Force | Out-Null
            New-ItemProperty -Path $KeyPath -Name "ExampleDword" -Value 42 -PropertyType DWord -Force | Out-Null
            New-ItemProperty -Path $KeyPath -Name "LastUpdated" -Value (Get-Date).ToString("s") -PropertyType String -Force | Out-Null
        }
    }
    "Read" {
        $value = if (Test-Path $KeyPath) {
            Get-ItemProperty -Path $KeyPath
        } else {
            [pscustomobject]@{ Message = "Registry key does not exist"; KeyPath = $KeyPath }
        }

        $value |
            Select-Object * -ExcludeProperty PSPath, PSParentPath, PSChildName, PSDrive, PSProvider |
            ConvertTo-Json -Depth 3 |
            Set-Content (Join-Path $OutputDirectory "registry-read.json") -Encoding UTF8

        $value
    }
    "Export" {
        $regPath = $KeyPath -replace "^HKCU:", "HKCU"
        $outFile = Join-Path $OutputDirectory "PracticeWindows.reg"
        if (Test-Path $KeyPath) {
            & reg export $regPath $outFile /y | Out-String
        } else {
            "Registry key does not exist: $KeyPath"
        }
    }
    "Remove" {
        if (Test-Path $KeyPath -and $PSCmdlet.ShouldProcess($KeyPath, "Remove practice registry key")) {
            Remove-Item -Path $KeyPath -Recurse -Force
        }
    }
}
