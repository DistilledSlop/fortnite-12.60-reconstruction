#!/usr/bin/env python3
"""Read the running original client's player-controller state (diagnostic, memory read only).

Background: porting/G6_CONTROLLER_PROBE_FIX.md. The client's per-tick driver 0x1423bc750 (vtable slot +0x1570 in
every player-controller table) has two gates before the readiness check 0x1423cc220 runs:
  gate 1: [this+0x220] non-null (probably the ULocalPlayer), gate 2: [this+0x658] & 0x20 clear.
Then +0x14e8 = [this+0x6f8] && [this+0x6f9]; OnReadyToStartMatch (slot +0x1590) runs when it is true.
A player-controller object's first qword is a vtable in .rdata whose slot +0x1570 holds the driver.

Usage:
  pc_probe.py --selftest        no client needed: checks the table list against the runtime image
  pc_probe.py [pid]             scan the running client (needs root: /proc/<pid>/mem)
"""
import re
import struct
import sys
from pathlib import Path

RDATA_LO, RDATA_HI = 0x1458FC000, 0x147811C00
TEXT_LO, TEXT_HI = 0x140001000, 0x1458FBC00
DRIVER, DRIVER_SLOT = 0x1423BC750, 0x1570
IMAGE = Path(__file__).resolve().parents[1] / 'generated/dump/FortniteClient-Win64-Shipping.runtime.exe'
CHUNK = 64 << 20
# Table starts from G6_CONTROLLER_PROBE_FIX.md (class names are not proven).
KNOWN_TABLES = [0x145E1AAC0, 0x14613F330, 0x1461A8248, 0x1461AA718, 0x1461AE3A8, 0x1461BA7F0, 0x14613D310,
                0x14613B108, 0x146128208, 0x1461E7520, 0x145E3D2F8, 0x14612A270, 0x14612C5E8, 0x146141E40,
                0x146150078, 0x14616A200]


def find_pid():
    """The Wine process whose command line names the shipping exe (not a shell or sudo wrapper)."""
    for entry in sorted(Path('/proc').iterdir(), key=lambda e: e.name):
        if entry.name.isdigit():
            try:
                cmd = (entry / 'cmdline').read_bytes().replace(b'\0', b' ').lower()
                comm = (entry / 'comm').read_text().strip().lower()
            except OSError:
                continue
            if b'fortniteclient-win64-shipping.exe' in cmd and comm.startswith('fortniteclient'):
                return int(entry.name)
    return None


def load_image():
    import numpy as np
    buf = np.fromfile(IMAGE, dtype=np.uint8)
    pe = struct.unpack_from('<I', buf, 0x3C)[0]
    base = struct.unpack_from('<Q', buf, pe + 24 + 24)[0]
    return buf, base


def lea_targets(buf, base):
    """Every RIP-relative lea target in .text that lands 8-aligned in .rdata (file offset == RVA in the dump)."""
    import numpy as np
    text = buf[TEXT_LO - base:TEXT_HI - base]
    n = len(text) - 7
    c = ((text[:n] == 0x48) | (text[:n] == 0x4C)) & (text[1:n + 1] == 0x8D) & ((text[2:n + 2] & 0xC7) == 0x05)
    idx = np.nonzero(c)[0].astype(np.int64)
    d = (text[idx + 3].astype(np.int64) | text[idx + 4].astype(np.int64) << 8 |
         text[idx + 5].astype(np.int64) << 16 | text[idx + 6].astype(np.int64) << 24)
    d = np.where(d >= 1 << 31, d - (1 << 32), d)
    tg = TEXT_LO + idx + 7 + d
    tg = tg[(tg >= RDATA_LO) & (tg < RDATA_HI) & (tg % 8 == 0)]
    return set(int(x) for x in np.unique(tg))


