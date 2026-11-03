"""Restore struct type exposure required by the original Blueprint-callable signatures.

The shipping property/struct tables retain layouts but omit editor BlueprintType
metadata. Keep the recovered callable signatures and make their known struct
identities available to UHT without changing their fields or property exposure.
"""
import hashlib
import json
from pathlib import Path

CONTRACTS = json.loads(Path(__file__).with_name('struct-contracts.json').read_text())
ROOTS = {'FFortDisplayAttribute', 'FFortConversionControlKeyCosts'}

def validate(info, expected):
    rec = info.rec
    digest = hashlib.sha256(json.dumps(rec['properties'], sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    if (info.kind != 'struct' or info.fn != expected['fn'] or info.package != expected['package']
            or rec.get('size') != expected['size'] or rec.get('align') != expected['align']
            or digest != expected['properties_sha256']):
        raise ValueError('checkpoint struct identity or layout differs: ' + info.cpp_name)

def apply_struct(info, model):
    expected = CONTRACTS.get(info.cpp_name)
    if expected is not None:
        validate(info, expected)
        model['blueprint_type'] = True
    return model

def add_roots(gate, seeds):
    for name in sorted(ROOTS):
        expected = CONTRACTS[name]
        info = gate.info(expected['fn'])
        if info is None or info.cpp_name != name:
            raise ValueError('missing native behavior struct: ' + name)
        validate(info, expected)
        if gate.reason(info) is not None:
            raise ValueError('unemittable native behavior struct: ' + name)
        seeds.setdefault(info.fn, set()).add('')
