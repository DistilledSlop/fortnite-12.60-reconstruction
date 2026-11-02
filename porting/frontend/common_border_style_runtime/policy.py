"""Bounded original-image replay; this candidate is not generator-registered."""
import hashlib
import json
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())


def validate_original(image):
    with Path(image).open('rb') as stream:
        def read(address, length):
            stream.seek(address - 0x140000000)
            data = stream.read(length)
            if len(data) != length:
                raise ValueError('short original image read')
            return data
        for item in CONTRACT['windows']:
            start, end = int(item['start'], 0), int(item['end'], 0)
            if hashlib.sha256(read(start, end - start)).hexdigest() != item['sha256']:
                raise ValueError('original window mismatch: ' + item['label'])
        v = CONTRACT['vtable']
        if struct.unpack('<Q', read(int(v['table'], 0) + int(v['slot'], 0), 8))[0] != int(v['target'], 0):
            raise ValueError('CommonBorder sync virtual slot differs')
        name, target = struct.unpack('<QQ', read(int(CONTRACT['native_array'], 0), 16))
        if (name, target) != (int(CONTRACT['name'], 0), int(CONTRACT['native_exec'], 0)) or read(name, 9) != b'SetStyle\0':
            raise ValueError('CommonBorder native registration differs')

FLAG = 'common_border_style_runtime'
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_classes(blocks, names, native_funcs):
    for name, expected in CONTRACT['classes'].items():
        identity = expected['identity']
        block = blocks.get(identity['block_va'])
        if names.get(name) != identity['block_va'] or block is None or any(
                block.get(key) != value for key, value in identity.items()):
            raise ValueError('original border class identity differs: ' + name)
        wanted = {p['name'] for p in expected['properties'] if p.get('enum_underlying_of') is None}
        properties = (block.get('properties', []) if name.startswith('UCommon') else
                      [p for p in block.get('properties', []) if p.get('name') in wanted
                       or (name == 'UWidget' and p.get('enum_underlying_of') == 'Visibility')])
        if properties != expected['properties']:
            raise ValueError('original border properties differ: ' + name)
    block = blocks[CONTRACT['classes']['UCommonBorder']['identity']['block_va']]
    if [f for f in block.get('functions', []) if f.get('name') == 'SetStyle'] != [CONTRACT['function']]:
        raise ValueError('original border SetStyle reflection differs')
    if native_funcs.get('UCommonBorder', {}).get('SetStyle') != CONTRACT['native']:
        raise ValueError('original border SetStyle exec differs')


def validate_types(gate):
    for fn, kind, cpp, package, expected in (
        ('0x142fdb590', 'struct', 'FSlateBrush', '/Script/SlateCore', CONTRACT['brush_identity']),
        ('0x143dbad80', 'enum', 'ESlateVisibility', '/Script/UMG', CONTRACT['enums']['ESlateVisibility']),
        ('0x142fdac90', 'enum', 'ESlateBrushDrawType::Type', '/Script/SlateCore', CONTRACT['enums']['ESlateBrushDrawType::Type']),
    ):
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != kind or info.cpp_name != cpp
                or info.package != package or any(info.rec.get(k) != v for k, v in expected.items())):
            raise ValueError('original border type differs or unavailable: ' + cpp)
        if kind == 'struct' and [p for p in info.rec.get('properties', []) if p.get('name') == 'DrawAs'] != [CONTRACT['brush_drawas']]:
            raise ValueError('original SlateBrush DrawAs differs')


def validate_style_model(model):
    identity = CONTRACT['classes']['UCommonBorderStyle']['identity']
    if (model.get('name'), model.get('package'), model.get('parent'), model.get('block_va')) != (
            'UCommonBorderStyle', '/Script/CommonUI', 'UObject', identity['block_va']):
        raise ValueError('border style model identity differs')
    if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == 'Background'] != ['FSlateBrush Background;']:
        raise ValueError('border style Background declaration differs')


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonBorder':
        return model
    identity = CONTRACT['classes']['UCommonBorder']['identity']
    if ((model.get('package'), model.get('parent'), model.get('block_va')) !=
            ('/Script/CommonUI', 'UBorder', identity['block_va']) or
            any(block.get(k) != v for k, v in identity.items()) or
            block.get('properties') != CONTRACT['classes']['UCommonBorder']['properties'] or
            '0x140807730' not in owner_fns):
        raise ValueError('border model or original owner differs')
    for name, declaration in (('Style', 'TSubclassOf<UCommonBorderStyle> Style;'),
                              ('bReducePaddingBySafezone', 'bool bReducePaddingBySafezone;'),
                              ('MinimumPadding', 'FMargin MinimumPadding;')):
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [declaration]:
            raise ValueError('border member declaration differs: ' + name)
    reserved = ('SetStyle', 'SynchronizeProperties', 'NativeUpdateSafeZonePadding', 'UCommonBorder')
    if model.get(FLAG) or any(m.get('name') in reserved for group in GROUPS for m in model.get(group, [])):
        raise ValueError('border runtime declaration collision')
    model['functions'].append({'name': 'SetStyle', 'rpc': False, 'service': None, 'ret': 'void',
        'args': ['TSubclassOf<UCommonBorderStyle> InStyle'], 'const': False, 'notify': False,
        'splice': False, 'blueprint_event': False, 'access': 'public',
        'runtime_implementation': True, 'specifiers': ['BlueprintCallable'], 'used': {}})
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, source, style_model, style_header):
    if not model.get(FLAG):
        raise ValueError('border policy not applied')
    validate_style_model(style_model)
    if (header.count(declarations(model).strip()) != 1 or source.count(definitions(model).strip()) != 1
            or header.count('void SetStyle(TSubclassOf<UCommonBorderStyle> InStyle);') != 1
            or style_header.count('FSlateBrush Background;') != 1):
        raise ValueError('border rendered output differs')
    for token in ('UCommonBorder(const FObjectInitializer& ObjectInitializer);',
                  'virtual void SynchronizeProperties() override;', 'void NativeUpdateSafeZonePadding();'):
        if header.count(token) != 1:
            raise ValueError('border rendered declaration differs: ' + token)
