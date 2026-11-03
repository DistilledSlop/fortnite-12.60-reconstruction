#!/usr/bin/env python3
"""Recover exact pure POD-struct field-copy getters into isolated proposals."""
import argparse
import collections
import hashlib
import importlib.util
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'porting/behavior/native_class_size_evidence_runtime'))
from scan import extract

ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT / 'porting/generated/reflection'
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
# Verified CoreUObject POD structs: reflected name -> byte size.
PODS = {'Vector': 12, 'Rotator': 12, 'Vector2D': 8, 'LinearColor': 16, 'Guid': 16, 'Color': 4, 'Quat': 16, 'Vector4': 16}
# P_FINISH frame advance (r9 form: rcx stays `this`).
ADV = 'mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP_HEAD = 'push rbx; sub rsp, (0x[0-9a-f]+); mov rax, qword ptr \\[rdx \\+ 0x20\\]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr \\[rdx \\+ 0x20\\], r9; lea rdx, \\[rsp \\+ (0x[0-9a-f]+)\\]; call (0x[0-9a-f]+); '
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
ACCESS = {0x20000: 'public', 0x40000: 'private', 0x80000: 'protected'}
RET_FLAGS = 0x10000000000580  # public-native | Parm | OutParm | ReturnParm; no Const/Reference/etc.
FAMILY = {'eax': 'a', 'rax': 'a', 'ecx': 'c', 'rcx': 'c', 'edx': 'd', 'rdx': 'd'}
GPR4, GPR8 = ('eax', 'ecx', 'edx'), ('rax', 'rcx', 'rdx')
MEM = r'\[(r[a-z0-9]+)(?: \+ (0x[0-9a-f]+|[0-9]+))?\]'
load = lambda p: json.loads(p.read_text())


