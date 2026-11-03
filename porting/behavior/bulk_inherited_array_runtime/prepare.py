#!/usr/bin/env python3
"""Recover inherited exact zero-arg TArray Num / empty / nonempty queries into isolated proposals."""
import argparse, collections, hashlib, importlib.util, json, re, sys
from pathlib import Path

sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

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
HEX = r'(0x[0-9a-f]+)'
INT_LEAF = re.compile(r'mov eax, dword ptr \[rcx \+ ' + HEX + r'\]; ret')
BOOL_LEAF = re.compile(r'(?:xor eax, eax; )?cmp dword ptr \[rcx \+ ' + HEX + r'\], 0; (sete|setne) al; ret')
INT_INLINE = re.compile(r'mov eax, dword ptr \[rcx \+ ' + HEX + r'\]; mov dword ptr \[r8\], eax; ret')
BOOL_INLINE = re.compile(r'(?:xor eax, eax; )?cmp dword ptr \[rcx \+ ' + HEX + r'\], 0; (sete|setne) al; mov byte ptr \[r8\], al; ret')
CALL = re.compile(r'call ' + HEX + r'; (mov dword ptr \[rbx\], eax|mov byte ptr \[rbx\], al); add rsp, 0x20; pop rbx; ret')
# header element spelling must agree with the reflected inner record kind
ELEM = {'Object': r'[AU]\w+\*', 'Int': r'int32', 'Float': r'float', 'Name': r'FName', 'Str': r'FString',
        'Struct': r'F\w+', 'Text': r'FText', 'Byte': r'uint8|E\w+', 'Int64': r'int64'}
