"""Fail closed on the recovered source prerequisites; does not wire the gate."""
import json
from pathlib import Path
import re

HERE = Path(__file__).resolve().parent
ENTRIES = json.loads((HERE / 'manifest-entries.proposed.json').read_text())
PARENTS = {
    'UFortGameInstance': 'UGameInstance',
    'UFortRegisteredPlayerInfo': 'UObject',
    'UFortMcpProfileWorld': 'UFortMcpProfile',
    'UFortMcpProfile': 'UMcpProfile',
    'AFortGameMode': 'AFortGameModeBase',
    'AFortGameMode_InGame': 'AFortGameMode',
    'AFortGameModeZone': 'AFortGameMode_InGame',
    'AFortGamePvPBase': 'AFortGameModeZone',
    'AFortGameModeAthena': 'AFortGamePvPBase',
}
FIELDS = {
    'UFortGameInstance': ('TArray<UFortRegisteredPlayerInfo*> RegisteredPlayers;',),
    'UFortRegisteredPlayerInfo': ('bool bIsInitialPlayer;', 'UFortMcpProfileWorld* WorldProfile;'),
    'AFortGameMode': ('int32 TheaterSlot;',),
}
DECLARATIONS = {
    'UFortGameInstance': 'void NativeGatherWorldProfiles(TArray<UFortMcpProfileWorld*>& OutProfiles, bool bInitialPlayersOnly);',
    'AFortGameMode': 'virtual int32 NativeGetTheaterSlot();',
}

def validate_required_classes(class_names):
    missing = set(PARENTS) - set(class_names)
    if missing:
        raise ValueError('world profile helpers require classes: ' + ', '.join(sorted(missing)))

def body_for(entry):
    return (HERE / Path(entry['inc_path']).name).read_text().strip()

def validate_model(model):
    name = model['name']
    if name not in PARENTS:
        return
    if model.get('parent') != PARENTS[name] or model.get('package') != '/Script/FortniteGame':
        raise ValueError(name + ': world profile ancestry/package changed')
    for decl in FIELDS.get(name, ()):
        field = re.search(r'(\w+);$', decl).group(1)
        matches = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        if len(matches) != 1 or matches[0].get('decl', '').strip() != decl:
            raise ValueError(name + ': world profile field changed: ' + field)
    if name == 'AFortGameModeAthena':
        if any(f.get('name') == 'NativeGetTheaterSlot' for key in ('helpers', 'static_helpers', 'functions') for f in model.get(key, [])):
            raise ValueError(name + ': unexpected theater slot override')
    for entry in ENTRIES:
        if entry['class'] != name:
            continue
        function = entry['function']
        tag = name + '::' + function
        helpers = [h for h in model.get('helpers', []) if h.get('name') == function]
        if (len(helpers) != 1 or model.get('splice', {}).get('accepted', []).count(tag) != 1
            or any(r.get('entry') == tag for r in model.get('splice', {}).get('rejected', []))
            or any(f.get('name') == function for key in ('functions', 'static_helpers') for f in model.get(key, []))):
            raise ValueError(tag + ': missing/rejected/competing helper')
        h = helpers[0]
        signatures = {
            'NativeGatherWorldProfiles': ('void', ['TArray<UFortMcpProfileWorld*>& OutProfiles', 'bool bInitialPlayersOnly'], False),
            'NativeGetTheaterSlot': ('int32', [], True),
        }
        ret, args, virtual = signatures[function]
        if ((h.get('ret'), h.get('args'), h.get('const'), h.get('access')) != (ret, args, False, 'public')
            or h.get('kind') != 'helper' or h.get('static') or bool(h.get('virtual')) != virtual
            or h.get('override') or h.get('body', '').strip() != body_for(entry)):
            raise ValueError(tag + ': helper contract/body changed')

def compact(text):
    return re.sub(r'\s+', '', text)

def finalize(model, header, cpp):
    validate_model(model)
    name = model['name']
    if name not in DECLARATIONS:
        return header, cpp
    if compact(header).count(compact(DECLARATIONS[name])) != 1:
        raise ValueError(name + ': helper declaration missing/duplicated')
    entry = next(e for e in ENTRIES if e['class'] == name)
    matches = list(re.finditer(r'\b' + name + '::' + entry['function'] + r'\([^;{}]*\)\s*\{', cpp))
    if len(matches) != 1:
        raise ValueError(name + ': helper definition missing/duplicated')
    start = end = matches[0].end()
    depth = 1
    while end < len(cpp) and depth:
        depth += (cpp[end] == '{') - (cpp[end] == '}')
        end += 1
    if depth or compact(cpp[start:end-1]) != compact(body_for(entry)):
        raise ValueError(name + ': emitted helper body changed')
    for inc in ({'UFortGameInstance': ('UFortRegisteredPlayerInfo.h', 'UFortMcpProfileWorld.h')}.get(name, ())):
        if cpp.count('#include "' + inc + '"') != 1:
            raise ValueError(name + ': required include missing/duplicated: ' + inc)
    return header, cpp
