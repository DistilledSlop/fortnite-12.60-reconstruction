import tempfile
import unittest
from pathlib import Path

from client_process import shipping_client_pids


class ClientProcessTests(unittest.TestCase):
    def test_finds_direct_and_launcher_paths_only_in_our_namespace(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'self/ns').mkdir(parents=True)
            (root / 'self/ns/net').symlink_to('net:[42]')
            for pid, argv0, namespace in [
                (10, b'FortniteClient-Win64-Shipping.exe', 'net:[42]'),
                (11, b'Z:\\game\\FortniteClient-Win64-Shipping.exe', 'net:[42]'),
                (12, b'FortniteClient-Win64-Shipping_BE.exe', 'net:[42]'),
                (13, b'FortniteClient-Win64-Shipping.exe', 'net:[99]'),
            ]:
                process = root / str(pid)
                (process / 'ns').mkdir(parents=True)
                (process / 'ns/net').symlink_to(namespace)
                (process / 'cmdline').write_bytes(argv0 + b'\0-arg\0')
            self.assertEqual(shipping_client_pids(root), [10, 11])


if __name__ == '__main__':
    unittest.main()
