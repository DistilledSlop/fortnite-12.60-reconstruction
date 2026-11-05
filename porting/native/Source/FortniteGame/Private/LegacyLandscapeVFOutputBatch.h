#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFOutputBatch.h"

// Prepare one decoded landscape map. The manifest and selected map bytes must
// each have independently supplied SHA-256 pins. MapOffset is the selected map's
// byte offset within the resource used to produce manifest occurrence offsets;
// MapIndex selects its occurrence inventory. Contracts are in decoded record
// order and their original-layout owners must outlive use of the outputs.
// No bindings or provider are installed. Outputs stay unchanged on failure.
bool BuildLegacyLandscapeVFOutputBatch(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ExpectedManifestSHA256,
    const FString& ExpectedMapSHA256, int32 MapIndex, int32 MapOffset,
    const FShaderParametersMetadata& Metadata, const TArray<FLegacyLocalVFRecordContracts>& Contracts,
    TArray<FLegacyMaterialShaderInput>& Outputs, TArray<FLegacyMaterialPipelineInput>& Pipelines,
    FString& Error);
#endif
