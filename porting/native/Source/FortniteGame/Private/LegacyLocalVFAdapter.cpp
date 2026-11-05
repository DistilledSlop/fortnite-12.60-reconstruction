#include "LegacyLocalVFAdapter.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "Dom/JsonObject.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "LegacyMaterialManifestSHA256.h"
#include "LegacyOriginalUniformLayout.h"
#include "Serialization/MemoryWriter.h"
#include "VulkanCommon.h"
#include "vulkan.h"
#include "VulkanShaderResources.h"

bool LegacyRewriteConvertedSpirv(const FString& Path, const FString& ExpectedSha1, const FString& ExpectedSha256,
    uint32 ExpectedModel, TArray<uint32>& Words, uint32& Crc,
    TMap<uint32, FLegacySpirvDescriptor>& Descriptors, FString& Error, bool bAllowMissingSha1)
{
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *Path) || Bytes.Num() < 20 || Bytes.Num() % 4 || Bytes.Num() > 4 * 1024 * 1024)
    { Error = TEXT("invalid SPIR-V file"); return false; }
    FSHAHash Hash;
    FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Hash.Hash);
    if ((!bAllowMissingSha1 || !ExpectedSha1.IsEmpty()) &&
        (ExpectedSha1.Len() != 40 || !Hash.ToString().Equals(ExpectedSha1, ESearchCase::IgnoreCase)))
    { Error = TEXT("SPIR-V digest mismatch"); return false; }
    uint8 Digest[32]; LegacyManifestSHA256(Bytes.GetData(),uint32(Bytes.Num()),Digest);
    if (ExpectedSha256.Len()!=64 || !BytesToHex(Digest,32).Equals(ExpectedSha256,ESearchCase::IgnoreCase))
    { Error=TEXT("SPIR-V SHA-256 mismatch"); return false; }
    Words.SetNumUninitialized(Bytes.Num() / 4);
    FMemory::Memcpy(Words.GetData(), Bytes.GetData(), Bytes.Num());
    if (Words[0] != 0x07230203 || Words[3] == 0 || Words[3] > 1000000)
    { Error = TEXT("invalid SPIR-V header"); return false; }
    int32 Entry = INDEX_NONE, OldNameWords = 0;
    for (int32 I = 5; I < Words.Num(); )
    {
        const uint32 Count = Words[I] >> 16, Op = Words[I] & 0xffff;
        if (!Count || Count > uint32(Words.Num() - I))
        { Error = TEXT("invalid SPIR-V instruction"); return false; }
        if (Op == 15)
        {
            if (Entry != INDEX_NONE || Count < 4 || Words[I + 1] != ExpectedModel)
            { Error = TEXT("unexpected SPIR-V entry point"); return false; }
            const ANSICHAR* Name = reinterpret_cast<const ANSICHAR*>(&Words[I + 3]);
            const int32 MaxBytes = (Count - 3) * 4;
            int32 Len = 0;
            while (Len < MaxBytes && Name[Len]) ++Len;
            if (Len != 4 || FCStringAnsi::Strncmp(Name, "main", 4) != 0)
            { Error = TEXT("unexpected SPIR-V entry point name"); return false; }
            Entry = I;
            OldNameWords = (Len + 4) / 4;
        }
        I += Count;
    }
    if (Entry == INDEX_NONE) { Error = TEXT("missing SPIR-V entry point"); return false; }
    Crc = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
    constexpr int32 NewNameWords = 6;
    const uint32 NewBytes = (Words.Num() + NewNameWords - OldNameWords) * 4;
    ANSICHAR NewName[24] = {};
    FCStringAnsi::Snprintf(NewName, UE_ARRAY_COUNT(NewName), "main_%08x_%08x", NewBytes, Crc);
    Words.RemoveAt(Entry + 3, OldNameWords);
    Words.InsertZeroed(Entry + 3, NewNameWords);
    FMemory::Memcpy(&Words[Entry + 3], NewName, sizeof(NewName));
    Words[Entry] += (NewNameWords - OldNameWords) << 16;
    Descriptors.Reset();
    for (int32 I = 5; I < Words.Num(); )
    {
        const uint32 Count = Words[I] >> 16, Op = Words[I] & 0xffff;
        if (!Count || Count > uint32(Words.Num() - I))
        { Error = TEXT("invalid renamed SPIR-V instruction"); return false; }
        // OpDecorate: DescriptorSet = 34, Binding = 33.
        if (Op == 71 && Count >= 4 && (Words[I + 2] == 33 || Words[I + 2] == 34))
        {
            const uint32 Id = Words[I + 1];
            if (!Id || Id >= Words[3] || Count != 4)
            { Error = TEXT("invalid descriptor decoration"); return false; }
            FLegacySpirvDescriptor& Descriptor = Descriptors.FindOrAdd(Id);
            uint32& Offset = Words[I + 2] == 33 ? Descriptor.BindingOffset : Descriptor.SetOffset;
            uint32& Value = Words[I + 2] == 33 ? Descriptor.Binding : Descriptor.Set;
            if (Offset != UINT32_MAX) { Error = TEXT("duplicate descriptor decoration"); return false; }
            Offset = I + 3; Value = Words[I + 3];
        }
        I += Count;
    }
    for (const auto& Pair : Descriptors)
        if (Pair.Value.SetOffset == UINT32_MAX || Pair.Value.BindingOffset == UINT32_MAX)
        { Error = TEXT("incomplete descriptor decoration"); return false; }
    return true;
}

