#!/usr/bin/env python3
"""Revalidate frozen action binding data evidence and record its bounded scope."""
import hashlib
import json
from pathlib import Path
import subprocess

REPO = Path(__file__).resolve().parents[2]
BASE = str(REPO / 'porting/generated/frontend-native-evidence/action-binding-data-runtime-1') + '/'
FROZEN = Path('/mnt/vibrato/fortnite-evidence/frozen-runtime/action-binding-data-runtime-1')
BUILD = '/mnt/vibrato/fortnite-work/action-binding-data-native-build-2/'
SOURCE = 'cf37b21351f47a13d565688a4a1c06563c8f5ef8'
CHECKPOINT = 'b3f7a28eec0743c1113578683b8fc16dda204e93ffc37e15ece5893e02d0f834'
RELEASE = 'fb2b0a6af2416ba6f9b68c0d4c27e02d31fb4a9e388ab816a9f90155ce4cd93a'

def require(ok, message):
 if not ok: raise ValueError(message)

def read_pinned(path, digest):
 data = Path(path).read_bytes()
 require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + str(path))
 return data

def committed(path):
 return subprocess.check_output(['git', 'show', SOURCE + ':' + path], cwd=REPO)

def verifier(name, text):
 path = 'porting/frontend/button_internal_runtime_probe/' + name
 ns = {'__name__': 'pinned_verifier'}
 exec(compile(committed(path), path, 'exec'), ns)
 return ns['verify'](text)

