param([string]$JavaHome)
$ErrorActionPreference = 'Stop'
if (-not $JavaHome) { $JavaHome = $env:JAVA_HOME }
if (-not $JavaHome -or -not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\javac.exe'))) {
    $candidate = Get-ChildItem 'C:\Program Files\Java' -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like 'jdk-21*' } | Select-Object -First 1
    if ($candidate) { $JavaHome = $candidate.FullName }
}
if (-not $JavaHome -or -not (Test-Path -LiteralPath (Join-Path $JavaHome 'bin\javac.exe'))) {
    throw 'The version launch probe needs a JDK. Supply -JavaHome or set JAVA_HOME.'
}
$fixtureRoot = Join-Path $PSScriptRoot ('out\Profile Unicode 路径 ' + [DateTime]::UtcNow.Ticks)
$classesDir = Join-Path $fixtureRoot 'classes'
$libraryDir = Join-Path $fixtureRoot 'libraries\test\profile\library\1'
$assetsDir = Join-Path $fixtureRoot 'assets'
New-Item -ItemType Directory -Force $classesDir, $libraryDir, $assetsDir | Out-Null
& (Join-Path $JavaHome 'bin\javac.exe') -encoding UTF-8 -d $classesDir `
    (Join-Path $PSScriptRoot 'probe\ProfileLibrary.java') (Join-Path $PSScriptRoot 'probe\ProfileProbe.java')
if ($LASTEXITCODE -ne 0) { throw 'Profile probe compilation failed.' }
& (Join-Path $JavaHome 'bin\jar.exe') cf (Join-Path $libraryDir 'library-1.jar') -C $classesDir ProfileLibrary.class
if ($LASTEXITCODE -ne 0) { throw 'Library probe JAR creation failed.' }
[IO.File]::WriteAllText((Join-Path $assetsDir 'profile-asset.txt'), 'shared-root-asset')
foreach ($profile in 'probe-shared', 'probe-isolated') {
    $versionDir = Join-Path $fixtureRoot ('versions\' + $profile)
    New-Item -ItemType Directory -Force $versionDir | Out-Null
    & (Join-Path $JavaHome 'bin\jar.exe') cf (Join-Path $versionDir ($profile + '.jar')) -C $classesDir ProfileProbe.class
    if ($LASTEXITCODE -ne 0) { throw 'Client probe JAR creation failed.' }
    $memory = if ($profile -eq 'probe-shared') { '768' } else { '1024' }
    $metadata = @{
        id=$profile; type='release'; mainClass='ProfileProbe'; assetIndex=@{id='probe'}
        libraries=@(@{name='test.profile:library:1'; downloads=@{artifact=@{path='test/profile/library/1/library-1.jar'}}})
        arguments=@{
            jvm=@('-cp','${classpath}','-Djava.library.path=${natives_directory}', '-Dprobe.libraries=${library_directory}')
            game=@('--profile',$profile,'--memory',$memory,'--gameDir','${game_directory}','--assetsDir','${assets_root}')
        }
    }
    $metadata | ConvertTo-Json -Depth 10 -Compress | Set-Content -Encoding utf8 (Join-Path $versionDir ($profile + '.json'))
}
$env:OFFSET_TEST_GAME = $fixtureRoot
$env:OFFSET_TEST_JAVA = Join-Path $JavaHome 'bin\java.exe'
Write-Host "Profile probe fixture: $fixtureRoot"
