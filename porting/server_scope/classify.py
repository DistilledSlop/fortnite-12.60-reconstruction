#!/usr/bin/env python3
"""Classify every in-scope native function by whether a dedicated server needs it.

Inputs (read-only): content refs from scan_content_refs.py, engine_parents.json from engine_parents.py,
porting/generated/reflection/{native_funcs,class_checksums}.json, progress/native_functions_inscope.json,
behavior/MANIFEST.json. Output: server_scope.json (summary) and functions.jsonl (one row per function).
Usage: python3 -I classify.py REFS.jsonl ENGINE_PARENTS.json OUT_DIR [--edges EDGES.json] [--no-prune]
With --edges (from callgraph.py) functions directly called, through helpers, from a required body become R5.
"""
import collections, json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
POR = os.path.dirname(HERE)
refs_path, eng_path, out_dir = sys.argv[1:4]
PRUNE = '--no-prune' not in sys.argv

ROOTS = ['/Game/Athena/Athena_GameMode', '/Game/Athena/Athena_GameState', '/Game/Athena/Athena_PlayerController',
         '/Game/Athena/PlayerPawn_Athena', '/Game/Athena/Apollo/Maps/Apollo_Terrain',
         '/Game/Athena/Playlists/Playlist_DefaultSolo', '/Game/Athena/Playlists/Playlist_DefaultDuo',
         '/Game/Athena/Playlists/Playlist_DefaultSquad']
# Engine classes a dedicated server never instantiates (no viewport, local player, Slate or HUD).
CLIENT_ONLY_ANCESTORS = {'UWidget', 'ULocalPlayerSubsystem', 'AHUD', 'UGameUserSettings', 'UCheatManager',
                         'USlateWidgetStyleAsset', 'ULocalPlayer', 'UGameViewportClient', 'UPlayerInput'}
# Packages whose natives only run when something is seen, heard or drawn.
PRUNE_PATHS = ('/Effects/', '/Sounds/', '/Audio/', '/HUD/', '/UI/', 'GameplayCue', '/Particles/', '/Niagara')
FLAG = dict(NetReliable=0x80, Net=0x40, NetMulticast=0x4000, NetServer=0x200000, NetClient=0x1000000,
            BlueprintAuthorityOnly=0x4, BlueprintCosmetic=0x8, BlueprintCallable=0x4000000, BlueprintEvent=0x8000000)

eng = json.load(open(eng_path))
cc = json.load(open(os.path.join(POR, 'generated/reflection/class_checksums.json')))['classes']
parent = {c['class']: c['parent'] for c in cc}
native_funcs = json.load(open(os.path.join(POR, 'generated/reflection/native_funcs.json')))
inscope = json.load(open(os.path.join(POR, 'progress/native_functions_inscope.json')))
manifest = json.load(open(os.path.join(POR, 'behavior/MANIFEST.json')))
ported = set()
for e in manifest:
    if e.get('status') == 'ported' and e.get('kind') != 'member_field':
        fn = e['function']
        for suffix in ('_Implementation',):
            if fn.endswith(suffix):
                fn = fn[:-len(suffix)]
        ported.add((e['class'], fn))


def chain(c):
    out = []
    while c and len(out) < 80:
        out.append(c)
        c = parent.get(c) or eng.get(c)
    return out


def client_only_class(c):
    return next((a for a in chain(c) if a in CLIENT_ONLY_ANCESTORS), None)


P = {}
for line in open(refs_path):
    r = json.loads(line)
    P[r['pkg']] = r


def package_is_client_only(pkg, r):
    if PRUNE and any(s in pkg for s in PRUNE_PATHS):
        return True
    for mod, cls in r.get('cls', []):
        for prefix in 'UA':
            if client_only_class(prefix + cls):
                return True
    return False


seen, queue, pruned = set(), [], 0
for root in ROOTS:
    if root in P:
        seen.add(root); queue.append(root)
while queue:
    p = queue.pop()
    r = P[p]
    for d in r['deps'] + r['soft']:
        if d in P and d not in seen:
            if package_is_client_only(d, P[d]):
                pruned += 1
                continue
            seen.add(d); queue.append(d)

bp_called = collections.defaultdict(set)   # class -> functions called from server-closure Blueprints
ref_classes = set()
for p in seen:
    r = P[p]
    for mod, cls, fn in r['nat']:
        bp_called[cls].add(fn); ref_classes.add(cls)
    for mod, cls in r.get('cls', []):
        ref_classes.add(cls)


def refd(c):
    return c[1:] in ref_classes

# classes instantiated on a server: referenced by closure content, plus their ancestors
server_classes = set()
for c in inscope:
    if refd(c):
        server_classes.update(a for a in chain(c))
for root in ('AFortGameModeAthena', 'AFortGameStateAthena', 'AFortPlayerControllerAthena', 'AFortPlayerStateAthena', 'AFortPlayerPawnAthena'):
    server_classes.update(chain(root))

