#!/usr/bin/env python3
"""Bind original direct-vector TextureProperty output initialization to raw code."""
import argparse
import hashlib
import json
import mmap
from pathlib import Path
import struct

BASE = 0x140000000
IMAGE_SHA256 = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
LOOP_ADDRESS = 0x144353B10
LOOP = bytes.fromhex('488b46104c8d45080f57c0498bd50f114508498b0c06488b01ff50180f104508ffc34d8d76080f11074883c7103b5e187cce')


def verify_vector_loop(raw):
    # Match the entire loop, including its backwards branch. R8 receives the
    # address of a freshly zeroed float4 before every expression virtual call.
    if raw != LOOP:
        raise ValueError('original vector initialization/call/copy loop differs')
    return dict(output_argument='R8 = RBP+8', initialization='four positive float zeros before every call',
                call_slot=0x18, result_copy_bytes=16, output_stride_bytes=16,
                vector_array_offset=0x10, vector_count_offset=0x18,
                scope='Direct vector roots only; nested expression callers are not covered')


def inspect(image):
    def raw(address, size):
        offset = address - BASE
        if offset < 0 or size < 0 or offset + size > len(image):
            raise ValueError('original memory window outside image')
        return bytes(image[offset:offset + size])

    loop = verify_vector_loop(raw(LOOP_ADDRESS, len(LOOP)))
    stat_address = 0x146AC2670
    name = 'STAT_FUniformExpressionSet_FillUniformBuffer'
    if raw(stat_address, (len(name) + 1) * 2) != (name + '\0').encode('utf-16le'):
        raise ValueError('original caller stat identity differs')
    reference = 0x144353955
    instruction = raw(reference, 7)
    if instruction[:3] != b'\x48\x8d\x05' or reference + 7 + struct.unpack_from('<i', instruction, 3)[0] != stat_address:
        raise ValueError('original caller stat reference differs')
    vtable = 0x146ABFE78
    evaluator = 0x14433E4B0
    if struct.unpack('<Q', raw(vtable + loop['call_slot'], 8))[0] != evaluator:
        raise ValueError('TextureProperty evaluator vtable slot differs')
    evaluator_bytes = raw(evaluator, 234)
    if hashlib.sha256(evaluator_bytes).hexdigest() != 'dc45fbdcd4c222f64e41870d8e32be2a8d023a97865053f5fbc9472c827ba6ff':
        raise ValueError('original TextureProperty evaluator differs')
    # The PE exception directory contains memory RVAs. Follow chained unwind
    # metadata instead of treating a secondary .pdata entry as a new function.
    pe = struct.unpack_from('<I', image, 0x3c)[0]
    optional = pe + 24
    if struct.unpack_from('<H', image, optional)[0] != 0x20b:
        raise ValueError('expected original PE32+ image')
    table, size = struct.unpack_from('<II', image, optional + 112 + 3 * 8)
    matches = [struct.unpack_from('<III', image, at) for at in range(table, table + size, 12)
               if struct.unpack_from('<II', image, at)[0] <= reference - BASE < struct.unpack_from('<II', image, at)[1]]
    if len(matches) != 1:
        raise ValueError('ambiguous original caller runtime function')
    begin, end, unwind = matches[0]
    if (begin, end, unwind) != (0x435394a, 0x4354b53, 0x76b91d0):
        raise ValueError('original caller secondary runtime-function entry differs')
    header = raw(BASE + unwind, 4)
    if header[0] >> 3 != 4:
        raise ValueError('original caller is not a chained unwind entry')
    chain_at = unwind + 4 + ((header[2] + 1) & ~1) * 2
    chain = struct.unpack_from('<III', image, chain_at)
    if chain != (0x4353900, 0x435394a, 0x76b91b8):
        raise ValueError('original caller primary unwind entry differs')
    span = raw(BASE + chain[0], end - chain[0])
    if hashlib.sha256(span).hexdigest() != 'fbb13b0928a4ffaf1313b8014d944c5f0563f6d7a3c8c4ca216271ebeb9ed85f':
        raise ValueError('original caller chained code span differs')
    return dict(stat=name, stat_address=hex(stat_address), stat_reference=hex(reference),
                entry=hex(BASE + chain[0]), secondary_entry=hex(BASE + begin), end=hex(BASE + end),
                primary_unwind=hex(BASE + chain[2]), secondary_unwind=hex(BASE + unwind),
                loop_address=hex(LOOP_ADDRESS), loop=loop, texture_property_vtable=hex(vtable),
                texture_property_evaluator=hex(evaluator),
                direct_size_property_success_lanes=['float(resource width)', 'float(resource height)', '+0.0', '+0.0'],
                missing_texture_or_resource_lanes=['+0.0'] * 4), {
                    'caller-chained-span.bin': span, 'vector-loop.bin': raw(LOOP_ADDRESS, len(LOOP)),
                    'texture-property-evaluator.bin': evaluator_bytes, 'texture-property-vtable.bin': raw(vtable, 64),
                    'caller-primary-unwind.bin': raw(BASE + chain[2], 24),
                    'caller-secondary-unwind.bin': raw(BASE + unwind, 32),
                }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image', type=Path, default=GENERATED / 'dump/FortniteClient-Win64-Shipping.runtime.bin')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists():
            raise ValueError('choose a fresh output directory under porting/generated')
        with args.image.open('rb') as handle:
            if hashlib.file_digest(handle, 'sha256').hexdigest() != IMAGE_SHA256:
                raise ValueError('original memory image hash differs')
            handle.seek(0)
            with mmap.mmap(handle.fileno(), 0, access=mmap.ACCESS_READ) as image:
                result, windows = inspect(image)
        args.output.mkdir(parents=True)
        source = Path(__file__).read_bytes()
        windows['inspector.py'] = source
        outputs = []
        for name, data in windows.items():
            (args.output / name).write_bytes(data)
            outputs.append(dict(path=name, bytes=len(data), sha256=hashlib.sha256(data).hexdigest()))
        receipt = dict(schema='fortnite.original-texture-property-direct-vector-caller.v1',
                       original=dict(path=str(args.image.resolve()), sha256=IMAGE_SHA256, layout='VA=0x140000000+fileoffset'),
                       caller=result, outputs=outputs, runtime_claim=False,
                       scope='Original direct-vector caller initialization only. Actual asset texture identity, dimensions, resource existence and native runtime evaluation need separate evidence.')
        (args.output / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
        print(json.dumps(receipt))
    except (OSError, ValueError, IndexError, struct.error) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
