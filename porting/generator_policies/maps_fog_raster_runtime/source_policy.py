"""Emit recovered FTeamMapInfo storage and the isolated fog raster paths."""
from pathlib import Path

HERE = Path(__file__).resolve().parent
NAME = 'FTeamMapInfo'


def _validate(model):
    if model['cpp_name'] != NAME:
        return False
    expected = [(0, 'TArray<uint8> ReplicatedSeedPack;'), (68, 'uint8 TeamId;')]
    actual = [(m['offset'], m['decl'].replace('TeamId = 0;', 'TeamId;'))
              for m in model['members']]
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 72
            or model['align'] != 8 or actual != expected):
        raise ValueError('FTeamMapInfo reflected identity changed')
    return True


def apply_struct(info, model):
    if _validate(model):
        model['members'][1]['decl'] = 'uint8 TeamId = 0;'
    return model


def includes(model):
    _validate(model)
    return []  # CoreMinimal.h supplies FMath and TArray.


def fields_after(model, member):
    if not _validate(model):
        return []
    if member['offset'] == 0:
        return [
            '\t// Native arrays: original copy uses byte elements for the first three.',
            '\tTArray<uint8> NativeRevealBits_0x10;',
            '\tTArray<uint8> NativeFogPixels_0x20;',
            '\t// Fourth array copies trivial eight-byte payloads; semantic type is unresolved.',
            '\tTArray<uint64> NativeEightByteEntries_0x30;',
            '\tint32 NativeTouchedCount_0x40 = 0;',
            ''
        ]
    if member['offset'] == 68:
        return [
            '\tuint8 NativeByte_0x45 = 0;',
            '\tuint8 NativeFlag0_0x46 : 1;',
            '\tuint8 NativeFlag1_0x46 : 1;',
            ''
        ]
    return []


def declarations(model):
    if not _validate(model):
        return []
    source = (HERE / 'FogRaster.cpp').read_text()
    start = source.index('    void DrawGradient(')
    end = source.rindex('\n}')
    body = source[start:end]
    old_gradient = ('void DrawGradient(TArray<uint8>& Pixels, int32& TouchedCount, '
                    'int32 CenterX, int32 CenterY,')
    old_fast = ('void DrawFastDisk(TArray<uint8>& Pixels, int32& TouchedCount, '
                'int32 CenterX, int32 CenterY,')
    if body.count(old_gradient) != 1 or body.count(old_fast) != 1:
        raise ValueError('fog raster draft signatures changed')
    body = body.replace(old_gradient, 'void NativeDrawFogGradient(int32 CenterX, int32 CenterY,')
    body = body.replace(old_fast, 'void NativeDrawFogFastDisk(int32 CenterX, int32 CenterY,')
    body = body.replace('Pixels[', 'NativeFogPixels_0x20[').replace('TouchedCount +=', 'NativeTouchedCount_0x40 +=')
    return [
        '\t// Original constructor clears both logical flags; array constructors own their storage.',
        '\tFTeamMapInfo() : NativeFlag0_0x46(0), NativeFlag1_0x46(0) {}',
        '',
        '\t// Isolated raster paths; original fast-mode selection remains unresolved.',
        '\t// Caller supplies a 256x256 pixel array and valid native arithmetic inputs.',
    ] + ['\t' + line[4:] if line.startswith('    ') else line for line in body.splitlines()] + ['']
