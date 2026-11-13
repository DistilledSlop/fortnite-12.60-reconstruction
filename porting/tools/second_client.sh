#!/usr/bin/env bash
# A second original client in an active vanilla_play_run.sh namespace.
set -euo pipefail
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
exec python3 "$here/second_client.py" "$@"
