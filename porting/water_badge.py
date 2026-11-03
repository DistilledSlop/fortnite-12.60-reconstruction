#!/usr/bin/env python3
"""Estimate gallons of water used from Codex token usage.

    python3 porting/water_badge.py            # refresh porting/progress/water.json
    python3 porting/progress/publish.py       # then republish the README badges

publish.py reads water.json (not ~/.codex) so its output stays deterministic and
`publish.py --check` keeps working.

Estimate: ~0.000085 gal (0.32 mL) per query, a published vendor figure, spread
over an assumed ~400 tokens per query = 2.1e-7 gal/token. Applied to uncached
input + output tokens summed from ~/.codex/sessions; cached input reads are
excluded. A rough order-of-magnitude figure, not a measurement.
"""
import glob
import json
import os
from pathlib import Path

GAL_PER_TOKEN = 0.000085 / 400
OUT = Path(__file__).resolve().parent / "progress" / "water.json"


def codex_tokens():
    """Uncached input + output tokens, from each session's final running total."""
    billed = 0
    for f in glob.glob(os.path.expanduser("~/.codex/sessions/**/*.jsonl"), recursive=True):
        tot = None
        with open(f, errors="ignore") as fp:
            for line in fp:
                if '"token_count"' not in line:
                    continue
                try:
                    tu = ((json.loads(line).get("payload") or {}).get("info") or {}).get("total_token_usage")
                except Exception:
                    continue
                if tu:
                    tot = tu
        if tot:
            billed += max(0, tot.get("input_tokens", 0) - tot.get("cached_input_tokens", 0))
            billed += tot.get("output_tokens", 0)
    return billed


if __name__ == "__main__":
    tokens = codex_tokens()
    gal = round(tokens * GAL_PER_TOKEN, 1)
    OUT.write_text(json.dumps({"gallons": gal, "tokens": tokens, "gal_per_token": GAL_PER_TOKEN}, indent=1) + "\n")
    print(f"{tokens:,} tokens -> {gal} gal -> {OUT}")
