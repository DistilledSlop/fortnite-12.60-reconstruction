#!/usr/bin/env python3
"""Translate verified recovered DXBC to validated SPIR-V without modifying UE."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
DEFAULT_TOOLS = GENERATED / 'material_pipeline/tools/root/usr'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def generated_path(path):
    path = Path(path).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('shader artifacts must be inside porting/generated')
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-report', type=Path, required=True)
    parser.add_argument('--tool-prefix', type=Path, default=DEFAULT_TOOLS)
    parser.add_argument('--output', type=Path, default=GENERATED / 'material_pipeline/translated')
    parser.add_argument('--limit', type=int, default=64)
    args = parser.parse_args()
    if not 1 <= args.limit <= 256:
        parser.error('--limit must be 1..256')
    try:
        output = generated_path(args.output)
        tool = args.tool_prefix.resolve()
        compiler = tool / 'bin/vkd3d-compiler'
        validator = tool / 'bin/spirv-val'
        library = tool / 'lib/x86_64-linux-gnu/libvkd3d-shader.so.1'
        identities = {str(p): digest(p) for p in (compiler, library, validator)}
        environment = dict(os.environ)
        environment['LD_LIBRARY_PATH'] = str(library.parent)
        source_bytes = args.source_report.read_bytes()
        source_identity = hashlib.sha256(source_bytes).hexdigest()
        source = json.loads(source_bytes)
        jobs = []
        for shader in source['shaders']:
            if type(shader['index']) is not int or shader['index'] < 0:
                raise ValueError('shader index must be a nonnegative integer')
            wrapper = generated_path(shader['wrapper_path'])
            if digest(wrapper) != shader['decoded_sha256']:
                raise ValueError('recovered shader wrapper hash changed')
            code = wrapper.read_bytes()
            for container in shader['dxbc']:
                path = generated_path(container['path'])
                offset, size = container['offset'], container['size']
                if offset < 0 or size < 32 or offset + size > len(code) or path.read_bytes() != code[offset:offset + size]:
                    raise ValueError('DXBC bytes do not match the verified recovered wrapper')
                jobs.append((shader, path))
        if not jobs:
            raise ValueError('source report has no recovered DXBC jobs')
        output.mkdir(parents=True, exist_ok=True)
        rows = []
        for shader, path in jobs[:args.limit]:
            identity = digest(path)
            dest = output / f"{shader['index']}-{identity[:20]}.spv"
            temporary = dest.with_suffix('.partial.spv')
            command = [str(compiler), '-x', 'dxbc-tpf', '-b', 'spirv-binary', '-o', str(temporary), str(path)]
            compiled = subprocess.run(command, env=environment, capture_output=True, text=True, timeout=30)
            validation = None
            if compiled.returncode == 0:
                validation = subprocess.run([str(validator), '--target-env', 'vulkan1.0', str(temporary)],
                                            capture_output=True, text=True, timeout=30)
            passed = compiled.returncode == 0 and validation is not None and validation.returncode == 0
            row = {'shader_index': shader['index'], 'frequency': shader['frequency'],
                   'input': str(path), 'input_sha256': identity, 'command': command,
                   'compile_exit': compiled.returncode,
                   'validation_exit': validation.returncode if validation else None,
                   'diagnostic': compiled.stderr + (validation.stdout + validation.stderr if validation else ''),
                   'validated': passed}
            if digest(path) != identity:
                passed = False
                row.update(validated=False, diagnostic=row['diagnostic'] + '\nDXBC input changed during compilation')
            if passed:
                temporary.replace(dest)
                row.update(output=str(dest), output_sha256=digest(dest), output_bytes=dest.stat().st_size)
            elif temporary.exists():
                failed = dest.with_suffix('.failed.spv')
                temporary.replace(failed)
                row['failed_artifact'] = str(failed)
            rows.append(row)
        report = {'source_report': str(args.source_report.resolve()), 'source_report_sha256': source_identity,
                  'archive_sha256': source['sha256'], 'tool_sha256': identities, 'target_env': 'vulkan1.0',
                  'selected_jobs': len(jobs), 'attempted_jobs': len(rows), 'truncated': len(jobs) > args.limit,
                  'validated_jobs': sum(row['validated'] for row in rows), 'records': rows,
                  'limitations': ['Not inserted into UE shader maps; material bindings and actual draw parity remain unverified']}
        receipt = output / 'translation-report.json'
        receipt.with_suffix('.tmp').write_text(json.dumps(report, indent=2) + '\n')
        receipt.with_suffix('.tmp').replace(receipt)
        print(f"validated {report['validated_jobs']}/{len(rows)}; selected {len(jobs)}; report {receipt}")
        return 0 if all(row['validated'] for row in rows) else 1
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    raise SystemExit(main())
