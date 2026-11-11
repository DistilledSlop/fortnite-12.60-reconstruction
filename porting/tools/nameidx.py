import sys,re,struct,json,collections
sys.path.insert(0,sys.argv[0].rsplit('/',1)[0])
from strx import m,base,secs,fn,code
n,va,vs,ro,rs,ch=[s for s in secs if s[0]=='.rdata'][0]
rx=re.compile(rb'(?:[\x20-\x7e]\x00){6,}')
nm=re.compile(r'\b([AUF]?Fort[A-Za-z0-9_]+|ABuilding[A-Za-z0-9_]*|UBuilding[A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]+)')
strs={}
for mt in rx.finditer(m,va,va+vs):
    t=mt.group().decode('utf-16le')
    g=nm.search(t)
    if g: strs[mt.start()]=(g.group(1),g.group(2),t[:120])
print('named strings',len(strs),file=sys.stderr)
out=collections.defaultdict(lambda: collections.Counter())
cva,cvs=code[0]; data=m[cva:cva+cvs]
for mt in re.finditer(rb'[\x48\x4c]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]',data):
    p=mt.start(); tgt=cva+p+7+struct.unpack_from('<i',data,p+3)[0]
    if tgt in strs:
        c,f,t=strs[tgt]; out[base+fn(cva+p)][(c,f)]+=1
res={hex(k):[[c,f,nn] for (c,f),nn in v.most_common(4)] for k,v in out.items()}
json.dump(res,open(sys.argv[1],'w'),indent=0)
cls=collections.Counter()
for k,v in out.items(): cls[v.most_common(1)[0][0][0]]+=1
print('functions with a class::function string',len(out),file=sys.stderr)
for c,nn in cls.most_common(40): print(c,nn)
