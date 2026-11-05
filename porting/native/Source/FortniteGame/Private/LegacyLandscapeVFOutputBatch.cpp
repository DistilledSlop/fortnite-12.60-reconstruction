#include "LegacyLandscapeVFOutputBatch.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyMaterialManifestSHA256.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Shader.h"

namespace
{
using FObject = TSharedPtr<FJsonObject>;
bool LandscapeBatchDigestMatches(const TArray<uint8>& Bytes, const FString& Expected)
{
    if (Expected.Len() != 64 || !Bytes.Num()) return false;
    uint8 Digest[32]; LegacyManifestSHA256(Bytes.GetData(), uint32(Bytes.Num()), Digest);
    return BytesToHex(Digest, 32).Equals(Expected, ESearchCase::IgnoreCase);
}
bool Int(const FObject& O, const TCHAR* Key, int32& Out)
{
    double N = 0;
    if (!O.IsValid() || !O->TryGetNumberField(Key, N) || !(N >= 0 && N <= MAX_int32) || N != double(int32(N))) return false;
    Out = int32(N); return true;
}
bool Str(const FObject& O, const TCHAR* Key, FString& Out)
{ return O.IsValid() && O->TryGetStringField(Key, Out); }
FObject Obj(const FObject& O, const TCHAR* Key)
{
    const TSharedPtr<FJsonObject>* P = nullptr;
    return O.IsValid() && O->TryGetObjectField(Key, P) && P ? *P : nullptr;
}
bool Arr(const FObject& O, const TCHAR* Key, const TArray<TSharedPtr<FJsonValue>>*& Out)
{ return O.IsValid() && O->TryGetArrayField(Key, Out) && Out; }
bool Name(const FObject& O, const TCHAR* Key, const FString& Expected)
{
    FObject N = Obj(O, Key); FString S; int32 Number;
    return Str(N, TEXT("name"), S) && Int(N, TEXT("number"), Number) && Number == 0 && S == Expected;
}
bool Null(const FObject& O, const TCHAR* Key)
{
    const TSharedPtr<FJsonValue>* V = O.IsValid() ? O->Values.Find(Key) : nullptr;
    return V && V->IsValid() && (*V)->IsNull();
}
bool EqualString(const FObject& O, const TCHAR* Key, const FString& Expected, ESearchCase::Type Case = ESearchCase::CaseSensitive)
{
    FString Value; return Str(O, Key, Value) && Value.Equals(Expected, Case);
}
}

