#!/usr/bin/env python3
"""Prepare a source-only bulk manifest for the original parameterless empty exec.

Run from any directory: python3 -I porting/behavior/bulk_empty_exec_runtime/prepare.py
Outputs are scratch artifacts; this script never edits the canonical manifest.
"""
import collections
import hashlib
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT / 'porting/generated/reflection'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
OUT = Path('/tmp/gameplay-bulk-empty')
BASE, EXEC, TARGET = 0x140000000, 0x14075b6b0, 0x1406801c0
IMAGE_SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
THUNK = bytes.fromhex('488b42204533c04885c0410f95c04c03c04c894220e9f64af2ff')
# UE 4.25 EFunctionFlags: only unused bits 0x10 and 0x20 are unknown.
KNOWN = 0xffffffcf
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x00004000 | 0x00001000 | 0x00000100 | 0x00000080 | 0x00000040
ACCESS = {0x20000: 'public', 0x40000: 'private', 0x80000: 'protected'}

def load(path):
    with path.open(encoding='utf-8') as f:
        return json.load(f)

def main():
    image_hash = hashlib.file_digest(IMAGE.open('rb'), 'sha256').hexdigest()
    if image_hash != IMAGE_SHA:
        raise SystemExit('original image SHA-256 mismatch')
    with IMAGE.open('rb') as f:
        f.seek(EXEC - BASE); actual = f.read(len(THUNK))
        f.seek(TARGET - BASE); target_bytes = f.read(3)
    if actual != THUNK or actual[21] != 0xe9 or EXEC + len(THUNK) + struct.unpack('<i', actual[22:])[0] != TARGET or target_bytes != b'\xc2\x00\x00':
        raise SystemExit('original thunk or tail target mismatch')
    native = load(REFL / 'native_funcs.json')
    names = load(REFL / 'class_name_map.json')
    parents = load(REFL / 'class_parent_map.json')
    reflection = load(REFL / 'fortnite_game_reflection.json')
    manifest = load(ROOT / 'porting/behavior/MANIFEST.json')
    selected = {line.strip() for line in (ROOT / 'porting/native_class_set.txt').read_text().splitlines() if line.strip() and not line.lstrip().startswith('#')}
    ported = {(e.get('class'), e.get('function')) for e in manifest if e.get('status') == 'ported'}
    blocks = {c['block_va'].lower(): c for c in reflection['classes_in_groups_with_fort_names']}
    by_name = collections.defaultdict(list)
    for va, name in names.items():
        by_name[name].append((va.lower(), parents.get(va), blocks.get(va.lower())))
    candidates, rejected, proposals = [], [], []
    for cls, funcs in sorted(native.items()):
        for name, fn in sorted(funcs.items()):
            if not isinstance(fn, dict) or str(fn.get('exec_addr', '')).lower() != hex(EXEC):
                continue
            row = {'class': cls, 'function': name, 'exec_addr': fn.get('exec_addr'), 'flags': fn.get('flags'), 'params': fn.get('params')}
            reasons = []
            matches = by_name.get(cls, [])
            if len(matches) != 1:
                reasons.append('class_mapping_not_unique')
                parent, block = None, None
            else:
                va, parent, block = matches[0]
                row['block_va'] = va
                if parent is None or block is None or parent.get('name') != cls:
                    reasons.append('incomplete_class_mapping')
            package = parent.get('package') if parent else None
            row['package'] = package
            if package not in ('/Script/FortniteGame', '/Script/FortniteUI'):
                reasons.append('outside_target_packages')
            if cls not in selected:
                reasons.append('class_not_selected')
            if block is None:
                reasons.append('class_flags_missing')
            else:
                try:
                    if int(block['class_flags'], 0) & 0x4000:
                        reasons.append('interface_class')
                except (KeyError, ValueError, TypeError):
                    reasons.append('class_flags_missing')
            if (cls, name) in ported:
                reasons.append('already_ported')
            if fn.get('params') != []:
                reasons.append('parameters_not_exact_empty_list')
            try:
                flags = int(fn['flags'], 0)
                if flags < 0 or flags & ~KNOWN: reasons.append('unknown_function_flags')
                if not flags & 0x400: reasons.append('not_native')
                if flags & NET: reasons.append('network_function')
                if flags & 0x08000000: reasons.append('blueprint_event')
                if flags & 0x20000000: reasons.append('editor_only')
                if flags & 0x2000 and flags & 0x40000000: reasons.append('static_const_invalid')
                access = ACCESS.get(flags & 0xe0000)
                if access is None: reasons.append('ambiguous_access')
            except (KeyError, TypeError, ValueError):
                flags, access = 0, None
                reasons.append('function_flags_missing')
            if not re.fullmatch(r'[A-Za-z_]\w*', name) or not re.fullmatch(r'[A-Za-z_]\w*', cls):
                reasons.append('invalid_cpp_identifier')
            row['reasons'] = reasons
            if reasons:
                rejected.append(row)
                continue
            const = bool(flags & 0x40000000)
            static = bool(flags & 0x2000)
            row.update(access=access, static=static, const=const)
            candidates.append(row)
            proposals.append({'class': cls, 'function': name, 'kind': 'exec',
                'signature': 'void %s()%s' % (name, ' const' if const else ''),
                'signature_status': 'proposed', 'inc_path': 'porting/behavior/bulk_empty_exec_runtime/empty.inc',
                'confidence': 'high', 'address': hex(EXEC), 'status': 'ported',
                'reflection_flags': fn['flags'], 'static': static, 'access': access,
                'notes': 'Original shared parameterless exec thunk advances FFrame.Code when non-null and tail-jumps to a void return; see bulk_empty_exec_runtime/README.md. Source recovery only.'})
    OUT.mkdir(parents=True, exist_ok=True)
    report = {'image_sha256': image_hash, 'exec_addr': hex(EXEC), 'exec_bytes': actual.hex(), 'target': hex(TARGET), 'target_bytes': target_bytes.hex(), 'counts': {'matching': len(candidates) + len(rejected), 'candidate': len(candidates), 'rejected': len(rejected)}, 'rejection_reasons': dict(sorted(collections.Counter(x for r in rejected for x in r['reasons']).items())), 'candidates': candidates, 'rejected': rejected}
    for filename, data in [('report.json', report), ('manifest-entries.json', proposals)]:
        (OUT / filename).write_text(json.dumps(data, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('counts', 'rejection_reasons')}, indent=2))

if __name__ == '__main__':
    main()
