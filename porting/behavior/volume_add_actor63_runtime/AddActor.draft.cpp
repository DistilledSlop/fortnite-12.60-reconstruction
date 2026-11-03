// UNCOUNTED RECOVERY DRAFT. Not a compiled implementation or a port claim.
// Native AFortVolume::AddActor, 0x141c5c710; offsets are from the shipping image.
// These declarations mark integration work; no substitute behavior is implied.
#include <cstdint>
struct AFortVolume; struct AActor; struct APawn; struct UActorComponent;
struct UWorld; struct UClass; struct AAthenaNavInvokerBox; struct UObject;
struct FName { int32_t ComparisonIndex, Number; };
struct FWeakActor { int32_t ObjectIndex, ObjectSerialNumber; };
struct FVector { float X,Y,Z; }; struct FQuat { float X,Y,Z,W; };
struct FNativeTag { FName Name; };
struct FEntryEvent { AActor* Actor; UActorComponent* Component; APawn* InstigatorPawn; AFortVolume* Volume; };
struct FTrackingInput { uint64_t Arg0; uint32_t Arg1; }; // original third argument: 12 significant bytes
struct FTrackingEvent { AActor* Actor; uint64_t Arg0; uint32_t Arg1; };
struct FSpawnParams { uint8_t Bytes[48]; };

// TODO: implement these against the native owners and exact ABI, not independent replacement state.
extern bool NativeStatEnabled_1482472e4();
extern void* StatStartOrNull_141c5c710(const wchar_t*); // recover stat prologue if enabled
extern void StatStop_142cd3770(void*);
extern FWeakActor MakeWeak_142ee3b30(AActor*);
extern bool WeakSetContains_140782160(AFortVolume*, FWeakActor); // set +0x318
extern bool IsBuildingSMActor_142752f80(AActor*);
extern bool BuildingSMRejectedFlag_6a9(AActor*); // bit 2
extern void RegisterActorCallbacks_141c64450(AFortVolume*, AActor*, bool);
extern uint8_t LocalRole_0f0(AFortVolume*);
extern void NonAuthorityAdd_141c93b20(AFortVolume*, AActor*, bool);
extern bool WeakSetInsert_1411b9710_1407818b0(AFortVolume*, FWeakActor); // return duplicate flag
extern APawn* CastPawn_1447ae380(AActor*);
extern UActorComponent* ActorComponent_130(AActor*);
extern void BroadcastActorEnter_1406a9dc0(AFortVolume*, const FEntryEvent&); // +0x520
extern void BroadcastTracking_1406a9dc0(AFortVolume*, const FTrackingEvent&); // +0x4c0
extern bool BuildingStatsEnabled_1478c33fc();
extern bool IsBuildingActor_142732ac0(AActor*);
extern uint8_t BuildingCategory_3eb(AActor*);
extern int32_t& BuildingCategoryCount_141c55d00(AFortVolume*, uint8_t); // map +0x368
extern FName MakeName_142cd49c0(const char*, int);
extern FNativeTag NativeGameplayTag_141ed1800_2050();
extern bool VolumeHasTag_250_260(AFortVolume*, FNativeTag);
extern bool ActorHasTag_143f67a70(AActor*, FName); // actor Tags +0x168
extern int32_t& DirectorUsers_4ec(AFortVolume*);
extern int32_t& NavigationUsers_4e8(AFortVolume*);
extern UWorld* GetWorld_Virtual150(AFortVolume*);
extern UObject* WorldGameMode_128(UWorld*);
extern bool IsFortGameModeAthena_1428ee970(UObject*);
extern UObject* GetOrCreateDirector_141e9a660(UObject*); // cached +0x6e0, virtual +0xc68 if null
extern void ClearDirectorFlagBit1_b60(UObject*);
extern UClass* NavInvokerClass_4d8(AFortVolume*);
extern bool IsNavInvokerSubclass_14271f760(UClass*);
extern AAthenaNavInvokerBox*& NavigationInvoker_4e0(AFortVolume*);
// Resolve component +0x130 and its +0x1c0 quaternion/+0x1d0 translation; if quaternion
// differs from cached +0x180 copy it and recompute +0x190 using 0x142c5d000.
// Missing component uses globals 0x148218640/648 rotation and 0x148218630/638 location.
extern void ReadSpawnTransformAndUpdateRotationCache_142c5d000(AFortVolume*, FVector& location, FVector& rotation);
extern FSpawnParams MakeSpawnParams_144680860();
extern AAthenaNavInvokerBox* SpawnNavInvoker_1442e74f0(UWorld*, UClass*, const FVector&, const FVector&, const FSpawnParams&);
extern UObject* Object_2a0(AFortVolume*);
extern void NavAttach_143f840a0(AAthenaNavInvokerBox*, void* componentOffset1c0, int, int, uint64_t);
extern void* AtOffset1c0(UObject*);
extern void NavEnable_141748280(AAthenaNavInvokerBox*, bool); // component +0x220, virtual +0x268 when disabled

