#!/usr/bin/env python3
"""Verify viewport lifecycle markers, independently of Root initialization errors."""
import re

SCOPE = 'Viewport membership and cached widget observations only; no pixels, input, or UI parity'
PHASE = re.compile(r'\bphase=([^\s]+)')
ROW = re.compile(r'ROOTVIEW v=1 phase=(before_attach|after_attach|frame|before_remove|after_remove) (?:index=([0-9]+) engine_frame=([0-9]+) )?in_viewport=([01]) cached_widget=([01])|ROOTVIEW v=1 phase=complete ok=([01])')


def verify(text):
    rows = []
    last = None
    problem = None
    for line in text.splitlines():
        if 'ROOTVIEW' not in line:
            continue
        row = line[line.index('ROOTVIEW'):].strip()
        phase = PHASE.search(row)
        if phase:
            last = phase.group(1)
        match = ROW.fullmatch(row) if line.count('ROOTVIEW') == 1 else None
        if not match:
            problem = problem or 'malformed or unsupported viewport marker'
        rows.append(match)
    result = {'passed': False, 'last_phase': last, 'rows': len(rows), 'scope': SCOPE,
              'attachment_observed': False, 'engine_frames': []}
    # Preserve a valid initial attachment observation even if a later marker
    # fails or the process exits before the lifecycle completes.
    if len(rows) >= 2 and rows[0] and rows[1]:
        first, second = rows[:2]
        result['attachment_observed'] = (
            first.group(1) == 'before_attach' and first.group(2) is None
            and first.group(4) == '0'
            and second.group(1) == 'after_attach' and second.group(2) is None
            and second.group(4) == '1' and second.group(5) == '1')
    try:
        if problem:
            raise ValueError(problem)
        expected = ('before_attach', 'after_attach', 'frame', 'frame', 'frame',
                    'before_remove', 'after_remove', 'complete')
        if len(rows) != len(expected):
            raise ValueError('expected eight viewport markers, got %d' % len(rows))
        frames = []
        for i, (match, phase) in enumerate(zip(rows, expected)):
            actual = match.group(1) or 'complete'
            if actual != phase:
                raise ValueError('wrong viewport phase or order at ' + phase)
            index, engine, viewport, cached, ok = match.group(2, 3, 4, 5, 6)
            if phase == 'frame':
                if index != str(i - 1) or engine is None:
                    raise ValueError('invalid frame index or engine frame')
                try:
                    frames.append(int(engine))
                except ValueError:
                    raise ValueError('invalid engine frame') from None
            elif index is not None or engine is not None:
                raise ValueError('unexpected frame fields')
            if phase == 'complete':
                if ok != '1':
                    raise ValueError('viewport completion failed')
            elif phase == 'before_attach':
                if viewport != '0':
                    raise ValueError('widget already in viewport')
            elif phase == 'after_remove':
                if viewport != '0':
                    raise ValueError('widget remains in viewport')
            elif viewport != '1' or cached != '1':
                raise ValueError('viewport or cached widget observation failed at ' + phase)
        result['engine_frames'] = frames
        if any(a >= b for a, b in zip(frames, frames[1:])):
            raise ValueError('engine frames are not strictly increasing')
        result['passed'] = True
    except ValueError as exc:
        result['error'] = str(exc)
    return result
