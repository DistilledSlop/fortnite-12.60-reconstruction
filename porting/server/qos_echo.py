#!/usr/bin/env python3
"""Loopback-only UE UDP QoS echo; no latency is invented by the server.

The request layout is 14 opaque bytes followed by the eight-byte UE magic.
The original 12.60 client's wire format has not yet been verified.
"""

import argparse
import ipaddress
import socket
import time

MAGIC = b"\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb"
PACKET_SIZE = 22


def hotfix(port):
    """Return the narrow candidate QoS hotfix for the bound local port."""
    return ("[/Script/Qos.QosRegionManager]\n"
            "!DatacenterDefinitions=ClearArray\n"
            f'+DatacenterDefinitions=(Id="LOCAL",RegionId="NAE",bEnabled=true,Servers[0]=(Address="127.0.0.1",Port={port}))\n').encode("ascii")


def valid_request(data, address):
    """Accept only complete UE echo requests from an IPv4 loopback address."""
    try:
        source = ipaddress.ip_address(address[0])
    except (ValueError, IndexError, TypeError):
        return False
    return (source.version == 4 and source.is_loopback and
            len(data) == PACKET_SIZE and data[-len(MAGIC):] == MAGIC)


class TokenBucket:
    """Bound responses globally, including when multiple local clients send."""

    def __init__(self, rate=100.0, burst=100, clock=time.monotonic):
        if rate <= 0 or burst < 1:
            raise ValueError("rate and burst must be positive")
        self.rate, self.burst, self.clock = rate, burst, clock
        self.tokens = float(burst)
        self.last = clock()

    def take(self):
        now = self.clock()
        self.tokens = min(self.burst, self.tokens + max(0, now - self.last) * self.rate)
        self.last = now
        if self.tokens < 1:
            return False
        self.tokens -= 1
        return True


def serve(sock, stop_event=None, rate=100.0, burst=100):
    """Serve an already bound IPv4 loopback socket until stopped or interrupted."""
    if sock.family != socket.AF_INET or sock.getsockname()[0] != "127.0.0.1":
        raise ValueError("socket must be bound to 127.0.0.1")
    limiter = TokenBucket(rate, burst)
    sock.settimeout(0.2)
    while stop_event is None or not stop_event.is_set():
        try:
            # Read one extra byte so oversized datagrams cannot be accepted as 22 bytes.
            data, address = sock.recvfrom(PACKET_SIZE + 1)
        except socket.timeout:
            continue
        if valid_request(data, address) and limiter.take():
            sock.sendto(data, address)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=22222, help="UDP port on 127.0.0.1 (default: 22222)")
    args = parser.parse_args()
    if not 1 <= args.port <= 65535:
        parser.error("port must be between 1 and 65535")
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(("127.0.0.1", args.port))
        print(f"QoS echo listening on 127.0.0.1:{args.port}", flush=True)
        try:
            serve(sock)
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
