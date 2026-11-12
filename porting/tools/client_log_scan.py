#!/usr/bin/env python3
"""Summarise an Unreal log (client from a Wine run of the original client, or our server).

    python3 -I tools/client_log_scan.py <FortniteGame.log> [--server <server.log>] [--json]
                                        [--examples N] [--top N]

Per category group: counts of Warning/Error/Fatal (and Display) lines, first/last timestamp,
and the most frequent distinct messages after normalising digits / hex / GUIDs. Then a "top
normalised messages overall" list and a "milestones" list (first timestamp of known progress
markers). Secrets (tokens, 32-hex account ids, emails, bearer strings, EncryptionToken,
AuthTicket, exchange_code) are redacted before anything is printed.

UE line shape: `[2026.11.12-18.31.02:123][ 45]LogNet: Warning: text`. Lines without that prefix
but shaped `LogX: [Level:] text` are still parsed; their timestamp is the last one seen.
Claims here are "observed" log facts only; nothing is interpreted.
"""
import argparse
import collections
import json
import re
import sys

# ---- configuration (extend here) -------------------------------------------------------

# Group name -> matcher. Exact category names first, then prefixes. Anything else is "other".
EXACT_GROUPS = [
    "LogNet", "LogNetPackageMap", "LogRep", "LogNetTraffic", "LogNetPlayerMovement",
    "LogAbilitySystem", "LogStreaming", "LogLoad", "LogPlayerController", "LogGameMode",
    "LogGameState", "LogScript",
]
PREFIX_GROUPS = [("LogFort*", "LogFort"), ("LogAthena*", "LogAthena")]
GROUP_ORDER = ["LogNet", "LogNetPackageMap", "LogRep", "LogNetTraffic", "LogNetPlayerMovement",
               "LogFort*", "LogAthena*", "LogAbilitySystem", "LogStreaming", "LogLoad",
               "LogPlayerController", "LogGameMode", "LogGameState", "LogScript", "other"]

# Milestone substrings (case-sensitive, matched anywhere in a raw line). Easy to extend.
CLIENT_MILESTONES = [
    "Welcomed by server", "Join succeeded", "NetChecksumMismatch", "SerializeNewActor failed",
    "LoadingScreen", "GamePhase", "Aircraft", "SafeZone", "Died", "Eliminated", "Victory",
    "ReturnToMainMenu", "NetworkFailure", "Connection closed",
]
SERVER_MILESTONES = ["MatchFlow:", "Inventory:", "Pawn tracker", "Test floor"]

SEVERITIES = ("Warning", "Error", "Fatal")
MAX_MSG = 150  # truncate printed messages

# ---- parsing / normalising / redaction -------------------------------------------------

LINE_RE = re.compile(
    r"^(?:\[(\d{4}\.\d\d\.\d\d-\d\d\.\d\d\.\d\d:\d+)\]\[\s*\d+\])?"
    r"([A-Za-z][A-Za-z0-9_]*):\s+(?:(Warning|Error|Fatal|Display|Verbose|VeryVerbose):\s*)?(.*)$")

