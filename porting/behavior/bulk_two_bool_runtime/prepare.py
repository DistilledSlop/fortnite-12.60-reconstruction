#!/usr/bin/env python3
"""Recover two complete reflected-Boolean leaf stores into isolated proposals."""
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
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-two-bool'))
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
    bits = load(REFL / 'bool_bits.json')['by_class']
    manifest = load(ROOT / 'porting/behavior/MANIFEST.json')
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
                    name_va=struct.unpack_from('<Q',image,prop_va-BASE)[0]
                    npos=name_va-BASE; nend=image.find(b'\0',npos,npos+256)
                    if npos<0 or nend<0 or image[npos:nend].decode('ascii')!=p['name']: break
                    setter=struct.unpack_from('<Q',image,prop_va-BASE+48)[0]
                    code=decode(setter)
                    off={'mov byte ptr [rcx], 1; ret':0,'mov byte ptr [rcx + 1], 1; ret':1}.get(code)
                    if off is not None: order.append((off,p))
                except (struct.error,TypeError,UnicodeDecodeError): break
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
            header=headers[cls][0].read_text()
            if cls not in parent.get('chain',[]): reject('invalid ancestry'); continue
            local_fields=[]; failed=None
            for off in offsets:
                candidates=[(owner,b) for owner in parent['chain'] for b in bits.get(owner,[]) if b.get('offset')==off]
                if len(candidates)!=1: failed='ambiguous bool'; break
                owner,b=candidates[0]; prop=b.get('name','')
                if b.get('form')!='mov8' or b.get('mask')!=1 or len(byname.get(owner,[]))!=1 or len(headers.get(owner,[]))!=1 or not re.fullmatch(r'[A-Za-z_]\w*',prop): failed='bool form'; break
                all_members=[]; field_members=[]
                for anc in parent['chain']:
                    av=byname.get(anc,[]); ab=blocks.get(av[0].lower()) if len(av)==1 else None
                    if not ab: failed='missing ancestor'; break
                    members,problems=generator.split_properties(ab['properties'])
                    if problems: failed='member metadata'; break
                    all_members.extend(x for x,ch in members if not ch and x.get('offset')==off)
                    if anc==owner: field_members.extend(x for x,ch in members if not ch and x.get('name')==prop and x.get('kind')=='Bool' and x.get('array_dim')==1)
                if failed or all_members or len(field_members)!=1: failed='overlap or field'; break
                # A canonical non-reflected member at the same byte is an overlap.
                for e in manifest:
                    if e.get('class') in parent['chain'] and e.get('kind')=='member_field' and e.get('status')=='ported':
                        try: loc=int(str(e['offset']),0)
                        except (ValueError,TypeError,KeyError): failed='unknown canonical offset'; break
                        if loc==off: failed='canonical overlap'; break
                if failed: break
                oh=headers[owner][0].read_text(); decl=re.search(r'^\s*bool\s+'+re.escape(prop)+r'\s*;',oh,re.M)
                if not decl: failed='field declaration'; break
                labels=list(re.finditer(r'^\s*(public|protected|private)\s*:',oh[:decl.start()],re.M))
                if owner!=cls and (not labels or labels[-1].group(1)=='private'): failed='field access'; break
                local_fields.append((off,prop,owner))
            if failed: reject(failed); continue
            resolver=generator.Resolver({}); ret,arguments=generator.decode_params(fn['params'],resolver)
            expected=[f'bool {p["name"]}' for p in ps]
            if resolver.todos or ret!='void' or arguments!=expected: reject('signature'); continue
            sig=f'void {name}('+', '.join(arguments)+')'; cpp=headers[cls][0].with_suffix('.cpp')
            if (cpp.exists() and re.search(r'\b'+re.escape(cls)+r'::'+re.escape(name)+r'\s*\(',cpp.read_text())) or re.search(r'\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*(?:const\s*)?\{',header): reject('existing body'); continue
            declarations=re.findall(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\([^;{}\n]*\)[^;{}\n]*;',header,re.M)
            if declarations and (len(declarations)!=1 or not re.fullmatch(r'\s*'+re.escape(sig)+r'\s*;',declarations[0])): reject('declaration'); continue
            inc=HERE/f'{addr:x}_{cls}_{name}.inc'; op='=' if operation=='mov' else '='
            body=f'// Original exec {hex(addr)}, complete reflected-Boolean leaf {hex(target)}.\n// CONFIDENCE: HIGH\n'+''.join(f'this->{prop} = '+(f'this->{prop} | ' if operation=='or' else '')+f'{p["name"]};\n' for (_,prop,_),p in zip(local_fields,ps))
            if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs')
            inc.write_text(body)
            entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete normalized-bool wrapper and reflected full-byte Boolean {operation} leaf at {hex(target)}; valid bool representations assumed.'}
            results.append({'entry':entry,'fields':local_fields,'parameter_setter_offsets':[(p['name'],off) for off,p in order],'assembly':[(i.mnemonic+' '+i.op_str).rstrip() for i in ins],'leaf':leaf})
    args.output_dir.mkdir(parents=True,exist_ok=True)
    (args.output_dir/'manifest-entries.json').write_text(json.dumps(fields+[x['entry'] for x in results],indent=2)+'\n')
    (args.output_dir/'report.json').write_text(json.dumps({'image_sha256':digest,'cases':len(cases),'candidates':len(results),'fields':len(fields),'rejections':rejects,'results':results},indent=2)+'\n')
    print(json.dumps({'candidates':len(results),'fields':len(fields),'rejections':rejects},indent=2))

if __name__=='__main__': main()
