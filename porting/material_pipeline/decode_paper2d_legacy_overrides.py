#!/usr/bin/env python3
"""Decode the bounded UE4 v379 uncooked MaterialInstance native tail.

The start receipt must come from Cue4Inspect --legacy-material-start. This
supports only empty old static parameter arrays and no inline shader resources;
all other cases fail closed. It is not a general Unreal package parser.
"""
import argparse
import hashlib
import json
import math
import struct
from pathlib import Path


def decode(data, evidence):
    if hashlib.sha256(data).hexdigest() != evidence['sourceSha256']:
        raise ValueError('asset hash does not match start report')
    c = evidence['constants']
    version = evidence['ue4']
    if evidence['cooked'] or evidence['filterEditorOnly'] or evidence['classDefaultObject'] or not evidence['staticPermutation']:
        raise ValueError('unsupported package or export flags')
    if version != 379 or evidence['customVersions'] != 0 or not (c['purged'] <= version < c['fixedSerialization'] and c['phase2'] <= version and version >= c['shadowDepth'] and version < c['dither']):
        raise ValueError('unsupported version or custom-version layout')
    start, end = evidence['nativeStart'], evidence['exportOffset'] + evidence['exportSize']
    if not (0 <= evidence['exportOffset'] <= start < end <= len(data)):
        raise ValueError('invalid export bounds')
    offset = start
    fields = {}

    def read(name, fmt):
        nonlocal offset
        size = struct.calcsize('<' + fmt)
        if offset + size > end:
            raise ValueError('truncated ' + name)
        value = struct.unpack_from('<' + fmt, data, offset)[0]
        fields[name] = {'offset': offset, 'value': value}
        offset += size
        return value

    def flag(name):
        value = read(name, 'I')
        if value not in (0, 1):
            raise ValueError('invalid bool ' + name)
        return bool(value)

    # MaterialInstance.cpp::Serialize -> old FStaticParameterSet::Serialize.
    for name in ('staticSwitchParameters', 'staticComponentMaskParameters', 'terrainLayerWeightParameters'):
        if read(name + 'Count', 'i') != 0:
            raise ValueError('nonempty static parameter array unsupported')
    # Material.cpp::SerializeInlineShaderMaps.
    if read('inlineShaderResourceCount', 'i') != 0:
        raise ValueError('inline shader resources unsupported')
    flag('deprecatedOverrideBaseProperties')
    if flag('hasPropertyOverrides'):
        flag('overrideOpacityMaskClipValue')
        if not math.isfinite(read('opacityMaskClipValue', 'f')):
            raise ValueError('invalid opacity mask clip')
        flag('overrideBlendMode')
        blend = read('blendMode', 'B')
        flag('overrideShadingModel')
        shading = read('shadingModel', 'B')
        flag('overrideTwoSided')
        flag('twoSided')
        if blend > 6 or shading > 11:
            raise ValueError('unknown enum value')
    if offset != end:
        raise ValueError(f'unconsumed export bytes: {end - offset}')
    return {'version': version, 'nativeStart': start, 'exportEnd': end, 'fields': fields,
            'effectiveBlend': ('BLEND_Translucent' if fields.get('overrideBlendMode', {}).get('value') == 1 and fields.get('blendMode', {}).get('value') == 2 else 'not established'),
            'sourceSha256': hashlib.sha256(data).hexdigest()}


def main():
    p = argparse.ArgumentParser()
    p.add_argument('asset'); p.add_argument('start'); p.add_argument('--output', required=True)
    args = p.parse_args()
    raw = Path(args.start).read_bytes()
    report = decode(Path(args.asset).read_bytes(), json.loads(raw[raw.index(b'{'):]))
    report['startReportSha256'] = hashlib.sha256(raw).hexdigest()
    Path(args.output).write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
