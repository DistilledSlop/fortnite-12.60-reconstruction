"""Admit complete original persistable-definition and hero-level queries."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ENTRIES = json.loads((HERE / 'manifest-entries.json').read_text())
PARENTS = {'UFortHero': 'UFortWorker', 'UFortPersistableItem': 'UFortItem'}


def apply(model):
    name = model.get('name')
    if name not in PARENTS:
        return model
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]:
        raise ValueError(name + ': hero item ancestry changed')
    for entry in (e for e in ENTRIES if e['class'] == name):
        fn = entry['function']
        helpers = [h for h in model.get('helpers', []) if h.get('name') == fn]
        competing = [h for group in ('functions','static_helpers','fields','members_decl_order')
                     for h in model.get(group, []) if h.get('name') == fn]
        if len(helpers) != 1 or competing:
            raise ValueError(name + ': missing/competing ' + fn + '; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
        h = helpers[0]
        if (h.get('args') != [] or h.get('const') is not True or
                h.get('virtual') != entry['virtual'] or h.get('override') != entry['override'] or
                h.get('body','').strip() != (ROOT / entry['inc_path']).read_text().strip() or
                model.get('splice', {}).get('accepted', []).count(name + '::' + fn) != 1):
            raise ValueError(name + ': hero item contract changed for ' + fn)
    include = 'UFortHeroType.h' if name == 'UFortHero' else 'UFortPersistableItemDefinition.h'
    if include not in model.setdefault('cpp_includes', []):
        model['cpp_includes'].append(include)
    if name == 'UFortHero':
        fields = [f for f in model.get('fields', []) if f.get('name') == 'NativeRecoveredInt32_0x228']
        if len(fields) != 1 or fields[0].get('type') != 'int32':
            raise ValueError(name + ': original level clamp field changed: ' + repr(fields))
    return model
