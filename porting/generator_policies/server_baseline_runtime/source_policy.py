"""Preserve three validated 12.60 server baseline types across generation.

The playlist body is the pre-existing baseline, recovered at 0x142a9ccc0;
it is deliberately not inferred from the generic fast-array serializer.
"""
import hashlib
import json
from pathlib import Path

ROOTS = {
    '0x142a9daf0': ('FPlaylistPropertyArray', 'struct', '7ea2c071fd2903d9c9eec12c4bb849a75d847daa66501f6b68a9804bf50a0aa9'),
    '0x142710070': ('EAIHotSpotSlotFilter', 'enum', 'e9b08869e03a1ba6886ddee1ba48397900a60e0dc8993d2012121432a1ce211c'),
    '0x142b8f1a0': ('ESolarisPlayerGameAttribute', 'enum', '6f679a76d7d4e4eab66fe29cbe5429307937408485cd4e707055539b15f13345'),
}


def add_roots(gate, seeds):
    for fn, (name, kind, digest) in ROOTS.items():
        info = gate.info(fn)
        if info is None or info.cpp_name != name or info.kind != kind or info.package != '/Script/FortniteGame':
            raise ValueError('baseline type identity changed: ' + name)
        actual = hashlib.sha256(json.dumps(info.rec, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
        if actual != digest:
            raise ValueError('baseline reflection record changed: ' + name)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('baseline type cannot be emitted: %s: %s' % (name, reason))
        seeds.setdefault(fn, set()).add('')


def preserve_playlist(gate, models, texts):
    fn = '0x142a9daf0'
    info = gate.info(fn)
    model = models.get(fn)
    if model is None or model.get('parent') != 'FFastArraySerializer':
        raise ValueError('baseline playlist model or parent missing')
    net = model.get('net') or {}
    expected = {'kind': 'delta', 'member': 'PropertyOverrides', 'item': 'FPropertyOverride',
                'parent': 'FFastArraySerializer', 'traits': ['HasNetDeltaSerializer'], 'vtable': '0x1464e1fd0'}
    if net != expected:
        raise ValueError('baseline playlist serializer identity changed')
    if [m['name'] for m in model['members']] != ['PropertyOverrides', 'PlaylistReplicationKey', 'BasePlaylist', 'OverridePlaylist', 'PropertyOverrideScopes']:
        raise ValueError('baseline playlist emitted members changed')
    header = texts[info.header_name]
    generic = ('\t// Native NetDeltaSerialize, the FFastArraySerializer pattern (ops vtable 0x1464e1fd0, traits HasNetDeltaSerializer): the\n'
               '\t// decompiled function calls FFastArraySerializer::FastArrayDeltaSerialize over PropertyOverrides.\n'
               '\tbool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)\n\t{\n'
               '\t\treturn FFastArraySerializer::FastArrayDeltaSerialize<FPropertyOverride, FPlaylistPropertyArray>(PropertyOverrides, DeltaParms, *this);\n'
               '\t}\n')
    include = '#include "Engine/NetSerialization.h"\n'
    if header.count(generic) != 1 or header.count(include) != 1 or 'UObject/CoreNet.h' in header:
        raise ValueError('baseline playlist generated header changed')
    body = Path(__file__).with_name('playlist_net_delta.inc').read_text(encoding='utf-8')
    if not body.endswith('\n'):
        raise ValueError('baseline playlist serializer body missing final newline')
    header = header.replace(include, include + '#include "UObject/CoreNet.h"\n', 1)
    texts[info.header_name] = header.replace(generic, body, 1)
