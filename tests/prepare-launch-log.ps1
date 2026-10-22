param([string]$JavaHome)
$ErrorActionPreference='Stop'
if (!$JavaHome) { $JavaHome=$env:JAVA_HOME }
if (!$JavaHome -or !(Test-Path (Join-Path $JavaHome 'bin\javac.exe'))) {
    $candidate=Get-ChildItem 'C:\Program Files\Java' -Directory -Filter 'jdk-21*' | Select-Object -First 1
    if ($candidate) { $JavaHome=$candidate.FullName }
}
if (!$JavaHome) { throw 'A JDK is required for launch log verification.' }
$fixture=Join-Path $PSScriptRoot ('out\Launch logs 中文 '+[DateTime]::UtcNow.Ticks)
$classes=Join-Path $fixture 'classes'
New-Item -ItemType Directory -Force $classes | Out-Null
& (Join-Path $JavaHome 'bin\javac.exe') -encoding UTF-8 -d $classes (Join-Path $PSScriptRoot 'probe\LaunchLogProbe.java')
if ($LASTEXITCODE -ne 0) { throw 'Log probe compilation failed.' }
foreach ($mode in @('success','crash','spam')) {
    $version=Join-Path $fixture "versions\$mode"
    New-Item -ItemType Directory -Force $version | Out-Null
    & (Join-Path $JavaHome 'bin\jar.exe') cf (Join-Path $version "$mode.jar") -C $classes LaunchLogProbe.class
    if ($LASTEXITCODE -ne 0) { throw 'Log probe JAR failed.' }
    $metadata=@{id=$mode; type='release'; mainClass='LaunchLogProbe'; assetIndex=@{id='probe'}; libraries=@(); arguments=@{jvm=@('-cp','${classpath}','-Dfile.encoding=COMPAT'); game=@('--mode',$mode,'--token','${auth_access_token}')}}
    $metadata | ConvertTo-Json -Depth 10 -Compress | Set-Content -Encoding utf8 (Join-Path $version "$mode.json")
}
New-Item -ItemType Directory -Force (Join-Path $fixture 'crash-reports') | Out-Null
[IO.File]::WriteAllText((Join-Path $fixture 'crash-reports\stale.txt'),'STALE-MUST-NOT-BE-ATTACHED')
(Get-Item -LiteralPath (Join-Path $fixture 'crash-reports\stale.txt')).LastWriteTimeUtc=[DateTime]'2000-01-01'
[IO.File]::WriteAllText((Join-Path $fixture 'crash-reports\future.txt'),'STALE-MUST-NOT-BE-ATTACHED')
(Get-Item -LiteralPath (Join-Path $fixture 'crash-reports\future.txt')).LastWriteTimeUtc=[DateTime]'2099-01-01'
[IO.File]::WriteAllText((Join-Path $fixture 'invalid-java.exe'),'not executable')
$failure=Join-Path $fixture 'log-save-failure'
New-Item -ItemType Directory -Force $failure | Out-Null
[IO.File]::WriteAllText((Join-Path $failure '.offset-logs'),'preserve this existing file')
$env:OFFSET_LOG_GAME=$fixture
$env:OFFSET_LOG_JAVA=Join-Path $JavaHome 'bin\javaw.exe'
Write-Host "Log probe fixture: $fixture"
