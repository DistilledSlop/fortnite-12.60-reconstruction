#!/usr/bin/env python3
"""Recover complete scalar/object argument wrappers calling constant Boolean leaves."""
import collections
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
REF = ROOT / 'porting/generated/reflection'
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT / 'porting')]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import gen_native_classes as gen

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
BASE = 0x140000000
OUT = Path('/tmp/gameplay-bulk-parameter-constant')
ACCESS = {0x20000: 'public', 0x40000: 'private', 0x80000: 'protected'}
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
load = lambda p: json.loads(p.read_text())

# Stack layout is literal: every extraction and saved register has a distinct
# slot, the branch destinations are checked as instruction indices below.
INSTANCE_OBJECT = '''mov qword ptr [rsp + 8], rbx; mov qword ptr [rsp + 0x18], rbp; mov qword ptr [rsp + 0x20], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rbp, rcx; mov rsi, r8; mov rbx, rdx; mov rcx, rdx; mov qword ptr [rsp + 0x38], rdi; cmp qword ptr [rdx + 0x20], rdi; je ALT; mov rdx, qword ptr [rdx + 0x18]; lea r8, [rsp + 0x38]; call 0x142ed9cf0; jmp JOIN; mov r8, qword ptr [rdx + 0x80]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rdx + 0x80], rax; lea rdx, [rsp + 0x38]; call 0x142ed9d20; mov rax, qword ptr [rbx + 0x20]; mov rcx, rbp; mov rdx, qword ptr [rsp + 0x38]; test rax, rax; setne dil; add rdi, rax; mov qword ptr [rbx + 0x20], rdi; call LEAF; mov rbx, qword ptr [rsp + 0x30]; mov rbp, qword ptr [rsp + 0x40]; mov byte ptr [rsi], al; mov rsi, qword ptr [rsp + 0x48]; add rsp, 0x20; pop rdi; ret'''
INSTANCE_INT = INSTANCE_OBJECT.replace('mov qword ptr [rsp + 0x38], rdi', 'mov dword ptr [rsp + 0x38], edi').replace('mov rdx, qword ptr [rsp + 0x38]', 'mov edx, dword ptr [rsp + 0x38]')
STATIC_OBJECT = '''mov qword ptr [rsp + 8], rbx; mov qword ptr [rsp + 0x18], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, r8; mov rbx, rdx; mov qword ptr [rsp + 0x38], rdi; mov rcx, rdx; cmp qword ptr [rdx + 0x20], rdi; je ALT; mov rdx, qword ptr [rdx + 0x18]; lea r8, [rsp + 0x38]; call 0x142ed9cf0; jmp JOIN; mov r8, qword ptr [rdx + 0x80]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rdx + 0x80], rax; lea rdx, [rsp + 0x38]; call 0x142ed9d20; mov rax, qword ptr [rbx + 0x20]; mov rcx, qword ptr [rsp + 0x38]; test rax, rax; setne dil; add rdi, rax; mov qword ptr [rbx + 0x20], rdi; call LEAF; mov rbx, qword ptr [rsp + 0x30]; mov byte ptr [rsi], al; mov rsi, qword ptr [rsp + 0x40]; add rsp, 0x20; pop rdi; ret'''
TEMPLATES = {(False, 'Object'): INSTANCE_OBJECT, (False, 'Int'): INSTANCE_INT,
             (False, 'UInt32'): INSTANCE_INT, (True, 'Object'): STATIC_OBJECT}


