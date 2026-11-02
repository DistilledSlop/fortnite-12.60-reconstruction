#!/usr/bin/env python3
"""Fail closed on the opt-in GUI diagnostic's complete ordered transcript."""
import re
import sys
from pathlib import Path

EVENTS = ("begin", "context", "original_row", "original_handle", "before_slate",
          "slate_order", "hold_visible", "subscribed", "registry_available",
          "borrow_binding", "single_detaches", "array_detaches", "showkeys_off",
          "showkeys_on", "release", "registry_cleanup", "restored", "complete")
LINE = re.compile(r"ACTIONWIDGETFLOW v=1 seq=(\d+) event=([a-z_]+) ok=([01])(?:\s|$)")

def verify(text):
    rows = LINE.findall(text)
    expected = [(str(i), event, "1") for i, event in enumerate(EVENTS, 1)]
    if rows != expected or text.count("ACTIONWIDGETFLOW") != len(expected):
        raise ValueError(f"diagnostic transcript mismatch: {rows!r}")
    return {"passed": True, "rows": len(EVENTS), "scope": "controlled UObject/Slate action-widget flow; no authentic frontend, pixels, audio or full input routing claim"}

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify.py RUNTIME_LOG")
    try:
        verify(Path(sys.argv[1]).read_text(errors="replace"))
    except ValueError as error:
        raise SystemExit(str(error))
    print(f"PASS {len(EVENTS)} ordered observations")
