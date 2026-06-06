[CmdletBinding()]
param(
    [string]$Target = "example.com",
    [int]$Port = 443,
    [string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $workspaceRoot "sandbox\network"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

ipconfig /all | Out-File (Join-Path $OutputDirectory "ipconfig.txt") -Encoding UTF8

$summary = New-Object System.Collections.Generic.List[string]
$summary.Add("Generated: $((Get-Date).ToString("s"))")
$summary.Add("Target: $Target")
$summary.Add("Port: $Port")

try {
    $dns = Resolve-DnsName $Target -ErrorAction Stop
    $summary.Add("DNS: ok")
    $dns | Select-Object Name, Type, IPAddress, NameHost |
        Export-Csv (Join-Path $OutputDirectory "dns.csv") -NoTypeInformation -Encoding UTF8
} catch {
    $summary.Add("DNS: failed: $($_.Exception.Message)")
}

try {
    $tcp = Test-NetConnection $Target -Port $Port -WarningAction SilentlyContinue
    $summary.Add("TCP: TcpTestSucceeded=$($tcp.TcpTestSucceeded)")
    $tcp | Select-Object ComputerName, RemoteAddress, RemotePort, InterfaceAlias, SourceAddress, TcpTestSucceeded |
        ConvertTo-Json -Depth 3 |
        Set-Content (Join-Path $OutputDirectory "tcp-test.json") -Encoding UTF8
} catch {
    $summary.Add("TCP: failed: $($_.Exception.Message)")
}

try {
    Get-NetTCPConnection -ErrorAction Stop |
        Where-Object State -eq Listen |
        Select-Object LocalAddress, LocalPort, OwningProcess,
            @{ Name = "ProcessName"; Expression = {
                try { (Get-Process -Id $_.OwningProcess -ErrorAction Stop).ProcessName } catch { "" }
            } } |
        Sort-Object LocalPort |
        Export-Csv (Join-Path $OutputDirectory "tcp-listeners.csv") -NoTypeInformation -Encoding UTF8
    $summary.Add("TCP listeners: written")
} catch {
    $summary.Add("TCP listeners: failed: $($_.Exception.Message)")
    "Get-NetTCPConnection failed: $($_.Exception.Message)" |
        Set-Content (Join-Path $OutputDirectory "tcp-listeners-error.txt") -Encoding UTF8
}

$summary | Set-Content (Join-Path $OutputDirectory "network-summary.txt") -Encoding UTF8
Get-Content (Join-Path $OutputDirectory "network-summary.txt")
