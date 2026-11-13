#!/usr/bin/env python3
"""Explicit patched local mode: replace six existing pins with a public CA SPKI.

TLS certificate/hostname verification stays enabled. This is a runtime data
patch, not unmodified-client G11. Linux/Wine private-network experiment only.
Layout proof: original digest verifier 1438bda50 reads manager+18/+20,
32-byte domain entries, and 40-byte inline digests (32 data + int Num/Max).
Original X509 verifier 1438bdc10 calls slot38 at1438bddb0 unconditionally.
"""
import argparse
from contextlib import contextmanager
from dataclasses import dataclass
import hashlib
import math
import os
from pathlib import Path
import re
import signal
import struct
import subprocess
import sys
import tempfile
import time

DOMAINS = frozenset(('.ol.epicgames.net', '.ol.epicgames.com',
                     '.on.epicgames.net', '.on.epicgames.com',
                     '.ak.epicgames.com', '.fortnite.qq.com'))
VTABLE = 0x146761E00
BASE = 0x140000000
EXE = 'fortniteclient-win64-shipping.exe'
SIGNATURES = {
    0x1438BDA50: bytes.fromhex('e9377f7905'),
    0x1438BDC10: bytes.fromhex('e9ca3dbb05'),
    0x1438BDDB0: bytes.fromhex('ff503890e9cb2a0706'),
    0x144866E28: bytes.fromhex('e883a340fe4533f644387424607554448be6'),
}


class Rejected(RuntimeError):
    """A guard rejected the target without exposing process contents."""


class RollbackFailed(Rejected):
    pass


class NotReady(Rejected):
    """The identified image has not exposed its original runtime TLS code yet."""


@dataclass(frozen=True)
class Snapshot:
    manager: int
    guards: tuple
    pins: tuple


def array_header(memory, address, limit):
    raw = memory.read(address, 16)
    pointer, count, capacity = struct.unpack('<Qii', raw)
    if not (pointer >= 0x10000 and pointer % 8 == 0 and
            0 < count <= capacity <= limit):
        raise Rejected('invalid array layout')
    return raw, pointer, count, capacity


def inspect_layout(memory, manager):
    """Validate all containers before permitting any write; adapter bounds reads."""
    guards = []

    def remember(address, data):
        guards.append((address, data))
        return data

    header = remember(manager, memory.read(manager, 40))
    if struct.unpack_from('<Q', header)[0] != VTABLE:
        raise Rejected('manager signature mismatch')
    _, roots, root_count, root_max = array_header(memory, manager + 8, 8192)
    memory.read(roots, root_max * 8)
    _, entries, count, capacity = array_header(memory, manager + 24, 128)
    if count != len(DOMAINS):
        raise Rejected('expected six pinned domains')
    memory.read(entries, capacity * 32)
    domains = set()
    pins = []
    digest_ranges = []
    metadata_ranges = [(manager, manager + 40), (roots, roots + root_max * 8),
                       (entries, entries + capacity * 32)]
    for index in range(count):
        entry = entries + index * 32
        raw = remember(entry, memory.read(entry, 32))
        name_ptr, name_count, name_max = struct.unpack_from('<Qii', raw)
        if not (name_ptr >= 0x10000 and name_ptr % 2 == 0 and
                2 <= name_count <= name_max <= 256):
            raise Rejected('invalid domain string layout')
        memory.read(name_ptr, name_max * 2)
        metadata_ranges.append((name_ptr, name_ptr + name_max * 2))
        encoded = remember(name_ptr, memory.read(name_ptr, name_count * 2))
        try:
            domain = encoded[:-2].decode('utf-16-le')
        except UnicodeError:
            raise Rejected('invalid domain encoding') from None
        if encoded[-2:] != b'\0\0' or domain not in DOMAINS or domain in domains:
            raise Rejected('unexpected or duplicate pinned domain')
        domains.add(domain)
        _, digests, number, maximum = array_header(memory, entry + 16, 64)
        memory.read(digests, maximum * 40)
        digest_ranges.append((digests, digests + maximum * 40))
        domain_pins = []
        for item in range(number):
            address = digests + item * 40
            data = remember(address, memory.read(address, 40))
            if struct.unpack_from('<ii', data, 32) != (32, 32):
                raise Rejected('invalid inline digest length or capacity')
            domain_pins.append((address, data[:32]))
        pins.append(tuple(domain_pins))
    if domains != DOMAINS:
        raise Rejected('pinned domain set mismatch')
    # Disjoint ownership prevents writes to aliased digests or metadata.
    for i, (lo, hi) in enumerate(digest_ranges):
        for lo2, hi2 in digest_ranges[i + 1:]:
            if lo < hi2 and lo2 < hi:
                raise Rejected('aliased digest arrays')
        for address, end in metadata_ranges:
            if lo < end and address < hi:
                raise Rejected('digest array overlaps metadata')
    return Snapshot(manager, tuple(guards), tuple(pins))


