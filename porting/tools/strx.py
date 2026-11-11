#!/usr/bin/env python3
# usage: strx.py "substring" ...  -> string VA, xref VA, containing function start (from .pdata)
import sys,struct,mmap,bisect,re
P='/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe'
f=open(P,'rb'); m=mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ)
pe=struct.unpack_from('<I',m,0x3c)[0]; nsec=struct.unpack_from('<H',m,pe+6)[0]; opt=struct.unpack_from('<H',m,pe+20)[0]
base=struct.unpack_from('<Q',m,pe+24+24)[0]
secs=[]
o=pe+24+opt
for i in range(nsec):
    name=m[o:o+8].rstrip(b'\0').decode(); vs,va,rs,ro=struct.unpack_from('<IIII',m,o+8); ch=struct.unpack_from('<I',m,o+36)[0]
    secs.append((name,va,vs,ro,rs,ch)); o+=40
def rva2off(r): return r  # dump: file offsets equal RVAs
# pdata
dd=pe+24+ (112 if struct.unpack_from('<H',m,pe+24)[0]==0x20b else 96)
exc_rva,exc_sz=struct.unpack_from('<II',m,dd+3*8)
starts=[struct.unpack_from('<I',m,exc_rva+i)[0] for i in range(0,exc_sz,12)]
starts.sort()
def fn(rva):
    i=bisect.bisect_right(starts,rva)-1
    return starts[i] if i>=0 else 0
code=[(va,vs) for n,va,vs,ro,rs,ch in secs if ch&0x20]
if __name__=='__main__':
    if sys.argv[1]=='--secs':
        for s in secs: print(s[0],hex(base+s[1]),hex(s[2]),hex(s[5]))
        sys.exit()
    targets={}
    for s in sys.argv[1:]:
        for enc,w in ((s.encode('utf-16le'),2),(s.encode(),1)):
            for mt in re.finditer(re.escape(enc),m):
                a=mt.start()
                # back up to string start
                st=a
                while st>=w and m[st-w:st]!=b'\0'*w and a-st<400: st-=w
                targets[st]=s
    print('strings',len(targets))
    tl=sorted(targets)
    tset=set(tl)
    for va,vs in code[:1]:
        data=m[va:va+vs]
        for mt in re.finditer(rb'[\x48\x4c]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]',data):
            p=mt.start(); disp=struct.unpack_from('<i',data,p+3)[0]; tgt=va+p+7+disp
            if tgt in tset:
                print('str',hex(base+tgt),repr(targets[tgt])[:40],'xref',hex(base+va+p),'fn',hex(base+fn(va+p)))