namespace
{
bool Fail(FString& Error, const TCHAR* Reason)
{
    Error = Reason;
    return false;
}

bool Number(const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, uint32& Out)
{
    double Value;
    if (!Object || !Object->TryGetNumberField(Key, Value) ||
        !(Value >= 0 && Value <= UINT32_MAX) || Value != uint32(Value)) return false;
    Out = uint32(Value);
    return true;
}

TSharedPtr<FJsonObject> Object(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Key)
{
    const TSharedPtr<FJsonObject>* Value = nullptr;
    return Parent && Parent->TryGetObjectField(Key, Value) ? *Value : nullptr;
}

const TArray<TSharedPtr<FJsonValue>>* Array(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Key)
{
    const TArray<TSharedPtr<FJsonValue>>* Value = nullptr;
    return Parent && Parent->TryGetArrayField(Key, Value) ? Value : nullptr;
}

bool Text(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Key, FString& Out)
{
    return Parent && Parent->TryGetStringField(Key, Out);
}

bool SameMembers(const TArray<FLegacyLocalMemberContract>& Source,
    const FShaderParametersMetadata& Current, int32 Depth)
{
    if (Depth > 16 || Source.Num() != Current.GetMembers().Num()) return false;
    for (int32 I = 0; I < Source.Num(); ++I)
    {
        const auto& S = Source[I];
        const auto& C = Current.GetMembers()[I];
        const auto* Nested = C.GetStructMetadata();
        if (S.Name.IsEmpty() || S.Name != C.GetName() || S.ShaderType != (C.GetShaderType() ? C.GetShaderType() : TEXT("")) ||
            S.Offset != C.GetOffset() || S.BaseType != C.GetBaseType() ||
            S.Precision != C.GetPrecision() || S.Rows != C.GetNumRows() ||
            S.Columns != C.GetNumColumns() || S.Elements != C.GetNumElements()) return false;
        if (Nested)
        {
            if (S.StructTypeName != Nested->GetStructTypeName() || S.StructSize != Nested->GetSize() ||
                !SameMembers(S.Members, *Nested, Depth + 1)) return false;
        }
        else if (!S.StructTypeName.IsEmpty() || S.StructSize || S.Members.Num()) return false;
    }
    return true;
}

bool SameUniform(const FLegacyLocalUniformContract& Contract)
{
    const auto* Metadata = Contract.CurrentMetadata;
    if (!Metadata || Contract.ParameterName.IsEmpty() || Contract.OriginalMembers.Num() == 0 ||
        !Contract.OriginalStaticSlotName.IsEmpty() ||
        !Metadata->GetShaderVariableName() || Contract.ParameterName != Metadata->GetShaderVariableName() ||
        Contract.OriginalStructTypeName != Metadata->GetStructTypeName() ||
        Contract.OriginalStaticSlotName != (Metadata->GetStaticSlotName() ? Metadata->GetStaticSlotName() : TEXT("")) ||
        Contract.OriginalSize != Metadata->GetSize() || !SameMembers(Contract.OriginalMembers, *Metadata, 0)) return false;
    if (Metadata->GetUseCase() == FShaderParametersMetadata::EUseCase::UniformBuffer)
    {
        if (FShaderParametersMetadata::GetNameStructMap().FindRef(Metadata->GetShaderVariableHashedName()) != Metadata) return false;
    }
    else if (Metadata->GetUseCase() != FShaderParametersMetadata::EUseCase::DataDrivenUniformBuffer ||
        Contract.ParameterName != TEXT("Material")) return false;
    const auto& Layout = Metadata->GetLayout();
    if (!Layout.GetHash() || Layout.ConstantBufferSize != Contract.OriginalSize ||
        Layout.Resources.Num() != Contract.OriginalResources.Num()) return false;
    for (int32 I = 0; I < Layout.Resources.Num(); ++I)
        if (Contract.OriginalResources[I].Offset != Layout.Resources[I].MemberOffset ||
            Contract.OriginalResources[I].BaseType != Layout.Resources[I].MemberType) return false;
    return true;
}

bool OriginalUniformTarget(const FLegacyLocalUniformContract& C,
    const FShaderParametersMetadata& Target, TArray<uint16>& ResourceIndices)
{
    const auto* Current = C.CurrentMetadata;
    // The current registered input is a distinct contract from this complete
    // original output ABI. BuildLegacyOriginalUniformData validates its contents.
    if (!Current || C.ParameterName == TEXT("Material") ||
        Current->GetUseCase() != FShaderParametersMetadata::EUseCase::UniformBuffer ||
        !Current->GetShaderVariableName() || C.ParameterName != Current->GetShaderVariableName() ||
        FShaderParametersMetadata::GetNameStructMap().FindRef(Current->GetShaderVariableHashedName()) != Current ||
        Target.GetUseCase() != FShaderParametersMetadata::EUseCase::DataDrivenUniformBuffer ||
        C.ParameterName.IsEmpty() || !Target.GetShaderVariableName() || C.ParameterName != Target.GetShaderVariableName() ||
        C.OriginalStructTypeName != Target.GetStructTypeName() || !C.OriginalStaticSlotName.IsEmpty() || Target.HasStaticSlot() ||
        C.OriginalSize != Target.GetSize() || !SameMembers(C.OriginalMembers, Target, 0)) return false;
    const auto& Layout = Target.GetLayout();
    if (!Layout.GetHash() || Layout.ConstantBufferSize != C.OriginalSize ||
        Layout.Resources.Num() != C.OriginalResources.Num()) return false;
    TSet<uint16> Used;
    for (const auto& Resource : C.OriginalResources)
    {
        int32 Index = INDEX_NONE;
        for (int32 I = 0; I < Layout.Resources.Num(); ++I)
            if (Resource.Offset == Layout.Resources[I].MemberOffset && Resource.BaseType == Layout.Resources[I].MemberType)
            {
                if (Index != INDEX_NONE) return false;
                Index = I;
            }
        if (Index == INDEX_NONE || Index > UINT16_MAX || Used.Contains(uint16(Index))) return false;
        Used.Add(uint16(Index)); ResourceIndices.Add(uint16(Index));
    }
    return true;
}

// Fortnite's original layout predates StaticSlot in the UE4.25 RHI layout
// hash. Hash original fields independently; current headers still use the
// current metadata hash. Only independently identified legacy resource enums
// are accepted here, without assuming later RDG enum values are compatible.
bool OriginalResourceHash(const FLegacyLocalUniformContract& Contract, uint32& Out)
{
    if (!Contract.OriginalStaticSlotName.IsEmpty() || !Contract.OriginalSize || Contract.OriginalSize > 65536) return false;
    TArray<uint32> Types;
    uint32 Hash = Contract.OriginalSize << 16;
    for (const auto& R : Contract.OriginalResources)
    {
        if (R.Offset % 8 || uint32(R.Offset) + 8 > Contract.OriginalSize) return false;
        uint32 LegacyType;
        switch (R.BaseType)
        {
        case UBMT_TEXTURE: LegacyType = 5; break;
        case UBMT_SRV: LegacyType = 6; break;
        case UBMT_UAV: LegacyType = 7; break;
        case UBMT_SAMPLER: LegacyType = 8; break;
        default: return false;
        }
        Types.Add(LegacyType);
        Hash ^= R.Offset;
    }
    int32 N = Types.Num();
    while (N >= 4)
    {
        Hash ^= Types[--N] << 0;
        Hash ^= Types[--N] << 8;
        Hash ^= Types[--N] << 16;
        Hash ^= Types[--N] << 24;
    }
    while (N >= 2)
    {
        Hash ^= Types[--N] << 0;
        Hash ^= Types[--N] << 16;
    }
    while (N > 0) Hash ^= Types[--N];
    Out = Hash;
    return true;
}

bool BoundUniformsCovered(const FLegacyOriginalRecord& R, const TMap<uint32, int32>& Indices,
    const TArray<FLegacyLocalUniformContract>& Contracts)
{
    const auto Covered = [&Indices](const FLegacyOriginalUniformBinding& B)
    { return !B.bBound || Indices.Contains(B.BaseIndex); };
    if (!Covered(R.SceneTextures) || !Covered(R.Material) || !Covered(R.Pass)) return false;
    for (const auto& B : R.CollectionBuffers) if (!Covered(B)) return false;
    for (const auto& B : R.AdditionalUniforms) if (!Covered(B)) return false;
    for (const auto& B : R.UniformParameters)
    {
        if (!B.Binding.bBound) continue;
        const int32* Index = Indices.Find(B.Binding.BaseIndex);
        if (!Index || B.Name.ToString() != Contracts[*Index].OriginalStructTypeName) return false;
    }
    return true;
}

bool SameRegister(const TSharedPtr<FJsonObject>& Reflected, const TSharedPtr<FJsonObject>& Mapped)
{
    uint32 Space, Index, MSpace, MIndex;
    FString Kind, MKind;
    return Text(Reflected, TEXT("kind"), Kind) && Text(Mapped, TEXT("kind"), MKind) && Kind == MKind &&
        Number(Reflected, TEXT("space"), Space) && Number(Mapped, TEXT("register_space"), MSpace) && Space == MSpace &&
        Number(Reflected, TEXT("index"), Index) && Number(Mapped, TEXT("register_index"), MIndex) && Index == MIndex;
}

bool SameTableEntry(const TSharedPtr<FJsonObject>& A, const TSharedPtr<FJsonObject>& B)
{
    uint32 AU, AR, AB, BU, BR, BB;
    FString AT, BT;
    return Number(A, TEXT("uniform_buffer"), AU) && Number(A, TEXT("resource"), AR) && Number(A, TEXT("bind"), AB) &&
        Number(B, TEXT("uniform_buffer"), BU) && Number(B, TEXT("resource"), BR) && Number(B, TEXT("bind"), BB) &&
        Text(A, TEXT("table"), AT) && Text(B, TEXT("table"), BT) && AU == BU && AR == BR && AB == BB && AT == BT;
}
}

