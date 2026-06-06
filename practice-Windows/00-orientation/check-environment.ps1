$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$outDir = Join-Path $root "sandbox"
$outFile = Join-Path $outDir "environment-report.txt"

New-Item -ItemType Directory -Path $outDir -Force | Out-Null

$osCaption = [System.Runtime.InteropServices.RuntimeInformation]::OSDescription
$osVersion = [Environment]::OSVersion.VersionString

try {
    $os = Get-CimInstance Win32_OperatingSystem -ErrorAction Stop
    $osCaption = $os.Caption
    $osVersion = $os.Version
} catch {
    $osCaption = "$osCaption (CIM query failed: $($_.Exception.Message))"
}

$report = [ordered]@{
    Timestamp = (Get-Date).ToString("s")
    UserName = $env:USERNAME
    ComputerName = $env:COMPUTERNAME
    PowerShellVersion = $PSVersionTable.PSVersion.ToString()
    PSEdition = $PSVersionTable.PSEdition
    OS = $osCaption
    OSVersion = $osVersion
    CurrentDirectory = (Get-Location).Path
    ExecutionPolicyCurrentUser = Get-ExecutionPolicy -Scope CurrentUser
    ExecutionPolicyLocalMachine = Get-ExecutionPolicy -Scope LocalMachine
}

$lines = foreach ($item in $report.GetEnumerator()) {
    "{0}: {1}" -f $item.Key, $item.Value
}

$winget = Get-Command winget -ErrorAction SilentlyContinue
if ($winget) {
    $lines += "Winget: $($winget.Source)"
    try {
        $wingetVersion = & winget --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            $lines += "WingetVersion: $wingetVersion"
        } else {
            $lines += "WingetVersion: command failed with exit code $LASTEXITCODE"
        }
    } catch {
        $lines += "WingetVersion: command failed: $($_.Exception.Message)"
    }
} else {
    $lines += "Winget: not found"
}

$lines | Set-Content -Path $outFile -Encoding UTF8
Get-Content -Path $outFile
