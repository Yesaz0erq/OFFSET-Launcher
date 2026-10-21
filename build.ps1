param([switch]$Run)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$assemblerPath = Join-Path $projectRoot 'tools\FASM.EXE'
if (!(Test-Path -LiteralPath $assemblerPath)) {
    throw 'FASM missing. Extract https://flatassembler.net/fasmw17332.zip into tools/ (FASM.EXE and INCLUDE/).'
}
# UI and status text for zh / en / ja -> src/strings.inc, src/strings_status.inc
& (Join-Path $projectRoot 'tools\gen-strings.ps1') -ProjectRoot $projectRoot
New-Item -ItemType Directory -Force (Join-Path $projectRoot 'dist') | Out-Null
$previousInclude = $env:INCLUDE
try {
    $env:INCLUDE = Join-Path $projectRoot 'tools\INCLUDE'
    Push-Location (Join-Path $projectRoot 'src')
    & $assemblerPath '-m' '262144' 'main.asm' '..\dist\OFFSET.exe'
    if ($LASTEXITCODE -ne 0) { throw "FASM exited with $LASTEXITCODE" }
} finally {
    Pop-Location
    $env:INCLUDE = $previousInclude
}
$binary = Get-Item -LiteralPath (Join-Path $projectRoot 'dist\OFFSET.exe')
Write-Host ('Built OFFSET.exe: {0:N0} bytes ({1:N1} KiB)' -f $binary.Length,($binary.Length / 1KB))
if ($Run) { Start-Process -FilePath $binary.FullName }
