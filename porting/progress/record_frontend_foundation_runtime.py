#!/usr/bin/env python3
"""Revalidate and record narrowly scoped frontend foundation observations."""
import hashlib
import json
from pathlib import Path
import subprocess

REPO = Path(__file__).resolve().parents[2]
BASE = str(REPO / 'porting/generated/frontend-native-evidence/common-input-current-type-runtime-1') + '/'
BUILD = '/mnt/vibrato/fortnite-work/common-input-current-type-native-build-1/'
SPECS = {
 'common-input-current-type-runtime-1': ('bad3def28dc6c5a3f72e39f9f1ab720c39675b78c718df5e6b39f5d387772192', '86103596459b8329ea54f862979f7e4412aa17b5c67c44723429251b81245b2b', 'dcc14c391c2c67d9ae4bdc1b379848586e739596', 14),
 'common-input-action-row-runtime-1': ('f4fc348961f36825b3cd059240e634b6ea8914fc8c122443199f63d2866abb56', '45875fffea484bd3c73521d67e8859361144a82c1bd26a58a16c5baebdd74010', '29a2c24469885237cf34e05d2959f28f15b3a26c', 19),
 'main-button-defaults-runtime-1': ('fc8a03edc6e938e455229bbf7ab552a58899bede6ffbe9b5021389f2f178a8ca', 'f2ed9e2c6a82fdc887bc5dfa095e4d952e9588c6a75f0095d59d03fcc23facd1', '04822b2e36498c1e7262ee1e04f08c804d0749e9', 10),
 'athena-controller-default-runtime-1': ('a86f92e378125af3271f11a0e613a50242125a784d41a0309c0bb8e015474064', '481dcdb0210adef749d7d0344bd91d65b075dfd03c7bce6684c4830ab59ef2a8', '78c5b616998028c78befb2053ca97f844d3e477f', 3),
}
SOURCE = SPECS['common-input-current-type-runtime-1'][2]

def require(ok, message):
 if not ok: raise ValueError(message)

def read_pinned(path, digest):
 data = Path(path).read_bytes()
 require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + str(path))
 return data

def committed(commit, path):
 return subprocess.check_output(['git', 'show', commit + ':' + path], cwd=REPO)

def verifier(path, text):
 # Git object identity pins these historical verifier implementations independently of HEAD.
 namespace = {'__name__': 'pinned_verifier'}
 exec(compile(committed(SOURCE, path), path, 'exec'), namespace)
 return namespace['verify'](text)

