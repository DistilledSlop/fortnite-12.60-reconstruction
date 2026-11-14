#!/usr/bin/env python3
"""Summarise FortMovementPair server logs joined with Client* RPCs in network-rpc.jsonl."""

from __future__ import annotations

import argparse
import bisect
import datetime as dt
import json
import math
import re
import statistics
import sys
from pathlib import Path
from typing import Any

PAIR_LINE_RE = re.compile(
    r"^\[(?P<ts>\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]\[\s*\d+\]LogNetPlayerMovement:\s+FortMovementPair\s+(?P<body>.+)$"
)
KV_RE = re.compile(
    r"(\w+)=(X=[-+]?\d+(?:\.\d+)?\s+Y=[-+]?\d+(?:\.\d+)?\s+Z=[-+]?\d+(?:\.\d+)?|\S+)"
)
VEC_RE = re.compile(
    r"X=([-+]?\d+(?:\.\d+)?)\s+Y=([-+]?\d+(?:\.\d+)?)\s+Z=([-+]?\d+(?:\.\d+)?)"
)

MOVEMENT_RPCS = {
    "ClientAckGoodMove",
    "ClientAdjustPosition",
    "ClientVeryShortAdjustPosition",
}
CORRECTION_RPCS = {
    "ClientAdjustPosition",
    "ClientVeryShortAdjustPosition",
}


def parse_ue_utc_ms(ts: str) -> int:
    parsed = dt.datetime.strptime(ts, "%Y.%m.%d-%H.%M.%S:%f").replace(tzinfo=dt.timezone.utc)
    return int(round(parsed.timestamp() * 1000.0))


def parse_vec(value: str) -> tuple[float, float, float] | None:
    m = VEC_RE.fullmatch(value.strip())
    if not m:
        return None
    return float(m.group(1)), float(m.group(2)), float(m.group(3))


def vec_norm(v: tuple[float, float, float] | None) -> float:
    if v is None:
        return 0.0
    return math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])


def parse_pairs(server_log: Path) -> list[dict[str, Any]]:
    pairs: list[dict[str, Any]] = []
    with server_log.open("r", encoding="utf-8", errors="replace") as fh:
        for line_no, raw in enumerate(fh, 1):
            m = PAIR_LINE_RE.match(raw.rstrip("\r\n"))
            if not m:
                continue
            utc_ms = parse_ue_utc_ms(m.group("ts"))
            fields: dict[str, str] = {}
            for km in KV_RE.finditer(m.group("body")):
                fields[km.group(1)] = km.group(2)
            error_vec = parse_vec(fields.get("error", ""))
            vel_vec = parse_vec(fields.get("velocity", ""))
            accel_vec = parse_vec(fields.get("accel", ""))
            pairs.append(
                {
                    "line_no": line_no,
                    "ts": m.group("ts"),
                    "utc_ms": utc_ms,
                    "pawn": fields.get("pawn", ""),
                    "time": float(fields.get("time", "0") or 0.0),
                    "physicsDt": float(fields.get("physicsDt", "0") or 0.0),
                    "error_raw": fields.get("error", ""),
                    "error_vec": error_vec,
                    "error_norm": vec_norm(error_vec),
                    "velocity_vec": vel_vec,
                    "vel_norm": vec_norm(vel_vec),
                    "accel_vec": accel_vec,
                    "accel_norm": vec_norm(accel_vec),
                    "clientMode": fields.get("clientMode", ""),
                    "serverMode": fields.get("serverMode", ""),
                    "clientBase": fields.get("clientBase", ""),
                    "serverBase": fields.get("serverBase", ""),
                    "floorBlocking": fields.get("floorBlocking", ""),
                    "floorWalkable": fields.get("floorWalkable", ""),
                    "floorDist": fields.get("floorDist", ""),
                    "floorComponent": fields.get("floorComponent", ""),
                    "ignoreError": fields.get("ignoreError", "0"),
                    "componentClientAuthority": fields.get("componentClientAuthority", "0"),
                    "managerClientAuthority": fields.get("managerClientAuthority", "0"),
                    "rpc": None,
                    "rpc_utc_ms": None,
                    "rpc_gap_ms": None,
                }
            )
    return pairs


