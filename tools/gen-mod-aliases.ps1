param([string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot))
$ErrorActionPreference = 'Stop'
$rows = @([IO.File]::ReadAllText((Join-Path $ProjectRoot 'src/mod_aliases.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json)
$out = [Collections.Generic.List[string]]::new()
$out.Add('; Generated from mod_aliases.json. Original name/alias index; no reference launcher data.')
$aliases = [Collections.Generic.List[string]]::new()
$projects = [Collections.Generic.List[string]]::new()
$seen = @{}
function Bytes([string]$s) { ((@([Text.Encoding]::UTF8.GetBytes($s)) + 0) -join ',') }
function Words([string]$s) { ((@($s.ToCharArray() | ForEach-Object {[int]$_}) + 0) -join ',') }
for ($i=0; $i -lt $rows.Count; $i++) {
    $r=$rows[$i]
    if (!$r.query -or $r.query.Length -gt 120 -or $r.query -notmatch '^[A-Za-z0-9 .+''_-]+$') { throw "Invalid query at $i" }
    if (!$r.name -or [Text.Encoding]::UTF8.GetByteCount($r.name) -gt 160) { throw "Invalid name at $i" }
    $out.Add(('cnQuery{0} db {1}' -f $i,(Bytes $r.query)))
    $out.Add(('cnTitle{0} db {1}' -f $i,(Bytes $r.name)))
    if ($r.id) {
        if ($r.id -notmatch '^[A-Za-z0-9]{8}$') {throw "Invalid project ID at $i"}
        $out.Add(('cnId{0} db {1}' -f $i,(Bytes $r.id)))
        $projects.Add(('dd cnId{0},cnTitle{0}' -f $i))
    }
    $j=0
    foreach($a in $r.aliases) {
        $norm=($a.ToLowerInvariant() -replace '[\s\p{P}\p{S}]','')
        if (!$norm -or $norm.Length -gt 120) {throw "Invalid alias at $i"}
        if ($seen.ContainsKey($norm)) {if ($seen[$norm] -ne $i) {throw "Ambiguous alias: $a"}; continue}
        $seen[$norm]=$i
        $label='cnAlias{0}_{1}' -f $i,$j
        $out.Add('align 2')
        $out.Add(('{0} dw {1}' -f $label,(Words $norm)))
        $aliases.Add(('dd {0},{1},cnQuery{2}' -f $label,$norm.Length,$i))
        $j++
    }
}
$out.Add(('CN_ALIAS_COUNT = {0}' -f $aliases.Count))
$out.Add('align 4')
$out.Add('cnAliasTable:'); $out.AddRange($aliases)
$out.Add(('CN_PROJECT_COUNT = {0}' -f $projects.Count))
$out.Add('cnProjectTable:'); $out.AddRange($projects)
[IO.File]::WriteAllLines((Join-Path $ProjectRoot 'src/mod_aliases_data.inc'),$out,[Text.Encoding]::ASCII)
