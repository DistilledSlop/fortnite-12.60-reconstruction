#!/usr/bin/env python3
"""Recover exact enum byte-field getters into isolated proposals."""
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
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-enum-getter'))
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
    maintained = set()
    for cpp in (ROOT / 'porting/native/Source').rglob('*.cpp'):
        if 'Generated' not in cpp.parts:
            maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{', cpp.read_text(errors='replace')))
    headers = collections.defaultdict(list)
    for module in ('FortniteGame', 'FortniteUI'):
        for p in (ROOT / 'porting/native/Source' / module / 'Generated').glob('*.h'):
            headers[p.stem].append(p)
    enum_headers = collections.defaultdict(list)
    for module in ('FortniteGame', 'FortniteUI'):
        for p in (ROOT / 'porting/native/Source' / module / 'Generated').glob('*.h'):
            enum_headers[p.name].append(p)
    spec = importlib.util.spec_from_file_location('gen_native_classes', ROOT / 'porting/gen_native_classes.py')
    generator = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = generator
    spec.loader.exec_module(generator)
    tables = generator.TypeTables(str(REFL))
    def ancestry(va):
        chain, seen = [], set()
        while va:
            if va in seen: return []
            seen.add(va)
            meta, owner, block = parents.get(va, {}), names.get(va), blocks.get(va.lower())
            if meta.get('package') not in ('/Script/FortniteGame', '/Script/FortniteUI'): break
            if not owner or len(byname.get(owner, [])) != 1 or owner not in selected or not block or not meta.get('chain_ok') or int(block['class_flags'], 0) & 0x4000 or len(headers[owner]) != 1: return []
            header = headers[owner][0].read_text()
            base = re.search(r'class\s+\w+_API\s+' + re.escape(owner) + r'\s*:\s*public\s+(\w+)', header)
            if not base or base.group(1) != meta.get('parent_name'): return []
            chain.append((owner, block, header))
            va = meta.get('parent_va')
        return chain
    cache = {}
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
            if not isinstance(ps, list):
                rejects['not_enum_return'] += 1; continue
            pairs, notes = generator.split_properties(ps)
            if notes or len(pairs) != 1 or len(ps) != 2:
                rejects['not_enum_return'] += 1; continue
            p, children = pairs[0]
            if children or p.get('name') != 'ReturnValue' or p.get('kind') != 'Enum' or int(p.get('property_flags', '0'), 0) & 0x400 == 0 or not generator.enum_underlying_record(ps[1], 'ReturnValue') or ps[1].get('kind') != 'Byte' or int(ps[1].get('property_flags', '0'), 0) != 0:
                rejects['not_byte_enum_return'] += 1; continue
            kinds['Enum'] += 1
            if p.get('array_dim') != 1 or int(p.get('property_flags', '0'), 0) & 0x08000000 or set(p.get('type_refs') or {}) != {'enum'}:
                rejects['return_reference_array_or_type'] += 1; continue
            try:
                flags, addr = int(fn['flags'], 0), int(fn['exec_addr'], 0)
            except (KeyError, TypeError, ValueError):
                rejects['missing_flags_or_address'] += 1; continue
            if flags & (~0xffffffcf | NET | 0x08000000 | 0x20000000 | 0x2000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS:
                rejects['flags'] += 1; continue
            if (cls, name) in ported or (cls, name) in maintained:
                rejects['already_ported_or_maintained'] += 1; continue
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
            q = None
            expression = r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]'
            if asm.startswith(PREFIX):
                q = re.fullmatch(expression + r'; mov byte ptr \[r8\], al; ret', asm[len(PREFIX):])
                form = 'inline'
            elif asm.startswith(WRAP):
                m = re.fullmatch(r'call (0x[0-9a-f]+); mov byte ptr \[rbx\], al; add rsp, 0x20; pop rbx; ret', asm[len(WRAP):])
                if m:
                    target = int(m.group(1), 0)
                    leaf = decode(target)
                    q = re.fullmatch(expression + r'; ret', leaf)
                    form = 'wrapper'
            if not q:
                rejects['unmatched_complete_code'] += 1; continue
            offset = int(q.group(1), 0)
            chain = cache.setdefault(va, ancestry(va)) if va not in cache else cache[va]
            matches = []
            for owner, owner_block, owner_header in chain:
                for field, children in generator.split_properties(owner_block['properties'])[0]:
                    if not children and field.get('kind') in ('Byte', 'Enum') and set(field.get('type_refs') or {}) == {'enum'} and field.get('array_dim') == 1 and field.get('offset') == offset and re.fullmatch(r'[A-Za-z_]\w*', field.get('name', '')):
                        matches.append((owner, field, owner_header))
            if len(matches) != 1:
                rejects['field_absent_or_ambiguous'] += 1; continue
            owner, field, owner_header = matches[0]
            prop, field_kind = field['name'], field['kind']
            # An inherited name hidden by a more-derived field must not bind incorrectly.
            if any(any(p.get('name') == prop for p, _ in generator.split_properties(b['properties'])[0]) for o, b, h in chain[:next(i for i, (o, _, _) in enumerate(chain) if o == owner)]):
                rejects['hidden_inherited_field'] += 1; continue
            ref = field['type_refs']['enum']
            info = tables.by_fn.get(ref.lower()) if isinstance(ref, str) else None
            if not info or info.kind != 'enum' or not info.cpp_name:
                rejects['unresolved_reflected_type'] += 1; continue
            if field_kind == 'Enum':
                props = next(b['properties'] for o, b, h in chain if o == owner)
                i = next((i for i, x in enumerate(props) if x is field), -1)
                if i < 0 or i + 1 >= len(props) or not generator.enum_underlying_record(props[i + 1], prop) or props[i + 1].get('kind') != 'Byte' or int(props[i + 1].get('property_flags', '0'), 0) != 0:
                    rejects['enum_underlying_not_byte'] += 1; continue
                eh = enum_headers.get(info.header_name, [])
                if len(eh) != 1 or not re.search(r'\benum\s+class\s+' + re.escape(info.cpp_name) + r'\s*:\s*uint8\b', eh[0].read_text()):
                    rejects['enum_header_underlying_mismatch'] += 1; continue
            resolver = generator.Resolver({ref: info.cpp_name})
            field_type = resolver.ctype(field, {})
            if resolver.todos or field_type != (f'TEnumAsByte<{info.cpp_name}>' if field_kind == 'Byte' else info.cpp_name):
                rejects['generator_type_unresolved'] += 1; continue
            declarations = list(re.finditer(r'^\s*' + re.escape(field_type) + r'\s+' + re.escape(prop) + r'\s*;', owner_header, re.M))
            if len(declarations) != 1:
                rejects['field_header_type_mismatch'] += 1; continue
            labels = list(re.finditer(r'^\s*(public|protected|private):', owner_header[:declarations[0].start()], re.M))
            if not labels or (owner != cls and labels[-1].group(1) == 'private'):
                rejects['inaccessible_field'] += 1; continue
            return_ref = p['type_refs']['enum']
            return_info = tables.by_fn.get(return_ref.lower()) if isinstance(return_ref, str) else None
            if not return_info or return_info.kind != 'enum' or return_info.cpp_name != info.cpp_name or return_ref.lower() != ref.lower():
                rejects['return_field_enum_mismatch'] += 1; continue
            eh = enum_headers.get(info.header_name, [])
            if len(eh) != 1 or not re.search(r'\benum\s+class\s+' + re.escape(info.cpp_name) + r'\s*:\s*uint8\b', eh[0].read_text()):
                rejects['enum_header_underlying_mismatch'] += 1; continue
            ret_resolver = generator.Resolver({return_ref: info.cpp_name})
            ret, arguments = generator.decode_params(ps, ret_resolver)
            if ret_resolver.todos or arguments or ret != info.cpp_name:
                rejects['return_generator_type_mismatch'] += 1; continue
            header = headers[cls][0].read_text()
            sigs = re.findall(r'^\s*' + re.escape(ret) + r'\s+' + re.escape(name) + r'\(\)( const)?\s*;', header, re.M)
            any_decl = re.search(r'^\s*[^\n;]*\b' + re.escape(name) + r'\s*\(', header, re.M)
            if any_decl and (len(sigs) != 1 or bool(sigs[0]) != bool(flags & 0x40000000)):
                rejects['header_signature_mismatch'] += 1; continue
            value_expr = f"{prop}.GetValue()" if field_kind == 'Byte' else prop
            stem = f'{addr:x}_{cls}_{name}'
            inc = HERE / (stem + '.inc')
            body = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Exact enum byte-field getter; see bulk_enum_getter_runtime/README.md.\nreturn {value_expr};\n'
            if inc.exists() and inc.read_text() != body:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists():
                inc.write_text(body)
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{ret} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} enum getter at {hex(addr)}, reflected {field_kind} enum field {owner}::{prop} offset {hex(offset)}; source recovery only.'}
            results.append({'entry': entry, 'field': prop, 'field_owner': owner, 'field_kind': field_kind, 'offset': offset, 'enum': info.cpp_name, 'enum_ref': ref, 'assembly': asm, 'target': hex(target) if target else None, 'leaf': leaf if target else None})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'return_kinds': dict(sorted(kinds.items())), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'return_kinds', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
