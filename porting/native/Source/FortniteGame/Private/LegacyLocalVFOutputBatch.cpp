#include "LegacyLocalVFOutputBatch.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyMaterialManifestSHA256.h"
#include "LegacyOriginalUniformContracts.h"
#include "LegacyOriginalUniformLayout.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Shader.h"

namespace
{
using FObject = TSharedPtr<FJsonObject>;
bool DigestMatches(const TArray<uint8>& Bytes, const FString& Expected)
{
    if (Expected.Len() != 64) return false;
    uint8 Digest[32]; LegacyManifestSHA256(Bytes.GetData(), uint32(Bytes.Num()), Digest);
    return BytesToHex(Digest, 32).Equals(Expected, ESearchCase::IgnoreCase);
}
bool Integer(const FObject& Object, const TCHAR* Key, int32& Out)
{
    double Number = 0;
    if (!Object.IsValid() || !Object->TryGetNumberField(Key, Number) || !(Number >= 0 && Number <= MAX_int32) || Number != double(int32(Number))) return false;
    Out = int32(Number); return true;
}
bool String(const FObject& Object, const TCHAR* Key, FString& Out)
{
    return Object.IsValid() && Object->TryGetStringField(Key, Out);
}
FObject ObjectField(const FObject& Object, const TCHAR* Key)
{
    const TSharedPtr<FJsonObject>* Value = nullptr;
    return Object.IsValid() && Object->TryGetObjectField(Key, Value) && Value ? *Value : nullptr;
}
bool NameField(const FObject& Object, const TCHAR* Key, const FString& Expected)
{
    FObject Name = ObjectField(Object, Key); FString Text; int32 Number = -1;
    return String(Name, TEXT("name"), Text) && Integer(Name, TEXT("number"), Number) && Number == 0 && Text == Expected;
}
bool ArrayField(const FObject& Object, const TCHAR* Key, const TArray<TSharedPtr<FJsonValue>>*& Out)
{
    return Object.IsValid() && Object->TryGetArrayField(Key, Out) && Out;
}
}

