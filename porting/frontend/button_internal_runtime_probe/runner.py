#!/usr/bin/env python3
"""Launch a fixed diagnostic command and bind its receipt to the staged ELF."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import time

import verifier
import main_button_verifier
import panel_intro_verifier
import panel_outro_verifier
import confirmation_description_verifier
import common_input_action_row_verifier
import current_input_type_verifier
import binding_data_verifier
import binding_provider_verifier
import binding_creation_verifier
import button_text_style_verifier
import text_block_style_verifier
import text_transform_verifier
import erebus_verifier
import button_size_verifier

ENVIRONMENT = ('DISPLAY', 'VK_ICD_FILENAMES', 'SDL_AUDIODRIVER', 'LP_NUM_THREADS')
BASE_ARGS = ('-stdout', 'FortniteLinux', '/Engine/Maps/Entry', '-vulkan', '-windowed',
             '-ResX=1280', '-ResY=720', '-nosound', '-unattended', '-log',
             '-FortniteLegacySequencer', '-FortniteLegacyFieldPaths', '-NOCONSOLE')


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def runtime_args(path):
    raw = path.read_bytes()
    values = json.loads(raw)
    allowed = {'fortnitelegacymaterialingress', 'fortnitelegacymaterialmanifest',
               'fortnitelegacymaterialmanifestsha256'}
    if not isinstance(values, list) or not values:
        raise ValueError('runtime args must be a nonempty JSON array')
    seen = set()
    for value in values:
        if not isinstance(value, str) or not value.startswith('-') or '=' not in value:
            raise ValueError('invalid runtime argument')
        key, val = value[1:].split('=', 1)
        key = key.lower()
        if key not in allowed or key in seen or not val or any(c in value for c in '\x00\r\n'):
            raise ValueError('disallowed or duplicate runtime argument')
        seen.add(key)
    if seen != allowed:
        raise ValueError('ingress, manifest, and manifest hash arguments are required')
    return values, hashlib.sha256(raw).hexdigest()


def command(binary, case):
    if case in ('registry', 'lifecycle'):
        flags = ('-ContextRuntimeProbe', '-ContextProbeCase=' + case,
                 '-ContextProbeTravelMap=/Engine/Maps/Entry')
    else:
        flags = {'button': ('-ButtonInternalProbe',),
                 'activation': ('-ActivationBaseProbe',),
                 'main-button': ('-MainButtonStateProbe',),
                 'panel-intro': ('-PanelIntroProbe',),
                 'panel-outro': ('-PanelOutroProbe',),
                 'confirmation-description': ('-ConfirmationDescriptionProbe',),
                 'common-input-row': ('-CommonInputActionRowProbe',),
                 'common-input-current-type': ('-CommonInputCurrentTypeProbe',),
                 'action-binding-data': ('-ActionBindingDataProbe',),
                 'action-binding-provider': ('-ActionBindingProviderProbe',),
                 'action-binding-creation': ('-ActionBindingCreationProbe',),
                 'button-text-style': ('-CommonButtonTextStyleProbe',),
                 'text-block-style': ('-CommonTextBlockStyleProbe',),
                 'text-transform': ('-CommonTextTransformProbe',),
                 'erebus': ('-FortGlobalsErebusProbe',),
                 'button-size': ('-ButtonSizeProbe',)}[case]
    return [str(binary), *flags, *BASE_ARGS]


def validate_build(path, binary):
    rows = json.loads(path.read_text())
    if not isinstance(rows, list):
        raise ValueError('build results must be a list')
    matches = [r for r in rows if isinstance(r, dict) and r.get('target') == 'FortniteLinux']
    if len(matches) != 1:
        raise ValueError('expected exactly one FortniteLinux build result')
    row = matches[0]
    record = row.get('binary')
    if (type(row.get('exit')) is not int or row['exit'] != 0 or row.get('source_tree_unchanged') is not True
            or not isinstance(row.get('source_head'), str) or not row['source_head']
            or not isinstance(record, dict) or not isinstance(record.get('path'), str)
            or not record['path'] or not isinstance(record.get('sha256'), str)
            or not isinstance(record.get('size'), int) or isinstance(record.get('size'), bool)):
        raise ValueError('unsuccessful or incomplete FortniteLinux build result')
    if binary.stat().st_size != record['size'] or digest(binary) != record['sha256']:
        raise ValueError('staged binary does not match build result')
    with binary.open('rb') as f:
        if f.read(4) != b'\x7fELF':
            raise ValueError('staged binary is not an ELF executable')
    return row


def stop_group(proc, sig):
    try:
        os.killpg(proc.pid, sig)
    except ProcessLookupError:
        pass


def write_json(path, data):
    with path.open('x') as f:
        json.dump(data, f, indent=2)
        f.write('\n')


def run(args):
    binary = args.binary.resolve(strict=True)
    cwd = args.cwd.resolve(strict=True)
    if not cwd.is_dir() or not 0 < args.timeout <= 3600:
        raise ValueError('cwd must be a directory and timeout must be in (0, 3600]')
    build_file = args.build_results.resolve(strict=True)
    row = validate_build(build_file, binary)
    runtime_file = getattr(args, 'runtime_args', None)
    runtime_record = None
    if runtime_file is not None:
        if args.case not in ('common-input-row', 'button-text-style', 'text-block-style', 'text-transform'):
            raise ValueError('runtime args are only supported for original asset diagnostics')
        runtime_file = runtime_file.resolve(strict=True)
        values, runtime_hash = runtime_args(runtime_file)
        runtime_record = {'path': str(runtime_file), 'sha256': runtime_hash, 'values': values}
    out = args.out.absolute()
    out.mkdir(parents=False, exist_ok=False)
    log = out / 'runtime.log'
    receipt_path = out / 'receipt.json'
    argv = command(binary, args.case)
    if runtime_record is not None:
        argv.extend(runtime_record['values'])
    started = time.time_ns()
    deadline = time.monotonic() + args.timeout
    receipt = {'schema': 'button-diagnostic-run-v1', 'case': args.case,
               'build_results': {'path': str(build_file), 'sha256': digest(build_file)},
               'build': row, 'binary': str(binary), 'argv': argv, 'cwd': str(cwd),
               'environment': {k: os.environ.get(k) for k in ENVIRONMENT},
               'start_ns': started, 'pid': None, 'returncode': None,
               'timed_out': False, 'proc_exe_inode_verified': False,
               'proc_exe': None, 'proc_argv': None, 'proc_cwd': None,
               'binary_after_verified': False, 'error': None}
    if runtime_record is not None:
        receipt['runtime_args'] = runtime_record
    try:
        with log.open('xb') as stream:
            proc = subprocess.Popen(argv, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT,
                                    start_new_session=True, close_fds=True)
            receipt['pid'] = proc.pid
            try:
                actual = os.stat('/proc/%d/exe' % proc.pid)
                expected = binary.stat()
                receipt['proc_exe'] = {'device': actual.st_dev, 'inode': actual.st_ino}
                receipt['proc_exe_inode_verified'] = (actual.st_dev, actual.st_ino) == (expected.st_dev, expected.st_ino)
                proc_dir = Path('/proc') / str(proc.pid)
                # Linux can briefly expose an empty cmdline immediately after exec.
                # Retry only that empty state, while the child is live and within
                # both a short binding window and the overall launch timeout.
                binding_deadline = min(deadline, time.monotonic() + 0.25)
                while True:
                    raw_argv = (proc_dir / 'cmdline').read_bytes()
                    if raw_argv or proc.poll() is not None or time.monotonic() >= binding_deadline:
                        break
                    time.sleep(min(0.005, max(0, binding_deadline - time.monotonic())))
                receipt['proc_argv'] = [os.fsdecode(x) for x in raw_argv.split(b'\0') if x]
                receipt['proc_cwd'] = str((proc_dir / 'cwd').resolve(strict=True))
                if not receipt['proc_exe_inode_verified'] or receipt['proc_argv'] != argv or receipt['proc_cwd'] != str(cwd):
                    receipt['error'] = 'process executable, argv, or cwd differs from requested launch'
                    stop_group(proc, signal.SIGTERM)
            except OSError as exc:
                receipt['error'] = 'could not bind process executable: ' + str(exc)
                stop_group(proc, signal.SIGTERM)
            try:
                receipt['returncode'] = proc.wait(timeout=max(0, deadline - time.monotonic()))
            except subprocess.TimeoutExpired:
                receipt['timed_out'] = True
                stop_group(proc, signal.SIGTERM)
                try:
                    receipt['returncode'] = proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    stop_group(proc, signal.SIGKILL)
                    receipt['returncode'] = proc.wait()
    except OSError as exc:
        receipt['error'] = 'launch failed: ' + str(exc)
    receipt['end_ns'] = time.time_ns()
    try:
        receipt['binary_after_verified'] = (binary.stat().st_size == row['binary']['size'] and
                                             digest(binary) == row['binary']['sha256'])
    except OSError as exc:
        receipt['error'] = (receipt['error'] or '') + ' binary recheck failed: ' + str(exc)
    receipt['log'] = {'path': str(log), 'sha256': digest(log)}
    if args.case in ('button', 'main-button', 'panel-intro', 'panel-outro', 'confirmation-description', 'common-input-row', 'common-input-current-type', 'action-binding-data', 'action-binding-provider', 'action-binding-creation', 'button-text-style', 'text-block-style', 'text-transform', 'erebus', 'button-size'):
        try:
            transcript_verifier = {'button': verifier, 'main-button': main_button_verifier,
                                   'panel-intro': panel_intro_verifier,
                                   'panel-outro': panel_outro_verifier,
                                   'confirmation-description': confirmation_description_verifier,
                                   'common-input-row': common_input_action_row_verifier,
                                   'common-input-current-type': current_input_type_verifier,
                                   'action-binding-data': binding_data_verifier,
                                   'action-binding-provider': binding_provider_verifier,
                                   'action-binding-creation': binding_creation_verifier,
                                   'button-text-style': button_text_style_verifier,
                                   'text-block-style': text_block_style_verifier,
                                   'text-transform': text_transform_verifier,
                                   'erebus': erebus_verifier,
                                   'button-size': button_size_verifier}[args.case]
            result = transcript_verifier.verify(log.read_text(errors='replace'))
        except (ValueError, OSError) as exc:
            result = {'passed': False, 'error': str(exc)[:300]}
        write_json(out / 'verification.json', result)
        receipt['transcript'] = result
    receipt['passed'] = (receipt['returncode'] == 0 and not receipt['timed_out'] and
                         receipt['proc_exe_inode_verified'] and receipt['binary_after_verified'] and
                         receipt['error'] is None and
                         (args.case not in ('button', 'main-button', 'panel-intro', 'panel-outro', 'confirmation-description', 'common-input-row', 'common-input-current-type', 'action-binding-data', 'action-binding-provider', 'action-binding-creation', 'button-text-style', 'text-block-style', 'text-transform', 'erebus', 'button-size') or receipt['transcript']['passed']))
    write_json(receipt_path, receipt)
    return receipt['passed']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-results', type=Path, required=True)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--cwd', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--case', choices=('button', 'main-button', 'panel-intro', 'panel-outro', 'confirmation-description', 'common-input-row', 'common-input-current-type', 'action-binding-data', 'action-binding-provider', 'action-binding-creation', 'button-text-style', 'text-block-style', 'text-transform', 'erebus', 'button-size', 'activation', 'registry', 'lifecycle'), required=True)
    parser.add_argument('--runtime-args', type=Path, help='JSON array of material ingress and manifest flags for original asset diagnostics')
    parser.add_argument('--timeout', type=float, default=240)
    args = parser.parse_args()
    try:
        passed = run(args)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        parser.exit(1, 'runner: %s\n' % exc)
    parser.exit(0 if passed else 1, 'receipt: %s\n' % (args.out / 'receipt.json'))


if __name__ == '__main__':
    main()
