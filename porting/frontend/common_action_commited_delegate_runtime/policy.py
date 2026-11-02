"""Fail-closed seed and model for the original standalone CommonUI delegate."""
import json
from pathlib import Path

ROOT = '0x140809720'
EXPECTED = json.loads((Path(__file__).resolve().parent / 'original-type.json').read_text())


def validate_identity(info):
    if (info is None or info.fn != ROOT or info.kind != 'delegate'
            or info.cpp_name != 'FCommonActionCommited'
            or info.package != '/Script/CommonUI'
            or info.header_name != 'FCommonActionCommited.h'
            or info.rec != EXPECTED):
        raise ValueError('original CommonActionCommited delegate identity or metadata differs')


def build_delegate(info):
    """Return a model only for this exact identity; None means use the ordinary gate.

    Hook at the beginning of TypeGate.build_delegate, before generic flag rejection.
    A name OR function collision is checked to prevent silently accepting drift.
    """
    if info.fn != ROOT and info.cpp_name != 'FCommonActionCommited':
        return None
    validate_identity(info)
    return {'fn': ROOT, 'cpp_name': info.cpp_name, 'kind': 'delegate',
            'package': info.package, 'params': [('bool&', 'bPassThrough')],
            'used': {}, 'forward_decls': [], 'multicast': False}, None


def add_roots(gate, seeds):
    info = gate.info(ROOT)
    validate_identity(info)
    reason = gate.reason(info)
    if reason is not None:
        raise ValueError('original CommonActionCommited delegate unavailable: ' + reason)
    model = gate.model(info)
    expected = build_delegate(info)[0]
    if model != expected:
        raise ValueError('original CommonActionCommited delegate model differs')
    seeds.setdefault(ROOT, set()).add('Delegate')