bool BuildLegacyPaper2DLooseContracts(const FLegacyOriginalRecord& Record,
    TArray<FLegacyLocalLooseContract>& Out, FString& Error)
{
    // Source identity is constructor -> FShaderParameter member -> original
    // serializer -> decoded allocation, not current names or matching widths.
    // See behavior/paper2d_loose_contracts_runtime/evidence.json.
    TArray<FLegacyLocalLooseContract> Result;
    const auto Add = [&Result](const TCHAR* Name, uint16 Offset, uint16 Size)
    {
        auto& C = Result.AddDefaulted_GetRef();
        C.ParameterName = Name; C.SourceBuffer = 0; C.ByteOffset = Offset; C.ByteSize = Size;
    };
    if (Record.Type == TEXT("TLightMapDensityPSFNoLightMapPolicy"))
    {
        Add(TEXT("LightMapResolutionScale"), 20, 8);
        Add(TEXT("BuiltLightingAndSelectedFlags"), 32, 12);
        Add(TEXT("LightMapDensityDisplayOptions"), 48, 16);
    }
    else if (Record.Type == TEXT("FHitProxyPS")) Add(TEXT("HitProxyId"), 32, 16);
    else if (Record.Type == TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_Standard>") ||
        Record.Type == TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_PerspectiveCorrect>"))
        Add(TEXT("TranslucentShadowStartOffset"), 20, 4);

    if (Result.Num() == 0)
    {
        if (Record.LooseBuffers.Num())
        { Error = TEXT("unsupported original loose allocation identity"); return false; }
    }
    else
    {
        uint32 Bytes = 0;
        for (const auto& C : Result) Bytes += C.ByteSize;
        if (Record.LooseBuffers.Num() != 1 || Record.LooseBuffers[0].Header.First != 0 ||
            Record.LooseBuffers[0].Header.Second != Bytes || Record.LooseBuffers[0].Parameters.Num() != Result.Num())
        { Error = TEXT("original loose buffer differs from recovered identity"); return false; }
        TSet<int32> Used;
        for (const auto& P : Record.LooseBuffers[0].Parameters)
        {
            int32 Match = INDEX_NONE;
            for (int32 I = 0; I < Result.Num(); ++I)
                if (P.First == Result[I].ByteOffset && P.Second == Result[I].ByteSize) Match = I;
            if (Match == INDEX_NONE || Used.Contains(Match))
            { Error = TEXT("original loose allocation differs from recovered identity"); return false; }
            Used.Add(Match);
        }
    }
    Out = MoveTemp(Result); Error.Reset(); return true;
}

bool PrepareLegacyPaper2DOutputBatch(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ExpectedManifestSHA256, const FString& OwnerPath,
    const FString& ContractPath, const FString& ExpectedContractSHA256,
    const FShaderParametersMetadata& Metadata, TArray<FLegacyLocalVFRecordContracts>& RecordContracts,
    TArray<FLegacyMaterialShaderInput>& Outputs, TArray<FLegacyMaterialPipelineInput>& Pipelines, FString& Error)
{
    auto Fail = [&Error](const TCHAR* Message) { Error = Message; return false; };
    TArray<FLegacyOriginalUniformContractBinding> Bindings;
    if (!LoadLegacyOriginalUniformBindings(ContractPath, ExpectedContractSHA256, Bindings, Error)) return false;
    TArray<uint8> Bytes;
    if (ManifestPath.IsEmpty() || OwnerPath.IsEmpty() || !FFileHelper::LoadFileToArray(Bytes, *ManifestPath) ||
        !Bytes.Num() || !DigestMatches(Bytes, ExpectedManifestSHA256)) return Fail(TEXT("manifest read or pinned SHA-256 mismatch"));
    FString Json; FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
    FObject Root; FString Schema;
    const TArray<TSharedPtr<FJsonValue>>* Owners = nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) ||
        !String(Root, TEXT("schema"), Schema) || Schema != TEXT("paper2d-original-shader-manifest-v1") ||
        !ArrayField(Root, TEXT("owners"), Owners)) return Fail(TEXT("invalid shader manifest"));
    FObject Owner;
    for (const auto& Value : *Owners)
    {
        FObject Candidate = Value.IsValid() ? Value->AsObject() : nullptr; FString Path;
        if (!String(Candidate, TEXT("owner_path"), Path)) return Fail(TEXT("invalid manifest owner"));
        if (Path == OwnerPath) { if (Owner.IsValid()) return Fail(TEXT("duplicate manifest owner")); Owner = Candidate; }
    }
    const TArray<TSharedPtr<FJsonValue>>* Records = nullptr;
    if (!ArrayField(Owner, TEXT("shader_records"), Records) || Records->Num() != Material.Records.Num() || !Records->Num())
        return Fail(TEXT("owner record inventory mismatch"));
    TArray<FLegacyLocalVFRecordContracts> NewContracts;
    NewContracts.Reserve(Records->Num());
    for (int32 I = 0; I < Records->Num(); ++I)
    {
        FObject Occurrence = (*Records)[I].IsValid() ? (*Records)[I]->AsObject() : nullptr; int32 Index;
        if (!Integer(Occurrence, TEXT("archive_index"), Index)) return Fail(TEXT("invalid occurrence archive index"));
        auto& Record = NewContracts.AddDefaulted_GetRef();
        if (!BuildLegacyPaper2DLooseContracts(Material.Records[I], Record.Loose, Error)) return false;
        for (const auto& Binding : Bindings) if (Binding.ShaderIndex == uint32(Index))
        {
            FLegacyLocalUniformContract Contract;
            if (Binding.ParameterName == TEXT("Material"))
            {
                if (!BuildLegacyPaper2DMaterialUniformContract(Material, Binding.SourceRegister,
                    Binding.SourceResourceLayoutHash, Metadata, Contract, Error)) return false;
            }
            else
            {
                Contract = Binding.Contract;
                TSharedPtr<FLegacyOriginalUniformLayout> Layout;
                if (!BuildLegacyOriginalUniformLayout(Contract, Layout, Error)) return false;
                auto& Target = Record.OriginalTargets.AddDefaulted_GetRef();
                Target.SourceRegister = Binding.SourceRegister; Target.Layout = MoveTemp(Layout);
            }
            Record.Uniforms.Add(MoveTemp(Contract));
        }
    }
    TArray<FLegacyMaterialShaderInput> NewOutputs;
    TArray<FLegacyMaterialPipelineInput> NewPipelines;
    if (!BuildLegacyLocalVFOutputBatch(Material, ManifestPath, ExpectedManifestSHA256, OwnerPath,
        Metadata, NewContracts, NewOutputs, NewPipelines, Error)) return false;
    RecordContracts = MoveTemp(NewContracts); Outputs = MoveTemp(NewOutputs); Pipelines = MoveTemp(NewPipelines);
    Error.Reset(); return true;
}

