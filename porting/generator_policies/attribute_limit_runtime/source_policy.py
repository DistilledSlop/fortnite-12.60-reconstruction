"""Admit original Fort attribute minimum/maximum updates and owning ASC calls."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ENTRIES = json.loads((HERE / 'manifest-entries.json').read_text())


def apply(model):
    if model.get('name') != 'UFortAbilitySystemComponent':
        return model
    if model.get('parent') != 'UAbilitySystemComponent' or model.get('package') != '/Script/FortniteGame':
        raise ValueError('Fort attribute limit owner changed')
    for entry in ENTRIES:
        fn = entry['function']
        hs = [h for h in model.get('helpers', []) if h.get('name') == fn]
        if len(hs) != 1 or any(h.get('name') == fn for group in ('functions','static_helpers','fields','members_decl_order') for h in model.get(group, [])):
            raise ValueError('missing/competing attribute limit helper ' + fn + '; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
        h = hs[0]
        if ((h.get('ret'), h.get('args'), h.get('const'), h.get('virtual'), h.get('override')) != ('void', ['const FGameplayAttribute& Attribute', 'float Value'], False, False, False) or
                h.get('body','').strip() != (ROOT / entry['inc_path']).read_text().strip() or
                model.get('splice', {}).get('accepted', []).count(model['name'] + '::' + fn) != 1):
            raise ValueError('attribute limit helper changed: ' + fn)
    if 'FFortGameplayAttributeData.h' not in model.setdefault('cpp_includes', []):
        model['cpp_includes'].append('FFortGameplayAttributeData.h')
    return model
