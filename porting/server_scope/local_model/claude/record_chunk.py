#!/usr/bin/env python3
"""Record a finished haiku chunk: python3 -I work/claude/record_chunk.py NN  (run from the package dir).
Claims each function for owner claude-br, then records it proposed if out/<C>__<F>.inc and .evidence.json exist."""
import json, os, subprocess, sys
n = sys.argv[1]
P = ['python3', '-I', 'scripts/inventory_progress.py']
for e in json.load(open(f'work/claude/chunks/{n}.json')):
    fn = e['function']; c, f = fn.split('::')
    base = f'work/claude/out/{c}__{f}'
    if not (os.path.exists(base + '.inc') and os.path.exists(base + '.evidence.json')):
        print('MISSING', fn); continue
    r0 = subprocess.run(P + ['claim', '--owner', 'claude-br', '--class', c, '--function', fn], capture_output=True)
    if r0.returncode: subprocess.run(P + ['reopen', '--owner', 'claude-br', '--function', fn, '--reason', 'hand port'], capture_output=True)
    r = subprocess.run(P + ['record', '--owner', 'claude-br', '--function', fn, '--status', 'proposed',
                            '--evidence', base + '.evidence.json', '--inc', base + '.inc'], capture_output=True, text=True)
    print('ok' if r.returncode == 0 else 'FAIL ' + fn + ' ' + (r.stdout + r.stderr)[-200:])
