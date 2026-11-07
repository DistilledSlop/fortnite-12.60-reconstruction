#!/usr/bin/env python3
"""Merge a proposal batch into the manifest and source-generate it, fail-closed (orchestrator-owned).

  python3 -I porting/run_native_batch.py ENTRIES.json TAG

1. Append the entries (skipping existing class/function keys) to porting/behavior/MANIFEST.json.
2. Run the promotion (gen_native_classes.py --promote, command saved in generated/gameplay-bulk-integration-3)
   into generated/<TAG>-N/. Source mtimes of unchanged files are restored.
3. Entries whose signature is a placeholder (@SIG@, @PARAMS@, leading 'T* ') are replaced by the generator's own
   reflection-derived signature (read from its rejection text) and the promotion is rerun (up to 3 rounds).
4. Prints accepted/rejected and every remaining rejection. It never drops entries itself: remove or record them
   (behavior/stub_bulk_runtime/blocked.json pattern) and rerun. Exit 0 only with zero rejections and no removals.
No builds or tests are run. Commit only exact paths afterwards.
"""
import hashlib, json, os, re, subprocess, sys
from pathlib import Path

R = Path(__file__).resolve().parents[1]
entries_path, tag = sys.argv[1], sys.argv[2]
MAN = R / 'porting/behavior/MANIFEST.json'
SRC = R / 'porting/native/Source'


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def promote(n):
    out = R / f'porting/generated/{tag}-{n}'
    out.mkdir(exist_ok=True)
    before = {str(p.relative_to(SRC)): (sha(p), p.stat().st_atime_ns, p.stat().st_mtime_ns) for p in SRC.rglob('*') if p.is_file()}
    args = json.loads((R / 'porting/generated/gameplay-bulk-integration-3/promotion.command.json').read_text())
    args[-1] = str(out / 'reports')
    with (out / 'promotion.log').open('w') as f:
        res = subprocess.run(args, cwd=R, stdout=f, stderr=subprocess.STDOUT)
    removed = []
    for k, (h, at, mt) in before.items():
        p = SRC / k
        if not p.exists():
            removed.append(k)
        elif sha(p) == h:
            os.utime(p, ns=(at, mt))
    sp = json.loads((out / 'reports/behavior_splice.json').read_text())
    return res.returncode, removed, sp


m = json.loads(MAN.read_text())
keys = {(e['class'], e['function']) for e in m}
new = [e for e in json.loads(Path(entries_path).read_text()) if (e['class'], e['function']) not in keys]
m.extend(new)
MAN.write_text(json.dumps(m, indent=2) + '\n')
print('appended', len(new), 'entries')
for rnd in range(1, 5):
    rc, removed, sp = promote(rnd)
    rej = sp['rejections']
    print(f'round {rnd}: accepted {sp["spliced"]} rejected {sp["rejected"]} rc={rc} removed={len(removed)}')
    learn = {}
    for r in rej:
        w = re.search(r"reflection-derived '(.*)'$", r['reason'])
        s = re.search(r"manifest signature '(.*?)' differs", r['reason'])
        if w and s and ('@PARAMS@' in s.group(1) or '@SIG@' in s.group(1) or s.group(1).startswith('T* ')):
            learn[r['entry']] = w.group(1)
    if not learn or rnd == 4:
        break
    for path in (MAN, Path(entries_path)):
        data = json.loads(path.read_text())
        for e in data:
            k = f"{e['class']}::{e['function']}"
            if k in learn:
                e['signature'] = learn[k]
        path.write_text(json.dumps(data, indent=2 if path == MAN else 1) + '\n')
    print('  learned', len(learn), 'signatures; rerunning')
for r in rej:
    print('REJECTED', r['entry'], '|', r['reason'][:160])
sys.exit(1 if rej or removed or rc else 0)
