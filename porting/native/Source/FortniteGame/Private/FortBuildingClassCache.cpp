#include "FortBuildingClassCache.h"
#include "AFortGameStateAthena.h"
#include "ABuildingActor.h"
#include "ABuildingSMActor.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

namespace
{
    // Exact bytes of the complete default-scan extraction, not a digest supplied by the JSON.
    // Original AssetRegistry.bin SHA-256: 8cdd9e893b9e043d45702fb285d312476105b448330493cbb872ba3d58fe47b0.
    constexpr TCHAR ManifestSha1[] = TEXT("7DFA4A945E3FCF26CC6F0FD5D7B22F4700BD28B1");
    TWeakObjectPtr<AFortGameStateAthena> CachedState;

    TCHAR FoldAscii(TCHAR C) { return C >= TEXT('A') && C <= TEXT('Z') ? C + (TEXT('a') - TEXT('A')) : C; }
    bool PathLess(const FString& A, const FString& B)
    {
        const int32 N = FMath::Min(A.Len(), B.Len());
        for (int32 I = 0; I < N; ++I)
        {
            const TCHAR X = FoldAscii(A[I]), Y = FoldAscii(B[I]);
            if (X != Y) return X < Y;
        }
        return A.Len() < B.Len();
    }
    bool Fail(const TCHAR* Reason)
    {
        UE_LOG(LogTemp, Error, TEXT("Building cache: %s; no handles enabled"), Reason);
        return false;
    }
}

bool FortBuildingClassCache::Prepare(AFortGameStateAthena* State, bool bOnlyBaseLevel)
{
    CachedState.Reset();
    if (!State) return Fail(TEXT("missing game state"));

    const FString File = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("PlayerBuildingClassRegistry.json"));
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *File)) return Fail(TEXT("missing registry manifest"));
    FSHAHash Digest;
    FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Digest.Hash);
    if (!Digest.ToString().Equals(ManifestSha1, ESearchCase::IgnoreCase))
        return Fail(TEXT("registry manifest digest mismatch"));
    FString Json;
    FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
        return Fail(TEXT("invalid registry manifest"));
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Root->TryGetArrayField(TEXT("rows"), Rows) || Rows->Num() != 331)
        return Fail(TEXT("unexpected registry record count"));

    TArray<UClass*> Classes;
    TSet<FString> Seen;
    for (const TSharedPtr<FJsonValue>& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Row = nullptr;
        FString Path;
        double Level = -1;
        if (!Value.IsValid() || !Value->TryGetObject(Row) || !(*Row)->TryGetStringField(TEXT("classPathCandidate"), Path) ||
            !(*Row)->TryGetNumberField(TEXT("level"), Level) || (Level != 0 && Level != 1 && Level != 2))
            return Fail(TEXT("invalid record"));
        if (!Path.StartsWith(TEXT("/Game/Building/ActorBlueprints/Player/"), ESearchCase::CaseSensitive) ||
            !Path.EndsWith(TEXT("_C"), ESearchCase::CaseSensitive) || Seen.Contains(Path.ToLower()))
            return Fail(TEXT("invalid or duplicate class path"));
        Seen.Add(Path.ToLower());
        // 0x141df9620: when the game state's +0xe17 byte is set,
        // exclude nonzero CurrentBuildingLevel. The signed manifest pins the tags.
        if (bOnlyBaseLevel && Level != 0) continue;
        UClass* Class = StaticLoadClass(ABuildingActor::StaticClass(), nullptr, *Path);
        if (!Class || !Class->IsChildOf(ABuildingActor::StaticClass()) || Class->GetPathName() != Path || !Class->GetDefaultObject())
        {
            UE_LOG(LogTemp, Error, TEXT("Building cache: failed to load expected class %s"), *Path);
            return Fail(TEXT("incomplete or redirected class table"));
        }
        Classes.Add(Class);
    }
    if (Classes.Num() != (bOnlyBaseLevel ? 133 : 331)) return Fail(TEXT("unexpected filtered class count"));
    Classes.Sort([](const UClass& A, const UClass& B) { return PathLess(A.GetPathName(), B.GetPathName()); });
    TMap<TSubclassOf<ABuildingActor>, int32> NewIndex;
    TArray<TSubclassOf<ABuildingSMActor>> ByResource[5];
    for (int32 I = 0; I < Classes.Num(); ++I)
    {
        UClass* Class = Classes[I];
        NewIndex.Add(Class, I);
        if (const ABuildingSMActor* CDO = Cast<ABuildingSMActor>(Class->GetDefaultObject()))
        {
            const int32 Resource = static_cast<int32>(CDO->ResourceType.GetValue());
            if (Resource >= 0 && Resource < 5) ByResource[Resource].Add(Class);
        }
        UE_LOG(LogTemp, Display, TEXT("Building cache candidate %d %s"), I, *Class->GetPathName());
    }
    if (NewIndex.Num() != Classes.Num()) return Fail(TEXT("duplicate loaded class"));
    // Publish only after every expected class resolved and the table is complete.
    State->AllPlayerBuildableClasses.Reset();
    for (UClass* Class : Classes) State->AllPlayerBuildableClasses.Add(Class);
    State->AllPlayerBuildableClassesIndexLookup = MoveTemp(NewIndex);
    for (int32 I = 0; I < 5; ++I) State->PlayerBuildableClasses[I].BuildingClasses = MoveTemp(ByResource[I]);
    CachedState = State;
    UE_LOG(LogTemp, Warning, TEXT("Building cache: %d candidates loaded (base level only %d); original runtime ordering/load parity remains unverified"), Classes.Num(), bOnlyBaseLevel ? 1 : 0);
    return true;
}

UClass* FortBuildingClassCache::Resolve(const AFortGameStateAthena* State, uint32 Handle)
{
    if (CachedState.Get() != State || !State || !State->AllPlayerBuildableClasses.IsValidIndex(Handle)) return nullptr;
    return State->AllPlayerBuildableClasses[Handle].Get();
}
