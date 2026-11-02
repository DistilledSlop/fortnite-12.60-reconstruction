#!/usr/bin/env python3
"""Fail closed on the controlled GUI production-button transcript."""
import re
import sys
from pathlib import Path

EVENTS = ("begin", "context", "empty_initialized", "empty_unbound", "one_shot_empty",
          "empty_weak_gc", "root_initialized", "root_slot", "copied_fields", "five_bindings",
          "one_shot_root", "root_gc", "style_dimensions", "style_null_equal", "style_wrong_class", "style_reentry",
          "focus_selection", "click_toggle", "click_rejected", "double_click", "touch_press_release",
          "raw_commit_byte", "restored", "complete")
LINE = re.compile(r"BUTTONFLOW v=1 seq=(\d+) event=([a-z_]+) ok=([01])(?:\s|$)")

def verify(text):
    rows = LINE.findall(text)
    expected = [(str(i), event, "1") for i, event in enumerate(EVENTS, 1)]
    if rows != expected or text.count("BUTTONFLOW") != len(expected):
        raise ValueError(f"diagnostic transcript mismatch: {rows!r}")
    return {"passed": True, "rows": len(EVENTS), "scope": "controlled production initialization, delegates and style flow; no physical input, pixels or audio claim"}

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify.py RUNTIME_LOG")
    try:
        verify(Path(sys.argv[1]).read_text(errors="replace"))
    except ValueError as error:
        raise SystemExit(str(error))
    print(f"PASS {len(EVENTS)} ordered observations")