def selftest():
    buf, base = load_image()
    targets = lea_targets(buf, base)
    bad = 0
    for t in KNOWN_TABLES:
        slot = struct.unpack_from('<Q', buf, t - base + DRIVER_SLOT)[0]
        ok = t in targets and slot == DRIVER
        bad += not ok
        print(f'{t:#x} lea-target={t in targets} [+{DRIVER_SLOT:#x}]={slot:#x} {"ok" if ok else "FAIL"}')
    # Every .rdata address whose slot +0x1570 is the driver and that is a lea target should be listed.
    found = []
    for t in sorted(targets):
        if struct.unpack_from('<Q', buf, t - base + DRIVER_SLOT)[0] == DRIVER:
            found.append(t)
    extra = [t for t in found if t not in KNOWN_TABLES]
    print(f'{len(found)} lea-anchored tables hold the driver at +{DRIVER_SLOT:#x}; not in KNOWN_TABLES: '
          f'{[hex(t) for t in extra]}')
    if bad:
        print(f'SELFTEST FAILED: {bad} table(s)')
        return 1
    print('SELFTEST OK')
    return 0


def u(mem, addr, fmt, n):
    try:
        mem.seek(addr)
        d = mem.read(n)
        return struct.unpack(fmt, d)[0] if len(d) == n else None
    except (OSError, ValueError, OverflowError, struct.error):
        return None


def is_a(mem, obj, cls_ptr):
    """UObject::IsA via the class inheritance table: [cls+0x38] depth, [cls+0x30] table of 8-byte entries."""
    cls = u(mem, obj + 0x10, '<Q', 8)
    if not cls or not cls_ptr:
        return None
    depth_t, depth_c = u(mem, cls_ptr + 0x38, '<i', 4), u(mem, cls + 0x38, '<i', 4)
    table = u(mem, cls + 0x30, '<Q', 8)
    if None in (depth_t, depth_c, table) or depth_t > depth_c:
        return False
    return u(mem, table + depth_t * 8, '<Q', 8) == cls_ptr + 0x30


def path_eval(mem, pc, playerstate):
    """Evaluate the inputs of the client path 0x1423bc750 -> 0x1423cc220 -> 0x1423af810 (see G6 notes)."""
    fortps = u(mem, 0x1481B6550, '<Q', 8)
    print(f'   gate1 class test: PlayerState IsA FortPlayerState = {is_a(mem, playerstate, fortps)}')
    b28c, b54c = u(mem, pc + 0x28C, '<B', 1), u(mem, pc + 0x54C, '<B', 1)
    print(f'   SetHasFinishedLoading needs [+0x28c]&2 (is {b28c:#x}) and IsLocalController (cache [+0x54c]={b54c})')
    # Pawn customization (0x1423593a0) logs "Hero:None": AFortPlayerPawn::Hero (+0x1818, transient). Read the chain.
    pawn = u(mem, pc + 0x248, '<Q', 8) or 0
    pps = u(mem, pawn + 0x238, '<Q', 8) if pawn else None
    print(f'   Pawn={pawn:#x} pawn.PlayerState={pps or 0:#x} (controller PS {playerstate:#x}) '
          f'pawn.Hero[+0x1818]={(u(mem, pawn + 0x1818, "<Q", 8) or 0) if pawn else 0:#x} '
          f'PS.HeroType[+0x370]={u(mem, playerstate + 0x370, "<Q", 8) or 0:#x}')
    outer = u(mem, pc + 0x20, '<Q', 8)
    print(f'   outer={outer or 0:#x}')
    if not outer:
        return
    for o in range(0, 0x400, 8):
        w = u(mem, outer + o, '<Q', 8)
        if not w or w < 0x10000 or w == outer or u(mem, w + 0x30, '<Q', 8) != outer:
            continue
        gs = u(mem, w + 0x130, '<Q', 8)
        print(f'   UWorld candidate {w:#x} (level+{o:#x}) GameState={gs or 0:#x}')
        if not gs:
            continue
        wm = u(mem, gs + 0x3D0, '<Q', 8)
        print(f'   GameState+0x3d0 WorldManager={wm or 0:#x}')
        if wm:
            print(f'   WorldManager +0x860={u(mem, wm + 0x860, "<i", 4)} +0x870={u(mem, wm + 0x870, "<i", 4)} '
                  f'(foundations gate passes only if equal and every entry is replicated)')
        nm = u(mem, gs + 0x3D0, '<Q', 8)
    # Globals read by the loading-counter gate FUN_143a9f860.
    obj = u(mem, 0x14830D600, '<Q', 8)
    if obj:
        print(f'   loading counters [+0xe8]={u(mem, obj + 0xE8, "<q", 8)} [+0xf0]={u(mem, obj + 0xF0, "<q", 8)} '
              f'[+0x1f0]={u(mem, obj + 0x1F0, "<q", 8)}')


