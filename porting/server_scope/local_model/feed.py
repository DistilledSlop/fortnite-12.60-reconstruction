#!/usr/bin/env python3
"""Feed the local model (opencode + llama.cpp) with batches of server-required
native functions, one run at a time, and save whatever it finishes.

Order: fewest parameters first. Cheap functions go in batches; heavy ones one at a time.
Saved output (never the mutable package state) goes to porting/server_scope/local_model/proposals/.
Usage: feed.py PKG_DIR OPENCODE_CONFIG_JSON [--once] [--model llamacpp/NAME]
"""
import json, os, shutil, subprocess, sys, time, glob, re
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
OUT = os.path.join(HERE, 'proposals')
PKG, CFG = sys.argv[1], sys.argv[2]
ONCE = '--once' in sys.argv
MODEL = sys.argv[sys.argv.index('--model') + 1] if '--model' in sys.argv else 'llamacpp/qwen3.8-27b'
PROG = ['python3', '-I', os.path.join(PKG, 'scripts', 'inventory_progress.py')]
OWNER = 'q27'

def sh(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, cwd=kw.pop('cwd', PKG), **kw)

def state():
    return json.load(open(os.path.join(PKG, 'state', 'inventory-progress.json')))

def status_of(st, key):
    def find(o):
        if isinstance(o, dict):
            v = o.get(key)
            if isinstance(v, dict) and 'status' in v: return v
            for x in o.values():
                r = find(x)
                if r: return r
    r = find(st); return r['status'] if r else 'available'

def busy():
    return sh(['pgrep', '-f', '[o]pencode run --standalone']).stdout.strip() != ''

def pick(n_runs):
    q = json.load(open(os.path.join(HERE, 'queue.json')))
    inv = {f"{r['class']}::{r['function']}": r for r in json.load(open(os.path.join(PKG, 'inventory', 'remaining.json')))}
    st = state()
    rows = []
    for c, f in q:
        k = f'{c}::{f}'
        if k not in inv or status_of(st, k) != 'available': continue
        if os.path.exists(os.path.join(OUT, c, f + '.json')) or os.path.exists(os.path.join(OUT, c, f + '.inc')): continue
        np_ = len((inv[k].get('reflection') or {}).get('params') or [])
        rows.append((np_, c, f, k))
    rows.sort()
    if not rows: return None
    np0 = rows[0][0]
    size = 3 if np0 <= 1 else 2 if np0 == 2 else 1
    out, seen = [], {}
    for np_, c, f, k in rows:
        if (np_ <= 1) != (np0 <= 1) or (np_ >= 3) != (np0 >= 3): break
        if seen.get(c, 0) >= 3: continue
        out.append((c, k)); seen[c] = seen.get(c, 0) + 1
        if len(out) == size: break
    return out, np0

def run_batch(batch, np0, tag):
    keys = [k for _, k in batch]
    for c, k in batch:
        sh(PROG + ['claim', '--owner', OWNER + '-' + tag, '--class', c, '--function', k])
    wd = os.path.join(PKG, 'work', OWNER + '-' + tag); os.makedirs(wd, exist_ok=True)
    heavy = np0 >= 3
    text = (f"Bounded assignment (owner: {OWNER}-{tag}). Follow AGENTS.md in this directory.\n\n"
            f"Recover these native functions, all already claimed under owner {OWNER}-{tag}:\n{', '.join(keys)}.\n\n"
            f"For each function: read its original behavior with scripts/evidence.py and scripts/inventory_query.py show, follow the callees you need, "
            f"and write the whole-function body as a body-only .inc proposal under work/{OWNER}-{tag}/ with the evidence JSON beside it, exactly as AGENTS.md describes. "
            + ("This is one hard function: take the time to resolve its contracts. " if heavy else
               "Do not spend long on any one function; if it is not resolved within a few lookups, record it blocked and move on. ")
            + "Use the existing porting/behavior/*.inc files of other functions of the same class in the checkout as style references. Do not touch the main checkout.\n\n"
            "When a function is finished, record it with scripts/inventory_progress.py record (status proposed). When you cannot finish one because a contract is unresolved, "
            "record it as blocked with the address, the missing evidence, what you tried and a concrete next step. Never invent callees or returns. "
            "Never write raw pointer arithmetic, Stack/FFrame offsets, or vtable-slot casts in a body: if the exec entry is only the shared prologue plus a tail-call to a virtual, write the named _Implementation call (the declaration is in the generated header), and if the target cannot be named from evidence, record the function blocked instead. Keep your context small: never print a whole decompile or asm file, use sed -n ranges or head/tail with a few hundred lines at most, and record each function the moment it is finished instead of at the end. Stop after all are recorded. Finish with one line: proposed count, blocked count.\n")
    open(os.path.join(wd, 'ASSIGNMENT.txt'), 'w').write(text)
    env = dict(os.environ, OPENCODE_CONFIG_CONTENT=open(CFG).read())
    t0 = time.time()
    with open(os.path.join(wd, 'run.log'), 'w') as lg:
        subprocess.run(['timeout', '2400' if heavy else '1800', './scripts/sandbox.sh', os.path.expanduser('~/.opencode/bin/opencode'),
                        'run', '--standalone', '--auto', '-m', MODEL, text], cwd=PKG, env=env, stdout=lg, stderr=subprocess.STDOUT)
    return keys, wd, time.time() - t0

