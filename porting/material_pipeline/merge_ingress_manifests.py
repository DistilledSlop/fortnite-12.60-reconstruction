#!/usr/bin/env python3
"""Combine generated ingress manifests without changing their source bundles.

The runtime still validates the resulting manifest and each resource. This tool
checks bundle hashes and duplicate owners before producing an optional probe input.
"""
import argparse
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('manifests', type=Path, nargs='+')
    args = parser.parse_args()
    entries = []
    owners = set()
    for path in args.manifests:
        data = json.loads(path.read_bytes())
        if data.get('schema') != 'fortnite.legacy-material-manifest.v1' or not isinstance(data.get('entries'), list):
            parser.error(f'invalid manifest: {path}')
        for source in data['entries']:
            entry = dict(source)
            owner = entry['owner_path']
            if owner in owners:
                parser.error(f'duplicate owner: {owner}')
            owners.add(owner)
            bundle = Path(entry['bundle_path'])
            if not bundle.is_absolute():
                bundle = path.resolve().parent / bundle
            bundle = bundle.resolve(strict=True)
            if hashlib.sha1(bundle.read_bytes()).hexdigest() != entry['bundle_sha1'].lower():
                parser.error(f'bundle hash mismatch: {bundle}')
            entry['bundle_path'] = str(bundle)
            entries.append(entry)
    if not entries or len(entries) > 256:
        parser.error('runtime requires 1 to 256 manifest entries')
    output = json.dumps({'schema': 'fortnite.legacy-material-manifest.v1', 'entries': entries}, indent=2).encode() + b'\n'
    if len(output) > 4 * 1024 * 1024:
        parser.error('manifest exceeds the runtime size limit')
    with args.output.open('xb') as out:
        out.write(output)
    print(f'{args.output}: {len(entries)} entries, SHA256 {hashlib.sha256(output).hexdigest()}')


if __name__ == '__main__':
    main()
