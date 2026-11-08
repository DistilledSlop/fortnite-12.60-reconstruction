#!/usr/bin/env python3
"""Reproduce the four-step original-shader preparation (archive extract, D3D inspect, mapped DXBC->SPIR-V, reflection)
for one login-art material from its own decoded original resource. Indices come only from the original serialized
shader map associated with the pinned original archive; nothing is selected by hand. Output directory must be new."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full')
INVENTORY = GENERATED / 'frontend-probes/frontend-material-inventory'
ARCHIVE = EXTRACTED / 'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
ARCHIVE_SHA256 = '2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21'


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def slate_indices(package):
    numeric = sibling('export_numeric_ingress')
    bundle = numeric.bundle
    stem = package.replace('/', '_')
    framing = json.loads((INVENTORY / (stem + '.inline-resource.json')).read_bytes())
    raw = (EXTRACTED / (package + '.uexp')).read_bytes()
    payload = bundle.validate_source(raw, framing)
    parsed = bundle.decoder.parse_prefix(payload, framing['names'])
    if bundle.library_reader.sha256(ARCHIVE) != ARCHIVE_SHA256:
        raise ValueError('original archive changed')
    with bundle.library_reader.ShaderArchive(ARCHIVE, layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed, archive)
    indices = sorted({r['library_association']['index'] for r in parsed['material_shaders']['records']})
    if len(indices) != 7:
        raise ValueError('expected seven distinct Slate shaders')
    return indices


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if GENERATED.resolve() not in output.parents or output.exists():
        raise SystemExit('fresh output under generated required')
    indices = slate_indices(args.package)
    output.mkdir(parents=True)
    py = [sys.executable, '-I']
    steps = [
        py + [str(HERE / 'shader_archive.py'), '--archive', str(ARCHIVE), '--max-shaders', '7', '--extract-dir', str(output / 'original'),
              '--report', str(output / 'archive.json')] + [x for i in indices for x in ('--shader-index', str(i))],
        py + [str(HERE / 'inspect_d3d_code.py'), '--source-report', str(output / 'archive.json'), '--report', str(output / 'd3d.json')],
        py + [str(HERE / 'translate_mapped_dxbc.py'), '--source-report', str(output / 'archive.json'), '--d3d-report', str(output / 'd3d.json'),
              '--output', str(output / 'mapped')],
        py + [str(HERE / 'reflect_mapped_spirv.py'), '--mapped-report', str(output / 'mapped/mapped-translation-report.json'),
              '--report', str(output / 'reflection.json')],
    ]
    for number, command in enumerate(steps):
        done = subprocess.run(command, capture_output=True, text=True, check=False)
        (output / ('step-%d.log' % number)).write_text(done.stdout + done.stderr)
        if done.returncode:
            raise SystemExit('step %d failed: %s' % (number, (done.stdout + done.stderr)[-400:]))
    print(json.dumps(dict(indices=indices)))


if __name__ == '__main__':
    main()
