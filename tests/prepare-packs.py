"""Deterministic fixtures; only creates files in tests/fixtures/packs."""
import hashlib, json, pathlib, zipfile, struct, io
p = pathlib.Path(__file__).resolve().parent / 'fixtures' / 'packs'
p.mkdir(parents=True, exist_ok=True)
payload = b'OFFSET native modpack file verification\n'
(p / 'payload.txt').write_bytes(payload)
sha = hashlib.sha1(payload).hexdigest()
mr = {'formatVersion':1,'game':'minecraft','name':'Native Pack','versionId':'fixture',
      'dependencies':{'minecraft':'1.21.1'},
      'files':[{'path':'config/native.txt','hashes':{'sha1':sha},'downloads':['https://cdn.modrinth.com/data/fixture/native.txt'], 'fileSize':len(payload)},
               {'path':'mods/server.jar','hashes':{'sha1':sha},'downloads':['https://cdn.modrinth.com/data/fixture/server.jar'],'env':{'client':'unsupported','server':'required'}}]}
def archive(name, manifest, extra=None):
    with zipfile.ZipFile(p/name, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('modrinth.index.json', json.dumps(manifest, ensure_ascii=False))
        z.writestr('overrides/config/override.txt', 'common\n')
        z.writestr('client-overrides/config/override.txt', 'client\n')
        if extra:
            for path, data in extra: z.writestr(path,data)
archive('valid.mrpack',mr)
class NonSeek(io.BytesIO):
    def seek(self,*args): raise io.UnsupportedOperation('fixture streaming ZIP')
stream=NonSeek()
with zipfile.ZipFile(stream,'w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('modrinth.index.json',json.dumps(mr))
raw_descriptor=stream.getvalue()
(p/'descriptor.mrpack').write_bytes(raw_descriptor)
bad_descriptor=bytearray(raw_descriptor); struct.pack_into('<I',bad_descriptor,18,1)
(p/'descriptor-mismatch.mrpack').write_bytes(bad_descriptor)
bad = json.loads(json.dumps(mr)); bad['files'][0]['path']='../escape.txt'
archive('bad-manifest.mrpack',bad)
bad = json.loads(json.dumps(mr)); bad['files'][0]['hashes']['sha1']='0'*40
archive('bad-hash.mrpack',bad)
archive('traversal.mrpack',mr,[('../escape.txt','unsafe')])
archive('alias.mrpack',mr,[('overrides/CONFIG/override.txt','alias')])
archive('device.mrpack',mr,[('overrides/config/CON.txt','unsafe')])
fallback=json.loads(json.dumps(mr)); fallback['files'][0]['downloads']=['https://cdn.modrinth.com/fixture-failure', 'https://cdn.modrinth.com/data/fixture/native.txt']
archive('fallback.mrpack',fallback)
with zipfile.ZipFile(p/'unicode-extra.mrpack','w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('modrinth.index.json',json.dumps(mr))
    info=zipfile.ZipInfo('overrides/safe.txt')
    data=b'\x01'+struct.pack('<I',0)+b'../../outside'
    info.extra=struct.pack('<HH',0x7075,len(data))+data
    z.writestr(info,'unsafe')
with zipfile.ZipFile(p/'timestamp-extra.mrpack','w',zipfile.ZIP_DEFLATED) as z:
    info=zipfile.ZipInfo('modrinth.index.json'); info.extra=struct.pack('<HHBI',0x5455,5,1,1000)
    z.writestr(info,json.dumps(mr))
with zipfile.ZipFile(p/'symlink.mrpack','w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('modrinth.index.json',json.dumps(mr))
    link=zipfile.ZipInfo('overrides/link'); link.create_system=3; link.external_attr=(0o120777<<16)
    z.writestr(link,'../../outside')
raw=(p/'valid.mrpack').read_bytes(); raw=bytearray(raw); raw[30]=ord('X'); (p/'local-mismatch.mrpack').write_bytes(raw)
cf={'minecraft':{'version':'1.21.1','modLoaders':[{'id':'forge-52.1.20','primary':True}]},
    'manifestType':'minecraftModpack','manifestVersion':1,'name':'CF Native Pack','version':'fixture',
    'files':[{'projectID':123,'fileID':456,'required':True}], 'overrides':'overrides'}
with zipfile.ZipFile(p/'curseforge.zip','w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('manifest.json',json.dumps(cf)); z.writestr('overrides/config/test.cfg','CF\n')
(p/'sha1.txt').write_text(sha)
print('Pack fixtures:',p)
