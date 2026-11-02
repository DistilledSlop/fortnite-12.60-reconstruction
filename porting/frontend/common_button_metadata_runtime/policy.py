"""Real CommonButton Slate metadata, consumed by action-route eligibility."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_metadata_runtime'


def validate_source(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('CommonButton metadata original mismatch: ' + w['va'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonButton':
        return model
    if (model.get('package') != '/Script/CommonUI'
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())):
        raise ValueError('CommonButton metadata owner mismatch')
    if model.get(FLAG) or any(x.get('name') == 'OnWidgetRebuilt' for group in
            ('functions', 'helpers', 'fields') for x in model.get(group, [])):
        raise ValueError('CommonButton metadata lifecycle collision')
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {'CommonButtonMetaData.h', 'Widgets/SWidget.h'})
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if not model.get(FLAG) or header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1:
        raise ValueError('CommonButton metadata lifecycle missing or duplicated')


def sources():
    return {name:(HERE / name).read_text() for name in ('CommonButtonMetaData.h', 'CommonButtonMetaData.cpp')}
