"""Exercise the native pack workers in an isolated Windows directory."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from zipfile import ZipFile, ZIP_DEFLATED

ROOT = Path(__file__).resolve().parents[1]
# This entry point is assembled only in the temporary test copy.
ENTRY = r"""
start:
    stdcall EngineInit
    invoke GetEnvironmentVariableW,.out,peDestW,2048
    invoke GetEnvironmentVariableW,.root,peRootW,1024
    invoke GetEnvironmentVariableW,.mode,pePathW,2048
    movzx eax,word [pePathW]
    sub eax,'0'
    cmp eax,0
    je .inspect
    cmp eax,6
    je .install
    cmp eax,9
    je .manualFile
    cmp eax,7
    jae .rememberMod
    cmp eax,5
    je .unpack
    mov [peFormat],eax
    invoke lstrcpyA,peVersionA,.id
    invoke GetEnvironmentVariableW,.version,pePathW,128
    test eax,eax
    jz .export
    invoke WideCharToMultiByte,65001,0,pePathW,-1,peVersionA,128,0,0
.export:
    stdcall PackExportBuild
    jmp .result
.install:
    stdcall PackInspect,peDestW
    test eax,eax
    jz .failure
    stdcall PackInstallChecked,peDestW,peRootW,packInspectHash
    jmp .result
.manualFile:
    invoke lstrcpyW,pkArchive,peDestW
    invoke lstrcpyW,pkPath,peRootW
    invoke lstrcatW,pkPath,.manualPath
    invoke lstrcpyW,pkFiles+PK_FWNAME,.manualName
    invoke lstrcpyA,pkFiles+PK_FHASH,.manualHash
    stdcall PkUseLocalFile,pkFiles
    jmp .result
.manualName du 'mods/manual.jar',0
.manualPath du '\mods\manual.jar',0
.manualHash db '96845139bb7425846cbe416076367f464cb8c1ff',0
.rememberMod:
    sub eax,7
    mov [modProvider],eax
    mov [modProjectCount],1
    mov [modSelectedProject],0
    mov [modFileCount],1
    invoke lstrcpyA,modProjects,.project
    invoke lstrcpyA,modFiles,.fileId
    invoke lstrcpyA,modFiles+MF_NAME,.modName
    mov eax,.mrUrl
    cmp [modProvider],0
    je .url
    mov eax,.cfUrl
.url:
    invoke lstrcpyA,modFiles+MF_URL,eax
    mov [modContextActive],1
    invoke lstrcpyW,modContextRootW,peRootW
    stdcall Sha1File,peDestW,modFiles+MF_SHA1
    stdcall ModsInstall,0
    jmp .result
.inspect:
    stdcall PackInspect,peDestW
    test eax,eax
    jz .failure
    invoke ExitProcess,[packInspectFormat]
.unpack:
    stdcall PackInspect,peDestW
    test eax,eax
    jz .failure
    invoke lstrcpyW,pkStage,peRootW
    invoke lstrcpyW,pkExtract,pkStage
    invoke lstrcatW,pkExtract,pkExtractSuffix
    stdcall EnsureDirectory,pkExtract
    stdcall PkExtractArchive
    test eax,eax
    jz .failure
    stdcall PkReadManifest
    test eax,eax
    jz .failure
    cmp [pkFormat],4
    jne .files
    stdcall PkVerifyAddons
    test eax,eax
    jz .failure
.files:
    stdcall PkDownloadFiles
    test eax,eax
    jz .failure
    stdcall PkCopyOverrides,pkOverrides
    test eax,eax
    jz .failure
    cmp [pkFormat],1
    jne .success
    stdcall PkCopyOverrides,pkClientOverrides
.result:
    test eax,eax
    jz .failure
.success:
    invoke ExitProcess,0
.failure:
    invoke ExitProcess,99
