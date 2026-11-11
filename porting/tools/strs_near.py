import sys,re,mmap
sys.path.insert(0,sys.argv[0].rsplit('/',1)[0])
from strx import m,base
a=int(sys.argv[1],16)-base; b=int(sys.argv[2],16)-base
for mt in re.finditer(rb'(?:[\x20-\x7e]\x00){4,}',m[a:b]):
    print(hex(base+a+mt.start()), mt.group().decode('utf-16le')[:150])
