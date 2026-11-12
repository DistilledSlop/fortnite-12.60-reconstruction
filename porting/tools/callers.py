#!/usr/bin/env python3
"""Find direct callers (call/jmp rel32) of an address, and vtable slots holding it.
Usage: python3 -I porting/tools/callers.py 0xADDR [0xADDR...]
Prints the calling instruction address and the 16-aligned .pdata start of the function containing it."""
import sys,os,struct,bisect
sys.path.insert(0,os.path.dirname(os.path.abspath(sys.argv[0])))
from strx import m,base,secs
n,va,vs,ro,rs,ch=[s for s in secs if s[0]=='.pdata'][0]
starts=sorted(struct.unpack_from('<I',m,va+i)[0] for i in range(0,vs-11,12))
al=[s for s in starts if s%16==0]
def fn(rva):
    i=bisect.bisect_right(al,rva)-1
    return base+al[i]
T0,T1=0x1000,0x58fc000
import re
for a in sys.argv[1:]:
    t=int(a,16); res=[]
    mv=memoryview(m)
    for op in (0xe8,0xe9):
        i=T0
        bop=bytes([op])
        while True:
            i=m.find(bop,i,T1)
            if i<0: break
            if i+5+struct.unpack_from('<i',m,i+1)[0]+base==t: res.append((i+base,'call' if op==0xe8 else 'jmp'))
            i+=1
    print('==',a,len(res),'direct refs')
    for x,k in sorted(res)[:40]: print('  %s at %#x in fn %#x'%(k,x,fn(x-base)))
    p=struct.pack('<Q',t); i=m.find(p,0x58fc000)
    c=0
    while i>=0 and c<8:
        print('  data ref at %#x'%(i+base)); c+=1; i=m.find(p,i+8)