def copy_cover(instrs, src, dst, size, leaf):
    """Prove a straight-line sequence only copies size bytes from [src+delta..] to [dst+0..] unchanged.
    Returns delta, or None. Handles only mov/movss/movsd/movups/movdqu loads and stores of proven registers.
    leaf=True additionally requires `mov rax, rdx` with rax/rdx intact at the end (sret return)."""
    fam, bases, xmm = {}, {}, {}
    for n in (src, dst):
        if n in FAMILY:
            fam[FAMILY[n]] = ('base', n)
        else:
            bases[n] = ('base', n)
    rax_is_out = False
    def getbase(n):
        return fam.get(FAMILY[n]) if n in FAMILY else bases.get(n)
    def setdata(n, v):
        if n in FAMILY:
            fam[FAMILY[n]] = v
        else:
            bases.pop(n, None)
    loads, stores = [], []
    for ins in instrs:
        m = re.fullmatch(r'(mov) (eax|ecx|edx), dword ptr ' + MEM, ins)
        sz = 4
        if not m:
            m = re.fullmatch(r'(mov) (rax|rcx|rdx), qword ptr ' + MEM, ins); sz = 8
        if not m:
            m = re.fullmatch(r'(movss) (xmm[01]), dword ptr ' + MEM, ins); sz = 4
        if not m:
            m = re.fullmatch(r'(movsd) (xmm[01]), qword ptr ' + MEM, ins); sz = 8
        if not m:
            m = re.fullmatch(r'(movups|movdqu) (xmm[01]), xmmword ptr ' + MEM, ins); sz = 16
        if m:
            d, b, off = m.group(2), m.group(3), int(m.group(4) or '0', 0)
            if getbase(b) != ('base', src):
                return None
            if leaf and d in FAMILY and FAMILY[d] == 'a' and rax_is_out:
                return None   # rax already holds the out pointer
            loads.append((off, sz))
            v = ('data', off, sz, len(loads))
            if d.startswith('xmm'):
                xmm[d] = v
            else:
                if d in FAMILY and FAMILY[d] == 'd' and leaf:
                    return None   # rdx holds the out pointer
                setdata(d, v)
            continue
        m = re.fullmatch(r'mov dword ptr ' + MEM + ', (eax|ecx|edx)', ins)
        sz = 4
        if not m:
            m = re.fullmatch(r'mov qword ptr ' + MEM + ', (rax|rcx|rdx)', ins); sz = 8
        if not m:
            m = re.fullmatch(r'movss dword ptr ' + MEM + ', (xmm[01])', ins); sz = 4
        if not m:
            m = re.fullmatch(r'movsd qword ptr ' + MEM + ', (xmm[01])', ins); sz = 8
        if not m:
            m = re.fullmatch(r'(?:movups|movdqu) xmmword ptr ' + MEM + ', (xmm[01])', ins); sz = 16
        if m:
            b, off, r = m.group(1), int(m.group(2) or '0', 0), m.group(3)
            if getbase(b) != ('base', dst):
                return None
            v = xmm.get(r) if r.startswith('xmm') else fam.get(FAMILY[r])
            if not v or v[0] != 'data' or v[2] < sz:
                return None
            stores.append((off, sz, v[1], v[3]))
            continue
        if leaf and ins == 'mov rax, rdx' and fam.get('d') == ('base', 'rdx') and not rax_is_out:
            fam['a'] = ('base', 'rdx'); rax_is_out = True
            continue
        return None
    if leaf and (not rax_is_out or fam.get('a') != ('base', 'rdx') or fam.get('d') != ('base', 'rdx')):
        return None
    if not stores:
        return None
    deltas = {s - d for d, _, s, _ in stores}
    if len(deltas) != 1:
        return None
    delta = deltas.pop()
    covered = set()
    for d, sz, _, _ in stores:
        covered.update(range(d, d + sz))
    if covered != set(range(size)) or delta < 0:
        return None
    for off, sz in loads:
        if off < delta or off + sz > delta + size:
            return None
    used = {n for _, _, _, n in stores}
    if used != set(range(1, len(loads) + 1)):
        return None
    return delta


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revalidate-own', action='store_true')
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-native-copy'))
    args = parser.parse_args()
    image = IMAGE.read_bytes()
    digest = hashlib.sha256(image).hexdigest()
    if digest != SHA:
        raise SystemExit('image digest mismatch')
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True

    def decode(addr):
        pos = addr - BASE
        if pos < 0 or pos + 128 > len(image):
            return None
        ins = []
        for i in md.disasm(image[pos:pos + 128], addr):
            ins.append((i.mnemonic + ' ' + i.op_str).rstrip())
            if i.mnemonic == 'ret':
                return ins
            if i.mnemonic.startswith('j'):
                return None
        return None
    native = load(REFL / 'native_funcs.json')
    names = load(REFL / 'class_name_map.json')
    parents = load(REFL / 'class_parent_map.json')
    blocks = {c['block_va'].lower(): c for c in load(REFL / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    bits = load(REFL / 'bool_bits.json')['by_class']
    index = load(REFL / 'native_class_index.json')
    targets = collections.Counter(g.lower() for v in index.values() for g in v.get('gpsc_targets', []))
    size_cache = {}
    manifest = load(ROOT / 'porting/behavior/MANIFEST.json')
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
    tables = generator.TypeTables(str(REFL))
    pods = {}   # struct construct fn -> (cpp name, size)
    for fn, info in tables.by_fn.items():
        if info.kind == 'struct' and info.package == '/Script/CoreUObject' and info.rec['name'] in PODS and info.cpp_name == 'F' + info.rec['name'] and info.rec['size'] == PODS[info.rec['name']]:
            pods[fn] = (info.cpp_name, info.rec['size'])
    if len(pods) < len(PODS):
        raise SystemExit('POD struct tables incomplete')
    def size_of(va):
     if not isinstance(va,str): return None
     va=va.lower()
     if va in size_cache: return size_cache[va]
     block=blocks.get(va); name=names.get(va); parent=parents.get(va) or {}; entry=index.get(name) or {}
     gs=entry.get('gpsc_targets') or []
     if not block or not name or byname.get(name)!=[va] or parent.get('name')!=name or not parent.get('package') or entry.get('block_va','').lower()!=va or len(gs)!=1 or targets[gs[0].lower()]!=1:
      size_cache[va]=None; return None
     proof=extract(image,md,int(gs[0],0),name,parent['package'])
     if proof is None: size_cache[va]=None; return None
     pairs,_=generator.split_properties(block.get('properties',[]))
     ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
     if len(ss)>1 or ss and next(iter(ss))!=proof['size']: raise SystemExit('reflected size conflict: '+name)
     size_cache[va]=(proof['size'],bool(ss),proof['gpsc'])
     return size_cache[va]
    def occupancy(cls,block,own):
     pairs,notes=generator.split_properties(block['properties'])
     if notes: return None
     props=[p for p,_ in pairs]
     locations=[]
     for p in props:
      if p.get('kind')=='Bool':
       bs=[b for b in bits.get(cls,[]) if b['name']==p['name']]
       if len(bs)!=1 or not isinstance(bs[0].get('offset'),int): return None
       loc=bs[0]['offset']
      else: loc=p.get('offset')
      if not isinstance(loc,int) or loc<0 or loc>=own: return None
      locations.append((loc,p))
     fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_copy_runtime')]
     for e in fields:
      try: loc=int(str(e['offset']),0)
      except (KeyError,ValueError,TypeError): return None
      if loc<0 or loc>=own: return None
      locations.append((loc,e))
     hs=headers.get(cls,[])
     if len(hs)!=1: return None
     hdr=hs[0].read_text()
     annotated=re.findall(r'// Non-reflected field \(behaviour kind "member_field", offset (0x[0-9a-fA-F]+)\):[^\n]*\n\s*([^;\n]+);',hdr)
     native_fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported']
     if hdr.count('// Non-reflected field (behaviour kind "member_field"')!=len(annotated): return None
     for off,decl in annotated:
      if not any(str(e.get('offset','')).lower()==off.lower() and (e.get('signature') or '').strip()==decl.strip() for e in native_fields): return None
     offsets=sorted({loc for loc,_ in locations}|{own})
     widths={'Bool':1,'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'Int':4,'UInt32':4,'Float':4,'Int64':8,'UInt64':8,'Double':8,'Name':8,'Object':8,'Class':8,'Array':16,'Str':16}
     spans=[]
     for loc,p in locations:
      kind=p.get('kind'); width=widths.get(kind)
      if kind=='member_field':
       typ=(p.get('signature') or '').split(' ',1)[0]
       width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get(typ)
      dim=p.get('array_dim',1)
      if not isinstance(dim,int) or dim<1: return None
      end=loc+width*dim if width else next(x for x in offsets if x>loc)
      if end>own: return None
      spans.append((loc,end))
     return spans
    definitions = set()
    for path in (ROOT / 'porting/native/Source').rglob('*.cpp'):
        if 'Generated' in path.parts:
            continue
        for cls, name in re.findall(r'\b([AUF]\w*)::(\w+)\s*\(', path.read_text(errors='replace')):
            definitions.add((cls, name))
    fields = {}
    results, rejects, total, considered = [], collections.Counter(), 0, 0
    for cls, funcs in sorted(native.items()):
        for name, fn in sorted(funcs.items()):
            total += 1
            ps = fn.get('params') if isinstance(fn, dict) else None
            if not isinstance(ps, list) or len(ps) != 1 or ps[0].get('name') != 'ReturnValue':
                rejects['not_zero_arg_getter'] += 1; continue
            p = ps[0]
            ref = None
            if p.get('kind') == 'Name' and not p.get('type_refs'):
                cpp, size = 'FName', 8
            elif p.get('kind') == 'Struct':
                ref = (p.get('type_refs') or {}).get('struct')
                if set(p.get('type_refs') or {}) != {'struct'} or not isinstance(ref, str) or ref.lower() not in pods:
                    rejects['struct_not_verified_pod'] += 1; continue
                cpp, size = pods[ref.lower()]
            else:
                rejects['unsupported_return'] += 1; continue
            considered += 1
            if p.get('array_dim') != 1 or int(p.get('property_flags', '0'), 0) != RET_FLAGS:
                rejects['return_flags_const_ref_or_array'] += 1; continue
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
            own_proof, parent_proof = size_of(va), size_of(parent.get('parent_va'))
            if not own_proof or not parent_proof:
                rejects['size_proof'] += 1; continue
            own_size, parent_size = own_proof[0], parent_proof[0]
            if len(headers[cls]) != 1:
                rejects['missing_or_ambiguous_header'] += 1; continue
            header = headers[cls][0].read_text()
            declaration = re.findall(r'\bclass\s+\w+_API\s+' + re.escape(cls) + r'\s*:\s*public\s+(\w+)', header)
            if declaration != [parent.get('parent_name')] or names.get(parent.get('parent_va')) != parent.get('parent_name'):
                rejects['generated_parent_mismatch'] += 1; continue
            spans = occupancy(cls, block, own_size)
            if spans is None:
                rejects['unknown_occupancy'] += 1; continue
            code = decode(addr)
            if not code:
                rejects['undecodable_or_branching'] += 1; continue
            text = '; '.join(code) + '; '
            offset, form, target, leaf_code = None, None, None, None
            if text.startswith(ADV):
                offset = copy_cover(code[6:-1], 'rcx', 'r8', size, False)
                form = 'inline'
            else:
                m = re.match(WRAP_HEAD, text)
                if m:
                    S, T, tgt = int(m.group(1), 0), int(m.group(2), 0), int(m.group(3), 0)
                    tail = code[len(re.findall(';', m.group(0))):]
                    if (S % 16 == 0 and T >= 0x20 and (T + size <= S or (cpp == 'FName' and S == 0x20 and T == 0x38)) and len(tail) >= 4 and tail[-3:] == [f'add rsp, {hex(S)}', 'pop rbx', 'ret']):
                        leaf_code = decode(tgt)
                        if leaf_code and leaf_code[-1] == 'ret':
                            d_leaf = copy_cover(leaf_code[:-1], 'rcx', 'rdx', size, True)
                            d_wrap = copy_cover(tail[:-3], 'rax', 'rbx', size, False)
                            # The wrapper must read exactly the temp buffer the leaf filled.
                            if d_leaf is not None and d_wrap == 0:
                                offset, form, target = d_leaf, 'wrapper', tgt
            if offset is None:
                rejects['unmatched_complete_code'] += 1; continue
            if not (parent_size <= offset and offset + size <= own_size):
                rejects['outside_declaring_class'] += 1; continue
            if any(offset < end and start < offset + size for start, end in spans):
                rejects['occupied'] += 1; continue
            resolver = generator.Resolver({ref: cpp} if ref else {})
            if resolver.ctype(p, {}) != cpp or resolver.todos:
                rejects['generator_type_unresolved'] += 1; continue
            declarations = re.findall(r'^\s*([^;{}\n]*\b' + re.escape(name) + r'\s*\([^;{}\n]*\)[^;{}\n]*);', header, re.M)
            sigs = re.findall(r'^\s*([A-Za-z_]\w*)\s+' + re.escape(name) + r'\(\)( const)?\s*;', header, re.M)
            # Missing generated declarations are expected before opt-in.
            if declarations and (len(declarations) != 1 or len(sigs) != 1 or sigs[0][0] != cpp or bool(sigs[0][1]) != bool(flags & 0x40000000)):
                rejects['header_signature_mismatch'] += 1; continue
            if (cls, name) in definitions:
                rejects['maintained_definition'] += 1; continue
            prop = f'NativeRecovered{cpp}_0x{offset:x}'
            taken = {x.get('name') for x in block['properties']} | set(funcs) | {e.get('function') for e in manifest if e.get('class') == cls and not (args.revalidate_own and e.get('recovery_source') == 'bulk_native_copy_runtime')}
            if prop in taken:
                rejects['field_name_collision'] += 1; continue
            stem = f'{addr:x}_{cls}_{name}'
            inc = HERE / (stem + '.inc')
            body = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Mechanically matched complete {size}-byte {cpp} field copy; see bulk_native_copy_runtime/README.md.\nreturn {prop};\n'
            if inc.exists() and inc.read_text() != body:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists():
                inc.write_text(body)
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{cpp} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} {cpp} getter at {hex(addr)}, native same-class field {prop} offset {hex(offset)} ({size} bytes); source recovery only.'}
            key = (cls, offset)
            field = {'class': cls, 'function': prop, 'kind': 'member_field', 'signature': f'{cpp} {prop}', 'signature_status': 'proposed', 'access': 'public', 'offset': hex(offset), 'inc_path': None, 'confidence': 'high', 'address': None, 'status': 'ported', 'recovery_source': 'bulk_native_copy_runtime', 'notes': 'Original copy identifies storage; initialization and writers remain unported. Source recovery only.'}
            if key in fields and fields[key] != field:
                raise SystemExit('conflicting native field: ' + str(key))
            fields[key] = field
            results.append({'parent_size': parent_size, 'own_size': own_size, 'parent_gpsc': parent_proof[2], 'own_gpsc': own_proof[2], 'entry': entry, 'field': prop, 'offset': offset, 'type_ref': ref, 'assembly': code, 'target': hex(target) if target else None, 'leaf': leaf_code})
    proposed = list(fields.values())
    widths = {'FName': 8, **{'F' + k: v for k, v in PODS.items()}}
    for i, a in enumerate(proposed):
        for b in proposed[:i]:
            if a['class'] != b['class']: continue
            ao, bo = int(a['offset'], 0), int(b['offset'], 0)
            aw, bw = widths[a['signature'].split()[0]], widths[b['signature'].split()[0]]
            if ao < bo + bw and bo < ao + aw:
                raise SystemExit('overlapping proposed fields: ' + str((a, b)))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps(list(fields.values()) + [r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'pod_struct_returns_considered': considered, 'candidates': len(results), 'fields': len(fields), 'by_struct': dict(collections.Counter(r['entry']['signature'].split()[0] for r in results)), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'pod_struct_returns_considered', 'candidates', 'by_struct', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
