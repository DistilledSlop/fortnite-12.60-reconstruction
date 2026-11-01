#!/usr/bin/env python3
"""Prove the original Splash float-channel framing without executing game code."""
import argparse
import bisect
import hashlib
import json
import mmap
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x140000000
DUMP_SHA256 = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
PACKAGE = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Login/SplashScreen/SplashScreenWidget')


def sha(path):
    with Path(path).open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def unpack(fmt, data, position, end):
    if position < 0 or position + struct.calcsize(fmt) > min(end, len(data)):
        raise ValueError('read escapes declared boundary')
    return struct.unpack_from(fmt, data, position)


def fname(data, position, end, names):
    index, number = unpack('<ii', data, position, end)
    if not 0 <= index < len(names) or number != 0:
        raise ValueError('unknown or numbered property name')
    return names[index]


def tag(data, position, end, names):
    name = fname(data, position, end, names)
    if name == 'None':
        return dict(name=name, next=position + 8)
    kind = fname(data, position + 8, end, names)
    size, index = unpack('<ii', data, position + 16, end)
    if size < 0 or index != 0:
        raise ValueError('invalid property size or array index')
    cursor = position + 24
    extra = None
    if kind == 'StructProperty':
        extra = fname(data, cursor, end, names)
        unpack('<16s', data, cursor + 8, end)
        cursor += 24
    elif kind == 'ArrayProperty':
        extra = fname(data, cursor, end, names)
        cursor += 8
    else:
        raise ValueError('unsupported property in bounded channel proof')
    has_guid, = unpack('<B', data, cursor, end)
    if has_guid not in (0, 1):
        raise ValueError('invalid property GUID flag')
    cursor += 1
    if has_guid:
        unpack('<16s', data, cursor, end)
        cursor += 16
    if cursor + size > end:
        raise ValueError('property payload escapes declared boundary')
    return dict(name=name, kind=kind, extra=extra, size=size,
                data=cursor, next=cursor + size)


def channel(data, position, end, names):
    """The observed channel has Times, Values and None; retain every key byte."""
    arrays = {}
    while position < end:
        item = tag(data, position, end, names)
        position = item['next']
        if item['name'] == 'None':
            if position != end or set(arrays) != {'Times', 'Values'}:
                raise ValueError('incomplete channel or bytes after None')
            if arrays['Times']['count'] != arrays['Values']['count']:
                raise ValueError('time/value key counts differ')
            return arrays
        if item['name'] not in ('Times', 'Values') or item['name'] in arrays:
            raise ValueError('unexpected or duplicate channel property')
        if item['kind'] != 'ArrayProperty' or item['extra'] != 'StructProperty':
            raise ValueError('expected tagged struct array')
        count, = unpack('<i', data, item['data'], item['next'])
        inner = tag(data, item['data'] + 4, item['next'], names)
        expected = 'FrameNumber' if item['name'] == 'Times' else 'MovieSceneFloatValue'
        stride = 4 if item['name'] == 'Times' else 23
        if (count < 0 or inner.get('kind') != 'StructProperty' or inner['name'] != item['name']
                or inner['extra'] != expected or inner['size'] != count * stride
                or inner['next'] != item['next']):
            raise ValueError('struct-array count/size/framing mismatch')
        raw = data[inner['data']:inner['next']]
        values = [unpack('<i', raw, i * stride, len(raw))[0] for i in range(count)] if stride == 4 else [
            dict(zip(('value', 'interp_mode', 'tangent_mode', 'arrive_tangent',
                      'leave_tangent', 'weight_mode', 'arrive_weight', 'leave_weight'),
                     unpack('<fBBffBff', raw, i * stride, len(raw)))) for i in range(count)]
        arrays[item['name']] = dict(count=count, stride=stride, raw_hex=raw.hex(), values=values)
    raise ValueError('missing channel terminator')