def main():
    if '--selftest' in sys.argv[1:]:
        return selftest()
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    pid = int(args[0]) if args else find_pid()
    if not pid:
        print('no client process')
        return 1
    import numpy as np
    mem = open(f'/proc/{pid}/mem', 'rb', buffering=0)
    vt_cache = {}

    def is_pc_vtable(v):
        if v not in vt_cache:
            vt_cache[v] = u(mem, v + DRIVER_SLOT, '<Q', 8) == DRIVER
        return vt_cache[v]

    hits, unread, skipped = [], 0, []
    for line in Path(f'/proc/{pid}/maps').read_text().splitlines():
        m = re.match(r'([0-9a-f]+)-([0-9a-f]+) (r)(w)', line)
        if not m:
            continue
        lo, hi = int(m.group(1), 16), int(m.group(2), 16)
        if lo > 0x7FFFFFFFFFFF:
            continue
        for start in range(lo, hi, CHUNK):
            size = min(CHUNK, hi - start)
            try:
                mem.seek(start)
                data = mem.read(size)
            except (OSError, ValueError, OverflowError):
                unread += size
                continue
            unread += size - len(data)
            arr = np.frombuffer(data[: len(data) // 8 * 8], dtype='<u8')
            idx = np.nonzero((arr >= RDATA_LO) & (arr < RDATA_HI))[0]
            if idx.size == 0:
                continue
            vals = arr[idx]
            for v in np.unique(vals):
                if is_pc_vtable(int(v)):
                    for k in idx[vals == v]:
                        hits.append((start + int(k) * 8, int(v)))
    print(f'unread bytes: {unread}')

    for addr, vt in hits:
        role = u(mem, addr + 0xF0, '<B', 1)
        p220 = u(mem, addr + 0x220, '<Q', 8)
        b658 = u(mem, addr + 0x658, '<B', 1)
        b6f8 = u(mem, addr + 0x6F8, '<B', 1)
        b6f9 = u(mem, addr + 0x6F9, '<B', 1)
        qb = u(mem, addr + 0x14D0, '<Q', 8)
        wi = u(mem, addr + 0x1738, '<Q', 8)
        mypi = u(mem, addr + 0x19E8, '<Q', 8)
        if None in (role, p220, b658, b6f8, b6f9) or role > 4:
            continue
        gate1 = 'FAIL(null)' if not p220 else 'ok'
        gate2 = 'FAIL(0x20 set)' if b658 & 0x20 else 'ok'
        print(f'PC@{addr:#x} table={vt:#x} role={role} | gate1 [+0x220]={p220:#x} {gate1} | '
              f'gate2 [+0x658]={b658:#x} {gate2} | clientFin[+0x6f8]={b6f8} serverFin[+0x6f9]={b6f9} | '
              f'QuickBars={qb or 0:#x} WorldInventory={wi or 0:#x} MyPlayerInfo={mypi or 0:#x}', flush=True)
        if mypi and mypi > 0x10000:
            # FUN_14238e820 (first early-out of the check 0x1423cc220): true when MyPlayerInfo is null, when
            # [info+0x1c8][+0x2a0] != [+0x1b8]-[+0x1e4], or when [info+0x230] > 0.
            o1c8 = u(mem, mypi + 0x1C8, '<Q', 8) or 0
            n230 = u(mem, mypi + 0x230, '<i', 4)
            txt = f'   info+0x1c8={o1c8:#x} info+0x230={n230}'
            if o1c8 > 0x10000:
                a, b, c = (u(mem, o1c8 + o, '<i', 4) for o in (0x2A0, 0x1B8, 0x1E4))
                txt += f' [0x2a0]={a} [0x1b8]={b} [0x1e4]={c}'
                if None not in (a, b, c):
                    txt += f' mismatch={a != b - c}'
            print(txt + ' => FUN_14238e820 ' + ('returns TRUE (check aborts)' if (o1c8 > 0x10000 and None not in (a, b, c) and a != b - c) or (n230 or 0) > 0 else 'returns false'), flush=True)
        if role == 2 and p220:
            path_eval(mem, addr, p220)
    print(f'scanned pid {pid}, {len(hits)} controller-table hits')
    return 0


if __name__ == '__main__':
    sys.exit(main())
