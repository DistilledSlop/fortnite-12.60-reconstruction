#!/usr/bin/env python3
"""Condensed, annotated view of a captured decompile.

usage: rd.py ADDR [Class] [--full] [--grep RE]
  Captures ADDR if needed (through the local-model package), then prints the C with
  - local declarations removed,
  - callee names filled in from server_scope/string_names.json, native_funcs.json and the stub table,
  - `param_1` member offsets annotated with property names of Class (whole parent chain).
"""
import sys,re,json,glob,os,subprocess,pickle
PKG='/opt/fortnite/local-model-package'; REPO='/opt/fortnite/FortniteClient-12.60-CL-13477524/porting'
STUB={'140680300':'Stub_Null','1406801c0':'Stub_Nop','1406801e0':'Stub_False','1406801d0':'Stub_True'}
def realstart(a):
    # .pdata chunk starts in the middle of a function are common; walk back to an aligned start after padding
    sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
    import strx,bisect
    r=int(a,16)-strx.base; i=bisect.bisect_right(strx.starts,r)-1
    for k in range(i,max(i-40,0),-1):
        st=strx.starts[k]
        if st%16==0: return hex(strx.base+st)
    return a
def capture(a):
    fs=sorted(glob.glob(f'{PKG}/captures/*/{a}.c'),key=os.path.getmtime)
    if fs: return fs[-1]
    os.makedirs(f'{PKG}/work/claude-plan',exist_ok=True)
    json.dump([a],open(f'{PKG}/work/claude-plan/req_rd.json','w'))
    subprocess.run(['python3','-I','scripts/evidence.py','capture','--input','work/claude-plan/req_rd.json'],cwd=PKG,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    fs=sorted(glob.glob(f'{PKG}/captures/*/{a}.c'),key=os.path.getmtime)
    return fs[-1] if fs else None
def names():
    c=f'{PKG}/work/claude-plan/names.pkl'
    if os.path.exists(c): return pickle.load(open(c,'rb'))
    n={}
    for a,v in json.load(open(f'{REPO}/server_scope/string_names.json')).items():
        n[a[2:]]='~'+v[0][0]+'::'+v[0][1]
    nf=json.load(open(f'{REPO}/generated/reflection/native_funcs.json'))
    for cl,fs in nf.items():
        if isinstance(fs,dict):
            for f,x in fs.items():
                if isinstance(x,dict) and x.get('exec_addr'): n.setdefault(x['exec_addr'][2:],'exec'+cl+'::'+f)
    pickle.dump(n,open(c,'wb')); return n
def offsets(cls):
    c=f'{PKG}/work/claude-plan/off_{cls}.pkl'
    if os.path.exists(c): return pickle.load(open(c,'rb'))
    root=f'{REPO}/generated/reflection/'
    nm=json.load(open(root+'class_name_map.json')); par=json.load(open(root+'class_parent_map.json'))
    classes=json.load(open(root+'fortnite_game_reflection.json'))['classes_in_groups_with_fort_names']
    bits=json.load(open(root+'bool_bits.json'))['by_class']
    by={}
    for k in classes: by.setdefault(k.get('name') or nm.get(k.get('block_va')),[]).append(k)
    it=by.get(cls,[None])[0]; out={}
    if it:
        for name in (par.get(it['block_va'],{}).get('chain') or [cls]):
            for e in by.get(name,[])[:1]:
                for m in e.get('properties',[]):
                    if not m.get('array_inner_of') and m.get('offset') is not None: out.setdefault(m['offset'],m.get('name'))
            for m in bits.get(name,[]): out.setdefault(m['offset'],out.get(m['offset']) or (m.get('name') or '?')+':bit')
    pickle.dump(out,open(c,'wb')); return out
def main():
    args=[a for a in sys.argv[1:] if not a.startswith('--')]
    full='--full' in sys.argv; gre=None
    if '--grep' in sys.argv: gre=re.compile(sys.argv[sys.argv.index('--grep')+1]); args=[a for a in args if a!=sys.argv[sys.argv.index('--grep')+1]]
    a=args[0]; cls=args[1] if len(args)>1 else None
    if '--exact' not in sys.argv:
        b=realstart(a)
        if b!=a.lower(): print('// start resolved',a,'->',b)
        a=b
    f=capture(a)
    if not f: print('capture failed'); return
    n=names(); off=offsets(cls) if cls else {}
    src=open(f).read()
    isptr=bool(re.search(r'\(longlong \*+param_1',src))
    def fn(m):
        x=m.group(1).lower(); return (n.get(x) or STUB.get(x) or 'FUN')+'_'+x if (x in n or x in STUB) else m.group(0)
    src=re.sub(r'FUN_([0-9a-fA-F]{9})',fn,src)
    if off:
        def lab(o): return '/*%s*/'%off[o] if o in off else '/*+%#x*/'%o
        if isptr:
            src=re.sub(r'\(longlong\)param_1 \+ (0x[0-9a-f]+|\d+)\b',lambda m:m.group(0)+lab(int(m.group(1),0)),src)
            src=re.sub(r'(?<!\))param_1 \+ (0x[0-9a-f]+|\d+)\b',lambda m:m.group(0)+lab(int(m.group(1),0)*8),src)
            src=re.sub(r'param_1\[(0x[0-9a-f]+|\d+)\]',lambda m:m.group(0)+lab(int(m.group(1),0)*8),src)
        else:
            src=re.sub(r'param_1 \+ (0x[0-9a-f]+|\d+)\b',lambda m:m.group(0)+lab(int(m.group(1),0)),src)
    brief='--brief' in sys.argv
    out=[]
    for ln in src.split('\n'):
        s=ln.strip()
        if not s: continue
        if not full and re.match(r'^(undefined\d*|longlong|ulonglong|int|uint|char|byte|bool|float|double|short|ushort|code|FName|UObject|wchar_t|undefined)\b[ \*\w\[\]]*;$',s): continue
        if gre and not gre.search(ln): continue
        if brief and not re.search(r'L"|/\*|~|exec[AU]|Stub_|\+ 0x[0-9a-f]+\)\)\(|^\S|case |LAB_\w+:|[A-Z][A-Za-z]+_[A-Za-z]+\(',ln): continue
        out.append(ln[:200])
    print('\n'.join(out))
main()
