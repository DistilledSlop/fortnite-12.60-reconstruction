#!/usr/bin/env python3
"""Recover exact plain-object pointer field getters into isolated proposals."""
import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import importlib.util

ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT / 'porting/generated/reflection'
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
PREFIX = 'mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP = 'push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
ACCESS = {0x20000: 'public', 0x40000: 'private', 0x80000: 'protected'}
load = lambda p: json.loads(p.read_text())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revalidate-own', action='store_true')
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-object'))
    args = parser.parse_args()
    image = IMAGE.read_bytes()
    digest = hashlib.sha256(image).hexdigest()
    if digest != SHA:
        raise SystemExit('image digest mismatch')
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    def decode(addr):
        pos = addr - BASE
        if pos < 0 or pos + 128 > len(image):
            return ''
        ins = []
        for i in md.disasm(image[pos:pos + 128], addr):
            ins.append((i.mnemonic + ' ' + i.op_str).rstrip())
            if i.mnemonic == 'ret':
                return '; '.join(ins)
            if i.mnemonic.startswith('j'):
                return ''
        return ''
    native = load(REFL / 'native_funcs.json')
    names = load(REFL / 'class_name_map.json')
    parents = load(REFL / 'class_parent_map.json')
    refs = load(REFL / 'class_ref_map.json')['resolved']
    blocks = {c['block_va'].lower(): c for c in load(REFL / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    byname = collections.defaultdict(list)
    for va, name in names.items():
        byname[name].append(va)
    selected = {x.strip() for x in (ROOT / 'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
    own = str(HERE.relative_to(ROOT)) + '/'
    ported = {(e.get('class'), e.get('function')) for e in load(ROOT / 'porting/behavior/MANIFEST.json') if e.get('status') == 'ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own))}
    headers = collections.defaultdict(list)
    for module in ('FortniteGame', 'FortniteUI'):
        for p in (ROOT / 'porting/native/Source' / module / 'Generated').glob('*.h'):
            headers[p.stem].append(p)
    spec = importlib.util.spec_from_file_location('gen_native_classes', ROOT / 'porting/gen_native_classes.py')
    generator = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = generator
    spec.loader.exec_module(generator)
    results = []
    rejects = collections.Counter()
    kinds = collections.Counter()
    total = 0
    for cls, funcs in sorted(native.items()):
        for name, fn in sorted(funcs.items()):
            total += 1
            if not isinstance(fn, dict):
                rejects['invalid_record'] += 1; continue
            ps = fn.get('params')
            if not isinstance(ps, list) or len(ps) != 1 or ps[0].get('name') != 'ReturnValue' or int(ps[0].get('property_flags', '0'), 0) & 0x400 == 0:
                rejects['not_zero_arg_getter'] += 1; continue
            p = ps[0]
            kind = p.get('kind')
            kinds[kind or 'unknown'] += 1
            if kind != 'Object':
                rejects['unsupported_return_' + str(kind)] += 1; continue
            if p.get('array_dim') != 1 or int(p.get('property_flags', '0'), 0) & 0x08000000:
                rejects['return_reference_or_array'] += 1; continue
            try:
                flags, addr = int(fn['flags'], 0), int(fn['exec_addr'], 0)
            except (KeyError, TypeError, ValueError):
                rejects['missing_flags_or_address'] += 1; continue
            if flags & (~0xffffffcf | NET | 0x08000000 | 0x20000000 | 0x2000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS:
                rejects['flags'] += 1; continue
            if (cls, name) in ported:
                rejects['already_ported'] += 1; continue
            vs = byname.get(cls, [])
            if len(vs) != 1 or cls not in selected:
                rejects['class_not_unique_or_selected'] += 1; continue
            va = vs[0]
            parent, block = parents.get(va, {}), blocks.get(va.lower())
            if not block or parent.get('name') != cls or parent.get('package') not in ('/Script/FortniteGame', '/Script/FortniteUI') or int(block['class_flags'], 0) & 0x4000:
                rejects['class_or_package'] += 1; continue
            if not re.fullmatch(r'[A-Za-z_]\w*', name):
                rejects['identifier'] += 1; continue
            asm = decode(addr)
            target = None
            offset = None
            if asm.startswith(PREFIX):
                m = re.fullmatch(r'mov rax, qword ptr \[rcx \+ (0x[0-9a-f]+)\]; mov qword ptr \[r8\], rax; ret', asm[len(PREFIX):])
                if m:
                    offset = int(m.group(1), 0); form = 'inline'
            elif asm.startswith(WRAP):
                m = re.fullmatch(r'call (0x[0-9a-f]+); mov qword ptr \[rbx\], rax; add rsp, 0x20; pop rbx; ret', asm[len(WRAP):])
                if m:
                    target = int(m.group(1), 0)
                    leaf = decode(target)
                    q = re.fullmatch(r'mov rax, qword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret', leaf)
                    if q:
                        offset = int(q.group(1), 0); form = 'wrapper'
            if offset is None:
                rejects['unmatched_complete_code'] += 1; continue
            matches = [x for x in block['properties'] if x.get('kind') == kind and x.get('type_refs') == p.get('type_refs') and x.get('array_dim') == 1 and x.get('offset') == offset and not any(k in x for k in ('array_inner_of', 'map_key_of', 'map_value_of', 'set_element_of')) and re.fullmatch(r'[A-Za-z_]\w*', x.get('name', ''))]
            if len(matches) != 1:
                rejects['field_absent_or_ambiguous'] += 1; continue
            prop = matches[0]['name']
            # The generated declaration supplies the exact spelling; class_ref_map
            # independently ties its pointee to the reflected return and field.
            type_refs = p.get('type_refs') or {}
            ref = type_refs.get('class')
            block_va = refs.get(ref.lower()) if isinstance(ref, str) else None
            type_name = names.get(block_va.lower()) if block_va else None
            if set(type_refs) != {'class'} or not type_name or len(byname[type_name]) != 1:
                rejects['unresolved_reflected_type'] += 1; continue
            if len(headers[cls]) != 1:
                rejects['missing_or_ambiguous_header'] += 1; continue
            header = headers[cls][0].read_text()
            target_block = blocks.get(block_va.lower())
            if not target_block or int(target_block['class_flags'], 0) & 0x4000:
                rejects['target_interface_or_unavailable'] += 1; continue
            resolver = generator.Resolver({ref: type_name})
            ret = resolver.ctype(p, {})
            if resolver.todos or ret != type_name + '*':
                rejects['generator_type_unresolved'] += 1; continue
            fields = re.findall(r'^\s*([AU][A-Za-z_0-9]*\s*\*)\s*' + re.escape(prop) + r'\s*;', header, re.M)
            if len(fields) != 1 or re.sub(r'\s+', '', fields[0]) != ret:
                rejects['field_header_type_mismatch'] += 1; continue
            sigs = re.findall(r'^\s*([AU][A-Za-z_0-9]*\s*\*)\s*' + re.escape(name) + r'\(\)( const)?\s*;', header, re.M)
            if sigs and (len(sigs) != 1 or re.sub(r'\s+', '', sigs[0][0]) != ret or bool(sigs[0][1]) != bool(flags & 0x40000000)):
                rejects['header_signature_mismatch'] += 1; continue
            stem = f'{addr:x}_{cls}_{name}'
            inc = HERE / (stem + '.inc')
            body = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Mechanically matched complete object pointer read; see bulk_object_runtime/README.md.\nreturn {prop};\n'
            if inc.exists() and inc.read_text() != body:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists():
                inc.write_text(body)
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{ret} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} object getter at {hex(addr)}, reflected {type_name} field {prop} offset {hex(offset)}; source recovery only.'}
            results.append({'entry': entry, 'field': prop, 'offset': offset, 'type_ref': ref, 'assembly': asm, 'target': hex(target) if target else None, 'leaf': leaf if target else None})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'return_kinds': dict(sorted(kinds.items())), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'return_kinds', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
