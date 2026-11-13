import json
import os
from pathlib import Path
import re
import select
import signal
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request
import struct
import threading
import unittest

from porting.server import qos_echo


class QosEchoTests(unittest.TestCase):
    def test_loopback_echo_preserves_both_endian_payloads_and_drops_invalid(self):
        stop = threading.Event()
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server:
            server.bind(("127.0.0.1", 0))
            thread = threading.Thread(target=qos_echo.serve, args=(server, stop))
            thread.start()
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
                    client.bind(("127.0.0.1", 0))
                    client.settimeout(0.15)
                    target = server.getsockname()
                    # Network ID/sequence plus checksum and timestamp in each convention.
                    prefixes = (
                        struct.pack("!HHHQ", 7, 9, 0x1234, 0x0102030405060708),
                        struct.pack("!HH", 7, 9) + struct.pack("<HQ", 0x1234, 0x0102030405060708),
                    )
                    for prefix in prefixes:
                        packet = prefix + qos_echo.MAGIC
                        client.sendto(packet, target)
                        reply, sender = client.recvfrom(100)
                        self.assertEqual((reply, sender), (packet, target))
                    for packet in (b"", qos_echo.MAGIC, b"x" * 14 + qos_echo.MAGIC + b"x",
                                   b"x" * 14 + b"badmagic"):
                        client.sendto(packet, target)
                        with self.assertRaises(socket.timeout):
                            client.recvfrom(100)
            finally:
                stop.set()
                thread.join(timeout=2)
                self.assertFalse(thread.is_alive())

    def test_source_filter_and_bind_guard(self):
        packet = b"x" * 14 + qos_echo.MAGIC
        self.assertTrue(qos_echo.valid_request(packet, ("127.0.0.2", 12)))
        self.assertFalse(qos_echo.valid_request(packet, ("192.0.2.1", 12)))
        self.assertFalse(qos_echo.valid_request(packet, ("::1", 12)))
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(("0.0.0.0", 0))
            with self.assertRaises(ValueError):
                qos_echo.serve(sock)

    def test_global_rate_limit_refills(self):
        now = [0.0]
        bucket = qos_echo.TokenBucket(rate=2, burst=2, clock=lambda: now[0])
        self.assertTrue(bucket.take())
        self.assertTrue(bucket.take())
        self.assertFalse(bucket.take())
        now[0] += 0.5
        self.assertTrue(bucket.take())
        self.assertFalse(bucket.take())


class BackendQosTests(unittest.TestCase):
    script = Path(__file__).resolve().parents[1] / "probe_server.py"

    def start_backend(self, *args):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        process = subprocess.Popen([sys.executable, str(self.script), "--stub", "--ephemeral",
                                    "--port", "0", "--record", str(Path(temp.name) / "requests"), *args],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.addCleanup(self.stop_backend, process)
        return process

    @staticmethod
    def stop_backend(process):
        if process.poll() is None:
            process.send_signal(signal.SIGINT)
            try:
                process.communicate(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.communicate()
        else:
            process.communicate()

    def read_ready(self, process):
        output = b""
        deadline = time.monotonic() + 10
        while b"Listening on " not in output:
            ready, _, _ = select.select([process.stdout], [], [], max(0, deadline - time.monotonic()))
            if not ready:
                self.fail("backend did not become ready")
            chunk = os.read(process.stdout.fileno(), 4096)
            if not chunk:
                self.fail("backend exited before ready: " + process.stderr.read().decode())
            output += chunk
        return output.decode()

    def test_opt_in_http_echo_and_shutdown(self):
        with tempfile.TemporaryDirectory() as cloud:
            Path(cloud, "Other.ini").write_text("other")
            process = self.start_backend("--local-qos-port", "0", "--cloudstorage-dir", cloud)
            output = self.read_ready(process)
            qos_port = int(re.search(r"Local QoS echo on 127\.0\.0\.1:(\d+)", output).group(1))
            http_port = int(re.search(r"Listening on 127\.0\.0\.1:(\d+)", output).group(1))
            base = f"http://127.0.0.1:{http_port}/fortnite/api/cloudstorage/system"
            with urllib.request.urlopen(base, timeout=3) as response:
                listing = json.load(response)
            self.assertEqual([entry["filename"] for entry in listing], ["DefaultEngine.ini", "Other.ini"])
            with urllib.request.urlopen(base + "/DefaultEngine.ini", timeout=3) as response:
                data = response.read()
            self.assertEqual(data, qos_echo.hotfix(qos_port))
            self.assertEqual(listing[0]["length"], len(data))
            with urllib.request.urlopen(base + "/Other.ini", timeout=3) as response:
                self.assertEqual(response.read(), b"other")
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
                client.settimeout(2)
                packet = bytes(range(14)) + qos_echo.MAGIC
                client.sendto(packet, ("127.0.0.1", qos_port))
                self.assertEqual(client.recvfrom(100)[0], packet)
            process.send_signal(signal.SIGINT)
            process.communicate(timeout=5)
            self.assertEqual(process.returncode, 0)
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as check:
                check.bind(("127.0.0.1", qos_port))

    def test_conflicts_fail_visibly(self):
        with tempfile.TemporaryDirectory() as cloud:
            Path(cloud, "defaultengine.ini").write_text("existing")
            process = self.start_backend("--local-qos-port", "0", "--cloudstorage-dir", cloud)
            _, stderr = process.communicate(timeout=10)
            self.assertNotEqual(process.returncode, 0)
            self.assertIn(b"conflicts with built-in DefaultEngine.ini", stderr)
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as occupied:
            occupied.bind(("127.0.0.1", 0))
            process = self.start_backend("--local-qos-port", str(occupied.getsockname()[1]))
            _, stderr = process.communicate(timeout=10)
            self.assertNotEqual(process.returncode, 0)
            self.assertIn(b"cannot enable local QoS", stderr)

    def test_default_listing_remains_empty(self):
        process = self.start_backend()
        output = self.read_ready(process)
        port = int(re.search(r"Listening on 127\.0\.0\.1:(\d+)", output).group(1))
        with urllib.request.urlopen(f"http://127.0.0.1:{port}/fortnite/api/cloudstorage/system", timeout=3) as response:
            self.assertEqual(json.load(response), [])


if __name__ == "__main__":
    unittest.main()