def load_release(name):
 ch, rh, source, count = SPECS[name]
 cp = REPO / 'porting/generated/frontend-native-evidence' / name / 'checkpoint.json'
 rp = Path('/mnt/vibrato/fortnite-evidence/frozen-runtime') / name / 'source-release.json'
 c, r = json.loads(read_pinned(cp, ch)), json.loads(read_pinned(rp, rh))
 require(c['schema'] == 'fortnite.' + name.removesuffix('-1') + '-checkpoint.v1' and c['native_commit'] == source and c['engine_commit'] == 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca', 'checkpoint identity')
 require(r['schema'] == 'fortnite.source-release.v1' and r['native_commit'] == source and r['checkpoint_sha256'] == ch and r['all_build_stage_probe_join_handles_terminal'] is True and r['active_native_build_or_stage_owner'] is None, 'release identity')
 require(len(c['source_files']) == count and len({x['path'] for x in c['source_files']}) == count, 'source inventory')
 for row in c['source_files']:
  rel = Path(row['path']).relative_to(REPO).as_posix()
  require(hashlib.sha256(committed(source, rel)).hexdigest() == row['sha256'], 'committed source mismatch: ' + rel)
  read_pinned(row['archive'], row['sha256'])
 return c

def validate(c):
 artifacts = {x['path']: x for x in c['artifacts']}
 require(len(artifacts) == len(c['artifacts']) == 86, 'artifact inventory')
 for x in artifacts.values(): read_pinned(x['archive'], x['sha256'])
 def artifact(path): return read_pinned(artifacts[path]['archive'], artifacts[path]['sha256'])
 def doc(path): return json.loads(artifact(path))
 builds = doc(BUILD + 'results.json')
 require(builds == c['builds'] and len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build inventory')
 for b in builds:
  require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0 and artifacts[b['log']]['sha256'] == b['log_sha256'], 'build receipt')
 rows = doc(BUILD + 'stage.json')['rows']
 require(rows == c['binary_rows'] and len(rows) == 18 and doc(BUILD + 'stage.json')['build_source'] == SOURCE, 'stage')
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
 launches = c['retained_diagnostics']
 require(launches == doc(BASE + 'retained-native-1/launch-results.json') and len(launches) == 8 and {x['case'] for x in launches} == {'confirmation-description','panel-outro','panel-intro','main-button','button','activation','registry','lifecycle'} and all(x['exit'] == 0 for x in launches), 'retained launches')
 prefix = BASE + 'retained-native-1/main-button/'
 p, t = doc(prefix + 'receipt.json'), doc(prefix + 'retained-transcript-check.json')
 text = process(p)
 require(p['case'] == 'main-button' and p['passed'] is True and '-MainButtonStateProbe' in p['argv'] and t['passed'] is True, 'button process')
 result = verifier('porting/frontend/button_internal_runtime_probe/main_button_verifier.py', text)
 require(result == p['transcript'] == t['detail'] == doc(prefix + 'verification.json') and result['rows'] == 44, 'button transcript')
 for key, directory, flag, vpath, expected in [('common_input_row','common-input-row-1','-CommonInputActionRowProbe','common_input_action_row_verifier.py',30), ('current_input_type','common-input-current-type-1','-CommonInputCurrentTypeProbe','current_input_type_verifier.py',13)]:
  p = c[key]; prefix = BASE + directory + '/'; require(p == doc(prefix + 'receipt.json') and p['passed'] is True and flag in p['argv'], key + ' receipt')
  result = verifier('porting/frontend/button_internal_runtime_probe/' + vpath, process(p))
  require(result == p['transcript'] == doc(prefix + 'verification.json') and result['rows'] == expected, key + ' transcript')
  if key == 'common_input_row': require(result['asset_row_count'] == 421, 'input table rows')
 p = c['creation_control']; prefix = BASE + 'root-create-control-1/'
 require(p == doc(prefix + 'receipt.json') and '-RootWidgetInstanceProbe' in p['argv'], 'root receipt')
 result = verifier('porting/frontend/root_widget_instance_probe/verifier.py', process(p))
 require(result == p['transcript'] == doc(prefix + 'verification.json') and result['creation_completed'] and result['startup_classes_match'] and result['widget_count'] == 7 and result['dependency_error_count'] == 20 and result['passed'] is False, 'root observations')
 require(result['startup_class_observations'] == {'game_instance_class':'/Script/FortniteGame.FortGameInstance','player_class':'/Script/FortniteGame.FortLocalPlayer','viewport_class':'/Script/FortniteGame.FortGameViewportClient','controller_class':'/Script/FortniteGame.FortPlayerControllerAthena'}, 'startup class selections')
 joins = c['joins']; require(joins == doc(BASE + 'joins.json') and len(joins) == 2 and {x['case'] for x in joins} == {'default','athena'}, 'joins')
 for j in joins:
  require(j['exit'] == 0 and j['end_ns'] > j['start_ns'] > max(b['end_ns'] for b in builds) and set(j['logs']) == {'result.txt','server.log','client.log'}, 'join receipt')
  for x in j['logs'].values(): read_pinned(x['path'], x['sha256'])

def check():
 c = load_release('common-input-current-type-runtime-1')
 for name, critical in [('common-input-action-row-runtime-1', ('FCommonInputActionData.h', 'FCommonInputTypeInfo.h', 'CommonInputActionRowProbe.cpp')), ('main-button-defaults-runtime-1', ('UCommonButton.cpp', 'UCommonButton.h')), ('athena-controller-default-runtime-1', ('FortniteAthenaGameplay.cpp',))]:
  old = load_release(name)
  for filename in critical:
   matches = [x for x in old['source_files'] if Path(x['path']).name == filename]
   require(len(matches) == 1, 'critical source inventory: ' + filename)
   row = matches[0]; rel = Path(row['path']).relative_to(REPO).as_posix()
   require(hashlib.sha256(committed(SOURCE, rel)).hexdigest() == row['sha256'], 'critical source changed: ' + rel)
 validate(c)

def main():
 check()
 path = REPO / 'porting/progress/evidence.json'; ledger = json.loads(path.read_text())
 scopes = {
  'frontend_foundation_button_defaults_query': 'CommonButton reflected defaults and controlled 44-row CDO/new-widget query diagnostic; not full lifecycle or interactive UI.',
  'frontend_foundation_input_action_row': 'C++ and reflected input row construction/layout and original InputActionDataTable row-struct identity, 30 checks and 421 rows; not input routing or table contents.',
  'frontend_foundation_current_input_type': 'Controlled current input type LocalPlayerSubsystem metadata and direct/reflected getter diagnostic, 13 rows; not input producers or routing.',
  'frontend_foundation_startup_selection': 'Four observed startup class selections and controller constructor source; Root CreateWidget produced seven widgets while 20 unresolved imports remain. Not full startup UI or parity.',
 }
 for key, scope in scopes.items():
  ledger['checks'][key] = {'id':key,'result':'pass','exit_code':0,'command':'python3 -I porting/progress/record_frontend_foundation_runtime.py','checkpoint':BASE + 'checkpoint.json','checkpoint_sha256':SPECS['common-input-current-type-runtime-1'][0], 'release':'/mnt/vibrato/fortnite-evidence/frozen-runtime/common-input-current-type-runtime-1/source-release.json','release_sha256':SPECS['common-input-current-type-runtime-1'][1], 'scope':scope, 'provenance_note':'Historical Git source and frozen source pairs and artifacts are required. Stage binary hashes are cross-checked, not rehashed.'}
 path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__': main()
