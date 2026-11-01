#!/usr/bin/env python3
"""Read-only original View metadata probes; never saves the Ghidra program."""
import argparse,json,os,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'porting/ghidra'))
import decompile_targets as base
ap=argparse.ArgumentParser();ap.add_argument('--strings',nargs='*',default=[]);ap.add_argument('--functions',nargs='*',default=[]);ap.add_argument('--chain');ap.add_argument('--out',required=True);args=ap.parse_args()
out=Path(args.out);out.mkdir(parents=True,exist_ok=True)
import pyghidra
from pyghidra.launcher import HeadlessPyGhidraLauncher
launcher=HeadlessPyGhidraLauncher(install_dir=base.INSTALL);launcher.add_vmargs('-Xmx16G');launcher.start()
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import TaskMonitor
before=base.snapshot([os.path.join(base.PROJECT_DIR,base.PROJECT+'.rep'),os.path.join(base.PROJECT_DIR,base.PROJECT+'.gpr')])
project=pyghidra.open_project(base.PROJECT_DIR,base.PROJECT,create=False)
program,consumer=pyghidra.consume_program(project,base.PROGRAM_PATH)
try:
 af=program.getAddressFactory().getDefaultAddressSpace();fm=program.getFunctionManager();ls=program.getListing();rm=program.getReferenceManager()
 refs=[];funcs={}
 for a in args.strings:
  for r in rm.getReferencesTo(af.getAddress(a)):
   frm=r.getFromAddress();f=fm.getFunctionContaining(frm)
   row={'target':a,'from':str(frm),'instruction':str(ls.getInstructionAt(frm)),'function':str(f.getEntryPoint()) if f else None};refs.append(row)
   if f:funcs[str(f.getEntryPoint())]=f
 for a in args.functions:
  f=fm.getFunctionContaining(af.getAddress(a))
  if f:funcs[str(f.getEntryPoint())]=f
 if args.strings:(out/'refs.json').write_text(json.dumps(refs,indent=2)+'\n')
 if args.chain:
  chain=[];entry=args.chain;seen=set()
  while entry and entry not in seen:
   seen.add(entry);f=fm.getFunctionAt(af.getAddress(entry))
   if not f:raise RuntimeError('Missing chain function '+entry)
   ins=list(ls.getInstructions(f.getBody(),True));links=[];strings=[]
   for i in ins:
    if str(i.getMnemonicString())=='LEA' and str(i.getDefaultOperandRepresentation(0))=='RAX':
     for r in i.getReferencesFrom():
      target=r.getToAddress()
      if fm.getFunctionAt(target):links.append(str(target))
    for r in i.getReferencesFrom():
     val=base.read_wstring(program.getMemory(),r.getToAddress())
     if val:strings.append({'address':str(r.getToAddress()),'text':val})
   row={'function':entry,'strings':strings,'next':links[-1] if links else None,'asm':[str(i.getAddress())+' '+str(i) for i in ins]};chain.append(row)
   entry=row['next']
  (out/'member-chain.json').write_text(json.dumps(chain,indent=2)+'\n')
  print('chain',len(chain),'GameTime',[(i,r['function']) for i,r in enumerate(chain) if any(s['text']=='GameTime' for s in r['strings'])],flush=True)
 dec=DecompInterface();dec.openProgram(program)
 for a,f in funcs.items():
  res=dec.decompileFunction(f,90,TaskMonitor.DUMMY)
  if res.decompileCompleted():(out/(a+'.c')).write_text(str(res.getDecompiledFunction().getC()))
  (out/(a+'.asm')).write_text('\n'.join(str(i.getAddress())+' '+str(i) for i in ls.getInstructions(f.getBody(),True)))
  print(a,f.getBody().getNumAddresses(),res.decompileCompleted(),flush=True)
 dec.dispose()
finally:
 program.release(consumer);project.close()

after=base.snapshot([os.path.join(base.PROJECT_DIR,base.PROJECT+'.rep'),os.path.join(base.PROJECT_DIR,base.PROJECT+'.gpr')])
changed=base.snapshot_diff(before,after)
(out/'project-integrity.json').write_text(json.dumps({'project_files_changed':changed,'program_saved':False},indent=2)+'\n')
if changed:print('Project bookkeeping changed; see project-integrity.json (program never saved).',flush=True)
