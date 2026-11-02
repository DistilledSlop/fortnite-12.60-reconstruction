"""Original class/struct defaults consumed by the connected analog cursor."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'settings-original-policy.json').read_text())
FLAG = 'common_analog_settings_runtime'


def validate_source(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in CONTRACT['windows']:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('Analog settings constructor source mismatch')
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonUIInputSettings':
        return model
    if model.get('parent') != 'UObject' or model.get('package') != '/Script/CommonUI' or block != CONTRACT['class']:
        raise ValueError('Analog settings class mismatch')
    if model.get(FLAG):
        raise ValueError('Analog settings constructor collision')
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'UCommonUIInputSettings.members.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'UCommonUIInputSettings.definitions.inc').read_text() if model.get(FLAG) else ''


def struct_declarations(model):
    if model.get('cpp_name') != 'FCommonAnalogCursorSettings':
        return []
    return (HERE / 'FCommonAnalogCursorSettings.members.inc').read_text().splitlines()


def validate_types(gate):
    info = gate.info(CONTRACT['struct']['construct_fns'][0])
    if info is None or info.rec != CONTRACT['struct']:
        raise ValueError('Analog settings struct mismatch')


def validate_output(model, header, cpp):
    if model.get(FLAG) and (header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1):
        raise ValueError('Analog settings constructor missing or duplicated')
