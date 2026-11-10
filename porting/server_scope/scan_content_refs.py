#!/usr/bin/env python3
"""Scan cooked .uasset/.umap headers for package dependencies and native function imports (read-only).
Usage: python3 -I porting/scan_content_native_refs.py ROOT OUT.jsonl [WORKERS]
ROOT is the extracted Content tree (FortniteGame/Content). One JSON line per package:
{pkg, deps:[hard imports], soft:[/Game/... names found only in the name table (soft references, string properties)], nat:[[module, class, function], ...], err?}.
Only the package summary, name table and import table are read."""
import json, multiprocessing as mp, os, struct, sys

TAG = 0x9E2A83C1


def fstring(b, o):
    n = struct.unpack_from('<i', b, o)[0]; o += 4
    if n == 0:
        return '', o
    if n < 0:
        n = -n
        return b[o:o + 2 * (n - 1)].decode('utf-16-le', 'replace'), o + 2 * n
    return b[o:o + n - 1].decode('latin-1'), o + n


def parse(path):
    with open(path, 'rb') as f:
        b = f.read(1 << 20)
    o = 0
    tag, legacy = struct.unpack_from('<Ii', b, o); o += 8
    if tag != TAG or legacy >= 0:
        raise ValueError('unsupported header')
    o += 4  # legacy UE3 version
    ue4, lic = struct.unpack_from('<ii', b, o); o += 8
    if legacy <= -2:
        n = struct.unpack_from('<i', b, o)[0]; o += 4
        o += n * (16 + 4) if legacy <= -3 else n * (4 + 4)
    if ue4 == 0:
        ue4 = 522  # unversioned cooked package: current engine version (4.25)
    o += 4  # total header size
    _, o = fstring(b, o)
    o += 4  # package flags
    name_count, name_off = struct.unpack_from('<ii', b, o); o += 8
    if ue4 >= 459:  # gatherable text data
        o += 8
    o += 8  # export count/offset
    imp_count, imp_off = struct.unpack_from('<ii', b, o)
    # name table (file offsets are absolute in the .uasset)
    with open(path, 'rb') as f:
        f.seek(name_off)
        blob = f.read(imp_off + imp_count * 28 + 64 - name_off)
    p = 0
    names = []
    for _ in range(name_count):
        s, p = fstring(blob, p)
        p += 4 if ue4 >= 504 else 0
        names.append(s)
    p = imp_off - name_off
    imps = []
    for _ in range(imp_count):
        cp, cpn, cn, cnn, outer, on, onn = struct.unpack_from('<iiiiiii', blob, p); p += 28
        imps.append((names[cp], names[cn], outer, names[on]))
    soft = {n.split('.')[0].split(':')[0] for n in names if n.startswith('/Game/')}
    return imps, sorted(soft)


def one(arg):
    path, root = arg
    rel = os.path.relpath(path, root)
    pkg = '/Game/' + os.path.splitext(rel)[0].replace(os.sep, '/')
    try:
        imps, soft = parse(path)
        deps, nat, cls_ = set(), set(), set()
        for i, (cpkg, cname, outer, oname) in enumerate(imps):
            if cname == 'Package' and oname.startswith('/Game/'):
                deps.add(oname)
            elif cname == 'Class' and outer < 0 and imps[-outer - 1][3].startswith('/Script/'):
                cls_.add((imps[-outer - 1][3], oname))
            elif cname == 'Function' and outer < 0:
                cls = imps[-outer - 1]
                if cls[2] < 0:
                    mod = imps[-cls[2] - 1]
                    if mod[3].startswith('/Script/'):
                        nat.add((mod[3], cls[3], oname))
        return dict(pkg=pkg, deps=sorted(deps), soft=[x for x in soft if x not in deps and x != pkg], nat=sorted(nat), cls=sorted(cls_))
    except Exception as e:
        return dict(pkg=pkg, err='%s: %s' % (type(e).__name__, e))


if __name__ == '__main__':
    ROOT = sys.argv[1]
    out = sys.argv[2]
    workers = int(sys.argv[3]) if len(sys.argv) > 3 else 64
    files = [os.path.join(d, f) for d, _, fs in os.walk(ROOT) for f in fs if f.endswith(('.uasset', '.umap'))]
    print(len(files), 'packages', file=sys.stderr)
    with mp.Pool(workers) as pool, open(out, 'w') as w:
        for n, r in enumerate(pool.imap_unordered(one, [(f, ROOT) for f in files], chunksize=64)):
            w.write(json.dumps(r) + '\n')
            if n % 50000 == 0:
                print(n, file=sys.stderr)
