"""Unregistered Initialize splice and bounded source/metadata gates."""
import hashlib
import json
import re
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_initialize_runtime'
HELPERS = ('Initialize', 'NativeClickMethod', 'NativeTouchMethod', 'NativePressMethod')
RECEIVERS = ('HandleButtonClicked', 'NativeHandleDoubleClicked', 'NativeHandleFocusReceived',
             'HandleButtonPressed', 'HandleButtonReleased')


def validate_original(image):
    """Read only pinned windows. The image's full provenance is an earlier receipt."""
    def read(address, size):
        with Path(image).open('rb') as stream:
            stream.seek(address - 0x140000000)
            data = stream.read(size)
        if len(data) != size:
            raise ValueError('short original image read')
        return data
    for window in CONTRACT['windows']:
        start, end = int(window['start'], 0), int(window['end'], 0)
        if hashlib.sha256(read(start, end-start)).hexdigest() != window['sha256']:
            raise ValueError('original window mismatch: ' + window['label'])
    for item in CONTRACT['vtables']:
        pointer = struct.unpack('<Q', read(int(item['table'], 0) + int(item['slot'], 0), 8))[0]
        if pointer != int(item['target'], 0):
            raise ValueError('original lifecycle vtable mismatch')
    for item in CONTRACT['strings']:
        expected = (item['text'] + '\0').encode('utf-16le')
        if read(int(item['address'], 0), len(expected)) != expected:
            raise ValueError('original Initialize string mismatch')


def validate_classes(blocks, name_to_block, delegates=None):
    expected = CONTRACT['identity']
    block = blocks.get(expected['block_va'])
    if name_to_block.get('UCommonButton') != expected['block_va'] or block is None:
        raise ValueError('CommonButton identity mismatch')
    if any(block.get(key) != value for key, value in expected.items()):
        raise ValueError('CommonButton original class metadata mismatch')

    for name, expected in CONTRACT['callbacks'].items():
        records = [f for f in block.get('functions', []) if f.get('name') == name]
        if len(records) != 1 or records[0].get('create_func') != expected['create_func']:
            raise ValueError('CommonButton callback metadata identity mismatch: ' + name)
        record = records[0]
        if 'candidates' in record:
            signatures = [s for s in record['candidates'] if s.get('outer_fn') == '0x140805cb0']
        else:
            signatures = [{k: record.get(k) for k in ('outer_fn', 'flags', 'params')}]
        if signatures != [expected['signature']]:
            raise ValueError('CommonButton callback signature mismatch: ' + name)


def validate_model(model):
    if (model.get('name'), model.get('package'), model.get('block_va')) != (
            'UCommonButton', '/Script/CommonUI', CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton Initialize model identity mismatch')


def apply(model, block, owner_fns, native_execs=None):
    if model.get('name') != 'UCommonButton':
        return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton Initialize owner mismatch')
    groups = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
    if model.get(FLAG) or any(item.get('name') in HELPERS for group in groups for item in model.get(group, [])):
        raise ValueError('CommonButton Initialize helper collision')
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, button_header, button_cpp):
    validate_model(model)
    if not model.get(FLAG):
        raise ValueError('CommonButton Initialize policy not applied')
    for name in HELPERS:
        if len(re.findall(r'\b' + name + r'\b', button_header)) != 1:
            raise ValueError('CommonButton Initialize declaration collision/omission: ' + name)
    if button_header.count((HERE / 'declarations.inc').read_text().strip()) != 1:
        raise ValueError('CommonButton Initialize declaration source mismatch')
    if button_header.count('virtual bool Initialize() override;') != 1:
        raise ValueError('CommonButton Initialize override missing')
    if button_cpp.count((HERE / 'definitions.inc').read_text().strip()) != 1:
        raise ValueError('CommonButton Initialize source mismatch')
    for name in RECEIVERS:
        result = 'FReply' if name == 'NativeHandleDoubleClicked' else 'void'
        if len(re.findall(r'\b' + result + r'\s+UCommonButton::' + name + r'\s*\(\s*\)', button_cpp)) != 1:
            raise ValueError('required CommonButton callback definition missing/duplicate: ' + name)
    if 'TWeakObjectPtr<UCommonButtonInternal> NativeInternalButton;' not in button_header:
        raise ValueError('internal weak ownership declaration missing')


if __name__ == '__main__':
    import sys
    validate_original(sys.argv[1])
    print('PASS: 8 bounded source windows, 21 lifecycle slots, 4 strings')
