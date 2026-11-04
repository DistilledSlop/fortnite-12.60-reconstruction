#!/usr/bin/env python3
"""Bounded, resumable conversion of extracted AnimSequence packages to UEFormat .ueanim files.

Candidates are packages whose export table contains an AnimSequence export, read from the
--scan-export-classes TSV (or porting/asset_pipeline/animation_inventory.json). Reuses the
checkpoint/fingerprint machinery of export_meshes.py; outputs live under <output>/anim/<package>.
"""
import argparse
import concurrent.futures
import fcntl
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import tempfile
import time

import importlib.util

# Load the sibling module by path: `python3 -I` does not put the script directory on sys.path.
_spec = importlib.util.spec_from_file_location('export_meshes', Path(__file__).resolve().with_name('export_meshes.py'))
em = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(em)

PORTING = em.PORTING
DEFAULT_INPUT = em.DEFAULT_INPUT
DEFAULT_TOOL = Path('/mnt/vibrato/fortnite-asset-pipeline/tool-scan')
DEFAULT_OUTPUT = PORTING / 'generated/asset_pipeline_anim'
DEFAULT_CLASSES = Path('/mnt/vibrato/fortnite-asset-pipeline/export-classes.tsv')
INVENTORY = PORTING / 'asset_pipeline/animation_inventory.json'
MAGIC = b'UEFORMAT'
SUBDIR = 'anim'


def validate_ueanim(path):
    """Non-empty UEFormat container whose header identifier is UEANIM; not a semantic check."""
    with path.open('rb') as f:
        head = f.read(64)
    if len(head) < 16 or head[:8] != MAGIC or b'UEANIM' not in head[8:32]:
        raise ValueError('missing UEFORMAT/UEANIM header')


def package_path(value):
    p = PurePosixPath(value)
    if p.is_absolute() or not p.parts or any(x in ('.', '..') for x in p.parts) or p.suffix:
        raise ValueError('unsafe package path')
    in_content = p.parts[:2] in (('FortniteGame', 'Content'), ('Engine', 'Content')) or (
        p.parts[:2] == ('FortniteGame', 'Plugins') and 'Content' in p.parts[2:-1])
    if len(p.parts) < 3 or not in_content:
        raise ValueError('expected extensionless {FortniteGame,Engine}/Content or FortniteGame/Plugins/**/Content package')
    return p.as_posix()


def anim_packages(classes_tsv=None):
    """Sorted packages with an AnimSequence export, from the scan TSV or the tracked inventory."""
    if classes_tsv is not None and classes_tsv.is_file():
        names = []
        with classes_tsv.open(encoding='utf-8') as f:
            for line in f:
                name, _, classes = line.rstrip('\n').partition('\t')
                if 'AnimSequence' in classes.split(','):
                    names.append(name)
    else:
        names = json.loads(INVENTORY.read_text())['anim_sequence_packages']
    return sorted(set(package_path(n) for n in names))


def verified(record, root, output, tool_id, name):
    if record.get('status') != 'ok' or record.get('tool') != tool_id:
        return False
    try:
        if record.get('input') != em.input_fingerprint(root, name) or not record.get('files'):
            return False
        base = (output / SUBDIR / name).resolve()
        for entry in record['files']:
            p = (base / entry['name']).resolve()
            if p.parent != base or not p.is_file() or em.digest(p) != entry['sha256']:
                return False
            validate_ueanim(p)
        return True
    except (OSError, ValueError, KeyError, TypeError):
        return False


def select_candidates(names, records, root, output, tool_id, retry_failed, limit, deep):
    chosen = []
    for name in names:
        rec = records.get(name, {})
        # Fast resume trusts an ok record for the same tool whose files still exist; --verify rehashes.
        if deep:
            if verified(rec, root, output, tool_id, name):
                continue
        elif rec.get('status') == 'ok' and rec.get('tool') == tool_id and rec.get('files') and all(
                (output / SUBDIR / name / e['name']).is_file() for e in rec['files']):
            continue
        if rec.get('status') == 'failed' and not retry_failed:
            continue
        chosen.append(name)
        if len(chosen) == limit:
            break
    return chosen


def classify(log):
    """Short failure class from a converter log (last matching line wins)."""
    text = log.decode('utf-8', 'replace')
    if 'Timed out' in text[-200:]:
        return 'timeout'
    for line in reversed(text.splitlines()):
        if 'Inspection failed:' in line:
            msg = line.split('Inspection failed:', 1)[1].strip()
            return msg.split('\n')[0][:200]
    return 'unknown'


