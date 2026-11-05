#!/usr/bin/env python3
"""Extract the resolver's canonical third-output names from a captured Ghidra asm.

Run after view_original_probe.py --functions 1422cb480 --out <capture>:
  python3 extract_first_stage.py <capture>/1422cb480.asm <runtime-image> > first-stage.json
The image is a memory-layout dump based at 0x140000000, not a PE file.
"""
import json, re, sys
from pathlib import Path
asm = Path(sys.argv[1]).read_text().splitlines()
image = Path(sys.argv[2]).open('rb')
def literal(address):
    image.seek(address - 0x140000000)
    data = image.read(512)
    end = next(i for i in range(0, len(data)-1, 2) if data[i:i+2] == b'\0\0')
    return data[:end].decode('utf-16le')
rows = []
for i, line in enumerate(asm):
    address = int(line.split()[0], 16)
    if not 0x1422cc3bd <= address < 0x1422d00a0 or 'CALL 0x142bcf780' not in line:
        continue
    prior = '\n'.join(asm[max(0,i-16):i])
    literals = re.findall(r'LEA RDX,\[(0x[0-9a-f]+)\]', prior)
    if not literals: raise ValueError(f'missing comparison literal at {address:x}')
    tail = '\n'.join(asm[i+1:i+25])
    branch = re.search(r'([0-9a-f]+) TEST SIL,SIL\n[0-9a-f]+ (JZ|JNZ) (0x[0-9a-f]+)', tail)
    if not branch: raise ValueError(f'missing condition at {address:x}')
    if branch[2] == 'JNZ':
        target = next(x for x in asm if x.startswith(branch[3][2:]+' '))
        assignment = re.search(r'LEA R(?:AX|SI),\[(0x[0-9a-f]+)\]', target)
        assignment_branch = branch[3]
    else:
        assignment = re.search(r'LEA R(?:AX|SI),\[(0x[0-9a-f]+)\]', tail[branch.end():])
        assignment_branch = '0x' + asm[i+1+tail[:branch.end()].count('\n')+1].split()[0]
    if not assignment: raise ValueError(f'missing assignment at {address:x}')
    name_addr, result_addr = int(literals[-1],16), int(assignment[1],16)
    rows.append({'name':literal(name_addr), 'third_output':literal(result_addr), 'comparison_call':hex(address), 'condition':branch[2], 'branch_target':branch[3], 'assignment_literal':hex(result_addr), 'assignment_branch':assignment_branch})
groups = {'CI': {'CI'}}
for lo, hi, target, output, count in [
    (0x1422d01a1, 0x1422d0942, '0x1422d0942', 'GameDev', 14),
    (0x1422d0b29, 0x1422d0de1, '0x1422d0de1', 'Stage', 5),
    (0x1422d0ed9, 0x1422d29ca, '0x1422d253d', 'Prod', 41),
]:
    group = set()
    for i, line in enumerate(asm):
        addr = int(line.split()[0],16)
        if lo <= addr < hi and 'CALL 0x142bcf780' in line:
            prev = '\n'.join(asm[max(0,i-20):i])
            name = re.findall(r'LEA RDX,\[(0x[0-9a-f]+)\]',prev)
            if not name or 'TEST EAX,EAX' not in asm[i+1] or not asm[i+2].endswith('JZ '+target):
                raise ValueError(f'unexpected second-stage branch at {addr:x}')
            group.add(literal(int(name[-1],16)))
    if len(group) != count: raise ValueError(f'unexpected {output} group size')
    groups[output] = group
canonical = {r['third_output'] for r in rows}
assert len(rows) == len({r['name'].casefold() for r in rows}) == 62
assert sum(map(len, groups.values())) == len(canonical) == 61
assert set.union(*groups.values()) == canonical
for row in rows:
    row['second_output'] = next(output for output, names in groups.items() if row['third_output'] in names)
    row['result'] = True
print(json.dumps({'image_base':'0x140000000','scope':'nonempty normalized override; second output is second argument, third output is third argument', 'names':rows},indent=2))
