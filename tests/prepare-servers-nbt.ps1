# Standard binary NBT fixtures. Requires only PowerShell/.NET; no real game data.
$ErrorActionPreference = 'Stop'
$fixtureRoot = Join-Path $PSScriptRoot 'fixtures\servers-nbt'
New-Item -ItemType Directory -Force -Path $fixtureRoot | Out-Null

function Write-NbtBytes([IO.MemoryStream]$Stream, [byte[]]$Bytes) {
    $Stream.Write($Bytes, 0, $Bytes.Length)
}
function Write-NbtInt([IO.MemoryStream]$Stream, [int]$Value) {
    $bytes = [BitConverter]::GetBytes($Value)
    if ([BitConverter]::IsLittleEndian) { [Array]::Reverse($bytes) }
    Write-NbtBytes $Stream $bytes
}
function New-NbtMutf([string]$Text) {
    $body = [IO.MemoryStream]::new()
    foreach ($unit in $Text.ToCharArray()) {
        $code = [int]$unit
        if ($code -gt 0 -and $code -lt 128) {
            $body.WriteByte([byte]$code)
        } elseif ($code -lt 2048) {
            $body.WriteByte([byte](0xc0 -bor ($code -shr 6)))
            $body.WriteByte([byte](0x80 -bor ($code -band 63)))
        } else {
            $body.WriteByte([byte](0xe0 -bor ($code -shr 12)))
            $body.WriteByte([byte](0x80 -bor (($code -shr 6) -band 63)))
            $body.WriteByte([byte](0x80 -bor ($code -band 63)))
        }
    }
    if ($body.Length -gt 65535) { throw 'MUTF fixture exceeds its 16-bit length.' }
    $result = [IO.MemoryStream]::new()
    $result.WriteByte([byte]($body.Length -shr 8))
    $result.WriteByte([byte]($body.Length -band 255))
    Write-NbtBytes $result $body.ToArray()
    $body.Dispose()
    $bytes = $result.ToArray()
    $result.Dispose()
    return ,$bytes
}
function New-NbtTag([int]$Kind, [string]$Name, [byte[]]$Data) {
    $result = [IO.MemoryStream]::new()
    $result.WriteByte([byte]$Kind)
    Write-NbtBytes $result (New-NbtMutf $Name)
    Write-NbtBytes $result $Data
    $bytes = $result.ToArray()
    $result.Dispose()
    return ,$bytes
}
function New-NbtServer([string]$Name = 'Local', [string]$Ip = 'localhost:25565', [byte[]]$Extra = @()) {
    $result = [IO.MemoryStream]::new()
    Write-NbtBytes $result (New-NbtTag 8 'name' (New-NbtMutf $Name))
    Write-NbtBytes $result (New-NbtTag 8 'ip' (New-NbtMutf $Ip))
    Write-NbtBytes $result $Extra
    $result.WriteByte(0)
    $bytes = $result.ToArray()
    $result.Dispose()
    return ,$bytes
}
function New-NbtRows([int]$Count, [byte[]]$Row) {
    $rows = [Collections.Generic.List[byte[]]]::new()
    for ($index = 0; $index -lt $Count; $index++) { $rows.Add($Row) }
    return ,$rows
}
function New-NbtDocument([Collections.Generic.List[byte[]]]$Rows, [byte[]]$Extra = @()) {
    $payload = [IO.MemoryStream]::new()
    $payload.WriteByte(10)
    Write-NbtInt $payload $Rows.Count
    foreach ($row in $Rows) { Write-NbtBytes $payload $row }
    $result = [IO.MemoryStream]::new()
    Write-NbtBytes $result ([byte[]]@(10, 0, 0))
    Write-NbtBytes $result (New-NbtTag 9 'servers' $payload.ToArray())
    Write-NbtBytes $result $Extra
    $result.WriteByte(0)
    $payload.Dispose()
    $bytes = $result.ToArray()
    $result.Dispose()
    return ,$bytes
}

