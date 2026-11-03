#!/usr/bin/env python3
"""Bounded, resumable conversion of extracted SM_ packages to isolated GLBs."""
import argparse
import concurrent.futures
import fcntl
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import struct
import subprocess
import tempfile
import time

PORTING = Path(__file__).resolve().parents[1]
DEFAULT_INPUT = Path('/mnt/vibrato/fortnite-extracted-full')
DEFAULT_TOOL = PORTING / 'cue4inspect/bin/Release/net10.0'
DEFAULT_OUTPUT = PORTING / 'generated/asset_pipeline'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def validate_glb(path):
    """Validate GLB 2 container and JSON chunk; not geometry or runtime usability."""
    with path.open('rb') as f:
        header = f.read(20)
        if len(header) != 20:
            raise ValueError('short GLB header')
        magic, version, total, size, kind = struct.unpack('<4sIIII', header)
        if magic != b'glTF' or version != 2 or total != path.stat().st_size or kind != 0x4e4f534a:
            raise ValueError('invalid GLB header')
        if size < 4 or size % 4 or 20 + size > total:
            raise ValueError('invalid GLB JSON chunk length')
        doc = json.loads(f.read(size))
        if not isinstance(doc, dict) or not isinstance(doc.get('asset'), dict) or doc['asset'].get('version') != '2.0':
            raise ValueError('missing glTF 2.0 asset metadata')
        meshes = doc.get('meshes')
        if not isinstance(meshes, list) or not any(
            isinstance(mesh, dict) and any(isinstance(p, dict) and 'POSITION' in p.get('attributes', {})
                                           for p in mesh.get('primitives', [])) for mesh in meshes
        ):
            raise ValueError('no mesh POSITION primitive')
        offset = 20 + size
        while offset < total:
            chunk_header = f.read(8)
            if len(chunk_header) != 8:
                raise ValueError('truncated GLB chunk header')
            length, _ = struct.unpack('<II', chunk_header)
            offset += 8 + length
            if length % 4 or offset > total:
                raise ValueError('invalid GLB chunk length')
            f.seek(length, 1)
        if offset != total:
            raise ValueError('invalid GLB chunk boundary')


def package_path(value):
    p = PurePosixPath(value)
    if p.is_absolute() or not p.parts or any(x in ('.', '..') for x in p.parts):
        raise ValueError('unsafe package path')
    if len(p.parts) < 4 or p.parts[:2] != ('FortniteGame', 'Content') or not p.name.startswith('SM_') or p.suffix:
        raise ValueError('expected extensionless FortniteGame/Content/.../SM_ package')
    return p.as_posix()


def input_fingerprint(root, name):
    result = {}
    for ext in ('.uasset', '.uexp', '.ubulk', '.uptnl', '.m.ubulk'):
        p = root / (name + ext)
        if p.is_file():
            stat = p.stat()
            result[ext] = {"size": stat.st_size, "sha256": digest(p)}
    if '.uasset' not in result:
        raise FileNotFoundError(f'missing package {name}.uasset')
    return result


def tool_fingerprint(tool):
    if not (tool / 'Cue4Inspect.dll').is_file():
        raise FileNotFoundError(f'missing inspector in {tool}')
    # Tool and dependency bytes, not timestamp alone: changing a dependency invalidates prior success.
    return {p.relative_to(tool).as_posix(): digest(p) for p in sorted(tool.rglob('*')) if p.is_file()}


def verified(record, root, output, tool_id, name):
    if record.get('status') != 'ok' or record.get('tool') != tool_id:
        return False
    try:
        if record.get('input') != input_fingerprint(root, name) or not record.get('files'):
            return False
        base = (output / 'glb' / name).resolve()
        for entry in record['files']:
            p = (base / entry['name']).resolve()
            if p.parent != base or not p.is_file() or digest(p) != entry['sha256']:
                return False
            validate_glb(p)
        return True
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError):
        return False


def select_candidates(names, records, root, output, tool_id, retry_failed, limit):
    chosen = []
    for name in names:
        rec = records.get(name, {})
        if verified(rec, root, output, tool_id, name):
            continue
        if rec.get('status') == 'failed' and not retry_failed:
            continue
        chosen.append(name)
        if len(chosen) == limit:
            break
    return chosen


def atomic_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + '.tmp')
    temp.write_text(json.dumps(data, indent=2, sort_keys=True) + '\n')
    os.replace(temp, path)


