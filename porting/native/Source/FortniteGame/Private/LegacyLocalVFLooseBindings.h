#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFOutputBatch.h"
#include "Shader.h"

class FMeshDrawSingleShaderBindings;

struct FLegacyLocalVFLooseValue
{
    // Exact recovered original parameter name; bytes are supplied by the caller.
    FString ParameterName;
    TArray<uint8> Bytes;
};

// Owned snapshot of validated parameters and bytes. Treat as immutable after
// successful preparation; apply only to the same draw binding layout.
struct FLegacyLocalVFPreparedLooseBinding
{
    FShaderParameter Parameter;
    uint8 Bytes[16];
};
struct FLegacyLocalVFPreparedLooseBindings
{
    TArray<FLegacyLocalVFPreparedLooseBinding> Entries;
};

// Record and Contracts must be the corresponding decoded record and prepared
// contracts for Output. Shader must be Output's installed shader and DrawBindings
// its layout. Call on the rendering thread (or game thread without threaded
// rendering), after ordinary bindings. Every recovered loose allocation requires
// exactly one explicit byte value of its exact size; no values are inferred.
// Validation completes before DrawBindings is changed. This does not install a
// pass/provider or obtain values from one.
// Prepare leaves Prepared and DrawBindings unchanged on failure. Apply performs
// no checks and cannot fail; its binding layout must be the one prepared.
bool PrepareLegacyLocalVFLooseParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader, const TArray<FLegacyLocalVFLooseValue>& Values,
    const FMeshDrawSingleShaderBindings& DrawBindings, FLegacyLocalVFPreparedLooseBindings& Prepared,
    FString& Error);
void ApplyLegacyLocalVFLooseParameters(const FLegacyLocalVFPreparedLooseBindings& Prepared,
    FMeshDrawSingleShaderBindings& DrawBindings);

bool BindLegacyLocalVFLooseParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader, const TArray<FLegacyLocalVFLooseValue>& Values,
    FMeshDrawSingleShaderBindings& DrawBindings, FString& Error);
#endif
