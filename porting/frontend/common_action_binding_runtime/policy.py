"""Guard exact source records before emitting the isolated binding data templates."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-types.json').read_text())


def validate_types(gate):
    for fn, expected in CONTRACT.items():
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != 'struct'
                or info.cpp_name != 'F' + expected['name']
                or info.package != '/Script/CommonUI' or info.rec != expected):
            raise ValueError('original binding data type mismatch: ' + fn)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('original binding data type unavailable: ' + fn + ': ' + reason)


def emit_runtime(source_root, generated_names):
    if 'UCommonUIInputSettings' not in generated_names:
        return
    target = Path(source_root) / 'CommonUI' / 'Generated'
    for name in ('CommonActionBindingData.h', 'CommonActionBindingData.cpp'):
        (target / name).write_text((HERE / name).read_text())
