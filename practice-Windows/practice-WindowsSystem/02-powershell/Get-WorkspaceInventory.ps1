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
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\powershell"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$files = Get-ChildItem -Path $Root -Recurse -File -Force |
    Where-Object { $_.FullName -notlike "*\sandbox\*" } |
    Sort-Object FullName

$fileRows = $files | Select-Object FullName, Name, Extension, Length, LastWriteTime
$fileRows | Export-Csv (Join-Path $OutputDirectory "workspace-files.csv") -NoTypeInformation -Encoding UTF8

$extensionRows = $files |
    Group-Object { if ($_.Extension) { $_.Extension.ToLowerInvariant() } else { "[none]" } } |
    Sort-Object Count -Descending |
    Select-Object @{ Name = "Extension"; Expression = { $_.Name } }, Count

$extensionRows | Export-Csv (Join-Path $OutputDirectory "extension-summary.csv") -NoTypeInformation -Encoding UTF8

$summary = [ordered]@{
    Root = (Resolve-Path $Root).Path
    GeneratedAt = (Get-Date).ToString("s")
    FileCount = $files.Count
    TotalBytes = ($files | Measure-Object -Property Length -Sum).Sum
    LargestFiles = @($files | Sort-Object Length -Descending | Select-Object -First 10 FullName, Length)
    Extensions = @($extensionRows)
}

$summary | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $OutputDirectory "workspace-summary.json") -Encoding UTF8

$summary