void Draft_AddActor_141c5c710(AFortVolume* volume, AActor* actor, const FTrackingInput& input)
{
    void* stat = StatStartOrNull_141c5c710(L"STAT_Volume_AddActor");
    FWeakActor key = actor ? MakeWeak_142ee3b30(actor) : FWeakActor{-1, 0};
    if (!WeakSetContains_140782160(volume, key) &&
        !(actor && IsBuildingSMActor_142752f80(actor) && BuildingSMRejectedFlag_6a9(actor))) {
        RegisterActorCallbacks_141c64450(volume, actor, true);
        if (LocalRole_0f0(volume) != 3) {
            NonAuthorityAdd_141c93b20(volume, actor, true);
        } else {
            key = actor ? MakeWeak_142ee3b30(actor) : FWeakActor{-1, 0};
            bool duplicate = WeakSetInsert_1411b9710_1407818b0(volume, key);
            // The original dereferences actor here even if null; do not add a null guard.
            APawn* pawn = actor ? CastPawn_1447ae380(actor) : nullptr;
            FEntryEvent enter{actor, ActorComponent_130(actor), pawn, volume};
            BroadcastActorEnter_1406a9dc0(volume, enter);
            FTrackingEvent tracking{actor, input.Arg0, input.Arg1};
            BroadcastTracking_1406a9dc0(volume, tracking);
            if (BuildingStatsEnabled_1478c33fc() && !duplicate && IsBuildingActor_142732ac0(actor))
                ++BuildingCategoryCount_141c55d00(volume, BuildingCategory_3eb(actor));
            FName navigation = MakeName_142cd49c0("AthenaAIRequiresNavigation", 1);
            FName director = MakeName_142cd49c0("AthenaAIRequiresAIDirector", 1);
            FNativeTag required = NativeGameplayTag_141ed1800_2050();
            if (VolumeHasTag_250_260(volume, required)) {
                if (ActorHasTag_143f67a70(actor, director)) {
                    if (DirectorUsers_4ec(volume) == 0) {
                        UWorld* world = GetWorld_Virtual150(volume);
                        if (world) {
                            UObject* mode = WorldGameMode_128(world);
                            if (mode && IsFortGameModeAthena_1428ee970(mode)) {
                                UObject* ai = GetOrCreateDirector_141e9a660(mode);
                                if (ai) ClearDirectorFlagBit1_b60(ai);
                            }
                        }
                    }
                    ++DirectorUsers_4ec(volume);
                }
                if (ActorHasTag_143f67a70(actor, navigation)) {
                    ++NavigationUsers_4e8(volume);
                    if (!NavigationInvoker_4e0(volume)) {
                        UWorld* world = GetWorld_Virtual150(volume);
                        if (world && NavInvokerClass_4d8(volume) && IsNavInvokerSubclass_14271f760(NavInvokerClass_4d8(volume))) {
                            FVector location, rotation;
                            ReadSpawnTransformAndUpdateRotationCache_142c5d000(volume, location, rotation);
                            UClass* spawnClass = NavInvokerClass_4d8(volume);
                            if (!spawnClass || !IsNavInvokerSubclass_14271f760(spawnClass)) spawnClass = nullptr;
                            FSpawnParams params = MakeSpawnParams_144680860();
                            auto* nav = SpawnNavInvoker_1442e74f0(world, spawnClass, location, rotation, params);
                            NavigationInvoker_4e0(volume) = nav;
                            if (nav && Object_2a0(volume)) NavAttach_143f840a0(nav, AtOffset1c0(Object_2a0(volume)), 0, 0, reinterpret_cast<uint64_t>(&params) & ~uint64_t(0xff));
                        }
                    }
                    if (NavigationInvoker_4e0(volume)) NavEnable_141748280(NavigationInvoker_4e0(volume), true);
                }
            }
        }
    }
    if (NativeStatEnabled_1482472e4() && stat) StatStop_142cd3770(stat);
}
