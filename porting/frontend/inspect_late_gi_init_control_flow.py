#!/usr/bin/env python3
"""Replay closed late-capture GI Init CFG, raw windows and proven RET dispatches."""
import argparse,hashlib,json,struct
from pathlib import Path
BASE=0x140000000
IMAGE_SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
PROOF_SHA='e9fae0fe8e7b3d61fe766440c76f3497872aa057a91d9d326e30affbd812fdcf'
REGS=['rax','rbx','rcx','rdx','rbp','rsi','rdi','r8','r9','r10','r11','r12','r13','r14','r15']
def dispatch(p,taken):
    state={k:k for k in REGS};sp=0;stack={};dest=None
    def address(s):
        assert s.startswith('qword ptr [rsp') and s.endswith(']'),s
        suffix=s[14:-1].replace(' ','')
        return sp+(int(suffix,0) if suffix else 0)
    def get(s):
        if s.startswith('qword ptr [rsp'):
            k=address(s);return stack.get(k,('stack',k))
        if s.startswith('0x'):return int(s,16)
        assert s in state,s
        return state[s]
    def put(s,v):
        if s.startswith('qword ptr [rsp'):stack[address(s)]=v
        else:
            assert s in state,s
            state[s]=v
    for x in p['chain']:
        m=x['mnemonic'];o=x['op_str'];ops=o.split(', ')
        if m=='push':v=get(o);sp-=8;stack[sp]=v
        elif m=='pop':put(o,stack.get(sp,('stack',sp)));sp+=8
        elif m in ('mov','movabs'):put(ops[0],get(ops[1]))
        elif m=='lea':
            assert ops[0]=='rbp' and ops[1].startswith('[rip ')
            put('rbp',int(x['va'],16)+len(bytes.fromhex(x['bytes']))+int(ops[1].split('rip ')[1][:-1].replace(' ',''),16))
        elif m=='xchg':a,b=get(ops[0]),get(ops[1]);put(ops[0],b);put(ops[1],a)
        elif m in ('cmove','cmovne','cmovle','cmovg'):
            assert m.replace('cmov','j')==p['condition']
            if taken:put(ops[0],get(ops[1]))
        elif m=='ret':dest=stack.get(sp,('stack',sp));sp+=8
        elif m=='jmp':assert o.startswith('0x')
        else:raise AssertionError('unexpected dispatcher operation '+m)
    assert sp==0,(p['va'],sp)
    assert state=={k:k for k in REGS},p['va']
    assert dest==int(p['targets'][taken if len(p['targets'])==2 else 0],16),p['va']
    return dest

def relative(a,b):
    if b[0] in (0xe8,0xe9):return a+5+struct.unpack_from('<i',b,1)[0]
    if b[0]==0xeb or 0x70<=b[0]<=0x7f:return a+2+struct.unpack_from('<b',b,1)[0]
    if b[:1]==b'\x0f' and 0x80<=b[1]<=0x8f:return a+6+struct.unpack_from('<i',b,2)[0]
    raise AssertionError('unsupported relative control instruction '+hex(a))

def stack_closure(cfg,dispatches):
    nodes={int(x['va'],16):x for x in cfg['instructions']}
    pending=[(int(cfg['entry'],16),0)];deltas={};exits=[]
    while pending:
        a,sp=pending.pop()
        if a in deltas:
            assert deltas[a]==sp,('inconsistent stack merge',hex(a),sp,deltas[a])
            continue
        deltas[a]=sp;x=nodes[a];m=x['mnemonic'];o=x['op_str'];n=a+len(bytes.fromhex(x['bytes']))
        if a in dispatches:
            pending.extend((int(t,16),sp) for t in dispatches[a]['targets']);continue
        if m=='push':sp-=8
        elif m=='pop':sp+=8
        elif m in ('add','sub') and o.startswith('rsp,'):
            value=int(o.split(', ')[1],0);sp+=value if m=='add' else -value
        elif m=='lea' and o.startswith('rsp, '):
            assert o.startswith('rsp, [rsp '),x
            sp+=int(o.split('[rsp ')[1][:-1].replace(' ',''),0)
        if m.startswith('j'):
            if o.startswith('0x'):pending.append((int(o,16),sp))
            else:
                assert a==0x141e109aa and sp==8,('caller return slot',hex(a),sp)
                exits.append((hex(a),sp))
            if m=='jmp':continue
        pending.append((n,sp))
    assert set(deltas)==set(nodes) and exits==[('0x141e109aa',8)]
    return len(deltas)

