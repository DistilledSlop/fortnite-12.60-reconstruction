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
p = module('flying_accel_policy', Path(__file__).with_name('source_policy.py'))
selector = []
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
old_apply = g._movement_max_acceleration_policy.apply
def apply(model):
    return p.apply(old_apply(model))
integrated = False
if not integrated:
    g._movement_max_acceleration_policy.apply = apply
finalize_owner = g._aircraft_exit_bridge_policy
old_finalize = finalize_owner.finalize
seen = set()
def finalize(model, header, cpp):
    header, cpp = old_finalize(model, header, cpp)
    raw_header, raw_cpp = header, cpp
    if not integrated:
        header, cpp = p.finalize(model, header, cpp)
    if model['name'] in tuple(p.PARENTS):
        mutations = []
        m = copy.deepcopy(model); m['parent'] = 'Broken'; mutations.append(m)
        fn = 'GetMaxAcceleration' if model['name'] == 'UFortMovementComp_CharacterAthena' else p.FLY
        m = copy.deepcopy(model); p.one_helper(m, fn)['body'] += '\nreturn;'; mutations.append(m)
        m = copy.deepcopy(model); p.one_helper(m, fn)['const'] = False; mutations.append(m)
        m = copy.deepcopy(model); m['helpers'].append(copy.deepcopy(p.one_helper(m, fn))); mutations.append(m)
        for m in mutations:
            try: p.apply(m)
            except ValueError: pass
            else: raise AssertionError('modified model was admitted')
        if model['name'] == 'UFortMovementComp_CharacterAthena':
            # Final emission must reject conflicting state and constructor shapes.
            for bad_header, bad_cpp in ((raw_header + '\nfloat ' + p.SCALE + ';\n', raw_cpp),
                                        (raw_header, raw_cpp.replace('const FObjectInitializer& ObjectInitializer', 'int Unsupported', 1))):
                try: p.finalize(model, bad_header, bad_cpp)
                except ValueError: pass
                else: raise AssertionError('conflicting multiplier or constructor was admitted')
            m = copy.deepcopy(model)
            next(x for x in m['members_decl_order'] if x['name'] == 'MaxAccelerationFlying')['decl'] = 'double MaxAccelerationFlying;'
            try: p.apply(m)
            except ValueError: pass
            else: raise AssertionError('altered reflected acceleration field admitted')
        print('PASS real admission and negative guards:', model['name'], flush=True)
        out = PORTING / 'generated/agents/flying-acceleration-family16'
        (out / (model['name'] + '.h')).write_text(header)
        (out / (model['name'] + '.cpp')).write_text(cpp)
        seen.add(model['name'])
        if len(seen) == len(p.PARENTS): raise SystemExit(0)
    return header, cpp
finalize_owner.finalize = finalize
r = PORTING / 'generated/reflection'; out = PORTING / 'generated/agents/flying-acceleration-family16/diagnostic-generation'
g.main(['--promote', '--reflection', str(r/'fortnite_game_reflection.json'), '--name-map', str(r/'class_name_map.json'), '--evidence', str(r/'class_name_map.evidence.json'), '--parent-map', str(r/'class_parent_map.json'), '--class-ref-map', str(r/'class_ref_map.json'), '--class-list', str(PORTING/'native_class_set.txt'), '--engine-classes', '/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes', '--source-root', str(out/'Source'), '--report-dir', str(out/'reports')])
raise AssertionError('did not see all target classes: ' + repr(seen))
