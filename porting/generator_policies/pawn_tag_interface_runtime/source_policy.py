"""Restore AFortPawn's original tag interface and Athena getter override."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
_VALIDATED = False
IDENTITIES = {'AFortPawn': '0x146484820', 'AFortPlayerPawnAthena': '0x1464cd530'}
BASE_METHODS = {'GetOwnedGameplayTags', 'HasMatchingGameplayTag',
                'HasAllMatchingGameplayTags', 'HasAnyMatchingGameplayTags'}


def validate():
    global _VALIDATED
    if _VALIDATED:
        return
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['source_image']).open('rb') as stream:
        for window in evidence['windows']:
            stream.seek(int(window['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('pawn tag/profile copy original evidence changed: ' + window['begin'])
    _VALIDATED = True


def apply(model, block, owner_fns):
    name = model['name']
    if name not in IDENTITIES:
        return model
    validate()
    expected = IDENTITIES[name]
    if model.get('block_va') != expected or block.get('block_va') != expected:
        raise ValueError('pawn tag interface original class identity changed')
    flag = 'pawn_tag_interface_runtime'
    if model.get(flag):
        raise ValueError('duplicate pawn tag interface policy')
    names = BASE_METHODS if name == 'AFortPawn' else {'GetOwnedGameplayTags'}
    for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'static_helpers'):
        if any(member.get('name') in names for member in model.get(group, [])):
            raise ValueError('pawn tag interface declaration collision: ' + group)
    for method in names:
        helpers = [helper for helper in model.get('helpers', []) if helper['name'] == method]
        if len(helpers) != 1 or not helpers[0].get('virtual') or not helpers[0].get('override') or not helpers[0].get('const'):
            raise ValueError('pawn tag interface missing concrete original override: ' + name + '::' + method)
    if name == 'AFortPawn':
        bases = model.setdefault('native_interface_bases', [])
        if 'IGameplayTagAssetInterface' in bases:
            raise ValueError('duplicate pawn tag interface base')
        bases.append('IGameplayTagAssetInterface')
        model['splice'].setdefault('header_includes', set()).add('GameplayTagAssetInterface.h')
    model[flag] = True
    return model


def sources():
    validate()
    return {'SharedFortPerkSystemLog.h': (HERE / 'SharedFortPerkSystemLog.h').read_text()}