bool BuildLegacyLandscapeVFOutputBatch(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ExpectedManifestSHA256,
    const FString& ExpectedMapSHA256, int32 MapIndex, int32 MapOffset,
    const FShaderParametersMetadata& Metadata, const TArray<FLegacyLocalVFRecordContracts>& Contracts,
    TArray<FLegacyMaterialShaderInput>& Outputs, TArray<FLegacyMaterialPipelineInput>& Pipelines, FString& Error)
{
    auto Fail = [&Error](const FString& Message) { Error = Message; return false; };
    TArray<uint8> Bytes;
    if (MapIndex < 0 || MapOffset < 0 || !LandscapeBatchDigestMatches(Material.Bytes, ExpectedMapSHA256) ||
        ManifestPath.IsEmpty() || !FFileHelper::LoadFileToArray(Bytes, *ManifestPath) ||
        !LandscapeBatchDigestMatches(Bytes, ExpectedManifestSHA256)) return Fail(TEXT("manifest or selected map SHA-256 mismatch"));
    FString Json; FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
    FObject Root; FString Schema;
    const TArray<TSharedPtr<FJsonValue>> *Occurrences = nullptr, *Shaders = nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) ||
        !Str(Root, TEXT("schema"), Schema) || Schema != TEXT("landscape-original-shader-manifest-v1") ||
        !Arr(Root, TEXT("occurrences"), Occurrences) || !Arr(Root, TEXT("shaders"), Shaders) ||
        !Material.Records.Num() || Contracts.Num() != Material.Records.Num()) return Fail(TEXT("invalid landscape manifest or contracts"));
    TMap<int32, FObject> ShaderByIndex;
    for (const auto& Value : *Shaders)
    {
        FObject Shader = Value.IsValid() ? Value->AsObject() : nullptr; int32 Index;
        if (!Int(Shader, TEXT("archive_index"), Index) || ShaderByIndex.Contains(Index)) return Fail(TEXT("duplicate or invalid shader index"));
        ShaderByIndex.Add(Index, Shader);
    }
    TArray<FObject> Selected;
    TSet<int32> Offsets;
    for (const auto& Value : *Occurrences)
    {
        FObject Occ = Value.IsValid() ? Value->AsObject() : nullptr, Location = Obj(Occ, TEXT("location"));
        int32 Map, Offset;
        if (!Int(Location, TEXT("map_index"), Map) || !Int(Location, TEXT("record_offset"), Offset) || Offsets.Contains(Offset))
            return Fail(TEXT("invalid or duplicate occurrence location"));
        Offsets.Add(Offset);
        if (Map == MapIndex) Selected.Add(Occ);
    }
    if (Selected.Num() != Material.Records.Num()) return Fail(TEXT("selected map record count mismatch"));
    TArray<FObject> RecordShaders; RecordShaders.Reserve(Selected.Num());
    TArray<int32> Meshes, PipelineNumbers, StageNumbers;
    TMap<int32, FName> MeshVF; TMap<FName, int32> VFMesh;
    for (int32 I = 0; I < Selected.Num(); ++I)
    {
        const auto& Original = Material.Records[I]; const auto& Contract = Contracts[I];
        FObject Occ = Selected[I], Location = Obj(Occ, TEXT("location"));
        FString Hash, CodeHash, SpvPath, SpvHash; int32 Offset, Index, Frequency, Size, Mesh;
        if (!Int(Location, TEXT("record_offset"), Offset) || int64(Offset) != int64(MapOffset) + Original.Begin ||
            !Int(Location, TEXT("mesh_index"), Mesh) || Original.VertexFactory.IsNone() ||
            !Int(Occ, TEXT("library_index"), Index) || !Int(Occ, TEXT("frequency"), Frequency) ||
            !Int(Occ, TEXT("bytes"), Size) || uint32(Size) != Original.UncompressedBytes ||
            !Str(Occ, TEXT("output_hash"), Hash) || !Hash.Equals(Original.OutputHash.ToString(), ESearchCase::IgnoreCase) ||
            Frequency != int32(Original.Frequency) || !Str(Occ, TEXT("sha256"), CodeHash) ||
            !Str(Occ, TEXT("spirv_path"), SpvPath) || !Str(Occ, TEXT("spirv_sha256"), SpvHash) ||
            !Name(Occ, TEXT("type"), Original.Type) || !Name(Occ, TEXT("pipeline"), Original.Pipeline.ToString()) ||
            !Name(Occ, TEXT("vertex_factory"), Original.VertexFactory.ToString()))
            return Fail(FString::Printf(TEXT("landscape occurrence %d mismatch"), I));
        const FName* ExistingVF = MeshVF.Find(Mesh); const int32* ExistingMesh = VFMesh.Find(Original.VertexFactory);
        if ((ExistingVF && *ExistingVF != Original.VertexFactory) || (ExistingMesh && *ExistingMesh != Mesh))
            return Fail(TEXT("mesh index and vertex factory mismatch"));
        MeshVF.Add(Mesh, Original.VertexFactory); VFMesh.Add(Original.VertexFactory, Mesh);
        const FObject* Shader = ShaderByIndex.Find(Index); int32 ShaderFrequency;
        if (!Shader || !Int(*Shader, TEXT("frequency"), ShaderFrequency) || ShaderFrequency != Frequency ||
            !EqualString(*Shader, TEXT("archive_output_hash"), Hash, ESearchCase::IgnoreCase) ||
            !EqualString(Obj(*Shader, TEXT("source")), TEXT("decoded_sha256"), CodeHash, ESearchCase::IgnoreCase) ||
            !EqualString(Obj(*Shader, TEXT("reflection")), TEXT("output"), SpvPath) ||
            !EqualString(Obj(*Shader, TEXT("reflection")), TEXT("spirv_sha256"), SpvHash, ESearchCase::IgnoreCase))
            return Fail(TEXT("occurrence shader association mismatch"));
        int32 P = INDEX_NONE, S = INDEX_NONE;
        if (Original.Pipeline.IsNone())
        {
            if (!Null(Location, TEXT("pipeline_index")) || !Null(Location, TEXT("stage_index"))) return Fail(TEXT("non-pipeline occurrence location mismatch"));
        }
        else if (!Int(Location, TEXT("pipeline_index"), P) || !Int(Location, TEXT("stage_index"), S))
            return Fail(TEXT("pipeline occurrence location mismatch"));
        for (const auto& Uniform : Contract.Uniforms)
            if (Uniform.ParameterName == TEXT("Material") && Uniform.CurrentMetadata != &Metadata)
                return Fail(TEXT("Material contract does not use factory metadata"));
        Meshes.Add(Mesh); PipelineNumbers.Add(P); StageNumbers.Add(S); RecordShaders.Add(*Shader);
    }
    // Validate the complete mapping before constructing any shader output.
    TArray<FLegacyMaterialPipelineInput> NewPipelines; TSet<int32> SeenStages;
    TMap<int32, int32> NextPipeline;
    for (const auto& Original : Material.Pipelines)
    {
        if (!Original.StageIndices.Num() || !Material.Records.IsValidIndex(Original.StageIndices[0])) return Fail(TEXT("empty or invalid pipeline"));
        const int32 First = Original.StageIndices[0], Mesh = Meshes[First];
        int32& Next = NextPipeline.FindOrAdd(Mesh);
        if (PipelineNumbers[First] != Next++) return Fail(TEXT("pipeline order mismatch"));
        const FShaderPipelineType* Type = FShaderPipelineType::GetShaderPipelineTypeByName(FHashedName(*Original.Type.ToString()));
        if (!Type || Original.Type.ToString() != Type->GetName() || !Type->ShouldOptimizeUnusedOutputs(Material.IR.Platform))
            return Fail(TEXT("pipeline type unavailable or shared-stage linking unsupported"));
        FLegacyMaterialPipelineInput Result; Result.TypeName = Original.Type.ToString(); Result.VertexFactoryName = Original.VertexFactory.ToString();
        for (int32 J = 0; J < Original.StageIndices.Num(); ++J)
        {
            const int32 Index = Original.StageIndices[J];
            if (!Material.Records.IsValidIndex(Index) || SeenStages.Contains(Index) || Meshes[Index] != Mesh ||
                PipelineNumbers[Index] != PipelineNumbers[First] || StageNumbers[Index] != J ||
                Material.Records[Index].Pipeline != Original.Type || Material.Records[Index].VertexFactory != Original.VertexFactory)
                return Fail(TEXT("pipeline stage ownership or order mismatch"));
            SeenStages.Add(Index); Result.StageIndices.Add(Index);
        }
        NewPipelines.Add(MoveTemp(Result));
    }
    for (int32 I = 0; I < Material.Records.Num(); ++I)
        if (!Material.Records[I].Pipeline.IsNone() && !SeenStages.Contains(I)) return Fail(TEXT("unowned pipeline occurrence"));
    TArray<FLegacyMaterialShaderInput> NewOutputs; NewOutputs.Reserve(Selected.Num());
    for (int32 I = 0; I < Selected.Num(); ++I)
    {
        FLegacyMaterialShaderInput Output; const auto& C = Contracts[I];
        if (!BuildLegacyLandscapeVFOutputWithOriginalUniforms(Material.Records[I], RecordShaders[I], C.Uniforms, C.Loose, C.OriginalTargets, Output, Error)) return false;
        if (!Material.Records[I].Pipeline.IsNone()) Output.PipelineName = Material.Records[I].Pipeline.ToString();
        NewOutputs.Add(MoveTemp(Output));
    }
    Outputs = MoveTemp(NewOutputs); Pipelines = MoveTemp(NewPipelines); Error.Reset(); return true;
}
#endif
