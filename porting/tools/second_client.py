#!/usr/bin/env python3
"""Start a second isolated Wine client inside an active play run's namespaces."""
import argparse
import builtins
import fcntl
import importlib
import os
from pathlib import Path
import pwd
import shutil
import signal
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
PREFIX = ROOT / 'porting/generated/wine-prefix-2-dxvk'
EXE = b'fortniteclient-win64-shipping.exe'
SERVER = ROOT / 'porting/server'
BIN = ROOT / 'Fortnite/FortniteGame/Binaries/Win64'
SCRIPT = Path(__file__).resolve()


def proc(pid):
    p = Path('/proc') / str(pid)
    stat = (p / 'stat').read_text().rsplit(')', 1)[1].split()
    env = {}
    for entry in (p / 'environ').read_bytes().split(b'\0'):
        key, sep, value = entry.partition(b'=')
        if sep and key in (b'WINEPREFIX', b'SHOTS_DIR', b'FN_SHOTS_DIR', b'FN_SECOND_OWNER'):
            env[key] = value
    args = (p / 'cmdline').read_bytes().split(b'\0')
    return {'uid': p.stat().st_uid, 'ticks': int(stat[19]), 'session': int(stat[3]), 'state': stat[0],
            'env': env, 'args': args, 'net': os.readlink(p / 'ns/net'),
            'mnt': os.readlink(p / 'ns/mnt')}


def processes():
    for p in Path('/proc').iterdir():
        if p.name.isdigit():
            try:
                yield int(p.name), proc(int(p.name))
            except (OSError, ValueError, IndexError):
                continue


def shipping(info):
    return bool(info['args'] and info['args'][0].replace(b'\\', b'/').rsplit(b'/', 1)[-1].lower() == EXE)


def first_matches(info, uid, shots, marker):
    started = time.time() - float(Path('/proc/uptime').read_text().split()[0]) + info['ticks'] / os.sysconf('SC_CLK_TCK')
    return (info['uid'] == uid and shipping(info) and
            info['env'].get(b'SHOTS_DIR') == os.fsencode(shots) and
            bool(info['env'].get(b'WINEPREFIX')) and started >= marker.stat().st_mtime - 0.02)


def server_idle(prefix):
    server = os.environ.get('FN_WINESERVER') or shutil.which('wineserver') or '/usr/lib/x86_64-linux-gnu/wine/wineserver'
    if not os.access(server, os.X_OK):
        raise RuntimeError('wineserver unavailable; set FN_WINESERVER')
    env = dict(os.environ, WINEPREFIX=str(prefix))
    try:
        return subprocess.run([server, '-w'], env=env, timeout=5, stdout=subprocess.DEVNULL,
                              stderr=subprocess.DEVNULL).returncode == 0
    except subprocess.TimeoutExpired:
        return False


def prepare():
    if PREFIX.exists() or PREFIX.is_symlink():
        if PREFIX.is_symlink() or not PREFIX.is_dir():
            raise RuntimeError('second prefix is not a real directory')
        return
    source = Path(os.environ.get('FN_DXVK_PREFIX', '/tmp/fortnite-dxvk-experiment/prefix')).expanduser()
    if source.is_symlink() or not source.is_dir():
        raise RuntimeError('source DXVK prefix missing or is a symlink')
    if not server_idle(source):
        raise RuntimeError('source prefix is busy; run --prepare before starting the first client')
    temp = Path(tempfile.mkdtemp(prefix='.wine-prefix-2-', dir=PREFIX.parent))
    try:
        subprocess.run(['cp', '-a', '--reflink=auto', '--', str(source) + '/.', str(temp)], check=True)
        # Check again to reject a source that became active during the copy.
        if not server_idle(source):
            raise RuntimeError('source prefix became busy while copying')
        if PREFIX.exists() or PREFIX.is_symlink():
            raise RuntimeError('second prefix appeared during preparation')
        os.rename(temp, PREFIX)
    finally:
        if temp.exists():
            shutil.rmtree(temp)
    print('Second prefix prepared.', flush=True)


