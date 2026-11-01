#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "CoreMinimal.h"
#include "LegacySlateMaterial.h"

// The offsets below refer to the complete selected resource, including its two
// leading cooked/valid words. Raw bytes remain authoritative provenance.
struct FLegacyOriginalRecord
{
    FString Type;
    int32 Begin = 0, End = 0;
    FSHAHash OutputHash;
    uint32 Frequency = 0, UncompressedBytes = 0;
};
struct FLegacyOriginalMaterial
{
    TArray<uint8> Bytes;
    TArray<FName> Names;
    FLegacyMaterialIR IR;
    FString MaterialPath, FriendlyName;
    TArray<FLegacyOriginalRecord> Records;
    uint32 UsedSceneTextures = 0;
    uint16 TextureSamplesVS = 0, TextureSamplesPS = 0, VirtualTextureLookups = 0;
    uint8 UVScalars = 0, CustomInterpolatorScalars = 0, VirtualTextureMask = 0, Flags = 0;
};
bool DecodeLegacyOriginalMaterial(FLegacyOriginalMaterial& Out, FString& Error);
#endif
