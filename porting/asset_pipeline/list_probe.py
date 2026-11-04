#!/usr/bin/env python3
"""Batch asset load probe over a staged FortniteLinux build.

    python3 -I list_probe.py <stage-dir> <objects.txt> --out results.jsonl
        [--batch 50] [--jobs 16] [--timeout 900] [--probe-arg=-Flag ...]

`objects.txt` has one UE object path per line (for example `/Game/UI/Foo.Foo`, or
`/Game/Chars/BP_X.BP_X_C` for a Blueprint class). Each batch runs one
`FortniteLinux -FortniteProbeAssetList=<file>` process, which logs
`Fortnite asset probe begin: <path>` before and `Fortnite asset probe: <path> -> <Class|LOAD_FAILED>`
after every load. A crash is attributed to the last asset that began without a result; the
assets after it are re-queued. Error lines between begin and result are kept per asset
(`errors`, first 5). Output is one JSON object per line:
{object, status: loaded|load_failed|crashed|timeout, class, errors}.
The runner disables core dumps for the child processes.
"""
import argparse
import json
import os
import re
import resource
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

BEGIN = re.compile(r"Fortnite asset probe begin: (\S+)")
DONE = re.compile(r"Fortnite asset probe: (\S+) -> (\S+)")
ERROR = re.compile(r"\b(Error|Fatal)\b|Assertion failed|Corrupt data")
# Messages that appear on every launch or do not stop a load.
BENIGN = ("CDO Constructor (WaterSubsystem)", "GameUserSettings.ini", "/Water/", "/Water.",
          "Attempt to build nav collision data")


def _no_core():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def parse_log(text):
    """Return ({object: {class, errors}} for finished loads, started-without-result object or None)."""
    done, errors, current = {}, [], None
    for line in text.splitlines():
        m = BEGIN.search(line)
        if m:
            current, errors = m.group(1), []
            continue
        m = DONE.search(line)
        if m and m.group(1) == current:
            done[current] = {"class": m.group(2), "errors": errors[:5]}
            current = None
            continue
        if current and ERROR.search(line) and not any(b in line for b in BENIGN):
            errors.append(re.sub(r"^\[[^\]]*\]\[\s*\d+\]", "", line).strip()[:300])
    return done, current


def run_batch(stage, objects, extra, timeout, tmp):
    """Probe `objects` in as many processes as a crash requires. Returns result dicts."""
    results, todo = [], list(objects)
    while todo:
        with tempfile.NamedTemporaryFile("w", dir=tmp, suffix=".txt", delete=False) as lst:
            lst.write("\n".join(todo) + "\n")
        log = Path(lst.name).with_suffix(".log")
        status = "ok"
        try:
            subprocess.run(["./FortniteLinux.sh", "-nullrhi", "-nosound", "-unattended", "-ExecCmds=Quit",
                            f"-FortniteProbeAssetList={lst.name}", f"-abslog={log}", *extra],
                           cwd=stage, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           timeout=timeout, preexec_fn=_no_core)
        except subprocess.TimeoutExpired:
            status = "timeout"
        text = log.read_text(errors="replace") if log.exists() else ""
        done, pending = parse_log(text)
        for obj in todo:
            if obj in done:
                d = done[obj]
                loaded = d["class"] != "LOAD_FAILED" and not d["errors"]
                results.append({"object": obj, "status": "loaded" if loaded else "load_failed",
                                "class": d["class"], "errors": d["errors"]})
        remaining = [o for o in todo if o not in done]
        for p in (lst.name, str(log)):
            try:
                os.unlink(p)
            except OSError:
                pass
        if not remaining:
            break
        if pending is None or pending not in remaining:
            # the process died before it began anything (bad stage?): fail everything left
            for o in remaining:
                results.append({"object": o, "status": "crashed", "class": None,
                                "errors": ["no probe output; process exited early"]})
            break
        results.append({"object": pending, "status": "timeout" if status == "timeout" else "crashed",
                        "class": None, "errors": [l for l in text.splitlines() if "Assertion failed" in l][:1]})
        todo = [o for o in remaining if o != pending]
    return results


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stage", type=Path)
    ap.add_argument("objects", type=Path)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--batch", type=int, default=50)
    ap.add_argument("--jobs", type=int, default=16)
    ap.add_argument("--timeout", type=int, default=900)
    ap.add_argument("--probe-arg", action="append", default=[])
    ap.add_argument("--resume", action="store_true", help="skip objects already in --out")
    a = ap.parse_args()
    objects = [l.strip() for l in a.objects.read_text().splitlines() if l.strip()]
    seen = set()
    if a.resume and a.out.exists():
        seen = {json.loads(l)["object"] for l in a.out.read_text().splitlines() if l.strip()}
    objects = [o for o in objects if o not in seen]
    batches = [objects[i:i + a.batch] for i in range(0, len(objects), a.batch)]
    a.out.parent.mkdir(parents=True, exist_ok=True)
    tmp = tempfile.mkdtemp(prefix="listprobe-", dir=a.out.parent)
    done = 0
    with a.out.open("a") as out, ThreadPoolExecutor(min(a.jobs, 16)) as ex:
        for res in ex.map(lambda b: run_batch(a.stage, b, a.probe_arg, a.timeout, tmp), batches):
            for r in res:
                out.write(json.dumps(r, sort_keys=True) + "\n")
            out.flush()
            done += len(res)
            print(f"{done}/{len(objects)}", file=sys.stderr, flush=True)
    os.rmdir(tmp)
    return 0


if __name__ == "__main__":
    sys.exit(main())
