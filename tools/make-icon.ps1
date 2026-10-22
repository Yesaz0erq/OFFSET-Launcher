# Splits the multi-size application icon (assets/logo/offset.ico) into the
# single-image .ico files that main.asm embeds (src/icons/offset-<size>.ico).
# Run after replacing assets/logo/offset.ico, then rebuild with build.ps1.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $projectRoot 'assets\logo\offset.ico'
$outputDir = Join-Path $projectRoot 'src\icons'
New-Item -ItemType Directory -Force $outputDir | Out-Null
$bytes = [IO.File]::ReadAllBytes($source)
$count = [BitConverter]::ToUInt16($bytes,4)
$wanted = @(16,24,32,48,64,256)
for ($i = 0; $i -lt $count; $i++) {
    $entry = 6 + 16 * $i
    $size = [int]$bytes[$entry]
    if ($size -eq 0) { $size = 256 }
    if ($wanted -notcontains $size) { continue }
    $length = [BitConverter]::ToUInt32($bytes,$entry + 8)
    $offset = [BitConverter]::ToUInt32($bytes,$entry + 12)
    $stream = New-Object IO.MemoryStream
    $writer = New-Object IO.BinaryWriter($stream)
    $writer.Write([UInt16]0); $writer.Write([UInt16]1); $writer.Write([UInt16]1)
    $writer.Write($bytes,$entry,8)
    $writer.Write([UInt32]$length); $writer.Write([UInt32]22)
    $writer.Write($bytes,[int]$offset,[int]$length)
    $writer.Flush()
    [IO.File]::WriteAllBytes((Join-Path $outputDir ('offset-{0}.ico' -f $size)),$stream.ToArray())
    $writer.Dispose()
    Write-Host ('icon {0}x{0}: {1} bytes' -f $size,$length)
}
