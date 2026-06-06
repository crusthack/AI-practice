param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [int]$SmokeMilliseconds = 900
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$solution = Join-Path $repoRoot "PracticeD3D11.slnx"

function Find-MSBuild {
    $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    $roots = @(
        "${env:ProgramFiles}\Microsoft Visual Studio",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio"
    )

    foreach ($root in $roots) {
        if (-not (Test-Path -LiteralPath $root)) {
            continue
        }

        $candidate = Get-ChildItem -LiteralPath $root -Recurse -Filter MSBuild.exe -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -like "*\MSBuild\Current\Bin\MSBuild.exe" } |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }

    throw "MSBuild.exe was not found. Install Visual Studio Build Tools or add MSBuild to PATH."
}

$msbuild = Find-MSBuild
Write-Host "MSBuild: $msbuild"
Write-Host "Building $solution ($Configuration|$Platform)"
& $msbuild $solution "/p:Configuration=$Configuration" "/p:Platform=$Platform" /m
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$outputDir = Join-Path $repoRoot "$Platform\$Configuration"
if (-not (Test-Path -LiteralPath $outputDir)) {
    throw "Output directory was not found: $outputDir"
}

$executables = Get-ChildItem -LiteralPath $outputDir -Filter "*.exe" | Sort-Object Name
if ($executables.Count -eq 0) {
    throw "No sample executables were found in $outputDir"
}

$results = foreach ($exe in $executables) {
    $process = Start-Process -FilePath $exe.FullName -WorkingDirectory $outputDir -WindowStyle Hidden -PassThru
    Start-Sleep -Milliseconds $SmokeMilliseconds

    if ($process.HasExited) {
        [pscustomobject]@{
            Name = $exe.Name
            Result = "Exited"
            ExitCode = $process.ExitCode
        }
    }
    else {
        Stop-Process -Id $process.Id
        [pscustomobject]@{
            Name = $exe.Name
            Result = "Started"
            ExitCode = ""
        }
    }
}

$results | Format-Table -AutoSize

$failed = $results | Where-Object { $_.Result -eq "Exited" -and $_.ExitCode -ne 0 }
if ($failed) {
    Write-Error "One or more samples exited with a non-zero code during smoke testing."
    exit 1
}

Write-Host "Verification completed: build succeeded and $($executables.Count) samples started."
