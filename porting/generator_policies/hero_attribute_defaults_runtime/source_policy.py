"""Validate the recovered Fort curve loader and hero legacy row queries."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ENTRIES = json.loads((Path(__file__).parent / 'manifest-entries.json').read_text())
PARENTS = {'UFortAbilitySystemGlobals': 'UAbilitySystemGlobals',
           'AFortGameState': 'AFortGameStateBase', 'AFortGameStateAthena': 'AFortGameStateZone',
           'UFortHeroType': 'UFortWorkerType'}


def apply(model):
    name = model.get('name')
    if name not in PARENTS:
        return model
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]:
        raise ValueError(name + ': hero attribute helper ancestry changed')
    for entry in (e for e in ENTRIES if e['class'] == name):
        function = entry['function']
        helpers = [h for h in model.get('helpers', []) if h.get('name') == function]
        competing = [h for key in ('functions', 'static_helpers', 'fields', 'members_decl_order')
                     for h in model.get(key, []) if h.get('name') == function]
        if len(helpers) != 1 or competing:
            raise ValueError(name + ': missing or competing ' + function + '; rejected=' +
                             repr(model.get('splice', {}).get('rejected', [])))
        h = helpers[0]
        if (h.get('virtual') != entry['virtual'] or h.get('override') != entry['override'] or
                h.get('body', '').strip() != (ROOT.parent / entry['inc_path']).read_text().strip() or
                model.get('splice', {}).get('accepted', []).count(name + '::' + function) != 1):
            raise ValueError(name + ': hero attribute helper contract changed for ' + function)
    if name in ('AFortGameState', 'AFortGameStateAthena', 'UFortHeroType'):
        model['includes'] = sorted(set(model.get('includes', [])) | {'FFortPlayerPawnStats.h'})
        model['forward_decls'] = [ref for ref in model.get('forward_decls', []) if ref != 'FFortPlayerPawnStats']
    if name in ('AFortGameStateAthena', 'UFortHeroType'):
        for include in (('UFortPlaylistAthena.h', 'FFortPlayerPawnStats.h') if name == 'AFortGameStateAthena' else ('FFortPlayerPawnStats.h',)):
            if include not in model.setdefault('cpp_includes', []):
                model['cpp_includes'].append(include)
    if name == 'UFortAbilitySystemGlobals':
        fields = [m for m in model.get('members_decl_order', [])
                  if m.get('name') == 'BROnlyAttributeSetDefaultsToExclude']
        if len(fields) != 1 or fields[0].get('decl', '').strip() != 'TArray<FName> BROnlyAttributeSetDefaultsToExclude;':
            raise ValueError(name + ': excluded table list changed')
        for include in ('FortAttributeSetInitter.h', 'FortAttributeDefaultsPolicy.h'):
            if include not in model.setdefault('cpp_includes', []):
                model['cpp_includes'].append(include)
    return model