def patch(pid, ticks, uid, net, mnt, kind, log, mode):
    def valid():
        try:
            i = proc(pid)
            return (i['ticks'] == ticks and i['uid'] == uid and i['net'] == net and i['mnt'] == mnt
                    and i['env'].get(b'WINEPREFIX') == os.fsencode(PREFIX) and shipping(i))
        except (OSError, ValueError, IndexError):
            return False
    if not valid():
        raise RuntimeError('second client identity changed')
    sys.path.insert(0, str(SERVER))
    # These modules parse argv at import as well as in main.
    sys.argv = [kind, '86400', str(log), mode] if kind != 'oodle' else [kind, '86400']
    module = importlib.import_module({'curl': 'patch_curl_verify', 'appes': 'patch_appes_diagnostic',
                                      'oodle': 'patch_oodle_min_size'}[kind])
    module.shipping_client_pids = lambda: [pid] if valid() else []
    proc_fd = os.open(f'/proc/{pid}', os.O_RDONLY | os.O_DIRECTORY)
    def guarded_open(path, *args, **kwargs):
        if str(path).startswith('/proc/') and str(path).endswith('/mem'):
            if str(path) != f'/proc/{pid}/mem' or not valid():
                raise OSError('client identity changed')
            # Keep the original proc directory pinned across the check/open race.
            return builtins.open('mem', *args, opener=lambda name, flags: os.open(name, flags, dir_fd=proc_fd), **kwargs)
        return builtins.open(path, *args, **kwargs)
    module.open = guarded_open
    try:
        return module.main() or 0
    finally:
        os.close(proc_fd)


