#!/usr/bin/env python3
"""Read-only runtime dump of the shipping exe image from a running Wine client.

Reads /proc/<pid>/maps and /proc/<pid>/mem only. It never attaches, stops, or writes to the
process, and never touches the exe on disk. Needs root under ptrace_scope=1, so run it with
sudo -n. Polls until the executable sections look decrypted, then writes the whole image in
virtual layout (SizeOfImage bytes from the image base) and a JSON record next to it.

The existing whole-image heuristic checks median 1 MiB-window entropy of executable sections
against --max-entropy and the objdump sampled-instruction bad fraction against --max-bad.
It does not establish that each individual code region is decrypted or readable.
If verification fails, polling continues, so a later attempt can still succeed.

Usage:
  sudo -n python3 -I porting/dump_runtime_image.py [--out DIR] [--timeout S] [--interval S]
Output (ignored by git): DIR/FortniteClient-Win64-Shipping.runtime.bin and DIR/dump.json.
Convert to a loadable PE with porting/dump_to_pe.py.

To wait for selected locations to change relative to an earlier raw virtual-layout dump:
  --compare-dump EARLIER.runtime.bin --changed-va 0x1495a4a24 --changed-va 0x14998573a
Each address compares 16 bytes by default; --compare-width changes that width. Every range
must differ by at least one byte. Use a baseline from the same build and image base. Changed
bytes do not prove decryption or valid code. With comparison enabled, any unreadable page
prevents acceptance.
"""
import argparse
import bisect
import collections
import hashlib
import json
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import time

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE_NAME = "FortniteClient-Win64-Shipping.exe"
EXE_PATH = os.path.join(REPO, "Fortnite", "FortniteGame", "Binaries", "Win64", EXE_NAME)
DEFAULT_OUT = os.path.join(REPO, "porting", "generated", "dump")
DUMP_NAME = "FortniteClient-Win64-Shipping.runtime.bin"
PAGE = 4096
WINDOW = 1 << 20
SLICE = 64 << 10
IMAGE_BASE = 0x140000000
IMAGE_CHARACTERISTICS_EXEC = 0x20000000


def find_pid():
    """PID of the Wine process that has the exe image mapped at the expected base."""
    out = subprocess.run(["pgrep", "-f", "^" + EXE_NAME], capture_output=True, text=True).stdout.split()
    for pid in out:
        try:
            with open(f"/proc/{pid}/maps") as stream:
                if re.search(rf"^{IMAGE_BASE:x}-", stream.read(), re.M):
                    return int(pid)
        except OSError:
            continue
    return None


def process_age(pid):
    """Seconds since the process started, from /proc/<pid>/stat and /proc/uptime."""
    with open(f"/proc/{pid}/stat") as stream:
        fields = stream.read().rsplit(")", 1)[1].split()
    start_ticks = int(fields[19])
    with open("/proc/uptime") as stream:
        uptime = float(stream.read().split()[0])
    return uptime - start_ticks / os.sysconf("SC_CLK_TCK")


def read_at(fd, va, size):
    """Read size bytes at va from /proc/<pid>/mem. Unreadable pages come back zero-filled."""
    buf = bytearray(size)
    bad_pages = 0
    pos = 0
    while pos < size:
        n = min(1 << 20, size - pos)
        try:
            data = os.pread(fd, n, va + pos)
            if len(data) != n:
                raise OSError("short read")
            buf[pos:pos + n] = data
        except OSError:
            for page in range(pos, pos + n, PAGE):
                m = min(PAGE, size - page)
                try:
                    data = os.pread(fd, m, va + page)
                    if len(data) != m:
                        raise OSError("short read")
                    buf[page:page + m] = data
                except OSError:
                    bad_pages += 1
        pos += n
    return bytes(buf), bad_pages