bool BuildLegacyLocalVFOutputBatch(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ExpectedManifestSHA256, const FString& OwnerPath,
    const FShaderParametersMetadata& Metadata, const TArray<FLegacyLocalVFRecordContracts>& Contracts,
    TArray<FLegacyMaterialShaderInput>& Outputs, TArray<FLegacyMaterialPipelineInput>& Pipelines, FString& Error)
{
    auto Fail = [&Error](const FString& Message) { Error = Message; return false; };
    TArray<uint8> Bytes;
    if (ManifestPath.IsEmpty() || OwnerPath.IsEmpty() || !FFileHelper::LoadFileToArray(Bytes, *ManifestPath) ||
        Bytes.Num() == 0 || !DigestMatches(Bytes, ExpectedManifestSHA256)) return Fail(TEXT("manifest read or pinned SHA-256 mismatch"));
    FString Json; FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
    FObject Root; FString Schema;
    const TArray<TSharedPtr<FJsonValue>> *Owners = nullptr, *Shaders = nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) ||
        !String(Root, TEXT("schema"), Schema) || Schema != TEXT("paper2d-original-shader-manifest-v1") ||
        !ArrayField(Root, TEXT("owners"), Owners) || !ArrayField(Root, TEXT("shaders"), Shaders)) return Fail(TEXT("invalid shader manifest"));
    FObject Owner;
    for (const auto& Value : *Owners)
    {
        FObject Candidate = Value.IsValid() ? Value->AsObject() : nullptr; FString Path;
        if (!String(Candidate, TEXT("owner_path"), Path)) return Fail(TEXT("invalid manifest owner"));
        if (Path == OwnerPath) { if (Owner.IsValid()) return Fail(TEXT("duplicate manifest owner")); Owner = Candidate; }
    }
    FString Identity, ResourceHash;
    FObject Framing = ObjectField(Owner, TEXT("framing"));
    const TArray<TSharedPtr<FJsonValue>> *Records = nullptr, *ManifestPipelines = nullptr, *Meshes = nullptr;
    FObject DecodedResource = ObjectField(Owner, TEXT("decoded_resource"));
    if (!String(Owner, TEXT("shader_map_identity"), Identity) || Identity != Material.MaterialPath ||
        !String(Framing, TEXT("resource_sha256"), ResourceHash) || !DigestMatches(Material.Bytes, ResourceHash) ||
        !ArrayField(Owner, TEXT("shader_records"), Records) || !ArrayField(Owner, TEXT("pipelines"), ManifestPipelines) ||
        !ArrayField(DecodedResource, TEXT("mesh_shader_maps"), Meshes) ||
        Records->Num() != Material.Records.Num() || Contracts.Num() != Material.Records.Num() ||
        ManifestPipelines->Num() != Material.Pipelines.Num() || Records->Num() == 0)
        return Fail(TEXT("owner resource or inventory mismatch"));
    TMap<int32, FObject> ShaderByIndex;
    for (const auto& Value : *Shaders)
    {
        FObject Shader = Value.IsValid() ? Value->AsObject() : nullptr; int32 Index;
        if (!Integer(Shader, TEXT("archive_index"), Index) || ShaderByIndex.Contains(Index)) return Fail(TEXT("invalid or duplicate archive index"));
        ShaderByIndex.Add(Index, Shader);
    }
    TMap<FString, int32> RecordIds;
    TArray<FLegacyMaterialShaderInput> NewOutputs; NewOutputs.Reserve(Records->Num());
    for (int32 I = 0; I < Records->Num(); ++I)
    {
        const auto& Original = Material.Records[I]; const auto& Contract = Contracts[I];
        FObject Occurrence = (*Records)[I].IsValid() ? (*Records)[I]->AsObject() : nullptr;
        FObject Decoded = ObjectField(Occurrence, TEXT("decoded_record")), Location = ObjectField(Occurrence, TEXT("location"));
        FString Id, Hash, Kind, RecordHash; int32 Index, Frequency, Offset, End, Permutation, AssociationIndex, AssociationFrequency, DecodedFrequency;
        if (!String(Occurrence, TEXT("id"), Id) || Id.IsEmpty() || RecordIds.Contains(Id) ||
            !Integer(Occurrence, TEXT("archive_index"), Index) || !Integer(Occurrence, TEXT("frequency"), Frequency) ||
            !String(Occurrence, TEXT("archive_output_hash"), Hash) || !String(Location, TEXT("kind"), Kind) ||
            !Integer(Decoded, TEXT("offset"), Offset) || !Integer(Decoded, TEXT("end"), End) ||
            !Integer(Decoded, TEXT("permutation_id"), Permutation) || !Integer(Decoded, TEXT("frequency"), DecodedFrequency) ||
            !Integer(ObjectField(Decoded, TEXT("library_association")), TEXT("index"), AssociationIndex) ||
            !Integer(ObjectField(Decoded, TEXT("library_association")), TEXT("frequency"), AssociationFrequency) ||
            AssociationIndex != Index || AssociationFrequency != Frequency || DecodedFrequency != Frequency || Offset != Original.Begin || End != Original.End ||
            Permutation != Original.PermutationId || Frequency != int32(Original.Frequency) ||
            !Hash.Equals(Original.OutputHash.ToString(), ESearchCase::IgnoreCase) ||
            !String(ObjectField(Decoded, TEXT("output_hash")), TEXT("hash"), RecordHash) || !RecordHash.Equals(Hash, ESearchCase::IgnoreCase) ||
            !NameField(Decoded, TEXT("type"), Original.Type) ||
            !NameField(Decoded, TEXT("vertex_factory"), Original.VertexFactory.ToString()) ||
            !NameField(Decoded, TEXT("pipeline"), Original.Pipeline.ToString()))
            return Fail(FString::Printf(TEXT("original occurrence %d mismatch"), I));
        const FObject* Shader = ShaderByIndex.Find(Index); FString ShaderHash; int32 ShaderFrequency;
        if (!Shader || !String(*Shader, TEXT("archive_output_hash"), ShaderHash) || !ShaderHash.Equals(Hash, ESearchCase::IgnoreCase) ||
            !Integer(*Shader, TEXT("frequency"), ShaderFrequency) || ShaderFrequency != Frequency)
            return Fail(FString::Printf(TEXT("converted shader %d mismatch"), I));
        if ((Original.VertexFactory.IsNone() && Kind != TEXT("material")) ||
            (!Original.VertexFactory.IsNone() && Original.Pipeline.IsNone() && Kind != TEXT("mesh")) ||
            (!Original.Pipeline.IsNone() && Kind != TEXT("pipeline_stage"))) return Fail(TEXT("occurrence location mismatch"));
        if (!Original.VertexFactory.IsNone())
        {
            int32 MeshIndex;
            if (!Integer(Location, TEXT("mesh_index"), MeshIndex) || !Meshes->IsValidIndex(MeshIndex) ||
                !(*Meshes)[MeshIndex].IsValid() || !NameField((*Meshes)[MeshIndex]->AsObject(), TEXT("vertex_factory"), Original.VertexFactory.ToString()))
                return Fail(TEXT("occurrence mesh location mismatch"));
        }
        for (const auto& Uniform : Contract.Uniforms)
            if (Uniform.ParameterName == TEXT("Material") && Uniform.CurrentMetadata != &Metadata)
                return Fail(TEXT("Material contract does not use factory metadata"));
        FLegacyMaterialShaderInput Output;
        if (!BuildLegacyLocalVFOutputWithOriginalUniforms(Original, *Shader, Contract.Uniforms, Contract.Loose, Contract.OriginalTargets, Output, Error)) return false;
        NewOutputs.Add(MoveTemp(Output)); RecordIds.Add(Id, I);
    }
    TArray<FLegacyMaterialPipelineInput> NewPipelines; TSet<int32> SeenStages;
    for (int32 I = 0; I < Material.Pipelines.Num(); ++I)
    {
        const auto& Original = Material.Pipelines[I];
        FObject Entry = (*ManifestPipelines)[I].IsValid() ? (*ManifestPipelines)[I]->AsObject() : nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Stages = nullptr; int32 Offset, MeshIndex, PipelineIndex;
        if (!NameField(Entry, TEXT("type"), Original.Type.ToString()) || !Integer(Entry, TEXT("offset"), Offset) || Offset != Original.Begin ||
            !Integer(Entry, TEXT("mesh_index"), MeshIndex) || !Integer(Entry, TEXT("pipeline_index"), PipelineIndex) ||
            !ArrayField(Entry, TEXT("stage_record_ids"), Stages) || Stages->Num() != Original.StageIndices.Num()) return Fail(TEXT("pipeline inventory mismatch"));
        const FShaderPipelineType* Type = FShaderPipelineType::GetShaderPipelineTypeByName(FHashedName(*Original.Type.ToString()));
        if (!Type || Original.Type.ToString() != Type->GetName()) return Fail(TEXT("pipeline type unavailable"));
        const bool bUnique = Type->ShouldOptimizeUnusedOutputs(Material.IR.Platform);
        // All three observed Paper2D pipelines use unique stages on Vulkan SM5.
        // Shared-stage linking needs a distinct occurrence-to-shader mapping.
        if (!bUnique) return Fail(TEXT("shared pipeline stage linking is not reconstructed"));
        FLegacyMaterialPipelineInput Result; Result.TypeName = Original.Type.ToString();
        Result.VertexFactoryName = Original.VertexFactory.IsNone() ? FString() : Original.VertexFactory.ToString();
        for (int32 J = 0; J < Stages->Num(); ++J)
        {
            FString Id; if (!(*Stages)[J].IsValid() || !(*Stages)[J]->TryGetString(Id)) return Fail(TEXT("invalid pipeline stage ID"));
            const int32* Index = RecordIds.Find(Id);
            if (!Index || *Index != Original.StageIndices[J] || SeenStages.Contains(*Index)) return Fail(TEXT("pipeline stage order or ownership mismatch"));
            FObject Occurrence = (*Records)[*Index]->AsObject(), Location = ObjectField(Occurrence, TEXT("location"));
            int32 M, P, S;
            if (!Integer(Location, TEXT("mesh_index"), M) || !Integer(Location, TEXT("pipeline_index"), P) ||
                !Integer(Location, TEXT("stage_index"), S) || M != MeshIndex || P != PipelineIndex || S != J ||
                Material.Records[*Index].Pipeline != Original.Type || Material.Records[*Index].VertexFactory != Original.VertexFactory)
                return Fail(TEXT("pipeline stage location mismatch"));
            SeenStages.Add(*Index); Result.StageIndices.Add(*Index);
            NewOutputs[*Index].PipelineName = bUnique ? Result.TypeName : FString();
        }
        NewPipelines.Add(MoveTemp(Result));
    }
    for (int32 I = 0; I < Material.Records.Num(); ++I)
        if (!Material.Records[I].Pipeline.IsNone() && !SeenStages.Contains(I)) return Fail(TEXT("unowned pipeline occurrence"));
    Outputs = MoveTemp(NewOutputs); Pipelines = MoveTemp(NewPipelines); Error.Reset(); return true;
}
#endif
