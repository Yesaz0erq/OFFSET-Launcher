$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$executable = Join-Path $projectRoot 'dist\OFFSET.exe'
if (!(Test-Path -LiteralPath $executable)) { throw 'Build first: .\build.ps1' }
$destination = Join-Path $projectRoot 'dist\OFFSET-0.9.0-windows.zip'
Compress-Archive -LiteralPath @($executable,(Join-Path $projectRoot 'README.md'),(Join-Path $projectRoot 'LICENSE'),(Join-Path $projectRoot 'THIRD_PARTY.md')) -DestinationPath $destination -Force
$digest = Get-FileHash -LiteralPath $executable -Algorithm SHA256
[IO.File]::WriteAllText((Join-Path $projectRoot 'dist\SHA256.txt'),"$($digest.Hash.ToLower())  OFFSET.exe`r`n",[Text.Encoding]::ASCII)
Write-Host "Package: $destination"
Get-Item -LiteralPath $executable,$destination | Select-Object Name,Length
