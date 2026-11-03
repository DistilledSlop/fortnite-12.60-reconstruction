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

def apply_signatures(name, block, functions, owner_fns, function_view):
    if name != 'UFortLegacySlateBridgeWidget':
        return
    declared = [f for f in block.get('functions', []) if f.get('name') == 'SetOnCloseHandler']
    if len(declared) != 1 or declared[0].get('create_func') != '0x14527dfa0':
        raise ValueError('legacy close delegate function identity changed')
    flags, params, problem = function_view(declared[0], owner_fns)
    if problem or flags != 0x4420401 or len(params) != 1 or params[0].get('property_flags') != '0x10000008000182':
        raise ValueError('legacy close delegate input contract changed')
    for fn in functions:
        if fn['name'] == 'SetOnCloseHandler':
            if fn['args'] != ['FOnCloseLegacyWidget& OnCloseHandler']:
                raise ValueError('legacy close delegate parameter changed')
            # UHT passes a delegate wrapper temporary. The native body only copies
            # the input into OnClose and never writes back through its reference.
            fn['args'] = ['const FOnCloseLegacyWidget& OnCloseHandler']

def configure_model(model):
    if model['name'] == 'AFortQuickBars':
        # Inline support dereferences AFortQuickBars and must follow its definition.
        model['includes'] = [i for i in model['includes'] if i != 'QuickBarSwapNative.h']
        model['cpp_includes'] = sorted(set(model['cpp_includes']) | {'QuickBarSwapNative.h'})