def inspect_package(header, payload):
    if unpack('<6i', header, 0, len(header)) != (-1641380927, -7, 0, 0, 0, 0):
        raise ValueError('expected original unversioned summary without custom versions')
    header_size, length = unpack('<ii', header, 24, len(header))
    if header_size != len(header) or length < 1 or 32 + length > len(header):
        raise ValueError('invalid header boundary/folder string')
    summary = 32 + length
    flags, count, name_offset = unpack('<Iii', header, summary, len(header))
    if not flags & 0x80000000 or not 0 < count < 100000:
        raise ValueError('expected filtered package with bounded name table')
    names, cursor = [], name_offset
    for _ in range(count):
        length, = unpack('<i', header, cursor, len(header))
        cursor += 4
        size = length if length >= 0 else -2 * length
        raw, = unpack(f'<{size}s', header, cursor, len(header))
        value = raw.decode('utf-8' if length >= 0 else 'utf-16le')
        if not value.endswith('\0'):
            raise ValueError('unterminated name')
        names.append(value[:-1])
        unpack('<I', header, cursor + size, len(header))
        cursor += size + 4
    export_count, export_offset = unpack('<ii', header, summary + 20, len(header))
    depends_offset, = unpack('<i', header, summary + 36, len(header))
    if export_count != 38 or depends_offset - export_offset != 104 * export_count:
        raise ValueError('unexpected bounded original export table')
    record = export_offset + 21 * 104  # independently inspected MovieSceneFloatSection_0
    name_index, name_number = unpack('<ii', header, record + 16, len(header))
    size, offset = unpack('<qq', header, record + 28, len(header))
    if not 0 <= name_index < len(names) or names[name_index] != 'MovieSceneFloatSection' or name_number != 1:
        raise ValueError('wrong section export identity')
    start, end = offset - header_size, offset - header_size + size
    if start < 0 or end > len(payload):
        raise ValueError('section escapes payload')
    outer = tag(payload, start, end, names)
    if outer['name'] != 'FloatCurve' or outer['kind'] != 'StructProperty' or outer['extra'] != 'MovieSceneFloatChannel':
        raise ValueError('expected original FloatCurve struct tag')
    arrays = channel(payload, outer['data'], outer['next'], names)
    pre, post, element_size, array_count = unpack('<BBii', payload, outer['data'], outer['next'])
    return dict(header_bytes=header_size, payload_bytes=len(payload), logical_package_bytes=header_size + len(payload),
                serialized_custom_versions=0, export_index=22, export_offset=offset, export_size=size,
                curve_logical_offset=outer['data'] + header_size, curve_bytes=outer['size'], arrays=arrays,
                incorrect_manual_prefix=dict(pre_extrap=pre, post_extrap=post,
                                             frame_element_size=element_size, time_count=array_count))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    dump = ROOT / 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'
    if sha(dump) != DUMP_SHA256:
        raise ValueError('wrong original memory image')
    result = inspect_package(PACKAGE.with_suffix('.uasset').read_bytes(), PACKAGE.with_suffix('.uexp').read_bytes())
    args.output.mkdir(parents=True, exist_ok=False)
    with dump.open('rb') as source:
        memory = mmap.mmap(source.fileno(), 0, access=mmap.ACCESS_READ)
        def raw(address, size):
            return memory[address - BASE:address - BASE + size]
        assert struct.unpack('<Q', raw(0x1467cfbf0 + 9 * 8, 8))[0] == 0x143b50e70
        assert raw(0x143b50e70, 8).hex() == '498bc8e918f9feff'
        assert raw(0x140476ec2, 5).hex() == 'ba0b000000'
        assert struct.unpack('<IIII', raw(0x147c39c78, 16)) == (0x7b5ae74c, 0xd2704c10, 0xa9585798, 0x0b212a5a)
        pe, = struct.unpack_from('<I', memory, 0x3c)
        rva, size = struct.unpack_from('<II', memory, pe + 24 + 112 + 24)
        bounds = [struct.unpack_from('<III', memory, rva + i) for i in range(0, size, 12)]
        starts = [entry[0] for entry in bounds]
        for name, address in [('registration', 0x140476eb0), ('channel-serializer', 0x143b40790),
                              ('value-serializer', 0x143b407b0), ('tangent-serializer', 0x143b40900)]:
            begin, end, _ = bounds[bisect.bisect_right(starts, address - BASE) - 1]
            assert begin == address - BASE
            data = memory[begin:end]
            if name == 'channel-serializer':
                assert data.hex() == '4883ec28488b02488bca488d15d7940f04ff90d801000032c04883c428c3'
            binary = args.output / (name + '.bin')
            binary.write_bytes(data)
            binary.with_suffix('.asm').write_text(subprocess.check_output([
                'objdump', '-D', '-b', 'binary', '-m', 'i386:x86-64', '--adjust-vma=' + hex(address), str(binary)], text=True))
    result.update(original_sequencer_version=11, original_channel_serializer_returns_false=True,
                  current_full_channel_version=12, runtime_verified=False)
    tool_snapshot = args.output / 'inspector-source.py'
    tool_snapshot.write_bytes(Path(__file__).read_bytes())
    inputs = [tool_snapshot, dump, PACKAGE.with_suffix('.uasset'), PACKAGE.with_suffix('.uexp')]
    artifacts = [dict(path=str(p.resolve()), sha256=sha(p)) for p in sorted(args.output.iterdir())]
    receipt = dict(schema='fortnite.original-float-channel-evidence.v1', proof=result,
                   inputs=[dict(path=str(p.resolve()), sha256=sha(p)) for p in inputs], artifacts=artifacts,
                   scope='Original registration, tagged channel framing and all key bytes. No reader patch, runtime loading or animation parity is asserted.')
    (args.output / 'receipt.json').write_text(json.dumps(receipt, indent=2, allow_nan=False) + '\n')
    print(json.dumps(result, allow_nan=False))


if __name__ == '__main__':
    main()
