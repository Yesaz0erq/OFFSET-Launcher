param([string]$JavaHome='C:\Program Files\Java\jdk-21.0.12')
$ErrorActionPreference='Stop'
$fixture = Join-Path $PSScriptRoot 'out\GUI crash 中文路径'
$profile = Join-Path $fixture 'versions\crash-demo'
$classes = Join-Path $fixture 'classes'
New-Item -ItemType Directory -Force $profile,$classes | Out-Null
& (Join-Path $JavaHome 'bin\javac.exe') -encoding UTF-8 -d $classes (Join-Path $PSScriptRoot 'probe\GuiCrashProbe.java')
if ($LASTEXITCODE -ne 0) { throw 'Compile failed.' }
& (Join-Path $JavaHome 'bin\jar.exe') cf (Join-Path $profile 'crash-demo.jar') -C $classes GuiCrashProbe.class
if ($LASTEXITCODE -ne 0) { throw 'JAR failed.' }
$metadata = @{
    id='crash-demo';type='release';mainClass='GuiCrashProbe';assetIndex=@{id='probe'};libraries=@()
    arguments=@{jvm=@('-cp','${classpath}');game=@()}
}
$metadata | ConvertTo-Json -Depth 10 -Compress | Set-Content -LiteralPath (Join-Path $profile 'crash-demo.json') -Encoding utf8
Write-Output $fixture