def load_release():
 c = json.loads(read_pinned(BASE + 'checkpoint.json', CHECKPOINT))
 r = json.loads(read_pinned(FROZEN / 'source-release.json', RELEASE))
 require(c['schema'] == 'fortnite.action-binding-data-runtime-checkpoint.v1' and c['native_commit'] == c['observed_repo_head'] == SOURCE and c['engine_commit'] == 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca', 'checkpoint identity')
 require(r['schema'] == 'fortnite.source-release.v1' and r['native_commit'] == SOURCE and r['checkpoint_sha256'] == CHECKPOINT and r['all_build_stage_probe_join_handles_terminal'] is True and r['active_native_build_or_stage_owner'] is None, 'release identity')
 rows = c['source_files']; require(len(rows) == len({x['path'] for x in rows}) == 14, 'source inventory')
 for row in rows:
  rel = Path(row['path']).relative_to(REPO).as_posix()
  require(row['revision'] == SOURCE and hashlib.sha256(committed(rel)).hexdigest() == row['sha256'], 'committed source mismatch: ' + rel)
  read_pinned(row['archive'], row['sha256'])
 for ext in ('h', 'cpp'):
  template = 'porting/frontend/common_action_binding_runtime/CommonActionBindingData.' + ext
  generated = 'porting/native/Source/CommonUI/Generated/CommonActionBindingData.' + ext
  require(committed(template) == committed(generated), 'generated source differs from maintained template: ' + ext)
 probe = committed('porting/native/Source/FortniteGame/ActionBindingDataProbe.cpp')
 for marker in (b'copy_arrays', b'map_find', b'map_replace_release', b'map_remove', b'weak_owner_live', b'weak_owner_expired', b'weak_callbacks_expired'):
  require(marker in probe, 'native probe missing: ' + marker.decode())
 return c

def validate(c):
 artifacts = {x['path']: x for x in c['artifacts']}
 require(len(artifacts) == len(c['artifacts']) == 151, 'artifact inventory')
 for x in artifacts.values(): read_pinned(x['archive'], x['sha256'])
 def artifact(path): return read_pinned(artifacts[path]['archive'], artifacts[path]['sha256'])
 def doc(path): return json.loads(artifact(path))
 builds = doc(BUILD + 'results.json')
 require(builds == c['builds'] and len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build inventory')
 failed = doc('/mnt/vibrato/fortnite-work/action-binding-data-native-build-1/results.json')
 require(len(failed) == 1 and failed[0]['target'] == 'FortniteLinux' and failed[0]['source_head'] == 'bbb1fcf17ffb9a07c9c819914167cc16f9853ac4' and failed[0]['exit'] != 0 and failed[0]['binary'] is None and failed[0]['end_ns'] < min(b['start_ns'] for b in builds) and artifacts[failed[0]['log']]['sha256'] == failed[0]['log_sha256'], 'failed build separation')
 for b in builds:
  require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['config_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0 and b['cwd'] == str(REPO) and b['command'][1:4] == [b['target'], 'Linux', 'Development'] and artifacts[b['log']]['sha256'] == b['log_sha256'], 'build receipt')
 stage = doc(BUILD + 'stage.json'); rows = stage['rows']
 require(rows == c['binary_rows'] and len(rows) == 18 and stage['build_source'] == SOURCE, 'stage')
 for target in ('FortniteLinux', 'FortniteLinuxServer'):
  b = next(x for x in builds if x['target'] == target)
  for suffix in ('', '.sym', '.debug'):
   group = [r for r in rows if r['target'] == target and Path(r['path']).name == target + suffix]
   require(len(group) == 3 and {r['role'] for r in group} == {'source', 'archive', 'stage'} and len({(r['size'], r['sha256']) for r in group}) == 1 and group[0]['size'] > 0 and len(group[0]['sha256']) == 64, 'stage binary fields')
   if not suffix: require((group[0]['size'], group[0]['sha256']) == (b['binary']['size'], b['binary']['sha256']), 'build binary fields')
 linux = next(b for b in builds if b['target'] == 'FortniteLinux')
 binary = next(r for r in rows if r['target'] == 'FortniteLinux' and r['role'] == 'stage' and Path(r['path']).name == 'FortniteLinux')
 def process(p):
  require(p['returncode'] == 0 and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['error'] is None and p['start_ns'] > max(b['end_ns'] for b in builds) and p['end_ns'] > p['start_ns'], 'process status')
  require(p['build'] == linux and p['build_results']['sha256'] == artifacts[BUILD + 'results.json']['sha256'] and p['binary'] == binary['path'] and p['argv'] == p['proc_argv'] and p['argv'][0] == p['binary'] and p['cwd'] == p['proc_cwd'] and artifacts[p['log']['path']]['sha256'] == p['log']['sha256'], 'process identity')
  return artifact(p['log']['path']).decode('utf-8', errors='replace')
 for key, directory, flag, vp, count in [('binding_data','action-binding-data-1','-ActionBindingDataProbe','binding_data_verifier.py',26), ('common_input_row','common-input-row-1','-CommonInputActionRowProbe','common_input_action_row_verifier.py',30), ('current_input_type','common-input-current-type-1','-CommonInputCurrentTypeProbe','current_input_type_verifier.py',13)]:
  p = c[key]; prefix = BASE + directory + '/'
  require(p == doc(prefix + 'receipt.json') and p['passed'] is True and flag in p['argv'], key + ' receipt')
  result = verifier(vp, process(p))
  require(result == p['transcript'] == doc(prefix + 'verification.json') and result['passed'] is True and result['rows'] == count, key + ' transcript')
  if key == 'common_input_row': require(result['asset_row_count'] == 421, 'input table count')
 launches = c['retained_diagnostics']
 require(launches == doc(BASE + 'retained-native-1/launch-results.json') and len(launches) == 8 and {x['case'] for x in launches} == {'confirmation-description','panel-outro','panel-intro','main-button','button','activation','registry','lifecycle'} and all(x['exit'] == 0 for x in launches), 'retained launches')
 for launch in launches:
  prefix = BASE + 'retained-native-1/' + launch['case'] + '/'
  p = doc(prefix + 'receipt.json'); t = doc(prefix + 'retained-transcript-check.json')
  process(p); require(p['case'] == launch['case'] and p['passed'] is True and t['passed'] is True and t['case'] == launch['case'] and ('transcript' not in p or p['transcript'] == t['detail']), 'retained diagnostic')
 joins = c['joins']; require(joins == doc(BASE + 'joins.json') and len(joins) == 2 and {x['case'] for x in joins} == {'default','athena'}, 'joins')
 for j in joins:
  require(j['exit'] == 0 and j['end_ns'] > j['start_ns'] > max(b['end_ns'] for b in builds) and set(j['logs']) == {'result.txt','server.log','client.log'}, 'join receipt')
  for x in j['logs'].values(): require(artifacts[x['path']]['sha256'] == x['sha256'], 'join log hash')
  require(b'Local UE join succeeded.' in artifact(j['logs']['result.txt']['path']), 'join result')

def check(): validate(load_release())

def main():
 check()
 path = REPO / 'porting/progress/evidence.json'; ledger = json.loads(path.read_text())
 key = 'frontend_commonui_binding_data_runtime'
 ledger['checks'][key] = {'id':key,'result':'pass','exit_code':0,'command':'python3 -I porting/progress/record_frontend_binding_data_runtime.py','checkpoint':BASE + 'checkpoint.json','checkpoint_sha256':CHECKPOINT,'release':str(FROZEN / 'source-release.json'),'release_sha256':RELEASE,'scope':'Engine-owned binding record, data and storage component with controlled resolvers; no original UFUNCTION registration, production providers, routing or interactive UI.','provenance_note':'Pinned Git sources and frozen source and artifact pairs are rehashed; stage binary hashes are cross-checked, not independently rehashed.'}
 path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__': main()
