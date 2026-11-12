#!/usr/bin/env python3
"""Sweep reflected RPCs: is the body behind the exec thunk real code or an empty stub?
Usage: python3 -I porting/tools/rpc_sweep.py [name-prefix=Server] > out.json   (run from repo root or porting/)
Reads generated/thunk-table/final.json. Kinds: stub_nop (ret), stub_false (xor eax/al; ret),
stub_true (mov al,1; ret), stub_null, jmp (tail jump, followed up to 4 times), real(size of first .pdata chunk)."""
import sys,os,json,struct,bisect
here=os.path.dirname(os.path.abspath(sys.argv[0])); sys.path.insert(0,here)
from strx import m,base,secs
P=os.path.join(here,'..')
pre=sys.argv[1] if len(sys.argv)>1 else 'Server'
n,va,vs,ro,rs,ch=[s for s in secs if s[0]=='.pdata'][0]
pd=sorted(struct.unpack_from('<II',m,va+i)[:2] for i in range(0,vs-11,12))
starts=[a for a,b in pd]
def size(rva):
    i=bisect.bisect_left(starts,rva)
    return pd[i][1]-pd[i][0] if i<len(pd) and pd[i][0]==rva else 0
STUB={0x140680300:'stub_null',0x1406801c0:'stub_nop',0x1406801e0:'stub_false',0x1406801d0:'stub_true'}
def kind(a,depth=0):
    if a in STUB: return STUB[a]
    o=a-base; b=m[o:o+8]
    if b[0]==0xc3 or b[:2]==b'\xc2\x00': return 'stub_nop'
    if b[:3] in(b'\x32\xc0\xc3',b'\x33\xc0\xc3'): return 'stub_false'
    if b[:3]==b'\xb0\x01\xc3': return 'stub_true'
    if b[0]==0xe9 and depth<4: return kind(a+5+struct.unpack_from('<i',b,1)[0],depth+1)
    s=size(o)
    return 'real(%d)'%s if s else 'real(?)'
rows=json.load(open(os.path.join(P,'generated/thunk-table/final.json')))
# subclass overrides: same vtable slot in every descendant class's vtable
vts=json.load(open(os.path.join(P,'generated/thunk-table/virtual_targets.json')))
slots={(o['cls'],o['fn']):[s['slot'] for s in o.get('sites',[]) if s.get('kind')=='primary' and s.get('slot') is not None] for o in vts}
nci=json.load(open(os.path.join(P,'generated/reflection/native_class_index.json')))
cpm=json.load(open(os.path.join(P,'generated/reflection/class_parent_map.json')))
cvt={o['cls']:o['vtable'] for o in vts if o.get('vtable') and o.get('vtable_status')=='ctor'}  # primary vtable written by the constructor
desc={}
for e in cpm.values():
    for anc in e.get('chain',[])[1:]: desc.setdefault(anc,[]).append(e['name'])
def overrides(cls,fn):
    res={}
    for sl in slots.get((cls,fn),[]):
        for d in desc.get(cls,[]):
            vt=cvt.get(d)
            if not vt: continue
            t=struct.unpack_from('<Q',m,int(vt,16)-base+sl)[0]
            if not (0x140001000<=t<0x1458fc000 or 0x148d2c000<=t<0x149e4d600): continue
            k=kind(t)
            if not k.startswith('stub'): res.setdefault(d,{})[hex(t)]=k
    return res
out=[]
for r in rows:
    if not r['fn'].startswith(pre): continue
    t=r.get('targets') or r.get('callees') or []
    ks=[kind(int(x,16)) for x in t]
    if r['group']=='A': v='inline'
    elif r['group']=='I': v='interface'
    elif not ks: v='unknown'
    elif all(k.startswith('stub') for k in ks): v='STUB'
    elif any(k.startswith('stub') for k in ks): v='mixed'
    else: v='real'
    row={'cls':r['cls'],'fn':r['fn'],'verdict':v,'targets':dict(zip(t,ks))}
    if v=='STUB':
        ov=overrides(r['cls'],r['fn'])
        if ov: row['verdict']='STUB_base_real_override'; row['overrides']=ov
    out.append(row)
json.dump(out,sys.stdout,indent=0)
