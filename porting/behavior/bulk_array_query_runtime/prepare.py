#!/usr/bin/env python3
"""Recover exact zero-arg TArray Num / empty / nonempty queries into isolated proposals."""
import argparse, collections, hashlib, json, re, sys
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
    ap.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-array'))
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
            if (cls, name) in ported:
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
            props = block['properties']
            idx = [i for i, x in enumerate(props) if x.get('offset') == fo and x.get('kind') == 'Array' and not x.get('array_inner_of')]
            if len(idx) != 1:
                rej('field_absent_or_ambiguous'); continue
            i = idx[0]
            f = props[i]
            prop = f.get('name', '')
            inner = props[i + 1] if i + 1 < len(props) else {}
            same = [x for x in props if x.get('name') == prop]
            if not re.fullmatch(r'[A-Za-z_]\w*', prop) or f.get('array_dim') != 1 or len(same) != 2 or inner.get('array_inner_of') != prop or inner.get('array_dim') != 1 or any(k in f for k in ('map_key_of', 'map_value_of', 'set_element_of')):
                rej('not_plain_array_field'); continue
            ek = inner.get('kind')
            if ek not in ELEM:
                rej('unsupported_array_element'); continue
            if len(headers[cls]) != 1:
                rej('missing_or_ambiguous_header'); continue
            text = headers[cls][0].read_text()
            decls = re.findall(r'^\s*TArray<([^;]*?)>\s+' + re.escape(prop) + r'\s*;', text, re.M)
            if len(decls) != 1 or 'TODO' in decls[0] or not re.fullmatch(ELEM[ek], decls[0].replace(' ', '')):
                rej('header_declaration_unproven'); continue
            if len(re.findall(r'\b' + re.escape(prop) + r'\b\s*;', text)) != 1:
                rej('header_name_ambiguous'); continue
            sigs = re.findall(r'^\s*(int32|bool)\s+' + re.escape(name) + r'\(\)( const)?\s*;', text, re.M)
            want = 'int32' if kind == 'Int' else 'bool'
            if sigs and (len(sigs) != 1 or sigs[0][0] != want or bool(sigs[0][1]) != bool(flags & 0x40000000)):
                rej('header_signature_mismatch'); continue
            expr = prop + '.Num()' + ('' if kind == 'Int' else (' == 0' if op == 'sete' else ' != 0'))
            inc = HERE / f'{addr:x}_{cls}_{name}.inc'
            text_inc = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Mechanically matched complete TArray count query; see bulk_array_query_runtime/README.md.\nreturn {expr};\n'
            if inc.exists() and inc.read_text() != text_inc:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists():
                inc.write_text(text_inc)
            what = 'Num' if kind == 'Int' else ('IsEmpty' if op == 'sete' else 'IsNonEmpty')
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{want} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} array {what} query at {hex(addr)}, reflected TArray<{decls[0]}> field {prop} offset {hex(fo)} (count +8); source recovery only.'}
            results.append({'entry': entry, 'field': prop, 'array_offset': fo, 'element': decls[0], 'assembly': asm, 'target': hex(target) if target else None, 'leaf': leaf})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'recognized_code_by_return': dict(kinds), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'recognized_code_by_return', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
