#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "CoreMinimal.h"
#include "LegacySlateMaterial.h"

// The offsets below refer to the complete selected resource, including its two
// leading cooked/valid words. Raw bytes remain authoritative provenance.
struct FLegacyOriginalUniformBinding { uint16 BaseIndex = 0; bool bBound = false; };
struct FLegacyOriginalParameter { uint16 BaseIndex = 0, Size = 0, BufferIndex = 0; };
struct FLegacyOriginalPair { uint16 First = 0, Second = 0; };
struct FLegacyOriginalNamedUniform { FName Name; FLegacyOriginalUniformBinding Binding; };
struct FLegacyOriginalLooseBuffer { FLegacyOriginalPair Header; TArray<FLegacyOriginalPair> Parameters; };
struct FLegacyOriginalRecord
{
    FString Type;
    int32 Begin = 0, End = 0;
    FSHAHash OutputHash;
    uint32 Frequency = 0, UncompressedBytes = 0;
    FName Pipeline, VertexFactory, ResourceType;
    int32 PermutationId = 0, ResourcePermutationId = 0;
    FSHAHash VertexFactorySourceHash, SourceHash, VFParameterSourceHash;
    FLegacyOriginalUniformBinding SceneTextures, Material, Pass;
    TArray<FLegacyOriginalUniformBinding> CollectionBuffers, AdditionalUniforms;
    TArray<FLegacyOriginalParameter> AdditionalParameters;
    FLegacyOriginalParameter VTFeedback, VFLod;
    bool bSpeedTree = false;
    uint32 LayoutSize = 0, InstructionCount = 0, BindingWord = 0;
    uint16 RootParameterBufferIndex = 0;
    FString DebugDescription;
    TArray<uint16> LayoutOffsets;
    TArray<uint8> LayoutTypes;
    TArray<FLegacyOriginalNamedUniform> UniformParameters;
    TArray<FLegacyOriginalPair> UniformBuffers, Samplers, SRVs;
    TArray<FLegacyOriginalLooseBuffer> LooseBuffers;
};
struct FLegacyOriginalPipeline
{
    FName Type, VertexFactory;
    TArray<int32> StageIndices;
    int32 Begin = 0, End = 0;
};
struct FLegacyOriginalMaterial
{
    TArray<uint8> Bytes;
    TArray<FName> Names;
    FLegacyMaterialIR IR;
    FString MaterialPath, FriendlyName;
    TArray<FLegacyOriginalRecord> Records;
    TArray<FLegacyOriginalPipeline> Pipelines;
    uint32 UsedSceneTextures = 0;
    uint16 TextureSamplesVS = 0, TextureSamplesPS = 0, VirtualTextureLookups = 0;
    uint8 UVScalars = 0, CustomInterpolatorScalars = 0, VirtualTextureMask = 0, Flags = 0;
};
bool DecodeLegacyOriginalMaterial(FLegacyOriginalMaterial& Out, FString& Error);
#endif