def replace_pins(memory, snapshot, digest):
    """Transactional, idempotent mutation of existing data; caller stops process."""
    if len(digest) != 32:
        raise Rejected('expected SHA256 digest')
    if inspect_layout(memory, snapshot.manager) != snapshot:
        raise Rejected('layout changed before update')
    changes = [(pins[0][0], pins[0][1]) for pins in snapshot.pins
               if not any(old == digest for _, old in pins)]
    attempted = []
    try:
        for address, old in changes:
            attempted.append((address, old))
            memory.write(address, digest)
            if memory.read(address, 32) != digest:
                raise Rejected('pin readback failed')
        expected = {(address): digest for address, _ in changes}
        for address, old in snapshot.guards:
            want = expected[address] + old[32:] if address in expected else old
            if memory.read(address, len(old)) != want:
                raise Rejected('post-update layout changed')
    except BaseException:
        failed = False
        for address, old in reversed(attempted):
            try:
                memory.write(address, old)
                if memory.read(address, 32) != old:
                    failed = True
            except BaseException:
                failed = True
        if failed:
            raise RollbackFailed('rollback failed; target retained stopped') from None
        raise
    return len(changes)


def public_ca_digest(path):
    """Accept one PEM public CA certificate; never pass keys to OpenSSL."""
    with open(path, 'rb') as stream:
        pem = stream.read(262145)
    if len(pem) > 262144 or not re.fullmatch(
            rb'\s*-----BEGIN CERTIFICATE-----\s+[A-Za-z0-9+/=\r\n]+-----END CERTIFICATE-----\s*', pem):
        raise Rejected('expected one public PEM certificate only')

    def openssl(arguments, data):
        result = subprocess.run(['openssl', *arguments], input=data,
                                capture_output=True, timeout=10, check=False)
        if result.returncode:
            raise Rejected('public CA certificate validation failed')
        return result.stdout

    constraints = openssl(['x509', '-noout', '-ext', 'basicConstraints'], pem)
    if b'CA:TRUE' not in constraints:
        raise Rejected('certificate must be a CA')
    # Verify certificate dates and self-signature using only this public root.
    with tempfile.NamedTemporaryFile(prefix='local-public-ca-', suffix='.pem') as public_file:
        public_file.write(pem)
        public_file.flush()
        verified = subprocess.run(['openssl', 'verify', '-check_ss_sig', '-CAfile', public_file.name,
                                   public_file.name], capture_output=True, timeout=10, check=False)
    if verified.returncode:
        raise Rejected('CA certificate trust or validity check failed')
    public = openssl(['x509', '-pubkey', '-noout'], pem)
    der = openssl(['pkey', '-pubin', '-outform', 'DER'], public)
    return hashlib.sha256(der).digest()


class LinuxMemory:
    def __init__(self, pid):
        self.proc = Path('/proc') / str(pid)
        self.fd = os.open(self.proc / 'mem', os.O_RDWR)
        self.regions = []
        for line in (self.proc / 'maps').read_text().splitlines():
            fields = line.split(maxsplit=5)
            lo, hi = (int(x, 16) for x in fields[0].split('-'))
            self.regions.append((lo, hi, fields[1], fields[5] if len(fields) > 5 else ''))

    def close(self):
        os.close(self.fd)

    def read(self, address, size, writable=True):
        if size <= 0 or not any(lo <= address and address + size <= hi and
                permissions.startswith('rw' if writable else 'r') and
                (not writable or permissions.endswith('p'))
                for lo, hi, permissions, _ in self.regions):
            raise Rejected('memory mapping bounds rejected')
        data = os.pread(self.fd, size, address)
        if len(data) != size:
            raise Rejected('short memory read')
        return data

    def write(self, address, data):
        self.read(address, len(data))
        if os.pwrite(self.fd, data, address) != len(data):
            raise Rejected('short memory write')

    def find_managers(self):
        needle = struct.pack('<Q', VTABLE)
        found = set()
        scanned = 0
        for lo, hi, permissions, _ in self.regions:
            if not permissions.startswith('rw') or not permissions.endswith('p') or hi-lo > 256*1024**2:
                continue
            for address in range(max(lo, 0x10000), hi, 1024**2):
                size = min(1024**2 + 40, hi - address)
                scanned += size
                if scanned > 4*1024**3:
                    raise Rejected('manager search exceeded bounded scan')
                try:
                    data = self.read(address, size)
                except (OSError, Rejected):
                    # Wine may reserve inaccessible pages inside a mapping.
                    # An unreadable search chunk is not a validated candidate.
                    continue
                offset = data.find(needle)
                while offset >= 0:
                    candidate = address + offset
                    if candidate % 8 == 0:
                        try:
                            inspect_layout(self, candidate)
                            found.add(candidate)
                        except (Rejected, OSError):
                            pass
                    offset = data.find(needle, offset + 8)
        return found

    def validate_image(self):
        if not any(lo == BASE and path.lower().endswith('/' + EXE)
                   for lo, _, _, path in self.regions):
            raise Rejected('expected shipping image mapping missing')
        header = self.read(BASE, 4096, False)
        pe = struct.unpack_from('<I', header, 60)[0]
        if header[:2] != b'MZ' or not 64 <= pe <= 3500:
            raise Rejected('image header mismatch')
        if (header[pe:pe+4] != b'PE\0\0' or
            struct.unpack_from('<H', header, pe+4)[0] != 0x8664 or
            struct.unpack_from('<I', header, pe+8)[0] != 0x5EBE1934 or
            struct.unpack_from('<Q', header, pe+48)[0] != BASE or
            struct.unpack_from('<I', header, pe+80)[0] != 0x9E4D600):
            raise Rejected('expected 12.60 CL13477524 image version')
        for address, expected in SIGNATURES.items():
            if self.read(address, len(expected), False) != expected:
                raise NotReady('original TLS image signatures not ready')
        if self.read(VTABLE+0x38, 16, False) != struct.pack('<QQ', 0x1438BDA50, 0x1438BDC10):
            raise Rejected('SSL manager vtable mismatch')
        if self.read(0x147C69478, 1, False) != b'\x01':
            raise Rejected('Curl peer verification is not enabled')