# Construct surrogate code units explicitly so the script is also ASCII-safe
# when read by Windows PowerShell 5.1 without a UTF-8 BOM.
$globe = ([string][char]0xd83c) + [char]0xdf0d
$localName = ([string][char]0x672c) + [char]0x5730 + ' ' + $globe
$unknown = [IO.MemoryStream]::new()
Write-NbtBytes $unknown (New-NbtTag 1 'byte' ([byte[]]@(1)))
Write-NbtBytes $unknown (New-NbtTag 2 'short' ([byte[]]@(1, 2)))
Write-NbtBytes $unknown (New-NbtTag 3 'int' ([byte[]]@(0, 0, 0, 1)))
Write-NbtBytes $unknown (New-NbtTag 4 'long' ([byte[]]::new(8)))
Write-NbtBytes $unknown (New-NbtTag 5 'float' ([byte[]]::new(4)))
Write-NbtBytes $unknown (New-NbtTag 6 'double' ([byte[]]::new(8)))
Write-NbtBytes $unknown (New-NbtTag 7 'bytes' ([byte[]]@(0, 0, 0, 5, 104, 101, 108, 108, 111)))
Write-NbtBytes $unknown (New-NbtTag 8 'icon' (New-NbtMutf 'data:image/png;base64,unused'))
Write-NbtBytes $unknown (New-NbtTag 9 'empty' ([byte[]]::new(5)))
$nested = [IO.MemoryStream]::new()
Write-NbtBytes $nested (New-NbtTag 8 'text' (New-NbtMutf ('skip' + [char]0 + 'and' + $globe)))
$nested.WriteByte(0)
Write-NbtBytes $unknown (New-NbtTag 10 'nested' $nested.ToArray())
$nested.Dispose()
Write-NbtBytes $unknown (New-NbtTag 11 'ints' ([byte[]](@(0, 0, 0, 2) + [byte[]]::new(8))))
Write-NbtBytes $unknown (New-NbtTag 12 'longs' ([byte[]](@(0, 0, 0, 2) + [byte[]]::new(16))))
$unknownBytes = $unknown.ToArray()
$unknown.Dispose()
$validRows = [Collections.Generic.List[byte[]]]::new()
$validRows.Add((New-NbtServer $localName '[::1]:25565' $unknownBytes))
$validRows.Add((New-NbtServer 'Second' 'example.invalid'))
$valid = New-NbtDocument $validRows $unknownBytes
$empty = [byte[]](@(10, 0, 0) + (New-NbtTag 9 'servers' ([byte[]]::new(5))) + @(0))
$badUtfRow = [byte[]]((New-NbtTag 8 'name' ([byte[]]@(0, 4, 0xf0, 0x9f, 0x8c, 0x8d))) +
    (New-NbtTag 8 'ip' (New-NbtMutf 'localhost')) + @(0))
$missingIpRow = [byte[]]((New-NbtTag 8 'name' (New-NbtMutf 'Missing')) + @(0))
$deepPayload = [IO.MemoryStream]::new()
for ($index = 0; $index -lt 34; $index++) { Write-NbtBytes $deepPayload ([byte[]]@(10, 0, 1, 120)) }
Write-NbtBytes $deepPayload ([byte[]]::new(35))
$deepTag = New-NbtTag 10 'level' $deepPayload.ToArray()
$deepPayload.Dispose()

# Insertion order is part of the native fixture-label contract.
$fixtures = [ordered]@{
    'valid.dat' = $valid
    'empty.dat' = $empty
    'truncated.dat' = [byte[]]$valid[0..($valid.Length - 2)]
    'trailing.dat' = [byte[]]($valid + @(98, 97, 100))
    'oversized-list.dat' = (New-NbtDocument (New-NbtRows 257 (New-NbtServer)))
    'long-name.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer ('a' * 128))))
    'long-address.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer -Ip ('a' * 256))))
    'bad-utf.dat' = (New-NbtDocument (New-NbtRows 1 $badUtfRow))
    'nul-name.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer ('a' + [char]0 + 'b'))))
    'negative-array.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer -Extra (New-NbtTag 7 'bad' ([byte[]]@(255, 255, 255, 255))))))
    'duplicate.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer -Extra (New-NbtTag 8 'name' (New-NbtMutf 'Duplicate')))))
    'missing-ip.dat' = (New-NbtDocument (New-NbtRows 1 $missingIpRow))
    'negative-list.dat' = [byte[]](@(10, 0, 0) + (New-NbtTag 9 'servers' ([byte[]]@(10, 255, 255, 255, 255))) + @(0))
    'unknown-tag.dat' = [byte[]](@(10, 0, 0) + (New-NbtTag 13 'bad' ([byte[]]@())) + @(0))
    'deep.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer -Extra $deepTag)))
    'boundary.dat' = (New-NbtDocument (New-NbtRows 1 (New-NbtServer ('a' * 127) ('b' * 255))))
    'capacity.dat' = (New-NbtDocument (New-NbtRows 256 (New-NbtServer)))
}
$labels = [Collections.Generic.List[string]]::new()
$index = 0
foreach ($entry in $fixtures.GetEnumerator()) {
    [IO.File]::WriteAllBytes((Join-Path $fixtureRoot $entry.Key), $entry.Value)
    $labels.Add(('nbtFixture{0}: file "{1}"' -f $index, $entry.Key))
    $labels.Add(('nbtFixture{0}End:' -f $index))
    $index++
}
[IO.File]::WriteAllText((Join-Path $fixtureRoot 'fixtures.inc'), (($labels -join "`r`n") + "`r`n"), [Text.Encoding]::ASCII)
$unicodeName = ([string][char]0x8def) + [char]0x5f84
$profile = Join-Path (Join-Path $fixtureRoot $unicodeName) 'versions\fixture'
$game = Join-Path $profile 'game'
New-Item -ItemType Directory -Force -Path $game | Out-Null
[IO.File]::WriteAllText((Join-Path $profile '.offset-profile.ini'), "[Profile]`r`nIsolate=1`r`n", [Text.Encoding]::ASCII)
[IO.File]::WriteAllBytes((Join-Path $game 'servers.dat'), $valid)
$largeContext = Join-Path $fixtureRoot 'oversized'
New-Item -ItemType Directory -Force -Path $largeContext | Out-Null
$largeFile = [IO.File]::Open((Join-Path $largeContext 'servers.dat'), [IO.FileMode]::Create, [IO.FileAccess]::Write)
try { $largeFile.SetLength(16777217) } finally { $largeFile.Dispose() }
Write-Host ('{0} NBT fixtures prepared' -f $fixtures.Count)
