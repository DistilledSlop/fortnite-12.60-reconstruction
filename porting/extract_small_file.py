#!/usr/bin/env python3
"""Extract one small uncompressed file from a v10 pak into ignored output."""

from __future__ import annotations

import argparse
import getpass
import struct
import zlib
from pathlib import Path

from inspect_install import PAK_V10_FOOTER_SIZE, inspect_pak
from list_pak_assets import Reader, decrypt, read_index


MAX_FILE_SIZE = 10 * 1024 * 1024


def find_encoded_entry(path: Path, key: bytes, target: str) -> tuple[bytes, int]:
    info = inspect_pak(path)
    with path.open("rb") as stream:
        stream.seek(-PAK_V10_FOOTER_SIZE, 2)
        primary_hash = stream.read(PAK_V10_FOOTER_SIZE)[41:61]
        primary = read_index(stream, info["index_offset"], info["index_size"], key, primary_hash)
        mount = primary.fstring()
        primary.i32()  # Declared file count
        primary.read(8)  # PathHashSeed
        if primary.i32() != 1:
            raise ValueError("path-hash index missing")
        primary.read(8 + 8 + 20)
        if primary.i32() != 1:
            raise ValueError("directory index missing")
        directory_offset = primary.i64()
        directory_size = primary.i64()
        directory_hash = primary.read(20)
        encoded_size = primary.i32()
        encoded_entries = primary.read(encoded_size)
        directory = read_index(stream, directory_offset, directory_size, key, directory_hash)

    for _ in range(directory.i32()):
        subdir = directory.fstring()
        for _ in range(directory.i32()):
            filename = directory.fstring()
            location = directory.i32()
            separator = "" if not mount.endswith("/") or not subdir.startswith("/") else "/"
            asset_name = mount + subdir.removeprefix(separator) + filename
            if asset_name == target:
                if location < 0:
                    raise ValueError("target uses a nonencoded entry; not supported yet")
                return encoded_entries, location
    raise FileNotFoundError(target)


def decode_entry(encoded_entries: bytes, location: int) -> dict:
    entry = Reader(encoded_entries)
    entry.pos = location
    bitfield = struct.unpack("<I", entry.read(4))[0]
    block_size_code = bitfield & 0x3F
    block_size = struct.unpack("<I", entry.read(4))[0] if block_size_code == 0x3F else block_size_code << 11
    compression_index = (bitfield >> 23) & 0x3F
    offset_32 = bool(bitfield & (1 << 31))
    size_32 = bool(bitfield & (1 << 30))
    compressed_size_32 = bool(bitfield & (1 << 29))
    encrypted = bool(bitfield & (1 << 22))
    block_count = (bitfield >> 6) & 0xFFFF
    offset = entry.i32() if offset_32 else entry.i64()
    uncompressed_size = entry.i32() if size_32 else entry.i64()
    if not 0 <= uncompressed_size <= MAX_FILE_SIZE:
        raise ValueError("target exceeds the small-file extraction limit")
    if offset < 0:
        raise ValueError("invalid file offset")
    compressed_size = (
        (entry.i32() if compressed_size_32 else entry.i64())
        if compression_index else uncompressed_size
    )
    if compression_index not in (0, 1):
        raise ValueError(f"compression method {compression_index} requires FModel/CUE4Parse")
    if compression_index and not 0 < block_count <= 1024:
        raise ValueError("invalid compression block count")
    if compressed_size < 0 or compressed_size > MAX_FILE_SIZE:
        raise ValueError("compressed target exceeds the small-file extraction limit")
    header_size = 8 * 3 + 4 * 2 + 1 + 20
    if compression_index:
        header_size += 4 + block_count * 16
    block_lengths = []
    if compression_index:
        if block_count == 1 and not encrypted:
            block_lengths = [compressed_size]
        else:
            block_lengths = [struct.unpack("<I", entry.read(4))[0] for _ in range(block_count)]
    return {
        "offset": offset,
        "size": uncompressed_size,
        "encrypted": encrypted,
        "compression_index": compression_index,
        "block_size": uncompressed_size if block_count == 1 else block_size,
        "block_lengths": block_lengths,
        "header_size": header_size,
    }


def extract(path: Path, key: bytes, target: str) -> bytes:
    encoded_entries, location = find_encoded_entry(path, key, target)
    entry = decode_entry(encoded_entries, location)
    offset = entry["offset"] + entry["header_size"]
    size = entry["size"]
    encrypted = entry["encrypted"]
    with path.open("rb") as stream:
        if not entry["compression_index"]:
            read_size = (size + 15) & ~15 if encrypted else size
            stream.seek(offset)
            data = stream.read(read_size)
            if len(data) != read_size:
                raise ValueError("short file read")
            return decrypt(data, key)[:size] if encrypted else data

        chunks = []
        for compressed_length in entry["block_lengths"]:
            if compressed_length > MAX_FILE_SIZE:
                raise ValueError("compression block exceeds the small-file limit")
            read_size = (compressed_length + 15) & ~15 if encrypted else compressed_length
            stream.seek(offset)
            data = stream.read(read_size)
            if len(data) != read_size:
                raise ValueError("short compression block read")
            if encrypted:
                data = decrypt(data, key)[:compressed_length]
            chunks.append(zlib.decompress(data))
            offset += read_size
    result = b"".join(chunks)
    if len(result) != size:
        raise ValueError(f"decompressed size {len(result)} differs from expected {size}")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parent.parent
    parser.add_argument("--pak", type=Path, default=root / "Fortnite/FortniteGame/Content/Paks/pakChunkEarly-WindowsClient.pak")
    parser.add_argument("--asset", default="../../../FortniteGame/FortniteGame.uproject")
    parser.add_argument("--output", type=Path, default=root / "porting/generated/FortniteGame.uproject")
    args = parser.parse_args()
    output = args.output.resolve()
    generated = (root / "porting/generated").resolve()
    if not output.is_relative_to(generated):
        raise SystemExit("Output must stay under porting/generated")
    key_hex = getpass.getpass("AES key (input hidden, never saved): ").strip().removeprefix("0x")
    try:
        key = bytes.fromhex(key_hex)
    except ValueError as error:
        raise SystemExit("Expected a hexadecimal AES key") from error
    if len(key) != 32:
        raise SystemExit("Expected a 32-byte AES-256 key")
    data = extract(args.pak, key, args.asset)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(data)
    print(f"Extracted {len(data)} bytes to {output}")


if __name__ == "__main__":
    main()
