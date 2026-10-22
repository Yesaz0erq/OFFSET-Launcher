"""Import-format fixtures, built only below tests/fixtures/pack-import."""
import json
import pathlib
import zipfile
import sys

root = pathlib.Path(__file__).resolve().parent / 'fixtures' / 'pack-import'
root.mkdir(parents=True, exist_ok=True)
mr = {'formatVersion': 1, 'game': 'minecraft', 'name': 'MR Import',
      'versionId': 'test', 'dependencies': {'minecraft': '1.21.1'}, 'files': []}
cf = {'manifestType': 'minecraftModpack', 'manifestVersion': 1,
      'minecraft': {'version': '1.21.1', 'modLoaders': []},
      'name': 'CF Import', 'version': 'test', 'files': [], 'overrides': 'overrides'}
mmc = {'formatVersion': 1, 'components': [{'uid': 'net.minecraft', 'version': '1.21.1'}]}

def archive(name, entries):
    with zipfile.ZipFile(root / name, 'w', zipfile.ZIP_DEFLATED) as z:
        for path, data in entries.items():
            if isinstance(data, dict):
                data = json.dumps(data, ensure_ascii=False)
            z.writestr(path, data)

archive('modrinth.zip', {'modrinth.index.json': mr, 'overrides/config/import.txt': 'MR\n'})
archive('curseforge.zip', {'manifest.json': cf, 'overrides/config/import.txt': 'CF\n'})
archive('wrapped-mr.zip', {'Wrapped/modrinth.index.json': mr,
                         'Wrapped/overrides/config/import.txt': 'MR wrapped\n',
                         'Wrapped/client-overrides/config/import.txt': 'client\n'})
archive('wrapped-cf.zip', {'Wrapped/manifest.json': cf,
                         'Wrapped/overrides/config/import.txt': 'CF wrapped\n'})
prism = {'mmc-pack.json': mmc, 'instance.cfg': '[General]\nname=Prism 中文\n',
         '.minecraft/config/import.txt': 'Prism\n'}
archive('prism.zip', prism)
archive('multimc.zip', {'Instance/mmc-pack.json': mmc,
                        'Instance/instance.cfg': '[General]\nname=MultiMC\n',
                        'Instance/minecraft/config/import.txt': 'MultiMC\n'})
for kind, uid, version in [('fabric','net.fabricmc.fabric-loader','0.19.5'),
                           ('forge','net.minecraftforge','52.1.20'),
                           ('neoforge','net.neoforged','21.1.264'),
                           ('quilt','org.quiltmc.quilt-loader','0.30.1')]:
    components = mmc['components'] + [{'uid': uid, 'version': version}]
    archive(f'prism-{kind}.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': components}}))
archive('ambiguous.zip', {'modrinth.index.json': mr, 'manifest.json': cf})
archive('ambiguous-wrapped.zip', {'One/modrinth.index.json': mr, 'Two/manifest.json': cf})
archive('outside-wrapper.zip', {'Wrapped/modrinth.index.json': mr, 'other.txt': 'outside'})
archive('hook.zip', dict(prism, **{'instance.cfg': '[General]\nname=Hook\nPreLaunchCommand=cmd.exe /c something\n'}))
archive('patches.zip', dict(prism, **{'patches/net.minecraft.json': '{}'}))
archive('unknown-component.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': mmc['components'] + [{'uid': 'org.example.custom', 'version': '1'}]}}))
archive('duplicate-game.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': mmc['components'] * 2}}))
archive('owned-payload.zip', dict(prism, **{'.minecraft/versions/1.21.1/1.21.1.json': '{}'}))
archive('both-data.zip', dict(prism, **{'minecraft/config/test.txt': 'ambiguous'}))
archive('embedded-nul.zip', dict(prism, **{'instance.cfg': '[General]\nname=Good\x00\nPreLaunchCommand=bad\n'}))
archive('nested-manifest.zip', {'modrinth.index.json': mr, 'overrides/config/manifest.json': '{}'})
dependency_components = mmc['components'] + [
    {'uid': 'net.fabricmc.fabric-loader', 'version': '0.19.5'},
    {'uid': 'org.lwjgl3', 'dependencyOnly': True, 'cachedVersion': '3.3.3'},
    {'uid': 'net.fabricmc.intermediary', 'cachedDependencyOnly': True, 'cachedVersion': '1.21.1'}]
archive('prism-dependencies.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': dependency_components}}))
archive('runtime-no-version.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': mmc['components'] + [{'uid': 'org.lwjgl3', 'cachedVersion': '3.3.3'}]}}))
archive('unknown-no-version.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': mmc['components'] + [{'uid': 'org.example.custom', 'dependencyOnly': True, 'cachedVersion': '1'}]}}))
archive('disabled-loader.zip', dict(prism, **{'mmc-pack.json': {'formatVersion': 1, 'components': mmc['components'] + [{'uid': 'net.fabricmc.fabric-loader', 'version': '0.19.5', 'disabled': True}]}}))
print('Import fixtures:', root)
if '--large' in sys.argv:
    large = root.parent.parent / 'out' / 'pack-import-large.zip'
    large.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(large, 'w', zipfile.ZIP_STORED) as z:
        z.writestr('modrinth.index.json', json.dumps(mr))
        block = bytes(1024 * 1024)
        for name in ('a.bin', 'b.bin'):
            with z.open('overrides/resourcepacks/' + name, 'w') as entry:
                for _ in range(70):
                    entry.write(block)
    print('Large archive fixture:', large, large.stat().st_size)
