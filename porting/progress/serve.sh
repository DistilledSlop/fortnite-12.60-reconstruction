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

exec python3 -m http.server "$port" --bind 0.0.0.0 --directory "$out"