def inner(a):
    if os.geteuid() != 0:
        raise RuntimeError('namespace controller must run as root')
    uid = a.uid
    user = pwd.getpwuid(uid).pw_name
    run = Path(a.run).resolve(strict=True)
    marker = run / 'start.marker'
    shots = run / 'shots'
    net, mnt = os.readlink('/proc/self/ns/net'), os.readlink('/proc/self/ns/mnt')
    if net == os.readlink('/proc/1/ns/net') or mnt == os.readlink('/proc/1/ns/mnt'):
        raise RuntimeError('first client must use private network and mount namespaces')
    # Pin the run owner as well: Wine can outlive its launcher after a timeout.
    runners = [(pid, i) for pid, i in processes() if i['net'] == net and i['mnt'] == mnt
               and i['env'].get(b'FN_SHOTS_DIR') == os.fsencode(shots)
               and os.fsencode(ROOT / 'porting/run_original_client.sh') in i['args']]
    if len(runners) != 1:
        raise RuntimeError('cannot identify one active first-client runner')
    runner_pid, runner_info = runners[0]
    def first_alive():
        try:
            i = proc(a.first)
            owner = proc(runner_pid)
            return (i['ticks'] == a.ticks and i['net'] == net and i['mnt'] == mnt and
                    owner['ticks'] == runner_info['ticks'] and owner['state'] != 'Z' and
                    first_matches(i, uid, shots, marker))
        except (OSError, ValueError, IndexError):
            return False
    if not first_alive():
        raise RuntimeError('first client identity changed')
    first = proc(a.first)
    if first['env'].get(b'WINEPREFIX') == os.fsencode(PREFIX):
        raise RuntimeError('first and second prefixes must differ')
    for arg in first['args']:
        if arg.lower().startswith(b'-auth_password=local-username:'):
            first_username = arg.split(b':', 1)[1].decode('utf-8').strip().casefold()
            if first_username == a.username.strip().casefold():
                raise RuntimeError('second username must differ from first username')
    # Fail closed if the first account cannot be established.
    if not any(x.lower().startswith(b'-auth_password=local-username:') for x in first['args']):
        raise RuntimeError('cannot verify first account')
    if PREFIX.is_symlink() or not PREFIX.is_dir():
        raise RuntimeError('second prefix missing or unsafe')
    if any(i['env'].get(b'WINEPREFIX') == os.fsencode(PREFIX) for _, i in processes()):
        raise RuntimeError('second prefix is already in use')
    out = run / 'client2'
    shotdir = out / 'shots'
    shotdir.mkdir(parents=True, exist_ok=True)
    os.chown(out, uid, a.gid); os.chown(shotdir, uid, a.gid)
    log = PREFIX / 'drive_c/users' / user / 'AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log'
    log.unlink(missing_ok=True)
    if not hasattr(os, 'pidfd_open') or not hasattr(signal, 'pidfd_send_signal'):
        raise RuntimeError('pidfd support is required for safe process cleanup')
    token = os.urandom(32).hex()
    env = ['FN_SECOND_OWNER=' + token, 'WINEPREFIX=' + str(PREFIX), 'WINEDEBUG=-all', 'SHOTS_DIR=' + str(shotdir),
           'FN_ACTIONS=' + a.actions, 'FN_ACTION_LOG=' + str(run / 'server.log'),
           'WINEDLLOVERRIDES=' + ('d3d11,dxgi=n' if a.renderer == 'dxvk' else 'd3d11,dxgi=b')]
    if a.renderer == 'dxvk':
        env += ['VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json', 'LP_NUM_THREADS=' + a.threads]
    cmd = ['sudo', '-n', '-u', user, 'env', *env, 'xvfb-run', '-a', '-s', '-screen 0 1280x720x24',
           'bash', '-c', 'cd "$1"; source "$2"; start_client_actions; exec wine FortniteClient-Win64-Shipping.exe "${@:3}"',
           'second-client', str(BIN), str(ROOT / 'porting/tools/client_actions.sh'),
           '-NOEPICPORTAL', '-nosound', '-NoSplash', '-log', '-epicapp=Fortnite', '-epicenv=Prod',
           '-epiclocale=en-us', '-skippatchcheck', '-nobe', '-noeac', '-fromfl=none',
           '-AUTH_LOGIN=unused', '-AUTH_PASSWORD=local-username:' + a.username, '-AUTH_TYPE=exchangecode']
    child = None
    workers = {}
    reported = set()
    owned = {}
    launch_ticks = 0
    stop = False
    def interrupted(signum, frame):
        nonlocal stop
        stop = True
    for sig in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        signal.signal(sig, interrupted)
    def collect():
        if child is None:
            return
        leader_alive = child.poll() is None
        for pid, i in processes():
            token_owned = (i['uid'] == uid and i['env'].get(b'FN_SECOND_OWNER') == os.fsencode(token)
                           and i['env'].get(b'WINEPREFIX') == os.fsencode(PREFIX))
            session_owned = leader_alive and i['session'] == child.pid
            if ((pid, i['ticks']) not in owned and i['ticks'] >= launch_ticks and i['net'] == net and i['mnt'] == mnt
                    and (token_owned or session_owned)):
                fd = None
                try:
                    fd = os.pidfd_open(pid)
                    if proc(pid)['ticks'] == i['ticks']:
                        owned[(pid, i['ticks'])] = fd
                        fd = None
                except (OSError, ValueError, IndexError):
                    pass
                finally:
                    if fd is not None:
                        os.close(fd)
    try:
        if not first_alive():
            raise RuntimeError('first client ended before launch')
        launch_ticks = int(float(Path('/proc/uptime').read_text().split()[0]) * os.sysconf('SC_CLK_TCK')) - 1
        child = subprocess.Popen(cmd, start_new_session=True)
        deadline = time.monotonic() + 90
        target = None
        while child.poll() is None and first_alive() and not stop:
            collect()
            found = [(pid, i) for pid, i in processes() if i['uid'] == uid and i['net'] == net and
                     i['mnt'] == mnt and i['env'].get(b'WINEPREFIX') == os.fsencode(PREFIX) and
                     i['env'].get(b'FN_SECOND_OWNER') == os.fsencode(token) and shipping(i)]
            if len(found) > 1:
                raise RuntimeError('multiple second clients found')
            if found and target is None:
                target, info = found[0]
                print(f'second client pid {target}', flush=True)
                modes = [('curl', a.verify == '1', ''), ('appes', a.appes in ('1', 'callback', 'timer', 'handler-only', 'timer-only'),
                          'callback' if a.appes == '1' else a.appes), ('oodle', a.oodle == '1', '')]
                for kind, enabled, mode in modes:
                    if enabled:
                        workers[kind] = subprocess.Popen([sys.executable, str(SCRIPT), '--patch', str(target), str(info['ticks']),
                                                           str(uid), net, mnt, kind, str(log), mode])
            if target is not None and not any(pid == target and i['ticks'] == info['ticks'] for pid, i in found):
                print('Second client ended; stopping its launcher.', flush=True)
                break
            failed = False
            for kind, worker in workers.items():
                status = worker.poll()
                if status is not None and kind not in reported:
                    print(f'{kind} patch exited with status {status}', flush=True)
                    reported.add(kind)
                    if status != 0:
                        failed = True
            if failed:
                break
            if target is None and time.monotonic() >= deadline:
                raise RuntimeError('second client did not start within 90 seconds')
            time.sleep(.2)
        collect()
        if not first_alive():
            print('First client ended; stopping second client.', flush=True)
        if target is None:
            print('Second client exited before its process was identified.', flush=True)
            return 1
        patch_failure = False
        for kind, worker in workers.items():
            status = worker.poll()
            if kind in ('curl', 'appes') and status != 0:
                if kind not in reported:
                    print(f'{kind} patch did not complete successfully', flush=True)
                patch_failure = True
            elif status is not None and status != 0:
                patch_failure = True
        if patch_failure:
            return 1
        if child.returncode is None:
            return 0
        return child.returncode if child.returncode >= 0 else 128 - child.returncode
    finally:
        for w in workers.values():
            if w.poll() is None:
                w.terminate()
        for w in workers.values():
            try: w.wait(timeout=5)
            except subprocess.TimeoutExpired: w.kill(); w.wait()
        collect()
        for sig in (signal.SIGTERM, signal.SIGKILL):
            if sig == signal.SIGKILL:
                collect()
            for fd in list(owned.values()):
                try:
                    signal.pidfd_send_signal(fd, sig)
                except OSError:
                    pass
            if sig == signal.SIGTERM:
                time.sleep(1)
        for fd in owned.values():
            os.close(fd)
        if child is not None:
            try: child.wait(timeout=5)
            except subprocess.TimeoutExpired: pass
        if log.is_file():
            shutil.copyfile(log, out / 'client.log')
            os.chown(out / 'client.log', uid, a.gid)