def inspect(image,proof_path):
    raw=proof_path.read_bytes();assert hashlib.sha256(raw).hexdigest()==PROOF_SHA,'proof SHA mismatch'
    p=json.loads(raw);d=image.read_bytes();assert len(d)==165991936
    assert hashlib.sha256(d).hexdigest()==IMAGE_SHA,'late image SHA mismatch'
    for w in p['windows']:
        a=int(w['va'],16)-BASE;b=d[a:a+w['size']]
        assert hashlib.sha256(b).hexdigest()==w['sha256'],w['va']
        assert b.hex()==w['bytes'],w['va']
    originals={int(x['va'],16):x for x in p['raw_cfg']['instructions']}
    for x in originals.values():
        a=int(x['va'],16)-BASE;b=bytes.fromhex(x['bytes']);assert d[a:a+len(b)]==b,x['va']
    dispatches={int(x['va'],16):x for x in p['dispatches']}
    for x in dispatches.values():
        assert originals[int(x['va'],16)]['mnemonic']=='push'
        for i in x['chain']:assert originals[int(i['va'],16)]==i,i['va']
        for taken in (False,True):dispatch(x,taken)
    cfg=p['exact_cfg'];semantic={int(x['va'],16):x for x in cfg['instructions']}
    q=[int(cfg['entry'],16)];seen=set();calls=[];actual_edges=[];terminals=[];used=set()
    while q:
        a=q.pop()
        while a not in seen:
            assert a in semantic,hex(a);x=semantic[a];assert x==originals[a];seen.add(a)
            b=bytes.fromhex(x['bytes']);m=x['mnemonic'];o=x['op_str']
            if a in dispatches:
                used.add(a)
                for t in dispatches[a]['targets']:q.append(int(t,16));actual_edges.append([hex(a),t,'proven-dispatch'])
                break
            if m.startswith('j'):
                if o.startswith('0x'):
                    t=relative(a,b);assert t==int(o,16);q.append(t);actual_edges.append([hex(a),hex(t),'jump'])
                else:
                    assert a==0x141e109aa and b.hex()=='ff6424f8' and o=='qword ptr [rsp - 8]'
                    terminals.append({'va':hex(a),'kind':'caller_return_after_epilogue'})
                if m=='jmp':break
            elif m=='call':
                if b[0]==0xe8:t=hex(relative(a,b));assert t==o
                else:assert b[0] in (0xff,0x41,0x42,0x48,0x49);t=o
                calls.append([hex(a),t,'call']);actual_edges.append([hex(a),t,'call'])
            else:assert m not in ('ret','ud2','int3'),x
            a+=len(b)
    assert seen==set(semantic)
    assert used==set(dispatches)
    assert sorted(actual_edges)==sorted(cfg['edges'])
    assert terminals==cfg['terminals'] and not cfg['frontiers']
    assert stack_closure(cfg,dispatches)==4037
    assert len(seen)==4037 and len(originals)==5702 and len(calls)==334 and len(used)==149
    # Concrete initial entry, fourth UI caller, paired callbacks and cleanup byte anchors.
    assert ['0x141e6934b','0x141e6c120','call'] in calls
    for a,t in [('0x1495c3d91','0x141e72150'),('0x140930381','0x141e728e0')]:
        x=originals[int(a,16)];b=bytes.fromhex(x['bytes']);assert b[:3]==b'\x48\x8d\x05'
        assert int(a,16)+7+struct.unpack_from('<i',b,3)[0]==int(t,16)
    return {'passed':True,'image_sha256':IMAGE_SHA,'proof_sha256':PROOF_SHA,'windows_verified':len(p['windows']),
            'reachable_original_instructions':5702,'semantic_instructions_including_dispatch_entries':4037,
            'proven_dispatches':149,'both_condition_outcomes_verified':True,'calls':334,
            'unknown_control_frontiers':[],'complete_intraprocedural_cfg':True,'all_path_stack_closure_verified':True,
            'full_transitive_source_implementation_ready':False,'runtime_claim':False,'product_milestones':[False]*4}
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--image',type=Path,required=True);ap.add_argument('--proof',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    assert not a.output.exists(),'output must be new'
    result=inspect(a.image,a.proof);a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if __name__=='__main__':main()
