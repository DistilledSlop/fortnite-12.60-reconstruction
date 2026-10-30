#!/usr/bin/env python3
"""Turn a raw virtual-layout runtime dump into a loadable PE file for Ghidra (stdlib only).

The input is SizeOfImage bytes from the image base, as written by dump_runtime_image.py. Its
headers are the in-memory copy of the exe headers. The output keeps those headers and sets each
section's PointerToRawData to its VirtualAddress, so file offset equals RVA. SizeOfRawData is
VirtualSize rounded up to FileAlignment. Output bytes are the dump bytes unchanged, so the
decrypted code and runtime data in the dump are preserved as they were in memory.

Usage: python3 -I porting/dump_to_pe.py DUMP_BIN [-o OUT_EXE]
Default output: DUMP_BIN with the suffix .bin replaced by .exe.
"""
import argparse
import os
import struct
import sys


def align(value, unit):
    return (value + unit - 1) // unit * unit


def convert(src_path, dst_path):
    with open(src_path, "rb") as stream:
        img = bytearray(stream.read())
    if img[:2] != b"MZ":
        raise SystemExit("input does not start with an MZ header")
    e_lfanew = struct.unpack_from("<I", img, 0x3C)[0]
    if img[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        raise SystemExit("bad PE signature in headers")
    nsec = struct.unpack_from("<H", img, e_lfanew + 6)[0]
    opt_size = struct.unpack_from("<H", img, e_lfanew + 20)[0]
    opt = e_lfanew + 24
    if struct.unpack_from("<H", img, opt)[0] != 0x20B:
        raise SystemExit("expected PE32+ image")
    size_of_image = struct.unpack_from("<I", img, opt + 56)[0]
    size_of_headers = struct.unpack_from("<I", img, opt + 60)[0]
    file_align = struct.unpack_from("<I", img, opt + 36)[0]
    if len(img) != size_of_image:
        raise SystemExit(f"dump is {len(img)} bytes, SizeOfImage is {size_of_image}")
    sec_table = opt + opt_size
    report = []
    for i in range(nsec):
        o = sec_table + 40 * i
        name = img[o:o + 8].rstrip(b"\0").decode("latin1")
        vsize = struct.unpack_from("<I", img, o + 8)[0]
        va = struct.unpack_from("<I", img, o + 12)[0]
        if va < size_of_headers:
            raise SystemExit(f"section {name} overlaps the headers")
        if va + vsize > size_of_image:
            raise SystemExit(f"section {name} runs past SizeOfImage")
        raw_size = align(vsize, file_align)
        # PointerToRawData = VirtualAddress, SizeOfRawData = VirtualSize rounded to FileAlignment
        struct.pack_into("<I", img, o + 16, raw_size)
        struct.pack_into("<I", img, o + 20, va)
        report.append((name, va, vsize, raw_size))
    # Keep the file size a multiple of FileAlignment and cover every section's raw range.
    out_len = align(size_of_image, file_align)
    if len(img) < out_len:
        img += bytes(out_len - len(img))
    tmp = dst_path + ".tmp"
    with open(tmp, "wb") as stream:
        stream.write(img)
    os.replace(tmp, dst_path)
    return report, out_len


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    ap.add_argument("dump")
    ap.add_argument("-o", "--out")
    args = ap.parse_args()
    out = args.out or os.path.splitext(args.dump)[0] + ".exe"
    report, size = convert(args.dump, out)
    for name, va, vsize, raw in report:
        print(f"{name:8s} va=0x{va:08x} vsize=0x{vsize:08x} raw_ptr=0x{va:08x} raw_size=0x{raw:08x}")
    print(f"wrote {out} ({size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
