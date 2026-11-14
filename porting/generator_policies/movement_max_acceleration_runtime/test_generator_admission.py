"""Exercise real generator model/render admission without writing production output."""
import copy
import importlib.util
import json
from pathlib import Path
import sys

PORTING = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PORTING))
def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result)
    return result

g = module('diagnostic_generator', PORTING / 'gen_native_classes.py')
p = module('movement_accel_policy', Path(__file__).with_name('source_policy.py'))
selector = json.loads((PORTING / 'generator_policies/air_selector_admission_runtime/manifest-entries.json').read_text())
proposal = json.loads(Path(__file__).with_name('manifest-entries.proposed.json').read_text())
load = g.load_behavior
def overlay(directory):
    entries = load(directory)
    for item in selector + proposal:
        key = (item['class'], item['function'])
        matches = [e for e in entries if (e['class'], e['function']) == key]
        if matches:
            metadata = [{k: v for k, v in e.items() if k not in ('body', 'reject')} for e in matches]
            if len(matches) != 1 or metadata[0] != item:
                raise ValueError('proposal collision: ' + repr(key))
            if item['kind'] == 'helper' and (matches[0]['body'], matches[0]['reject']) != g.read_snippet(item):
                raise ValueError('proposal body collision: ' + repr(key))
            continue
        e = dict(item)
        e['body'], e['reject'] = g.read_snippet(e) if e['kind'] == 'helper' else (None, None)
        entries.append(e)
    return entries
g.load_behavior = overlay
old_apply = g._pawn_aim_rotation_policy.apply
def apply(model):
    return p.apply(old_apply(model))
integrated = hasattr(g, '_movement_max_acceleration_policy')
if not integrated:
    g._pawn_aim_rotation_policy.apply = apply
finalize_owner = g._movement_max_acceleration_policy if integrated else g._athena_playlist_selection_policy
old_finalize = finalize_owner.finalize
seen = set()
def finalize(model, header, cpp):
    header, cpp = old_finalize(model, header, cpp)
    if not integrated:
        header, cpp = p.finalize(model, header, cpp)
    if model['name'] in ('AFortPawn', 'UFortMovementComp_Character'):
        mutations = []
        m = copy.deepcopy(model); m['parent'] = 'Broken'; mutations.append(m)
        fn = p.PAWN_HELPER if model['name'] == 'AFortPawn' else 'GetMaxAcceleration'
        m = copy.deepcopy(model); p.one_helper(m, fn)['body'] += '\nreturn;'; mutations.append(m)
        m = copy.deepcopy(model); p.one_helper(m, fn)['const'] = False; mutations.append(m)
        m = copy.deepcopy(model); m['helpers'].append(copy.deepcopy(p.one_helper(m, fn))); mutations.append(m)
        for m in mutations:
            try: p.apply(m)
            except ValueError: pass
            else: raise AssertionError('modified model was admitted')
        print('PASS real admission and negative guards:', model['name'], flush=True)
        out = PORTING / 'generated/agents/movement-max-acceleration7'
        (out / (model['name'] + '.h')).write_text(header)
        (out / (model['name'] + '.cpp')).write_text(cpp)
        seen.add(model['name'])
        if len(seen) == 2: raise SystemExit(0)
    return header, cpp
finalize_owner.finalize = finalize
r = PORTING / 'generated/reflection'; out = PORTING / 'generated/agents/movement-max-acceleration7/diagnostic-generation'
g.main(['--promote', '--reflection', str(r/'fortnite_game_reflection.json'), '--name-map', str(r/'class_name_map.json'), '--evidence', str(r/'class_name_map.evidence.json'), '--parent-map', str(r/'class_parent_map.json'), '--class-ref-map', str(r/'class_ref_map.json'), '--class-list', str(PORTING/'native_class_set.txt'), '--engine-classes', '/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes', '--source-root', str(out/'Source'), '--report-dir', str(out/'reports')])
raise AssertionError('did not see both target classes: ' + repr(seen))
