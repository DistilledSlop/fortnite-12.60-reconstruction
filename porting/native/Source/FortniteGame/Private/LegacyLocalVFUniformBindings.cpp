#include "LegacyLocalVFUniformBindings.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalUniformBuffer.h"
#include "LegacyOriginalUniformLayout.h"
#include "MeshDrawShaderBindings.h"
#include "RenderingThread.h"

namespace
{
bool SameRHILayout(const FRHIUniformBufferLayout& A, const FRHIUniformBufferLayout& B)
{
    if (A.ConstantBufferSize != B.ConstantBufferSize || A.GetHash() != B.GetHash() ||
        A.Resources.Num() != B.Resources.Num()) return false;
    for (int32 I = 0; I < A.Resources.Num(); ++I)
        if (A.Resources[I].MemberOffset != B.Resources[I].MemberOffset ||
            A.Resources[I].MemberType != B.Resources[I].MemberType) return false;
    return true;
}
}

bool BindLegacyLocalVFUniformBuffers(const FLegacyLocalVFRecordContracts& Contracts,
    const FLegacyMaterialShaderInput& Output, const TShaderRef<FShader>& Shader,
    const TArray<FLegacyLocalVFUniformContents>& Contents,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    FMeshDrawSingleShaderBindings& DrawBindings,
    TArray<FUniformBufferRHIRef>& RetainedBuffers, FString& Error)
{
    auto Fail = [&Error](const TCHAR* Message) { Error = Message; return false; };
    if (RetainedBuffers.Num()) return Fail(TEXT("retained draw buffers must initially be empty"));
    if (GRenderingThread ? !IsInActualRenderingThread() : !IsInGameThread())
        return Fail(TEXT("legacy draw binding requires the rendering thread"));
    if (!Shader.IsValid() || !Shader.GetShaderMap() || !Output.Output.bSucceeded ||
        Output.Output.OutputHash == FSHAHash() || Output.Output.Target.GetPlatform() != SP_VULKAN_SM5 ||
        Shader->GetShaderPlatform() != Output.Output.Target.GetPlatform() ||
        Shader->GetFrequency() != Output.Output.Target.GetFrequency() ||
        !(DrawBindings == FMeshDrawShaderBindingsLayout(Shader)))
        return Fail(TEXT("legacy output, installed shader and draw binding layout differ"));
    FShaderMapResource* Resource = Shader.GetResource();
    const int32 ResourceIndex = Shader->GetResourceIndex();
    if (!Resource || ResourceIndex < 0 || ResourceIndex >= Resource->GetNumShaders())
        return Fail(TEXT("installed legacy shader resource is unavailable"));
    // FShader::GetOutputHash is editor-only data. The inline map resource sets
    // the compiled output hash on its actual RHI shader in every configuration.
    FRHIShader* RHIShader = Resource->GetShader(ResourceIndex);
    if (!RHIShader || RHIShader->GetHash() != Output.Output.OutputHash)
        return Fail(TEXT("installed RHI shader differs from converted output"));
    if (Contents.Num() != Contracts.Uniforms.Num() ||
        Shader->ParameterMapInfo.UniformBuffers.Num() != Contracts.Uniforms.Num())
        return Fail(TEXT("incomplete legacy draw uniform inputs"));
    TArray<FUniformBufferRHIRef> NewBuffers;
    TArray<FShaderUniformBufferParameter> Parameters;
    TSet<uint16> Registers, RemappedIndices;
    TSet<FString> Names;
    for (const auto& Contract : Contracts.Uniforms)
    {
        if (Registers.Contains(Contract.SourceRegister) || Names.Contains(Contract.ParameterName))
            return Fail(TEXT("duplicate legacy draw uniform contract"));
        Registers.Add(Contract.SourceRegister); Names.Add(Contract.ParameterName);
        const FLegacyLocalVFUniformContents* Input = nullptr;
        for (const auto& Candidate : Contents) if (Candidate.SourceRegister == Contract.SourceRegister)
        {
            if (Input) return Fail(TEXT("duplicate legacy draw uniform contents"));
            Input = &Candidate;
        }
        if (!Input) return Fail(TEXT("missing legacy draw uniform contents"));
        uint16 BufferIndex, BaseIndex, Size;
        if (!Output.Output.ParameterMap.FindParameterAllocation(*Contract.ParameterName, BufferIndex, BaseIndex, Size) ||
            Size != 1 || RemappedIndices.Contains(BufferIndex))
            return Fail(TEXT("invalid converted uniform allocation"));
        bool bFoundIndex = false;
        for (const auto& Parameter : Shader->ParameterMapInfo.UniformBuffers)
            if (Parameter.BaseIndex == BufferIndex) bFoundIndex = true;
        if (!bFoundIndex) return Fail(TEXT("converted uniform index is absent from installed shader"));
        RemappedIndices.Add(BufferIndex);
        FShaderUniformBufferParameter Parameter;
        Parameter.Bind(Output.Output.ParameterMap, *Contract.ParameterName);
        if (!Parameter.IsBound() || Parameter.GetBaseIndex() != BufferIndex)
            return Fail(TEXT("converted uniform parameter cannot be bound"));
        FUniformBufferRHIRef Buffer;
        if (Contract.ParameterName == TEXT("Material"))
        {
            if (!Contract.CurrentMetadata || !Input->MaterialBuffer.IsValid() || Input->CurrentContents ||
                Input->CurrentContentsSize || Input->Values.Num() ||
                !SameRHILayout(Input->MaterialBuffer->GetLayout(), Contract.CurrentMetadata->GetLayout()))
                return Fail(TEXT("Material requires the actual expression buffer layout"));
            Buffer = Input->MaterialBuffer;
        }
        else
        {
            if (Input->MaterialBuffer.IsValid()) return Fail(TEXT("fixed uniform cannot use a Material buffer"));
            const FLegacyLocalOriginalUniformTarget* Target = nullptr;
            for (const auto& Candidate : Contracts.OriginalTargets) if (Candidate.SourceRegister == Contract.SourceRegister)
            {
                if (Target) return Fail(TEXT("duplicate original draw uniform target"));
                Target = &Candidate;
            }
            if (!Target || !Target->Layout.IsValid()) return Fail(TEXT("missing original draw uniform target"));
            TSharedPtr<FLegacyOriginalUniformBuffer> Owner;
            if (!CreateLegacyOriginalUniformBuffer(Contract, Input->CurrentContents, Input->CurrentContentsSize,
                Input->Values, Usage, Validation, Owner, Error)) return false;
            if (!SameRHILayout(Owner->GetBuffer()->GetLayout(), Target->Layout->GetMetadata().GetLayout()))
                return Fail(TEXT("reconstructed buffer differs from prepared original target"));
            Buffer = Owner->GetBuffer();
        }
        NewBuffers.Add(MoveTemp(Buffer)); Parameters.Add(Parameter);
    }
    // All fallible checks and creation precede mutation of raw draw bindings.
    RetainedBuffers = MoveTemp(NewBuffers);
    for (int32 I = 0; I < Parameters.Num(); ++I) DrawBindings.Add(Parameters[I], RetainedBuffers[I].GetReference());
    Error.Reset(); return true;
}
#endif
