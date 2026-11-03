#!/usr/bin/env python3 -I
"""Reproduce the Niagara (NS_*) load probe against a COPY of the staged build.

Steps (see porting/niagara_load_probe.md):
  2. copy the staged LinuxNoEditor build to <work>/stage (skipped if present),
     recover each NS_ package's /Game closure with Cue4Inspect --save-closure
     and merge the recovered files (no-clobber) into <work>/stage/{FortniteLinux,Engine}.
  3. run the headless FortniteProbeAsset probe per system (bounded parallelism,
     per-run timeout) and classify the log.
  4. copy only verified NiagaraSystem loads with no error lines into
     porting/generated/converted/<FortniteGame/...relative path>.

Usage: python3 -I niagara_probe.py <work-dir> [--jobs 16] [--timeout 300]
           [--skip-closure] [--skip-probe] [--no-promote] [--limit N]
Never touches porting/native or the original staged build.
"""
import argparse, collections, json, os, re, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

PORTING = Path(__file__).resolve().parent.parent
EXTRACTED = Path("/mnt/vibrato/fortnite-extracted-full")
TOOL = Path("/mnt/vibrato/fortnite-asset-pipeline/tool-skeletal2")
STAGE_SRC = PORTING / "native/Saved/StagedBuilds/LinuxNoEditor"
CONVERTED = PORTING / "generated/converted"
EXTS = (".uasset", ".uexp", ".ubulk", ".uptnl")


def list_systems():
    out = []
    for root in ("Content", "Plugins"):
        for p in (EXTRACTED / "FortniteGame" / root).rglob("NS_*.uasset"):
            out.append(p.relative_to(EXTRACTED).with_suffix("").as_posix())
    return sorted(out)


def probe_path(pkg):
    """FortniteGame/Content/a/NS_X -> /Game/a/NS_X.NS_X ; plugin -> /<Plugin>/..."""
    name = pkg.rsplit("/", 1)[1]
    parts = pkg.split("/")
    if parts[1] == "Content":
        mount, rest = "Game", parts[2:]
    else:  # FortniteGame/Plugins/GameFeatures/<P>/Content/...
        i = parts.index("Content")
        mount, rest = parts[i - 1], parts[i + 1:]
    return "/" + mount + "/" + "/".join(rest) + "." + name


def sh(cmd, **kw):
    return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          text=True, **kw)


def closures(work, pkgs, jobs):
    # Cue4Inspect resolves --closure-output root as <tool>/../../../../generated/
    # frontend-probes, so run a copy of the tool at <work>/x/y/z/tool.
    tool = work / "x/y/z/tool"
    if not tool.exists():
        tool.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(TOOL, tool)
    (work / "generated/frontend-probes").mkdir(parents=True, exist_ok=True)
    (work / "logs/closure").mkdir(parents=True, exist_ok=True)

    def one(pkg):
        n = pkg.rsplit("/", 1)[1]
        out = work / "generated/frontend-probes" / n
        log = work / "logs/closure" / (n + ".log")
        if out.exists():
            return
        with open(log, "w") as f:
            subprocess.run(["dotnet", str(tool / "Cue4Inspect.dll"), pkg, str(EXTRACTED),
                            "--loose", "--save-closure", "--max-packages=2000",
                            f"--closure-output={out}"], stdout=f, stderr=subprocess.STDOUT,
                           timeout=900)
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(one, pkgs))


def merge(work):
    """Merge recovered closure files (FortniteGame and Engine Content/Plugins), no-clobber."""
    stage = work / "stage"
    for d in sorted((work / "generated/frontend-probes").iterdir()):
        for top, base in (("FortniteGame", "FortniteLinux"), ("Engine", "Engine")):
            for sub in ("Content", "Plugins"):
                src = d / top / sub
                for f in src.rglob("*") if src.exists() else []:
                    if f.is_file() and f.suffix in EXTS:
                        dst = stage / base / sub / f.relative_to(src)
                        if not dst.exists():
                            dst.parent.mkdir(parents=True, exist_ok=True)
                            shutil.copy2(f, dst)


def closure_stats(work, pkgs):
    sizes = {}
    for p in pkgs:
        n = p.rsplit("/", 1)[1]
        try:
            txt = (work / "logs/closure" / (n + ".log")).read_text(errors="replace")
            m = re.search(r"^Closure: (\d+) visited, (\d+) failed", txt, re.M)
            sizes[n] = int(m.group(1)) if m else None
        except OSError:
            sizes[n] = None
    return sizes


ERR = re.compile(r"(Error|Fatal|Failed|Can't find file|Failed to load|ensure|Assertion)", re.I)


EXTRA_PROBE_ARGS = []


def run_probe(work, pkg, timeout):
    n = pkg.rsplit("/", 1)[1]
    log = work / "logs/probe" / (n + ".log")
    log.parent.mkdir(parents=True, exist_ok=True)
    for old in log.parent.glob(n + "-backup-*.log"):
        old.unlink()
    try:
        r = subprocess.run(
            ["./FortniteLinux.sh", "-nullrhi", "-nosound", "-unattended",
             f"-FortniteProbeAsset={probe_path(pkg)}", "-ExecCmds=Quit", f"-abslog={log}", *EXTRA_PROBE_ARGS],
            cwd=work / "stage", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            timeout=timeout)
        rc = r.returncode
    except subprocess.TimeoutExpired:
        rc = "timeout"
    return classify(pkg, log, rc)


