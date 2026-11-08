import struct,sys,json
P=sys.argv[2] if len(sys.argv)>2 else '/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe'
d=open(P,'rb').read()
pe=struct.unpack_from('<I',d,0x3c)[0]
nsec=struct.unpack_from('<H',d,pe+6)[0]; osz=struct.unpack_from('<H',d,pe+20)[0]
opt=pe+24; base=struct.unpack_from('<Q',d,opt+24)[0]
impdir=struct.unpack_from('<II',d,opt+120)[0]
secs=[]
so=opt+osz
for i in range(nsec):
    n,vs,va,rs,ro=struct.unpack_from('<8sIIII',d,so+i*40); secs.append((va,max(vs,rs),ro))
def off(rva):
    for va,sz,ro in secs:
        if va<=rva<va+sz: return rva-va+ro
def cs(rva):
    o=off(rva); e=d.index(b'\0',o); return d[o:e].decode()
m={}
p=impdir
while True:
    ot,_,_,nm,ft=struct.unpack_from('<IIIII',d,off(p))
    if not nm: break
    dll=cs(nm); t=off(ot or ft); i=0
    while True:
        v=struct.unpack_from('<Q',d,t+i*8)[0]
        if not v: break
        name=cs((v&0x7fffffff)+2) if not v>>63 else 'ord%d'%(v&0xffff)
        m[hex(base+ft+i*8)]=dll+'!'+name; i+=1
    p+=20
json.dump(m,open(sys.argv[1],'w'))
print(len(m))
