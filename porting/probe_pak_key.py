#!/usr/bin/env python3
"""Test an AES key against the encrypted v10 pak indexes without saving it."""

from __future__ import annotations

import argparse
import getpass
import hashlib
import json
import struct
from pathlib import Path

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

from inspect_install import PAK_V10_FOOTER_SIZE, inspect_pak


def read_fstring(data: bytes) -> str:
    if len(data) < 4:
        raise ValueError("short index")
    length = struct.unpack_from("<i", data)[0]
    if 0 < length <= 4096:
        raw = data[4 : 4 + length]
        if len(raw) != length or not raw.endswith(b"\0"):
            raise ValueError("invalid UTF-8 mount point")
        return raw[:-1].decode("utf-8")
    if -4096 <= length < 0:
        raw = data[4 : 4 + (-length * 2)]
        if len(raw) != -length * 2 or not raw.endswith(b"\0\0"):
            raise ValueError("invalid UTF-16 mount point")
        return raw[:-2].decode("utf-16-le")
    raise ValueError("invalid FString length")


def probe(path: Path, key: bytes) -> dict:
    info = inspect_pak(path)
    with path.open("rb") as stream:
        stream.seek(-PAK_V10_FOOTER_SIZE, 2)
        expected_hash = stream.read(PAK_V10_FOOTER_SIZE)[41:61]
        stream.seek(info["index_offset"])
        encrypted = stream.read(info["index_size"])
    if len(encrypted) % 16:
        raise ValueError(f"{path.name}: encrypted index is not AES-block aligned")
    decryptor = Cipher(algorithms.AES(key), modes.ECB()).decryptor()
    plaintext = decryptor.update(encrypted) + decryptor.finalize()
    hash_matches = hashlib.sha1(plaintext).digest() == expected_hash
    try:
        mount_point = read_fstring(plaintext)
        plausible_mount = mount_point.startswith("../../../") or mount_point.startswith("/")
    except (UnicodeDecodeError, ValueError):
        mount_point = None
        plausible_mount = False
    return {
        "pak": path.name,
        "key_guid": info["key_guid"],
        "index_hash_matches": hash_matches,
        "mount_point": mount_point if plausible_mount else None,
        "key_works": hash_matches and plausible_mount,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    default_root = Path(__file__).resolve().parent.parent
    parser.add_argument(
        "--pak-dir", type=Path,
        default=default_root / "Fortnite/FortniteGame/Content/Paks",
    )
    args = parser.parse_args()
    key_hex = getpass.getpass("AES key (input hidden, never saved): ").strip().removeprefix("0x")
    try:
        key = bytes.fromhex(key_hex)
    except ValueError as error:
        raise SystemExit("Expected a hexadecimal AES key") from error
    if len(key) != 32:
        raise SystemExit("Expected a 32-byte AES-256 key")
    results = [probe(path, key) for path in sorted(args.pak_dir.glob("*.pak"))]
    print(json.dumps({
        "tested": len(results),
        "working": sum(result["key_works"] for result in results),
        "results": results,
    }, indent=2))


if __name__ == "__main__":
    main()
