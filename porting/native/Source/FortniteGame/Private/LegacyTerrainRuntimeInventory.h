#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalUniformContracts.h"
#include "LegacyLocalVFOutputBatch.h"

// Evidence indexed by decoded-record order, including repeated pipeline stages.
// Uniform entries retain their unresolved static-slot and metadata flags; they
// are not ready for buffer construction. Loose contains only recovered VF names,
// not the complete set of pass-specific allocations.
struct FLegacyTerrainRuntimeInventoryRecord
{
    uint32 ShaderIndex = 0;
    TArray<FLegacyOriginalUniformContractBinding> Uniforms;
    TArray<FLegacyLocalLooseContract> VFLoose;
};

// Authenticate both inputs and associate each decoded occurrence with the
// independently recovered uniform inventory. MapOffset is the selected map's
// offset in the resource used by the manifest. This does not build shader
// outputs or prove static slots, runtime values, or descriptor compatibility.
// Out remains unchanged on failure.
bool BuildLegacyTerrainRuntimeInventory(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ManifestSHA256, const FString& MapSHA256,
    int32 MapIndex, int32 MapOffset, const FString& ContractPath, const FString& ContractSHA256,
    TArray<FLegacyTerrainRuntimeInventoryRecord>& Out, FString& Error);
#endif
