"""Typed support for complete world-manager lifecycle helpers.

These helpers do not install the unresolved startup/Tick driver.
"""
import hashlib
import json
from pathlib import Path

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
EVIDENCE = Path(__file__).with_name('native-evidence.json')
HEADER = 'FortWorldManagerNative.h'


def sources():
    with IMAGE.open('rb') as stream:
        for window in json.loads(EVIDENCE.read_text())['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('world-manager lifecycle evidence changed: ' + window['address'])
    return {HEADER: r'''// Generated from world_manager_lifecycle_runtime/support.py.
#pragma once
#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include <emmintrin.h>

// Original native single-cast delegate at AFortWorldManager +0x848.
DECLARE_DELEGATE_TwoParams(FFortNativeWorldQueryResult, bool, const FString&);

namespace FortWorldManagerOriginalGrid
{
inline int32 Mul32(int32 A, int32 B)
{
    const uint32 Bits = uint32(A) * uint32(B);
    int32 Result;
    FMemory::Memcpy(&Result, &Bits, sizeof(Result));
    return Result;
}
inline int32 ConvertAndHalf(float Value)
{
    // Native CVTSS2SI uses MXCSR rounding, followed by an arithmetic shift.
    return _mm_cvtsi128_si32(_mm_srai_epi32(_mm_cvtsi32_si128(
        _mm_cvtss_si32(_mm_set_ss(Value))), 1));
}
inline float Origin(int32 Dimension, int32 TileEdge)
{
    const float HalfProduct = float(Mul32(Dimension, TileEdge) / 2);
    return float(ConvertAndHalf((HalfProduct + HalfProduct) - 0.5f)) * -512.0f;
}
}
''',
        'FortFoundationViewSignificance.h': Path(__file__).with_name('FortFoundationViewSignificance.h.in').read_text(),
        'FortFoundationViewSignificance.cpp': Path(__file__).with_name('FortFoundationViewSignificance.cpp.in').read_text(),
        'FortFoundationStreamingTiming.h': Path(__file__).with_name('FortFoundationStreamingTiming.h.in').read_text(),
        'FortFoundationStreamingTiming.cpp': Path(__file__).with_name('FortFoundationStreamingTiming.cpp.in').read_text(),
        'FortFoundationCandidateNative.h': Path(__file__).with_name('FortFoundationCandidateNative.h.in').read_text(),
        'FortBuildingFoundationRuntime.h': Path(__file__).with_name('FortBuildingFoundationRuntime.h.in').read_text(),
        'FortBuildingFoundationRuntime.cpp': Path(__file__).with_name('FortBuildingFoundationRuntime.cpp.in').read_text(),
    }


def validate(ctx, snippets_by_class):
    for name in ('AFortWorldManager', 'AFortGameState', 'AFortGameStateZone',
                 'AWorldTileFoundation', 'UBuildingStructuralSupportSystem',
                 'ABuildingFoundation', 'AFortWorldSettings', 'UBuildingGroup', 'UFortSignificanceManager',
                 'AFortHLODSMActor'):
        if ctx['class_pkg'].get(name) != '/Script/FortniteGame':
            raise ValueError('world-manager lifecycle class changed: ' + name)
    fields = {
        ('AFortWorldManager', 'NativeWorldQueryResult_0x848'):
            ('FFortNativeWorldQueryResult NativeWorldQueryResult_0x848', 0x848),
        ('AFortWorldManager', 'NativeUsedBuildingNames_0x758'):
            ('TSet<FName> NativeUsedBuildingNames_0x758', 0x758),
        ('AWorldTileFoundation', 'NativeTileX_0xee4'): ('int32 NativeTileX_0xee4', 0xee4),
        ('AWorldTileFoundation', 'NativeTileY_0xee8'): ('int32 NativeTileY_0xee8', 0xee8),
        ('ABuildingFoundation', 'NativeStreamingTiming_0xdf8'):
            ('FFoundationStreamingTiming NativeStreamingTiming_0xdf8', 0xdf8),
        ('ABuildingFoundation', 'NativeCachedBuildingGroup'):
            ('UBuildingGroup* NativeCachedBuildingGroup', 0xd98),
        ('AWorldTileFoundation', 'NativeTilePackage_0xeec'):
            ('FName NativeTilePackage_0xeec', 0xeec),
        ('AFortWorldSettings', 'NativeFoundationStreamingCacheInitialized_0x818'):
            ('bool NativeFoundationStreamingCacheInitialized_0x818', 0x818),
        ('AFortWorldSettings', 'NativeFoundationStreamingRecords_0x820'):
            ('TArray<FBuildingFoundationStreamingData> NativeFoundationStreamingRecords_0x820', 0x820),
    }
    for (owner, name), (signature, offset) in fields.items():
        entries = [e for e in snippets_by_class.get(owner, []) if e.get('function') == name]
        if (len(entries) != 1 or entries[0].get('kind') != 'member_field'
                or entries[0].get('signature') != signature
                or int(str(entries[0].get('offset')), 0) != offset):
            raise ValueError('world-manager lifecycle native field changed: ' + owner + '::' + name)
    cache_flag = next(e for e in snippets_by_class['AFortWorldSettings']
                      if e.get('function') == 'NativeFoundationStreamingCacheInitialized_0x818')
    if cache_flag.get('default') != 'false':
        raise ValueError('foundation cache constructor default changed')
