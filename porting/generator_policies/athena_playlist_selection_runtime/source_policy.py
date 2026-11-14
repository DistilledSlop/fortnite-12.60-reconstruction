"""Emit the complete recovered base/Athena playlist selection virtual family.

No InitGame or playlist population is activated here. Admit manifest-entries.json
as a complete family, call validate_required_classes once for the full output set,
then finalize for every class model. Definitions use ordinary helper admission.
"""
from pathlib import Path
import re
import json

HERE = Path(__file__).resolve().parent
PARENTS = {
    'AFortGameMode': 'AFortGameModeBase',
    'AFortGameModeAthena': 'AFortGamePvPBase',
    'AFortGamePvPBase': 'AFortGameModeZone',
    'AFortGameModeZone': 'AFortGameMode_InGame',
    'AFortGameMode_InGame': 'AFortGameMode',
    'UFortGameInstance': 'UGameInstance',
    'UFortPlaylistManager': 'UObject',
    'UFortPlaylistAthena': 'UFortPlaylist',
    'UFortPlaylist': 'UPrimaryDataAsset',
}
FIELDS = {
    'AFortGameMode': ('int32 CurrentPlaylistId;', 'FName CurrentPlaylistName;'),
    'UFortGameInstance': ('UFortPlaylistManager* PlaylistManager;',),
    'UFortPlaylistManager': ('TArray<UFortPlaylistAthena*> AthenaPlaylists;',),
    'UFortPlaylist': ('int32 PlaylistId;', 'FName PlaylistName;'),
}
DECLARATIONS = {
    'AFortGameMode': (
        'virtual void SetCurrentPlaylistId(int32 PlaylistId);',
        'virtual void SetCurrentPlaylistName(FName PlaylistName);',
        'virtual void NativeSetCurrentPlaylistName(FName PlaylistName);',
        'UFortPlaylistManager* GetPlaylistManagerNative();',
    ),
    'AFortGameModeAthena': (
        'virtual void SetCurrentPlaylistId(int32 PlaylistId) override;',
        'virtual void SetCurrentPlaylistName(FName PlaylistName) override;',
    ),
    'UFortPlaylistManager': (
        'UFortPlaylistAthena* GetPlaylistByIdNative(int32 PlaylistId);',
        'UFortPlaylistAthena* GetPlaylistByNameNative(FName PlaylistName);',
    ),
}
INCLUDES = {
    'AFortGameMode': ('UFortGameInstance.h', 'UFortPlaylistManager.h', 'UFortPlaylistAthena.h', 'Engine/World.h'),
    'AFortGameModeAthena': ('UFortPlaylistManager.h', 'UFortPlaylistAthena.h'),
    'UFortPlaylistManager': ('UFortPlaylistAthena.h',),
}
NAMES = {owner: tuple(re.search(r'(\w+)\(', d).group(1) for d in decls)
         for owner, decls in DECLARATIONS.items()}


def validate_required_classes(class_names):
    missing = set(PARENTS) - set(class_names)
    if missing:
        raise ValueError('playlist selection requires generated classes: ' + ', '.join(sorted(missing)))


def validate_model(model):
    name = model['name']
    if name not in PARENTS:
        return
    if model.get('parent') != PARENTS[name] or model.get('package') != '/Script/FortniteGame':
        raise ValueError(name + ': playlist selection ancestry/package changed')
    members = model.get('members_decl_order', [])
    for declaration in FIELDS.get(name, ()):
        field = re.search(r'(\w+);$', declaration).group(1)
        matches = [m for m in members if m.get('name') == field]
        if len(matches) != 1 or matches[0].get('decl', '').strip() != declaration:
            raise ValueError(name + ': playlist selection field changed: ' + field)
    for entry in ENTRIES:
        if entry['class'] != name:
            continue
        function = entry['function']
        tag = name + '::' + function
        helpers = [h for h in model.get('helpers', []) if h.get('name') == function]
        if (len(helpers) != 1 or model.get('splice', {}).get('accepted', []).count(tag) != 1
                or any(r.get('entry') == tag for r in model.get('splice', {}).get('rejected', []))
                or any(f.get('name') == function for key in ('functions', 'static_helpers')
                       for f in model.get(key, []))):
            raise ValueError(tag + ': missing/rejected/competing playlist helper: ' + repr([r for r in model.get('splice', {}).get('rejected', []) if r.get('entry') == tag]))
        helper = helpers[0]
        match = re.fullmatch(r'(.*?) ' + function + r'\((.*?)\)', entry['signature'])
        expected_args = match.group(2).split(', ') if match.group(2) else []
        if ((helper.get('ret'), helper.get('args'), helper.get('const'), helper.get('access'))
                != (match.group(1), expected_args, False, 'public')
                or helper.get('kind') != 'helper' or helper.get('static')
                or bool(helper.get('virtual')) != entry.get('virtual', False)
                or bool(helper.get('override')) != entry.get('override', False)
                or helper.get('body', '').strip() != body_for(entry).strip()):
            raise ValueError(tag + ': playlist helper signature/body changed')


def body_for(entry):
    return (HERE / Path(entry['inc_path']).name).read_text()


ENTRIES = json.loads((HERE / 'manifest-entries.json').read_text())


def _compact(text):
    return re.sub(r'\s+', '', text)


def finalize(model, header, cpp):
    validate_model(model)
    name = model['name']
    if name not in DECLARATIONS:
        return header, cpp
    for declaration in DECLARATIONS[name]:
        if _compact(header).count(_compact(declaration)) != 1:
            raise ValueError(name + ': missing/duplicate playlist virtual declaration: ' + declaration)
    for entry in ENTRIES:
        if entry['class'] != name:
            continue
        function = entry['function']
        # The body must have been emitted by the regular helper path exactly once.
        pattern = r'\b' + name + '::' + function + r'\([^;{}]*\)\s*\{'
        definitions = list(re.finditer(pattern, cpp))
        if len(definitions) != 1:
            raise ValueError(name + ': missing/duplicate playlist helper definition: ' + function)
        match = definitions[0]
        start, end, depth = match.end(), match.end(), 1
        while end < len(cpp) and depth:
            depth += (cpp[end] == '{') - (cpp[end] == '}')
            end += 1
        if depth or _compact(cpp[start:end-1]) != _compact(body_for(entry)):
            raise ValueError(name + ': emitted playlist helper body changed: ' + function)
    own_include = '#include "' + name + '.h"'
    if cpp.count(own_include) != 1:
        raise ValueError(name + ': unexpected own include')
    for include in INCLUDES[name]:
        if cpp.count('#include "' + include + '"') != 1:
            raise ValueError(name + ': missing/duplicate playlist helper include: ' + include)
    if ('LogFortPlaylistSelection' in header
            or re.search(r'(?:DEFINE|DECLARE)_LOG_CATEGORY\w*\(LogFortPlaylistSelection', cpp)):
        raise ValueError(name + ': competing playlist log category')
    # All three implementation units include the manager header. Its pragma-once
    # declaration also avoids duplicate category types when UE merges unity TUs.
    if name == 'UFortPlaylistManager':
        header += '\nDECLARE_LOG_CATEGORY_EXTERN(LogFortPlaylistSelection, Log, All);\n'
    if name == 'AFortGameMode':
        include = '#include "UFortPlaylistManager.h"'
        cpp = cpp.replace(include, include + '\nDEFINE_LOG_CATEGORY(LogFortPlaylistSelection);', 1)
    return header, cpp