def outer(a):
    if os.geteuid() == 0:
        raise RuntimeError('run the controller as the ordinary desktop user')
    PREFIX.parent.mkdir(parents=True, exist_ok=True)
    lock_path = PREFIX.parent / '.wine-prefix-2-dxvk.lock'
    fd = os.open(lock_path, os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
    with os.fdopen(fd, 'r+') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise RuntimeError('second prefix is locked by another controller')
        if not a.prepare and not a.run:
            raise RuntimeError('RUN_DIR is required unless --prepare is specified')
        if PREFIX.is_dir() and not PREFIX.is_symlink() and not server_idle(PREFIX):
            raise RuntimeError('second prefix is busy')
        prepare()
        if a.prepare:
            return 0
        if not server_idle(PREFIX):
            raise RuntimeError('second prefix is busy')
        run = Path(a.run).resolve(strict=True)
        marker = run / 'start.marker'
        if not marker.is_file() or not (run / 'shots').is_dir():
            raise RuntimeError('RUN_DIR is missing start.marker or shots/')
        shots = run / 'shots'
        matches = [(pid, i) for pid, i in processes() if first_matches(i, os.getuid(), shots, marker)]
        if len(matches) != 1:
            raise RuntimeError(f'expected one first client for run, found {len(matches)}')
        first, info = matches[0]
        if info['net'] == os.readlink('/proc/self/ns/net') or info['mnt'] == os.readlink('/proc/self/ns/mnt'):
            raise RuntimeError('first client does not have private network and mount namespaces')
        out = run / 'client2'
        out.mkdir(exist_ok=True)
        inner_cmd = ['sudo', '-n', 'nsenter', '-t', str(first), '-n', '-m', '--', 'python3', str(SCRIPT),
                     '--inner', '--run', str(run), '--first', str(first), '--ticks', str(info['ticks']),
                     '--uid', str(os.getuid()), '--gid', str(os.getgid()), '--username', a.username,
                     '--renderer', a.renderer, '--actions', a.actions, '--threads', os.environ.get('FN_LP_NUM_THREADS', '8'),
                     '--verify', os.environ.get('FN_PATCH_VERIFY', '1'),
                     '--appes', os.environ.get('FN_PATCH_APPES_DIAGNOSTIC', '1'),
                     '--oodle', os.environ.get('FN_PATCH_OODLE', '1')]
        with open(out / 'runner.out', 'ab', buffering=0) as output:
            child = subprocess.Popen(inner_cmd, stdout=output, stderr=output)
            def forward(signum, frame):
                if child.poll() is None:
                    child.send_signal(signum)
            handlers = {s: signal.signal(s, forward) for s in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP)}
            try:
                return child.wait()
            finally:
                for s, handler in handlers.items():
                    signal.signal(s, handler)


