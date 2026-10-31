#!/usr/bin/env python3
"""Find the client's HTTP route strings and JSON keys in the 12.60 runtime image, and the code that uses them.

Python standard library only. Run with `python3 -I porting/server/evidence_scan.py`.

What it does:
  1. Reads a PE image (default: the read-only runtime dump under porting/generated/dump, whose
     file offsets equal RVAs and whose .text is decrypted; the on-disk exe's .text is encrypted).
  2. Finds every exact NUL-terminated ASCII or UTF-16LE string that looks like a service path
     (`/api/...`, `/party/api/...`, `/fortnite/api/...`, ...) or is listed in --keys.
  3. Finds RIP-relative `lea reg, [rip+disp32]` instructions in the code sections whose target is
     one of those strings (x64 string literals are loaded this way).
  4. Maps each referencing address to its function with the .pdata RUNTIME_FUNCTION table.

Output is JSON on stdout (or --out). It records addresses and counts only, never secrets.
A reference proves that the code loads the string. It does not prove the field's type or the HTTP
verb; those are read from the surrounding code and recorded as hints, with their confidence.

    python3 -I porting/server/evidence_scan.py --identifier-keys --out SCAN.json
"""

import argparse
import bisect
import json
import os
import re
import struct
import sys

IMAGE_SCN_CNT_CODE = 0x20

DEFAULT_EXE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "generated", "dump",
                           "FortniteClient-Win64-Shipping.runtime.exe")

# Service path shapes. A candidate must contain one of these.
PATH_MARKERS = ("/api/", "/party/", "/presence/", "/lightswitch/", "/content/", "/friends/",
                "/datarouter/", "/fortnite/", "/account/", "/socialban/", "/cloudstorage/")

RUN_A = re.compile(rb"[\x20-\x7e]{3,}(?=\x00)")
RUN_W = re.compile(rb"(?:[\x20-\x7e]\x00){3,}(?=\x00\x00)")
LEA_RIP = re.compile(rb"[\x48-\x4f]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]", re.DOTALL)


class Image:
    def __init__(self, path):
        with open(path, "rb") as handle:
            self.data = handle.read()
        d = self.data
        e_lfanew = struct.unpack_from("<I", d, 0x3C)[0]
        if d[:2] != b"MZ" or d[e_lfanew:e_lfanew + 4] != b"PE\0\0":
            raise SystemExit(f"{path} is not a PE image")
        _, nsec, _, _, _, optsz, _ = struct.unpack_from("<HHIIIHH", d, e_lfanew + 4)
        opt = e_lfanew + 24
        self.image_base = struct.unpack_from("<Q", d, opt + 24)[0]
        self.sections = []
        secoff = opt + optsz
        for i in range(nsec):
            raw = d[secoff + 40 * i: secoff + 40 * i + 40]
            name = raw[:8].rstrip(b"\0").decode("ascii", "replace")
            vsize, va, rsize, rptr = struct.unpack_from("<IIII", raw, 8)
            chars = struct.unpack_from("<I", raw, 36)[0]
            self.sections.append({"name": name, "va": va, "vsize": vsize, "rptr": rptr,
                                  "rsize": rsize, "code": bool(chars & IMAGE_SCN_CNT_CODE)})

    def file_to_rva(self, off):
        for sec in self.sections:
            if sec["rptr"] <= off < sec["rptr"] + sec["rsize"]:
                return sec["va"] + (off - sec["rptr"])
        return None

    def section_for_rva(self, rva):
        for sec in self.sections:
            if sec["va"] <= rva < sec["va"] + max(sec["vsize"], sec["rsize"]):
                return sec
        return None

    def functions(self):
        """(begin_rva, end_rva) from .pdata, sorted by begin."""
        pdata = next((s for s in self.sections if s["name"] == ".pdata"), None)
        if pdata is None:
            return []
        base = pdata["rptr"]
        out = []
        for off in range(base, base + pdata["rsize"] - 11, 12):
            begin, end, _ = struct.unpack_from("<III", self.data, off)
            if begin and end > begin:
                out.append((begin, end))
        out.sort()
        return out


def read_strings(image):
    """Every exact NUL-terminated string in the image: (encoding, rva, text)."""
    found = []
    data = image.data
    for m in RUN_A.finditer(data):
        raw = m.group()
        text = raw.decode("ascii")
        rva = image.file_to_rva(m.start())
        if rva is not None:
            found.append(("ascii", rva, text))
    for m in RUN_W.finditer(data):
        if m.start() % 2:
            continue
        text = m.group().decode("utf-16-le")
        rva = image.file_to_rva(m.start())
        if rva is not None:
            found.append(("utf16", rva, text))
    return found