def convert(name, root, tool, output, timeout, tool_id):
    started = time.time()
    before = em.input_fingerprint(root, name)
    with tempfile.TemporaryDirectory(prefix='anim-', dir=output / 'work') as temp:
        worker = Path(temp)
        binary = worker / 'cue4inspect/bin/Release/net10.0'
        shutil.copytree(tool, binary)
        cmd = ['dotnet', str(binary / 'Cue4Inspect.dll'), name, str(root), '--loose', '--export-anim']
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
            raise RuntimeError(f'converter exit {code}: {classify(log)}')
        converted = worker / 'generated/converted' / name
        files = sorted(converted.parent.glob(converted.name + '*.ueanim'))
        if not files:
            raise RuntimeError('converter exited 0 but produced no .ueanim')
        for path in files:
            validate_ueanim(path)
        if em.input_fingerprint(root, name) != before:
            raise RuntimeError('input changed during conversion')
        dest = output / SUBDIR / name
        dest.mkdir(parents=True, exist_ok=True)
        entries = []
        for src in files:
            target = dest / src.name
            staging = dest / (src.name + '.partial')
            shutil.copyfile(src, staging)
            os.replace(staging, target)
            entries.append({'name': src.name, 'size': target.stat().st_size, 'sha256': em.digest(target)})
        return {'status': 'ok', 'input': before, 'tool': tool_id, 'files': entries,
                'elapsed_seconds': round(time.time() - started, 2), 'log': str((log_dir / 'latest.log').relative_to(output))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input-root', type=Path, default=DEFAULT_INPUT)
    parser.add_argument('--tool-dir', type=Path, default=DEFAULT_TOOL)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--classes', type=Path, default=DEFAULT_CLASSES,
                        help='--scan-export-classes TSV (falls back to animation_inventory.json)')
    parser.add_argument('--package', action='append', help='extensionless package path; repeat for a bounded selection')
    parser.add_argument('--batch', type=int, default=100, help='maximum attempts (1–100)')
    parser.add_argument('--workers', type=int, default=24, help='parallel converters (1–24)')
    parser.add_argument('--timeout', type=int, default=300)
    parser.add_argument('--retry-failed', action='store_true')
    parser.add_argument('--verify', action='store_true', help='rehash recorded outputs and inputs before skipping')
    parser.add_argument('--min-free-gb', type=float, default=15.0, help='refuse to start when / has less free space')
    parser.add_argument('--dry-run', action='store_true', help='do not write or run converter')
    args = parser.parse_args()
    if not 1 <= args.batch <= 100 or not 1 <= args.workers <= 24 or not 1 <= args.timeout <= 3600:
        parser.error('batch must be 1–100, workers 1–24, timeout 1–3600 seconds')
    root, tool, output = args.input_root.resolve(), args.tool_dir.resolve(), args.output.resolve()
    if not (root / 'FortniteGame/Content').is_dir():
        parser.error('input root lacks FortniteGame/Content')
    if (PORTING / 'generated').resolve() not in output.parents:
        parser.error('output must be inside porting/generated')
    if output == root or root in output.parents:
        parser.error('output must be outside the input tree')
    tool_id = em.tool_fingerprint(tool)
    names = sorted(set(package_path(p) for p in args.package)) if args.package else anim_packages(args.classes)

    def run():
        state_path = output / 'state.json'
        state = json.loads(state_path.read_text()) if state_path.exists() else {'version': 1, 'input_root': str(root), 'records': {}}
        if state.get('version') != 1 or state.get('input_root') != str(root) or not isinstance(state.get('records'), dict):
            raise ValueError('checkpoint version or input root mismatch')
        records = state['records']
        chosen = select_candidates(names, records, root, output, tool_id, args.retry_failed, args.batch, args.verify)
        print(f'candidates={len(names)} attempts={len(chosen)} dry_run={args.dry_run}', flush=True)
        if args.dry_run or not chosen:
            return 0 if args.dry_run else 3
        free = shutil.disk_usage('/').free / 2**30
        if free < args.min_free_gb:
            print(f'refusing: / has {free:.1f} GB free (< {args.min_free_gb})', flush=True)
            return 4
        (output / 'work').mkdir(parents=True, exist_ok=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
            jobs = {pool.submit(convert, name, root, tool, output, args.timeout, tool_id): name for name in chosen}
            for job in concurrent.futures.as_completed(jobs):
                name = jobs[job]
                try:
                    records[name] = job.result()
                except Exception as e:
                    records[name] = {'status': 'failed', 'error': str(e), 'tool': tool_id}
                print(f"{records[name]['status']}: {name}", flush=True)
        # One checkpoint per batch: the state holds tens of thousands of records.
        em.atomic_json(state_path, state)
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
