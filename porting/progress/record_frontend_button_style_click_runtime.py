#!/usr/bin/env python3
"""Revalidate the frozen CommonButton style/text/click evidence and record two ledger checks.

Pins the checkpoint and release hashes, rehashes every committed source and archived
artifact, re-runs the pinned per-case verifiers on the archived transcripts, and checks
build, stage and process receipts. Records `frontend_commonui_button_style_runtime` and
`frontend_commonui_button_click_runtime` in evidence.json only when everything passes.
"""
import hashlib
import json
from pathlib import Path
import subprocess

REPO = Path(__file__).resolve().parents[2]
BASE = str(REPO / 'porting/generated/frontend-native-evidence/button-base-click-runtime-3') + '/'
FROZEN = Path('/mnt/vibrato/fortnite-evidence/frozen-runtime/button-base-click-runtime-3')
BUILD = '/mnt/vibrato/fortnite-work/button-base-click-native-build-3/'
SOURCE = 'd08549a56c2d3f259ca770ef8a22a4b6b3e20c36'
CHECKPOINT = '5c6df6fce0c9264664caa4c842d120a32facb8446d40c306b10f58e1d9892b2f'
RELEASE = '34652ba0de81bb9944a28ad7171e012f86c1a84c60942314f29e983c32214d28'
PROBE = 'porting/frontend/button_internal_runtime_probe/'

# key in checkpoint, receipt directory, probe flag, pinned verifier, expected transcript rows
CASES = {
 'button_text_style': ('button-text-style-1', '-CommonButtonTextStyleProbe', 'button_text_style_verifier.py', 32),
 'text_block_style': ('text-block-style-1', '-CommonTextBlockStyleProbe', 'text_block_style_verifier.py', 23),
 'text_transform': ('text-transform-1', '-CommonTextTransformProbe', 'text_transform_verifier.py', 22),
 'erebus': ('erebus-1', '-FortGlobalsErebusProbe', 'erebus_verifier.py', 9),
 'button_size': ('button-size-1', '-ButtonSizeProbe', 'button_size_verifier.py', 11),
 'style_rebuild': ('style-rebuild-1', '-ButtonStyleRebuildProbe', 'style_rebuild_verifier.py', 93),
 'base_click': ('base-click-1', '-ButtonBaseClickProbe', 'base_click_verifier.py', 14),
}
STYLE = ('button_text_style', 'text_block_style', 'text_transform', 'erebus', 'button_size', 'style_rebuild')
CLICK = ('base_click',)

def require(ok, message):
 if not ok: raise ValueError(message)

def read_pinned(path, digest):
 data = Path(path).read_bytes()
 require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + str(path))
 return data

def committed(path):
 return subprocess.check_output(['git', 'show', SOURCE + ':' + path], cwd=REPO)

def verifier(name, text):
 path = PROBE + name
 ns = {'__name__': 'pinned_verifier'}
 exec(compile(committed(path), path, 'exec'), ns)
 return ns['verify'](text)

def load():
 c = json.loads(read_pinned(BASE + 'checkpoint.json', CHECKPOINT))
 r = json.loads(read_pinned(FROZEN / 'source-release.json', RELEASE))
 require(c['schema'] == 'fortnite.button-base-click-runtime-checkpoint.v1' and c['native_commit'] == c['observed_repo_head'] == SOURCE, 'checkpoint identity')
 require(r['native_commit'] == SOURCE and r['checkpoint_sha256'] == CHECKPOINT and r['all_build_stage_probe_join_handles_terminal'] is True and r['active_native_build_or_stage_owner'] is None, 'release identity')
 rows = c['source_files']; require(len(rows) == len({x['path'] for x in rows}) == 116, 'source inventory')
 for row in rows:
  rel = Path(row['path']).relative_to(REPO).as_posix()
  require(row['revision'] == SOURCE and hashlib.sha256(committed(rel)).hexdigest() == row['sha256'], 'committed source mismatch: ' + rel)
  read_pinned(row['archive'], row['sha256'])
 return c