def parse_pe(img):
    """Parse the PE headers of an image (bytes indexed by RVA). Returns a dict."""
    if img[:2] != b"MZ":
        raise ValueError("no MZ header at image base")
    e = struct.unpack_from("<I", img, 0x3C)[0]
    if img[e:e + 4] != b"PE\0\0":
        raise ValueError("bad PE signature")
    nsec = struct.unpack_from("<H", img, e + 6)[0]
    opt_size = struct.unpack_from("<H", img, e + 20)[0]
    opt = e + 24
    if struct.unpack_from("<H", img, opt)[0] != 0x20B:
        raise ValueError("expected PE32+")
    pe = {
        "file_align": struct.unpack_from("<I", img, opt + 36)[0],
        "size_of_image": struct.unpack_from("<I", img, opt + 56)[0],
        "size_of_headers": struct.unpack_from("<I", img, opt + 60)[0],
        "sections": [],
    }
    so = opt + opt_size
    for i in range(nsec):
        o = so + 40 * i
        pe["sections"].append({
            "name": img[o:o + 8].rstrip(b"\0").decode("latin1"),
            "vsize": struct.unpack_from("<I", img, o + 8)[0],
            "va": struct.unpack_from("<I", img, o + 12)[0],
            "rsize": struct.unpack_from("<I", img, o + 16)[0],
            "raw": struct.unpack_from("<I", img, o + 20)[0],
            "chars": struct.unpack_from("<I", img, o + 36)[0],
        })
    return pe


def entropy(data):
    """Shannon entropy in bits per byte."""
    n = len(data)
    total = 0.0
    for count in collections.Counter(data).values():
        p = count / n
        total -= p * math.log2(p)
    return total


def code_sections(pe):
    return [s for s in pe["sections"] if s["chars"] & IMAGE_CHARACTERISTICS_EXEC]


