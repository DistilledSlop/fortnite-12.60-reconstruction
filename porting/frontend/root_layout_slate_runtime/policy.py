"""Native root Slate hierarchy and teardown, with original console controls."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'root_layout_slate_runtime'
FIELDS = [('NativePresentationBox2b8', 'SBox', 0x2b8),
          ('NativePresentationScale2c8', 'SScaleBox', 0x2c8),
          ('NativePresentationWidget2d8', 'SWidget', 0x2d8),
          ('NativeRootBox2e8', 'SBox', 0x2e8)]
INCLUDES = ['Widgets/SOverlay.h', 'Widgets/Layout/SBox.h', 'Widgets/Layout/SScaleBox.h',
            'UFortLetoLayoutBorder.h', 'Engine/LocalPlayer.h', 'HAL/IConsoleManager.h',
            'RootLayoutSlateSettings.h']


def validate_original(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('Original root Slate source mismatch: ' + w['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names):
    block = blocks.get(CONTRACT['identity']['block_va'])
    if (names.get('UFortRootViewportLayout') != CONTRACT['identity']['block_va'] or block is None
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('Original root class or property metadata mismatch')


def apply(model, block, owner_fns):
    if model.get('name') != 'UFortRootViewportLayout':
        return model
    if (model.get('parent') != 'UCommonUserWidget' or model.get('package') != '/Script/FortniteUI'
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())):
        raise ValueError('Root Slate owner mismatch')
    if [m.get('decl') for m in model['members_decl_order'] if m.get('name') == 'LetoLayoutBorderClass'] != ['TSubclassOf<UFortLetoLayoutBorder> LetoLayoutBorderClass;']:
        raise ValueError('Real Leto layout class unavailable')
    reserved = {name for name, _, _ in FIELDS} | {'RebuildWidget', 'ReleaseSlateResources'}
    if model.get(FLAG) or any(x.get('name') in reserved for group in
            ('fields', 'helpers', 'functions', 'members_decl_order') for x in model.get(group, [])):
        raise ValueError('Root Slate member collision')
    for name, typ, offset in FIELDS:
        model['fields'].append({'name':name, 'decl':f'TSharedPtr<{typ}> {name};',
                                'access':'protected', 'offset':offset})
    model['forward_decls'] = sorted(set(model['forward_decls']) | {'SBox', 'SScaleBox'})
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | set(INCLUDES))
    model[FLAG] = True
    return model


def declarations(model):
    if not model.get(FLAG):
        return ''
    return 'public:\n    virtual void ReleaseSlateResources(bool bReleaseChildren) override;\nprotected:\n    virtual TSharedRef<SWidget> RebuildWidget() override;\npublic:\n'


def definitions(model):
    if not model.get(FLAG):
        return ''
    return '\n'.join((HERE / name).read_text() for name in
                     ('settings.definitions.inc', 'root.rebuild.definitions.inc', 'root.release.definitions.inc'))


def validate_output(model, header, cpp):
    if not model.get(FLAG) or header.count(declarations(model).strip()) != 1:
        raise ValueError('Root Slate declarations unavailable')
    if any(header.count(f'TSharedPtr<{typ}> {name};') != 1 for name, typ, _ in FIELDS):
        raise ValueError('Root Slate retained fields unavailable or duplicated')
    if cpp.count(definitions(model).strip()) != 1:
        raise ValueError('Root Slate bodies/console registrations unavailable or duplicated')


def extra_headers():
    return {'RootLayoutSlateSettings.h': (HERE / 'RootLayoutSlateSettings.h').read_text()}