bool BuildLegacyLocalVFUniformContract(uint16 SourceRegister, uint32 SourceResourceLayoutHash,
    FLegacyLocalUniformContract& Out, FString& Error)
{
    constexpr uint32 OriginalLocalVFHash = 105252376;
    if (SourceRegister >= 32 || (SourceResourceLayoutHash && SourceResourceLayoutHash != OriginalLocalVFHash))
        return Fail(Error, TEXT("unsupported original LocalVF register or resource hash"));
    FLegacyLocalUniformContract Result;
    Result.SourceRegister = SourceRegister;
    Result.ParameterName = TEXT("LocalVF");
    Result.OriginalStructTypeName = TEXT("FLocalVertexFactoryUniformShaderParameters");
    Result.OriginalSize = 64;
    Result.OriginalResourceLayoutHash = SourceResourceLayoutHash;
    // Original helper chain 1442fe8e0..1442fe7a0, initializer 140546830;
    // material_pipeline/localvf_layout_contract.py independently decodes it.
    const auto Add = [&Result](const TCHAR* Name, uint32 Offset, EUniformBufferBaseType BaseType,
        uint32 Columns, const TCHAR* ShaderType)
    {
        auto& M = Result.OriginalMembers.AddDefaulted_GetRef();
        M.Name = Name; M.Offset = Offset; M.BaseType = BaseType;
        M.Rows = 1; M.Columns = Columns; M.ShaderType = ShaderType;
        if (BaseType == UBMT_SRV)
        {
            auto& R = Result.OriginalResources.AddDefaulted_GetRef();
            R.Offset = Offset; R.BaseType = UBMT_SRV;
        }
    };
    Add(TEXT("VertexFetch_Parameters"), 0, UBMT_INT32, 4, TEXT(""));
    Add(TEXT("PreSkinBaseVertexIndex"), 16, UBMT_INT32, 1, TEXT(""));
    Add(TEXT("LODLightmapDataIndex"), 20, UBMT_UINT32, 1, TEXT(""));
    Add(TEXT("VertexFetch_TexCoordBuffer"), 24, UBMT_SRV, 1, TEXT("Buffer<float2>"));
    Add(TEXT("VertexFetch_PositionBuffer"), 32, UBMT_SRV, 1, TEXT("Buffer<float>"));
    Add(TEXT("VertexFetch_PreSkinPositionBuffer"), 40, UBMT_SRV, 1, TEXT("Buffer<float>"));
    Add(TEXT("VertexFetch_PackedTangentsBuffer"), 48, UBMT_SRV, 1, TEXT("Buffer<float4>"));
    Add(TEXT("VertexFetch_ColorComponentsBuffer"), 56, UBMT_SRV, 1, TEXT("Buffer<float4>"));
    uint32 DerivedHash;
    Result.CurrentMetadata = FShaderParametersMetadata::GetNameStructMap().FindRef(FHashedName(TEXT("LocalVF")));
    if (!OriginalResourceHash(Result, DerivedHash) || DerivedHash != OriginalLocalVFHash || !SameUniform(Result))
        return Fail(Error, TEXT("registered current LocalVF metadata differs from recovered original contract"));
    Out = MoveTemp(Result);
    return true;
}

