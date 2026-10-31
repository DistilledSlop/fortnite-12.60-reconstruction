#!/usr/bin/env python3
"""Print the vtable slot name for a class and a byte offset (porting/behavior/vtable_slots.json).

Usage: python3 -I porting/behavior/vtable_lookup.py <class> <hexoffset>
       python3 -I porting/behavior/vtable_lookup.py AFortPlayerControllerAthena 0x700
Offsets are byte offsets into the vtable (slot = offset / 8). Use --classes to list the classes in the map.
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MAP = os.path.join(HERE, 'vtable_slots.json')


def load():
    with open(MAP) as fh:
        return json.load(fh)


def lookup(doc, cls, off):
    rec = doc['classes'].get(cls)
    if rec is None:
        return None, 'class %s is not in the map (use --classes)' % cls
    if off % 8:
        return None, 'offset 0x%x is not a slot boundary (slots are 8 bytes)' % off
    slot = off // 8
    if 'slots' not in rec:
        return None, '%s: %s' % (cls, rec.get('status', 'no table'))
    if slot < len(rec['slots']):
        e = rec['slots'][slot]
        out = [
            '%s +0x%x = slot %d (vtable %s, engine part of %s)' % (cls, off, slot, rec.get('vtable', '?'),
                                                                 rec.get('engine_ancestor', cls)),
            '  name:       %s' % e['name'],
            '  owner:      %s' % e['owner'],
            '  signature:  %s' % e['signature'],
            '  header:     %s' % e['header'],
            '  inline:     %s' % (e['inline_body'] or '-'),
            '  validated:  %s (%s)' % ('yes' if e['validated'] else 'no', e['validation']),
            '  confidence: %s' % e['confidence'],
        ]
        return '\n'.join(out), None
    for a in rec.get('anchors', []):
        if a['slot'] == slot:
            return ('%s +0x%x = slot %d (vtable %s)\n  name:       %s (anchor)\n  validated:  %s\n  confidence: HIGH\n'
                    '  evidence:   %s' % (cls, off, slot, rec.get('vtable', '?'), a['name'],
                                          'yes' if a['validated'] else 'no', a['evidence'])), None
    part = rec.get('game_part_from')
    return None, ('%s +0x%x = slot %d is game-declared (index >= %s, the engine part of %s ends there): '
                  'not named by the engine headers' % (cls, off, slot, part, rec.get('engine_ancestor', '?')))


def main(argv):
    doc = load()
    if len(argv) >= 2 and argv[1] == '--classes':
        for c, r in doc['classes'].items():
            if 'slots' in r:
                print('%-30s %s slots=%d game_part_from=%s' % (c, r.get('vtable'), len(r['slots']), r.get('game_part_from', '-')))
        return 0
    if len(argv) != 3:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    cls = argv[1]
    try:
        off = int(argv[2], 16) if argv[2].lower().startswith('0x') else int(argv[2], 16)
    except ValueError:
        print('bad hex offset: %s' % argv[2], file=sys.stderr)
        return 2
    text, err = lookup(doc, cls, off)
    if err:
        print(err, file=sys.stderr)
        return 1
    print(text)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
