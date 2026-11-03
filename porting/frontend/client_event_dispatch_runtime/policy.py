"""Original ExecuteClientEvent, owned weak registry, and widget listener lifetime."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'client_event_dispatch_runtime'
EVENT_FN = '0x142b25280'
OWNERS = {'UFortGameInstance', 'UFortUserWidget'}
IDENTITY = ('block_va', 'class_noregister_fn', 'base_fn', 'package_fn', 'class_flags', 'interface_count')


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for window in CONTRACT['windows']:
            data = os.pread(fd, window['size'], int(window['va'], 16) - 0x140000000)
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('client event source differs: ' + window['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names, native_execs):
    for name, expected in CONTRACT['classes'].items():
        block = blocks.get(names.get(name))
        if (block is None or any(block.get(k) != expected.get(k) for k in IDENTITY)
                or block.get('properties') != expected['properties']):
            raise ValueError('client event class/schema mismatch: ' + name)
    block = blocks[names['UFortKismetLibrary']]
    if [f for f in block['functions'] if f['name'] == 'ExecuteClientEvent'] != [CONTRACT['function']]:
        raise ValueError('ExecuteClientEvent original signature differs')
    if native_execs.get('UFortKismetLibrary', {}).get('ExecuteClientEvent') != CONTRACT['native_exec']:
        raise ValueError('ExecuteClientEvent original native exec differs')


def validate_types(gate):
    info = gate.info(EVENT_FN)
    if (info is None or info.cpp_name != 'FFortClientEventName' or info.kind != 'struct'
            or info.rec != CONTRACT['event_type'] or gate.reason(info) is not None):
        raise ValueError('original client event name type unavailable')


def manual_declaration():
    return ('    UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext="WorldContextObject"))\n'
            '    static void ExecuteClientEvent(UObject* WorldContextObject, FFortClientEventName EventType, UObject* EventSource, UObject* EventFocus);')


def validate_manual(header, cpp):
    if header.count(manual_declaration()) != 1 or cpp.count((HERE / 'kismet.definitions.inc').read_text().strip()) != 1:
        raise ValueError('hand-written Kismet client-event signature/body missing or duplicated')


def apply(model, block, native_execs=None):
    name = model.get('name')
    if name not in OWNERS:
        return model
    expected = CONTRACT['classes'][name]
    if (model.get('block_va') != expected['block_va']
            or any(block.get(k) != expected.get(k) for k in IDENTITY)
            or block.get('properties') != expected['properties']):
        raise ValueError('client event model identity/schema mismatch: ' + name)
    if model.get(FLAG):
        raise ValueError('duplicate client event policy')
    forbidden = {'NativeClientEventListeners6c0', 'NativeExecuteClientEvent141e59c80',
                 'NativeRegisterClientEventObject141e78cb0', 'NativeUnregisterClientEventObject141e84a70'}
    if name == 'UFortKismetLibrary': forbidden.add('ExecuteClientEvent')
    if name == 'UFortUserWidget': forbidden.update({'NativeConstruct', 'NativeDestruct'})
    if any(x.get('name') in forbidden for group in ('functions', 'fields', 'helpers', 'members_decl_order')
           for x in model.get(group, [])):
        raise ValueError('client event declaration collision: ' + name)
    if name == 'UFortGameInstance':
        model.setdefault('fields', []).append(dict(name='NativeClientEventListeners6c0',
            decl='TMap<FName, TSet<TWeakObjectPtr<UObject>>> NativeClientEventListeners6c0;',
            access='private', offset=0x6c0))
    if name in ('UFortGameInstance', 'UFortKismetLibrary'):
        model.setdefault('used', {}).setdefault(EVENT_FN, set()).add('Struct')
    if name == 'UFortKismetLibrary':
        if not isinstance(native_execs, dict) or native_execs.get('ExecuteClientEvent') != CONTRACT['native_exec']:
            raise ValueError('ExecuteClientEvent owner exec mismatch')
        model['functions'].append(dict(name='ExecuteClientEvent', ret='void',
            args=['UObject* WorldContextObject', 'FFortClientEventName EventType', 'UObject* EventSource', 'UObject* EventFocus'],
            static=True, const=False, rpc=False, service=None, notify=False, splice=False,
            blueprint_event=False, runtime_implementation=True, access='public',
            specifiers=['BlueprintCallable', 'BlueprintCosmetic', 'meta=(WorldContext="WorldContextObject")'],
            used={EVENT_FN: {'Struct'}}))
        model['skipped'] = [x for x in model.get('skipped', []) if not
            (x.get('item') == 'function' and x.get('name') == 'ExecuteClientEvent')]
    if name == 'UFortUserWidget':
        if model.get('parent') != 'UUserWidget' or model.get('native_interface_bases'):
            raise ValueError('client-event widget parent/interface mismatch')
        model['native_interface_bases'] = ['IFortClientEventInterface']
        model['runtime_module_dependencies'] = sorted(set(model.get('runtime_module_dependencies', [])) | {'FortniteGame'})
    model[FLAG] = True
    return configure_model(model)


def configure_model(model):
    if not model.get(FLAG): return model
    name = model['name']
    headers = {'UFortGameInstance': {'FFortClientEventName.h'},
               'UFortUserWidget': {'UFortClientEventInterface.h'},
               'UFortKismetLibrary': set()}[name]
    model['includes'] = sorted(set(model.get('includes', [])) | headers)
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {
        'UFortGameInstance.h', 'UFortClientEventInterface.h', 'Engine/Engine.h',
        'Engine/World.h', 'UObject/UnrealType.h', 'SharedFortGameLog.h'})
    return model


def declarations(model):
    if not model.get(FLAG): return ''
    part = {'UFortGameInstance': 'game-instance', 'UFortUserWidget': 'widget'}.get(model['name'])
    return (HERE / (part + '.declarations.inc')).read_text() if part else ''


def definitions(model):
    if not model.get(FLAG): return ''
    part = {'UFortGameInstance': 'game-instance', 'UFortUserWidget': 'widget', 'UFortKismetLibrary': 'kismet'}[model['name']]
    return (HERE / (part + '.definitions.inc')).read_text()


def sources():
    return {name: (HERE / name).read_text() for name in ('SharedFortGameLog.h', 'SharedFortGameLog.cpp')}


def validate_output(model, header, cpp):
    if not model.get(FLAG): return
    decl = declarations(model).strip()
    if decl and header.count(decl) != 1:
        raise ValueError('client event declarations missing/duplicated')
    if cpp.count(definitions(model).strip()) != 1:
        raise ValueError('client event definitions missing/duplicated')
    if model['name'] == 'UFortUserWidget' and 'public IFortClientEventInterface' not in header:
        raise ValueError('real client event native interface not emitted')