.out du 'PACK_TEST_ARCHIVE',0
.root du 'PACK_TEST_ROOT',0
.mode du 'PACK_TEST_MODE',0
.version du 'PACK_TEST_VERSION',0
.id db '1.20.1',0
.project db '1234',0
.fileId db '5678',0
.modName db 'second.jar',0
.mrUrl db 'https://cdn.modrinth.com/fixture/second.jar',0
.cfUrl db 'https://mediafilez.forgecdn.net/files/5/678/second.jar',0
"""


def archive(path, entries):
    with ZipFile(path, "w", ZIP_DEFLATED) as out:
        for name, data in entries.items():
            if isinstance(data, dict):
                data = json.dumps(data, ensure_ascii=False).encode()
            out.writestr(name, data)


def build_worker(work):
    source = work / "src"
    shutil.copytree(ROOT / "src", source)
    main = source / "main.asm"
    main.write_bytes(main.read_bytes().replace(b"entry start", b"entry packTest")
                     + ENTRY.replace("start:", "packTest:", 1)
                     .replace("packTest:", "section '.test' code readable executable\npackTest:").encode())
    exe = work / "pack-test.exe"
    subprocess.run([ROOT / "tools/FASM.EXE", "-m", "262144", main.name, exe],
                   cwd=source, env=dict(os.environ, INCLUDE=str(ROOT / "tools/INCLUDE")), check=True)

    return exe


def run_tests(work):
    exe = build_worker(work)
    game = work / "instance"
    version = game / "versions/1.20.1"
    version.mkdir(parents=True)
    (version / "1.20.1.jar").write_bytes(b"fixture")
    (version / "1.20.1.json").write_text(json.dumps(dict(
        id="1.20.1", arguments={}, mainClass="fixture.Main", libraries=[])))
    checks = 0

    def worker(mode, path, *, root=game, expect=0):
        nonlocal checks
        result = subprocess.run([exe], cwd=work, timeout=120, env=dict(
            os.environ, PACK_TEST_ARCHIVE=str(path), PACK_TEST_MODE=str(mode), PACK_TEST_ROOT=str(root),
            PACK_TEST_VERSION="1.20.1"))
        assert result.returncode == expect, (mode, path.name, result.returncode, expect)
        checks += 1

    # A restricted file can be supplied beside the archive, but only if its hash matches.
    manual = work / "manual.jar"
    dest = work / "manual-instance"
    manual.write_bytes(b"wrong file")
    worker(9, work / "manual.zip", root=dest, expect=99)
    assert not (dest / "mods/manual.jar").exists()
    manual.write_bytes(b"manual file")
    worker(9, work / "manual.zip", root=dest)
    assert (dest / "mods/manual.jar").read_bytes() == manual.read_bytes()

    payload = b"fixture platform file"
    sha1 = hashlib.sha1(payload).hexdigest()
    mod_path = "mods/example.jar"
    (game / "mods").mkdir()
    (game / mod_path).write_bytes(payload)
    ref = dict(path=mod_path, hashes=dict(sha1=sha1, sha512=hashlib.sha512(payload).hexdigest()),
               env=dict(client="optional", server="required"),
               downloads=["https://cdn.modrinth.com/fixture/example.jar"], fileSize=len(payload))
    mr = dict(formatVersion=1, game="minecraft", name="中文测试", versionId="1.0",
              files=[ref], dependencies=dict(minecraft="1.20.1"))
    source_pack = work / "source.mrpack"
    archive(source_pack, {"modrinth.index.json": mr, "overrides/config/mod.txt": b"common",
                          "client-overrides/config/mod.txt": b"client", "server-overrides/server.txt": b"server"})
    # The verified local file exercises the real download-cache and receipt path offline.
    worker(5, source_pack)
    assert (game / "config/mod.txt").read_bytes() == b"client"
    assert not (game / "server.txt").exists()

    local_mod = game / "mods/second.jar"
    local_mod.write_bytes(b"local mod with a catalog match")
    worker(7, local_mod)
    worker(8, local_mod)

    for kind, manifest in enumerate(["modrinth.index.json", "manifest.json", "mmc-pack.json", "mcbbs.packmeta"], 1):
        path = work / (f"export-{kind}" + (".mrpack" if kind == 1 else ".zip"))
        worker(kind, path)
        worker(0, path, expect=kind)
        with ZipFile(path) as out:
            assert out.testzip() is None
            meta = json.loads(out.read(manifest))
            if kind == 1:
                refs = {f["path"]: f for f in meta["files"]}
                assert refs[mod_path] == ref
                assert refs["mods/second.jar"]["hashes"]["sha512"] == hashlib.sha512(local_mod.read_bytes()).hexdigest()
                assert "overrides/" + mod_path not in out.namelist()
            if kind == 2:
                assert meta["files"] == [dict(projectID=1234, fileID=5678, required=True)]
                assert "overrides/mods/second.jar" not in out.namelist()
            if kind == 4:
                companion = json.loads(out.read("manifest.json"))
                assert companion["minecraft"]["version"] == "1.20.1"
                assert meta["addons"] == [dict(id="game", version="1.20.1")]
                assert {f["path"]: f["hash"] for f in meta["files"]}[mod_path] == sha1
            if kind in (3, 4):
                destination = work / f"installed-{kind}"
                worker(5, path, root=destination)
                assert (destination / mod_path).read_bytes() == payload
                assert (destination / "config/mod.txt").read_bytes() == b"client"

    cf = dict(manifestType="minecraftModpack", manifestVersion=1, name="CF", files=[],
              minecraft=dict(version="1.20.1", modLoaders=[
                  dict(id="forge-47.1.0", primary=False), dict(id="forge-47.2.0", primary=True)]))
    pack = work / "wrapped.zip"
    archive(pack, {"outer/manifest.json": cf})
    worker(0, pack, expect=2)
    bbs = dict(manifestType="minecraftModpack", manifestVersion=2, name="双清单", files=[
        dict(type="addon", path=mod_path, hash=sha1)], addons=[
            dict(id="forge", version="47.2.0"), dict(id="game", version="1.20.1")])
    entries = {"outer/mcbbs.packmeta": bbs, "outer/manifest.json": cf, "outer/overrides/" + mod_path: payload}
    for order in [entries, dict(reversed(entries.items()))]:
        archive(pack, order)
        worker(0, pack, expect=4)
        worker(5, pack, root=work / "dual")
    archive(pack, {"manifest.json": bbs, "overrides/" + mod_path: payload})
    worker(0, pack, expect=4)
    archive(pack, {"mcbbs.packmeta": bbs, "overrides/" + mod_path: b"changed"})
    worker(5, pack, root=work / "mismatch", expect=99)
    assert not (work / "mismatch" / mod_path).exists()

    # ZIP permits a central directory in a different order from file data.
    with ZipFile(pack, "w", ZIP_DEFLATED) as z:
        z.writestr("prism/mmc-pack.json", json.dumps(dict(formatVersion=1, components=[
            dict(uid="net.minecraft", version="1.20.1")])))
        z.writestr("prism/instance.cfg", "[General]\nname=Empty client\n")
        z.filelist.reverse()
    worker(0, pack, expect=3)

    # Resume an owned stage offline through the full install-and-publish entry point.
    target = work / "full-instance"
    stage = Path(str(target) + ".offset-stage")
    stage.mkdir()
    package = work / "export-4.zip"
    (stage / ".offset-pack-stage").write_text(hashlib.sha1(package.read_bytes()).hexdigest())
    vdir = stage / "versions/1.20.1"
    shutil.copytree(version, vdir)
    indexes = stage / "assets/indexes"
    indexes.mkdir(parents=True)
    index = indexes / "test.json"
    index.write_bytes(b'{"objects":{}}')
    def cache_record(file):
        return dict(url="https://example.invalid/cache", sha1=hashlib.sha1(file.read_bytes()).hexdigest())
    meta = json.loads((vdir / "1.20.1.json").read_text())
    meta.update(offsetNormalized=True, downloads=dict(client=cache_record(vdir / "1.20.1.jar")),
                assetIndex=dict(id="test", **cache_record(index)))
    (vdir / "1.20.1.json").write_text(json.dumps(meta))
    worker(6, package, root=target)
    assert json.loads((target / ".offset-pack.json").read_text())["complete"]
    assert (target / mod_path).read_bytes() == payload and not stage.exists()

    # Exceed the original 4,096-entry preflight limit with a nested Unicode payload.
    large = {"modrinth.index.json": mr}
    large.update({f"overrides/config/中文/{i}.txt": b"cfg" for i in range(4500)})
    archive(pack, large)
    worker(0, pack, expect=1)

    # Real Distant Horizons databases can exceed 512 MiB. Stream the fixture.
    block = bytes(1024 * 1024)
    with ZipFile(pack, "w", ZIP_DEFLATED, compresslevel=1) as out:
        out.writestr("manifest.json", json.dumps(cf))
        with out.open("overrides/saves/world/DistantHorizons.sqlite", "w") as db:
            for _ in range(513):
                db.write(block)
    worker(0, pack, expect=2)

    (game / mod_path).write_bytes(b"user edit")
    edited = work / "edited.mrpack"
    worker(1, edited)
    with ZipFile(edited) as out:
        assert [f["path"] for f in json.loads(out.read("modrinth.index.json"))["files"]] == ["mods/second.jar"]
        assert out.read("overrides/" + mod_path) == b"user edit"
    print(f"{checks} native worker calls passed, archive contents and checksums verified.")


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="offset-packs-") as directory:
        run_tests(Path(directory))
