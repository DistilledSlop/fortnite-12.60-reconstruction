"""Emit constructor stores from the original scalar registry and recovered struct evidence."""
import json
import math
import re
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
PORTING = HERE.parents[1]
CLASS = 'AFortWorldManager'
# Types are a contract, not another copy of the recovered scalar values.
OWN_SCALARS = {
    'CurrentZoneIndex': 'int32', 'DangerPlayerSpawnExclusionRadius': 'float',
    'FinalNumChosenBuildingFoundations': 'int32', 'MaxConsecutiveFails': 'int32',
    'MaxTiles': 'int32', 'NumNonAutoconstructedTiles': 'int32',
    'ObjectivePlayerSpawnExclusionRadius': 'float', 'SaveFrequency_Seconds': 'uint32',
    'TileEdgeSize': 'int32', 'VerticalCellNumOffsetFromOrigin': 'int32',
    'X_Size': 'int32', 'Y_Size': 'int32', 'Z_StructuralGrid_Size': 'int32',
}
INHERITED = {'NetDormancy': 'dormancy', 'NetPriority': 'float', 'RemoteRole': 'role',
             'bAlwaysRelevant': 'bool', 'bReplayRewindable': 'bool', 'bReplicates': 'bool'}
EXTRA = {'bSavingEnabled': ('bit', 'uint8'), 'bStreamInBuildings': ('bit', 'uint8'),
         'bConstructTileWorld': ('bit', 'uint8'), 'bUseSimMaps': ('bit', 'uint8'),
         'InitialGlobalWind': ('wind', 'FFortWindIntensityAndDirection'),
         'FriendlyPlayerSpawnExtent': ('vector', 'FVector'), 'PadPlayerSpawnExtent': ('vector', 'FVector')}


def literal(value, typ):
    if typ in ('bool', 'bit'):
        if type(value) is not bool:
            raise ValueError('invalid boolean default')
        return 'true' if value else 'false'
    if typ in ('int32', 'uint32', 'dormancy', 'role'):
        if type(value) is not int or not (-2147483648 <= value <= 2147483647):
            raise ValueError('invalid integer default')
        if typ == 'uint32' and value < 0:
            raise ValueError('negative unsigned default')
        if typ == 'dormancy': return 'static_cast<ENetDormancy>(%d)' % value
        if typ == 'role': return 'static_cast<ENetRole>(%d)' % value
        return str(value)
    if typ == 'float':
        if type(value) not in (float, int) or not math.isfinite(value):
            raise ValueError('invalid float default')
        # Nine significant digits round-trip the original IEEE-754 float.
        return format(value, '.9g') + ('f' if '.' in format(value, '.9g') or 'e' in format(value, '.9g') else '.0f')
    raise ValueError('unknown default type ' + typ)


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'AInfo':
        raise ValueError(CLASS + ': unexpected parent')
    scalars = json.loads((PORTING / 'native/Config/OriginalDefaults.json').read_text())['FortWorldManager']
    if set(scalars) != set(OWN_SCALARS) | set(INHERITED):
        raise ValueError(CLASS + ': scalar defaults registry changed; review constructor evidence')
    recovered = json.loads((HERE / 'recovered.json').read_text())
    if recovered.get('class') != CLASS or recovered.get('constructor') != '0x141f7c920' or set(recovered['fields']) != set(EXTRA):
        raise ValueError(CLASS + ': unexpected recovered evidence')
    bits_metadata = json.loads((PORTING / 'generated/reflection/bool_bits.json').read_text())['by_class'][CLASS]
    for name, (kind, _) in EXTRA.items():
        if kind != 'bit':
            continue
        evidence = recovered['fields'][name]
        matches = [b for b in bits_metadata if b.get('name') == name]
        if (len(matches) != 1 or matches[0].get('offset') != int(evidence['offset'], 16)
                or matches[0].get('mask') != int(evidence['mask'], 16)):
            raise ValueError(CLASS + ': bitfield metadata changed: ' + name)
    wind_header = (PORTING / 'native/Source/FortniteGame/Generated/FFortWindIntensityAndDirection.h').read_text()
    for component in ('WindIntensity', 'WindHeading'):
        if len(re.findall(r'\bfloat\s+' + component + r'\s*;', wind_header)) != 1:
            raise ValueError(CLASS + ': wind struct metadata changed: ' + component)
    members = model.get('members_decl_order', [])
    for name, typ in OWN_SCALARS.items():
        if len([m for m in members if m.get('name') == name and m.get('decl', '').strip() == typ + ' ' + name + ';']) != 1:
            raise ValueError(CLASS + ': unexpected scalar declaration: ' + name)
    for name, (kind, typ) in EXTRA.items():
        decl = typ + ' ' + name + (' : 1;' if kind == 'bit' else ';')
        found = [m.get('decl', '').strip() for m in members if m.get('name') == name]
        if len(found) != 1 or re.sub(r'\s+', '', found[0]) != re.sub(r'\s+', '', decl):
            raise ValueError(CLASS + ': unexpected declaration: ' + name)
        if recovered['fields'][name].get('type') != kind:
            raise ValueError(CLASS + ': unexpected recovered type: ' + name)
    marker = ('AFortWorldManager::AFortWorldManager(const FObjectInitializer& ObjectInitializer)\n'
              '    : Super(ObjectInitializer)\n{\n'
              '    USceneComponent* NativeDefaultSubobject0 = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));\n'
              '    (void)NativeDefaultSubobject0;\n}')
    if cpp.count(marker) != 1 or len(re.findall(r'\bAFortWorldManager::AFortWorldManager\s*\(', cpp)) != 1:
        raise ValueError(CLASS + ': native subobject constructor changed; review merge')
    lines = ['    // Recovered original constructor defaults; map serialization can override these.']
    for name, typ in {**OWN_SCALARS, **INHERITED}.items():
        value = literal(scalars[name], typ)
        if name == 'RemoteRole': lines.append('    SetRemoteRoleForBackwardsCompat(%s);' % value)
        else: lines.append('    %s = %s;' % (name, value))
    for name, (kind, _) in EXTRA.items():
        data = recovered['fields'][name]
        if kind == 'bit':
            lines.append('    %s = %s;' % (name, literal(data['value'], kind)))
        else:
            bits = data['bits']
            if len(bits) != (3 if kind == 'vector' else 2):
                raise ValueError(CLASS + ': invalid struct default ' + name)
            vals = [literal(struct.unpack('<f', struct.pack('<I', int(b, 16)))[0], 'float') for b in bits]
            if kind == 'vector': lines.append('    %s = FVector(%s);' % (name, ', '.join(vals)))
            else:
                lines.append('    %s.WindIntensity = %s;' % (name, vals[0]))
                lines.append('    %s.WindHeading = %s;' % (name, vals[1]))
    return header, cpp.replace(marker, marker[:-1] + '\n'.join(lines) + '\n}', 1)