load = lambda p: json.loads(p.read_text())


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--revalidate-own', action='store_true')
    ap.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-inherited-array'))
    args = ap.parse_args()
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
    blocks = {c['block_va'].lower(): c for c in load(REFL / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    byname = collections.defaultdict(list)
    for va, n in names.items():
        byname[n].append(va)
    selected = {x.strip() for x in (ROOT / 'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
    own = str(HERE.relative_to(ROOT)) + '/'
    ported = {(e.get('class'), e.get('function')) for e in load(ROOT / 'porting/behavior/MANIFEST.json')
              if e.get('status') == 'ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own))}
    headers = collections.defaultdict(list)
    for module in ('FortniteGame', 'FortniteUI'):
        for p in (ROOT / 'porting/native/Source' / module / 'Generated').glob('*.h'):
            headers[p.stem].append(p)
    definitions = set()
    for cpp in (ROOT / 'porting/native/Source').rglob('*.cpp'):
        definitions.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{', cpp.read_text(errors='replace')))
    spec = importlib.util.spec_from_file_location('gen_native_classes', ROOT / 'porting/gen_native_classes.py')
    generator = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = generator
    spec.loader.exec_module(generator)
    tables = generator.TypeTables(str(REFL))
    type_map = {fn: info.cpp_name for fn, info in tables.by_fn.items() if info.cpp_name}
    def ancestry(va):
        chain, seen = [], set()
        while va:
            if va in seen: return []
            seen.add(va)
            meta, owner, block = parents.get(va, {}), names.get(va), blocks.get(va.lower())
            if meta.get('package') not in ('/Script/FortniteGame', '/Script/FortniteUI'):
                # An unknown Engine parent cannot prove the generated inheritance chain.
                if meta.get('name', '').startswith('unknownEngine'): return []
                break
            if not owner or owner.startswith('unknownEngine') or len(byname.get(owner, [])) != 1 or owner not in selected or not block or not meta.get('chain_ok') or int(block['class_flags'], 0) & 0x4000 or len(headers[owner]) != 1: return []
            header = headers[owner][0].read_text()
            base = re.search(r'class\s+\w+_API\s+' + re.escape(owner) + r'\s*:\s*public\s+(\w+)', header)
            if not base or base.group(1) != meta.get('parent_name') or base.group(1).startswith('unknownEngine'): return []
            chain.append((owner, block, header))
            va = meta.get('parent_va')
        return chain
    cache = {}
    results, rejects, kinds, total = [], collections.Counter(), collections.Counter(), 0
    for cls, funcs in sorted(native.items()):
        for name, fn in sorted(funcs.items()):
            total += 1
            rej = lambda r: rejects.update([r])
            if not isinstance(fn, dict):
                rej('invalid_record'); continue
            ps = fn.get('params')
            if not isinstance(ps, list) or len(ps) != 1 or ps[0].get('name') != 'ReturnValue' or int(ps[0].get('property_flags', '0'), 0) & 0x400 == 0:
                rej('not_zero_arg_getter'); continue
            p = ps[0]
            kind = p.get('kind')
            if kind not in ('Int', 'Bool'):
                rej('return_kind_not_int_bool'); continue
            if p.get('array_dim') != 1 or int(p.get('property_flags', '0'), 0) & 0x08000000 or p.get('type_refs') or p.get('element_size', {'Int': 4, 'Bool': 1}[kind]) != {'Int': 4, 'Bool': 1}[kind]:
                rej('return_shape'); continue
            try:
                flags, addr = int(fn['flags'], 0), int(fn['exec_addr'], 0)
            except (KeyError, TypeError, ValueError):
                rej('missing_flags_or_address'); continue
            if flags & (~0xffffffcf | NET | 0x08000000 | 0x20000000 | 0x2000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS:
                rej('flags'); continue
            if (cls, name) in ported or (cls, name) in definitions:
                rej('already_ported'); continue
            vs = byname.get(cls, [])
            if len(vs) != 1 or cls not in selected:
                rej('class_not_unique_or_selected'); continue
            va = vs[0]
            parent, block = parents.get(va, {}), blocks.get(va.lower())
            if not block or parent.get('name') != cls or parent.get('package') not in ('/Script/FortniteGame', '/Script/FortniteUI') or int(block['class_flags'], 0) & 0x4000:
                rej('class_or_package'); continue
            if not re.fullmatch(r'[A-Za-z_]\w*', name):
                rej('identifier'); continue
            asm = decode(addr)
            offset = target = leaf = op = None
            if asm.startswith(PREFIX):
                body = asm[len(PREFIX):]
                m = (INT_INLINE if kind == 'Int' else BOOL_INLINE).fullmatch(body)
                if m:
                    offset, op, form = int(m.group(1), 0), (m.group(2) if kind == 'Bool' else 'num'), 'inline'
            elif asm.startswith(WRAP):
                c = CALL.fullmatch(asm[len(WRAP):])
                if c and c.group(2).startswith('mov dword') == (kind == 'Int'):
                    target = int(c.group(1), 0)
                    leaf = decode(target)
                    m = (INT_LEAF if kind == 'Int' else BOOL_LEAF).fullmatch(leaf)
                    if m:
                        offset, op, form = int(m.group(1), 0), (m.group(2) if kind == 'Bool' else 'num'), 'wrapper'
            if offset is None or offset < 8:
                rej('unmatched_complete_code'); continue
            kinds[kind] += 1
            fo = offset - 8
            chain = cache.setdefault(va, ancestry(va)) if va not in cache else cache[va]
            if len(chain) < 2:
                rej('no_supported_ancestor'); continue
            matches = []
            for owner, owner_block, owner_header in chain:
                props = owner_block['properties']
                for i, f in enumerate(props):
                    if f.get('offset') != fo or f.get('kind') != 'Array' or f.get('array_inner_of'): continue
                    prop = f.get('name', '')
                    inner = props[i + 1] if i + 1 < len(props) else {}
                    if not re.fullmatch(r'[A-Za-z_]\w*', prop) or f.get('array_dim') != 1 or len([x for x in props if x.get('name') == prop]) != 2 or inner.get('array_inner_of') != prop or inner.get('array_dim') != 1 or any(k in f for k in ('map_key_of', 'map_value_of', 'set_element_of')): continue
                    matches.append((owner, owner_header, prop, f, inner))
            if len(matches) != 1:
                rej('field_absent_or_ambiguous'); continue
            owner, owner_header, prop, field, inner = matches[0]
            if owner == cls:
                rej('not_inherited_field'); continue
            # Exact generated type must resolve from the reflected element reference.
            resolver = generator.Resolver(type_map)
            field_type = resolver.ctype(field, {'elem': inner})
            if resolver.todos or 'TODO' in field_type:
                rej('unresolved_array_element'); continue
            decls = list(re.finditer(r'^\s*' + re.escape(field_type) + r'\s+' + re.escape(prop) + r'\s*;', owner_header, re.M))
            if len(decls) != 1 or len(re.findall(r'\b' + re.escape(prop) + r'\b\s*;', owner_header)) != 1:
                rej('header_declaration_unproven'); continue
            labels = list(re.finditer(r'^\s*(public|protected|private):', owner_header[:decls[0].start()], re.M))
            if not labels or labels[-1].group(1) == 'private':
                rej('inaccessible_field'); continue
            text = chain[0][2]
            any_decl = re.search(r'^\s*[^\n;]*\b' + re.escape(name) + r'\s*\(', text, re.M)
            sigs = re.findall(r'^\s*(int32|bool)\s+' + re.escape(name) + r'\(\)( const)?\s*;', text, re.M)
            want = 'int32' if kind == 'Int' else 'bool'
            if any_decl and (len(sigs) != 1 or sigs[0][0] != want or bool(sigs[0][1]) != bool(flags & 0x40000000)):
                rej('header_signature_mismatch'); continue
            expr = owner + '::' + prop + '.Num()' + ('' if kind == 'Int' else (' == 0' if op == 'sete' else ' != 0'))
            inc = HERE / f'{addr:x}_{cls}_{name}.inc'
            text_inc = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Mechanically matched complete TArray count query; see bulk_inherited_array_runtime/README.md.\nreturn {expr};\n'
            if inc.exists() and inc.read_text() != text_inc:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists():
                inc.write_text(text_inc)
            what = 'Num' if kind == 'Int' else ('IsEmpty' if op == 'sete' else 'IsNonEmpty')
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{want} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} array {what} query at {hex(addr)}, reflected {field_type} field {owner}::{prop} offset {hex(fo)} (count +8); source recovery only.'}
            results.append({'entry': entry, 'field': prop, 'field_owner': owner, 'array_offset': fo, 'element': field_type, 'assembly': asm, 'target': hex(target) if target else None, 'leaf': leaf})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'recognized_code_by_return': dict(kinds), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'recognized_code_by_return', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