def harvest(keys, wd, secs, tag):
    """Copy everything the run produced (recorded or not) out of the package; release open claims."""
    st = state(); saved = []
    for k in keys:
        c, f = k.split('::')
        d = os.path.join(OUT, c); os.makedirs(d, exist_ok=True)
        stat = status_of(st, k)
        rec = None
        def find(o):
            if isinstance(o, dict):
                v = o.get(k)
                if isinstance(v, dict) and 'status' in v: return v
                for x in o.values():
                    r = find(x)
                    if r: return r
        rec = find(st) or {}
        inc = os.path.join(wd, f + '.inc'); ev = os.path.join(wd, f + '.evidence.json')
        if rec.get('inc_path') and os.path.exists(os.path.join(PKG, rec['inc_path'])): inc = os.path.join(PKG, rec['inc_path'])
        evl = [e for e in (rec.get('evidence') or []) if os.path.exists(os.path.join(PKG, e))]
        if evl: ev = os.path.join(PKG, evl[0])
        rejected = False
        if os.path.exists(inc):
            body = '\n'.join(l for l in open(inc).read().splitlines() if l.strip() and not l.strip().startswith('//'))
            rejected = bool(re.search(r'\(uint8\*\)|\(\*\(void|\bStack\b|reinterpret_cast', body))
            shutil.copy(inc, os.path.join(d, f + ('.inc.rejected' if rejected else '.inc')))
        if os.path.exists(ev): shutil.copy(ev, os.path.join(d, f + '.evidence.json'))
        if stat in ('proposed', 'blocked') or os.path.exists(inc):
            json.dump({'function': k, 'status': 'rejected_raw_memory' if rejected else stat, 'blocker': rec.get('blocker'), 'model': MODEL, 'run': tag,
                       'batch_seconds': round(secs)}, open(os.path.join(d, f + '.json'), 'w'), indent=1)
            saved.append(k)
        elif stat == 'claimed':
            sh(PROG + ['release', '--owner', OWNER + '-' + tag, '--function', k, '--reason', 'run ended before this function was recorded'])
    return saved

def commit(saved, tag):
    if not saved: return
    sh(['git', 'add', 'porting/server_scope/local_model/proposals'], cwd=REPO)
    msg = (f"Save local-model proposals for {len(saved)} server-required functions ({tag})\n\n"
           "Unverified body proposals and blocker records produced by the local model; not yet promoted.\n\n"
           "Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>\n")
    env = dict(os.environ, GIT_AUTHOR_NAME='Claude Sonnet 5.5', GIT_AUTHOR_EMAIL='noreply@anthropic.com',
               GIT_COMMITTER_NAME='Claude Sonnet 5.5', GIT_COMMITTER_EMAIL='noreply@anthropic.com')
    r = sh(['git', 'commit', '-m', msg, '--', 'porting/server_scope/local_model/proposals'], cwd=REPO, env=env)
    if r.returncode == 0:
        sh(['git', 'pull', '--rebase', '--autostash', 'origin', 'main'], cwd=REPO)
        sh(['git', 'push', 'origin', 'main'], cwd=REPO)
    print('commit', r.returncode, r.stdout[-120:], r.stderr[-200:], flush=True)

def main():
    n = 0
    while True:
        while busy(): time.sleep(30)
        p = pick(n)
        if not p: print('queue empty', flush=True); return
        batch, np0 = p; n += 1; tag = time.strftime('%m%d%H%M')
        print('batch', tag, np0, [k for _, k in batch], flush=True)
        keys, wd, secs = run_batch(batch, np0, tag)
        saved = harvest(keys, wd, secs, tag)
        print('done', tag, round(secs), 'saved', len(saved), flush=True)
        commit(saved, tag)
        if ONCE: return
        time.sleep(5)

if __name__ == '__main__': main()