def quick_sample(fd, pe):
    """Cheap pre-check: mean entropy of eight 64 KiB slices spread over each code section."""
    values = []
    for s in code_sections(pe):
        span = max(s["vsize"], s["rsize"])
        for k in range(1, 9):
            off = (span * k // 9) & ~0xFFF
            if off + SLICE > span:
                continue
            try:
                data = os.pread(fd, SLICE, IMAGE_BASE + s["va"] + off)
            except OSError:
                continue
            if len(data) == SLICE:
                values.append(entropy(data))
    return sum(values) / len(values) if values else None


def pdata_functions(img, pe):
    """(begin_rva, end_rva) of RUNTIME_FUNCTION entries that start inside an executable section."""
    opt = struct.unpack_from("<I", img, 0x3C)[0] + 24
    rva, size = struct.unpack_from("<II", img, opt + 112 + 8 * 3)
    funcs = []
    if not rva or not size:
        return funcs
    for o in range(rva, rva + size - 11, 12):
        begin, end = struct.unpack_from("<II", img, o)
        if any(s["va"] <= begin < s["va"] + s["vsize"] and s["chars"] & IMAGE_CHARACTERISTICS_EXEC for s in pe["sections"]):
            if 16 <= end - begin <= 16384:
                funcs.append((begin, end))
    return funcs


def decode_functions(img, funcs, scratch_dir, count=400):
    """objdump a sample of functions, each decoded from its own start, and count clean ones.

    Functions are laid out back to back, 16-byte aligned and int3-padded, in one file. Each
    output line is attributed to the function whose byte range holds its address."""
    if not funcs:
        return {"functions": 0}
    step = max(1, len(funcs) // count)
    sample = funcs[::step][:count]
    blob = bytearray()
    spans = []
    for begin, end in sample:
        while len(blob) % 16:
            blob.append(0xCC)
        spans.append((len(blob), len(blob) + (end - begin), begin, end))
        blob += img[begin:end]
    path = os.path.join(scratch_dir, "functions.bin")
    with open(path, "wb") as stream:
        stream.write(blob)
    out = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386:x86-64", path],
                         capture_output=True, text=True).stdout
    os.remove(path)
    starts = [s[0] for s in spans]
    bad_per_fn = [0] * len(spans)
    insns = bad = 0
    for line in out.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\t", line)
        if not m:
            continue
        insns += 1
        addr = int(m.group(1), 16)
        if "(bad)" in line:
            bad += 1
            k = bisect.bisect_right(starts, addr) - 1
            if k >= 0 and addr < spans[k][1]:
                bad_per_fn[k] += 1
    clean = sum(1 for b in bad_per_fn if b == 0)
    return {"functions": len(spans), "functions_clean": clean,
            "functions_clean_fraction": round(clean / len(spans), 4),
            "instructions": insns, "bad": bad,
            "bad_fraction": round(bad / insns, 5) if insns else None}


def verify(img, pe, scratch_dir, max_entropy, max_bad):
    """Full verification of the in-memory image. Returns (passed, report dict).

    Entropy: median 1 MiB-window entropy of every executable section <= max_entropy.
    Decode: share of sampled .pdata functions that objdump decodes with no '(bad)' line,
    and the share of '(bad)' lines overall, must be at least 1 - max_bad."""
    report = {"sections": [], "verdict": None}
    code_medians = []
    for s in pe["sections"]:
        size = min(s["vsize"], len(img) - s["va"])
        data = img[s["va"]:s["va"] + size]
        windows = []
        for off in range(0, len(data) - WINDOW + 1, WINDOW):
            windows.append(entropy(data[off:off + WINDOW]))
        entry = {"name": s["name"], "va": hex(s["va"]), "vsize": s["vsize"],
                 "exec": bool(s["chars"] & IMAGE_CHARACTERISTICS_EXEC)}
        if windows:
            srt = sorted(windows)
            entry["windows"] = len(windows)
            entry["entropy_min"] = round(srt[0], 3)
            entry["entropy_median"] = round(srt[len(srt) // 2], 3)
            entry["entropy_max"] = round(srt[-1], 3)
            entry["windows_ge_7_5"] = sum(1 for w in windows if w >= 7.5)
        if entry["exec"] and windows:
            code_medians.append(entry["entropy_median"])
        report["sections"].append(entry)
    funcs = pdata_functions(img, pe)
    report["pdata_functions_in_code"] = len(funcs)
    report["decode"] = decode_functions(img, funcs, scratch_dir)
    report["code_median_entropy"] = max(code_medians) if code_medians else None
    entropy_ok = bool(code_medians) and all(m <= max_entropy for m in code_medians)
    bad_fraction = report["decode"].get("bad_fraction")
    decode_ok = bad_fraction is not None and bad_fraction <= max_bad
    report["verdict"] = "decrypted" if entropy_ok and decode_ok else "encrypted_or_partial"
    return entropy_ok and decode_ok, report


def compare_with_disk(img, pe):
    """Share of bytes equal to the exe on disk, per section (diagnostic only)."""
    out = {}
    try:
        with open(EXE_PATH, "rb") as stream:
            disk = stream.read()
    except OSError:
        return None
    for s in pe["sections"]:
        n = min(s["rsize"], s["vsize"], len(img) - s["va"], len(disk) - s["raw"])
        if n <= 0:
            continue
        mem = img[s["va"]:s["va"] + n]
        ondisk = disk[s["raw"]:s["raw"] + n]
        same = sum(1 for a, b in zip(mem[::64], ondisk[::64]) if a == b)
        out[f"{s['name']}@0x{s['va']:x}"] = round(same / max(1, len(mem[::64])), 4)
    return out


def changed_ranges(img, baseline, addresses, width):
    """Compare image-layout byte ranges, returning a report without storing their contents."""
    if len(img) != len(baseline):
        raise ValueError("baseline and captured image sizes differ")
    checks = []
    for address in addresses:
        offset = address - IMAGE_BASE
        if offset < 0 or offset + width > len(img):
            raise ValueError(f"range at {address:#x} is outside the image")
        changed = sum(a != b for a, b in zip(img[offset:offset + width],
                                             baseline[offset:offset + width]))
        checks.append({"address": hex(address), "size": width,
                       "changed_bytes": changed, "changed": changed > 0})
    return checks


def chown_to_invoker(path):
    uid = os.environ.get("SUDO_UID")
    gid = os.environ.get("SUDO_GID")
    if os.geteuid() == 0 and uid and gid:
        os.chown(path, int(uid), int(gid))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    ap.add_argument("--out", default=DEFAULT_OUT)
    ap.add_argument("--timeout", type=float, default=90.0, help="seconds to keep polling")
    ap.add_argument("--interval", type=float, default=0.05, help="seconds between polls")
    ap.add_argument("--max-entropy", type=float, default=6.8)
    ap.add_argument("--max-bad", type=float, default=0.05)
    ap.add_argument("--compare-dump", help="earlier raw virtual-layout runtime .bin")
    ap.add_argument("--changed-va", action="append", default=[], type=lambda v: int(v, 0),
                    help="absolute VA whose range must differ from the earlier dump; repeatable")
    ap.add_argument("--compare-width", type=int, default=16,
                    help="bytes compared at each VA (default: 16)")
    args = ap.parse_args()
    if bool(args.compare_dump) != bool(args.changed_va):
        ap.error("--compare-dump and at least one --changed-va must be used together")
    if args.compare_width <= 0:
        ap.error("--compare-width must be positive")
    baseline = None
    baseline_hash = None
    if args.compare_dump:
        try:
            with open(args.compare_dump, "rb") as stream:
                baseline = stream.read()
            baseline_pe = parse_pe(baseline)
            if len(baseline) != baseline_pe["size_of_image"]:
                raise ValueError("baseline must be a complete raw virtual-layout image")
            changed_ranges(baseline, baseline, args.changed_va, args.compare_width)
        except (OSError, ValueError, struct.error, IndexError) as exc:
            ap.error(f"invalid comparison dump or address: {exc}")
        baseline_hash = hashlib.sha256(baseline).hexdigest()

    os.makedirs(args.out, exist_ok=True)
    chown_to_invoker(args.out)
    deadline = time.time() + args.timeout
    trace = []
    attempts = 0
    pid = None
    while time.time() < deadline:
        if pid is None or not os.path.exists(f"/proc/{pid}"):
            pid = find_pid()
            if pid is None:
                if trace:
                    print("process exited before a verified dump", file=sys.stderr)
                    return 2
                time.sleep(args.interval)
                continue
            print(f"attached to pid {pid} (read-only)", flush=True)
        try:
            fd = os.open(f"/proc/{pid}/mem", os.O_RDONLY)
        except OSError:
            time.sleep(args.interval)
            continue
        try:
            age = process_age(pid)
            head, _ = read_at(fd, IMAGE_BASE, PAGE)
            pe_head = parse_pe(head)
            sample = quick_sample(fd, pe_head) if pe_head["sections"] else None
            trace.append({"age_s": round(age, 3), "sample_entropy": round(sample, 3) if sample else None})
            if sample is None or sample > args.max_entropy + 0.4:
                time.sleep(args.interval)
                continue
            attempts += 1
            print(f"age {age:.2f}s sample entropy {sample:.3f}: full read attempt {attempts}", flush=True)
            size = pe_head["size_of_image"]
            img, bad_pages = read_at(fd, IMAGE_BASE, size)
            pe = parse_pe(img)
            comparison = None
            if baseline is not None:
                try:
                    checks = changed_ranges(img, baseline, args.changed_va, args.compare_width)
                except ValueError as exc:
                    print(f"comparison failed: {exc}", file=sys.stderr)
                    return 2
                comparison = {"baseline_file": os.path.abspath(args.compare_dump),
                              "baseline_sha256": baseline_hash, "checks": checks,
                              "all_changed": all(check["changed"] for check in checks),
                              "note": "Changed bytes do not prove decryption or valid code."}
                if bad_pages or not comparison["all_changed"]:
                    print(f"comparison pending: unreadable_pages={bad_pages}, checks={checks}", flush=True)
                    time.sleep(args.interval)
                    continue
            with tempfile.TemporaryDirectory(dir=args.out) as scratch:
                passed, report = verify(img, pe, scratch, args.max_entropy, args.max_bad)
            print(f"verify: {report['verdict']} code_median_entropy={report['code_median_entropy']}", flush=True)
            print("  per-section entropy median/max: " + ", ".join(
                f"{e['name']}={e.get('entropy_median')}/{e.get('entropy_max')}" for e in report["sections"]), flush=True)
            print(f"  decode: {report['decode']}", flush=True)
            if not passed:
                cand = os.path.join(args.out, "candidate.runtime.bin")
                with open(cand, "wb") as stream:
                    stream.write(img)
                chown_to_invoker(cand)
                time.sleep(args.interval)
                continue
            bin_path = os.path.join(args.out, DUMP_NAME)
            tmp_path = bin_path + ".tmp"
            with open(tmp_path, "wb") as stream:
                stream.write(img)
            os.replace(tmp_path, bin_path)
            if os.path.exists(os.path.join(args.out, "candidate.runtime.bin")):
                os.remove(os.path.join(args.out, "candidate.runtime.bin"))
            chown_to_invoker(bin_path)
            meta = {
                "pid": pid,
                "image_base": hex(IMAGE_BASE),
                "size_of_image": size,
                "process_age_s_at_dump": round(age, 3),
                "full_read_attempts": attempts,
                "unreadable_pages": bad_pages,
                "sha256": hashlib.sha256(img).hexdigest(),
                "max_entropy": args.max_entropy,
                "max_bad": args.max_bad,
                "verification": report,
                "address_comparison": comparison,
                "same_as_disk_sample_share": compare_with_disk(img, pe),
                "poll_trace": trace[-200:],
                "file": DUMP_NAME,
            }
            json_path = os.path.join(args.out, "dump.json")
            with open(json_path, "w") as stream:
                json.dump(meta, stream, indent=2)
            chown_to_invoker(json_path)
            print(f"wrote {bin_path} ({size} bytes); dump.json written", flush=True)
            return 0
        finally:
            os.close(fd)
    print("no verified dump before the timeout", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
