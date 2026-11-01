#!/usr/bin/env python3
"""Verify only the named diagnostic checks; this is not full runtime acceptance."""
import argparse
import hashlib
import json
from pathlib import Path
import re

REQUIRED = ('preconditions','classes','families','reflection_function','reflection_return','reflection_class','reflection_object','null','lookup_no_create','widget_owner',
            'subsystems_present','local_front','local_global','game_director','player_selection',
            'registry_creation','registry_gc_retains','registry_replacement','registry_remove','registry_pending_reachable',
            'registry_collect_after_remove')
MARK = re.compile(r'CONTEXT_PROBE_V1 (\w+)(?: (PASS|FAIL))?(?:\s*)$')

def verify(raw):
    lines = [m.groups() for line in raw.splitlines() if (m := MARK.search(line))]
    expected = [(name, 'PASS') for name in REQUIRED] + [('END', None)]
    return lines == expected

def main():
    p = argparse.ArgumentParser()
    p.add_argument('log', type=Path)
    a = p.parse_args()
    blob = a.log.read_bytes()
    ok = verify(blob.decode('utf-8', errors='strict'))
    print(json.dumps({'schema':'context-runtime-probe-log-v1','log_sha256':hashlib.sha256(blob).hexdigest(),
        'passed':ok,'coverage':'named live checks only', 'fresh_uht_or_elf_verified':False,
        'early_initialization_observed':False,'deferred_world_callbacks_observed':False,
        'authentic_startup_verified':False}, sort_keys=True))
    return 0 if ok else 1
if __name__ == '__main__':
    raise SystemExit(main())
