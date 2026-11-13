#!/usr/bin/env python3
"""Syntax and mocked integration tests for opt-in FN_REGISTERED_MATCHMAKING runner wiring."""

import json
import os
import pwd
import re
import shlex
import shutil
import socket
import stat
import subprocess
import sys
import tempfile
import time
import unittest
import urllib.error
import urllib.request
from pathlib import Path

PORTING = Path(__file__).resolve().parents[1]
REPO_ROOT = PORTING.parent


def free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def write_exec(path, content):
    path.write_text(content, encoding="utf-8")
    path.chmod(0o755)


class RegisteredRunnerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        (PORTING / "generated/runs").mkdir(parents=True, exist_ok=True)
        cls._tmp = tempfile.TemporaryDirectory(prefix="registered-runner-test-", dir=PORTING / "generated/runs")
        cls.sandbox = Path(cls._tmp.name)
        cls.porting = cls.sandbox / "porting"
        (cls.porting / "tools").mkdir(parents=True)
        (cls.porting / "generated" / "tls").mkdir(parents=True)
        (cls.porting / "generated" / "runs").mkdir(parents=True)
        (cls.sandbox / "Fortnite" / "FortniteGame" / "Binaries" / "Win64").mkdir(parents=True)

        # Exercise the actual production scripts inside an isolated fixture.
        shutil.copy2(REPO_ROOT / "porting/run_original_client.sh", cls.porting / "run_original_client.sh")
        shutil.copy2(REPO_ROOT / "porting/tools/vanilla_play_run.sh", cls.porting / "tools/vanilla_play_run.sh")
        shutil.copy2(REPO_ROOT / "porting/tools/client_actions.sh", cls.porting / "tools/client_actions.sh")
        shutil.copy2(REPO_ROOT / "porting/tools/second_client.sh", cls.porting / "tools/second_client.sh")
        shutil.copy2(REPO_ROOT / "porting/tools/second_client.py", cls.porting / "tools/second_client.py")

        # Copy server scripts so probe_server.py resolves generated/ inside the sandbox.
        shutil.copytree(
            REPO_ROOT / "porting/server",
            cls.porting / "server",
            ignore=shutil.ignore_patterns("__pycache__", "tests"),
        )
        (cls.porting / "generated" / "DefaultEngine.ini").write_text("", encoding="utf-8")
        (cls.porting / "generated" / "hosts").write_text("127.0.0.1 localhost\n", encoding="utf-8")

        # Generate real self-signed TLS cert once for probe_server.py tests.
        subprocess.run(
            [str(REPO_ROOT / "porting/server/gen_tls_cert.sh"), str(cls.porting / "generated/tls")],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    @classmethod
    def tearDownClass(cls):
        cls._tmp.cleanup()

    def make_shims(self, workdir, *, record_sudo_argv=True, record_wine_env=True, wait_for_files=()):
        bin_dir = workdir / "bin"
        bin_dir.mkdir(parents=True, exist_ok=True)
        sudo_log = workdir / "sudo-argv.jsonl"
        wine_env_log = workdir / "wine-env.json"

        # No-op root/namespace commands for unprivileged inner testing.
        for tool in ("ip", "mount", "chown", "xvfb-run"):
            if tool == "xvfb-run":
                write_exec(bin_dir / tool, '#!/usr/bin/env bash\nwhile [[ $# -gt 0 && $1 != timeout ]]; do shift; done\nexec "$@"\n')
            else:
                write_exec(bin_dir / tool, "#!/usr/bin/env bash\nexit 0\n")

        write_exec(bin_dir / "wineserver", "#!/usr/bin/env bash\nexit 0\n")
        write_exec(bin_dir / "tcpdump", "#!/usr/bin/env bash\nexit 0\n")

        # Fake wine optionally waits for background files/markers and captures its env.
        wait_specs = [(str(p), sub) for p, sub in wait_for_files]
        write_exec(
            bin_dir / "wine",
            f"""#!/usr/bin/env python3
import json, os, time
specs = {wait_specs!r}
deadline = time.monotonic() + 5.0
while time.monotonic() < deadline:
    ready = True
    for path, substr in specs:
        if not os.path.exists(path):
            ready = False
            break
        if substr and substr not in open(path, encoding="utf-8", errors="replace").read():
            ready = False
            break
    if ready:
        break
    time.sleep(0.05)
if {record_wine_env!r}:
    with open({str(wine_env_log)!r}, "w") as f:
        json.dump(dict(os.environ), f)
""",
        )

        # Fake sudo logs every argv entry and either records outer unshare call or executes inner command.
        write_exec(
            bin_dir / "sudo",
            f"""#!/usr/bin/env python3
import json, os, sys
argv = sys.argv[1:]
if {record_sudo_argv!r}:
    with open({str(sudo_log)!r}, "a") as f:
        f.write(json.dumps(argv) + "\\n")
# Outer recursion check: `sudo -n env ... unshare -n -m .../run_original_client.sh ...`
if "unshare" in argv:
    sys.exit(0)
# Inner `sudo -n -u <user> env ...`: strip -n and -u <user>, then exec directly.
rest = []
i = 0
while i < len(argv):
    if argv[i] == "-n":
        i += 1
    elif argv[i] == "-u":
        i += 2
    else:
        rest = argv[i:]
        break
os.execvp(rest[0], rest)
""",
        )
        return bin_dir, sudo_log, wine_env_log

    def base_env(self, bin_dir, workdir):
        prefix = workdir / "wine-prefix"
        prefix.mkdir(parents=True, exist_ok=True)
        user = pwd.getpwuid(os.getuid()).pw_name
        env = {
            "PATH": f"{bin_dir}:{os.environ['PATH']}",
            "HOME": os.environ.get("HOME", str(workdir)),
            "USER": user,
            "FN_WINEPREFIX": str(prefix),
            "FN_PATCH_VERIFY": "0",
            "FN_PATCH_OODLE": "0",
            "FN_PATCH_APPES_DIAGNOSTIC": "0",
            "FN_ACTIONS": "",
            "FN_PORTAL_ARG": "-NOEPICPORTAL",
            "FN_EPICENV": "Prod",
            "FN_DISPLAY": ":99",
            "FN_SERVER_START_WAIT": "0",
            "FN_STATE_DIR": str(workdir / "state"),
            "FN_SHOTS_DIR": str(workdir / "shots"),
        }
        return env

    def test_01_bash_syntax(self):
        for script in (
            self.porting / "run_original_client.sh",
            self.porting / "tools/vanilla_play_run.sh",
            self.porting / "tools/second_client.sh",
        ):
            res = subprocess.run(["bash", "-n", str(script)], capture_output=True, text=True)
            self.assertEqual(res.returncode, 0, f"{script}: {res.stderr}")
        py_res = subprocess.run(
            [sys.executable, "-m", "py_compile", str(self.porting / "tools/second_client.py")],
            capture_output=True,
            text=True,
        )
        self.assertEqual(py_res.returncode, 0, py_res.stderr)

    def test_02_outer_sudo_forwarding_flag_off_and_on(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            bin_dir, sudo_log, _ = self.make_shims(work)

            # Flag OFF
            env_off = self.base_env(bin_dir, work)
            res = subprocess.run(
                [str(self.porting / "run_original_client.sh"), "15"],
                env=env_off,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, res.stderr)
            argv_off = json.loads(sudo_log.read_text().splitlines()[-1])
            self.assertIn("FN_REGISTERED_MATCHMAKING=0", argv_off)
            self.assertFalse(any(a.startswith("FORTNITE_GAMESERVER_SECRET=") for a in argv_off))

            # Flag ON with explicit 0600 secret file
            secret_file = work / "explicit.secret"
            secret_val = "s3cr3t-v4lu3-never-in-argv"
            secret_file.write_text(secret_val, encoding="utf-8")
            secret_file.chmod(0o600)

            env_on = self.base_env(bin_dir, work)
            env_on.update(
                {
                    "FN_REGISTERED_MATCHMAKING": "1",
                    "FORTNITE_GAMESERVER_SECRET_FILE": str(secret_file),
                    "FORTNITE_GAMESERVER_PORT": "7780",
                    "FORTNITE_GAMESERVER_BEACON_PORT": "7781",
                    "FORTNITE_GAMESERVER_CAPACITY": "50",
                    "FORTNITE_GAMESERVER_PLAYLIST": "Playlist_DefaultSolo",
                    "FORTNITE_GAMESERVER_BUILD": "12896485",
                    "FORTNITE_GAMESERVER_ADDRESS": "127.0.0.1",
                }
            )
            res = subprocess.run(
                [str(self.porting / "run_original_client.sh"), "15"],
                env=env_on,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, res.stderr)
            argv_on = json.loads(sudo_log.read_text().splitlines()[-1])
            self.assertIn("FN_REGISTERED_MATCHMAKING=1", argv_on)
            self.assertIn(f"FORTNITE_GAMESERVER_SECRET_FILE={secret_file}", argv_on)
            self.assertIn("FORTNITE_GAMESERVER_PORT=7780", argv_on)
            self.assertIn("FORTNITE_GAMESERVER_BEACON_PORT=7781", argv_on)
            self.assertIn("FORTNITE_GAMESERVER_CAPACITY=50", argv_on)
            for item in argv_on:
                self.assertNotIn(secret_val, item)
                self.assertFalse(item.startswith("FORTNITE_GAMESERVER_SECRET="))

    def test_03_inner_flag_off_preserves_static_local_mode(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            server_env_out = work / "server-env.json"
            bin_dir, _, wine_env_log = self.make_shims(
                work,
                wait_for_files=[
                    (server_env_out, ""),
                    (self.porting / "generated/probe-server.out", "Listening on 127.0.0.1:"),
                ],
            )
            netkey = work / "netkey"
            netkey.write_text("ab" * 32, encoding="utf-8")
            netkey.chmod(0o600)
            server_script = work / "dump_env.py"
            server_script.write_text(
                f'import json, os\njson.dump(dict(os.environ), open({str(server_env_out)!r}, "w"))\n',
                encoding="utf-8",
            )

            env = self.base_env(bin_dir, work)
            env.update(
                {
                    "FN_NETNS_INNER": "1",
                    "FN_USER": env["USER"],
                    "FN_USERNAME": "LocalPlayer",
                    "FN_BACKEND_PORT": str(free_port()),
                    "FORTNITE_NET_ENCRYPTION_KEY_FILE": str(netkey),
                    "FN_SERVER_CMD": f"exec {sys.executable} {server_script}",
                }
            )
            res = subprocess.run(
                [str(self.porting / "run_original_client.sh"), "15"],
                env=env,
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, res.stderr)
            server_env = json.loads(server_env_out.read_text())
            self.assertEqual(server_env.get("FORTNITE_NET_ENCRYPTION_KEY_FILE"), str(netkey))
            self.assertNotIn("FORTNITE_GAMESERVER_URL", server_env)
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET", server_env)
            probe_out = (self.porting / "generated/probe-server.out").read_text()
            self.assertIn("Local matchmaking WebSocket on 127.0.0.1:", probe_out)
            self.assertNotIn("Registered matchmaking trace:", res.stdout)
            wine_env = json.loads(wine_env_log.read_text())
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET", wine_env)

    def test_04_inner_registered_auto_secret_and_real_backend_registration(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            reg_result = work / "reg-result.json"
            bin_dir, sudo_log, wine_env_log = self.make_shims(work, wait_for_files=[(reg_result, "")])

            # Mock server performs an actual authenticated POST /local/gameserver/register
            # against the live probe_server.py plain loopback listener, plus inspects cmdline/env.
            server_script = work / "mock_server.py"
            server_script.write_text(
                f"""import json, os, urllib.request
from pathlib import Path
env = dict(os.environ)
url = env["FORTNITE_GAMESERVER_URL"]
secret = env["FORTNITE_GAMESERVER_SECRET"]
req = urllib.request.Request(
    url + "/local/gameserver/register",
    data=json.dumps({{
        "address": env["FORTNITE_GAMESERVER_ADDRESS"],
        "port": int(env["FORTNITE_GAMESERVER_PORT"]),
        "beaconPort": int(env["FORTNITE_GAMESERVER_BEACON_PORT"]),
        "playlists": [env["FORTNITE_GAMESERVER_PLAYLIST"]],
        "maxPlayers": int(env["FORTNITE_GAMESERVER_CAPACITY"]),
        "buildUniqueId": env["FORTNITE_GAMESERVER_BUILD"],
    }}).encode("utf-8"),
    headers={{"Authorization": "Bearer " + secret, "Content-Type": "application/json"}},
    method="POST",
)
with urllib.request.urlopen(req, timeout=5) as resp:
    body = json.loads(resp.read().decode("utf-8"))
    status = resp.status

# Verify registered matchmaking disabled static local fallback (unknown session key returns 404).
key_err_status = None
try:
    urllib.request.urlopen(url + "/fortnite/api/game/v2/matchmaking/account/00000000000000000000000000000001/session/00000000000000000000000000000002", timeout=5)
except urllib.error.HTTPError as exc:
    key_err_status = exc.code

# Match second_client.py runner discovery logic (FN_SHOTS_DIR + exact run_original_client.sh arg).
shots_bytes = os.fsencode({str(work / "shots")!r})
runner_bytes = os.fsencode({str(self.porting / "run_original_client.sh")!r})
cmdline_matches = []
for p in Path("/proc").iterdir():
    if p.name.isdigit():
        try:
            args = (p / "cmdline").read_bytes().split(b"\\0")
            penv = dict(e.split(b"=", 1) for e in (p / "environ").read_bytes().split(b"\\0") if b"=" in e)
            if penv.get(b"FN_SHOTS_DIR") == shots_bytes and runner_bytes in args:
                cmdline_matches.append(int(p.name))
        except OSError:
            pass

with open({str(reg_result)!r}, "w") as f:
    json.dump({{
        "status": status,
        "key_err_status": key_err_status,
        "sessionId": body["sessionId"],
        "encryptionKey": body["encryptionKey"],
        "env": env,
        "secret_file_mode": oct(os.stat(env["FORTNITE_GAMESERVER_SECRET_FILE"]).st_mode & 0o777),
        "runner_cmdline_count": len(cmdline_matches),
    }}, f)
""",
                encoding="utf-8",
            )

            env = self.base_env(bin_dir, work)
            env.update(
                {
                    "FN_NETNS_INNER": "1",
                    "FN_USER": env["USER"],
                    "FN_USERNAME": "LocalPlayer",
                    "FN_REGISTERED_MATCHMAKING": "1",
                    "FN_BACKEND_PORT": str(free_port()),
                    "FN_SESSION_TRACE": "1",
                    "FORTNITE_NET_ENCRYPTION_KEY_FILE": "/should/not/be/forwarded",
                    "FN_SERVER_CMD": f"exec {sys.executable} {server_script}",
                }
            )
            res = subprocess.run(
                [str(self.porting / "run_original_client.sh"), "20"],
                env=env,
                capture_output=True,
                text=True,
            )
            probe_log = (self.porting / "generated/probe-server.out").read_text() if (self.porting / "generated/probe-server.out").exists() else ""
            self.assertEqual(res.returncode, 0, res.stderr + "\n" + res.stdout + "\nprobe:\n" + probe_log)

            data = json.loads(reg_result.read_text())
            self.assertEqual(data["status"], 200)
            self.assertEqual(data["key_err_status"], 404)
            self.assertEqual(len(data["sessionId"]), 32)
            self.assertEqual(data["secret_file_mode"], "0o600")
            # second_client.py requires exactly one runner process matching run_original_client.sh.
            self.assertEqual(data["runner_cmdline_count"], 1)

            s_env = data["env"]
            secret_val = s_env["FORTNITE_GAMESERVER_SECRET"]
            secret_file_path = Path(s_env["FORTNITE_GAMESERVER_SECRET_FILE"])
            self.assertEqual(len(secret_val), 64)
            self.assertEqual(s_env["FORTNITE_GAMESERVER_ADDRESS"], "127.0.0.1")
            self.assertEqual(s_env["FORTNITE_GAMESERVER_PLAYLIST"], "Playlist_DefaultSolo")
            self.assertEqual(s_env["FORTNITE_GAMESERVER_BUILD"], "12896485")
            self.assertEqual(s_env["FORTNITE_GAMESERVER_PORT"], "7777")
            self.assertEqual(s_env["FORTNITE_GAMESERVER_BEACON_PORT"], "7778")
            self.assertEqual(s_env["FORTNITE_GAMESERVER_CAPACITY"], "100")
            self.assertRegex(s_env["FORTNITE_GAMESERVER_URL"], r"^http://127\.0\.0\.1:[0-9]+$")
            self.assertNotIn("FORTNITE_NET_ENCRYPTION_KEY_FILE", s_env)
            self.assertNotIn("FORTNITE_NET_ENCRYPTION_KEY", s_env)

            # Ephemeral auto-generated secret file must be cleaned up on exit.
            self.assertFalse(secret_file_path.exists())

            # Wine client must never receive server secret or secret file path.
            wine_env = json.loads(wine_env_log.read_text())
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET", wine_env)
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET_FILE", wine_env)

            # Secret literal and encryptionKey must never appear in stdout, stderr, probe-server.out, or sudo argv.
            ws_port = s_env["FORTNITE_GAMESERVER_URL"].rsplit(":", 1)[1]
            self.assertIn(f"FN_REGISTERED_MATCHMAKING=1 loopback_port={ws_port}", res.stdout)
            for sensitive in (secret_val, data["encryptionKey"], data["sessionId"], "Authorization"):
                self.assertNotIn(sensitive, res.stdout)
                self.assertNotIn(sensitive, res.stderr)
                self.assertNotIn(sensitive, (self.porting / "generated/probe-server.out").read_text())
                self.assertNotIn(sensitive, sudo_log.read_text())

    def test_05_inner_registered_explicit_secret_file_preserved(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            reg_result = work / "reg-explicit.json"
            bin_dir, sudo_log, _ = self.make_shims(work, wait_for_files=[(reg_result, "")])
            explicit_file = work / "custom.secret"
            explicit_secret = "explicit-shared-secret-value-0123456789"
            explicit_file.write_text(explicit_secret, encoding="utf-8")
            explicit_file.chmod(0o600)

            server_script = work / "mock_server_explicit.py"
            server_script.write_text(
                f"""import json, os, urllib.request
env = dict(os.environ)
req = urllib.request.Request(
    env["FORTNITE_GAMESERVER_URL"] + "/local/gameserver/register",
    data=json.dumps({{
        "address": env["FORTNITE_GAMESERVER_ADDRESS"],
        "port": int(env["FORTNITE_GAMESERVER_PORT"]),
        "beaconPort": int(env["FORTNITE_GAMESERVER_BEACON_PORT"]),
        "playlists": [env["FORTNITE_GAMESERVER_PLAYLIST"]],
        "maxPlayers": int(env["FORTNITE_GAMESERVER_CAPACITY"]),
        "buildUniqueId": env["FORTNITE_GAMESERVER_BUILD"],
    }}).encode("utf-8"),
    headers={{"Authorization": "Bearer " + env["FORTNITE_GAMESERVER_SECRET"], "Content-Type": "application/json"}},
    method="POST",
)
with urllib.request.urlopen(req, timeout=5) as resp:
    json.dump({{"status": resp.status}}, open({str(reg_result)!r}, "w"))
""",
                encoding="utf-8",
            )

            env = self.base_env(bin_dir, work)
            env.update(
                {
                    "FN_NETNS_INNER": "1",
                    "FN_USER": env["USER"],
                    "FN_USERNAME": "LocalPlayer",
                    "FN_REGISTERED_MATCHMAKING": "1",
                    "FN_BACKEND_PORT": str(free_port()),
                    "FORTNITE_GAMESERVER_SECRET_FILE": str(explicit_file),
                    "FN_SERVER_CMD": f"exec {sys.executable} {server_script}",
                }
            )
            res = subprocess.run(
                [str(self.porting / "run_original_client.sh"), "20"],
                env=env,
                capture_output=True,
                text=True,
            )
            probe_log = (self.porting / "generated/probe-server.out").read_text() if (self.porting / "generated/probe-server.out").exists() else ""
            self.assertEqual(res.returncode, 0, res.stderr + "\n" + res.stdout + "\nprobe:\n" + probe_log)
            self.assertEqual(json.loads(reg_result.read_text())["status"], 200)
            self.assertTrue(explicit_file.exists())
            self.assertEqual(explicit_file.read_text(), explicit_secret)
            self.assertNotIn(explicit_secret, sudo_log.read_text())
            self.assertNotIn(explicit_secret, res.stdout)

    def test_06_vanilla_play_run_builds_expected_server_cmd_and_meta(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            porting = work / "porting"
            (porting / "tools").mkdir(parents=True)
            (porting / "generated/runs").mkdir(parents=True)
            (porting / "generated/wine-prefix").mkdir(parents=True)
            shutil.copy2(self.porting / "run_original_client.sh", porting / "run_original_client.sh")
            shutil.copy2(self.porting / "tools/vanilla_play_run.sh", porting / "tools/vanilla_play_run.sh")

            captured = work / "captured.json"
            write_exec(
                porting / "run_original_client.sh",
                f"""#!/usr/bin/env bash
if [[ "${{1:-}}" == --validate-registered ]]; then
    exec {str(self.porting / "run_original_client.sh")!r} --validate-registered
fi
python3 - {str(captured)!r} <<'PYEOF'
import json, os, sys
json.dump({{k: v for k, v in os.environ.items() if k.startswith(("FN_", "FORTNITE_"))}}, open(sys.argv[1], "w"))
PYEOF
""",
            )
            bin_dir, _, _ = self.make_shims(work)

            # Flag ON in vanilla_play_run.sh
            env_on = {
                "PATH": f"{bin_dir}:{os.environ['PATH']}",
                "HOME": os.environ.get("HOME", str(work)),
                "USER": pwd.getpwuid(os.getuid()).pw_name,
                "FN_RUN_LABEL": "reg-test",
                "FN_REGISTERED_MATCHMAKING": "1",
                "FORTNITE_GAMESERVER_PORT": "7785",
                "FN_SERVER_ARGS": "-FortniteWarmupSeconds=10",
                "FN_STATE_DIR": str(work / "state"),
            }
            res = subprocess.run(
                [str(porting / "tools/vanilla_play_run.sh"), "30"],
                env=env_on,
                capture_output=True,
                text=True,
            )
            # vanilla_play_run.sh exits 1 at end because no real FortniteLinuxServer/udp.pcap appeared;
            # verify the constructed command and metadata.
            self.assertTrue(captured.exists(), f"captured.json missing; rc={res.returncode} stdout={res.stdout} stderr={res.stderr}")
            cap = json.loads(captured.read_text())
            self.assertEqual(cap["FN_REGISTERED_MATCHMAKING"], "1")
            self.assertIn("-FortniteRegisteredMatchmaking", cap["FN_SERVER_CMD"])
            self.assertIn("-port=7785", cap["FN_SERVER_CMD"])
            self.assertIn("-FortniteWarmupSeconds=10", cap["FN_SERVER_CMD"])
            default_logs = "LogHandshake Log, LogNet Log, LogNetTraffic Warning, PacketHandlerLog Log"
            self.assertIn("-LogCmds=" + default_logs, shlex.split(cap["FN_SERVER_CMD"]))
            self.assertNotIn("FORTNITE_NET_ENCRYPTION_KEY_FILE", cap)
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET", cap)

            run_dirs = list((porting / "generated/runs").glob("*-reg-test-*"))
            self.assertEqual(len(run_dirs), 1)
            meta = json.loads((run_dirs[0] / "meta.json").read_text())
            self.assertEqual(meta["env"].get("FN_REGISTERED_MATCHMAKING"), "1")
            self.assertEqual(meta["env"]["FN_STATE_DIR"], str(work / "state"))
            self.assertEqual(meta["env"]["FN_SERVER_LOG_CMDS"], default_logs)
            self.assertNotIn("FORTNITE_GAMESERVER_SECRET", meta["env"])

            # Flag OFF in vanilla_play_run.sh
            captured.unlink()
            env_off = {
                "PATH": f"{bin_dir}:{os.environ['PATH']}",
                "HOME": os.environ.get("HOME", str(work)),
                "USER": pwd.getpwuid(os.getuid()).pw_name,
                "FN_RUN_LABEL": "local-test",
                "FN_SERVER_LOG_CMDS": "LogNetTraffic Verbose, LogNet VeryVerbose",
            }
            subprocess.run(
                [str(porting / "tools/vanilla_play_run.sh"), "30"],
                env=env_off,
                capture_output=True,
                text=True,
            )
            cap_off = json.loads(captured.read_text())
            self.assertNotIn("-FortniteRegisteredMatchmaking", cap_off["FN_SERVER_CMD"])
            self.assertIn("-port=7777", cap_off["FN_SERVER_CMD"])
            self.assertIn("-LogCmds=" + env_off["FN_SERVER_LOG_CMDS"], shlex.split(cap_off["FN_SERVER_CMD"]))
            local_run = next((porting / "generated/runs").glob("*-local-test-*"))
            local_meta = json.loads((local_run / "meta.json").read_text())
            self.assertEqual(local_meta["env"]["FN_SERVER_LOG_CMDS"], env_off["FN_SERVER_LOG_CMDS"])
            self.assertEqual(
                cap_off.get("FORTNITE_NET_ENCRYPTION_KEY_FILE"),
                str(porting / "generated/g3/netkey"),
            )

    def test_07_fail_closed_on_invalid_config_secret_and_unavailable_listener(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            bin_dir, _, _ = self.make_shims(work)

            bad_secret_nl = work / "newline.secret"
            bad_secret_nl.write_text("abc\n", encoding="utf-8")
            bad_secret_nl.chmod(0o600)

            bad_secret_perm = work / "perm.secret"
            bad_secret_perm.write_text("abc", encoding="utf-8")
            bad_secret_perm.chmod(0o644)

            symlink_secret = work / "link.secret"
            target_secret = work / "target.secret"
            target_secret.write_text("abc", encoding="utf-8")
            target_secret.chmod(0o600)
            symlink_secret.symlink_to(target_secret)

            cases = [
                ("bad_flag", {"FN_REGISTERED_MATCHMAKING": "maybe"}),
                ("raw_secret", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_SECRET": "raw"}),
                ("external_url", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_URL": "http://127.0.0.1:9000"}),
                ("missing_file", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_SECRET_FILE": str(work / "missing")}),
                ("newline_file", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_SECRET_FILE": str(bad_secret_nl)}),
                ("perm_file", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_SECRET_FILE": str(bad_secret_perm)}),
                ("symlink_file", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_SECRET_FILE": str(symlink_secret)}),
                ("bad_port", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_PORT": "70000"}),
                ("bad_capacity", {"FN_REGISTERED_MATCHMAKING": "1", "FORTNITE_GAMESERVER_CAPACITY": "0"}),
            ]
            for name, extra in cases:
                with self.subTest(case=name):
                    env = self.base_env(bin_dir, work)
                    env.update(extra)
                    res = subprocess.run(
                        [str(self.porting / "run_original_client.sh"), "15"],
                        env=env,
                        capture_output=True,
                        text=True,
                    )
                    self.assertNotEqual(res.returncode, 0, f"{name} unexpectedly succeeded")

            # Unavailable listener: copy sandbox script and make probe_server.py exit immediately.
            porting2 = work / "bad_listener_porting"
            (porting2 / "tools").mkdir(parents=True)
            (porting2 / "server").mkdir(parents=True)
            (porting2 / "generated/tls").mkdir(parents=True)
            shutil.copy2(self.porting / "run_original_client.sh", porting2 / "run_original_client.sh")
            shutil.copy2(self.porting / "tools/client_actions.sh", porting2 / "tools/client_actions.sh")
            shutil.copy2(self.porting / "generated/hosts", porting2 / "generated/hosts")
            shutil.copy2(self.porting / "generated/tls/cert.pem", porting2 / "generated/tls/cert.pem")
            shutil.copy2(self.porting / "generated/tls/key.pem", porting2 / "generated/tls/key.pem")
            write_exec(porting2 / "server/probe_server.py", "#!/usr/bin/env python3\nimport sys\nsys.exit(1)\n")

            server_ran = work / "server_should_not_run"
            env = self.base_env(bin_dir, work)
            env.update(
                {
                    "FN_NETNS_INNER": "1",
                    "FN_USER": env["USER"],
                    "FN_USERNAME": "LocalPlayer",
                    "FN_REGISTERED_MATCHMAKING": "1",
                    "FN_BACKEND_PORT": str(free_port()),
                    "FN_SERVER_CMD": f"touch {server_ran}",
                }
            )
            res = subprocess.run(
                [str(porting2 / "run_original_client.sh"), "15"],
                env=env,
                capture_output=True,
                text=True,
            )
            self.assertNotEqual(res.returncode, 0)
            self.assertIn("failed to become ready", res.stderr)
            self.assertFalse(server_ran.exists())


    def test_08_term_cleans_listeners_secret_and_running_children(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            bin_dir, _, _ = self.make_shims(work)
            wine_pid = work / "wine.pid"
            native_pid = work / "native.pid"
            nested_pid = work / "nested.pid"
            graceful_marker = work / "graceful"
            # A wrapper that does not forward TERM models sudo/xvfb/timeout
            # chains whose child would otherwise outlive the runner.
            write_exec(bin_dir / "wine", f'#!/usr/bin/env bash\necho $$ > "{wine_pid}"\ntrap \'echo graceful > "{graceful_marker}"\' TERM\nbash -c \'trap "" TERM; exec sleep 30\' &\necho $! > "{nested_pid}"\nwait\n')
            unrelated = subprocess.Popen(["sleep", "30"])
            env = self.base_env(bin_dir, work)
            env.update(FN_NETNS_INNER="1", FN_USER=env["USER"], FN_USERNAME="LocalPlayer",
                       FN_BACKEND_PORT=str(free_port()), FN_REGISTERED_MATCHMAKING="1",
                       FN_SERVER_CMD=f'echo $$ > "{native_pid}"; exec sleep 30')
            output = work / "runner.out"
            with output.open("w") as log:
                proc = subprocess.Popen([str(self.porting / "run_original_client.sh"), "30"],
                                        env=env, stdout=log, stderr=log)
                try:
                    deadline = time.monotonic() + 10
                    while time.monotonic() < deadline and proc.poll() is None:
                        if wine_pid.exists() and native_pid.exists() and nested_pid.exists():
                            break
                        time.sleep(0.05)
                    self.assertTrue(wine_pid.exists(), output.read_text())
                    self.assertTrue(native_pid.exists(), output.read_text())
                    trace = re.search(r'loopback_port=(\d+) secret_file=(\S+)', output.read_text())
                    self.assertIsNotNone(trace)
                    secret_path = Path(trace.group(2))
                    self.assertTrue(secret_path.exists())
                    proc.terminate()
                    self.assertEqual(proc.wait(timeout=3), 143)
                    self.assertFalse(secret_path.exists())
                    self.assertTrue(graceful_marker.exists(), "Client wrapper had no grace period")
                    for port in (int(trace.group(1)), int(env["FN_BACKEND_PORT"])):
                        deadline = time.monotonic() + 3
                        while time.monotonic() < deadline:
                            with socket.socket() as sock:
                                if sock.connect_ex(("127.0.0.1", port)) != 0:
                                    break
                            time.sleep(0.05)
                        else:
                            self.fail("Owned backend listener survived TERM")
                    for pid_file in (wine_pid, native_pid, nested_pid):
                        pid = int(pid_file.read_text())
                        deadline = time.monotonic() + 3
                        while time.monotonic() < deadline:
                            try:
                                state = Path(f"/proc/{pid}/stat").read_text().rsplit(")", 1)[1].split()[0]
                            except FileNotFoundError:
                                break
                            if state == "Z":
                                break
                            time.sleep(0.05)
                        else:
                            self.fail("Owned client/server child survived TERM")
                    self.assertIsNone(unrelated.poll(), "Unrelated process was terminated")
                finally:
                    unrelated.terminate()
                    unrelated.wait(timeout=3)
                    if proc.poll() is None:
                        proc.kill()
                        proc.wait()


    def test_09_timeout_cleans_nested_client_child(self):
        with tempfile.TemporaryDirectory(dir=self.sandbox) as td:
            work = Path(td)
            bin_dir, _, _ = self.make_shims(work)
            nested_pid = work / "nested.pid"
            write_exec(bin_dir / "wine", f'#!/usr/bin/env bash\nsleep 30 &\necho $! > "{nested_pid}"\nwait\n')
            env = self.base_env(bin_dir, work)
            env.update(FN_NETNS_INNER="1", FN_USER=env["USER"], FN_USERNAME="LocalPlayer",
                       FN_BACKEND_PORT=str(free_port()))
            result = subprocess.run([str(self.porting / "run_original_client.sh"), "1"],
                                    env=env, capture_output=True, text=True, timeout=8)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertTrue(nested_pid.exists())
            pid = int(nested_pid.read_text())
            deadline = time.monotonic() + 3
            while time.monotonic() < deadline:
                try:
                    state = Path(f"/proc/{pid}/stat").read_text().rsplit(")", 1)[1].split()[0]
                except FileNotFoundError:
                    break
                if state == "Z":
                    break
                time.sleep(0.05)
            else:
                self.fail("Nested client child survived timeout")


if __name__ == "__main__":
    unittest.main()
