#!/usr/bin/env python3
"""Extract static API path templates from the supplied Windows client.

This is a discovery aid, not a complete protocol specification. Endpoints
assembled at runtime, request methods, bodies, and response schemas require
separate analysis.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path


ROUTE = re.compile(r"/api/(?:game/v2|public|oauth)/[^\s\x00\"']+")


def routes_in_binary(binary: Path) -> list[str]:
    found: set[str] = set()
    for encoding in ("s", "l"):
        process = subprocess.Popen(
            ["strings", "-a", f"-e{encoding}", "-n", "8", str(binary)],
            stdout=subprocess.PIPE,
            text=True,
            errors="replace",
        )
        assert process.stdout is not None
        for line in process.stdout:
            match = ROUTE.search(line)
            if match:
                found.add(match.group().rstrip(".,;)}]"))
        if process.wait() != 0:
            raise RuntimeError(f"strings failed while scanning {binary}")
    return sorted(found)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    default_root = Path(__file__).resolve().parent.parent
    parser.add_argument(
        "--binary",
        type=Path,
        default=default_root / "Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe",
    )
    args = parser.parse_args()
    routes = routes_in_binary(args.binary)
    print(json.dumps({"binary": args.binary.name, "route_count": len(routes), "routes": routes}, indent=2))


if __name__ == "__main__":
    main()
