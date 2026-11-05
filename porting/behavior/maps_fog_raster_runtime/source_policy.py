"""Emit the evidenced, partial FTeamMapInfo fog storage and raster helpers."""
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
    if not _validate(model) or member['offset'] != 0:
        return []
    return [
        '\t// Recovered native storage; offsets name original Win64 fields, not this host layout.',
        '\t// The original fourth array at +0x30 and trailing native flags remain unrecovered.',
        '\tTArray<uint8> NativeRevealBits_0x10;',
        '\tTArray<uint8> NativeFogPixels_0x20;',
        '\tint32 NativeTouchedCount_0x40 = 0;',
        ''
    ]


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
        '\t// Partial raster helpers; original ABI and fast-mode dispatch are unresolved.',
        '\t// Caller supplies a 256x256 pixel array and valid native arithmetic inputs.',
    ] + ['\t' + line[4:] if line.startswith('    ') else line for line in body.splitlines()] + ['']
