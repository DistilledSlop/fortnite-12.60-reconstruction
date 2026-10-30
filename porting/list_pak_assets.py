#!/usr/bin/env python3
"""List filenames in readable v10 pak indexes without extracting assets."""

from __future__ import annotations

import argparse
import getpass
import hashlib
import json
import struct
from pathlib import Path

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

from inspect_install import PAK_V10_FOOTER_SIZE, inspect_pak


class Reader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def read(self, size: int) -> bytes:
        if size < 0 or self.pos + size > len(self.data):
            raise ValueError("index ended unexpectedly")
        result = self.data[self.pos : self.pos + size]
        self.pos += size
        return result

    def i32(self) -> int:
        return struct.unpack("<i", self.read(4))[0]

    def i64(self) -> int:
        return struct.unpack("<q", self.read(8))[0]

    def fstring(self) -> str:
        length = self.i32()
        if 0 < length <= 65536:
            raw = self.read(length)
            if not raw.endswith(b"\0"):
                raise ValueError("unterminated UTF-8 FString")
            return raw[:-1].decode("utf-8")
        if -65536 <= length < 0:
            raw = self.read(-length * 2)
            if not raw.endswith(b"\0\0"):
                raise ValueError("unterminated UTF-16 FString")
            return raw[:-2].decode("utf-16-le")
        raise ValueError("invalid FString length")


def decrypt(data: bytes, key: bytes) -> bytes:
    if len(data) % 16:
        raise ValueError("encrypted index is not AES-block aligned")
    context = Cipher(algorithms.AES(key), modes.ECB()).decryptor()
    return context.update(data) + context.finalize()


def read_index(stream, offset: int, size: int, key: bytes, expected_hash: bytes) -> Reader:
    stream.seek(offset)
    data = stream.read(size)
    if len(data) != size:
        raise ValueError("short index read")
    plaintext = decrypt(data, key)
    if hashlib.sha1(plaintext).digest() != expected_hash:
        raise ValueError("index hash mismatch; key may be wrong")
    return Reader(plaintext)


def list_pak(path: Path, key: bytes) -> dict:
    info = inspect_pak(path)
    with path.open("rb") as stream:
        stream.seek(-PAK_V10_FOOTER_SIZE, 2)
        primary_hash = stream.read(PAK_V10_FOOTER_SIZE)[41:61]
        primary = read_index(stream, info["index_offset"], info["index_size"], key, primary_hash)
        mount = primary.fstring()
        declared_count = primary.i32()
        primary.read(8)  # PathHashSeed
        if primary.i32() != 1:
            raise ValueError("path-hash index missing")
        primary.read(8 + 8 + 20)  # PathHashIndex offset, size, SHA-1
        if primary.i32() != 1:
            raise ValueError("directory index missing")
        directory_offset = primary.i64()
        directory_size = primary.i64()
        directory_hash = primary.read(20)
        directory = read_index(stream, directory_offset, directory_size, key, directory_hash)

    directory_count = directory.i32()
    if not 0 <= directory_count <= 1_000_000:
        raise ValueError("invalid directory count")
    names: list[str] = []
    for _ in range(directory_count):
        subdir = directory.fstring()
        entry_count = directory.i32()
        if not 0 <= entry_count <= 1_000_000:
            raise ValueError("invalid file count in directory")
        for _ in range(entry_count):
            filename = directory.fstring()
            directory.i32()  # Encoded pak entry offset or nonencoded entry index
            separator = "" if not mount.endswith("/") or not subdir.startswith("/") else "/"
            names.append(mount + subdir.removeprefix(separator) + filename)
    if len(names) != declared_count:
        raise ValueError(f"declared {declared_count} files; found {len(names)}")
    return {"pak": path.name, "mount_point": mount, "file_count": len(names), "files": names}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    default_root = Path(__file__).resolve().parent.parent
    parser.add_argument(
        "--pak-dir", type=Path,
        default=default_root / "Fortnite/FortniteGame/Content/Paks",
    )
    parser.add_argument("--pak", type=Path, help="List one pak instead of the whole directory")
    args = parser.parse_args()
    key_hex = getpass.getpass("AES key (input hidden, never saved): ").strip().removeprefix("0x")
    try:
        key = bytes.fromhex(key_hex)
    except ValueError as error:
        raise SystemExit("Expected a hexadecimal AES key") from error
    if len(key) != 32:
        raise SystemExit("Expected a 32-byte AES-256 key")
    results, skipped = [], []
    paths = [args.pak] if args.pak else sorted(args.pak_dir.glob("*.pak"))
    for path in paths:
        try:
            results.append(list_pak(path, key))
        except ValueError as error:
            skipped.append({"pak": path.name, "reason": str(error)})
    print(json.dumps({
        "indexed_paks": len(results),
        "total_files": sum(result["file_count"] for result in results),
        "skipped": skipped,
        "paks": results,
    }, indent=2))


if __name__ == "__main__":
    main()
