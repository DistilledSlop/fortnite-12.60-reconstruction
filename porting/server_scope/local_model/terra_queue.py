#!/usr/bin/env python3
"""Run terra (codex exec) tasks one after another and save what each finishes.

Usage: terra_queue.py PKG_DIR TASKS_DIR [--effort medium|high]
TASKS_DIR holds NN-name.json: {"owner": "terra-3", "functions": ["Class::Fn", ...], "brief": "text"}.
A task is skipped when its .done marker exists. After each task the claims are harvested
(follows each record's inc_path/evidence path) into proposals/, open claims are released, and
the proposals folder is committed and pushed. Never more than one codex exec at a time.
"""
import glob, json, os, subprocess, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.argv_saved = sys.argv[:]
PKG, TASKS = sys.argv[1], sys.argv[2]
EFFORT = sys.argv[sys.argv.index('--effort') + 1] if '--effort' in sys.argv else 'medium'
sys.argv = ['feed.py', PKG, '/dev/null']
sys.path.insert(0, HERE)
import feed  # reuses harvest/commit/state helpers
PROG = feed.PROG

def busy():
    return subprocess.run(['pgrep', '-f', '[c]odex exec -m gpt-6'], capture_output=True, text=True).stdout.strip() != ''

def prompt(task):
    fns = '\n'.join('  ' + f for f in task['functions'])
    return f"""You are working in {PKG}, a native-function recovery workspace for the Fortnite 12.60 reconstruction. Read AGENTS.md here first and follow it; it describes the tools (scripts/evidence.py, scripts/inventory_query.py, scripts/inventory_progress.py), the proposal format and where the evidence lives.

Context: the project is rebuilding the server side so the original client can run against it. These native functions are required for that and are claimed under owner {task['owner']}:
{fns}

{task['brief']}

Work method: explore as far as each function needs. Follow callees through the decompiles and captures, identify members, helpers and virtual slots, read the UE 4.25/4.26 engine headers in /opt/fortnite/UnrealEngine and /opt/fortnite/UnrealEngine4260, the Fortnite 14 research headers in /opt/fortnite/Fortnite14Research and the 23.20 SDK dump in /opt/fortnite/Fortnite-23.20-CL-23659353-SDK-Dump as naming references, and use porting/virtual_slots.py (run from the checkout root) to resolve vtable slots by address. The client is rebuilt from source, so declarations, signatures and override relationships must be right but byte-identical vtable offsets are not required. Write each whole-function body as a body-only .inc proposal under work/{task['owner']}/ with its evidence JSON beside it, put reusable helper, member or virtual declarations under work/dependencies/, and record each function with scripts/inventory_progress.py record (proposed). Dependency proposals count: recover the helper chain a body needs instead of stopping at it. Only record a function blocked as a last resort, after genuinely trying to resolve the gap, and then state precisely what is missing and the next step. Do not invent callees or return values, and write real source-level code rather than raw pointer arithmetic, Stack/FFrame offsets or vtable-slot casts. Do not build, and do not touch the main checkout or git. Keep tool output bounded (head/tail/ranges, not whole files).

Finish with a short report: for each function, proposed or blocked and the key finding, plus any shared dependency that other functions will need."""

def run(path):
    task = json.load(open(path)); owner = task['owner']
    wd = os.path.join(PKG, 'work', owner); os.makedirs(wd, exist_ok=True)
    for f in task['functions']:
        c = f.split('::')[0]
        st = feed.status_of(feed.state(), f)
        cmd = PROG + (['reopen', '--owner', owner, '--function', f, '--reason', 'queued for a stronger model'] if st in ('blocked', 'proposed')
                      else ['claim', '--owner', owner, '--class', c, '--function', f])
        feed.sh(cmd)
    text = prompt(task)
    open(os.path.join(wd, 'PROMPT.txt'), 'w').write(text)
    t0 = time.time()
    with open(os.path.join(wd, 'run.log'), 'w') as lg:
        subprocess.run(['codex', 'exec', '-m', 'gpt-6.1-terra', '-c', f'model_reasoning_effort="{task.get("effort", EFFORT)}"', '-C', PKG,
                        '--dangerously-bypass-approvals-and-sandbox', '-o', os.path.join(wd, 'final.txt'), text],
                       stdin=subprocess.DEVNULL, stdout=lg, stderr=subprocess.STDOUT)
    saved = feed.harvest(task['functions'], wd, time.time() - t0, owner)
    for dep in glob.glob(os.path.join(PKG, 'work', 'dependencies', '*')):
        dst = os.path.join(HERE, 'dependencies', os.path.basename(dep))
        if os.path.isdir(dep) and os.path.getmtime(dep) > t0:
            os.makedirs(dst, exist_ok=True)
            for f in glob.glob(os.path.join(dep, '*')):
                if os.path.isfile(f): subprocess.run(['cp', f, dst])
    feed.sh(['git', 'add', 'porting/server_scope/local_model'], cwd=feed.REPO)
    feed.commit(saved, owner, 'Co-Authored-By: Codex <codex@openai.com>\n')
    open(path + '.done', 'w').write(str(round(time.time() - t0)))

def main():
    for path in sorted(glob.glob(os.path.join(TASKS, '*.json'))):
        if os.path.exists(path + '.done'): continue
        while busy(): time.sleep(30)
        print('task', path, flush=True); run(path); print('done', path, flush=True)

if __name__ == '__main__': main()