def parse_rpcs(network_rpc: Path) -> list[dict[str, Any]]:
    rpcs: list[dict[str, Any]] = []
    if not network_rpc.is_file():
        return rpcs
    with network_rpc.open("r", encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            try:
                obj = json.loads(raw)
            except json.JSONDecodeError:
                continue
            if obj.get("event") != "rpc_send":
                continue
            fn = obj.get("function")
            if fn not in MOVEMENT_RPCS:
                continue
            if "outcome" in obj and obj["outcome"] != "submitted":
                continue
            utc_ms = obj.get("utc_unix_ms")
            if utc_ms is None:
                continue
            rpcs.append(
                {
                    "function": fn,
                    "utc_ms": int(utc_ms),
                    "world_time": obj.get("world_time"),
                    "actor_id": obj.get("actor_id"),
                }
            )
    return rpcs


def join_pairs_and_rpcs(
    pairs: list[dict[str, Any]],
    rpcs: list[dict[str, Any]],
    max_window_ms: int = 250,
) -> tuple[int, int]:
    """Match each Client* movement RPC to the nearest preceding/nearby unmatched FortMovementPair."""
    if not pairs or not rpcs:
        return 0, len(rpcs)

    pair_times = [p["utc_ms"] for p in pairs]
    used: set[int] = set()
    matched = 0
    unmatched = 0

    for rpc in rpcs:
        target = rpc["utc_ms"]
        pos = bisect.bisect_right(pair_times, target)
        # Prefer nearest unused pair within [-20ms, +max_window_ms] before rpc (or +-max_window_ms)
        best_idx = None
        best_score = None
        lo = max(0, pos - 8)
        hi = min(len(pairs), pos + 4)
        for idx in range(lo, hi):
            if idx in used:
                continue
            delta = target - pair_times[idx]  # expected small >= -5 ms
            if abs(delta) > max_window_ms:
                continue
            # Strongly prefer RPC occurring at or right after pair log line
            penalty = abs(delta) + (1000 if delta < -5 else 0)
            if best_score is None or penalty < best_score:
                best_score = penalty
                best_idx = idx
        if best_idx is None:
            unmatched += 1
            continue
        used.add(best_idx)
        pairs[best_idx]["rpc"] = rpc["function"]
        pairs[best_idx]["rpc_utc_ms"] = target
        pairs[best_idx]["rpc_gap_ms"] = target - pairs[best_idx]["utc_ms"]
        matched += 1
    return matched, unmatched


def compute_intervals(pairs: list[dict[str, Any]]) -> dict[str, Any]:
    last_by_pawn: dict[str, dict[str, Any]] = {}
    all_client_dts: list[float] = []
    moving_client_dts: list[float] = []
    moving_wall_dts: list[float] = []

    for p in pairs:
        pawn = p["pawn"]
        prev = last_by_pawn.get(pawn)
        if prev is not None:
            client_dt = p["time"] - prev["time"]
            wall_dt = (p["utc_ms"] - prev["utc_ms"]) / 1000.0
            p["client_dt"] = client_dt
            p["wall_dt"] = wall_dt
            if 0.0 < client_dt < 1.0:
                all_client_dts.append(client_dt)
                if p["accel_norm"] > 0.0 or p["vel_norm"] > 1.0:
                    moving_client_dts.append(client_dt)
                    if 0.0 <= wall_dt < 1.0:
                        moving_wall_dts.append(wall_dt)
        else:
            p["client_dt"] = None
            p["wall_dt"] = None
        last_by_pawn[pawn] = p

    return {
        "all_client_dt_median": statistics.median(all_client_dts) if all_client_dts else None,
        "moving_client_dt_median": statistics.median(moving_client_dts) if moving_client_dts else None,
        "moving_client_dt_p95": (
            sorted(moving_client_dts)[int(0.95 * (len(moving_client_dts) - 1))]
            if moving_client_dts
            else None
        ),
        "moving_wall_dt_median": statistics.median(moving_wall_dts) if moving_wall_dts else None,
        "moving_samples": len(moving_client_dts),
    }


def build_buckets(pairs: list[dict[str, Any]], bucket_seconds: float) -> list[dict[str, Any]]:
    if not pairs:
        return []
    t0 = pairs[0]["utc_ms"]
    buckets_map: dict[int, list[dict[str, Any]]] = {}
    for p in pairs:
        rel_s = (p["utc_ms"] - t0) / 1000.0
        b_idx = int(rel_s // bucket_seconds)
        buckets_map.setdefault(b_idx, []).append(p)

    rows: list[dict[str, Any]] = []
    for b_idx in sorted(buckets_map):
        items = buckets_map[b_idx]
        acks = sum(1 for x in items if x["rpc"] == "ClientAckGoodMove")
        full = sum(1 for x in items if x["rpc"] == "ClientAdjustPosition")
        short = sum(1 for x in items if x["rpc"] == "ClientVeryShortAdjustPosition")
        dts = [x["client_dt"] for x in items if x["client_dt"] is not None and 0.0 < x["client_dt"] < 1.0]
        errs = [x["error_norm"] for x in items]
        modes = sorted({f"{x['clientMode']}->{x['serverMode']}" for x in items})
        floors = sorted(
            {
                x["floorComponent"].split(".")[-1]
                for x in items
                if x["floorComponent"] and x["floorComponent"] != "None"
            }
        )
        rows.append(
            {
                "bucket_start_s": b_idx * bucket_seconds,
                "bucket_end_s": (b_idx + 1) * bucket_seconds,
                "moves": len(items),
                "acks": acks,
                "full_adjusts": full,
                "short_adjusts": short,
                "median_dt_s": statistics.median(dts) if dts else None,
                "max_error_cm": max(errs) if errs else 0.0,
                "modes": modes,
                "floors": floors[:3],
            }
        )
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", nargs="?", type=Path, help="Run directory containing server.log and network-rpc.jsonl")
    parser.add_argument("--server-log", type=Path, help="Explicit path to server.log")
    parser.add_argument("--network-rpc", type=Path, help="Explicit path to network-rpc.jsonl")
    parser.add_argument("--bucket-seconds", type=float, default=5.0, help="Bucket width in seconds (default: 5.0)")
    parser.add_argument("--max-corrections", type=int, default=40, help="Max corrected-move rows to print (0 = all)")
    parser.add_argument("--checkpoint-json", type=Path, help="Write summary metrics JSON to this path")
    parser.add_argument("--max-median-interval", type=float, default=None, help="Fail (exit 1) if moving median move interval exceeds this threshold in seconds")
    parser.add_argument("--json", action="store_true", help="Print JSON summary instead of text tables")
    args = parser.parse_args()

    server_log = args.server_log or (args.run_dir / "server.log" if args.run_dir else None)
    network_rpc = args.network_rpc or (args.run_dir / "network-rpc.jsonl" if args.run_dir else None)

    if server_log is None or not server_log.is_file():
        print(f"error: server log not found: {server_log}", file=sys.stderr)
        return 2

    pairs = parse_pairs(server_log)
    rpcs = parse_rpcs(network_rpc) if network_rpc else []
    matched, unmatched = join_pairs_and_rpcs(pairs, rpcs)
    intervals = compute_intervals(pairs)
    buckets = build_buckets(pairs, args.bucket_seconds)

    rpc_counts = {name: 0 for name in MOVEMENT_RPCS}
    for r in rpcs:
        rpc_counts[r["function"]] += 1

    corrections = [p for p in pairs if p["rpc"] in CORRECTION_RPCS]
    t0 = pairs[0]["utc_ms"] if pairs else 0
    max_err = max((p["error_norm"] for p in pairs), default=0.0)
    median_moving_dt = intervals["moving_client_dt_median"]
    interval_ok = True
    if args.max_median_interval is not None:
        interval_ok = median_moving_dt is not None and median_moving_dt <= args.max_median_interval

    summary = {
        "server_log": str(server_log),
        "network_rpc": str(network_rpc) if network_rpc and network_rpc.is_file() else None,
        "pairs": len(pairs),
        "rpcs_total": len(rpcs),
        "rpcs_matched": matched,
        "rpcs_unmatched": unmatched,
        "acks": rpc_counts["ClientAckGoodMove"],
        "full_adjusts": rpc_counts["ClientAdjustPosition"],
        "short_adjusts": rpc_counts["ClientVeryShortAdjustPosition"],
        "max_error_cm": round(max_err, 3),
        "median_move_interval_s": round(median_moving_dt, 6) if median_moving_dt is not None else None,
        "p95_move_interval_s": (
            round(intervals["moving_client_dt_p95"], 6)
            if intervals["moving_client_dt_p95"] is not None
            else None
        ),
        "median_wall_interval_s": (
            round(intervals["moving_wall_dt_median"], 6)
            if intervals["moving_wall_dt_median"] is not None
            else None
        ),
        "moving_samples": intervals["moving_samples"],
        "max_median_interval_s": args.max_median_interval,
        "interval_check_passed": interval_ok,
        "buckets": buckets,
    }

    if args.checkpoint_json:
        args.checkpoint_json.parent.mkdir(parents=True, exist_ok=True)
        args.checkpoint_json.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")

    if args.json:
        print(json.dumps(summary, indent=2))
    else:
        print("=== FortMovementPair Summary ===")
        print(
            f"pairs={len(pairs)} rpcs={len(rpcs)} (matched={matched}, unmatched={unmatched}) "
            f"acks={rpc_counts['ClientAckGoodMove']} full={rpc_counts['ClientAdjustPosition']} "
            f"short={rpc_counts['ClientVeryShortAdjustPosition']}"
        )
        print(
            f"median_moving_dt={median_moving_dt:.6f}s " if median_moving_dt is not None else "median_moving_dt=None "
            ,
            end="",
        )
        print(
            f"p95_moving_dt={intervals['moving_client_dt_p95']:.6f}s "
            if intervals["moving_client_dt_p95"] is not None
            else "p95_moving_dt=None ",
            end="",
        )
        print(
            f"moving_samples={intervals['moving_samples']} max_error={max_err:.3f}cm"
        )
        if args.max_median_interval is not None:
            status = "PASS" if interval_ok else "FAIL"
            print(f"interval_gate (<={args.max_median_interval:.3f}s): {status}")

        print(f"\n=== {args.bucket_seconds:g}s Buckets ===")
        print(
            f"{'window_s':<13} {'moves':>6} {'ack':>6} {'full':>5} {'short':>5} "
            f"{'med_dt':>8} {'max_err':>9} {'modes':<14} floors"
        )
        for b in buckets:
            win = f"{b['bucket_start_s']:.0f}-{b['bucket_end_s']:.0f}"
            med = f"{b['median_dt_s']:.4f}" if b["median_dt_s"] is not None else "-"
            modes = ",".join(b["modes"])
            floors = ",".join(b["floors"])
            print(
                f"{win:<13} {b['moves']:>6} {b['acks']:>6} {b['full_adjusts']:>5} "
                f"{b['short_adjusts']:>5} {med:>8} {b['max_error_cm']:>9.3f} {modes:<14} {floors}"
            )

        shown = corrections if args.max_corrections == 0 else corrections[: args.max_corrections]
        print(f"\n=== Corrected Moves ({len(shown)}/{len(corrections)} shown) ===")
        for c in shown:
            rel_s = (c["utc_ms"] - t0) / 1000.0
            dt_str = f"{c['client_dt']:.4f}" if c["client_dt"] is not None else "-"
            floor_short = c["floorComponent"].split(".")[-1] if c["floorComponent"] else "None"
            print(
                f"t={rel_s:7.3f}s rpc={c['rpc']:<29} gap={c['rpc_gap_ms']:>3}ms "
                f"dt={dt_str} physDt={c['physicsDt']:.4f} mode={c['clientMode']}->{c['serverMode']} "
                f"err={c['error_norm']:.3f} ({c['error_raw']}) floor={floor_short}"
            )

    return 0 if interval_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
