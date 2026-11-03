#!/usr/bin/env python3
"""Read-only gate for constructor provenance, slots, callbacks, and reflected fields."""
import hashlib, json, struct
from pathlib import Path
P = Path(__file__).resolve().parent
R = P.parents[2]
image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with image.open('rb') as f:
    assert hashlib.file_digest(f,'sha256').hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
    def raw(a,n):
        f.seek(a-0x140000000); return f.read(n)
    for a,n,h in [
      (0x1427feed0,0x70,'deb146d45eceee77cdb0422ceb9c9595b690c33372e0db6848105a688b6cf35d'),
      (0x142809ee0,0x70,'c04844f3546fcf935ec3ec1eac9974c3d1d30eef59e2c421e3c77cd42b904c97'),
      (0x141ac7260,8,'49380405c28b9d9dcdd960895ef35d0ee98d805b040633b153ce7cbd2446750b'),
      (0x141ad6740,0xd3,'051c176b039f18029dba26864c2ec0d20097c53ca7368a302d3ab14d73d85ab9'),
      (0x141afee60,0x32,'20aa85babb2e464295d474a78e57a37eed4f01e44f806a202cccda9227174fdd'),
      (0x1406f7390,0x20,'a5f7c28d5aec0a4272fbb16391f84d7d981b59c3ca9da49cff25d735d9bda903'),
      (0x142800420,13,'46c9a8265d1dbceaff336319aa99deeb8c2f593793bfbc93e736b1fb45f9fe8e'),
      (0x14280af90,13,'4c997849f35d79ff9bdac027d10337aa800e81f555e9610e90382e9d04e75b3b'),
      (0x141ac2200,0x1b3,'909776ffa4a32b2afd184f015a78ef8fc4028331f9ebe2e0a9c60a702da823cf'),
      (0x141aeeee0,0x135,'688d448be50819c032d0335b8db183fa521d7fdde77755efa6130a7252cf3358')]:
        assert hashlib.sha256(raw(a,n)).hexdigest()==h, hex(a)
    def q(a): return struct.unpack('<Q',raw(a,8))[0]
    assert q(0x145e903a8+0x6b8)==0x141ac7260
    assert q(0x145e903a8+0x9b8)==0x141ad6740
    assert q(0x145ea9350+0x6b8)==0x1406801c0
    assert q(0x145ea9350+0x9b8)==0x141afee60
p=json.loads((R/'porting/generated/reflection/fortnite_game_reflection.json').read_text())
classes={}
def walk(x):
    if isinstance(x,dict):
        if x.get('class_noregister_fn') in ('0x1427feed0','0x142809ee0','0x142a8ae70'):
            classes[x['class_noregister_fn']]={z['name']:z.get('offset') for z in x['properties'] if 'array_inner_of' not in z and z.get('offset') != 0}
        for v in x.values(): walk(v)
    elif isinstance(x,list):
        for v in x: walk(v)
walk(p)
for c,fields in {'0x1427feed0':{'CachedContext':0x2f8,'GameIsReverse':0x290,'UseInfiniteAmmo':0x270,'ScoreToWin':0x2e0},'0x142809ee0':{'ItemsWithInfiniteAmmo':0x320},'0x142a8ae70':{'KillScore':0xe74}}.items():
    for name,off in fields.items(): assert classes[c][name]==off,(c,name)
entries=json.loads((P/'manifest-entries.json').read_text())
assert [e['status'] for e in entries]==['ported','ported','ported','ported']
assert all((R/e['inc_path']).is_file() for e in entries if e['inc_path'])
print('Pinned override evidence passed; GG shared-state writers remain unported')

# Pin reviewed source and proposal.
assert hashlib.sha256((P/'manifest-entries.json').read_bytes()).hexdigest() == '983bf0ac9b1c03c1241fd63b86ff1d3878cf58bd2cf3b9b755f417bec7d07dd8'
assert hashlib.sha256((P/'LoadoutSwapQuery.inc').read_bytes()).hexdigest() == 'b9bdb98dbc3782bede30ac3d1ad433a75d6165c53cbf64cdf55d24f1a367eaa5'
assert hashlib.sha256((P/'GGSetContext.inc').read_bytes()).hexdigest() == 'be9bb1b3ae32631f295a55763a5689eec9b8623474787f28d6fc6d1a4b364def'
assert hashlib.sha256((P/'GGQuery.inc').read_bytes()).hexdigest() == '87f8785e865fa493d7534d2250bd246fb5af90e2fd9ce9a189a23783bddc5689'
