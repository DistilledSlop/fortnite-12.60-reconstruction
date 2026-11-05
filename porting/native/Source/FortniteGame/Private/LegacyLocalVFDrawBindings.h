#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFUniformBindings.h"
#include "LegacyLocalVFLooseBindings.h"

// Complete recovered constant bindings for one prepared original record. Loose
// values are checked before any uniform binding changes; uniform creation and
// validation finish before either kind of binding is written. On failure both
// DrawBindings and RetainedBuffers are unchanged. RetainedBuffers must initially
// be empty and must outlive the draw command's raw uniform-buffer pointers.
//
// Call on the actual rendering thread after ordinary per-shader bindings. The
// caller supplies producer values/resources and still owns pass integration,
// vertex streams and other mesh resources. No provider is registered here.
bool BindLegacyLocalVFDrawParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader,
    const TArray<FLegacyLocalVFUniformContents>& UniformContents,
    const TArray<FLegacyLocalVFLooseValue>& LooseValues,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    FMeshDrawSingleShaderBindings& DrawBindings,
    TArray<FUniformBufferRHIRef>& RetainedBuffers, FString& Error);
#endif
