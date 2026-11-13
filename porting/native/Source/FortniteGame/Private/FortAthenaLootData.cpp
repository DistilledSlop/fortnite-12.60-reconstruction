#include "FortAthenaLootData.h"

#include "AFortGameModeAthena.h"
#include "UFortPlaylistAthena.h"
#include "UFortGameData.h"
#include "UFortAssetManager.h"
#include "UFortItemDefinition.h"
#include "Engine/Engine.h"
#include "FFortLootLevelData.h"
#include "FFortLootTierData.h"
#include "FFortLootPackageData.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "UObject/GCObject.h"

// The original server preload/merge implementation is unavailable. Keep each
// feature table separate and expose all matches; callers must resolve merge rules.
namespace
{
    struct FData : FGCObject
    {
        TWeakObjectPtr<UWorld> World;
        TWeakObjectPtr<AFortGameModeAthena> Mode;
        TArray<UDataTable*> Tiers, Packages, ProbeParents;
        TArray<UObject*> Items;
        UDataTable* LevelTable = nullptr;
        TMap<FName, FName> Redirects;
        int32 Level = 0;
        bool bReady = false;
        FDelegateHandle Cleanup;
        FData() { Cleanup = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FData::OnCleanup); }
        ~FData() { FWorldDelegates::OnWorldCleanup.Remove(Cleanup); }
        void OnCleanup(UWorld* W, bool, bool) { if (World.Get() == W) Reset(); }
        void Reset() { bReady = false; World.Reset(); Mode.Reset(); Tiers.Empty(); Packages.Empty(); ProbeParents.Empty(); Items.Empty(); LevelTable = nullptr; Redirects.Empty(); Level = 0; }
        virtual void AddReferencedObjects(FReferenceCollector& Collector) override
        {
            Collector.AddReferencedObjects(Tiers);
            Collector.AddReferencedObjects(Packages);
            Collector.AddReferencedObjects(ProbeParents);
            Collector.AddReferencedObject(LevelTable);
            Collector.AddReferencedObjects(Items);
        }
        virtual FString GetReferencerName() const override { return TEXT("FortAthenaLootData"); }
    };
    FData& Data() { static FData* Value = new FData(); return *Value; }
    bool ForMode(const AFortGameModeAthena* Mode) { return Mode && Data().Mode.Get() == Mode && Data().World.Get() == Mode->GetWorld(); }

    UDataTable* Load(const TSoftObjectPtr<UDataTable>& Ref, const TCHAR* Label, int32& Missing)
    {
        UDataTable* Result = Ref.IsNull() ? nullptr : Ref.LoadSynchronous();
        if (!Result) { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: failed %s path=%s"), Label, *Ref.ToSoftObjectPath().ToString()); }
        return Result;
    }
    bool CheckRow(UDataTable* Table, UScriptStruct* Expected)
    {
        if (!Table) return false;
        const UScriptStruct* Actual = Table->GetRowStruct();
        UE_LOG(LogTemp, Display, TEXT("LootData: table=%s rowstruct=%s rows=%d"), *Table->GetPathName(), *GetPathNameSafe(Actual), Table->GetRowMap().Num());
        if (!Expected || Actual != Expected) { UE_LOG(LogTemp, Warning, TEXT("LootData: rowstruct mismatch expected=%s"), *GetPathNameSafe(Expected)); return false; }
        return true;
    }
}

