#include "LegacyLocalVFLooseBindings.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "MeshDrawShaderBindings.h"
#include "RHIResources.h"
#include "RenderingThread.h"

namespace
{
template<int32 N> struct TExactLooseBytes { uint8 Bytes[N]; };

template<int32 N>
void AddExact(FMeshDrawSingleShaderBindings& Bindings, FShaderParameter Parameter, const uint8* Bytes)
{
    static_assert(sizeof(TExactLooseBytes<N>) == N, "loose byte wrapper must be exact size");
    TExactLooseBytes<N> Value;
    FMemory::Memcpy(Value.Bytes, Bytes, N);
    Bindings.Add(Parameter, Value);
}
}

bool PrepareLegacyLocalVFLooseParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader, const TArray<FLegacyLocalVFLooseValue>& Values,
    const FMeshDrawSingleShaderBindings& DrawBindings, FLegacyLocalVFPreparedLooseBindings& Prepared,
    FString& Error)
{
    auto Fail = [&Error](const TCHAR* Message) { Error = Message; return false; };
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
    // The compiled output hash is available on the RHI shader in non-editor builds.
    FRHIShader* RHIShader = Resource->GetShader(ResourceIndex);
    if (!RHIShader || RHIShader->GetHash() != Output.Output.OutputHash)
        return Fail(TEXT("installed RHI shader differs from converted output"));

    TArray<FLegacyLocalLooseContract> Recovered;
    if (!BuildLegacyPaper2DLooseContracts(Record, Recovered, Error)) return false;
    if (Recovered.Num() != Contracts.Loose.Num() || Values.Num() != Recovered.Num())
        return Fail(TEXT("incomplete legacy draw loose inputs"));

    int32 ShaderParameterCount = 0;
    TSet<uint16> ShaderBuffers;
    for (const auto& Buffer : Shader->ParameterMapInfo.LooseParameterBuffers)
    {
        if (!Buffer.Parameters.Num()) return Fail(TEXT("empty installed loose buffer"));
        if (ShaderBuffers.Contains(Buffer.BaseIndex)) return Fail(TEXT("duplicate installed loose buffer"));
        ShaderBuffers.Add(Buffer.BaseIndex);
        uint32 Total = 0;
        TSet<uint16> Offsets;
        for (const auto& Parameter : Buffer.Parameters)
        {
            if (!Parameter.Size || Offsets.Contains(Parameter.BaseIndex))
                return Fail(TEXT("invalid installed loose parameter"));
            Offsets.Add(Parameter.BaseIndex); Total += Parameter.Size; ++ShaderParameterCount;
        }
        if (Total != Buffer.Size) return Fail(TEXT("invalid installed loose buffer size"));
    }
    if (ShaderParameterCount != Recovered.Num()) return Fail(TEXT("installed loose parameter coverage differs"));

    TArray<FLegacyLocalVFPreparedLooseBinding> Pending;
    TSet<FString> Names;
    TSet<uint64> Allocations;
    for (const auto& Contract : Contracts.Loose)
    {
        if (Names.Contains(Contract.ParameterName)) return Fail(TEXT("duplicate legacy loose contract name"));
        Names.Add(Contract.ParameterName);
        int32 RecoveredMatches = 0;
        for (const auto& Original : Recovered)
            if (Original.ParameterName == Contract.ParameterName && Original.SourceBuffer == Contract.SourceBuffer &&
                Original.ByteOffset == Contract.ByteOffset && Original.ByteSize == Contract.ByteSize) ++RecoveredMatches;
        if (RecoveredMatches != 1) return Fail(TEXT("loose contract differs from recovered record"));
        if (Contract.ByteSize != 4 && Contract.ByteSize != 8 && Contract.ByteSize != 12 && Contract.ByteSize != 16)
            return Fail(TEXT("unsupported exact loose parameter size"));
        const FLegacyLocalVFLooseValue* Value = nullptr;
        for (const auto& Candidate : Values) if (Candidate.ParameterName == Contract.ParameterName)
        {
            if (Value) return Fail(TEXT("duplicate legacy loose value"));
            Value = &Candidate;
        }
        if (!Value || Value->Bytes.Num() != Contract.ByteSize)
            return Fail(TEXT("missing or incorrectly sized legacy loose value"));
        uint16 BufferIndex, BaseIndex, Size;
        if (!Output.Output.ParameterMap.FindParameterAllocation(*Contract.ParameterName, BufferIndex, BaseIndex, Size) ||
            BaseIndex != Contract.ByteOffset || Size != Contract.ByteSize)
            return Fail(TEXT("invalid converted loose allocation"));
        const uint64 Key = (uint64(BufferIndex) << 32) | BaseIndex;
        if (Allocations.Contains(Key)) return Fail(TEXT("duplicate converted loose allocation"));
        Allocations.Add(Key);
        int32 Matches = 0;
        for (const auto& Buffer : Shader->ParameterMapInfo.LooseParameterBuffers)
            if (Buffer.BaseIndex == BufferIndex)
                for (const auto& Parameter : Buffer.Parameters)
                    if (Parameter.BaseIndex == BaseIndex && Parameter.Size == Size) ++Matches;
        if (Matches != 1) return Fail(TEXT("converted loose allocation absent from installed shader"));
        FLegacyLocalVFPreparedLooseBinding& Binding = Pending.AddDefaulted_GetRef();
        Binding.Parameter.Bind(Output.Output.ParameterMap, *Contract.ParameterName);
        if (!Binding.Parameter.IsBound() || Binding.Parameter.GetBufferIndex() != BufferIndex ||
            Binding.Parameter.GetBaseIndex() != BaseIndex || Binding.Parameter.GetNumBytes() != Size)
            return Fail(TEXT("converted loose parameter cannot be bound"));
        FMemory::Memcpy(Binding.Bytes, Value->Bytes.GetData(), Size);
    }
    Prepared.Entries = MoveTemp(Pending);
    Error.Reset(); return true;
}

void ApplyLegacyLocalVFLooseParameters(const FLegacyLocalVFPreparedLooseBindings& Prepared,
    FMeshDrawSingleShaderBindings& DrawBindings)
{
    for (const auto& Binding : Prepared.Entries)
    {
        switch (Binding.Parameter.GetNumBytes())
        {
        case 4: AddExact<4>(DrawBindings, Binding.Parameter, Binding.Bytes); break;
        case 8: AddExact<8>(DrawBindings, Binding.Parameter, Binding.Bytes); break;
        case 12: AddExact<12>(DrawBindings, Binding.Parameter, Binding.Bytes); break;
        case 16: AddExact<16>(DrawBindings, Binding.Parameter, Binding.Bytes); break;
        }
    }
}

bool BindLegacyLocalVFLooseParameters(const FLegacyOriginalRecord& Record,
    const FLegacyLocalVFRecordContracts& Contracts, const FLegacyMaterialShaderInput& Output,
    const TShaderRef<FShader>& Shader, const TArray<FLegacyLocalVFLooseValue>& Values,
    FMeshDrawSingleShaderBindings& DrawBindings, FString& Error)
{
    FLegacyLocalVFPreparedLooseBindings Prepared;
    if (!PrepareLegacyLocalVFLooseParameters(Record, Contracts, Output, Shader, Values,
        DrawBindings, Prepared, Error)) return false;
    ApplyLegacyLocalVFLooseParameters(Prepared, DrawBindings);
    return true;
}
#endif