def validate(c):
 artifacts = {x['path']: x for x in c['artifacts']}
 require(len(artifacts) == len(c['artifacts']) == 680, 'artifact inventory')
 for x in artifacts.values(): read_pinned(x['archive'], x['sha256'])
 def artifact(path): return read_pinned(artifacts[path]['archive'], artifacts[path]['sha256'])
 def doc(path): return json.loads(artifact(path))
 builds = doc(BUILD + 'results.json')
 require(builds == c['builds'] and len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build inventory')
 for b in builds:
  require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['config_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0 and b['binary'] and b['binary']['size'] > 0, 'build ' + b['target'])
 stage = doc(BUILD + 'stage.json'); rows = stage['rows']
 require(rows == c['binary_rows'] and len(rows) == 18, 'stage')
 linux = next(b for b in builds if b['target'] == 'FortniteLinux')
 binary = next(r for r in rows if r['target'] == 'FortniteLinux' and r['role'] == 'stage' and Path(r['path']).name == 'FortniteLinux')
 require((binary['size'], binary['sha256']) == (linux['binary']['size'], linux['binary']['sha256']), 'staged client binary differs from build')
 last_build = max(b['end_ns'] for b in builds)
 def process(p):
  require(p['returncode'] == 0 and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['error'] is None and p['start_ns'] > last_build, 'process receipt')
  require(p['build'] == linux and p['binary'] == binary['path'] and p['argv'] == p['proc_argv'] and p['argv'][0] == p['binary'], 'process binds to the staged client build')
  return artifact(p['log']['path']).decode('utf-8', errors='replace')
 for key, (directory, flag, vp, count) in CASES.items():
  p = c[key]; prefix = BASE + directory + '/'
  require(p == doc(prefix + 'receipt.json') and p['passed'] is True and flag in p['argv'], key + ' receipt')
  result = verifier(vp, process(p))
  require(result == p['transcript'] == doc(prefix + 'verification.json') and result['passed'] is True and result['rows'] == count, key + ' transcript')
 require(c['text_transform']['transcript']['unicode_length_refusal_exercised'] is True, 'text transform refusal case')
 look = c['base_click_lookup']
 require(look['passed'] is True and look['debugger_inferior_exit_code'] == 0 and look['normal_probe_transcript']['rows'] == 14 and look['owner_matches_actual_world_gi'] is True and look['false_analytics_lookup_calls'] == 0, 'base click lookup')
 launches = c['retained_diagnostics']
 require(launches == doc(BASE + 'retained-native-1/launch-results.json') and len(launches) == 8, 'retained diagnostics')
 for launch in launches:
  prefix = BASE + 'retained-native-1/' + launch['case'] + '/'
  p = doc(prefix + 'receipt.json'); t = doc(prefix + 'retained-transcript-check.json')
  process(p); require(p['case'] == launch['case'] and p['passed'] is True and t['passed'] is True, 'retained ' + launch['case'])
 joins = c['joins']; require(joins == doc(BASE + 'joins.json') and len(joins) == 2 and {x['case'] for x in joins} == {'default', 'athena'}, 'joins')
 for j in joins:
  require(j['exit'] == 0 and j['end_ns'] > j['start_ns'] > last_build, 'join receipt')
  for x in j['logs'].values(): require(artifacts[x['path']]['sha256'] == x['sha256'], 'join log hash')
  require(b'Local UE join succeeded.' in artifact(j['logs']['result.txt']['path']), 'join result')

def check(): validate(load())

def entry(key, keys, scope):
 return {'id': key, 'result': 'pass', 'exit_code': 0,
  'command': 'python3 -I porting/progress/record_frontend_button_style_click_runtime.py',
  'checkpoint': BASE + 'checkpoint.json', 'checkpoint_sha256': CHECKPOINT,
  'release': str(FROZEN / 'source-release.json'), 'release_sha256': RELEASE,
  'verified_cases': {k: CASES[k][3] for k in keys}, 'scope': scope,
  'provenance_note': 'Pinned Git sources and frozen source and artifact pairs are rehashed; stage binary hashes are cross-checked, not independently rehashed.'}

def main():
 check()
 path = REPO / 'porting/progress/evidence.json'; ledger = json.loads(path.read_text())
 ledger['checks']['frontend_commonui_button_style_runtime'] = entry('frontend_commonui_button_style_runtime', STYLE,
  'Source-authenticated CommonButton text/text-block style, text-case layout, IsErebus, internal sizing and style rebuild diagnostics (190 controlled rows) on the staged client; no SetStyle registration, production producers, interactive UI or pixels.')
 ledger['checks']['frontend_commonui_button_click_runtime'] = entry('frontend_commonui_button_click_runtime', CLICK,
  'Actual CommonButton base click BP/native/dynamic callback ordering, weak receivers, reentry and conditional analytics lookup (14 rows plus a GDB lookup observation); no accepted-click routing, selection, focus or interactive UI.')
 path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__': main()
