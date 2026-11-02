"""Recheck reflection records and exact instruction bytes in the pinned image."""
import hashlib, json
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
contract=json.loads((HERE/'original-policy.json').read_text())
r=ROOT/'porting/generated/reflection'
classes=json.loads((r/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
c=next(x for x in classes if x['block_va']=='0x145a0e0d0')
assert [x for x in c['functions'] if x['name']=='GetCurrentInputType']==[contract['function']]
assert [x for x in c['properties'] if x['name']=='CurrentInputType' or x.get('enum_underlying_of')=='CurrentInputType']==contract['properties']
assert json.loads((r/'native_funcs.json').read_text())['UCommonInputSubsystem']['GetCurrentInputType']==contract['exec']
assert contract['enum'] in json.loads((r/'enum_blocks.json').read_text())['enums']
p=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
h=hashlib.sha256()
with p.open('rb') as f:
    for chunk in iter(lambda:f.read(4*1024*1024),b''): h.update(chunk)
    assert h.hexdigest()=='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
    for va,expected in [(0x14081b540,'0fb64179c3'),(0x14081ef40,'40534883ec20488b42204533c94885c0498bd8410f95c14c03c84c894a20e8ddc5ffff88034883c4205bc3'),(0x14081e070,'4883ec28488b05fd518b074885c0751a488d15c9ee0107488d0dea518b07e81d636a02488b05de518b074883c428c3')]:
        f.seek(va-0x140000000); assert f.read(len(bytes.fromhex(expected)))==bytes.fromhex(expected)
print('PASS: pinned image and original reflection')
