param([switch]$Run)
$ErrorActionPreference='Stop'
$project=Split-Path -Parent $PSScriptRoot
$root=Join-Path $PSScriptRoot ('out\pack-export\run-' + [DateTime]::UtcNow.Ticks)
New-Item -ItemType Directory -Force $root,(Join-Path $root 'output') | Out-Null
function Write-Fixture([string]$Relative,[string]$Text) {
    $path=Join-Path $root $Relative
    New-Item -ItemType Directory -Force (Split-Path -Parent $path) | Out-Null
    [IO.File]::WriteAllText($path,$Text,[Text.UTF8Encoding]::new($false))
}
function Add-Profile([string]$Source,[string]$Id,[string]$Library='') {
    $metadata=@{id=$Id; mainClass='Probe'; arguments=@{jvm=@('-cp','${classpath}'); game=@()};
        assetIndex=@{id='probe'}; libraries=@()}
    if ($Library) { $metadata['offsetNormalized']=$true; $metadata['offsetBase']='1.21.1'; $metadata.libraries=@(@{name=$Library}) }
    Write-Fixture "$Source\versions\$Id\$Id.json" ($metadata | ConvertTo-Json -Depth 8 -Compress)
    Write-Fixture "$Source\versions\$Id\$Id.jar" 'PK fixture client'
}
Add-Profile 'source' '1.21.1'
Add-Profile 'loader' 'fabric-test' 'net.fabricmc:fabric-loader:0.19.3'
Add-Profile 'forge' 'forge-test' 'net.minecraftforge:forge:1.21.1-52.1.20:client'
Add-Profile 'neo' 'neo-test' 'net.neoforged:neoforge:21.1.264'
Add-Profile 'neo-modern' 'neo-test'
$modernNeo=@{id='neo-test';offsetNormalized=$true;offsetBase='1.21.1';
    mainClass='cpw.mods.bootstraplauncher.BootstrapLauncher';libraries=@();
    arguments=@{jvm=@('-cp','${classpath}');game=@('--fml.neoForgeVersion','21.1.264','--fml.mcVersion','1.21.1')};
    assetIndex=@{id='probe'}}
