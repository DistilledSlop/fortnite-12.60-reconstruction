"""Bounded original byte and reflection identity check; no full-image read."""
import json
from pathlib import Path
import policy

ROOT = Path(__file__).resolve().parents[3]
r = ROOT / 'porting/generated/reflection'
c = next(x for x in json.loads((r/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names'] if x['block_va'] == '0x14655a270')
n = json.loads((r/'native_funcs.json').read_text())['UFortUINotification']
assert json.loads((r/'class_ref_map.json').read_text())['builder_names']['0x142eea520'] == policy.CONTRACT['class_ref']
model = {'name':'UFortUINotification', 'package':'/Script/FortniteGame', 'parent':'UObject', 'members_decl_order':[{'name':'DisplayAsset','decl':'TSoftObjectPtr<UObject> DisplayAsset;'}], 'functions':[]}
policy.apply(model, c, {'0x142b26590'}, n)
assert model['functions'][0]['ret'] == 'TSoftObjectPtr<UObject>'
assert policy.definitions(model) == 'TSoftObjectPtr<UObject> UFortUINotification::GetImageOverride()\n{\n    return DisplayAsset;\n}\n'
p = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with p.open('rb') as f:
    for va, data in [(0x142b292a0,'40534883ec60488b42204533c94885c0498bd8410f95c14c03c84c894a20488d542438e8f8afacff488b08488d501848890b8b4808894b08488b481048894c2420488d4c2428e8f599b5fd488d4b10488d542420e8e7463900488b4c24504885c97405e808540c004883c4605bc3'), (0x1425f42c0,'48895c241048896c2418565741564883ec404533f6c702ffffffff4c897218418bc644897224488bfa44897204488be9488942108b42244c897424604489722085c0790b33d2488d4f18e851be08fe488b47184885c074046644893044897708488d95b8000000488b85a0000000488d4c24284889078b85a8000000894708488b85b00000004889442420e890e908fe488d542420488d4f10e882968c00488b5c2468488bc7488b6c24704883c440415e5f5ec3')]:
        expected = bytes.fromhex(data)
        f.seek(va - 0x140000000)
        assert f.read(len(expected)) == expected
print('PASS: original reflection, bounded native bytes, policy declaration and body')