_TOK = r"[A-Za-z0-9._~+/=\-]{10,}"
REDACTIONS = [
    (re.compile(r"(?i)(EncryptionToken|[\w.]*(?:access_?token|refresh_?token|id_?token|exchange_?code|"
                r"password|secret|api_?key|ticket))\s*=\s*\"?[^\s&\"',;]+"), r"\1=<redacted>"),
    (re.compile(r"(?i)\b(AuthTicket|exchange_?code|Authorization)\b(\s*[:=]?\s*)" + _TOK), r"\1\2<redacted>"),
    (re.compile(r"(?i)\bBearer\s+\S+"), "Bearer <redacted>"),
    (re.compile(r"\beg1~[A-Za-z0-9._~\-]+"), "<redacted-token>"),
    (re.compile(r"\beyJ[A-Za-z0-9_\-]{8,}\.[A-Za-z0-9_\-]{4,}\.[A-Za-z0-9_\-]*"), "<redacted-jwt>"),
    (re.compile(r"[A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}"), "<email>"),
    (re.compile(r"(?<![0-9A-Fa-f])[0-9a-fA-F]{32}(?![0-9A-Fa-f])"), "<ACCT-ID>"),
]
GUID_RE = re.compile(r"\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\b")
HEX0X_RE = re.compile(r"\b0[xX][0-9a-fA-F]+\b")
HEXLONG_RE = re.compile(r"\b(?=[0-9a-fA-F]*\d)(?=[0-9a-fA-F]*[a-fA-F])[0-9a-fA-F]{8,}\b")
NUM_RE = re.compile(r"\d+(?:\.\d+)?")
CTRL_RE = re.compile(r"[^\x20-\x7e]")


def redact(s):
    for rx, rep in REDACTIONS:
        s = rx.sub(rep, s)
    return s


def normalise(s):
    s = GUID_RE.sub("<GUID>", s)
    s = HEX0X_RE.sub("<HEX>", s)
    s = HEXLONG_RE.sub("<HEX>", s)
    s = NUM_RE.sub("N", s)
    s = re.sub(r"\s+", " ", s).strip()
    return s


def clean(s, limit=MAX_MSG):
    s = CTRL_RE.sub("?", s)
    return s if len(s) <= limit else s[: limit - 3] + "..."


def group_of(cat):
    if cat in EXACT_GROUPS:
        return cat
    for name, prefix in PREFIX_GROUPS:
        if cat.startswith(prefix):
            return name
    return "other"


# ---- scanning --------------------------------------------------------------------------

class Group:
    __slots__ = ("sev", "display", "first", "last", "msgs", "dmsgs")

    def __init__(self):
        self.sev = collections.Counter()   # Warning/Error/Fatal -> count
        self.display = 0
        self.first = None
        self.last = None
        self.msgs = collections.Counter()  # normalised "Level: msg" for W/E/F
        self.dmsgs = collections.Counter()  # normalised Display messages


def scan(path, milestones):
    groups = {g: Group() for g in GROUP_ORDER}
    overall = collections.Counter()
    ms = {m: [0, None, None] for m in milestones}  # count, first ts, first line no
    nlines = 0
    first_ts = last_ts = None
    cur_ts = None
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for nlines, raw in enumerate(fh, 1):
            line = raw.rstrip("\r\n")
            if nlines == 1:
                line = line.lstrip("\ufeff")
            m = LINE_RE.match(line)
            if m and m.group(1):
                cur_ts = m.group(1)
                if first_ts is None:
                    first_ts = cur_ts
                last_ts = cur_ts
            for name in milestones:
                if name in line:
                    rec = ms[name]
                    rec[0] += 1
                    if rec[1] is None:
                        rec[1], rec[2] = cur_ts, nlines
            if not m:
                continue
            cat, sev, text = m.group(2), m.group(3), m.group(4)
            if sev is None or sev in ("Verbose", "VeryVerbose"):
                continue
            g = group_of(cat)
            if sev == "Display" and g == "other":
                continue
            grp = groups[g]
            key = normalise(redact(text))
            if sev == "Display":
                grp.display += 1
                grp.dmsgs[cat + ": " + key] += 1
            else:
                grp.sev[sev] += 1
                full = "%s %s: %s" % (cat, sev, key)
                grp.msgs[full] += 1
                overall[full] += 1
            if grp.first is None:
                grp.first = cur_ts
            grp.last = cur_ts
    return {"path": path, "lines": nlines, "first_ts": first_ts, "last_ts": last_ts,
            "groups": groups, "overall": overall, "milestones": ms}


# ---- output ----------------------------------------------------------------------------

def short_ts(ts):
    # 2026.11.12-18.31.02:123 -> 11.12-18.31.02
    return ts[5:19] if ts else "-"


