#!/usr/bin/env python3
"""Refresh the server-first wiki page and treemap from the current recovery state.

Usage (repo root): python3 -I porting/progress/server_first/refresh_wiki.py /path/to/wiki/checkout
Re-renders server-first-treemap-v4.svg and rewrites the progress numbers in Server-First-Scope-Mockup.md and Project-Status.md.
Does not commit or push.
"""
import json, os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
WIKI = sys.argv[1]
SS = os.path.join(REPO, 'porting', 'server_scope'); LM = os.path.join(SS, 'local_model')
rows = [json.loads(l) for l in open(os.path.join(SS, 'functions.jsonl'))]
fp = {f"{r['c']}::{r['f']}" for r in rows if r['v'] == 'required' and r['conf'] in ('proven', 'strong', 'likely') and r['p']}
q = {f"{c}::{f}" for c, f in json.load(open(os.path.join(LM, 'queue.json')))}
cut = set(json.load(open(os.path.join(LM, 'claude', 'cut_not_br.json'))))
t = q - cut
prop = {x for x in t if os.path.exists(os.path.join(LM, 'claude', '%s__%s.inc' % tuple(x.split('::'))))}
F, T, P = len(fp), len(t), len(prop); done, total = F + P, F + T
out = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'render_server_first.py'), os.path.join(WIKI, 'server-first-treemap-v4.svg')],
                     capture_output=True, text=True, cwd=REPO).stdout.splitlines()[0].split()
overall = float(out[0])
grp = re.search(r"\('Gameplay classes[^']*', \d+, ([\d.]+)\)", subprocess.run([sys.executable, '-I', os.path.join(HERE, 'render_server_first.py'), os.devnull],
                capture_output=True, text=True, cwd=REPO).stdout)
gameplay = float(grp.group(1)) if grp else None
pc = lambda a, b: f"{100.0 * a / b:.1f}%"
p = os.path.join(WIKI, 'Server-First-Scope-Mockup.md'); s = open(p).read()
s = re.sub(r"\*\*Server function progress \([\d-]+\): [\d,]+ of [\d,]+ required server functions have a ported or proposed body \([\d.]+%\)\.\*\*",
           f"**Server function progress (2026-11-10): {done:,} of {total:,} required server functions have a ported or proposed body ({pc(done, total)}).**", s)
s = re.sub(r"\| Scope: [\d,]+ already ported \+ \d+ being recovered \| [\d,]+ \| 100% \|", f"| Scope: {F} already ported + {T} being recovered | {total:,} | 100% |", s)
s = re.sub(r"\| Ported before this work \(evidence-based required floor\) \| \d+ \| [\d.]+% \|", f"| Ported before this work (evidence-based required floor) | {F} | {pc(F, total)} |", s)
s = re.sub(r"\| Proposed source bodies in this recovery \(\d+ of \d+\) \| \d+ \| [\d.]+% \|", f"| Proposed source bodies in this recovery ({P} of {T}) | {P} | {pc(P, total)} |", s)
s = re.sub(r"\| \*\*Ported or proposed\*\* \| \*\*[\d,]+\*\* \| \*\*[\d.]+%\*\* \|", f"| **Ported or proposed** | **{done:,}** | **{pc(done, total)}** |", s)
s = re.sub(r"\| \*\*Proposed so far\*\* \| \*\*\d+ of \d+ \(\d+%\)\*\* \|", f"| **Proposed so far** | **{P} of {T} ({100 * P // T}%)** |", s)
s = re.sub(r"\| \*\*Server-first scope \(77 components\)\*\* \| \*\*1,470\*\* \| \*\*73%\*\* \| \*\*[\d.]+%\*\* \|", f"| **Server-first scope (77 components)** | **1,470** | **73%** | **{overall:.1f}%** |", s)
if gameplay: s = re.sub(r"\| Gameplay classes \| 645 \| [\d.]+% \|", f"| Gameplay classes | 645 | {gameplay:.1f}% |", s)
s = re.sub(r"lifts the gameplay group from 19\.2% to [\d.]+% and the server-first total from 33\.7% to [\d.]+%", f"lifts the gameplay group from 19.2% to {gameplay:.1f}% and the server-first total from 33.7% to {overall:.1f}%", s)
s = re.sub(r"\(\d+ ported plus \d+ in scope", f"({F} ported plus {T} in scope", s)
open(p, 'w').write(s)
p = os.path.join(WIKI, 'Project-Status.md'); s = open(p).read()
s = re.sub(r"[\d,]+ of [\d,]+ required server functions are ported or proposed \([\d.]+%: \d+ ported earlier plus \d+ proposed of the \d+ now being recovered;",
           f"{done:,} of {total:,} required server functions are ported or proposed ({pc(done, total)}: {F} ported earlier plus {P} proposed of the {T} now being recovered;", s)
open(p, 'w').write(s)
print(f"{done}/{total} {pc(done, total)}; proposed {P}/{T}; server-first {overall:.1f}%; gameplay {gameplay}")
