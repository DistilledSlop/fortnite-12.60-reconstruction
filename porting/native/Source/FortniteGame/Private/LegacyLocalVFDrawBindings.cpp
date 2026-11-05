#include "LegacyLocalVFDrawBindings.h"

#if PLATFORM_LINUX && !UE_SERVER
bool BindLegacyLocalVFDrawParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader,
    const TArray<FLegacyLocalVFUniformContents>& UniformContents,
    const TArray<FLegacyLocalVFLooseValue>& LooseValues,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    FMeshDrawSingleShaderBindings& DrawBindings,
    TArray<FUniformBufferRHIRef>& RetainedBuffers, FString& Error)
{
    if (Output.TypeName != Record.Type || Output.PermutationId != Record.PermutationId ||
        Output.Output.Target.GetFrequency() != Record.Frequency ||
        Output.VertexFactoryName != (Record.VertexFactory.IsNone() ? FString() : Record.VertexFactory.ToString()) ||
        Output.PipelineName != (Record.Pipeline.IsNone() ? FString() : Record.Pipeline.ToString()))
    {
        Error = TEXT("prepared draw output differs from original record identity");
        return false;
    }
    FLegacyLocalVFPreparedLooseBindings Loose;
    if (!PrepareLegacyLocalVFLooseParameters(Record, Contracts, Output, Shader, LooseValues,
        DrawBindings, Loose, Error)) return false;
    if (!BindLegacyLocalVFUniformBuffers(Contracts, Output, Shader, UniformContents, Usage,
        Validation, DrawBindings, RetainedBuffers, Error)) return false;
    // Preparation owns the bytes and bound parameters. Nothing can reject the
    // loose part after uniform bindings have committed.
    ApplyLegacyLocalVFLooseParameters(Loose, DrawBindings);
    Error.Reset(); return true;
}
#endif
