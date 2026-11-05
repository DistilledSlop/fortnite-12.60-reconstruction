#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFAdapter.h"

// One entry per decoded record, in exactly Material.Records order. Construct
// Material contracts with the metadata passed to FLegacyMaterialPrepareOutputs.
struct FLegacyLocalVFRecordContracts
{
    TArray<FLegacyLocalUniformContract> Uniforms;
    TArray<FLegacyLocalLooseContract> Loose;
    TArray<FLegacyLocalOriginalUniformTarget> OriginalTargets;
};

// Independently recovered original constructor/serializer names for every loose
// allocation in the observed Paper2D inventory. No aliases or values are inferred.
// Unknown nonempty allocation shapes fail; Out is unchanged on failure.
bool BuildLegacyPaper2DLooseContracts(const FLegacyOriginalRecord& Record,
    TArray<FLegacyLocalLooseContract>& Out, FString& Error);

// Call inside FLegacyMaterialPrepareOutputs with its actual Metadata. ManifestPath
// is read and hashed here; ExpectedManifestSHA256 must be independently pinned.
// The owner selects one manifest owner, and its resource digest must match the
// decoded material bytes. All records and stages are retained, including repeats.
// Outputs and Pipelines are unchanged on failure. No provider is installed.
bool BuildLegacyLocalVFOutputBatch(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ExpectedManifestSHA256, const FString& OwnerPath,
    const FShaderParametersMetadata& Metadata, const TArray<FLegacyLocalVFRecordContracts>& Contracts,
    TArray<FLegacyMaterialShaderInput>& Outputs, TArray<FLegacyMaterialPipelineInput>& Pipelines,
    FString& Error);
#endif
