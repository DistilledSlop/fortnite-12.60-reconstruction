#!/usr/bin/env python3
"""Record one verification check in the tracked evidence ledger porting/progress/evidence.json (stdlib).

Usage:
  record_evidence.py CHECK_ID [--timeout S] [--grep REGEX] [--log PATH] -- COMMAND [ARG...]
  record_evidence.py CHECK_ID --log PATH --grep REGEX        judge an existing log, no command run
  record_evidence.py --list                                  print the ledger
  record_evidence.py --remove CHECK_ID                       delete one entry

The command runs in the repository root with a timeout (default 900 s). Its exit code and up to
five key lines are stored with the HEAD commit and a UTC time. The entry for CHECK_ID is replaced.

result is "pass" when the command exits 0 (or no command ran) and, if --grep is given, at least
one line of the output (or of --log) matches REGEX. Otherwise "fail". key_lines are the first five
matching lines; with --grep absent they are the last three output lines, for context.

progress.py reads the ledger: a leaf lists check ids and counts a check as passed only when its
latest entry says "pass". A missing entry is not a pass. Run checks yourself; this tool does not
decide what is worth running (see the standard suite list in README.md).
"""

import argparse
import datetime
import json
import os
import re
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
LEDGER = os.path.join(HERE, "evidence.json")
ID_RE = re.compile(r"^[a-z0-9_]+$")
MAX_KEY_LINES = 5
LINE_CHARS = 200


def now_utc():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def head_sha():
    proc = subprocess.run(["git", "-C", REPO, "rev-parse", "HEAD"], capture_output=True, text=True)
    if proc.returncode != 0:
        raise SystemExit(f"record_evidence: cannot read HEAD: {proc.stderr.strip()}")
    return proc.stdout.strip()


def load_ledger():
    if not os.path.isfile(LEDGER):
        return {"checks": {}}
    with open(LEDGER, encoding="utf-8") as fh:
        data = json.load(fh)
    if not isinstance(data, dict) or not isinstance(data.get("checks"), dict):
        raise SystemExit(f"record_evidence: {LEDGER} has no 'checks' object")
    return data


def save_ledger(data):
    tmp = LEDGER + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        fh.write(json.dumps(data, indent=1, sort_keys=True) + "\n")
    os.replace(tmp, LEDGER)


def trim(line):
    line = line.rstrip()
    return line if len(line) <= LINE_CHARS else line[:LINE_CHARS - 3] + "..."


def run_command(cmd, timeout):
    """Returns (exit_code, output_text). Exit 124 on timeout, as timeout(1) does."""
    try:
        proc = subprocess.run(cmd, cwd=REPO, capture_output=True, timeout=timeout)
        out = (proc.stdout or b"") + (proc.stderr or b"")
        return proc.returncode, out.decode("utf-8", "replace")
    except subprocess.TimeoutExpired as exc:
        out = (exc.stdout or b"") + (exc.stderr or b"")
        text = out.decode("utf-8", "replace") if isinstance(out, bytes) else str(out)
        return 124, text + f"\n[record_evidence] timed out after {timeout:g} s\n"
    except OSError as exc:
        return 127, f"[record_evidence] cannot start command: {exc}\n"


def build_entry(args, cmd, text, exit_code, commit):
    lines = text.splitlines()
    entry = {
        "id": args.check_id,
        "command": shlex.join(cmd) if cmd else f"(log only) {args.log}",
        "exit_code": exit_code,
        "result": None,
        "key_lines": [],
        "commit": commit,
        "utc": now_utc(),
    }
    ok = exit_code == 0 if cmd else True
    if args.grep:
        rx = re.compile(args.grep)
        matched = [trim(l) for l in lines if rx.search(l)]
        entry["matches"] = len(matched)
        entry["key_lines"] = matched[:MAX_KEY_LINES]
        ok = ok and len(matched) > 0
    else:
        tail = [trim(l) for l in lines if l.strip()][-3:]
        entry["key_lines"] = tail
    entry["result"] = "pass" if ok else "fail"
    if not cmd:
        entry["exit_code"] = None
    return entry


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    cmd = []
    if "--" in argv:
        split = argv.index("--")
        cmd = argv[split + 1:]
        argv = argv[:split]
        if not cmd:
            raise SystemExit("record_evidence: nothing after '--'")

    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("check_id", nargs="?", help="check id, lowercase letters, digits and underscores")
    ap.add_argument("--timeout", type=float, default=900.0, help="seconds before the command is killed")
    ap.add_argument("--grep", help="regex; a line matching it is a key line and is required for pass")
    ap.add_argument("--log", help="read output from this log file instead of the command output")
    ap.add_argument("--list", action="store_true", help="print the ledger and exit")
    ap.add_argument("--remove", metavar="CHECK_ID", help="delete one entry and exit")
    args = ap.parse_args(argv)

    if args.list:
        ledger = load_ledger()
        for cid in sorted(ledger["checks"]):
            ent = ledger["checks"][cid]
            first = (ent.get("key_lines") or [""])[0]
            print(f"{cid:28s} {ent.get('result', '?'):5s} exit={ent.get('exit_code')} "
                  f"{str(ent.get('commit', ''))[:7]} {ent.get('utc', '')}  {first}")
        if not ledger["checks"]:
            print("(ledger is empty)")
        return 0

    if args.remove:
        ledger = load_ledger()
        if ledger["checks"].pop(args.remove, None) is None:
            print(f"record_evidence: no entry {args.remove}", file=sys.stderr)
            return 1
        save_ledger(ledger)
        print(f"record_evidence: removed {args.remove}")
        return 0

    if not args.check_id or not ID_RE.match(args.check_id):
        raise SystemExit("record_evidence: CHECK_ID must match [a-z0-9_]+")
    if not cmd and not args.log:
        raise SystemExit("record_evidence: give a command after '--', or --log PATH with --grep")
    if cmd and args.log:
        raise SystemExit("record_evidence: use either a command or --log, not both")
    if args.log and not args.grep:
        raise SystemExit("record_evidence: --log needs --grep")

    commit = head_sha()
    exit_code = None
    if cmd:
        exit_code, text = run_command(cmd, args.timeout)
    else:
        if not os.path.isfile(args.log):
            text = ""
        else:
            with open(args.log, encoding="utf-8", errors="replace") as fh:
                text = fh.read()
    entry = build_entry(args, cmd, text, exit_code, commit)
    if args.log and not os.path.isfile(args.log):
        entry["key_lines"] = [f"log missing: {args.log}"]

    ledger = load_ledger()
    ledger["checks"][args.check_id] = entry
    save_ledger(ledger)
    print(f"record_evidence: {args.check_id} {entry['result']} "
          f"(exit {entry['exit_code']}, {entry.get('matches', 'no --grep')} matches) at {commit[:7]}")
    for line in entry["key_lines"]:
        print(f"  {line}")
    return 0 if entry["result"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
