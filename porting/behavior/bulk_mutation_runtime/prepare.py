#!/usr/bin/env python3
"""Discover complete parameterless constant field mutations; never edits canonical sources."""
import collections, hashlib, importlib.util, json, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
REFL = ROOT / 'porting/generated/reflection'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
BASE = 0x140000000
OUT = Path('/tmp/gameplay-bulk-mutation')
ACCESS = {0x20000:'public', 0x40000:'private', 0x80000:'protected'}
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
load = lambda p: json.loads(p.read_text())
PREFIXES = [
 'mov rax, qword ptr [rdx + 0x20]; xor r8d, r8d; test rax, rax; setne r8b; add r8, rax; mov qword ptr [rdx + 0x20], r8; ',
 'mov r8, qword ptr [rdx + 0x20]; xor eax, eax; test r8, r8; setne al; add rax, r8; mov qword ptr [rdx + 0x20], rax; ',
]
PAT = re.compile(r'(mov|and|or) (byte|dword) ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret')

def main():
 image = IMAGE.read_bytes(); digest = hashlib.sha256(image).hexdigest()
 if digest != SHA: raise SystemExit('original image digest mismatch')
 native = load(REFL/'native_funcs.json'); names = load(REFL/'class_name_map.json'); parents = load(REFL/'class_parent_map.json')
 blocks = {x['block_va'].lower():x for x in load(REFL/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
 byname = collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 selected = {x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 ported = {(x.get('class'), x.get('function')) for x in load(ROOT/'porting/behavior/MANIFEST.json') if x.get('status') == 'ported'}
 headers = collections.defaultdict(list)
 for module in ('FortniteGame','FortniteUI'):
  for p in (ROOT/'porting/native/Source'/module/'Generated').glob('*.h'): headers[p.stem].append(p)
 spec = importlib.util.spec_from_file_location('gen_native_classes', ROOT/'porting/gen_native_classes.py'); gen = importlib.util.module_from_spec(spec); sys.modules[spec.name] = gen; spec.loader.exec_module(gen)
 md = Cs(CS_ARCH_X86, CS_MODE_64); rejects=collections.Counter(); shapes=collections.Counter(); matches=[]; proposals=[]; total=0
 for cls, funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict) or fn.get('params') != []: rejects['not_exact_parameterless_void']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): rejects['missing_flags_or_address']+=1; continue
   pos=addr-BASE
   if pos<0 or pos+128>len(image): rejects['address_outside_image']+=1; continue
   ins=[]
   for i in md.disasm(image[pos:pos+128], addr):
    ins.append((i.mnemonic+' '+i.op_str).rstrip())
    if i.mnemonic=='ret' or i.mnemonic.startswith('j'): break
   asm='; '.join(ins); q=None
   for prefix in PREFIXES:
    if asm.startswith(prefix): q=PAT.fullmatch(asm[len(prefix):]); break
   if not q: rejects['other_complete_body_or_thunk']+=1; continue
   op,width,offset,imm=q.groups(); offset=int(offset,0); value=int(imm,0); shapes[op+'_'+width]+=1
   row={'class':cls,'function':name,'address':hex(addr),'assembly':asm,'offset':offset,'operation':op,'width':width,'immediate':value}
   reasons=[]
   if (cls,name) in ported: reasons.append('already_ported')
   if flags & (~0xffffffcf | NET | 0x08000000 | 0x20000000 | 0x2000 | 0x40000000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS: reasons.append('unsupported_flags')
   if not re.fullmatch(r'[A-Za-z_]\w*',name) or not re.fullmatch(r'[A-Za-z_]\w*',cls): reasons.append('invalid_identifier')
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reasons.append('class_not_unique_or_selected')
   fields=[]
   if len(vs)==1:
    va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
    if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reasons.append('class_or_package')
    for ancestor in parent.get('chain',[]):
     avs=byname.get(ancestor,[])
     if len(avs)!=1: continue
     ab=blocks.get(avs[0].lower())
     if ab:
      fields += [(ancestor,x) for x,children in gen.split_properties(ab['properties'])[0] if not children and x.get('offset')==offset and x.get('array_dim')==1]
   row['reflected_fields']=[{'owner':o,'name':f.get('name'),'kind':f.get('kind')} for o,f in fields]
   # Only whole scalar writes are emitted. Packed booleans and overlapping fields need independent mask/layout evidence.
   wanted={'byte':{'Byte':'uint8'},'dword':{'Int':'int32','UInt32':'uint32','Float':'float'}}[width]
   if len(fields)!=1 or fields[0][1].get('kind') not in wanted or fields[0][1].get('type_refs'): reasons.append('missing_or_ambiguous_whole_scalar_field')
   if op != 'mov': reasons.append('bitwise_field_layout_unresolved')
   if len(headers[cls]) != 1: reasons.append('missing_or_ambiguous_class_header')
   elif re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(', headers[cls][0].read_text(), re.M) and not re.search(r'^\s*void\s+'+re.escape(name)+r'\(\)\s*;', headers[cls][0].read_text(), re.M): reasons.append('function_header_mismatch')
   if len(fields)==1 and fields[0][1].get('kind')=='Float':
    fval=struct.unpack('<f',struct.pack('<I',value & 0xffffffff))[0]
    if not (-16777216 <= fval <= 16777216 and fval == int(fval) and not (value == 0x80000000)): reasons.append('float_literal_not_exact_integer')
   if len(fields)==1 and fields[0][1].get('kind') in wanted:
    owner,field=fields[0]; typ=wanted[field['kind']]; prop=field.get('name','')
    resolver=gen.Resolver({}); resolved=resolver.ctype(field,{})
    if resolver.todos or resolved!=typ or not re.fullmatch(r'[A-Za-z_]\w*',prop): reasons.append('field_type_unresolved')
    if len(headers[owner])!=1 or not re.search(r'^\s*'+typ+r'\s+'+re.escape(prop)+r'\s*;',headers[owner][0].read_text(),re.M): reasons.append('field_header_mismatch')
   row['reasons']=reasons; matches.append(row)
   if reasons:
    rejects.update(reasons); continue
   bits=8 if width=='byte' else 32; value &= (1<<bits)-1
   if typ=='int32' and value>=2**31: value-=2**32
   literal=(str(int(fval))+'.0f') if typ=='float' else ('(-2147483647 - 1)' if value == -2147483648 else str(value)+('u' if typ=='uint32' else ''))
   inc=HERE/(f'{addr:x}_{cls}_{name}.inc'); body=f'// Original address: {hex(addr)}. Complete constant field write.\n// CONFIDENCE: HIGH\n{prop} = {literal};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   proposals.append({'class':cls,'function':name,'kind':'exec','signature':f'void {name}()','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags & 0xe0000],'notes':f'Complete original {width} constant write to reflected {owner}.{prop} at offset {hex(offset)}; source recovery only.'})
 OUT.mkdir(parents=True,exist_ok=True)
 (OUT/'manifest-entries.json').write_text(json.dumps(proposals,indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(proposals),'complete_mutation_shapes':dict(shapes),'rejections':dict(rejects),'matches':matches}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','complete_mutation_shapes','rejections')},indent=2))
if __name__=='__main__': main()
