"""CommonUI module and configured-resource lifecycle from original native code."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'commonui_module_settings_runtime'
INCLUDES = ['UObject/UObjectArray.h', 'UCommonUIRichTextData.h', 'Materials/MaterialInterface.h']


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('CommonUI original source mismatch: ' + w['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names):
    block = blocks.get(CONTRACT['identity']['block_va'])
    if (names.get('UCommonUISettings') != CONTRACT['identity']['block_va'] or block is None
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('CommonUISettings original class or fields mismatch')


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonUISettings':
        return model
    if (model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UObject'
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())):
        raise ValueError('CommonUISettings model identity mismatch')
    for name, decl in CONTRACT['members'].items():
        if [m.get('decl') for m in model['members_decl_order'] if m.get('name') == name] != [decl]:
            raise ValueError('CommonUISettings emitted member mismatch: ' + name)
    if model.get(FLAG) or any(x.get('name') in ('NativeDataLoaded', 'NativeInitializeData')
            for group in ('fields', 'helpers', 'functions', 'members_decl_order') for x in model.get(group, [])):
        raise ValueError('CommonUISettings runtime collision')
    model['fields'].append({'name':'NativeDataLoaded', 'decl':'bool NativeDataLoaded;',
                            'access':'private', 'offset':0xa8})
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | set(INCLUDES))
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'settings-declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'settings-definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if not model.get(FLAG) or header.count('bool NativeDataLoaded;') != 1:
        raise ValueError('CommonUISettings native loaded flag unavailable')
    if not (header.index('DefaultRichTextDataClass;') < header.index('bool NativeDataLoaded;')
            < header.index('DefaultImageResourceObjectInstance;')):
        raise ValueError('CommonUISettings native flag layout order mismatch')
    if header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1:
        raise ValueError('CommonUISettings runtime output missing or duplicated')


def sources(blocks, names):
    validate_classes(blocks, names)
    return {name:(HERE / name).read_text() for name in ('CommonUI.cpp', 'CommonUIModuleRuntime.h')}
