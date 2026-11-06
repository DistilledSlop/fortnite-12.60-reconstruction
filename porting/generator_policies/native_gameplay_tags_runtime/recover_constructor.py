import sys,mmap,json,collections,pathlib,hashlib,argparse
import capstone
from capstone.x86_const import *
parser=argparse.ArgumentParser(description="Recover the complete native gameplay tag constructor event sequence")
parser.add_argument("image", type=pathlib.Path)
parser.add_argument("--output", type=pathlib.Path, required=True)
args=parser.parse_args()
f=args.image.open("rb"); m=mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ)
assert hashlib.sha256(m).hexdigest()=="338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680", "Unexpected original image"
args.output.mkdir(parents=True,exist_ok=True)
BASE=0x140000000; SELF=0x10000000; STACK=0x20000000
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64); c.detail=True
regs={'rcx':SELF,'rsp':STACK}; mem={}; events=[]; zeros=[]
alias={'eax':'rax','ecx':'rcx','edx':'rdx','ebx':'rbx','esp':'rsp','ebp':'rbp','esi':'rsi','edi':'rdi','al':'rax','cl':'rcx','dl':'rdx','bl':'rbx'}
for k in range(8,16): alias[f'r{k}d']=alias[f'r{k}b']=f'r{k}'
def regname(r):
 n=c.reg_name(r);return alias.get(n,n)
def addr(i,o):return (i.address+i.size if o.mem.base==X86_REG_RIP else regs.get(regname(o.mem.base),0))+regs.get(regname(o.mem.index),0)*o.mem.scale+o.mem.disp
def get(i,o):
 if o.type==X86_OP_REG:return regs.get(regname(o.reg),0)
 if o.type==X86_OP_IMM:return o.imm
 a=addr(i,o)
 if a>=BASE:return int.from_bytes(m[a-BASE:a-BASE+o.size],'little')
 return mem.get(a,0)
def put(i,o,v):
 if o.type==X86_OP_REG:regs[regname(o.reg)]=v
 else:
  a=addr(i,o);mem[a]=v
  if SELF<=a<SELF+0x3000:zeros.append((i.address,a-SELF,o.size,v))
def cstr(a):return m[a-BASE:m.find(b'\0',a-BASE)].decode('ascii')
def run(a,size):
 for i in c.disasm(m[a-BASE:a-BASE+size],a):
  op=i.operands; n=i.mnemonic
  if n=='ret':return
  if n in ('mov','movzx','movsxd'):put(i,op[0],get(i,op[1]))
  elif n=='lea':put(i,op[0],addr(i,op[1]))
  elif n=='push':regs['rsp']-=8;mem[regs['rsp']]=get(i,op[0])
  elif n=='pop':put(i,op[0],mem.get(regs['rsp'],0));regs['rsp']+=8
  elif n in ('add','sub','xor','or','and'):
   x,y=get(i,op[0]),get(i,op[1]);put(i,op[0],{'add':lambda:x+y,'sub':lambda:x-y,'xor':lambda:x^y,'or':lambda:x|y,'and':lambda:x&y}[n]())
  elif n=='call':
   target=op[0].imm
   if target in [0x141ec98b0,0x141ec87c0,0x141ec9870]:
    offset=regs['rdx']-SELF; assert 0<=offset<0x2568,(hex(i.address),hex(offset))
    e={'kind':'tag','call':hex(i.address),'offset':hex(offset),'literal_va':hex(regs['r8']),'name':cstr(regs['r8'])}
    if target!=0x141ec98b0:e.update(map_offset=hex(0x8d0 if target==0x141ec87c0 else 0x880),value=regs['r9'])
    events.append(e)
   elif target==0x141ec8c00:
    regs['rsp']-=8;run(target,0xc68);regs['rsp']+=8
   elif target==0x141cf6b40:
    p=regs['r8']; events.append({'kind':'map','call':hex(i.address),'map_offset':hex(regs['rcx']-SELF),'tag_offset':hex(mem[p]-SELF),'value':mem[mem[p+8]]})
   elif target==0x143e8b3b0:events.append({'kind':'container','call':hex(i.address),'container_offset':hex(regs['rcx']-SELF),'tag_offset':hex(regs['rdx']-SELF)})
   elif target==0x143e8dfa0:events.append({'kind':'done','call':hex(i.address)})
   elif target==0x143e91c00:pass
   else:raise Exception(hex(i.address),hex(target))
  elif n in ('test','jne'):
   assert i.address in (0x141ec642a,0x141ec642d), (hex(i.address),n)
   # Both paths join with the same manager; creation has no separate source event.
  else:raise Exception(hex(i.address),n,i.op_str)
run(0x141ebb310,0xb1c9)
assert collections.Counter(e['kind'] for e in events)=={'tag':1136,'map':37,'container':7,'done':1}
assert len(set(e['offset'] for e in events if e['kind']=='tag'))==1136
(args.output/'constructor_events.json').write_text(json.dumps(events,indent=2)+'\n')
(args.output/'initialization.json').write_text(json.dumps(zeros,indent=2)+'\n')
print(json.dumps({'events':len(events),'kinds':dict(collections.Counter(e['kind'] for e in events)),'initialization_writes':len(zeros)}))
