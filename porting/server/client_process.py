"""Locate the shipping client in this network namespace for probe patchers."""
import os
from pathlib import Path


def shipping_client_pids(proc_root=Path('/proc')):
    namespace = os.readlink(proc_root / 'self/ns/net')
    pids = []
    for entry in proc_root.iterdir():
        if not entry.name.isdigit():
            continue
        try:
            if os.readlink(entry / 'ns/net') != namespace:
                continue
            argv0 = (entry / 'cmdline').read_bytes().split(b'\0', 1)[0]
            basename = argv0.replace(b'\\', b'/').rsplit(b'/', 1)[-1]
            if basename.lower() == b'fortniteclient-win64-shipping.exe':
                pids.append(int(entry.name))
        except (OSError, ValueError):
            continue
    return sorted(pids)
