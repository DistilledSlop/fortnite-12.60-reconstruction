"""off.py Class 0xOFF... : property names at offsets (parent chain)"""
import sys,importlib.util,io,contextlib
sys.argv,argv=['rd.py'],sys.argv
import os
src=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'rd.py')).read().replace('\nmain()','')
g={}; exec(compile(src,'rd','exec'),g)
o=g['offsets'](argv[1])
for a in argv[2:]:
    x=int(a,16); c=[k for k in o if k<=x]; k=max(c) if c else None
    print(a, o.get(x) or ('(in/after %s @%#x)'%(o[k],k) if k is not None else '?'))
