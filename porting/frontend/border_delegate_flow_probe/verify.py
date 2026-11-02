#!/usr/bin/env python3
"""Fail closed on the complete ordered opt-in GUI diagnostic transcript."""
import re
import sys
from pathlib import Path

EVENTS = ("begin", "context", "before_slate", "slate_style", "parent_padding",
          "same_style", "null_style", "incompatible_style", "safe_padding",
          "delegate_shape", "mobile_signature", "complete")
LINE = re.compile(r"BORDERDELEGATEFLOW v=1 seq=(\d+) event=([a-z_]+) ok=([01])(?:\s|$)")
CONTEXT = re.compile(r"BORDERDELEGATE_CONTEXT viewport_interface=([01])(?:\s|$)")

def verify(text):
    rows = LINE.findall(text)
    expected = [(str(i), event, "1") for i, event in enumerate(EVENTS, 1)]
    contexts = CONTEXT.findall(text)
    if rows != expected or text.count("BORDERDELEGATEFLOW") != len(expected):
        raise ValueError(f"diagnostic transcript mismatch: {rows!r}")
    if len(contexts) != 1 or text.count("BORDERDELEGATE_CONTEXT") != 1:
        raise ValueError("missing or duplicate viewport context")
    return {"passed": True, "rows": len(EVENTS), "viewport_interface": bool(int(contexts[0]))}

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify.py RUNTIME_LOG")
    try:
        result = verify(Path(sys.argv[1]).read_text(errors="replace"))
    except ValueError as error:
        raise SystemExit(str(error))
    print(f"PASS {result['rows']} ordered observations; viewport interface={int(result['viewport_interface'])}")
