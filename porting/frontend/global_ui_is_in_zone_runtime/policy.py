"""Original controller-based IsInZone query; no replacement context state."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
NAME = 'IsInZone'
FLAG = 'global_ui_is_in_zone_runtime'
INCLUDES = ['Engine/LocalPlayer.h', 'AFortPlayerControllerZone.h', 'AFortPlayerControllerSpectating.h']


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['length'], int(w['va'], 16) - 0x140000000)
            if hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('IsInZone original instruction/type mismatch: ' + w['va'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns, native_execs=None):
    if model['name'] != 'UFortGlobalUIContext':
        return model
    if (model.get('package') != '/Script/FortniteUI' or model.get('parent') != 'UFortLocalPlayerSubsystem'
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())
            or CONTRACT['owner_fn'] not in owner_fns):
        raise ValueError('IsInZone class identity mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == NAME] != [CONTRACT['function']]:
        raise ValueError('IsInZone reflected signature mismatch')
    if not isinstance(native_execs, dict) or native_execs.get(NAME) != CONTRACT['native']:
        raise ValueError('IsInZone native identity mismatch')
    if model.get(FLAG) or any(f.get('name') == NAME for f in model['functions']):
        raise ValueError('IsInZone declaration collision')
    model['functions'].append({'name':NAME, 'rpc':False, 'service':None, 'ret':'bool', 'args':[],
        'static':False, 'const':True, 'notify':False, 'splice':False, 'blueprint_event':False,
        'access':'public', 'runtime_implementation':True,
        'specifiers':['BlueprintCallable', 'BlueprintPure'], 'used':{}})
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | set(INCLUDES))
    model['skipped'] = [s for s in model.get('skipped', []) if not
        (s.get('item') == 'function' and s.get('name') == NAME and s.get('category') == 'non_void_return')]
    model[FLAG] = True
    return model


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if not model.get(FLAG) or header.count('bool IsInZone() const;') != 1:
        raise ValueError('IsInZone declaration missing or duplicated')
    if cpp.count(definitions(model).strip()) != 1:
        raise ValueError('IsInZone definition missing or duplicated')
    if any(cpp.count('#include "' + inc + '"') != 1 for inc in INCLUDES):
        raise ValueError('IsInZone controller types unavailable')
