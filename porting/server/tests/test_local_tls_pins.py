"""Layout/transaction tests with bounded fake memory; no running-client access."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
from unittest import mock
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / 'patch_local_tls_pins.py'
SPEC = importlib.util.spec_from_file_location('local_tls_pins', SOURCE)
pins = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = pins
SPEC.loader.exec_module(pins)


class Memory:
    def __init__(self):
        self.base = 0x10000
        self.data = bytearray(0x20000)
        self.writes = []
        self.fail_write = None

    def read(self, address, size):
        offset = address-self.base
        if offset < 0 or size <= 0 or offset+size > len(self.data):
            raise pins.Rejected('fake memory bounds')
        return bytes(self.data[offset:offset+size])

    def write(self, address, data):
        self.writes.append((address, len(data)))
        if len(self.writes) == self.fail_write:
            self.data[address-self.base:address-self.base+7] = data[:7]
            raise pins.Rejected('simulated partial write')
        self.data[address-self.base:address-self.base+len(data)] = data

    def put(self, address, data):
        self.data[address-self.base:address-self.base+len(data)] = data


def fixture():
    memory = Memory()
    manager, entries = 0x10000, 0x11000
    memory.put(manager, struct.pack('<QQiiQii', pins.VTABLE, 0x12000, 123, 128, entries, 6, 8))
    for index, domain in enumerate(sorted(pins.DOMAINS)):
        name = (domain+'\0').encode('utf-16-le')
        name_address = 0x13000 + index*0x100
        digest_address = 0x14000 + index*0x100
        memory.put(entries+index*32, struct.pack('<QiiQii', name_address,
                   len(name)//2, len(name)//2, digest_address, 2, 2))
        memory.put(name_address, name)
        for item in range(2):
            memory.put(digest_address+item*40, bytes([index*2+item+1])*32 + struct.pack('<ii', 32, 32))
    return memory, manager


class LocalTlsPinsTests(unittest.TestCase):
    def test_replace_preserves_other_digests_and_container_metadata(self):
        memory, manager = fixture()
        original = bytes(memory.data)
        snapshot = pins.inspect_layout(memory, manager)
        digest = b'X'*32
        self.assertEqual(pins.replace_pins(memory, snapshot, digest), 6)
        permitted = set()
        for group in snapshot.pins:
            address = group[0][0]
            permitted.update(range(address-memory.base, address-memory.base+32))
            self.assertEqual(memory.read(address, 32), digest)
            self.assertEqual(memory.read(group[1][0], 32), group[1][1])
        self.assertTrue(all(a == b or i in permitted for i, (a, b) in enumerate(zip(original, memory.data))))
        self.assertEqual(pins.replace_pins(memory, pins.inspect_layout(memory, manager), digest), 0)
        self.assertEqual(len(memory.writes), 6)

    def test_existing_pin_in_second_slot_is_idempotent(self):
        memory, manager = fixture()
        snapshot = pins.inspect_layout(memory, manager)
        digest = b'Z'*32
        for group in snapshot.pins:
            memory.put(group[1][0], digest)
        self.assertEqual(pins.replace_pins(memory, pins.inspect_layout(memory, manager), digest), 0)
        self.assertEqual(memory.writes, [])

    def test_unexpected_domain_rejected_without_write(self):
        memory, manager = fixture()
        memory.put(0x13000, b'x\0')
        with self.assertRaises(pins.Rejected):
            pins.inspect_layout(memory, manager)
        self.assertEqual(memory.writes, [])

    def test_malformed_containers_and_inline_digest_fail_closed(self):
        for address, replacement in [(0x10020, struct.pack('<i', 7)),
                                     (0x11018, struct.pack('<ii', 3, 2)),
                                     (0x14020, struct.pack('<ii', 31, 32)),
                                     (0x11000, struct.pack('<Q', 0xFFFF0000)),
                                     (0x110B0, struct.pack('<Q', 0x14000))]:
            with self.subTest(address=address):
                memory, manager = fixture()
                memory.put(address, replacement)
                with self.assertRaises(pins.Rejected):
                    pins.inspect_layout(memory, manager)
                self.assertEqual(memory.writes, [])

    def test_partial_write_rolls_back_every_attempt(self):
        memory, manager = fixture()
        original = bytes(memory.data)
        snapshot = pins.inspect_layout(memory, manager)
        memory.fail_write = 3
        with self.assertRaises(pins.Rejected):
            pins.replace_pins(memory, snapshot, b'X'*32)
        self.assertEqual(bytes(memory.data), original)
        self.assertEqual(len(memory.writes), 6)

    def test_failed_readback_rolls_back(self):
        memory, manager = fixture()
        original = bytes(memory.data)
        snapshot = pins.inspect_layout(memory, manager)
        write = memory.write
        def corrupt_once(address, data):
            write(address, data)
            if len(memory.writes) == 1:
                memory.put(address, b'?'*32)
        memory.write = corrupt_once
        with self.assertRaises(pins.Rejected):
            pins.replace_pins(memory, snapshot, b'X'*32)
        self.assertEqual(bytes(memory.data), original)

    def test_private_key_input_rejected_before_openssl(self):
        with tempfile.NamedTemporaryFile() as stream:
            stream.write(b'-----BEGIN PRIVATE KEY-----\nignored\n-----END PRIVATE KEY-----\n')
            stream.flush()
            with mock.patch.object(pins.subprocess, 'run') as run:
                with self.assertRaises(pins.Rejected):
                    pins.public_ca_digest(stream.name)
                run.assert_not_called()

    def test_changed_layout_rejected_before_write(self):
        memory, manager = fixture()
        snapshot = pins.inspect_layout(memory, manager)
        memory.put(snapshot.pins[0][0][0], b'Y'*32)
        with self.assertRaises(pins.Rejected):
            pins.replace_pins(memory, snapshot, b'X'*32)
        self.assertEqual(memory.writes, [])

    def test_rollback_failure_is_distinct(self):
        memory, manager = fixture()
        snapshot = pins.inspect_layout(memory, manager)
        def fail(address, data):
            raise OSError('simulated write failure')
        memory.write = fail
        with self.assertRaises(pins.RollbackFailed):
            pins.replace_pins(memory, snapshot, b'X'*32)


if __name__ == '__main__':
    unittest.main()