def is_candidate_path(text):
    if len(text) < 6 or len(text) > 240 or " " in text:
        return False
    return any(marker in text for marker in PATH_MARKERS)


def lea_targets(image, wanted):
    """Map target RVA -> list of referencing code RVAs, for targets in `wanted` only.

    Every code section is scanned for `lea reg, [rip+disp32]` (7 bytes, REX.W). The target is
    the address of the next instruction plus disp32.
    """
    refs = {}
    for sec in image.sections:
        if not sec["code"]:
            continue
        start = sec["rptr"]
        blob = image.data[start:start + sec["rsize"]]
        for m in LEA_RIP.finditer(blob):
            pos = m.start()
            disp = struct.unpack_from("<i", blob, pos + 3)[0]
            code_rva = sec["va"] + pos
            target = code_rva + 7 + disp
            if target in wanted:
                refs.setdefault(target, []).append(code_rva)
    return refs


def owner(functions, begins, rva):
    idx = bisect.bisect_right(begins, rva) - 1
    if idx >= 0:
        begin, end = functions[idx]
        if begin <= rva < end:
            return begin, end
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", default=DEFAULT_EXE)
    parser.add_argument("--keys", help="file with one JSON key per line to search as exact strings")
    parser.add_argument("--identifier-keys", action="store_true",
                        help="also search every identifier-like string (lower camel case, 3 to 41 characters) "
                             "and the HTTP verbs and generic JSON words (key, token, url)")
    parser.add_argument("--out", help="write JSON here instead of stdout")
    args = parser.parse_args()

    import time
    started = time.time()

    def note(message):
        print(f"[{time.time() - started:7.1f}s] {message}", file=sys.stderr, flush=True)

    image = Image(args.exe)
    note("image parsed")
    strings = read_strings(image)
    note(f"{len(strings)} exact strings")

    wanted_keys = set()
    if args.keys:
        with open(args.keys, encoding="utf-8") as handle:
            wanted_keys = {line.strip() for line in handle if line.strip()}

    if args.identifier_keys:
        ident = re.compile(r"[a-z][A-Za-z0-9_]{2,40}")
        wanted_keys |= {text for _, _, text in strings if ident.fullmatch(text)}
        wanted_keys |= {"GET", "POST", "PUT", "DELETE", "PATCH", "key", "token", "url"}

    selected = []
    for enc, rva, text in strings:
        if is_candidate_path(text) or text in wanted_keys:
            selected.append({"encoding": enc, "rva": rva, "text": text})

    note(f"{len(selected)} candidate strings")
    refs = lea_targets(image, {item["rva"] for item in selected})
    note(f"{len(refs)} distinct LEA targets")
    functions = image.functions()
    begins = [b for b, _ in functions]
    note(f"{len(functions)} functions")

    rows = []
    for item in selected:
        code = refs.get(item["rva"], [])
        funcs = []
        for code_rva in code:
            own = owner(functions, begins, code_rva)
            funcs.append({"code_rva": code_rva, "function_rva": own[0] if own else None})
        unique_funcs = sorted({f["function_rva"] for f in funcs if f["function_rva"] is not None})
        rows.append({
            "text": item["text"],
            "encoding": item["encoding"],
            "rva": item["rva"],
            "va": image.image_base + item["rva"],
            "lea_refs": len(code),
            "functions": [{"rva": f, "va": image.image_base + f} for f in unique_funcs],
            "code_refs": [hex(f["code_rva"]) for f in funcs][:32],
        })

    result = {
        "exe": os.path.basename(args.exe),
        "image_base": hex(image.image_base),
        "sections": image.sections,
        "function_count": len(functions),
        "string_count": len(strings),
        "candidate_count": len(rows),
        "rows": sorted(rows, key=lambda r: (r["text"], r["encoding"])),
    }
    text = json.dumps(result, indent=1, ensure_ascii=False)
    if args.out:
        with open(args.out, "w", encoding="utf-8") as handle:
            handle.write(text + "\n")
        print(f"wrote {args.out}: {len(rows)} candidate strings", file=sys.stderr)
    else:
        print(text)


if __name__ == "__main__":
    main()