def identity(proc):
    stat = (proc / 'stat').read_text().rsplit(')', 1)[1].split()
    return stat[19]  # field22 starttime, with field3 at index0


def validate_process(proc, original):
    if identity(proc) != original:
        raise Rejected('process identity changed')
    namespace = os.readlink('/proc/self/ns/net')
    if namespace == os.readlink('/proc/1/ns/net') or namespace != os.readlink(proc / 'ns/net'):
        raise Rejected('requires target in same private network namespace')
    argv0 = (proc/'cmdline').read_bytes().split(b'\0', 1)[0]
    if argv0.replace(b'\\', b'/').rsplit(b'/', 1)[-1].lower() != EXE.encode():
        raise Rejected('expected exact shipping process')
    status = (proc/'status').read_text()
    if re.search(r'^TracerPid:\s*(?!0\s*$)\d+', status, re.M):
        raise Rejected('detach debugger before updating pins')


@contextmanager
def stopped_process(proc, pidfd, original):
    validate_process(proc, original)
    already = re.search(r'^State:\s+T', (proc/'status').read_text(), re.M) is not None
    retain = False
    try:
        signal.pidfd_send_signal(pidfd, signal.SIGSTOP)
        deadline = time.monotonic() + 3
        while True:
            states = [(task/'status').read_text() for task in (proc/'task').iterdir()]
            if states and all(re.search(r'^State:\s+T', state, re.M) for state in states):
                break
            if time.monotonic() >= deadline:
                raise Rejected('could not stop every target thread')
            time.sleep(0.01)
        validate_process(proc, original)
        yield
    except RollbackFailed:
        retain = True
        raise
    finally:
        if not already and not retain:
            signal.pidfd_send_signal(pidfd, signal.SIGCONT)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pid', required=True, type=int)
    parser.add_argument('--ca-cert', required=True, type=Path)
    parser.add_argument('--timeout', required=True, type=float)
    args = parser.parse_args(argv)
    if args.pid <= 1 or not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error('pid must exceed 1 and timeout must be finite and positive')
    try:
        digest = public_ca_digest(args.ca_cert)
        proc = Path('/proc') / str(args.pid)
        pidfd = os.pidfd_open(args.pid)
        try:
            original = identity(proc)
            deadline = time.monotonic() + args.timeout
            while True:
                validate_process(proc, original)
                memory = LinuxMemory(args.pid)
                try:
                    try:
                        memory.validate_image()
                    except NotReady:
                        managers = set()
                    else:
                        managers = memory.find_managers()
                finally:
                    memory.close()
                if len(managers) > 1:
                    raise Rejected('multiple validated SSL managers')
                if managers:
                    with stopped_process(proc, pidfd, original):
                        memory = LinuxMemory(args.pid)
                        try:
                            memory.validate_image()
                            snapshot = inspect_layout(memory, next(iter(managers)))
                            changed = replace_pins(memory, snapshot, digest)
                        finally:
                            memory.close()
                    print(f'patched local TLS mode: domains=6 changed={changed} verification=enabled')
                    return 0
                if time.monotonic() >= deadline:
                    raise Rejected('SSL manager not ready before timeout')
                time.sleep(min(0.5, max(0, deadline-time.monotonic())))
        finally:
            os.close(pidfd)
    except RollbackFailed:
        print('local TLS update rollback failed; target retained stopped', file=sys.stderr)
        return 1
    except Rejected as error:
        print(f'local TLS pin update rejected: {error}', file=sys.stderr)
        return 1
    except (OSError, ValueError, subprocess.SubprocessError):
        # No raw exception strings: filenames, addresses or command output can
        # contain private information. Rollback failure has a fixed safe message.
        print('local TLS pin update rejected or failed; no success claimed', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