def main():
    image = IMAGE.read_bytes()
    digest = hashlib.sha256(image).hexdigest()
    if digest != SHA:
        raise SystemExit('original image digest mismatch')
    md = Cs(CS_ARCH_X86, CS_MODE_64)

    def decode(addr):
        if not BASE <= addr < BASE + len(image) - 256:
            return []
        result = []
        for ins in md.disasm(image[addr - BASE:addr - BASE + 256], addr):
            result.append(ins)
            if ins.mnemonic == 'ret':
                return result
        return []

    names = load(REF / 'class_name_map.json')
    parents = load(REF / 'class_parent_map.json')
    blocks = {b['block_va'].lower(): b for b in load(REF / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    refs = load(REF / 'class_ref_map.json')['resolved']
    selected = {s.strip() for s in (ROOT / 'porting/native_class_set.txt').read_text().splitlines() if s.strip() and not s.lstrip().startswith('#')}
    done = {(e.get('class'), e.get('function')) for e in load(ROOT / 'porting/behavior/MANIFEST.json') if e.get('status') == 'ported'}
    byname = collections.defaultdict(list)
    headers = collections.defaultdict(list)
    for va, name in names.items():
        byname[name].append(va)
    for p in (ROOT / 'porting/native/Source').glob('*/Generated/*.h'):
        headers[p.stem].append(p)
    maintained = set()
    for p in (ROOT / 'porting/native/Source').rglob('*.cpp'):
        if 'Generated' not in p.parts:
            maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{', p.read_text(errors='replace')))
    results, rejects = [], collections.Counter()
    total = 0
    for cls, funcs in sorted(load(REF / 'native_funcs.json').items()):
        for name, fn in sorted(funcs.items()):
            total += 1
            if not isinstance(fn, dict) or not isinstance(fn.get('params'), list):
                continue
            pairs, notes = gen.split_properties(fn['params'])
            if notes or len(pairs) != 2 or any(child for _, child in pairs):
                continue
            returns = [p for p, _ in pairs if int(p.get('property_flags', '0'), 0) & 0x400]
            inputs = [p for p, _ in pairs if not int(p.get('property_flags', '0'), 0) & 0x400]
            if len(returns) != 1 or len(inputs) != 1:
                continue
            ret, param = returns[0], inputs[0]
            if ret.get('kind') != 'Bool' or ret.get('array_dim') != 1 or ret.get('type_refs') or int(ret.get('property_flags', '0'), 0) != 0x10000000000580:
                continue
            if param.get('array_dim') != 1 or int(param.get('property_flags', '0'), 0) != 0x10000000000080 or not re.fullmatch(r'[A-Za-z_]\w*', param.get('name', '')):
                continue
            try:
                flags, addr = int(fn['flags'], 0), int(fn['exec_addr'], 0)
            except (KeyError, TypeError, ValueError):
                continue
            static = bool(flags & 0x2000)
            template = TEMPLATES.get((static, param.get('kind')))
            if not template or flags & (~0xffffffcf | NET | 0x08000000 | 0x800 | 0x20000000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS or (static and flags & 0x40000000):
                continue
            if (cls, name) in done or (cls, name) in maintained or cls not in selected or len(byname[cls]) != 1 or len(headers[cls]) != 1 or not re.fullmatch(r'[A-Za-z_]\w*', name):
                continue
            va = byname[cls][0]
            parent, block = parents.get(va, {}), blocks.get(va.lower())
            if not block or parent.get('name') != cls or parent.get('package') not in ('/Script/FortniteGame', '/Script/FortniteUI') or int(block['class_flags'], 0) & 0x4000:
                continue
            ctype = {'Int': 'int32', 'UInt32': 'uint32'}.get(param['kind'])
            param_refs = param.get('type_refs') or {}
            if param['kind'] == 'Object':
                ref = param_refs.get('class', '').lower()
                target_va = refs.get(ref, '').lower()
                target_name = names.get(target_va)
                target_block = blocks.get(target_va)
                if set(param_refs) != {'class'} or not target_name or target_name not in selected or not target_block or int(target_block['class_flags'], 0) & 0x4000:
                    rejects['object_type'] += 1
                    continue
                ctype = target_name + '*'
            elif param_refs:
                continue
            resolver = gen.Resolver({k: names.get(v.lower(), '') for k, v in refs.items()})
            resolved_ret, resolved_args = gen.decode_params(fn['params'], resolver)
            if resolver.todos or resolved_ret != 'bool' or resolved_args != [f'{ctype} {param["name"]}']:
                rejects['generator_type'] += 1
                continue
            ins = decode(addr)
            expected = template.split('; ')
            if len(ins) != len(expected):
                rejects['wrapper'] += 1
                continue
            alt = next(i for i, s in enumerate(expected) if s == 'mov r8, qword ptr [rdx + 0x80]')
            join = next(i for i, s in enumerate(expected) if s == 'call 0x142ed9d20') + 1
            leaf = None
            for i, want in zip(ins, expected):
                actual = (i.mnemonic + ' ' + i.op_str).rstrip()
                if want == 'je ALT':
                    want = f'je {hex(ins[alt].address)}'
                elif want == 'jmp JOIN':
                    want = f'jmp {hex(ins[join].address)}'
                elif want == 'call LEAF':
                    if i.mnemonic != 'call' or not re.fullmatch(r'0x[0-9a-f]+', i.op_str):
                        break
                    leaf = int(i.op_str, 0)
                    want = actual
                if actual != want:
                    break
            else:
                leaf_ins = decode(leaf)
                leaf_text = '; '.join((i.mnemonic + ' ' + i.op_str).rstrip() for i in leaf_ins)
                value = {'xor al, al; ret': False, 'mov al, 1; ret': True}.get(leaf_text)
                if value is None:
                    rejects['nonconstant_leaf'] += 1
                    continue
                sig = f'bool {name}({ctype} {param["name"]})' + (' const' if flags & 0x40000000 else '')
                header = headers[cls][0].read_text()
                cpp = headers[cls][0].with_suffix('.cpp')
                if (cpp.exists() and re.search(r'\b' + re.escape(cls) + r'::' + re.escape(name) + r'\s*\(', cpp.read_text())) or re.search(r'\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*(?:const\s*)?\{', header):
                    rejects['existing_generated_body'] += 1
                    continue
                declarations = re.findall(r'^\s*[^\n;]*\b' + re.escape(name) + r'\s*\([^;{}\n]*\)[^;{}\n]*;', header, re.M)
                if declarations and (len(declarations) != 1 or not re.fullmatch(r'\s*' + ('static\\s+' if static else '') + re.escape(sig) + r'\s*;', declarations[0])):
                    rejects['declaration'] += 1
                    continue
                inc = HERE / f'{addr:x}_{cls}_{name}.inc'
                body = f'// Original exec {hex(addr)}, complete constant leaf {hex(leaf)}.\n// CONFIDENCE: HIGH\nreturn {str(value).lower()};\n'
                if inc.exists() and inc.read_text() != body:
                    raise SystemExit('existing snippet differs: ' + str(inc))
                inc.write_text(body)
                entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': sig, 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': static, 'access': ACCESS[flags & 0xe0000], 'notes': f'Complete single-parameter extraction wrapper calls exact constant Boolean leaf {hex(leaf)}; source recovery only.'}
                results.append({'entry': entry, 'parameter': param, 'exec': [(i.mnemonic + ' ' + i.op_str).rstrip() for i in ins], 'leaf': leaf_text, 'leaf_address': hex(leaf)})
                continue
            rejects['wrapper'] += 1
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'rejections': dict(rejects), 'results': results}
    (OUT / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'rejections')}))


if __name__ == '__main__':
    main()
