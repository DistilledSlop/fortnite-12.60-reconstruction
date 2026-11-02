"""Connected CommonLazyImage/Slate load guard recovery; no generated-tree writes."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_lazy_image_connected_runtime'
SOURCES = ('SLoadGuard.h', 'SLoadGuard.cpp')
FUNCTIONS = {
    'SetBrushFromLazyDisplayAsset': ('void', ['const TSoftObjectPtr<UObject>& LazyObject', 'bool bMatchTextureSize'], False, ['BlueprintCallable']),
    'IsLoading': ('bool', [], True, ['BlueprintCallable', 'BlueprintPure']),
    'SetMaterialTextureParamName': ('void', ['FName TextureParamName'], False, ['BlueprintCallable']),
}
INCLUDES = ('SLoadGuard.h', 'CommonUIModuleRuntime.h', 'UCommonUISettings.h',
            'Engine/Texture2D.h', 'Engine/Texture2DDynamic.h', 'Materials/MaterialInterface.h',
            'Materials/MaterialInstanceDynamic.h')


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('CommonLazyImage original source mismatch: ' + w['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names, native_execs=None):
    want = CONTRACT['identity']
    block = blocks.get(want['block_va'])
    if (names.get('UCommonLazyImage') != want['block_va'] or block is None
            or any(block.get(k) != v for k, v in want.items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('CommonLazyImage original class or fields mismatch')
    if native_execs is not None:
        owner = native_execs.get('UCommonLazyImage', native_execs)
        for name in FUNCTIONS:
            if owner.get(name) != CONTRACT['functions'][name]:
                raise ValueError('CommonLazyImage native exec signature mismatch: ' + name)


def apply(model, block, owner_fns, native_execs):
    if model.get('name') != 'UCommonLazyImage':
        return model
    if (model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UImage'
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())
            or block.get('properties') != CONTRACT['properties']
            or '0x14080d990' not in owner_fns):
        raise ValueError('CommonLazyImage source owner mismatch')
    for name in FUNCTIONS:
        if native_execs.get(name) != CONTRACT['functions'][name]:
            raise ValueError('CommonLazyImage native exec signature mismatch: ' + name)
    aliases = set(FUNCTIONS) | {'NativeSetTexture', 'NativeRestoreDefaultImage',
            'NativeLoadingStateChanged', 'NativeLoadGuard', 'NativeRebuildImage'}
    if model.get(FLAG) or any(x.get('name') in aliases for group in
            ('functions', 'fields', 'helpers', 'static_helpers', 'members_decl_order')
            for x in model.get(group, [])):
        raise ValueError('CommonLazyImage connected runtime collision')
    for name, (ret, args, const, specs) in FUNCTIONS.items():
        model['functions'].append(dict(name=name, ret=ret, args=args, const=const,
            rpc=False, service=None, notify=False, splice=False, blueprint_event=False,
            runtime_implementation=True, access='public', specifiers=specs, used={}))
    model['skipped'] = [x for x in model.get('skipped', [])
            if not (x.get('item') == 'function' and x.get('name') in FUNCTIONS)]
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | set(INCLUDES))
    model['forward_decls'] = sorted(set(model.get('forward_decls', [])) | {'SLoadGuard', 'UTexture'})
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def sources(blocks, names):
    validate_classes(blocks, names)
    return {name: (HERE / name).read_text() for name in SOURCES}


def validate_output(model, header, cpp):
    if (not model.get(FLAG) or header.count(declarations(model).strip()) != 1
            or cpp.count(definitions(model).strip()) != 1):
        raise ValueError('CommonLazyImage connected runtime missing or duplicated')
    for name in FUNCTIONS:
        if header.count(' ' + name + '(') != 1 or cpp.count('UCommonLazyImage::' + name + '(') != 1:
            raise ValueError('CommonLazyImage callable missing or duplicated: ' + name)
