param([switch]$Network,[switch]$JavaProbe,[string]$JavaHome)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$assembler = Join-Path $projectRoot 'tools\FASM.EXE'
if (!(Test-Path -LiteralPath $assembler)) { throw 'Missing tools/FASM.EXE. See README.md.' }
$outputDir = Join-Path $projectRoot 'tests\out\bin'
New-Item -ItemType Directory -Force $outputDir | Out-Null
$previousInclude = $env:INCLUDE
function Invoke-AssemblyTest([string]$Name,[int]$TimeoutSeconds = 30) {
    $source = Join-Path $projectRoot "tests\$Name.asm"
    $executable = Join-Path $outputDir "$Name.exe"
    & $assembler '-m' '262144' $source $executable
    if ($LASTEXITCODE -ne 0) { throw "Assembly failed: $Name" }
    $start = New-Object Diagnostics.ProcessStartInfo
    $start.FileName = $executable
    $start.WorkingDirectory = $projectRoot
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $process = [Diagnostics.Process]::Start($start)
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
        $process.Kill()
        throw "Test timed out: $Name"
    }
    $exitCode = $process.ExitCode
    $output = $stdout.GetAwaiter().GetResult() + $stderr.GetAwaiter().GetResult()
    $process.Dispose()
    if ($output.Trim()) { Write-Host $output.Trim() }
    if ($exitCode -ne 0) { throw "Test failed: $Name (exit $exitCode)" }
    Write-Host "PASS $Name"
}
try {
    $env:INCLUDE = Join-Path $projectRoot 'tools\INCLUDE'
    Push-Location $projectRoot
    Invoke-AssemblyTest 'common'
    Invoke-AssemblyTest 'download-progress'
    Invoke-AssemblyTest 'browse-defaults'
    Invoke-AssemblyTest 'memory'
    Invoke-AssemblyTest 'auth_unit'
    Invoke-AssemblyTest 'auth_flow'
    Invoke-AssemblyTest 'auth_session'
    Invoke-AssemblyTest 'engine-auth'
    Invoke-AssemblyTest 'account-ui'
    Invoke-AssemblyTest 'skin'
    Invoke-AssemblyTest 'engine-test'
    Invoke-AssemblyTest 'mc-sources'
    Invoke-AssemblyTest 'profile-merge'
    Invoke-AssemblyTest 'loaders'
    Invoke-AssemblyTest 'loader-change'
    Invoke-AssemblyTest 'loader-change-ui'
    & (Join-Path $projectRoot 'tests\prepare-servers-nbt.ps1')
    Invoke-AssemblyTest 'servers-nbt'
    & (Join-Path $projectRoot 'tests\prepare-servers-net.ps1') -Run
    Invoke-AssemblyTest 'servers-ui'
    Invoke-AssemblyTest 'versions'
    Invoke-AssemblyTest 'version-ui'
    Invoke-AssemblyTest 'mod-profile'
    Invoke-AssemblyTest 'version-settings'
    Invoke-AssemblyTest 'version-page'
    Invoke-AssemblyTest 'version-mods'
    Invoke-AssemblyTest 'version-actions'
    Invoke-AssemblyTest 'mods-local'
    Invoke-AssemblyTest 'modrinth'
    Invoke-AssemblyTest 'personalize'
    Invoke-AssemblyTest 'catalog'
    Invoke-AssemblyTest 'project-details'
    Invoke-AssemblyTest 'packs'
    Invoke-AssemblyTest 'packs-ui'
    Invoke-AssemblyTest 'pack-import'
    Invoke-AssemblyTest 'pack-import-ui'
    Invoke-AssemblyTest 'logs-ui'
    Invoke-AssemblyTest 'zip-writer'
    Invoke-AssemblyTest 'pack-export-ui'
    & (Join-Path $projectRoot 'tests\prepare-pack-export.ps1') -Run
    Write-Host 'PASS pack-export: real archives, round trips and source preservation'
    if ($Network) {
        Invoke-AssemblyTest 'http' 180
        Invoke-AssemblyTest 'http-parallel' 180
        Invoke-AssemblyTest 'mc-sources-live' 180
        Invoke-AssemblyTest 'modrinth-live' 180
        Invoke-AssemblyTest 'catalog-live' 180
    }
    if ($JavaProbe) {
        & (Join-Path $projectRoot 'tests\prepare-probe.ps1') -JavaHome $JavaHome
        Invoke-AssemblyTest 'engine-probe' 60
        $resultPath = Join-Path $env:OFFSET_TEST_GAME 'probe-result.txt'
        if (!(Test-Path -LiteralPath $resultPath)) { throw 'JVM did not write probe-result.txt.' }
        $probe = Get-Content -LiteralPath $resultPath -Raw -Encoding utf8
        foreach ($expected in @('Notch','b50ad385829d3141a2167e7d7539ba7f','legacy','quote-"-inside','ends-with-\')) {
            if (!$probe.Contains($expected)) { throw "JVM probe missing expected argument: $expected" }
        }
        $argFile = Get-Content -LiteralPath (Join-Path $env:OFFSET_TEST_GAME 'versions\probe\offset-jvm.args') -Raw
        if ($argFile.Contains('--accessToken') -or $argFile.Contains('Notch')) {
            throw 'Session arguments leaked into JVM argfile.'
        }
        Write-Host 'PASS actual JVM arguments, Unicode/spaces, quoting, offline identity, session isolation'
        & (Join-Path $projectRoot 'tests\prepare-version-launch.ps1') -JavaHome $JavaHome
        Invoke-AssemblyTest 'version-launch' 60
        foreach ($relative in @('result-probe-shared.txt','versions\probe-isolated\game\result-probe-isolated.txt')) {
            if (!(Test-Path -LiteralPath (Join-Path $env:OFFSET_TEST_GAME $relative))) {
                throw "Per-version JVM probe result missing: $relative"
            }
        }
        Write-Host 'PASS per-version Java/memory, isolated working directory, shared libraries/assets and global restoration'
        & (Join-Path $projectRoot 'tests\prepare-launch-log.ps1') -JavaHome $JavaHome
        Invoke-AssemblyTest 'launch-log' 90
        Write-Host 'PASS actual JVM output, crash reports, credential redaction, export and restart discovery'
    }
} finally {
    Pop-Location
    $env:INCLUDE = $previousInclude
}
Write-Host 'All requested checks passed.'
