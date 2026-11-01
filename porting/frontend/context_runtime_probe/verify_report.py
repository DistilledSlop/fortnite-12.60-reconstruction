#!/usr/bin/env python3
"""Fail closed: receipt + current frozen artifacts + two fresh targets + two live cases."""
import argparse,json
from pathlib import Path
import provenance as p
import verify_metadata as m
import verify_events as e
import verify_v1

def check(v,msg):
 if not v: raise ValueError(msg)
def main():
 a=argparse.ArgumentParser();a.add_argument('--evidence',type=Path,required=True);a.add_argument('--original',type=Path,required=True);a.add_argument('--report',type=Path,required=True);args=a.parse_args();p.pin()
 root=args.evidence.resolve();start_file=root/'start.json';build_file=root/'build.json';st=json.loads(start_file.read_text());build=json.loads(build_file.read_text())
 check(st['schema']=='context-build-start-v1' and build['schema']=='context-build-v1' and p.digest(start_file)==build['start_sha256'],'build receipt chain');check(st['candidate_receipt_sha256']==p.RH and st['candidate_patch_sha256']==p.PH and st['root_audit_sha256']==p.AH,'original candidate chain')
 check(p.tree(Path(st['workspace'])/'porting/native/Source')==st['snapshot']['source_tree'] and p.engine_state(Path(st['snapshot']['engine']['root']))==st['snapshot']['engine'] and p.digest(Path(st['workspace'])/'porting/gen_native_classes.py')==st['snapshot']['generator'],'current source snapshot')
 for name,sha in st['snapshot']['instrumentation'].items(): check(p.expected_source_hash(name)==sha,'probe source pin')
 outputs={}
 for target in ('FortniteLinux','FortniteLinuxServer'):
  r=build['targets'][target]
  for k in ('cpp','object','rsp','unity_source','binary','debug','log'):
   item=r[k];path=Path(item['path']);check(path.is_file() and p.digest(path)==item['sha256'] and (k in ('rsp','unity_source') or item['mtime_ns']>=st['start_ns']),'fresh target artifact '+target+' '+k)
  resolved=p.unity.resolve(Path(r['object']['path']).parent,Path(r['cpp']['path']),st['start_ns']);check(all(str(resolved[k])==r[k]['path'] for k in ('object','rsp','unity_source')),'unity compilation recipe')
  log=Path(r['log']['path']).read_text(errors='replace');check('Total execution time:' in log and 'ERROR:' not in log and Path(r['unity_source']['path']).name in log,'build compilation marker')
  br=r['build_run'];check(p.digest(br['path'])==br['sha256'],'build run receipt hash');brun=json.loads(Path(br['path']).read_text());check(brun['schema']=='context-build-run-v1' and brun['target']==target and brun['exit']==0 and brun['log_sha256']==r['log']['sha256'] and brun['start_sha256']==p.digest(start_file) and brun['start_ns']>=st['start_ns'] and target in brun['argv'],'build invocation')
  outputs[target]=m.audit(args.original,*(Path(r[k]['path']) for k in ('cpp','object','binary','debug')))
 runs={}
 for case in ('registry','lifecycle'):
  path=root/(case+'.run.json');run=json.loads(path.read_text());check(run['schema']=='context-run-v2' and run['case']==case and run['target']=='FortniteLinux' and run['exit']==0 and run['proc_exe_inode_verified'] and run['binary_after_verified'] and not run['timed_out'] and run['error'] is None and run['build_sha256']==p.digest(build_file),'run receipt '+case)
  check(run['binary_sha256']==build['targets']['FortniteLinux']['binary']['sha256'] and p.digest(run['argv'][0])==run['binary_sha256'] and '-ContextRuntimeProbe' in run['argv'] and '-ContextProbeCase='+case in run['argv'],'live executable binding '+case)
  log=Path(run['log']['path']);check(log.resolve()==(root/(case+'.runtime.log')).resolve() and p.digest(log)==run['log']['sha256'],'live log hash '+case)
  text=log.read_text();check(text.count('CONTEXT_DRIVER start case='+case+' ')==1 and text.count('CONTEXT_DRIVER complete case='+case)==1 and 'CONTEXT_DRIVER timeout' not in text,'automatic driver completion');check(case!='registry' or verify_v1.verify(text),'live lookups, GC, removal and reflection checks')
  detail=e.registry(text) if case=='registry' else e.lifecycle(text)
  runs[case]={'run_sha256':p.digest(path),'log_sha256':p.digest(log),'pid':run['pid'],'start_ns':run['start_ns'],'end_ns':run['end_ns'],'observer':detail}
 check(runs['registry']['pid']!=runs['lifecycle']['pid'] or runs['registry']['end_ns']<runs['lifecycle']['start_ns'],'distinct process invocations')
 report={'schema':'context-live-verification-v1','passed':True,'source_receipt_sha256':p.RH,'candidate_patch_sha256':p.PH,'root_audit_sha256':p.AH,'build_sha256':p.digest(build_file),'metadata':outputs,'live_cases':runs,'limitations':'Diagnostic creation only; concrete subsystem init/tick/service behavior, authentic startup and four product milestones remain unverified.'}
 p.write_new(args.report,report);print(json.dumps({'report':str(args.report.resolve()),'sha256':p.digest(args.report),'passed':True}))
if __name__=='__main__':main()
