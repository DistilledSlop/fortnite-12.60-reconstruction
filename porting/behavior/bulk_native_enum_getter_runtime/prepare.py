#!/usr/bin/env python3
"""Recover exact native byte-backed enum getters into isolated proposals."""
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
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'native_class_size_evidence_runtime'))
from scan import extract

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
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-native-enum-getter'))
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
    bits = load(REFL / 'bool_bits.json')['by_class']
    index = load(REFL / 'native_class_index.json')
    targets = collections.Counter(g.lower() for v in index.values() for g in v.get('gpsc_targets', []))
    size_cache = {}
    manifest = load(ROOT / 'porting/behavior/MANIFEST.json')
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
     fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_enum_getter_runtime')]
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
      spans.append((loc,end,p))
     return spans
    fields = {}
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
            if not parent_size <= offset < own_size:
                rejects['outside_declaring_class'] += 1; continue
            spans = occupancy(cls, block, own_size)
            if spans is None:
                rejects['unknown_occupancy'] += 1; continue
            overlaps = [x for x in spans if offset < x[1] and x[0] < offset + 1]
            prop = f'NativeRecoveredByte_0x{offset:x}'
            canonical = False
            if overlaps:
                if len(overlaps) != 1:
                    rejects['occupied'] += 1; continue
                start, end, existing = overlaps[0]
                existing_name = existing.get('function')
                if (start != offset or end != offset + 1 or existing.get('kind') != 'member_field'
                    or existing.get('signature') != f'uint8 {existing_name}' or not re.fullmatch(r'[A-Za-z_]\w*', existing_name or '')):
                    rejects['occupied'] += 1; continue
                prop, canonical = existing_name, True
            ref = p['type_refs']['enum']
            info = tables.by_fn.get(ref.lower()) if isinstance(ref, str) else None
            if not info or info.kind != 'enum' or not info.cpp_name or info.package not in ('/Script/FortniteGame', '/Script/FortniteUI'):
                rejects['unresolved_enum_type'] += 1; continue
            if info.package != parent['package'] and (parent['package'], info.package) != ('/Script/FortniteUI', '/Script/FortniteGame'):
                rejects['enum_module_dependency'] += 1; continue
            eh = enum_headers.get(info.header_name, [])
            if len(eh) != 1 or eh[0].parents[1].name != info.package.rsplit('/', 1)[-1] or not re.search(r'\benum\s+class\s+' + re.escape(info.cpp_name) + r'\s*:\s*uint8\b', eh[0].read_text()):
                rejects['enum_header_underlying_mismatch'] += 1; continue
            ret_resolver = generator.Resolver({ref: info.cpp_name})
            ret, arguments = generator.decode_params(ps, ret_resolver)
            if ret_resolver.todos or arguments or ret != info.cpp_name:
                rejects['return_generator_type_mismatch'] += 1; continue
            taken = {x.get('name') for x in block['properties']} | set(funcs) | {e.get('function') for e in manifest if e.get('class') == cls and not (args.revalidate_own and e.get('recovery_source') == 'bulk_native_enum_getter_runtime')}
            if not canonical and prop in taken:
                rejects['field_name_collision'] += 1; continue
            header = headers[cls][0].read_text()
            sigs = re.findall(r'^\s*' + re.escape(ret) + r'\s+' + re.escape(name) + r'\(\)( const)?\s*;', header, re.M)
            any_decl = re.search(r'^\s*[^\n;]*\b' + re.escape(name) + r'\s*\(', header, re.M)
            if any_decl and (len(sigs) != 1 or bool(sigs[0]) != bool(flags & 0x40000000)):
                rejects['header_signature_mismatch'] += 1; continue
            stem = f'{addr:x}_{cls}_{name}'
            inc = HERE / (stem + '.inc')
            body = f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Exact native enum byte getter; see bulk_native_enum_getter_runtime/README.md.\nreturn static_cast<{ret}>({prop});\n'
            if inc.exists() and inc.read_text() != body:
                raise SystemExit('existing body differs: ' + str(inc))
            if not inc.exists(): inc.write_text(body)
            entry = {'class': cls, 'function': name, 'kind': 'exec', 'signature': f'{ret} {name}()' + (' const' if flags & 0x40000000 else ''), 'signature_status': 'proposed', 'inc_path': str(inc.relative_to(ROOT)), 'confidence': 'high', 'address': hex(addr), 'status': 'ported', 'reflection_flags': fn['flags'], 'static': False, 'access': ACCESS[flags & 0xe0000], 'notes': f'Original {form} enum byte getter at {hex(addr)}, native same-class field {prop} offset {hex(offset)}; initialization and writers unported.'}
            if not canonical:
                fields[(cls, offset)] = {'class': cls, 'function': prop, 'kind': 'member_field', 'signature': f'uint8 {prop}', 'signature_status': 'proposed', 'access': 'public', 'offset': hex(offset), 'inc_path': None, 'confidence': 'high', 'address': None, 'status': 'ported', 'recovery_source': 'bulk_native_enum_getter_runtime', 'notes': 'Original byte read identifies storage; initialization and writers remain unported. Source recovery only.'}
            results.append({'entry': entry, 'field': prop, 'canonical_field': canonical, 'offset': offset, 'enum': info.cpp_name, 'enum_ref': ref, 'assembly': asm, 'target': hex(target) if target else None, 'leaf': leaf if target else None, 'parent_size': parent_size, 'own_size': own_size, 'parent_gpsc': parent_proof[2], 'own_gpsc': own_proof[2]})
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'manifest-entries.json').write_text(json.dumps(list(fields.values()) + [r['entry'] for r in results], indent=2) + '\n')
    report = {'image_sha256': digest, 'inventory_records': total, 'candidates': len(results), 'fields': len(fields), 'return_kinds': dict(sorted(kinds.items())), 'rejections': dict(sorted(rejects.items())), 'results': results}
    (args.output_dir / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('inventory_records', 'candidates', 'return_kinds', 'rejections')}, indent=2))


if __name__ == '__main__':
    main()