def convert(name, root, tool, output, timeout, tool_id):
    started = time.time()
    before = input_fingerprint(root, name)
    with tempfile.TemporaryDirectory(prefix='mesh-', dir=output / 'work') as temp:
        worker = Path(temp)
        binary = worker / 'cue4inspect/bin/Release/net10.0'
        shutil.copytree(tool, binary)
        cmd = ['dotnet', str(binary / 'Cue4Inspect.dll'), name, str(root), '--loose', '--export-glb']
        try:
            run = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 timeout=timeout, check=False)
            code, log = run.returncode, run.stdout
        except subprocess.TimeoutExpired as e:
            code, log = None, (e.stdout or b'') + b'\nTimed out\n'
        log_dir = output / 'logs' / name
        log_dir.mkdir(parents=True, exist_ok=True)
        (log_dir / 'latest.log').write_bytes(log)
        if code != 0:
            raise RuntimeError(f'converter exit {code}; log {log_dir / "latest.log"}')
        converted = worker / 'generated/converted' / name
        # Exporter appends file suffixes to the package path in its parent directory.
        files = sorted(converted.parent.glob(converted.name + '*.glb'))
        if not files:
            raise RuntimeError('converter exited 0 but produced no GLB')
        for path in files:
            validate_glb(path)
        if input_fingerprint(root, name) != before:
            raise RuntimeError('input changed during conversion')
        dest = output / 'glb' / name
        dest.mkdir(parents=True, exist_ok=True)
        entries = []
        for src in files:
            target = dest / src.name
            staging = dest / (src.name + '.partial')
            shutil.copyfile(src, staging)
            os.replace(staging, target)
            entries.append({'name': src.name, 'size': target.stat().st_size, 'sha256': digest(target)})
        return {'status':'ok', 'input':before, 'tool':tool_id, 'files':entries,
                'elapsed_seconds':round(time.time() - started, 2), 'log':str((log_dir / 'latest.log').relative_to(output))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input-root', type=Path, default=DEFAULT_INPUT)
    parser.add_argument('--tool-dir', type=Path, default=DEFAULT_TOOL)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--package', action='append', help='extensionless package path; repeat for a bounded selection')
    parser.add_argument('--batch', type=int, default=10, help='maximum attempts (1–100)')
    parser.add_argument('--workers', type=int, default=1, help='parallel converters (1–32)')
    parser.add_argument('--timeout', type=int, default=120)
    parser.add_argument('--retry-failed', action='store_true')
    parser.add_argument('--dry-run', action='store_true', help='do not write or run converter')
    args = parser.parse_args()
    if not 1 <= args.batch <= 100 or not 1 <= args.workers <= 32 or not 1 <= args.timeout <= 3600:
        parser.error('batch must be 1–100, workers 1–32, timeout 1–3600 seconds')
    root, tool, output = args.input_root.resolve(), args.tool_dir.resolve(), args.output.resolve()
    if not (root / 'FortniteGame/Content').is_dir():
        parser.error('input root lacks FortniteGame/Content')
    if (PORTING / 'generated').resolve() not in output.parents:
        parser.error('output must be inside porting/generated')
    if output == root or root in output.parents:
        parser.error('output must be outside the input tree')
    tool_id = tool_fingerprint(tool)
    names = sorted(set(package_path(p) for p in args.package)) if args.package else sorted(
        p.relative_to(root).with_suffix('').as_posix() for p in (root / 'FortniteGame/Content').rglob('SM_*.uasset'))
    def run():
        state_path = output / 'state.json'
        state = json.loads(state_path.read_text()) if state_path.exists() else {'version':1, 'input_root':str(root), 'records':{}}
        if state.get('version') != 1 or state.get('input_root') != str(root) or not isinstance(state.get('records'), dict):
            raise ValueError('checkpoint version or input root mismatch')
        records = state['records']
        chosen = select_candidates(names, records, root, output, tool_id, args.retry_failed, args.batch)
        print(f'candidates={len(names)} attempts={len(chosen)} dry_run={args.dry_run}', flush=True)
        for name in chosen:
            print(name, flush=True)
        if args.dry_run:
            return 0
        (output / 'work').mkdir(parents=True, exist_ok=True)
        # Only the coordinator writes checkpoints; interrupted jobs have no success record and can resume.
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
            jobs = {pool.submit(convert, name, root, tool, output, args.timeout, tool_id): name for name in chosen}
            for job in concurrent.futures.as_completed(jobs):
                name = jobs[job]
                try:
                    records[name] = job.result()
                except Exception as e:
                    records[name] = {'status':'failed', 'error':str(e), 'tool':tool_id}
                atomic_json(state_path, state)
                print(f"{records[name]['status']}: {name}", flush=True)
        return 1 if any(records[n]['status'] != 'ok' for n in chosen) else 0
    if args.dry_run:
        return run()
    output.mkdir(parents=True, exist_ok=True)
    with (output / 'runner.lock').open('a') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            parser.error('another runner owns this output directory')
        return run()


if __name__ == '__main__':
    raise SystemExit(main())
