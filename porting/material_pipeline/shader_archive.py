#!/usr/bin/env python3
"""Read explicit UE4 shader-library layouts and recover bounded original DXBC.

The explicit Fortnite layout is verified against the supplied file boundaries;
the other layout follows RenderCore/Public/ShaderCodeArchive.h and its .cpp.
Artifacts are evidence for shader translation, not Linux shaders or material parity.
"""
import argparse
from collections import Counter
import hashlib
import json
import mmap
from pathlib import Path
import struct
import zlib

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
MAX_SHADER_SIZE = 64 * 1024 * 1024


def sha256(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def lz4_block(data, expected):
    """Decode raw LZ4 with bounded output, including overlapping match copies."""
    if not 0 <= expected <= MAX_SHADER_SIZE:
        raise ValueError('unreasonable shader size')
    pos, out = 0, bytearray()

    def length(base):
        nonlocal pos
        if base == 15:
            while True:
                if pos >= len(data):
                    raise ValueError('truncated LZ4 length')
                extra = data[pos]
                pos += 1
                base += extra
                if extra != 255:
                    break
        return base

    while pos < len(data):
        token = data[pos]
        pos += 1
        count = length(token >> 4)
        if pos + count > len(data) or len(out) + count > expected:
            raise ValueError('LZ4 literal exceeds input/output')
        out.extend(data[pos:pos + count])
        pos += count
        if pos == len(data):
            break
        if pos + 2 > len(data):
            raise ValueError('truncated LZ4 offset')
        offset = struct.unpack_from('<H', data, pos)[0]
        pos += 2
        count = length(token & 15) + 4
        if not offset or offset > len(out) or len(out) + count > expected:
            raise ValueError('invalid LZ4 match')
        for _ in range(count):
            out.append(out[-offset])
    if len(out) != expected:
        raise ValueError('LZ4 output size mismatch')
    return bytes(out)


def dxbc_containers(code):
    """Validate DXBC chunk boundaries inside the UE wrapper; do not invent bindings."""
    found, cursor = [], 0
    while True:
        start = code.find(b'DXBC', cursor)
        if start < 0:
            return found
        cursor = start + 4
        if start + 32 > len(code):
            continue
        marker, size, count = struct.unpack_from('<III', code, start + 20)
        if marker != 1 or count > 256 or size < 32 + count * 4 or start + size > len(code):
            continue
        chunks, ranges = [], []
        valid = True
        for i in range(count):
            offset = struct.unpack_from('<I', code, start + 32 + i * 4)[0]
            if offset % 4 or offset < 32 + count * 4 or offset + 8 > size:
                valid = False
                break
            tag, length = struct.unpack_from('<4sI', code, start + offset)
            end = offset + 8 + length
            if end > size or any(offset < other_end and begin < end for begin, other_end in ranges):
                valid = False
                break
            ranges.append((offset, end))
            chunks.append({'tag': tag.decode('ascii', errors='replace'), 'offset': offset, 'size': length})
        if valid and any(c['tag'] in ('SHDR', 'SHEX', 'DXIL') for c in chunks):
            found.append({'offset': start, 'size': size, 'chunks': chunks})
            cursor = start + size


class ShaderArchive:
    def __init__(self, path, layout='fortnite-12.60'):
        self.path = Path(path)
        self.layout = layout
        self.file = self.path.open('rb')
        try:
            self.data = mmap.mmap(self.file.fileno(), 0, access=mmap.ACCESS_READ)
            self.pos = 0
            if self.scalar('<I') != 2:
                raise ValueError('only non-IoStore version-2 shader libraries are supported')
            if layout == 'fortnite-12.60':
                # Both supplied libraries have 29-byte records, no serialized offsets
                # or shader-map arrays, and contiguous zlib payloads ending at EOF.
                rows = self.array('20sIIB')
                self.shader_hashes = [row[0] for row in rows]
                self.map_hashes, self.maps, self.preloads, self.indices = [], [], [], []
                self.shaders = []
                offset = 0
                for _hash, size, expanded, frequency in rows:
                    self.shaders.append((offset, size, expanded, frequency))
                    offset += size
            elif layout == 'ue4-serialized':
                self.map_hashes = self.array('20s')
                self.shader_hashes = self.array('20s')
                self.maps = self.array('IIII')
                self.shaders = self.array('QIIB')
                self.preloads = self.array('qq')
                self.indices = self.array('I')
            else:
                raise ValueError('unknown shader library layout')
            self.code_start = self.pos
            self.validate()
            self.map_lookup = {h: i for i, h in enumerate(self.map_hashes)}
            self.shader_lookup = {h: i for i, h in enumerate(self.shader_hashes)}
        except Exception:
            self.close()
            raise

    def close(self):
        if hasattr(self, 'data'):
            self.data.close()
        self.file.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def scalar(self, fmt):
        size = struct.calcsize(fmt)
        if self.pos + size > len(self.data):
            raise ValueError('truncated shader archive')
        result = struct.unpack_from(fmt, self.data, self.pos)[0]
        self.pos += size
        return result

    def array(self, fmt):
        count = self.scalar('<i')
        layout = struct.Struct('<' + fmt)
        if count < 0 or count > (len(self.data) - self.pos) // layout.size:
            raise ValueError('shader array count exceeds archive')
        result = []
        for _ in range(count):
            item = layout.unpack_from(self.data, self.pos)
            result.append(item[0] if len(item) == 1 else item)
            self.pos += layout.size
        return result

    def validate(self):
        if len(self.maps) != len(self.map_hashes) or len(self.shaders) != len(self.shader_hashes):
            raise ValueError('shader/hash counts differ')
        if len(set(self.map_hashes)) != len(self.map_hashes):
            raise ValueError('duplicate shader map hash')
        if len(set(self.shader_hashes)) != len(self.shader_hashes):
            raise ValueError('duplicate shader output hash')
        for first, count, preload, preload_count in self.maps:
            if first + count > len(self.indices) or preload + preload_count > len(self.preloads):
                raise ValueError('shader map range exceeds index arrays')
        if any(index >= len(self.shaders) for index in self.indices):
            raise ValueError('invalid shader index')
        payload = len(self.data) - self.code_start
        for offset, size, expanded, _frequency in self.shaders:
            if offset + size > payload or not 0 < expanded <= MAX_SHADER_SIZE or not size:
                raise ValueError('invalid shader payload range/size')
        for offset, size in self.preloads:
            if offset < 0 or size < 0 or offset + size > payload:
                raise ValueError('invalid preload payload range')
        if self.layout == 'fortnite-12.60' and sum(s[1] for s in self.shaders) != payload:
            raise ValueError('legacy contiguous shader payload does not end at EOF')

    def material_matches(self, path):
        raw = Path(path).read_bytes()
        # Exact 160-bit matches are candidates until the inline field is decoded.
        lookup = self.shader_lookup if self.layout == 'fortnite-12.60' else self.map_lookup
        kind = 'shader' if self.layout == 'fortnite-12.60' else 'shader_map'
        return [{'offset': pos, 'kind': kind, 'index': lookup[raw[pos:pos + 20]],
                 'hash': raw[pos:pos + 20].hex()}
                for pos in range(max(0, len(raw) - 19)) if raw[pos:pos + 20] in lookup]

    def map_shader_indices(self, map_index):
        first, count, _, _ = self.maps[map_index]
        return self.indices[first:first + count]

    def shader_code(self, index):
        offset, size, expanded, _ = self.shaders[index]
        raw = self.data[self.code_start + offset:self.code_start + offset + size]
        if size == expanded:
            return raw
        if self.layout == 'ue4-serialized':
            return lz4_block(raw, expanded)
        decoder = zlib.decompressobj()
        code = decoder.decompress(raw, expanded + 1)
        if len(code) != expanded or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError('zlib shader payload size or stream boundary mismatch')
        return code

    def summary(self):
        return {'archive': str(self.path.resolve()), 'sha256': sha256(self.path),
                'bytes': len(self.data), 'version': 2, 'layout': self.layout,
                'compression': 'zlib' if self.layout == 'fortnite-12.60' else 'lz4',
                'shader_maps': len(self.maps),
                'shader_count': len(self.shaders), 'shader_indices': len(self.indices),
                'payload_offset': self.code_start,
                'frequencies': dict(sorted(Counter(row[3] for row in self.shaders).items())),
                'compressed_shaders': sum(size != expanded for _, size, expanded, _ in self.shaders)}


def artifact_path(value):
    path = Path(value).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('artifacts must be inside porting/generated')
    return path


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--archive', type=Path, required=True)
    p.add_argument('--layout', choices=['fortnite-12.60', 'ue4-serialized'], default='fortnite-12.60')
    p.add_argument('--material-uexp', type=Path, action='append', default=[])
    p.add_argument('--map-hash', action='append', default=[])
    p.add_argument('--shader-index', type=int, action='append', default=[])
    p.add_argument('--max-shaders', type=int, default=8)
    p.add_argument('--extract-dir', type=artifact_path)
    p.add_argument('--report', type=artifact_path)
    args = p.parse_args()
    if not 1 <= args.max_shaders <= 256:
        p.error('--max-shaders must be 1..256')
    try:
        with ShaderArchive(args.archive, args.layout) as archive:
            report = archive.summary()
            report['materials'] = []
            maps, selected = set(), set()
            for path in args.material_uexp:
                matches = archive.material_matches(path)
                report['materials'].append({'path': str(path.resolve()), 'sha256': sha256(path),
                                            'candidate_library_hash_matches': matches})
                maps.update(m['index'] for m in matches if m['kind'] == 'shader_map')
                selected.update(m['index'] for m in matches if m['kind'] == 'shader')
            for value in args.map_hash:
                key = bytes.fromhex(value)
                if len(key) != 20 or key not in archive.map_lookup:
                    raise ValueError('unknown or invalid shader map hash')
                maps.add(archive.map_lookup[key])
            if any(i < 0 or i >= len(archive.shaders) for i in args.shader_index):
                raise ValueError('shader index outside archive')
            selected.update(args.shader_index)
            indices = sorted(selected | {i for m in maps for i in archive.map_shader_indices(m)})
            report.update(selected_maps=sorted(maps), selected_shader_count=len(indices), shaders=[])
            for index in indices[:args.max_shaders]:
                code = archive.shader_code(index)
                containers = dxbc_containers(code)
                item = {'index': index, 'library_output_hash': archive.shader_hashes[index].hex(),
                        'frequency': archive.shaders[index][3], 'decoded_bytes': len(code),
                        'decoded_sha256': hashlib.sha256(code).hexdigest(), 'dxbc': containers}
                if args.extract_dir:
                    args.extract_dir.mkdir(parents=True, exist_ok=True)
                    stem = f'{index}-{archive.shader_hashes[index].hex()}'
                    full = args.extract_dir / (stem + '.ue-shader-code')
                    full.write_bytes(code)
                    item['wrapper_path'] = str(full)
                    for number, container in enumerate(containers):
                        dest = args.extract_dir / f'{stem}-{number}.dxbc'
                        offset, size = container['offset'], container['size']
                        dest.write_bytes(code[offset:offset + size])
                        container['path'] = str(dest)
                report['shaders'].append(item)
            report['decoded_shader_count'] = len(report['shaders'])
            report['selection_truncated'] = len(indices) > args.max_shaders
            report['limitations'] = ['Material matches are exact archive hashes in bytes, not decoded inline fields',
                                    'Library output hash is not asserted to equal a raw bytecode SHA hash',
                                    'Original DXBC is not Vulkan/SPIR-V and has not been rendered',
                                    'Uniform binding, shader translation and material parity remain unverified']
        encoded = json.dumps(report, indent=2) + '\n'
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(encoded)
        print(encoded, end='')
        return 0
    except (OSError, ValueError, struct.error) as error:
        p.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    raise SystemExit(main())
