#!/usr/bin/env python3
"""Recover two-boolean native byte leaf stores into isolated proposals."""
import argparse
import collections
import hashlib
import json
import re
import sys
import struct
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
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-two-native-byte'))
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
     fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_two_native_byte_runtime')]
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
    # Finite slice: both complete leaves previously observed, no search by name.
    cases = [('AFortGameStateZone','SetBuildingAllowed',0x14292d0d0,0x141eae8c0,'mov'), ('UFortServerBotManagerAthena','DirtyBotFactions',0x142ad6c60,0x141a3f7b0,'or')]
    template = 'mov qword ptr [rsp + 8], rbx; push rbp; push rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rbp, rcx; mov rbx, rdx; mov dword ptr [rsp + 0x48], edi; mov rcx, rdx; cmp qword ptr [rdx + 0x20], rdi; je ALT1; mov rdx, qword ptr [rdx + 0x18]; lea r8, [rsp + 0x48]; call 0x142ed9cf0; jmp JOIN1; mov r8, qword ptr [rdx + 0x80]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rdx + 0x80], rax; lea rdx, [rsp + 0x48]; call 0x142ed9d20; cmp dword ptr [rsp + 0x48], edi; mov rcx, rbx; mov dword ptr [rsp + 0x58], edi; setne sil; cmp qword ptr [rbx + 0x20], rdi; je ALT2; mov rdx, qword ptr [rbx + 0x18]; lea r8, [rsp + 0x58]; call 0x142ed9cf0; jmp JOIN2; mov r8, qword ptr [rbx + 0x80]; lea rdx, [rsp + 0x58]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rbx + 0x80], rax; call 0x142ed9d20; mov rax, qword ptr [rbx + 0x20]; movzx edx, sil; test rax, rax; mov rcx, rbp; setne dil; add rdi, rax; cmp dword ptr [rsp + 0x58], 0; mov qword ptr [rbx + 0x20], rdi; setne r8b; call LEAF; mov rbx, qword ptr [rsp + 0x40]; add rsp, 0x20; pop rdi; pop rsi; pop rbp; ret'.split('; ')
    def instructions(addr):
        if not BASE <= addr < BASE+len(image)-512: return []
        result=[]
        for ins in md.disasm(image[addr-BASE:addr-BASE+512],addr):
            result.append(ins)
            if ins.mnemonic=='ret': return result
        return []
    fields=[]; results=[]; rejects={}
    for cls,name,addr,target,operation in cases:
        def reject(reason): rejects[cls+'::'+name]=reason
        fn=native.get(cls,{}).get(name,{})
        pairs,notes=generator.split_properties(fn.get('params',[])); raw=[p for p,_ in pairs]
        # Original FBoolPropertyParams setters establish the missing parameter byte offsets.
        candidates=[x for x in load(REFL/'type_block_candidates.json')['functions'] if x.get('name')==name and x.get('block_va')=={'SetBuildingAllowed':'0x147a6d4b0','DirtyBotFactions':'0x147bad4f0'}[name]]
        order=[]
        if len(candidates)==1 and candidates[0].get('nprop')==2:
            arr=candidates[0]['prop_arr']
            for k,p in enumerate(raw):
                try:
                    prop_va=struct.unpack_from('<Q',image,arr-BASE+8*k)[0]
                    setter=struct.unpack_from('<Q',image,prop_va-BASE+48)[0]
                    code=decode(setter)
                    off={'mov byte ptr [rcx], 1; ret':0,'mov byte ptr [rcx + 1], 1; ret':1}.get(code)
                    if off is not None: order.append((off,p))
                except (struct.error,TypeError): break
        ps=[p for _,p in sorted(order,key=lambda x:x[0])]
        if notes or len(ps)!=2 or any(ch for _,ch in pairs) or any(p.get('kind')!='Bool' or p.get('array_dim')!=1 or p.get('type_refs')  or int(p.get('property_flags','0'),0)&(0x400|0x100|0x08000000) or not re.fullmatch(r'[A-Za-z_]\w*',p.get('name','')) for p in ps) or [x[0] for x in sorted(order,key=lambda x:x[0])]!=[0,1]:
            reject('parameters'); continue
        try: flags=int(fn['flags'],0); actual_addr=int(fn['exec_addr'],0)
        except (KeyError,TypeError,ValueError): reject('flags/address'); continue
        if actual_addr!=addr or flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS or (cls,name) in ported or (cls,name) in maintained or cls not in selected or len(byname[cls])!=1 or len(headers[cls])!=1:
            reject('flags/class/ported'); continue
        va=byname[cls][0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
        if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000:
            reject('metadata'); continue
        ins=instructions(addr)
        if len(ins)!=len(template): reject('wrapper length'); continue
        # The four target indices identify alternate paths and instruction immediately after each StepExplicitProperty.
        dest={'ALT1':16,'JOIN1':21,'ALT2':31,'JOIN2':36}
        for i,w in zip(ins,template):
            actual=(i.mnemonic+' '+i.op_str).rstrip()
            for key,idx in dest.items(): w=w.replace(key,hex(ins[idx].address))
            if w=='call LEAF': w='call '+hex(target)
            if actual!=w: reject('wrapper mismatch: '+actual+' != '+w); break
        else:
            leaf=decode(target); q=re.fullmatch(operation+r' byte ptr \[rcx \+ (0x[0-9a-f]+)\], dl; '+operation+r' byte ptr \[rcx \+ (0x[0-9a-f]+)\], r8b; ret',leaf)
            if not q: reject('leaf'); continue
            offsets=[int(x,0) for x in q.groups()]
            if offsets[0]==offsets[1]: reject('same storage byte'); continue
            own_proof,parent_proof=size_of(va),size_of(parent.get('parent_va'))
            if not own_proof or not parent_proof: reject('size proof: class='+str(own_proof)+' parent='+str(parent_proof)); continue
            own_size,parent_size=own_proof[0],parent_proof[0]
            header=headers[cls][0].read_text(); declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',header)
            if declaration!=[parent.get('parent_name')] or names.get(parent.get('parent_va'))!=parent.get('parent_name'): reject('parent mismatch'); continue
            if any(not parent_size<=off<own_size for off in offsets): reject('outside class'); continue
            spans=occupancy(cls,block,own_size)
            if spans is None: reject('unknown occupancy'); continue
            local_fields=[]; failed=None
            for off in offsets:
                overlaps=[x for x in spans if off<x[1] and x[0]<off+1]; prop=f'NativeRecoveredByte_0x{off:x}'; canonical=False
                if overlaps:
                    if len(overlaps)!=1: failed='occupied at '+hex(off)+': '+repr([(hex(a),hex(b),e.get('name') or e.get('function'),e.get('kind')) for a,b,e in overlaps]); break
                    start,end,existing=overlaps[0]; existing_name=existing.get('function')
                    if start!=off or end!=off+1 or existing.get('kind')!='member_field' or existing.get('signature')!=f'uint8 {existing_name}' or not re.fullmatch(r'[A-Za-z_]\w*',existing_name or ''): failed='occupied at '+hex(off)+': '+repr([(hex(a),hex(b),e.get('name') or e.get('function'),e.get('kind')) for a,b,e in overlaps]); break
                    prop,canonical=existing_name,True
                taken={x.get('name') for x in block['properties']} | set(native[cls]) | {e.get('function') for e in manifest if e.get('class')==cls}
                if not canonical and prop in taken: failed='name collision'; break
                local_fields.append((off,prop,canonical))
            if failed: reject(failed); continue
            resolver=generator.Resolver({}); ret,arguments=generator.decode_params(fn['params'],resolver)
            expected=[f'bool {p["name"]}' for p in ps]
            if resolver.todos or ret!='void' or arguments!=expected: reject('signature'); continue
            sig=f'void {name}('+', '.join(arguments)+')'; cpp=headers[cls][0].with_suffix('.cpp')
            if (cpp.exists() and re.search(r'\b'+re.escape(cls)+r'::'+re.escape(name)+r'\s*\(',cpp.read_text())) or re.search(r'\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*(?:const\s*)?\{',header): reject('existing body'); continue
            declarations=re.findall(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\([^;{}\n]*\)[^;{}\n]*;',header,re.M)
            if declarations and (len(declarations)!=1 or not re.fullmatch(r'\s*'+re.escape(sig)+r'\s*;',declarations[0])): reject('declaration'); continue
            inc=HERE/f'{addr:x}_{cls}_{name}.inc'; op='=' if operation=='mov' else '|='
            body=f'// Original exec {hex(addr)}, complete native-byte leaf {hex(target)}.\n// CONFIDENCE: HIGH\n'+''.join(f'this->{prop} {op} static_cast<uint8>({p["name"]});\n' for (_,prop,_),p in zip(local_fields,ps))
            if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs')
            inc.write_text(body)
            entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete normalized-bool wrapper and {operation} byte leaf at {hex(target)}; initialization and other uses unported.'}
            for off,prop,canonical in local_fields:
                if not canonical: fields.append({'class':cls,'function':prop,'kind':'member_field','signature':f'uint8 {prop}','signature_status':'proposed','access':'public','offset':hex(off),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'bulk_two_native_byte_runtime','notes':'Original byte mutation identifies native storage; initialization and other uses unported.'})
            results.append({'entry':entry,'fields':local_fields,'parameter_setter_offsets':[(p['name'],off) for off,p in order],'assembly':[(i.mnemonic+' '+i.op_str).rstrip() for i in ins],'leaf':leaf,'parent_size':parent_size,'own_size':own_size,'parent_gpsc':parent_proof[2],'own_gpsc':own_proof[2]})
    args.output_dir.mkdir(parents=True,exist_ok=True)
    (args.output_dir/'manifest-entries.json').write_text(json.dumps(fields+[x['entry'] for x in results],indent=2)+'\n')
    (args.output_dir/'report.json').write_text(json.dumps({'image_sha256':digest,'cases':len(cases),'candidates':len(results),'fields':len(fields),'rejections':rejects,'results':results},indent=2)+'\n')
    print(json.dumps({'candidates':len(results),'fields':len(fields),'rejections':rejects},indent=2))

if __name__=='__main__': main()
