param([string]$JavaHome='C:\Program Files\Java\jdk-21.0.12',[switch]$Run,[string]$Archive)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$cacheRoot=Join-Path $PSScriptRoot 'out\pack-real-fo'
$fixtureRoot=Join-Path $PSScriptRoot ('out\pack-import-live\run-' + [DateTime]::UtcNow.Ticks)
$archiveRoot=Join-Path $fixtureRoot 'archives'
$instancesRoot=Join-Path $fixtureRoot 'instances'
$java=Join-Path $JavaHome 'bin\javaw.exe'
$modFile=Join-Path $cacheRoot 'mods\fabric-api-0.116.12+1.21.1.jar'
$sourceJson=Join-Path $cacheRoot 'versions\1.21.1\1.21.1.json'
$sourceJar=Join-Path $cacheRoot 'versions\1.21.1\1.21.1.jar'
foreach ($file in @($java,$modFile,$sourceJson,$sourceJar)) {
    if (!(Test-Path -LiteralPath $file -PathType Leaf)) { throw "Missing live-test input: $file" }
}
$before=@{}
foreach ($file in @($modFile,$sourceJson,$sourceJar)) { $before[$file]=(Get-FileHash -LiteralPath $file).Hash }
# The source caches are linked read-only in purpose: verify every original
# byte after installation as well, so a loader cannot silently mutate a link.
$cacheBefore=@{}
foreach ($folder in @('assets','libraries')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $cacheRoot $folder) -File -Recurse) {
        $cacheBefore[$file.FullName]=(Get-FileHash -LiteralPath $file.FullName).Hash
    }
}
New-Item -ItemType Directory -Force $archiveRoot,$instancesRoot | Out-Null
$configFile=Join-Path $fixtureRoot 'offset-import-live.txt'
[IO.File]::WriteAllText($configFile,"OFFSET actual CurseForge and Prism import verification`n",[Text.UTF8Encoding]::new($false))
$modSha=(Get-FileHash -LiteralPath $modFile -Algorithm SHA1).Hash.ToLower()
$configSha=(Get-FileHash -LiteralPath $configFile -Algorithm SHA1).Hash.ToLower()
$mrName='fabric-api-0.116.12+1.21.1.jar'
function Add-TextEntry($zip,[string]$Name,[string]$Text) {
    $entry=$zip.CreateEntry($Name,[IO.Compression.CompressionLevel]::Optimal)
    $stream=$entry.Open()
    try { $bytes=[Text.UTF8Encoding]::new($false).GetBytes($Text); $stream.Write($bytes,0,$bytes.Length) }
    finally { $stream.Dispose() }
}
$archives=@{}
$payloads=@{}
if ($Archive) {
    $archive=(Resolve-Path -LiteralPath $Archive).Path
    $archives['export-roundtrip']=$archive
    $before[$archive]=(Get-FileHash -LiteralPath $archive).Hash
    # The real native harness validates this exact MC/Fabric test version.
    # Read expected payload hashes from the actual exported archive, not a
    # parallel fixture copy, and validate every bundled file after import.
    $zip=[IO.Compression.ZipFile]::OpenRead($archive)
    try {
        foreach ($entry in $zip.Entries) {
            if (!$entry.FullName.StartsWith('overrides/',[StringComparison]::Ordinal) -or !$entry.Name) { continue }
            $relative=$entry.FullName.Substring('overrides/'.Length)
            if ($relative.Split('/') -contains '..') { throw 'Unsafe test archive payload' }
            $stream=$entry.Open()
            $sha=[Security.Cryptography.SHA1]::Create()
            try { $digest=[Convert]::ToHexString($sha.ComputeHash($stream)).ToLower() }
            finally { $sha.Dispose(); $stream.Dispose() }
            $payloads[$relative]=@{bytes=$entry.Length; sha1=$digest}
        }
    } finally { $zip.Dispose() }
    $modSha=$payloads["mods/$mrName"].sha1
    $configSha=$payloads['config/offset-import-live.txt'].sha1
    if (!$modSha -or !$configSha) { throw 'External archive lacks this live harness test payload' }
} else {
foreach ($kind in @('curseforge','prism')) {
    $archive=Join-Path $archiveRoot ($kind + '.zip')
    $zip=[IO.Compression.ZipFile]::Open($archive,[IO.Compression.ZipArchiveMode]::Create)
    try {
        if ($kind -eq 'curseforge') {
            $manifest=@{manifestType='minecraftModpack'; manifestVersion=1; name='OFFSET CF live import'; version='test';
                minecraft=@{version='1.21.1'; modLoaders=@(@{id='fabric-0.19.3'; primary=$true})};
                files=@(); overrides='overrides'} | ConvertTo-Json -Depth 8 -Compress
            Add-TextEntry $zip 'manifest.json' $manifest
            $prefix='overrides'
        } else {
            $manifest=@{formatVersion=1; components=@(@{uid='net.minecraft'; version='1.21.1'},@{uid='net.fabricmc.fabric-loader'; version='0.19.3'})} | ConvertTo-Json -Depth 8 -Compress
            Add-TextEntry $zip 'mmc-pack.json' $manifest
            Add-TextEntry $zip 'instance.cfg' "[General]`nname=OFFSET Prism live import`n"
            $prefix='.minecraft'
        }
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$modFile,"$prefix/mods/$mrName",[IO.Compression.CompressionLevel]::Optimal) | Out-Null
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$configFile,"$prefix/config/offset-import-live.txt",[IO.Compression.CompressionLevel]::Optimal) | Out-Null
    } finally { $zip.Dispose() }
    $archives[$kind]=$archive
}
}
foreach ($kind in $archives.Keys) {
    $archive=$archives[$kind]
    $stage=(Join-Path $instancesRoot $kind) + '.offset-stage'
    New-Item -ItemType Directory -Force $stage | Out-Null
    $owner=(Get-FileHash -LiteralPath $archive -Algorithm SHA1).Hash.ToLower()
    [IO.File]::WriteAllText((Join-Path $stage '.offset-pack-stage'),$owner,[Text.Encoding]::ASCII)
    foreach ($folder in @('assets','libraries')) {
        $source=Join-Path $cacheRoot $folder
        foreach ($file in Get-ChildItem -LiteralPath $source -File -Recurse) {
            $relative=$file.FullName.Substring($cacheRoot.Length+1)
            $target=Join-Path $stage $relative
            New-Item -ItemType Directory -Force (Split-Path -Parent $target) | Out-Null
            New-Item -ItemType HardLink -Path $target -Target $file.FullName | Out-Null
        }
    }
    # Metadata/client are private copies: normalization/download never edits a
    # source profile. Hard links above are only immutable library/asset caches.
    $version=Join-Path $stage 'versions\1.21.1'
    New-Item -ItemType Directory -Force $version | Out-Null
    Copy-Item -LiteralPath $sourceJson,$sourceJar -Destination $version
}
$data=[ordered]@{root=$fixtureRoot; java=$java; source_cache=$cacheRoot; mod_sha1=$modSha; config_sha1=$configSha; archives=$archives; instances=@{}; results=@{}; payloads=$payloads }
if ($Run) {
    $assembler=Join-Path $projectRoot 'tools\FASM.EXE'
    $exe=Join-Path $fixtureRoot 'pack-import-live.exe'
    $oldInclude=$env:INCLUDE
    try {
        $env:INCLUDE=Join-Path $projectRoot 'tools\INCLUDE'
        & $assembler -m 262144 (Join-Path $PSScriptRoot 'pack-import-live.asm') $exe
        if ($LASTEXITCODE -ne 0) { throw 'Live fixture assembly failed' }
    } finally { $env:INCLUDE=$oldInclude }
    foreach ($kind in $archives.Keys) {
        $destination=Join-Path $instancesRoot $kind
        $start=[Diagnostics.ProcessStartInfo]::new()
        $start.FileName=$exe
        $start.WorkingDirectory=$projectRoot
        $start.UseShellExecute=$false
        $start.CreateNoWindow=$true
        $start.RedirectStandardOutput=$true
        $start.RedirectStandardError=$true
        $start.Environment['OFFSET_PACK_LIVE_ROOT']=$cacheRoot
        $start.Environment['OFFSET_PACK_LIVE_ARCHIVE']=$archives[$kind]
        $start.Environment['OFFSET_PACK_LIVE_DEST']=$destination
        $start.Environment['OFFSET_PACK_LIVE_JAVA']=$java
        $start.Environment['OFFSET_PACK_LIVE_MOD_SHA']=$modSha
        $start.Environment['OFFSET_PACK_LIVE_CONFIG_SHA']=$configSha
        $process=[Diagnostics.Process]::Start($start)
        $stdout=$process.StandardOutput.ReadToEndAsync()
        $stderr=$process.StandardError.ReadToEndAsync()
        if (!$process.WaitForExit(300000)) { $process.Kill(); throw "$kind import timed out" }
        $exitCode=$process.ExitCode
        $output=$stdout.GetAwaiter().GetResult()+$stderr.GetAwaiter().GetResult()
        $process.Dispose()
        [IO.File]::WriteAllText((Join-Path $fixtureRoot "$kind.log"),$output,[Text.UTF8Encoding]::new($false))
        Write-Host "$kind real import exit: $exitCode"
        if ($exitCode -ne 0) { throw "$kind import failed; see $fixtureRoot\$kind.log" }
        $data.instances[$kind]=@{root=$destination; version='fabric-loader-0.19.3-1.21.1'}
        $data.results[$kind]=@{exit_code=0; mod_sha1_verified=$true; config_sha1_verified=$true; engine_classpath_verified=$true; launcher_selection_restored=$true}
        foreach ($relative in $payloads.Keys) {
            $actual=Get-Item -LiteralPath (Join-Path $destination $relative)
            if ($actual.Length -ne $payloads[$relative].bytes -or (Get-FileHash -LiteralPath $actual.FullName -Algorithm SHA1).Hash.ToLower() -ne $payloads[$relative].sha1) {
                throw "Roundtrip payload differs: $relative"
            }
        }
        $data.results[$kind]['all_archive_payloads_verified']=$payloads.Count
    }
}
$cacheAfterCount=0
foreach ($folder in @('assets','libraries')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $cacheRoot $folder) -File -Recurse) {
        $cacheAfterCount++
        if (!$cacheBefore.ContainsKey($file.FullName) -or (Get-FileHash -LiteralPath $file.FullName).Hash -ne $cacheBefore[$file.FullName]) {
            throw "Immutable source cache changed: $($file.FullName)"
        }
    }
}
if ($cacheAfterCount -ne $cacheBefore.Count) { throw 'Immutable source cache lost files' }
$data['source_cache_files_unchanged']=$cacheAfterCount
foreach ($file in $before.Keys) {
    if ((Get-FileHash -LiteralPath $file).Hash -ne $before[$file]) { throw "Source cache changed: $file" }
}
$data['source_profiles_and_mod_unchanged']=$true
$data | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $fixtureRoot 'validation.json') -Encoding utf8
Write-Host "Live import fixture: $fixtureRoot"
$data.instances | ConvertTo-Json -Depth 4
