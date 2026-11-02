"""Bounded reflected InitializeModal splice and original evidence gates."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'progress_modal_initialize_runtime'

def validate_original(image):
    with Path(image).open('rb') as stream:
        for window in CONTRACT['windows']:
            start, end = int(window['start'], 0), int(window['end'], 0)
            stream.seek(start - 0x140000000)
            data = stream.read(end - start)
            if len(data) != end - start or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('original window mismatch: ' + window['label'])

def validate_classes(blocks, name_to_block, native_execs):
    for name, identity_key, props_key in (('UFortProgressModal', 'modal_identity', 'modal_properties'),
                                          ('UFortProgressWidget', 'widget_identity', 'widget_properties')):
        identity = CONTRACT[identity_key]
        block = blocks.get(identity['block_va'])
        if name_to_block.get(name) != identity['block_va'] or block is None or any(block.get(k) != v for k, v in identity.items()):
            raise ValueError(name + ' identity mismatch')
        expected = CONTRACT[props_key]
        names = {p['name'] for p in expected}
        if [p for p in block.get('properties', []) if p.get('name') in names] != expected:
            raise ValueError(name + ' property mismatch')
    modal = blocks[CONTRACT['modal_identity']['block_va']]
    if [f for f in modal.get('functions', []) if f.get('name') == 'InitializeModal'] != [CONTRACT['function']]:
        raise ValueError('InitializeModal reflection mismatch')
    if native_execs.get('UFortProgressModal', {}).get('InitializeModal') != CONTRACT['exec']:
        raise ValueError('InitializeModal exec mismatch')

def validate_model(model):
    if (model.get('name'), model.get('package'), model.get('block_va')) != ('UFortProgressModal', '/Script/FortniteUI', CONTRACT['modal_identity']['block_va']):
        raise ValueError('modal model mismatch')
    for name, decl in (('Title', 'FText Title;'), ('Description', 'FText Description;'), ('Icon', 'FSlateBrush Icon;'), ('ProgressWidget_Main', 'UFortProgressWidget* ProgressWidget_Main;')):
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [decl]:
            raise ValueError('modal member mismatch: ' + name)

def apply(model, block, owner_fns, native_execs=None):
    if model.get('name') != 'UFortProgressModal':
        return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['modal_identity']['block_va'] or '0x1452ad6f0' not in owner_fns:
        raise ValueError('modal owner mismatch')
    if model.get(FLAG) or any(item.get('name') == 'InitializeModal' for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers') for item in model.get(group, [])):
        raise ValueError('InitializeModal collision')
    model[FLAG] = True
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {'UFortProgressWidget.h', 'Components/Image.h'})
    return model

def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''

def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''

def validate_output(model, header, cpp):
    validate_model(model)
    if not model.get(FLAG) or header.count((HERE / 'declarations.inc').read_text().strip()) != 1 or cpp.count((HERE / 'definitions.inc').read_text().strip()) != 1:
        raise ValueError('InitializeModal output missing or duplicated')

if __name__ == '__main__':
    import sys
    validate_original(sys.argv[1])
    print('PASS: InitializeModal exec, body, and widget body windows')
