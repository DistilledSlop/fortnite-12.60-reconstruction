#!/usr/bin/env python3
"""Engine pattern table: native address -> UE 4.25 equivalent. Merges porting/engine_patterns_manual.json with every one-instruction
`JMP qword ptr [IAT]` import thunk resolved through the PE import table. Writes porting/generated/small-pool/engine_patterns.json.
Run from repo root: python3 -I porting/build_engine_patterns.py"""
import glob, json, os, re, subprocess, sys
R = 'porting/generated'
PE = '/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe'
tmp = f'{R}/small-pool/iat.json'
os.makedirs(f'{R}/small-pool', exist_ok=True)
subprocess.run([sys.executable, '-I', 'porting/pe_imports.py', tmp, PE], check=True, stdout=subprocess.DEVNULL)
iat = json.load(open(tmp))
CPP = {'memcpy': 'FMemory::Memcpy(Dest, Src, Count) / memcpy', 'memmove': 'FMemory::Memmove(Dest, Src, Count) / memmove',
       'memset': 'FMemory::Memset(Dest, Value, Count) / memset', 'malloc': 'malloc(Size)', 'free': 'free(Ptr)', 'wcsstr': 'wcsstr(Str, Sub)'}
out = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if not m or m.group(1) in out:
            continue
        L = [l.split(None, 1)[1].strip() for l in open(d + f) if len(l.split(None, 1)) > 1]
        mm = len(L) == 1 and re.fullmatch(r'JMP qword ptr \[(0x[0-9a-f]+)\]', L[0])
        if mm and mm.group(1) in iat:
            name = iat[mm.group(1)]
            fn = name.split('!')[1]
            out[m.group(1)] = dict(kind='import', cpp=CPP.get(fn, fn), note=f'import thunk for {name}: call it directly with the Windows x64 ABI args (RCX,RDX,R8,R9)')
out.update(json.load(open('porting/engine_patterns_manual.json')))
json.dump(out, open(f'{R}/small-pool/engine_patterns.json', 'w'), indent=1)
print(len(out), 'patterns')
