#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFOutputBatch.h"
#include "LegacyOriginalUniformData.h"
#include "RHIResources.h"
#include "Shader.h"

class FMeshDrawSingleShaderBindings;

struct FLegacyLocalVFUniformContents
{
    uint16 SourceRegister = 0;
    // Fixed buffers: readable current registered parameter bytes and complete
    // explicit replacements for original members missing from those bytes.
    const void* CurrentContents = nullptr;
    uint32 CurrentContentsSize = 0;
    TArray<FLegacyOriginalUniformMemberValue> Values;
    // Material only: the factory expression cache's actual RHI buffer. Its
    // values/resources are supplied by the caller, not evaluated by this API.
    FUniformBufferRHIRef MaterialBuffer;
};

// Call after ordinary per-shader bindings, on the actual rendering thread (or
// game thread without threaded rendering). Uses the prepared converted output's
// remapped UB indices, never original D3D registers. Shader must be that output's
// installed shader and DrawBindings must have its layout. All supplied current
// bytes and resource handles must remain valid through creation. No defaults or
// implicit producer values are supplied.
//
// RetainedBuffers must be empty; on success keep it alive until the draw command
// and its raw bindings have been released. The RHI buffers retain original layout
// metadata through deferred deletion. On failure neither RetainedBuffers nor DrawBindings
// is changed, although temporary RHI buffers may have been created and released.
// This binds uniforms only; callers still supply loose parameters and other
// mesh resources and must integrate this API into their pass/provider.
bool BindLegacyLocalVFUniformBuffers(const FLegacyLocalVFRecordContracts& Contracts,
    const FLegacyMaterialShaderInput& Output, const TShaderRef<FShader>& Shader,
    const TArray<FLegacyLocalVFUniformContents>& Contents,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    FMeshDrawSingleShaderBindings& DrawBindings,
    TArray<FUniformBufferRHIRef>& RetainedBuffers, FString& Error);
#endif
