"""Fail-closed metadata gates for the unreflected controller brush lookup."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FIELDS = {
    'InputBrushDataMap': 'TArray<FCommonInputKeyBrushConfiguration> InputBrushDataMap;',
    'InputBrushKeySets': 'TArray<FCommonInputKeySetBrushConfiguration> InputBrushKeySets;',
}


def validate_classes(blocks, name_to_block):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if name_to_block.get('UCommonInputControllerData') != identity['block_va'] or block is None or any(
            block.get(k) != v for k, v in identity.items()):
        raise ValueError('original CommonInputControllerData identity mismatch')
    if [p for p in block.get('properties', []) if p.get('name') in FIELDS] != CONTRACT['properties']:
        raise ValueError('original controller brush arrays mismatch')


def validate_types(gate):
    for fn, expected in CONTRACT['structs'].items():
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != 'struct'
                or info.cpp_name != 'F' + expected['name']
                or info.package != '/Script/CommonInput' or info.rec != expected):
            raise ValueError('original brush configuration mismatch: ' + fn)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('original brush configuration unavailable: ' + reason)


def validate_model(model):
    if (model.get('name') != 'UCommonInputControllerData'
            or model.get('package') != '/Script/CommonInput'
            or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('controller data model identity mismatch')
    for name, declaration in FIELDS.items():
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == name]
        if len(members) != 1 or members[0].get('decl') != declaration:
            raise ValueError('controller data emitted array mismatch: ' + name)


def emit_runtime(source_root, generated_names):
    if 'UCommonInputControllerData' not in generated_names:
        return
    target = Path(source_root) / 'CommonInput' / 'Generated'
    for name in ('CommonInputControllerBrushLookup.h', 'CommonInputControllerBrushLookup.cpp'):
        (target / name).write_text((HERE / name).read_text())