def render_text(res, title, n_examples, n_top, out):
    out.append("== %s: %s  (%d lines, %s .. %s)" % (
        title, res["path"], res["lines"], short_ts(res["first_ts"]), short_ts(res["last_ts"])))
    out.append("-- groups (W/E/F = Warning/Error/Fatal, D = Display; other: W/E/F only)")
    empty = []
    for gname in GROUP_ORDER:
        g = res["groups"][gname]
        w, e, f = g.sev["Warning"], g.sev["Error"], g.sev["Fatal"]
        if not (w or e or f or g.display):
            empty.append(gname)
            continue
        out.append("%-20s W=%-5d E=%-5d F=%-3d D=%-6d %s .. %s" % (
            gname, w, e, f, g.display, short_ts(g.first), short_ts(g.last)))
        examples = g.msgs.most_common(n_examples)
        if not examples:
            examples = g.dmsgs.most_common(1)
        for msg, cnt in examples:
            out.append("    x%-5d %s" % (cnt, clean(msg)))
    if empty:
        out.append("(no W/E/F/D lines: %s)" % ", ".join(empty))
    out.append("-- top %d normalised W/E/F messages overall" % n_top)
    for msg, cnt in res["overall"].most_common(n_top):
        out.append("  x%-5d %s" % (cnt, clean(msg)))
    out.append("-- milestones (count, first timestamp, first line no)")
    for name, (cnt, ts, ln) in res["milestones"].items():
        if cnt:
            out.append("  %-26s x%-6d %s  line %d" % (name, cnt, short_ts(ts), ln))
        else:
            out.append("  %-26s not seen" % name)


def to_json(res, n_examples, n_top):
    groups = {}
    for gname in GROUP_ORDER:
        g = res["groups"][gname]
        groups[gname] = {
            "warning": g.sev["Warning"], "error": g.sev["Error"], "fatal": g.sev["Fatal"],
            "display": g.display, "first": g.first, "last": g.last,
            "examples": [{"count": c, "message": clean(m)} for m, c in g.msgs.most_common(n_examples)],
            "display_examples": [{"count": c, "message": clean(m)} for m, c in g.dmsgs.most_common(n_examples)],
        }
    return {
        "path": res["path"], "lines": res["lines"], "first_ts": res["first_ts"], "last_ts": res["last_ts"],
        "groups": groups,
        "top": [{"count": c, "message": clean(m)} for m, c in res["overall"].most_common(n_top)],
        "milestones": {k: {"count": v[0], "first_ts": v[1], "first_line": v[2]} for k, v in res["milestones"].items()},
    }


def main(argv):
    ap = argparse.ArgumentParser(description="Summarise W/E/F lines and milestones of a UE client/server log.")
    ap.add_argument("client_log")
    ap.add_argument("--server", help="also scan this server log")
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    ap.add_argument("--examples", type=int, default=2, help="examples per group (default 2)")
    ap.add_argument("--top", type=int, default=15, help="overall top-N messages (default 15)")
    args = ap.parse_args(argv)

    jobs = [("CLIENT", args.client_log, CLIENT_MILESTONES, args.examples, args.top)]
    if args.server:
        # Server section is kept shorter so the default output stays compact.
        jobs.append(("SERVER", args.server, SERVER_MILESTONES, min(args.examples, 1), min(args.top, 8)))
    results = []
    for title, path, miles, nex, ntop in jobs:
        try:
            results.append((title, scan(path, miles), nex, ntop))
        except OSError as exc:
            print("error: cannot read %s: %s" % (path, exc), file=sys.stderr)
            return 2
    if args.json:
        print(json.dumps({t.lower(): to_json(r, nex, ntop) for t, r, nex, ntop in results}, indent=1))
        return 0
    out = []
    for t, r, nex, ntop in results:
        render_text(r, t, nex, ntop, out)
    print("\n".join(out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
