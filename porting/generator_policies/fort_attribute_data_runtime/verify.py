#!/usr/bin/env python3
"""Read-only evidence/guards and isolated original setter execution (no game process/build)."""
import ctypes
import hashlib
import importlib.util
import itertools
import json
import mmap
from pathlib import Path
import struct
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
OUT = REPO / 'porting/generated/agents/fort-attribute-data-runtime22'
spec = importlib.util.spec_from_file_location('attribute_policy', HERE / 'source_policy.py')
policy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(policy)


def val(bits):
    return struct.unpack('<f', struct.pack('<I', bits))[0]


def isnan(bits):
    return bits & 0x7fffffff > 0x7f800000


def different(a, b):
    return not isnan(a) and not isnan(b) and val(a) != val(b)


def expected(data, incoming, base):
    result = bytearray(data)
    old_offset, target_offset, flag = (28, 8, 25) if base else (32, 12, 24)
    old = struct.unpack_from('<I', result, old_offset)[0]
    if not different(incoming, old):
        return bytes(result)
    struct.pack_into('<I', result, old_offset, incoming)
    minimum, maximum = struct.unpack_from('<II', result, 16)
    clamped = incoming
    if not base or result[26]:
        if isnan(minimum) or val(incoming) < val(minimum):
            clamped = minimum
        elif not isnan(maximum) and val(maximum) < val(incoming):
            clamped = maximum
    struct.pack_into('<I', result, target_offset, clamped)
    result[flag] = int(different(clamped, incoming))
    return bytes(result)


def main():
    evidence = json.loads((HERE / 'evidence.json').read_text())
    with Path(evidence['image']).open('rb') as image:
        for item in evidence['functions']:
            image.seek(int(item['address'], 16) - int(evidence['image_base'], 16))
            data = image.read(item['length'])
            assert data.hex() == item['bytes']
            assert hashlib.sha256(data).hexdigest() == item['sha256']
        image.seek(int(evidence['native_vtable']['address'], 16) - int(evidence['image_base'], 16))
        assert [hex(n) for n in struct.unpack('<QQQ', image.read(24))] == evidence['native_vtable']['slots']
    record = next(r for r in json.loads((REPO / 'porting/generated/reflection/struct_blocks.json').read_text())['structs']
                  if r.get('name') == 'FortGameplayAttributeData')
    info = SimpleNamespace(rec=record, cpp_name='FFortGameplayAttributeData', kind='struct',
                           fn=policy.CONTRACT['fn'], package=policy.CONTRACT['package'])
    policy.validate(info)
    model = {'parent': 'FGameplayAttributeData', 'size': 40, 'align': 8,
             'members': [{'name': n, 'ctype': t, 'decl': '%s %s;' % (t, n), 'bitfield': False,
                          'kind': 'Bool' if t == 'bool' else 'Float', 'offset': None if t == 'bool' else o}
                         for n, t, o in policy.FIELDS]}
    policy.apply_struct(info, model)
    assert [m['offset'] for m in model['members']] == [f[2] for f in policy.FIELDS]
    for field in ('size', 'align', 'super_fn', 'properties', 'struct_ops'):
        changed = dict(record)
        changed[field] = None
        try:
            policy.validate(SimpleNamespace(**dict(vars(info), rec=changed)))
        except ValueError:
            pass
        else:
            raise AssertionError('accepted drift: ' + field)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'FFortGameplayAttributeData.proposal.h').write_text(policy.render_isolated(info))
    # Original leaf setters need no imports or globals. Adapt SysV (RDI, ESI) to original
    # MS ABI (RCX, raw binary32 XMM1); no float conversion can quiet NaN payloads.
    funcs, keepalive = {}, []
    for base, addr in [(False, '141bc4960'), (True, '141bc47e0')]:
        code = bytes.fromhex('4889f9660f6ece') + bytes.fromhex(next(
            f['bytes'] for f in evidence['functions'] if f['address'] == '0x' + addr))
        region = mmap.mmap(-1, len(code), prot=mmap.PROT_READ | mmap.PROT_WRITE | mmap.PROT_EXEC)
        region.write(code)
        pointer = ctypes.addressof(ctypes.c_char.from_buffer(region))
        funcs[base] = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_uint32)(pointer)
        keepalive.append(region)
    values = [0x00000000, 0x80000000, 0x3f800000, 0xbf800000, 0x42c80000,
              0x7f7fffff, 0xff7fffff, 0x7f800000, 0xff800000, 0x7fc12345, 0x7fa12345]
    bounds = [(0xff7fffff, 0x7f7fffff), (0, 0x42c80000), (0x42c80000, 0),
              (0x80000000, 0), (0, 0x80000000), (0x7fc12345, 0x42c80000),
              (0, 0x7fc12345), (0x7fa12345, 0x42c80000), (0, 0x7fa12345)]
    count = 0
    for base, clamp, incoming, old, (minimum, maximum) in itertools.product(
            [False, True], [False, True], values, values, bounds):
        data = bytearray([0xa5] * 40)
        struct.pack_into('<IIII', data, 8, 0x447a0000, 0xc47a0000, minimum, maximum)
        data[24:27] = bytes([1, 1, clamp])
        struct.pack_into('<II', data, 28, old, old)
        wanted = expected(data, incoming, base)
        buffer = ctypes.create_string_buffer(bytes(data), 40)
        funcs[base](buffer, incoming)
        assert buffer.raw == wanted, (base, clamp, hex(incoming), hex(old), minimum, maximum)
        count += 1
    from verify_compiled import run as run_compiled
    compiled = run_compiled(policy, OUT, funcs)
    result = {'compiled_variants': compiled['variants'], 'verified_function_bytes': len(evidence['functions']), 'differential_setter_cases': count,
              'reflection_guard_mutations_rejected': 5, 'game_build_or_runtime_mutation': False,
              'scope': 'original leaf bytes vs bit oracle and actual emitted C++ compiled with bundled clang9 in two optimized modes'}
    (OUT / 'verification.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