bool BuildLegacyPaper2DMaterialUniformContract(const FLegacyOriginalMaterial& Material,
    uint16 SourceRegister, uint32 SourceResourceLayoutHash,
    const FShaderParametersMetadata& Metadata, FLegacyLocalUniformContract& Out, FString& Error)
{
    constexpr uint32 OriginalHash = 89655304;
    const auto& IR = Material.IR;
    if (SourceRegister >= 32 || SourceResourceLayoutHash != OriginalHash ||
        Material.MaterialPath != TEXT("/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial") ||
        IR.VectorRoots.Num() != 2 || (IR.ScalarRoots.Num() != 1 && IR.ScalarRoots.Num() != 2) ||
        IR.Texture2DParameters.Num() != 1 || IR.UniformBufferSize != 80 || IR.Resources.Num() != 4 ||
        Metadata.GetUseCase() != FShaderParametersMetadata::EUseCase::DataDrivenUniformBuffer)
        return Fail(Error, TEXT("unsupported original Paper2D Material expression shape"));
    FLegacyLocalUniformContract Result;
    Result.SourceRegister = SourceRegister; Result.ParameterName = TEXT("Material");
    // Original FName global 148336168 is initialized from Material by 140549be0.
    // Member construction is independently captured in 14434c060.
    Result.OriginalStructTypeName = TEXT("Material"); Result.OriginalSize = 80;
    Result.OriginalResourceLayoutHash = SourceResourceLayoutHash; Result.CurrentMetadata = &Metadata;
    const auto Numeric = [&Result](const TCHAR* Name, uint32 Offset, uint32 Count)
    {
        auto& M = Result.OriginalMembers.AddDefaulted_GetRef();
        M.Name = Name; M.Offset = Offset; M.BaseType = UBMT_FLOAT32;
        M.Precision = EShaderPrecisionModifier::Half; M.Rows = 1; M.Columns = 4; M.Elements = Count;
    };
    const auto Resource = [&Result](const TCHAR* Name, const TCHAR* ShaderType, uint16 Offset,
        EUniformBufferBaseType Type)
    {
        auto& M = Result.OriginalMembers.AddDefaulted_GetRef();
        M.Name = Name; M.ShaderType = ShaderType; M.Offset = Offset; M.BaseType = Type;
        M.Rows = M.Columns = 1;
        auto& R = Result.OriginalResources.AddDefaulted_GetRef(); R.Offset = Offset; R.BaseType = Type;
    };
    Numeric(TEXT("VectorExpressions"), 0, 2);
    Numeric(TEXT("ScalarExpressions"), 32, 1);
    Resource(TEXT("Texture2D_0"), TEXT("Texture2D"), 48, UBMT_TEXTURE);
    Resource(TEXT("Texture2D_0Sampler"), TEXT("SamplerState"), 56, UBMT_SAMPLER);
    Resource(TEXT("Wrap_WorldGroupSettings"), TEXT("SamplerState"), 64, UBMT_SAMPLER);
    Resource(TEXT("Clamp_WorldGroupSettings"), TEXT("SamplerState"), 72, UBMT_SAMPLER);
    for (int32 I = 0; I < IR.Resources.Num(); ++I)
        if (IR.Resources[I].Offset != Result.OriginalResources[I].Offset ||
            IR.Resources[I].BaseType != Result.OriginalResources[I].BaseType)
            return Fail(Error, TEXT("original Material IR resource layout differs from recovered contract"));
    uint32 DerivedHash;
    if (!OriginalResourceHash(Result, DerivedHash) || DerivedHash != OriginalHash || !SameUniform(Result))
        return Fail(Error, TEXT("factory Material metadata differs from recovered original contract"));
    Out = MoveTemp(Result);
    return true;
}

