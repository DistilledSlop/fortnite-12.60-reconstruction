#!/usr/bin/env python3
"""Isolate which staged direct Blueprint causes Apollo's Linux load failure."""

from __future__ import annotations

import argparse
import resource
import subprocess
from pathlib import Path


BLUEPRINTS = {
    "shadow": "ContentCreationTools/ShadowProxyGenerator/Blueprints/BP_DisplayShadowProxy_01",
    "storm": "Athena/SafeZone/BP_SafeZoneStorm",
    "poi": "Athena/Environments/Landscape/Blueprints/BP_SetupNamedPoiColors",
    "mapinfo": "Athena/Balance/MapInfos/DefaultMapInfo",
}


def disable_core_dumps() -> None:
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--timeout", type=int, default=30)
    parser.add_argument("--only", choices=("none", *BLUEPRINTS, "all"),
                        help="run one package selection")
    args = parser.parse_args()
    if args.timeout < 1:
        parser.error("--timeout must be positive")

    porting = Path(__file__).resolve().parent
    stage = porting / "generated/linux-stage/LinuxNoEditor"
    content = stage / "FortniteLinux/Content"
    executable = stage / "FortniteLinux/Binaries/Linux/FortniteLinux"
    if not executable.is_file():
        parser.error(f"Missing staged executable: {executable}")

    originals = {
        name: [content / (stem + suffix) for suffix in (".uasset", ".uexp")]
        for name, stem in BLUEPRINTS.items()
    }
    for paths in originals.values():
        for path in paths:
            if not path.is_file() or path.with_suffix(path.suffix + ".disabled").exists():
                parser.error(f"Package is absent or already disabled: {path}")

    output = porting / "generated/apollo-blueprint-isolation"
    output.mkdir(parents=True, exist_ok=True)
    for enabled in ((), *[(name,) for name in BLUEPRINTS], tuple(BLUEPRINTS)):
        label = "none" if not enabled else "-".join(enabled)
        if args.only and args.only != label and not (args.only == "all" and len(enabled) == len(BLUEPRINTS)):
            continue
        renamed: list[tuple[Path, Path]] = []
        try:
            for name, paths in originals.items():
                if name in enabled:
                    continue
                for path in paths:
                    disabled = path.with_suffix(path.suffix + ".disabled")
                    path.rename(disabled)
                    renamed.append((disabled, path))

            command = [
                str(executable),
                "/Game/Athena/Apollo/Maps/Apollo_Terrain",
                "-nullrhi", "-nosound", "-unattended", "-ExecCmds=Quit", "-log",
            ]
            try:
                result = subprocess.run(
                    command,
                    cwd=stage,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    errors="replace",
                    timeout=args.timeout,
                    preexec_fn=disable_core_dumps,
                    check=False,
                )
                code = result.returncode
                log = result.stdout
            except subprocess.TimeoutExpired as error:
                code = "timeout"
                log = error.stdout or b""
                if isinstance(log, bytes):
                    log = log.decode(errors="replace")
            (output / f"{label}.log").write_text(log)
            lines = log.splitlines()
            assertions = [line.strip() for line in lines if line.startswith("Assertion failed:")]
            field_errors = [line.strip() for line in lines if "Unresolved cooked field path:" in line]
            missing_classes = sum("Could not find class" in line for line in lines)
            loaded = any("LogLoad: Took" in line and "Apollo_Terrain" in line for line in lines)
            detail = assertions[0] if assertions else (field_errors[0] if field_errors else "no assertion")
            print(
                f"{label}: exit={code}; loaded={loaded}; missing_classes={missing_classes}; {detail[:230]}",
                flush=True,
            )
        finally:
            for disabled, path in reversed(renamed):
                disabled.rename(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
