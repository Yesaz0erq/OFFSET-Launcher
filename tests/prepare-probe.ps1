param([string]$JavaHome)
$ErrorActionPreference = 'Stop'
if (-not $JavaHome) { $JavaHome = $env:JAVA_HOME }
if (-not $JavaHome -or -not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\javac.exe'))) {
    $compiler = Get-Command javac.exe -ErrorAction SilentlyContinue
    if ($compiler) { $JavaHome = Split-Path (Split-Path $compiler.Source) }
}
if (-not $JavaHome -or -not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\javac.exe'))) {
    $candidate = Get-ChildItem 'C:\Program Files\Java' -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like 'jdk-21*' } | Select-Object -First 1
    if ($candidate) { $JavaHome = $candidate.FullName }
}
if (-not $JavaHome -or -not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\javac.exe'))) {
    throw 'Java probe requires a JDK (Java 11+). Supply -JavaHome or set JAVA_HOME.'
}
$fixtureRoot = Join-Path $PSScriptRoot 'out\Probe Unicode 路径'
$versionDir = Join-Path $fixtureRoot 'versions\probe'
$classesDir = Join-Path $fixtureRoot 'classes'
New-Item -ItemType Directory -Force $versionDir, $classesDir | Out-Null
& (Join-Path $JavaHome 'bin\javac.exe') -encoding UTF-8 -d $classesDir (Join-Path $PSScriptRoot 'probe\Probe.java')
if ($LASTEXITCODE -ne 0) { throw 'Probe compilation failed.' }
& (Join-Path $JavaHome 'bin\jar.exe') cf (Join-Path $versionDir 'probe.jar') -C $classesDir Probe.class
if ($LASTEXITCODE -ne 0) { throw 'Probe JAR creation failed.' }
$metadata = @{
 id='probe'; type='release'; mainClass='Probe'; assetIndex=@{id='probe'}; libraries=@()
 arguments=@{
  jvm=@('-cp','${classpath}','-Djava.library.path=${natives_directory}')
  game=@('--username','${auth_player_name}','--uuid','${auth_uuid}',
   '--gameDir','${game_directory}','--assetsDir','${assets_root}',
   '--accessToken','${auth_access_token}','--userType','${user_type}',
   'a b','ends-with-\','quote-"-inside','multi-\\"end')
 }
}
$metadata | ConvertTo-Json -Depth 10 -Compress | Set-Content -Encoding utf8 (Join-Path $versionDir 'probe.json')
$env:OFFSET_TEST_GAME = $fixtureRoot
$env:OFFSET_TEST_JAVA = Join-Path $JavaHome 'bin\java.exe'
$oldResult = Join-Path $fixtureRoot 'probe-result.txt'
if (Test-Path -LiteralPath $oldResult) { Remove-Item -LiteralPath $oldResult }
Write-Host "Java probe fixture: $fixtureRoot"