reach5 = set()
if '--edges' in sys.argv:
    edges = json.load(open(sys.argv[sys.argv.index('--edges') + 1]))
    adj, indeg = collections.defaultdict(set), collections.Counter()
    for a, b in edges:
        adj[a].add(b); indeg[b] += 1
    thunks = json.load(open(os.path.join(POR, 'generated/thunk-table/final3.json')))
    body = collections.defaultdict(set)
    for r in thunks:
        for x in [r['thunk']] + [y for y in (r.get('real_bodies') or []) if isinstance(y, str)]:
            body[int(x, 16)].add((r['cls'], r['fn']))
    HUB = 300  # skip generic helpers called from this many places or more
    reach_roots = set()
    _fl = {}
    for cls, fns in inscope.items():
        for fn in fns:
            info = native_funcs.get(cls, {}).get(fn, {})
            flags = int(info.get('flags') or '0x0', 16) if info else 0
            _fl[(cls, fn)] = flags
    # first pass classification is repeated below; seeds are recomputed here with the same rules
    def seed(cls, fn):
        if client_only_class(cls):
            return False
        f = _fl[(cls, fn)]
        if f & FLAG['NetClient'] or f & FLAG['BlueprintCosmetic'] or fn.startswith('OnRep_'):
            return False
        inst = cls in server_classes
        return fn in bp_called.get(cls[1:], set()) or (inst and bool(f & FLAG['NetServer']))
    for a, names in body.items():
        if any(seed(c, f) for c, f in names if c in inscope and f in inscope[c]):
            reach_roots.add(a)
    seen_a, queue_a = set(reach_roots), list(reach_roots)
    while queue_a:
        x = queue_a.pop()
        for y in adj.get(x, ()):
            if y not in seen_a and indeg[y] < HUB:
                seen_a.add(y); queue_a.append(y)
    for a in seen_a - reach_roots:
        names = body.get(a, ())
        if len(names) == 1:   # shared stubs serve many functions and prove nothing about one of them
            reach5.update(names)

rows, tally = [], collections.Counter()
for cls, fns in inscope.items():
    co = client_only_class(cls)
    inst = cls in server_classes
    for fn in fns:
        info = native_funcs.get(cls, {}).get(fn, {})
        flags = int(info.get('flags') or '0x0', 16) if info else 0
        fl = {k for k, v in FLAG.items() if flags & v}
        called = fn in bp_called.get(cls[1:], set())
        if co:
            tier, why = 'N1', 'class derives from %s, which a dedicated server never instantiates' % co
        elif 'NetClient' in fl:
            tier, why = 'N2', 'Client RPC: body runs on the owning client; the server only sends'
        elif 'BlueprintCosmetic' in fl:
            tier, why = 'N3', 'declared BlueprintCosmetic: not run on dedicated servers'
        elif fn.startswith('OnRep_') and (cls, fn) not in reach5:
            tier, why = 'N4', 'RepNotify: runs on the client when the property arrives'
        elif called:
            tier, why = 'R1', 'called from Blueprint bytecode in the server closure'
        elif 'NetServer' in fl and inst:
            tier, why = 'R2', 'Server RPC on a class the server instantiates'
        elif 'BlueprintAuthorityOnly' in fl and inst:
            tier, why = 'L1', 'declared authority-only on a class the server instantiates (server-side, but nothing shows it is called)'
        elif 'NetMulticast' in fl and inst:
            tier, why = 'L2', 'Multicast on a class the server instantiates: runs on the server when it calls it'
        elif (cls, fn) in reach5:
            tier, why = 'R5', 'directly called, through helpers, from the body of a required function'
        elif inst:
            tier, why = 'U1', 'class instantiated by server content; no direct evidence for this function'
        else:
            tier, why = 'U2', 'class not referenced by any server-closure content'
        tally[tier] += 1
        rows.append(dict(c=cls, f=fn, t=tier, p=(cls, fn) in ported, fl=sorted(fl)))

os.makedirs(out_dir, exist_ok=True)
with open(os.path.join(out_dir, 'functions.jsonl'), 'w') as w:
    for r in rows:
        w.write(json.dumps(r, separators=(',', ':')) + '\n')
summ = dict(prune=PRUNE, closure_packages=len(seen), pruned_edges=pruned, total=len(rows),
            tiers={t: dict(n=tally[t], ported=sum(1 for r in rows if r['t'] == t and r['p'])) for t in sorted(tally)},
            server_classes=len(server_classes & set(inscope)), inscope_classes=len(inscope))
json.dump(summ, open(os.path.join(out_dir, 'server_scope%s.json' % ('' if PRUNE else '_noprune'), ), 'w'), indent=1)
print(json.dumps(summ, indent=1))