def main():
    if len(sys.argv) > 1 and sys.argv[1] == '--patch':
        _, _, pid, ticks, uid, net, mnt, kind, log, mode = sys.argv
        return patch(int(pid), int(ticks), int(uid), net, mnt, kind, Path(log), mode)
    if len(sys.argv) > 1 and sys.argv[1] == '--inner':
        p = argparse.ArgumentParser()
        p.add_argument('--inner', action='store_true')
        for name in ('run', 'username', 'renderer', 'actions', 'threads', 'verify', 'appes', 'oodle'):
            p.add_argument('--' + name, required=True)
        for name in ('first', 'ticks', 'uid', 'gid'):
            p.add_argument('--' + name, type=int, required=True)
        return inner(p.parse_args())
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('run', nargs='?', metavar='RUN_DIR')
    p.add_argument('--prepare', action='store_true', help='prepare the second prefix while the source prefix is idle')
    p.add_argument('--username', default='LocalPlayer2')
    p.add_argument('--renderer', choices=('dxvk', 'builtin'), default='dxvk')
    p.add_argument('--actions', default=os.environ.get('FN_ACTIONS', ''))
    a = p.parse_args()
    if not a.username.strip() or '\0' in a.username or ':' in a.username:
        p.error('username must be nonempty and cannot contain a colon')
    if a.prepare and a.run:
        p.error('--prepare does not take RUN_DIR')
    return outer(a)


if __name__ == '__main__':
    try:
        sys.exit(main())
    except UnicodeError:
        print('error: invalid account encoding', file=sys.stderr)
        sys.exit(1)
    except (RuntimeError, ValueError) as exc:
        print(f'error: {exc}', file=sys.stderr)
        sys.exit(1)
    except OSError as exc:
        print(f'error: {exc.strerror or "operating system failure"}', file=sys.stderr)
        sys.exit(1)
    except subprocess.SubprocessError:
        print('error: subprocess failed', file=sys.stderr)
        sys.exit(1)
