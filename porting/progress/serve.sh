#!/usr/bin/env bash
# Serve the generated port progress dashboard, read-only, on the LAN.
#
#   bash porting/progress/serve.sh [port]          default port 8765, binds 0.0.0.0
#   setsid nohup bash porting/progress/serve.sh >> porting/generated/progress-serve.log 2>&1 </dev/null &
#
# python3 -m http.server only serves GET and HEAD files from out/. It never
# runs the generator on requests, so the pages stay static.
set -eu

here=$(cd "$(dirname "$0")" && pwd)
out="$here/out"
port="${1:-8765}"

if [ ! -f "$out/index.html" ]; then
  python3 -I "$here/progress.py" --quiet || true
fi

# Live refresh: re-render from git HEAD plus local outputs every REFRESH_SECONDS (default 60), without
# recording history (history points are written when a commit is recorded). The page needs no hand-edited
# count files. Set REFRESH_SECONDS=0 to disable.
refresh="${REFRESH_SECONDS:-60}"
if [ "$refresh" -gt 0 ]; then
  (while sleep "$refresh"; do python3 -I "$here/progress.py" --quiet --no-record >/dev/null 2>&1 || true; done) &
fi

exec python3 -m http.server "$port" --bind 0.0.0.0 --directory "$out"
