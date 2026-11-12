"""lea/rip-relative refs to addresses: learefs.py 0xADDR..."""
import sys,struct,bisect,re
sys.path.insert(0,__import__('os').path.dirname(__import__('os').path.abspath(__file__)))
from strx import m,base,secs
n,va,vs,ro,rs,ch=[s for s in secs if s[0]=='.pdata'][0]
starts=sorted(struct.unpack_from('<I',m,va+i)[0] for i in range(0,vs-11,12))
al=[s for s in starts if s%16==0]
def fn(rva): return base+al[bisect.bisect_right(al,rva)-1]
T={int(a,16)-base for a in sys.argv[1:]}
for lo,hi in ((0x1000,0x58fc000),(0x8d2c000,len(m)-8)):
    for mm in re.finditer(rb'[\x48\x4c]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]',m[lo:hi]):
        i=mm.start()+lo
        t=i+7+struct.unpack_from('<i',m,i+3)[0]
        if t in T: print('lea ->',hex(t+base),'at',hex(i+base),'fn',hex(fn(i)))