bool FortAthenaLootData::Initialize(AFortGameModeAthena* Mode, UFortPlaylistAthena* Playlist)
{
    FData& S = Data(); S.Reset();
    if (!Mode || !Mode->GetWorld() || !Playlist) { UE_LOG(LogTemp, Warning, TEXT("LootData: missing mode/world/playlist")); return false; }
    S.Mode = Mode; S.World = Mode->GetWorld(); S.Level = Playlist->LootLevel; S.Redirects = Mode->RedirectAthenaLootTierGroups;
    int32 Missing = 0;
    auto Add = [&Missing](TArray<UDataTable*>& Out, const TSoftObjectPtr<UDataTable>& Ref, const TCHAR* Label, UScriptStruct* Row)
    {
        UDataTable* Table = Load(Ref, Label, Missing);
        if (Table) { Out.Add(Table); if (!CheckRow(Table, Row)) ++Missing; }
    };
    Add(S.Tiers, Playlist->LootTierData, TEXT("playlist tier"), FFortLootTierData::StaticStruct());
    Add(S.Packages, Playlist->LootPackages, TEXT("playlist package"), FFortLootPackageData::StaticStruct());
    for (const auto& Ref : Mode->GameFeatureLootTierData) Add(S.Tiers, Ref, TEXT("feature tier"), FFortLootTierData::StaticStruct());
    for (const auto& Ref : Mode->GameFeatureLootPackageData) Add(S.Packages, Ref, TEXT("feature package"), FFortLootPackageData::StaticStruct());
    // The generated common-data getter logs Fatal when unavailable. Inspect the
    // cached asset instead, so missing data remains a reportable failure.
    UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
    UFortGameData* GameData = Manager ? Manager->GameDataCommon : nullptr;
    if (GameData) S.LevelTable = Load(GameData->LootLevelData, TEXT("game data loot level"), Missing);
    else { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: common game data unavailable")); }
    if (S.LevelTable && !CheckRow(S.LevelTable, FFortLootLevelData::StaticStruct())) ++Missing;
    // Probe the four DefaultSolo composite parents independently; do not add them
    // to selection (the composite already applies parent override precedence).
    if (Playlist->LootTierData.ToSoftObjectPath().GetLongPackageName() == TEXT("/Game/Athena/Playlists/AthenaCompositeLTD"))
    {
        const TCHAR* Paths[] = { TEXT("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"), TEXT("/Game/Athena/Playlists/OverrideLootTierData.OverrideLootTierData"), TEXT("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"), TEXT("/Game/Athena/Playlists/OverrideLootPackagesData.OverrideLootPackagesData") };
        for (int32 I = 0; I < 4; ++I)
        {
            TSoftObjectPtr<UDataTable> Ref{FSoftObjectPath(Paths[I])}; UDataTable* Table = Load(Ref, TEXT("composite parent"), Missing);
            if (Table) { S.ProbeParents.Add(Table); if (!CheckRow(Table, I < 2 ? FFortLootTierData::StaticStruct() : FFortLootPackageData::StaticStruct())) ++Missing; }
        }
    }
    // Conservative reachability check for the five requested groups, including
    // zero-weight rows. Feature merge semantics and other groups remain unresolved.
    const FName Groups[] = { FName(TEXT("Loot_AthenaFloorLoot")), FName(TEXT("Loot_AthenaFloorLoot_Warmup")), FName(TEXT("Loot_AthenaTreasure")), FName(TEXT("Loot_AthenaAmmoLarge")), FName(TEXT("Loot_AthenaAmmoSmall")) };
    TArray<FName> Pending; TSet<FName> Seen;
    for (UDataTable* Table : S.Tiers) if (Table->GetRowStruct() == FFortLootTierData::StaticStruct())
        for (const auto& Pair : Table->GetRowMap())
        {
            const auto* Row = reinterpret_cast<const FFortLootTierData*>(Pair.Value);
            if (Row) for (FName Group : Groups) if (Row->TierGroup == Group)
            {
                if (!FMath::IsFinite(Row->Weight) || Row->Weight < 0.f)
                { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: invalid tier weight row=%s weight=%g"), *Pair.Key.ToString(), Row->Weight); }
                // Keep all calls, including those behind zero-weight tiers, conservative.
                Pending.Add(Row->LootPackage);
            }
        }
    while (Pending.Num())
    {
        FName Id = Pending.Pop(); if (Id.IsNone() || Seen.Contains(Id)) continue; Seen.Add(Id);
        bool Found = false;
        for (UDataTable* Table : S.Packages) if (Table->GetRowStruct() == FFortLootPackageData::StaticStruct())
            for (const auto& Pair : Table->GetRowMap())
            {
                const auto* Row = reinterpret_cast<const FFortLootPackageData*>(Pair.Value);
                if (!Row || Row->LootPackageID != Id) continue;
                Found = true;
                if (!FMath::IsFinite(Row->Weight) || Row->Weight < 0.f)
                { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: invalid package weight row=%s weight=%g"), *Pair.Key.ToString(), Row->Weight); continue; }
                if (!Row->LootPackageCall.IsEmpty() && Row->LootPackageCall != TEXT("None")) Pending.Add(FName(*Row->LootPackageCall));
                if (!Row->ItemDefinition.IsNull())
                {
                    if (Row->Weight == 0.f)
                    {
                        // Named-weight behavior is unrecovered; fail closed when
                        // a zero row names any such modifier rather than assume.
                        if (!Row->NamedWeightMult.IsNone() || Row->PotentialNamedWeights.Num())
                        { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: zero-weight row has unresolved named weight row=%s"), *Pair.Key.ToString()); continue; }
                        UE_LOG(LogTemp, Display, TEXT("LootData: inactive zero-weight item reference row=%s path=%s (not checked)"), *Pair.Key.ToString(), *Row->ItemDefinition.ToSoftObjectPath().ToString());
                        continue;
                    }
                    UObject* Item = Row->ItemDefinition.LoadSynchronous();
                    if (Item) S.Items.AddUnique(Item);
                    else { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: failed item %s"), *Row->ItemDefinition.ToSoftObjectPath().ToString()); }
                }
            }
        if (!Found) { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: missing package id %s"), *Id.ToString()); }
    }
    // DefaultSolo relies on mode config, not playlist properties.
    if (Playlist->LootTierData.ToSoftObjectPath().GetLongPackageName() == TEXT("/Game/Athena/Playlists/AthenaCompositeLTD"))
    {
        const FName* Chest = S.Redirects.Find(FName(TEXT("Loot_Treasure")));
        const FName* Ammo = S.Redirects.Find(FName(TEXT("Loot_Ammo")));
        if (!Chest || *Chest != FName(TEXT("Loot_AthenaTreasure")) || !Ammo || *Ammo != FName(TEXT("Loot_AthenaAmmoLarge")))
        { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: DefaultSolo redirects missing or changed")); }
    }
    if (Mode->GameFeatureLootTierData.Num() || Mode->GameFeatureLootPackageData.Num())
    { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: active feature table merge semantics unsupported")); }
    FFortLootTierData TierDefault; FFortLootPackageData PackageDefault;
    if (TierDefault.Weight != 0 || TierDefault.MinWorldLevel != -1 || TierDefault.MaxWorldLevel != -1 || TierDefault.NumLootPackageDrops != 0 || TierDefault.bAllowBonusLootDrops || PackageDefault.Weight != 0 || PackageDefault.Count != 0 || PackageDefault.LootPackageCategory != -1 || PackageDefault.MinWorldLevel != -1 || PackageDefault.MaxWorldLevel != -1 || PackageDefault.bAllowBonusDrops)
    { ++Missing; UE_LOG(LogTemp, Warning, TEXT("LootData: row defaults mismatch")); }
    for (const auto& Pair : S.Redirects) UE_LOG(LogTemp, Display, TEXT("LootData: redirect %s -> %s"), *Pair.Key.ToString(), *Pair.Value.ToString());
    // Readiness covers direct table, redirect, row and reachable-item loads only.
    // It does not certify transitive staged dependencies or runtime parity.
    S.bReady = Missing == 0;
    UE_LOG(LogTemp, Display, TEXT("LootData: tier=%d package=%d featureTier=%d featurePackage=%d missing=%d"), S.Tiers.Num() ? S.Tiers[0]->GetRowMap().Num() : 0, S.Packages.Num() ? S.Packages[0]->GetRowMap().Num() : 0, Mode->GameFeatureLootTierData.Num(), Mode->GameFeatureLootPackageData.Num(), Missing);
    return S.bReady;
}

bool FortAthenaLootData::IsReady(const AFortGameModeAthena* Mode) { return ForMode(Mode) && Data().bReady; }
bool FortAthenaLootData::ResolveGroup(const AFortGameModeAthena* Mode, FName Input, FName& Output)
{
    if (!IsReady(Mode)) return false;
    Output = Input; TSet<FName> Seen;
    while (const FName* Next = Data().Redirects.Find(Output))
    {
        if (Seen.Contains(Output)) { UE_LOG(LogTemp, Warning, TEXT("LootData: redirect cycle at %s"), *Output.ToString()); return false; }
        Seen.Add(Output); Output = *Next;
    }
    UE_LOG(LogTemp, Display, TEXT("LootData: resolve %s -> %s"), *Input.ToString(), *Output.ToString()); return true;
}
void FortAthenaLootData::FindTiers(const AFortGameModeAthena* Mode, FName Group, TArray<const FFortLootTierData*>& Out)
{
    Out.Reset(); FName Resolved; if (!ResolveGroup(Mode, Group, Resolved)) return;
    for (const UDataTable* Table : Data().Tiers) for (const auto& Pair : Table->GetRowMap()) { const auto* Row = reinterpret_cast<const FFortLootTierData*>(Pair.Value); if (Row && Row->TierGroup == Resolved) Out.Add(Row); }
}
void FortAthenaLootData::FindPackages(const AFortGameModeAthena* Mode, FName Id, TArray<const FFortLootPackageData*>& Out)
{
    Out.Reset(); if (!IsReady(Mode)) return;
    for (const UDataTable* Table : Data().Packages) for (const auto& Pair : Table->GetRowMap()) { const auto* Row = reinterpret_cast<const FFortLootPackageData*>(Pair.Value); if (Row && Row->LootPackageID == Id) Out.Add(Row); }
}
int32 FortAthenaLootData::GetLootLevel(const AFortGameModeAthena* Mode) { return IsReady(Mode) ? Data().Level : -1; }
const UDataTable* FortAthenaLootData::GetLootLevelTable(const AFortGameModeAthena* Mode) { return IsReady(Mode) ? Data().LevelTable : nullptr; }
