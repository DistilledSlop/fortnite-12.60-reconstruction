"""Fail-closed source and schema gates for the UI configuration singleton."""
import json
import hashlib
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE.parent / 'startup_root_added_runtime' / 'contract.json').read_text())['classes']['UFortUIDataConfiguration']
FLAG = 'ui_config_singleton_runtime'
WINDOWS = {
    0x144db86e0: (1107, 'c9321a6985e6b03e25c54985a20b9b347eaf901cff06db7877fb4d08c36af1f1'),
    0x141ed8b19: (42, '909c37dbc2d49dca18c773d9639205bac465ff0710047dde1c40c70065daf234'),
    0x144db8955: (43, '80b1cf7d78a2d6355eb8ffb56ef54aff31009f88a32d31934cc44a0d2494e6b6'),
    0x1405dcde0: (50, '07ce5debb5cd465b152ecd8e705abb7867ba219f7a551432f66118d3e5ff89a8'),
    0x142c7627c: (35, '700c55297f8fd6e687f2c84a87fd6a2b53fa8bfd91ea2e46bfb28fc4c7d4ad21'),
    0x146d716a0: (20, 'afbcd26a89338953d01446cd534f23dc7c9e72b5489eeb5561c863d1b38d3c35'),
    0x1459a4c58: (10, 'e4606715453e2b532e700fcb9c4dde9e3180116f95ef9c661971c9b2b8e0045e'),
    0x141ed8a30: (400, 'f1762d0d615cd356cca17af045c47a4c5cdd39c135987db2f35c1eb5874bbb1c'),
}
IDENTITY = ('block_va', 'config_name', 'class_flags', 'dependency_fns', 'package_fn',
            'base_fn', 'class_noregister_fn', 'interface_count')
SOURCES = ('SharedFortUILog.h', 'SharedFortUILog.cpp', 'SingletonDependencies.h')
INCLUDES = {'SingletonDependencies.h', 'CoreGlobals.h', 'Misc/CommandLine.h',
            'Misc/Parse.h', 'Misc/ConfigCacheIni.h', 'UObject/Package.h',
            'UObject/UObjectGlobals.h'}


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for va, (size, expected) in WINDOWS.items():
            data = os.pread(fd, size, va - 0x140000000)
            if len(data) != size or hashlib.sha256(data).hexdigest() != expected:
                raise ValueError('UI config singleton source mismatch: ' + hex(va))
    finally:
        os.close(fd)


def validate_classes(blocks, names):
    va = CONTRACT['block_va']
    block = blocks.get(va)
    if (names.get('UFortUIDataConfiguration') != va or block is None
            or any(block.get(k) != CONTRACT[k] for k in IDENTITY)
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('UI config singleton original class or fields mismatch')


def apply(model, block):
    if model.get('name') != 'UFortUIDataConfiguration':
        return model
    if (model.get('package') != '/Script/FortniteUI' or model.get('parent') != 'UPrimaryDataAsset'
            or model.get('block_va') != CONTRACT['block_va']
            or any(block.get(k) != CONTRACT[k] for k in IDENTITY)
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('UI config singleton owner or schema mismatch')
    if model.get(FLAG):
        raise ValueError('UI config singleton policy already applied')
    if any(x.get('name') == 'NativeGet144db86e0' for group in
           ('functions', 'helpers', 'static_helpers', 'members_decl_order') for x in model.get(group, [])):
        raise ValueError('UI config singleton declaration collision')
    for member in ('DefaultUIDataConfigurationPath', 'BROnlyUIDataConfigurationPath', 'ErebusUIDataConfigurationPath'):
        if [x.get('decl') for x in model.get('members_decl_order', []) if x.get('name') == member] != ['FString ' + member + ';']:
            raise ValueError('UI config path field unavailable: ' + member)
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | INCLUDES)
    model[FLAG] = True
    return model


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def declarations(model):
    return '\npublic:\nstatic UFortUIDataConfiguration* NativeGet144db86e0();\n' if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if not model.get(FLAG): return
    if header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1:
        raise ValueError('UI config singleton fragment missing or duplicated')
    if any('#include "' + inc + '"' not in cpp for inc in INCLUDES):
        raise ValueError('UI config singleton include missing')


def sources(blocks, names):
    validate_classes(blocks, names)
    return {name: (HERE / name).read_text() for name in SOURCES}
