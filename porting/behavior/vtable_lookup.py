#!/usr/bin/env python3
"""Vtable slot lookup (porting/behavior/vtable_slots.json, written by build_vtable_slots.py).

Usage: python3 -I porting/behavior/vtable_lookup.py --class AFortPlayerControllerAthena --slot 0x700
       python3 -I porting/behavior/vtable_lookup.py AFortPlayerControllerAthena 0x700      (same, positional)
       python3 -I porting/behavior/vtable_lookup.py --classes          (named classes, header model)
       python3 -I porting/behavior/vtable_lookup.py --constructed AFortFoo   (classes with a constructor vtable)
--slot is a byte offset into the vtable (hex; slot = offset / 8). Without 0x it is still read as hex.
Output: the slot target (from the late-capture image), its label (exec / ported / ctor), the class that first set the
entry (owner, so inheritance is visible), the header-model name when one exists, and the cached decompile if any.
"""
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
MAP = os.path.join(HERE, 'vtable_slots.json')
GENERATED = os.path.join(REPO, 'porting', 'generated')


def load():
    with open(MAP) as fh:
        return json.load(fh)


def resolve(vt, cls, idx):
    """(target, owner): nearest class (cls first) whose own delta holds slot idx."""
    c, hops = cls, 0
    while c in vt and hops < 64:
        r = vt[c]
        if idx >= r['slots']:
            return None, None
        t = r['own'].get(str(idx))
        if t:
            return t, c
        c, hops = r['parent'], hops + 1
    return None, None


def decompile_for(target):
    """Cached decompile of the target address (decomp dirs first), or None."""
    if not target:
        return None
    hits = sorted(glob.glob(os.path.join(GENERATED, '*', target + '.c')),
                  key=lambda p: (os.path.basename(os.path.dirname(p)) not in ('decomp', 'decomp_improved'), p))
    return os.path.relpath(hits[0], REPO) if hits else None


def named_slot(doc, cls, idx, owner):
    """Header-model name of the slot (the named classes of the map), for cls or the owner that set it."""
    for c in (cls, owner):
        rec = doc['classes'].get(c) or {}
        for e in rec.get('slots', []) or []:
            if e.get('slot') == idx:
                return '%s (header model of %s, %s)' % (e['name'], c, e['confidence'])
    return None


def lookup(doc, cls, off):
    if off % 8:
        return None, 'offset 0x%x is not a slot boundary (slots are 8 bytes)' % off
    idx = off // 8
    vt = doc.get('vtables', {}).get('classes', {})
    labels = doc.get('vtables', {}).get('labels', {})
    if cls not in vt:
        return None, ('%s has no primary vtable in the map (use --constructed to list the classes, --classes for the '
                      'header-model names)' % cls)
    target, owner = resolve(vt, cls, idx)
    if target is None:
        if cls in vt and idx >= vt[cls]['slots']:
            return None, '%s has %d slots; slot %d is out of range' % (cls, vt[cls]['slots'], idx)
        return None, '%s +0x%x = slot %d: no target (class not in the all-class part)' % (cls, off, idx)
    lab = labels.get(target)
    out = ['%s +0x%x = slot %d (vtable %s, ctor %s)' % (cls, off, idx, vt[cls]['vtable'], vt[cls]['ctor']),
           '  target:     %s' % target,
           '  label:      %s%s' % (lab[0] if lab else '-', ' (%s)' % lab[1] if lab else ''),
           '  owner:      %s%s' % (owner, '' if owner == cls else ' (inherited by %s)' % cls)]
    named = named_slot(doc, cls, idx, owner)
    if named:
        out.append('  named:      %s' % named)
    dec = decompile_for(target)
    if lab and lab[1] == 'exec' and '::' in lab[0]:
        cname, fname = lab[0].split('::', 1)
        p = os.path.join(GENERATED, 'decomp', cname, fname + '.c')
        if os.path.exists(p):
            out.append('  decompile:  %s' % os.path.relpath(p, REPO))
    out.append('  cached:     %s' % (dec or '-'))
    return '\n'.join(out), None


def named_classes(doc):
    for c, r in doc['classes'].items():
        if 'slots' in r:
            print('%-30s %s slots=%d game_part_from=%s' % (c, r.get('vtable'), len(r['slots']), r.get('game_part_from', '-')))


def constructed(doc, cls=None):
    vt = doc.get('vtables', {}).get('classes', {})
    if cls:
        rec = vt.get(cls)
        if rec is None:
            print('%s: no primary vtable' % cls, file=sys.stderr)
            return 1
        print('%s vtable %s slots=%d parent=%s ctor=%s own=%d secondary=%s' % (
            cls, rec['vtable'], rec['slots'], rec['parent'], rec['ctor'], len(rec['own']),
            ','.join(sorted(rec.get('secondary', {}), key=lambda k: int(k, 16))) or '-'))
        return 0
    for c, r in sorted(vt.items()):
        print('%-40s %s slots=%d parent=%s' % (c, r['vtable'], r['slots'], r['parent'] or '-'))
    return 0


def parse(argv):
    """(mode, cls, off) from the argument list; mode is one of lookup / classes / constructed / usage."""
    args = argv[1:]
    if not args:
        return 'usage', None, None
    if args[0] == '--classes':
        return 'classes', None, None
    if args[0] == '--constructed':
        return 'constructed', args[1] if len(args) > 1 else None, None
    cls = off = None
    i = 0
    pos = []
    while i < len(args):
        a = args[i]
        if a == '--class' and i + 1 < len(args):
            cls = args[i + 1]
            i += 2
            continue
        if a == '--slot' and i + 1 < len(args):
            off = args[i + 1]
            i += 2
            continue
        pos.append(a)
        i += 1
    if cls is None and off is None and len(pos) == 2:
        cls, off = pos
    if not cls or off is None:
        return 'usage', None, None
    try:
        return 'lookup', cls, int(off, 16)
    except ValueError:
        return 'bad', cls, off


def main(argv):
    mode, cls, off = parse(argv)
    if mode == 'usage':
        print(__doc__.strip(), file=sys.stderr)
        return 2
    if mode == 'bad':
        print('bad hex offset: %s' % off, file=sys.stderr)
        return 2
    doc = load()
    if mode == 'classes':
        named_classes(doc)
        return 0
    if mode == 'constructed':
        return constructed(doc, cls)
    text, err = lookup(doc, cls, off)
    if err:
        print(err, file=sys.stderr)
        return 1
    print(text)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
