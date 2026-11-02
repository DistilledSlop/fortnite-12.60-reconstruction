"""Fail-closed gates for the original GlobalUIContext input-priority getter."""
import hashlib
import json
import os
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'global_ui_input_priority_runtime'
NAME = 'GetInputPriority'
DEFINITION = 'int32 UFortGlobalUIContext::GetInputPriority(EInputPriority Priority) const\n{\n    return static_cast<uint8>(Priority);\n}'


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            b = os.pread(fd, w['length'], int(w['va'], 16) - 0x140000000)
            if len(b) != w['length'] or hashlib.sha256(b).hexdigest() != w['sha256']:
                raise ValueError('original instruction mismatch: ' + w['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names, native_funcs, enums):
    identity = CONTRACT['identity']; va = identity['block_va']; block = blocks.get(va)
    if names.get('UFortGlobalUIContext') != va or not block or any(block.get(k) != v for k, v in identity.items()):
        raise ValueError('original class identity mismatch')
    if [x for x in block.get('functions', []) if x.get('name') == NAME] != [CONTRACT['function']]:
        raise ValueError('original function mismatch')
    if native_funcs.get('UFortGlobalUIContext', {}).get(NAME) != CONTRACT['native']:
        raise ValueError('original native mismatch')
    expected = CONTRACT['enum']
    related = [e for e in enums if e.get('name') == expected['name'] or e.get('block_va') == expected['block_va'] or set(e.get('construct_fns', [])) & set(expected['construct_fns'])]
    if related != [expected]:
        raise ValueError('original enum mismatch')


def validate_model(model):
    if (model.get('name') != 'UFortGlobalUIContext' or model.get('package') != '/Script/FortniteUI'
            or model.get('parent') != 'UFortLocalPlayerSubsystem' or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('class model mismatch')


def apply(model, block, owner_fns, native_execs=None):
    if model.get('name') != 'UFortGlobalUIContext': return model
    validate_model(model)
    if any(block.get(k) != v for k, v in CONTRACT['identity'].items()) or CONTRACT['owner_fn'] not in owner_fns:
        raise ValueError('original class owner mismatch')
    if [x for x in block.get('functions', []) if x.get('name') == NAME] != [CONTRACT['function']] or not isinstance(native_execs, dict) or native_execs.get(NAME) != CONTRACT['native']:
        raise ValueError('original getter mismatch')
    if model.get(FLAG) or any(x.get('name') == NAME for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers') for x in model.get(group, [])):
        raise ValueError('getter collision')
    model['functions'].append({'name': NAME, 'rpc': False, 'service': None, 'ret': 'int32', 'args': ['EInputPriority Priority'], 'static': False, 'const': True, 'notify': False, 'splice': False, 'blueprint_event': False, 'access': 'public', 'runtime_implementation': True, 'specifiers': ['BlueprintCallable', 'BlueprintPure'], 'used': {'0x1451ecbb0': {'Enum'}}})
    model.setdefault('used', {}).setdefault('0x1451ecbb0', set()).add('Enum')
    model['skipped'] = [s for s in model.get('skipped', []) if not
        (s.get('item') == 'function' and s.get('name') == NAME and s.get('category') == 'non_void_return')]
    model[FLAG] = True
    return model


def definitions(model):
    if not model.get(FLAG): return ''
    source = (HERE / 'definitions.inc').read_text()
    if source.strip() != DEFINITION: raise ValueError('getter source mismatch')
    return source


def validate_output(model, header, cpp, enum_header):
    validate_model(model)
    if not model.get(FLAG): raise ValueError('policy not applied')
    declaration = 'UFUNCTION(BlueprintCallable, BlueprintPure)\n\tint32 GetInputPriority(EInputPriority Priority) const;'
    if header.count(declaration) != 1 or len(re.findall(r'\bGetInputPriority\b', header)) != 1 or header.count('#include "EInputPriority.h"') != 1:
        raise ValueError('getter declaration or include mismatch')
    body = definitions(model).strip()
    if cpp.count(body) != 1 or len(re.findall(r'\bUFortGlobalUIContext::GetInputPriority\s*\(', cpp)) != 1:
        raise ValueError('getter definition mismatch')
    for e in CONTRACT['enum']['enumerators']:
        if not re.search(r'\b' + re.escape(e['name'].split('::')[-1]) + r'\s*=\s*' + str(e['value']) + r'\s*,', enum_header):
            raise ValueError('enum render mismatch')
    if not re.search(r'enum\s+class\s+EInputPriority\s*:\s*uint8\b', enum_header):
        raise ValueError('enum underlying type mismatch')
