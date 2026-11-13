#!/usr/bin/env python3
"""Stage cooked /Game packages (plus their hard-import closure) for the Linux dedicated server.

    python3 -I tools/stage_server_content.py /Game/X/Y [/Game/...] [--dry-run] [--max N] [--soft]
        [--src DIR] [--dst DIR] [-v]

Parses the cooked .uasset summary (legacy header, custom versions, name map, import map) with the stdlib only.
Follows /Game/ packages named in the import table (outer==0, class "Package"). /Script/ and /Engine/ packages are not
copied; /Script modules are listed at the end. --soft also follows /Game/ package names found in the name map
(soft object paths, string-asset references). Existing files in the stage are never overwritten.
"""
import argparse, os, shutil, struct, sys

SRC = '/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content'
DST = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'generated', 'linux-stage',
                   'LinuxNoEditor', 'FortniteLinux', 'Content')
EXTS = ('.uasset', '.uexp', '.ubulk', '.umap', '.uptnl', '.upipelinecache')
PKG_FILTER_EDITOR_ONLY = 0x80000000


class R:
    def __init__(s, b): s.b, s.p = b, 0
    def i32(s): v = struct.unpack_from('<i', s.b, s.p)[0]; s.p += 4; return v
    def u32(s): v = struct.unpack_from('<I', s.b, s.p)[0]; s.p += 4; return v
    def skip(s, n): s.p += n
    def fstr(s):
        n = s.i32()
        if n == 0: return ''
        if n > 0:
            v = s.b[s.p:s.p + n - 1].decode('latin-1'); s.p += n; return v
        n = -n
        v = s.b[s.p:s.p + 2 * (n - 1)].decode('utf-16-le'); s.p += 2 * n; return v


def parse_summary(path):
    """Return (names, import_list) where import_list = [(class_pkg, class, outer, objname)]."""
    with open(path, 'rb') as f:
        b = f.read(0x20000)
    r = R(b)
    if r.u32() != 0x9E2A83C1: raise ValueError('bad tag')
    legacy = r.i32()
    if legacy >= 0 or legacy < -7: raise ValueError('legacy version %d' % legacy)
    if legacy != -4: r.i32()                      # legacy UE3 version
    ue4 = r.i32()
    lic = r.i32()
    if legacy <= -2:
        for _ in range(r.i32()):
            r.skip(16 + 4)                        # guid + version (legacy -5..-7 format)
    total_hdr = r.i32()
    if ue4 == 0: ue4 = 518                        # unversioned package: latest 4.25 layout
    r.fstr()                                      # folder name
    flags = r.u32()
    name_count, name_off = r.i32(), r.i32()
    if not (flags & PKG_FILTER_EDITOR_ONLY) and ue4 >= 516:
        r.fstr()                                  # localization id
    if ue4 >= 459: r.skip(8)                      # gatherable text count/offset
    r.skip(8)                                     # export count/offset
    imp_count, imp_off = r.i32(), r.i32()
    if total_hdr > len(b) or name_off + 1 > len(b):
        with open(path, 'rb') as f: b = f.read(total_hdr)
    r = R(b); r.p = name_off
    names = []
    for _ in range(name_count):
        names.append(r.fstr())
        if ue4 >= 504: r.skip(4)                  # case-preserving + non-case hash
    imp_size = 28 + (8 if (ue4 >= 520 and not (flags & PKG_FILTER_EDITOR_ONLY)) else 0)
    imports = []
    r.p = imp_off

    def fname():
        i, n = r.i32(), r.i32()
        nm = names[i] if 0 <= i < len(names) else '?%d' % i
        return nm if n == 0 else '%s_%d' % (nm, n - 1)
    for _ in range(imp_count):
        st = r.p
        cp, cn = fname(), fname()
        outer = r.i32()
        on = fname()
        imports.append((cp, cn, outer, on))
        r.p = st + imp_size
    return names, imports


def pkg_file(src, pkg, ext):
    return os.path.join(src, pkg[len('/Game/'):] + ext)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('packages', nargs='+')
    ap.add_argument('--dry-run', action='store_true')
    ap.add_argument('--max', type=int, default=3000, help='max packages visited (default 3000)')
    ap.add_argument('--soft', action='store_true', help='also follow /Game/ package names in the name map')
    ap.add_argument('--src', default=SRC)
    ap.add_argument('--dst', default=os.path.normpath(DST))
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()

    seen, order, missing, scripts, engine = set(), [], [], set(), set()
    parse_err = []
    queue = []
    for p in a.packages:
        p = p.split('.')[0].rstrip('/')
        if not p.startswith('/Game/'): print('skip non-/Game/ root', p); continue
        queue.append(p)
    copied_files = 0; copied_bytes = 0; existing = 0; would_files = 0; would_bytes = 0
    truncated = False
    while queue:
        pkg = queue.pop(0)
        if pkg in seen: continue
        if len(seen) >= a.max:
            truncated = True; break
        seen.add(pkg)
        ua = pkg_file(a.src, pkg, '.uasset')
        um = pkg_file(a.src, pkg, '.umap')
        main_f = ua if os.path.exists(ua) else um if os.path.exists(um) else None
        if not main_f:
            missing.append(pkg); continue
        order.append(pkg)
        try:
            names, imports = parse_summary(main_f)
        except Exception as e:
            parse_err.append((pkg, repr(e))); imports, names = [], []
        deps = []
        for cp, cn, outer, on in imports:
            if outer == 0 and cn == 'Package':
                if on.startswith('/Game/'): deps.append(on)
                elif on.startswith('/Script/'): scripts.add(on)
                elif on.startswith('/Engine/'): engine.add(on)
        if a.soft:
            for n in names:
                if n.startswith('/Game/'):
                    deps.append(n.split('.')[0].split(':')[0])
        for d in deps:
            if d not in seen: queue.append(d)
        for ext in EXTS:
            sf = pkg_file(a.src, pkg, ext)
            if not os.path.exists(sf): continue
            df = pkg_file(a.dst, pkg, ext)
            if os.path.exists(df): existing += 1; continue
            sz = os.path.getsize(sf)
            would_files += 1; would_bytes += sz
            if not a.dry_run:
                os.makedirs(os.path.dirname(df), exist_ok=True)
                shutil.copyfile(sf, df)
                copied_files += 1; copied_bytes += sz
            if a.verbose: print(('would copy ' if a.dry_run else 'copied ') + pkg + ext, sz)
    print('roots: %d  packages visited: %d  found: %d  missing: %d' % (len(a.packages), len(seen), len(order), len(missing)))
    if a.dry_run:
        print('DRY RUN: would copy %d files, %.2f MB (%d bytes); %d files already staged' % (would_files, would_bytes / 1e6, would_bytes, existing))
    else:
        print('files copied: %d  bytes copied: %d (%.2f MB)  already staged: %d' % (copied_files, copied_bytes, copied_bytes / 1e6, existing))
    if truncated: print('WARNING: --max %d reached, closure truncated (%d queued)' % (a.max, len(queue)))
    if missing:
        print('missing from extraction (%d):' % len(missing))
        for m in missing[:60]: print('  ' + m)
        if len(missing) > 60: print('  ... %d more' % (len(missing) - 60))
    if parse_err:
        print('parse errors (%d):' % len(parse_err))
        for m in parse_err[:10]: print('  %s %s' % m)
    print('/Script modules referenced (%d): %s' % (len(scripts), ' '.join(sorted(scripts))))
    print('/Engine packages referenced (not copied): %d' % len(engine))


if __name__ == '__main__':
    main()
