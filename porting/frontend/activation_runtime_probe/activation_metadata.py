"""Activation Linux UHT object -> linked bytes. Reuses the context ELF parser.
Offsets are UE4 Linux FFunctionParams/FBoolPropertyParams from UObjectGlobals.h,
not original Win64 class layout. Original policy source remains the authority.
"""
import re, subprocess
from pathlib import Path
import verify_metadata as m

def symbols(debug):
 result=[]
 proc=subprocess.Popen(['nm','-a','-S','--defined-only',str(debug)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 for line in proc.stdout:
  if 'UCommonActivatableWidget' not in line:continue
  match=re.match(r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+\S\s+(\S+)$',line)
  if match:result.append((int(match[1],16),int(match[2],16),match[3]))
 err=proc.stderr.read();rc=proc.wait();m.need(rc==0,'nm activation symbols '+err[-200:]);return result

def audit(cpp,obj,binary,debug):
 text=Path(cpp).read_text();m.need(m.build_id(binary)==m.build_id(debug),'activation ELF/debug build ID')
 e=m.ObjectELF(obj);linked=m.LinkedELF(binary);syms=symbols(debug)
 owner=m.one_syms(syms,'Z_Construct_UClass_UCommonActivatableWidget','Z_Construct_UClass_UCommonActivatableWidgetv')
 funcs=('ActivateWidget','DeactivateWidget','IsActivated');outputs={}
 reg=e.sym('StaticRegisterNativesUCommonActivatableWidget','E5Funcs');m.need(len(e.raw(reg))==48,'exact activation native pair count')
 lr=m.one_syms(syms,'StaticRegisterNativesUCommonActivatableWidget','E5Funcs');m.need(lr[1]==48,'linked activation pair count')
 pairs=linked.read(lr[0],48)
 for i,name in enumerate(funcs):
  prefix='Z_Construct_UFunction_UCommonActivatableWidget_'+name+'_Statics';flags=0x54020401 if name=='IsActivated' else 0x04020401;ret=name=='IsActivated'
  m.need('DEFINE_FUNCTION(UCommonActivatableWidget::exec'+name+')' in text,'fresh native thunk '+name)
  fn=e.sym(prefix,'10FuncParamsE');raw=e.raw(fn);m.need(len(raw)==72 and m.unpack('Q',raw,40)[0]==int(ret) and m.unpack('III',raw,56)==(int(ret),0x45,flags),'object function fields '+name)
  m.need(e.rel(fn,0).endswith('Z_Construct_UClass_UCommonActivatableWidgetv') and e.rel(fn,16,True)==name,'object owner/name '+name)
  lf=m.one_syms(syms,prefix,'10FuncParamsE');b=linked.read(lf[0],72)
  m.need(b[40:48]==raw[40:48] and b[56:72]==raw[56:72] and m.unpack('Q',b,0)[0]==owner[0] and linked.cstr(m.unpack('Q',b,16)[0])==name,'linked function fields/owner/name '+name)
  creator=m.one_syms(syms,'Z_Construct_UFunction_UCommonActivatableWidget_'+name,'v');m.need(m.instruction_ref(binary,creator[0],creator[1],lf[0]),'linked constructor binds params '+name)
  native='UCommonActivatableWidget'+str(4+len(name))+'exec'+name
  execsym=m.one_syms(syms,native)
  m.need(e.rel(reg,i*16,True)==name and native in e.rel(reg,i*16+8),'object native pair '+name)
  m.need(linked.cstr(m.unpack('Q',pairs,i*16)[0])==name and m.unpack('Q',pairs,i*16+8)[0]==execsym[0],'linked native pair '+name)
  if ret:
   pa=e.sym(prefix,'12PropPointersE');prop=e.sym(prefix,'NewProp_ReturnValueE');p=e.raw(prop)
   # Flags Bool|NativeBool=0x2c; native bool element/outer size=1.
   m.need(e.rel(fn,48)==pa[3] and e.rel(pa,0)==prop[3] and e.rel(prop,0,True)=='ReturnValue','object bool property pointers')
   m.need(len(p)==56 and m.unpack('QIIIIQ',p,16)==(0x0010000000000580,0x2c,0x45,1,1,1),'object native bool property fields')
   lp=m.one_syms(syms,prefix,'NewProp_ReturnValueE');lpa=m.one_syms(syms,prefix,'12PropPointersE');pb=linked.read(lp[0],56)
   m.need(m.unpack('Q',b,48)[0]==lpa[0] and m.unpack('Q',linked.read(lpa[0],8))[0]==lp[0] and pb[16:48]==p[16:48] and linked.cstr(m.unpack('Q',pb)[0])=='ReturnValue','linked bool property fields/pointers')
   setter=m.one_syms(syms,prefix,'NewProp_ReturnValue_SetBitEPv');m.need(e.rel(prop,48)==setter[2] and m.unpack('Q',pb,48)[0]==setter[0],'native bool setter pointer')
  else:m.need(m.unpack('Q',raw,48)[0]==0 and m.unpack('Q',b,48)[0]==0,'no parameters '+name)
  outputs[name]={'function_hex':b.hex(),'native_exec':execsym[2]}
 register=m.one_syms(syms,'StaticRegisterNativesUCommonActivatableWidget','Ev');m.need(m.instruction_ref(binary,register[0],register[1],lr[0]),'linked native registration consumes exact pair array')
 return {'passed':True,'build_id':m.build_id(binary),'functions':outputs,'scope':'Activation Linux UHT metadata/native pairs only; no original private C++ ABI identity'}
