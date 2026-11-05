#include "LegacyTerrainRuntimeInventory.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyMaterialManifestSHA256.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Shader.h"

namespace
{
using FObj = TSharedPtr<FJsonObject>;
bool TerrainInventoryDigest(const TArray<uint8>& Bytes, const FString& Expected)
{
    if (!Bytes.Num() || Expected.Len() != 64) return false;
    uint8 Hash[32]; LegacyManifestSHA256(Bytes.GetData(), uint32(Bytes.Num()), Hash);
    return BytesToHex(Hash, 32).Equals(Expected, ESearchCase::IgnoreCase);
}
bool TerrainInventoryInt(const FObj& O, const TCHAR* Key, int32& Out)
{
    double N;
    if (!O.IsValid() || !O->TryGetNumberField(Key, N) || !(N >= 0 && N <= MAX_int32) || N != double(int32(N))) return false;
    Out = int32(N); return true;
}
FObj TerrainInventoryObj(const FObj& O, const TCHAR* Key)
{
    const TSharedPtr<FJsonObject>* P = nullptr;
    return O.IsValid() && O->TryGetObjectField(Key, P) && P ? *P : nullptr;
}
bool TerrainInventoryName(const FObj& O, const TCHAR* Key, const FString& Expected)
{
    FObj N = TerrainInventoryObj(O, Key); FString S; int32 Number;
    return N.IsValid() && N->TryGetStringField(TEXT("name"), S) && TerrainInventoryInt(N, TEXT("number"), Number) && Number == 0 && S == Expected;
}
}

bool BuildLegacyTerrainRuntimeInventory(const FLegacyOriginalMaterial& Material,
    const FString& ManifestPath, const FString& ManifestSHA256, const FString& MapSHA256,
    int32 MapIndex, int32 MapOffset, const FString& ContractPath, const FString& ContractSHA256,
    TArray<FLegacyTerrainRuntimeInventoryRecord>& Out, FString& Error)
{
    auto Fail = [&Error](const TCHAR* Message) { Error = Message; return false; };
    TArray<uint8> Bytes;
    if (MapIndex < 0 || MapOffset < 0 || !TerrainInventoryDigest(Material.Bytes, MapSHA256) ||
        !FFileHelper::LoadFileToArray(Bytes, *ManifestPath) || !TerrainInventoryDigest(Bytes, ManifestSHA256))
        return Fail(TEXT("terrain inventory manifest or map digest mismatch"));
    TArray<FLegacyOriginalUniformContractBinding> Bindings;
    if (!LoadLegacyLandscapeOriginalUniformBindings(ContractPath, ContractSHA256, Bindings, Error)) return false;
    FString Json; FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
    FObj Root; FString Schema; const TArray<TSharedPtr<FJsonValue>>* Occurrences = nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid() ||
        !Root->TryGetStringField(TEXT("schema"), Schema) || Schema != TEXT("landscape-original-shader-manifest-v1") ||
        !Root->TryGetArrayField(TEXT("occurrences"), Occurrences) || !Occurrences || !Material.Records.Num())
        return Fail(TEXT("invalid terrain inventory manifest"));
    TArray<FLegacyTerrainRuntimeInventoryRecord> Result;
    TSet<int32> Offsets;
    for (const auto& V : *Occurrences)
    {
        FObj Occ = V.IsValid() ? V->AsObject() : nullptr, Location = TerrainInventoryObj(Occ, TEXT("location"));
        int32 Map, Offset;
        if (!TerrainInventoryInt(Location, TEXT("map_index"), Map) || !TerrainInventoryInt(Location, TEXT("record_offset"), Offset) || Offsets.Contains(Offset))
            return Fail(TEXT("invalid or duplicate terrain occurrence"));
        Offsets.Add(Offset);
        if (Map != MapIndex) continue;
        if (!Material.Records.IsValidIndex(Result.Num())) return Fail(TEXT("too many terrain occurrences"));
        const auto& Original = Material.Records[Result.Num()];
        FString Hash; int32 Index, Frequency, Size;
        if (int64(Offset) != int64(MapOffset) + Original.Begin || !TerrainInventoryInt(Occ, TEXT("library_index"), Index) ||
            !TerrainInventoryInt(Occ, TEXT("frequency"), Frequency) || uint32(Frequency) != Original.Frequency ||
            !TerrainInventoryInt(Occ, TEXT("bytes"), Size) || uint32(Size) != Original.UncompressedBytes ||
            !Occ->TryGetStringField(TEXT("output_hash"), Hash) || !Hash.Equals(Original.OutputHash.ToString(), ESearchCase::IgnoreCase) ||
            !TerrainInventoryName(Occ, TEXT("type"), Original.Type) || !TerrainInventoryName(Occ, TEXT("pipeline"), Original.Pipeline.ToString()) ||
            !TerrainInventoryName(Occ, TEXT("vertex_factory"), Original.VertexFactory.ToString()))
            return Fail(TEXT("terrain occurrence differs from decoded record"));
        auto& Entry = Result.AddDefaulted_GetRef(); Entry.ShaderIndex = uint32(Index);
        for (const auto& Binding : Bindings) if (Binding.ShaderIndex == Entry.ShaderIndex) Entry.Uniforms.Add(Binding);
        if (Original.Frequency == SF_Vertex && !BuildLegacyLandscapeVFLooseContracts(Original, Entry.VFLoose, Error)) return false;
    }
    if (Result.Num() != Material.Records.Num()) return Fail(TEXT("terrain occurrence count mismatch"));
    Out = MoveTemp(Result); Error.Reset(); return true;
}
#endif
