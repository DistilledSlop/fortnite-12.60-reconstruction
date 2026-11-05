#!/usr/bin/env python3
"""Temporarily stage one recovered Apollo streaming level for a Linux load probe."""

from __future__ import annotations

import argparse
import hashlib
import json
import resource
import subprocess
from pathlib import Path


LEVEL_FILES = {
    "A1": (".umap", ".uexp", ".ubulk"),
    "Parent": (".umap", ".uexp"),
}


def disable_core_dumps() -> None:
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("level", choices=LEVEL_FILES)
    parser.add_argument("--timeout", type=int, default=20)
    args = parser.parse_args()
    if args.timeout < 1:
        parser.error("--timeout must be positive")

    porting = Path(__file__).resolve().parent
    stage = porting / "generated/linux-stage/LinuxNoEditor"
    game = stage / "FortniteLinux"
    executable = game / "Binaries/Linux/FortniteLinux"
    holding = porting / "generated/apollo-streaming-hold"
    destination = game / "Content/Athena/Apollo/Maps/Landscape"
    level_name = f"Apollo_Terrain_LS_{args.level}"
    paths = [
        (holding / f"{level_name}{suffix}", destination / f"{level_name}{suffix}")
        for suffix in LEVEL_FILES[args.level]
    ]
    if not executable.is_file():
        parser.error(f"Missing staged executable: {executable}")
    for source, target in paths:
        if not source.is_file() or target.exists():
            parser.error(f"Expected held source and absent staged target: {source}, {target}")

    hashes = []
    for source, _ in paths:
        with source.open("rb") as stream:
            digest = hashlib.file_digest(stream, "sha256").hexdigest()
        hashes.append({"source": str(source.resolve()), "sha256": digest})

    destination.mkdir(parents=True, exist_ok=True)
    moved: list[tuple[Path, Path]] = []
    try:
        for source, target in paths:
            source.rename(target)
            moved.append((target, source))
        command = [
            str(executable), "/Game/Athena/Apollo/Maps/Apollo_Terrain",
            "-nullrhi", "-nosound", "-unattended", "-log",
            "-FortniteLegacyFieldPaths", "-FortniteLegacyLandscapeData",
        ]
        try:
            result = subprocess.run(
                command, cwd=stage, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, text=True, errors="replace",
                timeout=args.timeout, preexec_fn=disable_core_dumps,
                check=False,
            )
            code = result.returncode
            log = result.stdout
        except subprocess.TimeoutExpired as error:
            code = "timeout"
            log = error.stdout or b""
            if isinstance(log, bytes):
                log = log.decode(errors="replace")
        log_path = porting / f"generated/apollo-streaming-{args.level.lower()}.log"
        log_path.write_text(log)
        log_path.with_suffix(".json").write_text(json.dumps({
            "source": "held streaming files; extraction provenance not verified",
            "packages": hashes, "command": command, "cwd": str(stage), "exit": code,
        }, indent=2) + "\n")
        lines = log.splitlines()
        failures = [
            line.strip() for line in lines
            if "Assertion failed:" in line or "FAsyncArchive" in line
        ]
        missing = sum("Could not find class" in line for line in lines)
        print(f"{level_name}: exit={code}; missing_classes={missing}")
        for line in failures[:5]:
            print(line[:300])
        print(f"Log: {log_path}")
    finally:
        for target, source in reversed(moved):
            target.rename(source)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
