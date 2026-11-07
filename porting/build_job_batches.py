#!/usr/bin/env python3
"""Turn a small-pool file into worker job batches with exact signatures (orchestrator-owned).

  python3 -I porting/build_job_batches.py POOL.json PREFIX BATCH_SIZE [SCRATCH_DIR]

For every pool row it derives the manifest entry kind/function (RPC -> impl, plain event -> native_event_impl,
both `<fn>_Implementation`; otherwise exec `<fn>`), probes the generator's reflection-derived signature by adding dummy
@SIG@ entries with _shared/return.inc through run_native_batch.py, reads the learned signatures back, then restores the
manifest copy and ONLY the generated sources the probe changed (never a directory-wide checkout). Writes
generated/work-batches/PREFIX1..N.json (jobs with header_decl, entry_function, entry_kind, static, packet).
Rows the generator rejects are listed in PREFIX_rejected.json (class not generated, policy class, ...).
"""
import json, os, subprocess, sys, shutil

pool_path, prefix, size = sys.argv[1], sys.argv[2], int(sys.argv[3])
scratch = sys.argv[4] if len(sys.argv) > 4 else 'porting/generated/work-batches'
os.makedirs(scratch, exist_ok=True)
pool = json.load(open(pool_path))
# functions a generator policy already emits (real body in the generated source) must not get a second entry: it collides
sys.path.insert(0, 'porting')
from generated_defs import defined_in_generated
policy_rows = [{'cls': j['cls'], 'fn': j['fn'], 'reason': 'policy_owned_function'} for j in pool if defined_in_generated(j['cls'], j['fn'])]
pool = [j for j in pool if not defined_in_generated(j['cls'], j['fn'])]
MAN = 'porting/behavior/MANIFEST.json'


def kind_of(j):
    f = int(j['flags'], 16)
    if f & 0x1c0:
        return 'impl', j['fn'] + '_Implementation'
    if f & 0x800 and (f & 0xc00) == 0xc00:
        return 'native_event_impl', j['fn'] + '_Implementation'
    return 'exec', j['fn']


def dirty(untracked=False):
    out = subprocess.run(['git', 'status', '--short', '--', 'porting/native/Source'], capture_output=True, text=True).stdout
    return {l[3:] for l in out.splitlines() if l.startswith('??') == untracked}


def access(f):
    return 'private' if f & 0x40000 else 'protected' if f & 0x80000 else 'public'


for j in pool:
    j['entry_kind'], j['entry_function'] = kind_of(j)
    j['static'] = bool(int(j['flags'], 16) & 0x2000)
    j['packet'] = f"porting/generated/work-packets/{j['body']}.md"
dummy = []
for j in pool:
    e = {'class': j['cls'], 'function': j['entry_function'], 'kind': j['entry_kind'], 'signature': '@SIG@',
         'signature_status': 'proposed', 'inc_path': 'porting/behavior/_shared/return.inc', 'confidence': 'high',
         'address': j['thunk'], 'status': 'ported', 'reflection_flags': j['flags'], 'static': j['static'],
         'access': access(int(j['flags'], 16)), 'native_body_address': j['body']}
    if j['entry_kind'] == 'exec' and j['group'] == 'C' and not j['static']:
        e['virtual'] = True
    dummy.append(e)
dpath = os.path.join(scratch, prefix + '_sigprobe.json')
json.dump(dummy, open(dpath, 'w'))
before_dirty = dirty()
before_new = dirty(True)
shutil.copy(MAN, os.path.join(scratch, prefix + '_manifest_backup.json'))
subprocess.run([sys.executable, '-I', 'porting/run_native_batch.py', dpath, prefix + 'probe'],
               stdout=open(os.path.join(scratch, prefix + '_probe.log'), 'w'), stderr=subprocess.STDOUT)
learned = {}
for e in json.load(open(MAN)):
    if e.get('inc_path') == 'porting/behavior/_shared/return.inc' and e.get('native_body_address'):
        learned[(e['class'], e['function'])] = e['signature']
shutil.copy(os.path.join(scratch, prefix + '_manifest_backup.json'), MAN)
changed = dirty() - before_dirty
if changed:
    subprocess.run(['git', 'checkout', '--'] + sorted(changed), check=True)  # exact paths the probe changed
for f in sorted(dirty(True) - before_new):  # new files the probe generated (exact paths)
    os.remove(f)
print('probe changed and restored', len(changed), 'generated files')
ok, rej = [], list(policy_rows)
for j in pool:
    sig = learned.get((j['cls'], j['entry_function']))
    if sig and '@' not in sig:
        j['header_decl'] = sig
        ok.append(j)
    else:
        rej.append({'cls': j['cls'], 'fn': j['fn'], 'reason': 'generator did not learn a signature'})
json.dump(rej, open(os.path.join(scratch, prefix + '_rejected.json'), 'w'), indent=1)
for i in range(0, len(ok), size):
    json.dump(ok[i:i + size], open(os.path.join(scratch, f'{prefix}{i // size + 1}.json'), 'w'))
print('jobs', len(ok), 'rejected', len(rej), 'batches', (len(ok) + size - 1) // size)