static bool BuildLegacyLocalVFOutputInternal(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    const TArray<FLegacyLocalOriginalUniformTarget>& OriginalTargets,
    bool bLandscape, FLegacyMaterialShaderInput& Out, FString& Error)
{
    const bool bSupportedVF = bLandscape
        ? (Original.VertexFactory == FName(TEXT("FLandscapeVertexFactory")) ||
           Original.VertexFactory == FName(TEXT("FLandscapeXYOffsetVertexFactory")) ||
           Original.VertexFactory == FName(TEXT("FLandscapeFixedGridVertexFactory")))
        : (Original.VertexFactory.IsNone() || Original.VertexFactory == FName(TEXT("FLocalVertexFactory")));
    // UE's constructor/bindings expect source records, not a subset with their
    // identity changed. Geometry stages are part of the original LocalVF map.
    if ((Original.Frequency != SF_Vertex && Original.Frequency != SF_Pixel && Original.Frequency != SF_Geometry) ||
        !bSupportedVF || (bLandscape && Original.VFLod.Size) ||
        Original.Type.IsEmpty() || Original.bSpeedTree || Original.Samplers.Num() || Original.SRVs.Num() ||
        Original.BindingWord || Original.RootParameterBufferIndex != UINT16_MAX)
        return Fail(Error, TEXT("unsupported original LocalVF record shape"));

    const auto Reflection = Object(ConvertedShader, TEXT("reflection"));
    const auto Mapped = Object(ConvertedShader, TEXT("mapped"));
    const auto D3D = Object(ConvertedShader, TEXT("d3d"));
    const auto Source = Object(ConvertedShader, TEXT("source"));
    const auto EntryPoint = Object(Reflection, TEXT("entry_point"));
    const auto Maps = Object(D3D, TEXT("resource_maps"));
    const auto SourceResourceBindings = Object(D3D, TEXT("resource_bindings"));
    const auto* Descriptors = Array(Reflection, TEXT("descriptors"));
    const auto* MappedBindings = Array(Mapped, TEXT("bindings"));
    const auto* Unmatched = Array(Mapped, TEXT("unmatched_resource_table_entries"));
    const auto* LayoutHashes = Array(Maps, TEXT("layout_hash"));
    const auto* OptionalData = Array(D3D, TEXT("optional_data"));
    const auto* Interfaces = Array(Reflection, TEXT("interfaces"));
    uint32 Frequency, ArchiveIndex, SourceIndex, MappedIndex, D3DIndex, ReflectedIndex, Model, SourceTableBits;
    FString ArchiveHash, LibraryHash, Path, MappedPath, Digest, MappedDigest, WrapperDigest, SourceDigest, D3DDigest;
    bool bValidated = false;
    if (!Number(ConvertedShader, TEXT("frequency"), Frequency) || Frequency != Original.Frequency ||
        !Number(ConvertedShader, TEXT("archive_index"), ArchiveIndex) ||
        !Number(Source, TEXT("index"), SourceIndex) || SourceIndex != ArchiveIndex ||
        !Number(Mapped, TEXT("shader_index"), MappedIndex) || MappedIndex != ArchiveIndex ||
        !Number(D3D, TEXT("shader_index"), D3DIndex) || D3DIndex != ArchiveIndex ||
        !Number(Reflection, TEXT("shader_index"), ReflectedIndex) || ReflectedIndex != ArchiveIndex ||
        !Text(ConvertedShader, TEXT("archive_output_hash"), ArchiveHash) ||
        !ArchiveHash.Equals(Original.OutputHash.ToString(), ESearchCase::IgnoreCase) ||
        !Text(Source, TEXT("library_output_hash"), LibraryHash) || LibraryHash != ArchiveHash ||
        !Text(Source, TEXT("decoded_sha256"), SourceDigest) ||
        !Text(Mapped, TEXT("wrapper_sha256"), WrapperDigest) || WrapperDigest != SourceDigest ||
        !Text(D3D, TEXT("wrapper_sha256"), D3DDigest) || D3DDigest != SourceDigest ||
        !Mapped || !Mapped->TryGetBoolField(TEXT("validated"), bValidated) || !bValidated ||
        !Text(Reflection, TEXT("output"), Path) || !Text(Mapped, TEXT("output"), MappedPath) || Path != MappedPath ||
        !Text(Reflection, TEXT("spirv_sha256"), Digest) || !Text(Mapped, TEXT("output_sha256"), MappedDigest) || Digest != MappedDigest ||
        !Number(EntryPoint, TEXT("execution_model"), Model) || Model != (Frequency == SF_Vertex ? 0u : Frequency == SF_Pixel ? 4u : 3u) ||
        !Descriptors || !MappedBindings || Descriptors->Num() > UINT16_MAX || UniformContracts.Num() > UINT16_MAX ||
        Descriptors->Num() != MappedBindings->Num() || !Unmatched || Unmatched->Num() ||
        !LayoutHashes || LayoutHashes->Num() > 32 || !Number(D3D, TEXT("resource_table_bits"), SourceTableBits) ||
        !OptionalData || !Interfaces || !SourceResourceBindings)
        return Fail(Error, TEXT("inconsistent original conversion receipts"));
    for (const auto& Receipt : {Source, Mapped, D3D, Reflection})
    {
        uint32 ReceiptFrequency;
        if (!Number(Receipt, TEXT("frequency"), ReceiptFrequency) || ReceiptFrequency != Frequency)
            return Fail(Error, TEXT("conversion receipt frequency mismatch"));
    }

    TArray<FString> SourceUniformNames;
    bool bFoundNames = false;
    for (const auto& Value : *OptionalData)
    {
        const auto Optional = Value->AsObject();
        const auto* Names = Array(Optional, TEXT("uniform_buffer_names"));
        if (!Names) continue;
        if (bFoundNames) return Fail(Error, TEXT("duplicate D3D uniform names receipt"));
        bFoundNames = true;
        for (const auto& Name : *Names)
        {
            FString S;
            if (!Name->TryGetString(S)) return Fail(Error, TEXT("invalid D3D uniform name"));
            SourceUniformNames.Add(S);
        }
    }
    if (!bFoundNames || UniformContracts.Num() != Original.UniformBuffers.Num())
        return Fail(Error, TEXT("incomplete original uniform contracts"));

    TMap<uint32, int32> UniformIndices;
    TSet<FString> ParameterNames;
    TSet<uint32> SourceUniformSlots;
    for (const auto& U : Original.UniformBuffers)
    {
        if (U.First >= 32 || U.Second || SourceUniformSlots.Contains(U.First)) return Fail(Error, TEXT("unsupported original UB allocation"));
        SourceUniformSlots.Add(U.First);
    }
    TMap<uint32, const FShaderParametersMetadata*> ExplicitTargets;
    for (const auto& Target : OriginalTargets)
    {
        if (!Target.Layout.IsValid() || !SourceUniformSlots.Contains(Target.SourceRegister) || ExplicitTargets.Contains(Target.SourceRegister))
            return Fail(Error, TEXT("invalid or duplicate original uniform target"));
        ExplicitTargets.Add(Target.SourceRegister, &Target.Layout->GetMetadata());
    }
    TArray<const FShaderParametersMetadata*> BindingMetadata;
    TArray<TArray<uint16>> BindingResources;
    for (uint32 I = 0; I < 32; ++I)
    {
        uint32 Hash = 0;
        if (LayoutHashes->IsValidIndex(I))
        {
            double Value;
            if (!(*LayoutHashes)[I]->TryGetNumber(Value) || !(Value >= 0 && Value <= UINT32_MAX) || Value != uint32(Value))
                return Fail(Error, TEXT("invalid original resource layout hash"));
            Hash = uint32(Value);
        }
        const bool bActive = !!(SourceTableBits & (1u << I));
        if ((bActive && (!Hash || !SourceUniformSlots.Contains(I))) || (!bActive && Hash))
            return Fail(Error, TEXT("original table bits disagree with resource layout hashes/allocations"));
    }
    for (int32 I = 0; I < UniformContracts.Num(); ++I)
    {
        const auto& C = UniformContracts[I];
        if (!SourceUniformSlots.Contains(C.SourceRegister) || UniformIndices.Contains(C.SourceRegister) ||
            !SourceUniformNames.IsValidIndex(C.SourceRegister) || SourceUniformNames[C.SourceRegister] != C.ParameterName ||
            ParameterNames.Contains(C.ParameterName))
            return Fail(Error, TEXT("source/current uniform member contract mismatch"));
        TArray<uint16> ResourceIndices;
        const auto* const* Explicit = ExplicitTargets.Find(C.SourceRegister);
        const FShaderParametersMetadata* Target = Explicit ? *Explicit : C.CurrentMetadata;
        if (Explicit)
        {
            if (!OriginalUniformTarget(C, *Target, ResourceIndices))
                return Fail(Error, TEXT("complete original uniform target differs from recovered contract"));
        }
        else
        {
            if (!SameUniform(C)) return Fail(Error, TEXT("source/current uniform member contract mismatch"));
            for (int32 J = 0; J < C.OriginalResources.Num(); ++J) ResourceIndices.Add(uint16(J));
        }
        BindingMetadata.Add(Target); BindingResources.Add(MoveTemp(ResourceIndices));
        uint32 SourceLayoutHash = 0;
        if (LayoutHashes->IsValidIndex(C.SourceRegister))
        {
            double Hash;
            if (!(*LayoutHashes)[C.SourceRegister]->TryGetNumber(Hash) || !(Hash >= 0 && Hash <= UINT32_MAX) || Hash != uint32(Hash))
                return Fail(Error, TEXT("invalid original resource layout hash"));
            SourceLayoutHash = uint32(Hash);
        }
        uint32 DerivedOriginalHash;
        const bool bActiveTable = !!(SourceTableBits & (1u << C.SourceRegister));
        if (!OriginalResourceHash(C, DerivedOriginalHash) || C.OriginalResourceLayoutHash != SourceLayoutHash ||
            (bActiveTable && (!C.OriginalResources.Num() || !SourceLayoutHash || SourceLayoutHash != DerivedOriginalHash)) ||
            (!bActiveTable && SourceLayoutHash))
            return Fail(Error, TEXT("original resource layout receipt mismatch"));
        if (C.ParameterName == TEXT("Material"))
        {
            if (!Original.Material.bBound || Original.Material.BaseIndex != C.SourceRegister ||
                C.OriginalSize != Original.LayoutSize || C.OriginalResources.Num() != Original.LayoutOffsets.Num() ||
                Original.LayoutOffsets.Num() != Original.LayoutTypes.Num())
                return Fail(Error, TEXT("material contract disagrees with original serialized debug layout"));
            for (int32 J = 0; J < C.OriginalResources.Num(); ++J)
                if (C.OriginalResources[J].Offset != Original.LayoutOffsets[J] ||
                    C.OriginalResources[J].BaseType != Original.LayoutTypes[J])
                    return Fail(Error, TEXT("material resource contract disagrees with original serialized layout"));
        }
        UniformIndices.Add(C.SourceRegister, I);
        ParameterNames.Add(C.ParameterName);
    }
    if (!BoundUniformsCovered(Original, UniformIndices, UniformContracts))
        return Fail(Error, TEXT("original material/VF/pass uniform binding lacks a matching contract"));
    for (int32 I = 0; I < SourceUniformNames.Num(); ++I)
        if (!SourceUniformNames[I].IsEmpty() && !UniformIndices.Contains(I))
            return Fail(Error, TEXT("named original uniform buffer has no original allocation contract"));

    // Cross-check the mapping/reflection entries against the original D3D table,
    // rather than treating an empty unmatched[] report as sufficient evidence.
    TSet<FString> RemainingTableEntries;
    uint32 SeenSourceTableBits = 0;
    for (const TCHAR* Table : {TEXT("texture"), TEXT("sampler"), TEXT("shader_resource_view"), TEXT("unordered_access_view")})
    {
        const auto* Entries = Array(SourceResourceBindings, Table);
        if (!Entries) return Fail(Error, TEXT("missing original D3D resource table"));
        for (const auto& Value : *Entries)
        {
            const auto E = Value->AsObject();
            uint32 SourceUB, Resource, Bind;
            if (!Number(E, TEXT("uniform_buffer"), SourceUB) || !Number(E, TEXT("resource"), Resource) ||
                !Number(E, TEXT("bind"), Bind) || SourceUB >= 32 || !(SourceTableBits & (1u << SourceUB)) ||
                Resource > UINT16_MAX || Bind > UINT16_MAX)
                return Fail(Error, TEXT("invalid original D3D resource table"));
            SeenSourceTableBits |= 1u << SourceUB;
            const FString Key = FString::Printf(TEXT("%s:%u:%u:%u"), Table, SourceUB, Resource, Bind);
            if (RemainingTableEntries.Contains(Key)) return Fail(Error, TEXT("duplicate original D3D table entry"));
            RemainingTableEntries.Add(Key);
        }
    }
    if (SeenSourceTableBits != SourceTableBits)
        return Fail(Error, TEXT("original table bits do not match actual D3D resource entries"));

    FLegacyMaterialShaderInput Result;
    Result.TypeName = Original.Type;
    Result.PermutationId = Original.PermutationId;
    Result.VertexFactoryName = Original.VertexFactory.IsNone() ? FString() : Original.VertexFactory.ToString();
    Result.PipelineName = Original.Pipeline.IsNone() ? FString() : Original.Pipeline.ToString();
    TArray<uint32> Spirv;
    TMap<uint32, FLegacySpirvDescriptor> Decorations;
    uint32 Crc;
    if (!LegacyRewriteConvertedSpirv(Path, FString(), Digest, Model, Spirv, Crc, Decorations, Error, true)) return false;
    if (Decorations.Num() != Descriptors->Num()) return Fail(Error, TEXT("SPIR-V descriptor count mismatch"));

    FVulkanShaderHeader Header(FVulkanShaderHeader::EZero);
    Header.bHasRealUBs = true;
    Header.SpirvCRC = Crc;
    // Header indices are independent of the D3D CB register. Resource-only UBs
    // still need a header/ParameterMap entry, without a phantom descriptor.
    for (int32 I = 0; I < UniformContracts.Num(); ++I)
    {
        const auto& C = UniformContracts[I];
        auto& U = Header.UniformBuffers.AddDefaulted_GetRef();
        U.LayoutHash = BindingMetadata[I]->GetLayout().GetHash();
        U.ConstantDataOriginalBindingIndex = UINT16_MAX;
        U.bOnlyHasResources = 1;
        Header.UniformBufferSpirvInfos.Emplace(UINT32_MAX, UINT32_MAX);
        Result.Output.ParameterMap.AddParameterAllocation(*C.ParameterName, Header.UniformBuffers.Num() - 1,
            FVulkanShaderHeader::UniformBuffer, 1, EShaderParameterType::UniformBuffer);
    }
    TSet<uint32> SeenIds, SeenBindings, SeenMapped;
    TSet<uint32> ConstantUniforms;
    TSet<uint64> UsedResources;
    TMap<uint32, uint32> PackedIndices, PackedSpans;
    uint32 SamplerCount = 0;
    for (const auto& Value : *Descriptors)
    {
        const auto D = Value->AsObject();
        const auto Register = Object(D, TEXT("d3d_register"));
        const auto* Entries = Array(D, TEXT("resource_table_entries"));
        uint32 Id, Set, Binding, DescriptorType, RegisterIndex, RegisterSpace;
        FString Kind;
        if (!Number(D, TEXT("variable_id"), Id) || !Number(D, TEXT("set"), Set) || !Number(D, TEXT("binding"), Binding) ||
            !Number(D, TEXT("vulkan_descriptor_type"), DescriptorType) || !Number(Register, TEXT("index"), RegisterIndex) ||
            !Number(Register, TEXT("space"), RegisterSpace) || RegisterSpace || RegisterIndex > UINT16_MAX ||
            !Text(Register, TEXT("kind"), Kind) || !Entries || SeenIds.Contains(Id) || SeenBindings.Contains(Binding))
            return Fail(Error, TEXT("invalid reflected descriptor/register"));
        const auto* Decoration = Decorations.Find(Id);
        if (!Decoration || Decoration->Set != Set || Set || Decoration->Binding != Binding || Binding > UINT16_MAX)
            return Fail(Error, TEXT("SPIR-V descriptor receipt mismatch"));
        int32 MatchingMapped = INDEX_NONE;
        for (int32 I = 0; I < MappedBindings->Num(); ++I)
        {
            const auto B = (*MappedBindings)[I]->AsObject();
            uint32 BSet, BBinding;
            if (!Number(B, TEXT("set"), BSet) || !Number(B, TEXT("binding"), BBinding))
                return Fail(Error, TEXT("invalid mapped descriptor receipt"));
            if (BSet != Set || BBinding != Binding) continue;
            const auto* BEntries = Array(B, TEXT("resource_table_entries"));
            uint32 Count;
            if (MatchingMapped != INDEX_NONE || SeenMapped.Contains(I) || !SameRegister(Register, B) ||
                !Number(B, TEXT("count"), Count) || Count != 1 || !BEntries || BEntries->Num() != Entries->Num())
                return Fail(Error, TEXT("ambiguous/unsupported mapped descriptor"));
            for (int32 J = 0; J < Entries->Num(); ++J)
                if (!SameTableEntry((*Entries)[J]->AsObject(), (*BEntries)[J]->AsObject()))
                    return Fail(Error, TEXT("mapped resource table receipt mismatch"));
            MatchingMapped = I;
        }
        if (MatchingMapped == INDEX_NONE) return Fail(Error, TEXT("descriptor has no mapped D3D register"));
        SeenMapped.Add(MatchingMapped); SeenIds.Add(Id); SeenBindings.Add(Binding);
        if (Kind == TEXT("cbv") && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            uint32 Span;
            if (Entries->Num() || !Number(D, TEXT("block_span_bytes"), Span) || !Span || Span % 16 || Span > 65536)
                return Fail(Error, TEXT("unsupported constant-buffer span"));
            const int32* UniformIndex = UniformIndices.Find(RegisterIndex);
            if (UniformIndex)
            {
                const auto& C = UniformContracts[*UniformIndex];
                // Numeric members may follow resource slots in a nested pass
                // struct. The full member proof covers those offsets and holes;
                // the first resource offset is not a general constant-size bound.
                if (Span > C.OriginalSize || ConstantUniforms.Contains(RegisterIndex))
                    return Fail(Error, TEXT("constant descriptor exceeds recovered original uniform members"));
                auto& U = Header.UniformBuffers[*UniformIndex];
                U.ConstantDataOriginalBindingIndex = Binding; U.bOnlyHasResources = 0;
                Header.UniformBufferSpirvInfos[*UniformIndex] = FVulkanShaderHeader::FSpirvInfo(Decoration->SetOffset, Decoration->BindingOffset);
                ConstantUniforms.Add(RegisterIndex);
            }
            else
            {
                bool bHasLooseBuffer = false;
                for (const auto& B : Original.LooseBuffers) if (B.Header.First == RegisterIndex) bHasLooseBuffer = true;
                if (!bHasLooseBuffer || PackedIndices.Contains(RegisterIndex) || Header.PackedUBs.Num() >= 255)
                    return Fail(Error, TEXT("constant descriptor lacks original loose allocation"));
                PackedIndices.Add(RegisterIndex, Header.PackedUBs.Num()); PackedSpans.Add(RegisterIndex, Span);
                auto& P = Header.PackedUBs.AddDefaulted_GetRef();
                P.SizeInBytes = Span; P.OriginalBindingIndex = Binding; P.PackedTypeIndex = CrossCompiler::EPackedTypeIndex::HighP;
                P.SPIRVDescriptorSetOffset = Decoration->SetOffset; P.SPIRVBindingIndexOffset = Decoration->BindingOffset;
            }
        }
        else
        {
            const bool bSampler = Kind == TEXT("sampler") && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
            const bool bTexture = Kind == TEXT("srv") && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            const bool bTexel = Kind == TEXT("srv") && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            if ((!bSampler && !bTexture && !bTexel) || Entries->Num() != 1)
                return Fail(Error, TEXT("unsupported/non-table original resource descriptor"));
            const auto E = (*Entries)[0]->AsObject();
            uint32 SourceUB, ResourceIndex, SourceBinding;
            FString Table;
            if (!Number(E, TEXT("uniform_buffer"), SourceUB) || !Number(E, TEXT("resource"), ResourceIndex) ||
                !Number(E, TEXT("bind"), SourceBinding) || SourceBinding != RegisterIndex || !Text(E, TEXT("table"), Table) ||
                Table != (bSampler ? TEXT("sampler") : bTexture ? TEXT("texture") : TEXT("shader_resource_view")))
                return Fail(Error, TEXT("invalid original table resource"));
            const int32* UBIndex = UniformIndices.Find(SourceUB);
            const uint64 ResourceKey = (uint64(SourceUB) << 32) | ResourceIndex;
            const auto BaseType = bSampler ? UBMT_SAMPLER : bTexture ? UBMT_TEXTURE : UBMT_SRV;
            const FString TableKey = FString::Printf(TEXT("%s:%u:%u:%u"), *Table, SourceUB, ResourceIndex, SourceBinding);
            if (!UBIndex || !RemainingTableEntries.Remove(TableKey) || !UniformContracts[*UBIndex].OriginalResources.IsValidIndex(ResourceIndex) ||
                UniformContracts[*UBIndex].OriginalResources[ResourceIndex].BaseType != BaseType || UsedResources.Contains(ResourceKey))
                return Fail(Error, TEXT("original/current table resource layout mismatch"));
            UsedResources.Add(ResourceKey);
            const uint16 GlobalIndex = Header.Globals.Num();
            auto& G = Header.Globals.AddDefaulted_GetRef();
            G.OriginalBindingIndex = Binding; G.CombinedSamplerStateAliasIndex = UINT16_MAX;
            G.TypeIndex = Header.GlobalDescriptorTypes.Add(static_cast<VkDescriptorType>(DescriptorType));
            Header.GlobalSpirvInfos.Emplace(Decoration->SetOffset, Decoration->BindingOffset);
            auto& R = Header.UniformBuffers[*UBIndex].ResourceEntries.AddDefaulted_GetRef();
            R.SourceUBResourceIndex = BindingResources[*UBIndex][ResourceIndex]; R.OriginalBindingIndex = RegisterIndex;
            R.GlobalIndex = GlobalIndex; R.UBBaseType = BaseType;
            if (bSampler) ++SamplerCount;
        }
    }
    if (RemainingTableEntries.Num()) return Fail(Error, TEXT("original D3D table entries were not consumed"));

    TSet<uint64> LooseAllocations;
    for (const auto& B : Original.LooseBuffers)
    {
        uint32 TotalBytes = 0;
        if (!PackedIndices.Contains(B.Header.First)) return Fail(Error, TEXT("original loose buffer has no converted descriptor"));
        for (int32 I = 0; I < B.Parameters.Num(); ++I)
        {
            const auto& P = B.Parameters[I];
            const uint64 Key = (uint64(B.Header.First) << 32) | (uint32(P.First) << 16) | P.Second;
            if (!P.Second || P.First % 4 || P.Second % 4 || uint32(P.First) + P.Second > PackedSpans[B.Header.First] || LooseAllocations.Contains(Key))
                return Fail(Error, TEXT("invalid/duplicate original loose allocation"));
            LooseAllocations.Add(Key); TotalBytes += P.Second;
            for (int32 J = 0; J < I; ++J)
            {
                const auto& Other = B.Parameters[J];
                if (P.First < uint32(Other.First) + Other.Second && Other.First < uint32(P.First) + P.Second)
                    return Fail(Error, TEXT("overlapping original loose allocations"));
            }
        }
        if (TotalBytes != B.Header.Second) return Fail(Error, TEXT("original loose allocation size sum mismatch"));
    }
    const auto LooseCovered = [&LooseContracts](const FLegacyOriginalParameter& P)
    {
        if (!P.Size) return true;
        for (const auto& C : LooseContracts)
            if (C.SourceBuffer == P.BufferIndex && C.ByteOffset == P.BaseIndex && C.ByteSize == P.Size) return true;
        return false;
    };
    if (bLandscape)
    {
        // These serialized VF slots have independently recovered names. A bound
        // slot must match its own named contract, not merely another allocation
        // with the same extent; an unbound slot must not invent a binding.
        const struct { const TCHAR* Name; const FLegacyOriginalParameter* Parameter; } Slots[] = {
            {TEXT("LodValues"), &Original.LandscapeLodValues},
            {TEXT("ForcedLod"), &Original.LandscapeForcedLod},
            {TEXT("LodTessellationParams"), &Original.LandscapeLodTessellation},
            {TEXT("NeighborSectionLod"), &Original.LandscapeNeighborSectionLod},
            {TEXT("LodBias"), &Original.LandscapeLodBias},
            {TEXT("SectionLods"), &Original.LandscapeSectionLods},
        };
        for (const auto& Slot : Slots)
        {
            int32 Matches = 0;
            for (const auto& C : LooseContracts)
            {
                if (C.ParameterName != Slot.Name) continue;
                ++Matches;
                if (!Slot.Parameter->Size || C.SourceBuffer != Slot.Parameter->BufferIndex ||
                    C.ByteOffset != Slot.Parameter->BaseIndex || C.ByteSize != Slot.Parameter->Size)
                    return Fail(Error, TEXT("landscape VF named loose contract mismatch"));
            }
            if (Matches != (Slot.Parameter->Size ? 1 : 0))
                return Fail(Error, TEXT("missing or duplicate landscape VF named loose contract"));
        }
    }
    if (!LooseCovered(Original.VFLod) || Original.VTFeedback.Size)
        return Fail(Error, TEXT("unsupported/uncovered original VF or VT parameter"));
    for (const auto& P : Original.AdditionalParameters)
        if (!LooseCovered(P)) return Fail(Error, TEXT("original pass parameter lacks named loose allocation"));
    if (LooseContracts.Num() != LooseAllocations.Num()) return Fail(Error, TEXT("incomplete named loose contracts"));
    for (const auto& C : LooseContracts)
    {
        const uint64 Key = (uint64(C.SourceBuffer) << 32) | (uint32(C.ByteOffset) << 16) | C.ByteSize;
        const uint32* PackedIndex = PackedIndices.Find(C.SourceBuffer);
        if (!PackedIndex || C.ParameterName.IsEmpty() || ParameterNames.Contains(C.ParameterName) || !LooseAllocations.Remove(Key))
            return Fail(Error, TEXT("loose parameter name/allocation mismatch"));
        ParameterNames.Add(C.ParameterName);
        Result.Output.ParameterMap.AddParameterAllocation(*C.ParameterName, *PackedIndex, C.ByteOffset, C.ByteSize, EShaderParameterType::LooseData);
        auto& G = Header.PackedGlobals.AddDefaulted_GetRef();
        G.ConstantDataSizeInFloats = C.ByteSize / 4; G.PackedTypeIndex = CrossCompiler::EPackedTypeIndex::HighP; G.PackedUBIndex = *PackedIndex;
    }
    // Resource-only UBs are legitimate, but an entirely unused source allocation
    // cannot be silently retained as a fake binding.
    for (int32 I = 0; I < Header.UniformBuffers.Num(); ++I)
        if (Header.UniformBuffers[I].bOnlyHasResources && !Header.UniformBuffers[I].ResourceEntries.Num())
            return Fail(Error, TEXT("original uniform allocation has no converted constant or resource binding"));
    for (const auto& Value : *Interfaces)
    {
        const auto I = Value->AsObject();
        FString Direction;
        if (!Text(I, TEXT("direction"), Direction)) return Fail(Error, TEXT("invalid converted interface"));
        if (Frequency == SF_Pixel && Direction == TEXT("output") && I->HasField(TEXT("builtin")))
        {
            uint32 Builtin;
            if (!Number(I, TEXT("builtin"), Builtin) || Builtin != 22)
                return Fail(Error, TEXT("unsupported pixel output builtin"));
            // Vulkan's shader header reserves bit 15 for gl_FragDepth.
            Header.InOutMask |= 0x8000;
        }
        const bool bMask = (Frequency == SF_Vertex && Direction == TEXT("input")) || (Frequency == SF_Pixel && Direction == TEXT("output"));
        if (!bMask || !I->HasField(TEXT("location"))) continue;
        uint32 Location;
        if (!Number(I, TEXT("location"), Location) || Location >= (Frequency == SF_Pixel ? 8u : 16u))
            return Fail(Error, TEXT("unsupported interface mask location"));
        Header.InOutMask |= 1u << Location;
    }
    Result.Output.Target = FShaderTarget(EShaderFrequency(Frequency), SP_VULKAN_SM5);
    Result.Output.NumInstructions = Original.InstructionCount;
    Result.Output.NumTextureSamplers = SamplerCount;
    FMemoryWriter Writer(Result.Output.ShaderCode.GetWriteAccess(), true);
    Writer << Header << Spirv;
    if (Writer.IsError()) return Fail(Error, TEXT("cannot serialize converted LocalVF shader"));
    Result.Output.bSucceeded = true;
    Result.Output.GenerateOutputHash();
    Out = MoveTemp(Result);
    return true;
}

bool BuildLegacyLocalVFOutput(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    FLegacyMaterialShaderInput& Out, FString& Error)
{
    const TArray<FLegacyLocalOriginalUniformTarget> Targets;
    return BuildLegacyLocalVFOutputInternal(Original, ConvertedShader, UniformContracts, LooseContracts, Targets, false, Out, Error);
}

bool BuildLegacyLocalVFOutputWithOriginalUniforms(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    const TArray<FLegacyLocalOriginalUniformTarget>& OriginalTargets,
    FLegacyMaterialShaderInput& Out, FString& Error)
{
    return BuildLegacyLocalVFOutputInternal(Original, ConvertedShader, UniformContracts, LooseContracts, OriginalTargets, false, Out, Error);
}
bool BuildLegacyLandscapeVFOutputWithOriginalUniforms(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    const TArray<FLegacyLocalOriginalUniformTarget>& OriginalTargets,
    FLegacyMaterialShaderInput& Out, FString& Error)
{
    return BuildLegacyLocalVFOutputInternal(Original, ConvertedShader, UniformContracts, LooseContracts, OriginalTargets, true, Out, Error);
}
#endif