Write-Fixture 'neo-modern\versions\neo-test\neo-test.json' ($modernNeo|ConvertTo-Json -Depth 8 -Compress)
Add-Profile 'quilt' 'quilt-test' 'org.quiltmc:quilt-loader:0.30.1'
Add-Profile 'isolated' 'isolate-test'
Write-Fixture 'isolated\versions\isolate-test\.offset-profile.ini' "[Profile]`r`nIsolate=1`r`n"
Write-Fixture 'isolated\versions\isolate-test\game\mods\isolated.jar' 'isolated payload'
Write-Fixture 'isolated\mods\wrong-root.jar' 'must not export this shared root'
$payload=[ordered]@{
    'mods\a.jar'='PK source mod A'; 'mods\b.jar.disabled'='PK disabled mod B';
    'config\options.json'='{"enabled":true}'; 'defaultconfigs\test.toml'='enabled=true';
    'kubejs\server_scripts\main.js'='// actual game script'; 'scripts\test.zs'='// game script';
    'resources\config.json'='{}'; 'resourcepacks\test.zip'='PK resource pack';
    'shaderpacks\test.zip'='PK shader pack'; 'datapacks\test.zip'='PK data pack';
    'options.txt'='music:0.5'; 'optionsof.txt'='ofFastRender:false'; 'optionsshaders.txt'='shaderPack=OFF'
}
foreach ($relative in $payload.Keys) { Write-Fixture "source\$relative" $payload[$relative] }
$unicode=('中' * 70)+'.json'
Write-Fixture "source\config\$unicode" 'Unicode name, exact bytes'
Write-Fixture 'source\saves\World\level.dat' 'saved world payload'
foreach ($relative in @('servers.dat','accounts.json','launcher_accounts.json','offset.ini','offset.cfkey',
    'offset.account','offset.account.new','config\offset.account','resources\offset.account.new',
    'config\offset.account.part','config\offset.account.part-1234-5678-9',
    'logs\latest.log','crash-reports\crash.txt','screenshots\secret.png',
    'config\accounts.json','config\credentials.json','config\cache\cache.dat',
    'config\.offset-private\hidden.json','resources\private.log')) {
    Write-Fixture "source\$relative" 'excluded fixture, must never enter an export'
}
foreach ($source in @('loader','forge','neo','quilt')) { Write-Fixture "$source\mods\test.jar" 'PK loader payload' }
Add-Profile 'reparse' '1.21.1'
Write-Fixture 'outside\secret.txt' 'outside source'
New-Item -ItemType Directory -Force (Join-Path $root 'reparse\mods') | Out-Null
New-Item -ItemType Junction -Path (Join-Path $root 'reparse\mods\linked') -Target (Join-Path $root 'outside') | Out-Null
New-Item -ItemType Junction -Path (Join-Path $root 'output-linked') -Target (Join-Path $root 'outside') | Out-Null
Add-Profile 'oversize' '1.21.1'
$oversize=Join-Path $root 'oversize\mods\large.jar'
New-Item -ItemType Directory -Force (Split-Path -Parent $oversize) | Out-Null
$stream=[IO.File]::Open($oversize,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write)
try { $stream.SetLength(134217729) } finally { $stream.Dispose() }
Write-Fixture 'output\existing.zip' 'preserve this pre-existing output'
$before=@{}
foreach ($source in @('source','loader','forge','neo','quilt','isolated')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $root $source) -File -Recurse) {
        $before[$file.FullName]=(Get-FileHash -LiteralPath $file.FullName).Hash
    }
}
$existingHash=(Get-FileHash -LiteralPath (Join-Path $root 'output\existing.zip')).Hash
if ($Run) {
    $old=$env:INCLUDE
    try {
        $env:INCLUDE=Join-Path $project 'tools\INCLUDE'
        $exe=Join-Path $root 'pack-export.exe'
        & (Join-Path $project 'tools\FASM.EXE') -m 262144 (Join-Path $PSScriptRoot 'pack-export.asm') $exe
        if ($LASTEXITCODE -ne 0) { throw 'Export fixture assembly failed' }
    } finally { $env:INCLUDE=$old }
    $env:OFFSET_EXPORT_TEST_ROOT=$root
    & $exe *> (Join-Path $root 'native.log')
    $result=$LASTEXITCODE
    Remove-Item Env:\OFFSET_EXPORT_TEST_ROOT
    if ($result -ne 0) { throw "Export fixture failed at step $result; see $root\native.log" }
    # Interoperability check against .NET ZIP/deflate reader, independent of
    # our exporter/importer: CRC payloads and metadata filenames must agree.
    $archiveChecks=@()
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $root 'output') -File | Where-Object Extension -In '.zip','.mrpack') {
        if ($file.Name -eq 'existing.zip') { continue }
        $zip=[IO.Compression.ZipFile]::OpenRead($file.FullName)
        try {
            $entries=@($zip.Entries | ForEach-Object FullName)
            foreach ($entry in $zip.Entries) {
                  if ($entry.FullName -match '(?i)(accounts|offset\.account|credentials|servers\.dat|cache/|\.offset-|latest\.log|screenshots|versions/)') {
                    throw "Sensitive/cache entry in export: $($entry.FullName)"
                }
                if ($file.Name -ne 'with-saves.zip' -and $entry.FullName -match '^overrides/saves/') { throw 'Saves exported without selection' }
                if ($file.Name -eq 'config-only.zip' -and $entry.FullName -match '^overrides/(mods|resources|resourcepacks|shaderpacks|datapacks)/') { throw 'Unselected content group exported' }
                $reader=$entry.Open()
                try { $memory=[IO.MemoryStream]::new(); $reader.CopyTo($memory); $memory.Dispose() } finally { $reader.Dispose() }
            }
            if ($file.Name -eq 'with-saves.zip' -and $entries -notcontains 'overrides/saves/World/level.dat') { throw 'Selected save missing' }
            $archiveChecks+=@{name=$file.Name; entries=$entries.Count; bytes=$file.Length}
        } finally { $zip.Dispose() }
    }
    if ((Get-FileHash -LiteralPath (Join-Path $root 'output\existing.zip')).Hash -ne $existingHash) { throw 'Existing output was modified' }
    foreach ($file in $before.Keys) { if ((Get-FileHash -LiteralPath $file).Hash -ne $before[$file]) { throw "Source changed: $file" } }
    @{exit_code=0; source_hashes_unchanged=$true; existing_output_preserved=$true; archives=$archiveChecks} |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $root 'validation.json') -Encoding utf8
}
Write-Host "Export fixture: $root"
