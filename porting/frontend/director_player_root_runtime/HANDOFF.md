# Bounded director/player-root handoff, 2026-11-02

Owned only this directory. No generator, Native, Engine, builds or commits were
changed. Candidate status: real source path recovered at this boundary;
promotion remains blocked on transitive native lifecycle/state dependencies.
All source method names are aliases. The README separates recovered executable
bodies from missing dependencies; policy.apply always fails closed.

Exact next method: **Startup root-added hook `145166f50`**, installed director
manager table `147011440 + 298`. Recover its state/input setters `14516e780`
and `145152480`, real root virtual `490`, player/root map `c0`, and GI multicast
`5d8`. Do not replace this hook with an empty method to permit root attachment.

Correction for the old native_ui_startup_evidence.md (root owns its edit):
reflection `14700d9a0` definitively binds **Entryway** path/class to **48/90**,
Athena to 60/98, **Campaign** to **78/a0**. The earlier prose reverses
Entryway/Campaign. Selection recovered in director.definitions.inc uses these
correct reflected names. Predicate `141f3ad70` is cached command-line `nomcp`,
not a guessed online/startup flag; its exact literal is `145bc3a00`.

Primary review additionally closed manager GetWorld14514f610 through the
director Outer -> UGameInstance cast14460bc60 -> direct Engine GetWorld144217e60.
manager-world.definitions.inc and five additional source/type windows/fourth
virtual target are included. Restore elided diagnostics before promoting the
attachment fragment. The old startup evidence ordering is now corrected.

Native storage blocker: current reflected FFortRootViewportLayoutInfo consists
of player/root pointers. Append its real unreflected attached bool at +10
(original stride18). Keep the two reflected pointers GC-visible. Original
base manager constructor false legacy default alone is insufficient to restore
its missing native state/delegates, so a piecemeal constructor was not supplied.

Verification command, run from repository root:

```bash
python3 - <<'PY'
import json
from porting.frontend.director_player_root_runtime import policy
policy.validate_original('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
r = json.load(open('porting/generated/reflection/fortnite_game_reflection.json'))
n = json.load(open('porting/generated/reflection/class_name_map.json'))
policy.validate_classes({b['block_va']: b for b in r['classes_in_groups_with_fort_names']}, {name: va for va, name in n.items()})
assert set(policy.fragments()['definitions']) == {'UFortUIDirector', 'UFortUIManager'}
try:
    policy.apply()
except ValueError:
    pass
else:
    raise AssertionError('incomplete lifecycle was permitted')
print('PASS original 19 windows, 4 slots, 5 class records; promotion fails closed')
PY
```

Outcome PASS. `git diff --check -- porting/frontend/director_player_root_runtime`
produced no diagnostics (files were untracked at handoff). No build/runtime was
run for these unregistered, dependency-incomplete source inputs. The parent's
batch4 runtime result does not validate these fragments.

Continue the user's planner/orchestrator/reviewer preference, bounded agent
rotation, economical capped reads, and Luna medium read-only / Terra medium or
high normal delegation / Sol medium complex delegation with fork_turns none.
