#!/usr/bin/env python3
"""Compare two existing translation receipts and inspect shader 25's SPIR-V.

This is an inventory, not a semantic equivalence test. Paths in receipts are local.
"""
import argparse
import collections
import hashlib
import json
import re
import subprocess
from pathlib import Path


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load(path):
    return json.loads(Path(path).read_text())


def index(records):
    result = {r['shader_index']: r for r in records}
    if len(result) != len(records):
        raise ValueError('duplicate shader index')
    return result


def inspect(disassembler, path):
    text = subprocess.check_output([disassembler, path], text=True)
    images = re.findall(r'OpTypeImage (.*)', text)
    fetches = collections.Counter(re.findall(r'OpImageFetch (%\S+)', text))
    return {'image_types': images, 'image_fetch_result_types': dict(sorted(fetches.items()))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--old', required=True, help='1.2 translation receipt')
    parser.add_argument('--new', required=True, help='2.0 translation receipt')
    parser.add_argument('--old-bindings', required=True)
    parser.add_argument('--new-bindings', required=True)
    parser.add_argument('--spirv-dis', required=True)
    args = parser.parse_args()
    old, new = load(args.old), load(args.new)
    old_b, new_b = load(args.old_bindings), load(args.new_bindings)
    if old['source_report_sha256'] != new['source_report_sha256'] or old['archive_sha256'] != new['archive_sha256']:
        raise ValueError('translation sources differ')
    if old_b['translation_report_sha256'] != digest(args.old) or new_b['translation_report_sha256'] != digest(args.new):
        raise ValueError('binding report refers to another translation receipt')
    if old_b['d3d_report_sha256'] != new_b['d3d_report_sha256'] or old_b['archive_sha256'] != old['archive_sha256'] or new_b['archive_sha256'] != new['archive_sha256']:
        raise ValueError('binding report source mismatch')
    a, b, ab, bb = (index(x['records']) for x in (old, new, old_b, new_b))
    if not (a.keys() == b.keys() == ab.keys() == bb.keys()):
        raise ValueError('shader selections differ')
    same_layout = 0
    for i in a:
        x, y = a[i], b[i]
        if x['input_sha256'] != y['input_sha256'] or digest(x['input']) != x['input_sha256'] or digest(y['input']) != y['input_sha256']:
            raise ValueError(f'input mismatch for {i}')
        for record, binding in ((x, ab[i]), (y, bb[i])):
            if not record['validated'] or record['compile_exit'] != 0 or record['validation_exit'] != 0:
                raise ValueError(f'unsuccessful translation {i}')
            if digest(record['output']) != record['output_sha256'] or binding['spirv_sha256'] != record['output_sha256']:
                raise ValueError(f'output mismatch for {i}')
        layout = lambda r: sorted((d['set'], d['binding'], d['storage_class']) for d in r['spirv_descriptors'])
        same_layout += layout(ab[i]) == layout(bb[i])
    if 25 not in a:
        raise ValueError('shader 25 missing')
    result = {
        'matched_inputs': len(a),
        'old_diagnostic_indices': sorted(i for i, r in a.items() if r['compiler_diagnostics_present']),
        'new_diagnostic_indices': sorted(i for i, r in b.items() if r['compiler_diagnostics_present']),
        'equal_descriptor_set_binding_storage_inventories': same_layout,
        'descriptor_counts_old_new': [sum(len(r['spirv_descriptors']) for r in x.values()) for x in (ab, bb)],
        'shader_25_old': inspect(args.spirv_dis, a[25]['output']),
        'shader_25_new': inspect(args.spirv_dis, b[25]['output']),
    }
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
