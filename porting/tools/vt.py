import sys,struct
sys.path.insert(0,sys.argv[0].rsplit('/',1)[0])
from strx import m,base,secs
tgt=int(sys.argv[1],16); pat=struct.pack('<Q',tgt)
n,va,vs,ro,rs,ch=[s for s in secs if s[0]=='.rdata'][0]
i=m.find(pat,va,va+vs)
while i!=-1:
    # walk back to vtable start (while previous qword points into .text)
    s=i
    while 0x140001000<=struct.unpack_from('<Q',m,s-8)[0]<0x1458fc000: s-=8
    print('hit',hex(base+i),'vtable start',hex(base+s),'slot',hex(i-s))
    j=s;k=0
    while 0x140001000<=struct.unpack_from('<Q',m,j)[0]<0x1458fc000 and k<40:
        print('  +%#x %#x'%(j-s,struct.unpack_from('<Q',m,j)[0])); j+=8;k+=1
    i=m.find(pat,i+8,va+vs)