# Per-launch messages unrelated to the probed asset: the stage lacks the Water plugin content.
BENIGN = ("CDO Constructor (WaterSubsystem)", "GameUserSettings.ini")


def classify(pkg, log, rc):
    text = log.read_text(errors="replace") if log.exists() else ""
    cls = None
    errors = []
    for line in text.splitlines():
        m = re.search(r"Fortnite asset probe: \S+ -> (\S+)", line)
        if m:
            cls = m.group(1)
        if ("Fortnite asset probe" not in line and "LogInit: Command Line" not in line
                and not any(b in line for b in BENIGN)
                and re.search(r"\b(Error|Fatal)\b|Warning: Failed|Can't find file|Failed to load", line)):
            errors.append(re.sub(r"^\[[^\]]*\]\[\s*\d+\]", "", line).strip())
    ok = cls == "NiagaraSystem" and not errors and rc == 0
    cause = root_cause(text, errors)
    return {"root_cause": cause, "package": pkg, "probe": probe_path(pkg), "class": cls, "rc": rc, "ok": ok,
            "first_error": errors[0] if errors else None, "error_count": len(errors)}


def root_cause(text, errors):
    """Collapse a failing log into a short root cause (first matching rule wins)."""
    for needle, label in (
            ("FShaderMapBase::Serialize", "UMaterial inline shader map deserialization fatal (cooked Windows shader maps; FShaderMapBase::Serialize)"),
            ("UStaticMesh::Serialize", "UStaticMesh render data deserialization fatal (FPositionVertexBuffer::Serialize array element-size mismatch)")):
        if needle in text:
            return label
    m = re.search(r"FortniteLinux!(U\w+::Serialize)", text)
    if m:
        return f"deserialization fatal in {m.group(1)}"
    for e in errors:
        m = re.search(r"DoesPackageExist: '(/[^/']+)/", e) or re.search(r"find object 'Object (/[^/']+)/", e)
        if m:
            return f"unmounted plugin/content root {m.group(1)} (no .uplugin/mount in stage)"
    return None


def reason_key(r):
    if r.get("root_cause"):
        return r["root_cause"]
    e = r["first_error"]
    if r["rc"] == "timeout":
        return "timeout"
    if e is None:
        return f"no NiagaraSystem result (class={r['class']}, rc={r['rc']})"
    e = re.sub(r"/[A-Za-z0-9_/.\-]+", lambda m: m.group(0) if m.group(0).startswith("/Script/") else "<path>", e)
    e = re.sub(r"0x[0-9a-fA-F]+|\b\d+\b", "N", e)
    return e[:200]


def promote(work, results):
    n = 0
    for r in results:
        if not r["ok"]:
            continue
        for ext in EXTS:
            src = work / "stage/FortniteLinux" / r["package"].split("/")[1] / ("/".join(r["package"].split("/")[2:]) + ext)
            if src.exists():
                dst = CONVERTED / (r["package"] + ext)
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src, dst)
                n += ext == ".uasset"
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("work", type=Path)
    ap.add_argument("--jobs", type=int, default=16)
    ap.add_argument("--timeout", type=int, default=300)
    ap.add_argument("--skip-closure", action="store_true")
    ap.add_argument("--skip-probe", action="store_true")
    ap.add_argument("--no-promote", action="store_true")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--probe-arg", action="append", default=[], help="extra engine command-line argument, e.g. -FortniteSkipUnconvertedLegacyShaderMaps")
    ap.add_argument("--reclassify", action="store_true", help="re-read existing probe logs/results.json instead of rerunning")
    a = ap.parse_args()
    work = a.work.resolve()
    if str(work).startswith(str(PORTING / "native")):
        sys.exit("work dir must not be inside porting/native")
    EXTRA_PROBE_ARGS.extend(a.probe_arg)
    jobs = min(a.jobs, 16)
    pkgs = list_systems()[: a.limit]
    print(f"{len(pkgs)} NS_ systems")
    work.mkdir(parents=True, exist_ok=True)
    if not (work / "stage").exists():
        shutil.copytree(STAGE_SRC, work / "stage", symlinks=True)
    if not a.skip_closure:
        closures(work, pkgs, jobs)
    merge(work)
    sizes = closure_stats(work, pkgs)
    v = sorted(x for x in sizes.values() if x)
    if v:
        print(f"closure sizes: min {v[0]} median {v[len(v)//2]} max {v[-1]} mean {sum(v)/len(v):.1f}")
    if a.skip_probe:
        return
    if a.reclassify:
        old = {r["package"]: r["rc"] for r in json.loads((work / "results.json").read_text())}
        results = [classify(p, work / "logs/probe" / (p.rsplit("/", 1)[1] + ".log"), old.get(p)) for p in pkgs]
    else:
        with ThreadPoolExecutor(jobs) as ex:
            results = list(ex.map(lambda p: run_probe(work, p, a.timeout), pkgs))
    for r in results:
        r["closure_size"] = sizes.get(r["package"].rsplit("/", 1)[1])
    (work / "results.json").write_text(json.dumps(results, indent=1))
    ok = [r for r in results if r["ok"]]
    print(f"loaded {len(ok)} / failed {len(results) - len(ok)}")
    for k, c in collections.Counter(reason_key(r) for r in results if not r["ok"]).most_common(15):
        print(f"{c:4d}  {k}")
    if not a.no_promote:
        print("promoted", promote(work, results), "systems ->", CONVERTED)


if __name__ == "__main__":
    main()
