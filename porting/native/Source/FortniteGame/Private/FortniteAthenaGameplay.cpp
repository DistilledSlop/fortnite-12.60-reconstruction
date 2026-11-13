#include "FortniteAthenaGameplay.h"
#include "UObject/ConstructorHelpers.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaHealth.h"
#include "FortAthenaBuilding.h"
#include "FortAthenaMatchFlow.h"
#include "AFortTeamInfoAthena.h"
#include "AMegaStormManager.h"
#include "AFortReplaySpectatorAthena.h"
#include "AFortLiveBroadcastController.h"
#include "AFortSpectateAFriendController.h"
#include "UFortServerBotManagerAthena.h"
#include "AFortUIPvP.h"
#include "AFortPlayerStateAthena.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortGameStateAthena.h"
#include "AFortInventory.h"
#include "AFortPlayerState.h"
#include "UFortHeroType.h"
#include "UCustomCharacterPart.h"
#include "FCustomCharacterData.h"
#include "EFortCustomPartType.h"
#include "EFortCustomGender.h"
#include "EFortCustomBodyType.h"
#include "AFortTimeOfDayManager.h"
#include "AFortWorldManager.h"
#include "EAthenaGamePhase.h"
#include "EAthenaGamePhaseStep.h"
#include "AFortPlayerController.h"
#include "EFortInventoryType.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "EngineUtils.h"
#include "Engine/NetDriver.h"
#include "UObject/CoreNet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

AApolloSkyAtmosphere::AApolloSkyAtmosphere(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    // The background meshes sit 1-3 million units away; default haze erases them.
    SkyAtmosphere->AerialPespectiveViewDistanceScale = 0.1f;
    RootComponent = SkyAtmosphere;
}

AApolloSunLight::AApolloSunLight(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(true);
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    RootComponent = SunLight;
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetIntensity(5.0f);
    SunLight->SetCastShadows(false);
    SunLight->bUsedAsAtmosphereSunLight = true;
}

void AApolloSunLight::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Display, TEXT("Apollo sun light ready: authority=%d"), HasAuthority() ? 1 : 0);
}

AFortGameModeAthena::AFortGameModeAthena()
{
    // Existing pawn default retained while original ancestor constructors are recovered.
    DefaultPawnClass = AFortPlayerPawnAthena::StaticClass();
    // Exact class-default stores recovered from original constructor 0x14192dab0.
    GameStateClass = AFortGameStateAthena::StaticClass(); // 0x14192e508, +0x230
    {
        // The client decides whether to keep its loading screen from class defaults that only the Blueprint
        // subclass carries (bShowLoadingScreenUntilAllLevelAreLoaded, volume manager class), and those are not
        // replicated. Spawn the original Athena_GameState_C so the client's own class defaults apply.
        static ConstructorHelpers::FClassFinder<AFortGameStateAthena> BlueprintGameState(TEXT("/Game/Athena/Athena_GameState"));
        if (BlueprintGameState.Succeeded())
        {
            GameStateClass = BlueprintGameState.Class;
        }
    }
    PlayerControllerClass = AFortPlayerControllerAthena::StaticClass(); // 0x14192e514, +0x238
    PlayerStateClass = AFortPlayerStateAthena::StaticClass(); // 0x14192e520, +0x240
    HUDClass = AFortUIPvP::StaticClass(); // 0x14192e52c, +0x248
    ServerBotManagerClass = UFortServerBotManagerAthena::StaticClass(); // 0x14192e538, +0x1040
    SpectateAFriendPlayerControllerClass = AFortSpectateAFriendController::StaticClass(); // 0x14192e544, +0x1058
    LiveBroadcastPlayerControllerClass = AFortLiveBroadcastController::StaticClass(); // 0x14192e550, +0x1060
    ReplaySpectatorPlayerControllerClass = AFortReplaySpectatorAthena::StaticClass(); // 0x14192e55c, +0x260
    MegaStormManagerClass = AMegaStormManager::StaticClass(); // 0x14192e568, +0xfb8
    TeamInfoClass = AFortTeamInfoAthena::StaticClass(); // 0x14192e58b, +0x4e8
}

// Diagnostic for the vanilla-client checksum rejection (WP A3): logs the engine's own class net checksum at every
// ancestor of the given classes, with each class's owned replicated properties and net functions, so the offline
// model (porting/class_checksum.py) can be compared against the real engine. Enabled by -FortniteDumpClassChecksums.
static void DumpChecksumPropertyTree(const FProperty* Property, int32 Depth)
{
    FString Indent;
    for (int32 i = 0; i < Depth; ++i) { Indent += TEXT("    "); }
    UE_LOG(LogTemp, Display, TEXT("FortChecksumDump:   TREE %s%s | %s | dim=%d | off=%d | flags=%llx"), *Indent, *Property->GetName(), *Property->GetCPPType(nullptr, 0), Property->ArrayDim, Property->GetOffset_ForInternal(), (uint64)Property->PropertyFlags);
    if (Depth > 4) { return; }
    if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
    {
        DumpChecksumPropertyTree(Array->Inner, Depth + 1);
    }
    else if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        UE_LOG(LogTemp, Display, TEXT("FortChecksumDump:   TREE %s  struct=%s"), *Indent, *StructProp->Struct->GetName());
        for (TFieldIterator<FProperty> It(StructProp->Struct); It; ++It)
        {
            if (It->PropertyFlags & CPF_RepSkip)
            {
                UE_LOG(LogTemp, Display, TEXT("FortChecksumDump:   TREE %s    (RepSkip) %s | %s | off=%d"), *Indent, *It->GetName(), *It->GetCPPType(nullptr, 0), It->GetOffset_ForInternal());
                continue;
            }
            DumpChecksumPropertyTree(*It, Depth + 2);
        }
    }
}

static void DumpFortniteClassChecksums(UWorld* World)
{
    UNetDriver* Driver = World ? World->GetNetDriver() : nullptr;
    if (!Driver || !Driver->NetCache.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("FortChecksumDump: no net driver or class net cache"));
        return;
    }
    const TCHAR* Roots[] = { TEXT("FortPlayerControllerAthena"), TEXT("FortPlayerPawnAthena"), TEXT("FortGameStateAthena"), TEXT("FortPlayerStateAthena") };
    for (const TCHAR* RootName : Roots)
    {
        UClass* Root = FindObject<UClass>(ANY_PACKAGE, RootName);
        if (!Root)
        {
            UE_LOG(LogTemp, Warning, TEXT("FortChecksumDump: class %s not found"), RootName);
            continue;
        }
        UE_LOG(LogTemp, Display, TEXT("FortChecksumDump: ROOT %s ClassChecksum=%u"), RootName, Driver->NetCache->GetClassNetCache(Root)->GetClassChecksum());
        for (UClass* C = Root; C; C = C->GetSuperClass())
        {
            const uint32 Sum = Driver->NetCache->GetClassNetCache(C)->GetClassChecksum();
            int32 OwnedReps = 0;
            for (TFieldIterator<FProperty> It(C, EFieldIteratorFlags::ExcludeSuper); It; ++It)
            {
                if (It->PropertyFlags & CPF_Net) { ++OwnedReps; }
            }
            UE_LOG(LogTemp, Display, TEXT("FortChecksumDump: CLASS %s %s checksum=%u ownedReps=%d"), RootName, *C->GetPathName(), Sum, OwnedReps);
            for (TFieldIterator<FProperty> It(C, EFieldIteratorFlags::ExcludeSuper); It; ++It)
            {
                if (It->PropertyFlags & CPF_Net)
                {
                    UE_LOG(LogTemp, Display, TEXT("FortChecksumDump:   REP %s | %s | dim=%d | off=%d | flags=%llx | own=%u | full=%u"), *It->GetName(), *It->GetCPPType(nullptr, 0), It->ArrayDim, It->GetOffset_ForInternal(), (uint64)It->PropertyFlags, Driver->NetCache->GetPropertyChecksum(*It, 0, false), Driver->NetCache->GetPropertyChecksum(*It, 0, true));
                    FString TreeProps;
                    FParse::Value(FCommandLine::Get(), TEXT("FortniteDumpChecksumProps="), TreeProps);
                    TArray<FString> TreeNames;
                    TreeProps.ParseIntoArray(TreeNames, TEXT(","));
                    if (TreeNames.Contains(It->GetName()))
                    {
                        DumpChecksumPropertyTree(*It, 0);
                    }
                }
            }
            for (TFieldIterator<UFunction> It(C, EFieldIteratorFlags::ExcludeSuper); It; ++It)
            {
                if (It->FunctionFlags & FUNC_Net)
                {
                    UE_LOG(LogTemp, Display, TEXT("FortChecksumDump:   FUNC %s | flags=%x"), *It->GetName(), (uint32)It->FunctionFlags);
                }
            }
        }
    }
    UE_LOG(LogTemp, Display, TEXT("FortChecksumDump: done"));
}

void AFortGameModeAthena::StartPlay()
{
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
    {
        if (UClass* BpControllerClass = LoadClass<APlayerController>(nullptr, TEXT("/Game/Athena/Athena_PlayerController.Athena_PlayerController_C")))
        {
            PlayerControllerClass = BpControllerClass;
            UE_LOG(LogTemp, Display, TEXT("OriginalClient: using PlayerControllerClass=%s"), *PlayerControllerClass->GetPathName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OriginalClient: failed to load /Game/Athena/Athena_PlayerController.Athena_PlayerController_C"));
        }
        // The original spawns the PlayerPawn_Athena Blueprint; its CharacterMesh0 carries AnimClass
        // Fortnite_M_Avg_Player_AnimBlueprint. With the native class the vanilla client built a pawn without an anim
        // blueprint and the character stood in its reference pose (runs 27-33).
        // Opt-in (-FortniteBlueprintPawn): spawning it crashed the server in SpawnDefaultPawnFor (run 20261113-pawnbp-1,
        // SIGSEGV, null read), most likely because the Blueprint expects native default subobjects our constructor-less
        // generated class does not create. See porting/SERVER_FIRST_PLAN.md, WP K2.
        if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteBlueprintPawn")))
        {
        }
        else if (UClass* BpPawnClass = LoadClass<APawn>(nullptr, TEXT("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C")))
        {
            DefaultPawnClass = BpPawnClass;
            UE_LOG(LogTemp, Display, TEXT("OriginalClient: using DefaultPawnClass=%s"), *DefaultPawnClass->GetPathName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("OriginalClient: failed to load /Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C; the pawn will have no anim blueprint"));
        }
    }
    Super::StartPlay();
    FortAthenaInventory::InstallRpcOverrides();
    FortAthenaBuilding::InstallRpcOverrides();
    FortAthenaBuilding::PrepareBuildableClasses(GetGameState<AFortGameStateAthena>());
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteNoHealth")))
        FortAthenaHealth::InstallRpcOverrides();
    FortAthenaMatchFlow::InstallRpcOverrides();
    FortAthenaMatchFlow::Start(this);
    // Debug aid (-FortniteSpawnTestPawn): spawn the PlayerPawn_Athena Blueprint once at start, so its spawn crash can be
    // reproduced under a debugger without a client.
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteSpawnTestPawn")))
    {
        if (UClass* TestPawnClass = LoadClass<APawn>(nullptr, TEXT("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C")))
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            UE_LOG(LogTemp, Display, TEXT("TestPawn: spawning %s"), *TestPawnClass->GetPathName());
            APawn* TestPawn = GetWorld()->SpawnActor<APawn>(TestPawnClass, FTransform(FVector(0.f, 0.f, 100000.f)), Params);
            UE_LOG(LogTemp, Display, TEXT("TestPawn: spawned %s"), TestPawn ? *TestPawn->GetName() : TEXT("null"));
        }
    }
    if (UWorld* World = GetWorld())
    {
        // Run evidence for gate G6: where each possessed pawn is, so a held input key can be shown to move it.
        static FTimerHandle PawnLogHandle;
        World->GetTimerManager().SetTimer(PawnLogHandle, FTimerDelegate::CreateLambda([World]()
        {
            for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
            {
                if (const APawn* Pawn = It->IsValid() ? It->Get()->GetPawn() : nullptr)
                {
                    const FVector Location = Pawn->GetActorLocation();
                    UE_LOG(LogTemp, Display, TEXT("Pawn tracker: %s at %.1f %.1f %.1f velocity %.1f"), *Pawn->GetName(), Location.X, Location.Y, Location.Z, Pawn->GetVelocity().Size());
                }
            }
        }), 5.0f, true);
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteDumpClassChecksums")))
    {
        DumpFortniteClassChecksums(GetWorld());
    }
    if (!GetWorld() || !GetWorld()->GetMapName().EndsWith(TEXT("Apollo_Terrain")))
        return;
    // Invented Apollo stand-in actors are not part of the original protocol. The
    // original client path skips them so no unknown class reaches that client.
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
    {
        UE_LOG(LogTemp, Display, TEXT("Apollo stand-in actors skipped: -FortniteOriginalClient"));
        return;
    }

    // The original cooked landscape levels cannot load on Linux. Place the
    // locally converted meshes at their saved root transforms instead.
    const FString ManifestPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ApolloLandscapeGrid.json"));
    FString ManifestText;
    if (!FFileHelper::LoadFileToString(ManifestText, *ManifestPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Apollo terrain manifest missing: %s"), *ManifestPath);
        return;
    }
    TArray<TSharedPtr<FJsonValue>> Entries;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ManifestText);
    if (!FJsonSerializer::Deserialize(Reader, Entries))
    {
        UE_LOG(LogTemp, Warning, TEXT("Apollo terrain manifest invalid: %s"), *ManifestPath);
        return;
    }

    int32 Spawned = 0;
    for (const TSharedPtr<FJsonValue>& Entry : Entries)
    {
        const TSharedPtr<FJsonObject> Object = Entry.IsValid() ? Entry->AsObject() : nullptr;
        if (!Object.IsValid())
            continue;
        FString Tile;
        const TArray<TSharedPtr<FJsonValue>>* Location = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Scale = nullptr;
        if (!Object->TryGetStringField(TEXT("tile"), Tile) || Tile.Len() != 2 ||
            !Object->TryGetArrayField(TEXT("location"), Location) || !Location || Location->Num() != 3 ||
            !Object->TryGetArrayField(TEXT("scale"), Scale) || !Scale || Scale->Num() != 3)
            continue;
        const FTransform TileTransform(
            FRotator::ZeroRotator,
            FVector((*Location)[0]->AsNumber(), (*Location)[1]->AsNumber(), (*Location)[2]->AsNumber()),
            FVector((*Scale)[0]->AsNumber(), (*Scale)[1]->AsNumber(), (*Scale)[2]->AsNumber()));
        AApolloLandscapeTile* TileActor = GetWorld()->SpawnActorDeferred<AApolloLandscapeTile>(
            AApolloLandscapeTile::StaticClass(), TileTransform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!TileActor)
            continue;
        TileActor->SetTileId(Tile);
        UGameplayStatics::FinishSpawningActor(TileActor, TileTransform);
        Spawned++;
    }
    UE_LOG(LogTemp, Display, TEXT("Apollo terrain grid: %d/%d meshes spawned"), Spawned, Entries.Num());
    SpawnApolloBackground();
    GetWorld()->SpawnActor<AApolloSkyAtmosphere>(AApolloSkyAtmosphere::StaticClass(),
        FVector::ZeroVector, FRotator::ZeroRotator);
    GetWorld()->SpawnActor<AApolloSunLight>(AApolloSunLight::StaticClass(),
        FVector::ZeroVector, FRotator(-50.0f, -35.0f, 0.0f));
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteProbeTerrainCollision")))
    {
        const FVector Start(16000.0f, -16000.0f, 10000.0f);
        const FVector End(16000.0f, -16000.0f, -10000.0f);
        FHitResult Hit;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
        UE_LOG(LogTemp, Display, TEXT("Apollo terrain collision probe: hit=%d actor=%s impact=%s"),
            bHit ? 1 : 0, bHit && Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"),
            bHit ? *Hit.ImpactPoint.ToString() : TEXT("none"));
    }
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        PositionPawnOnTerrain(It->Get());
}

void AFortGameModeAthena::SpawnApolloBackground()
{
    const FString ManifestPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ApolloBackground.json"));
    FString ManifestText;
    if (!FFileHelper::LoadFileToString(ManifestText, *ManifestPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Apollo background manifest missing: %s"), *ManifestPath);
        return;
    }
    TArray<TSharedPtr<FJsonValue>> Entries;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ManifestText);
    if (!FJsonSerializer::Deserialize(Reader, Entries))
    {
        UE_LOG(LogTemp, Warning, TEXT("Apollo background manifest invalid: %s"), *ManifestPath);
        return;
    }
    int32 Spawned = 0;
    for (const TSharedPtr<FJsonValue>& Entry : Entries)
    {
        const TSharedPtr<FJsonObject> Object = Entry.IsValid() ? Entry->AsObject() : nullptr;
        if (!Object.IsValid())
            continue;
        FString MeshId;
        const TArray<TSharedPtr<FJsonValue>>* Location = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Rotation = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Scale = nullptr;
        if (!Object->TryGetStringField(TEXT("mesh"), MeshId) || MeshId.IsEmpty() ||
            !Object->TryGetArrayField(TEXT("location"), Location) || !Location || Location->Num() != 3 ||
            !Object->TryGetArrayField(TEXT("rotation"), Rotation) || !Rotation || Rotation->Num() != 3 ||
            !Object->TryGetArrayField(TEXT("scale"), Scale) || !Scale || Scale->Num() != 3)
            continue;
        const FTransform Transform(
            FRotator((*Rotation)[0]->AsNumber(), (*Rotation)[1]->AsNumber(), (*Rotation)[2]->AsNumber()),
            FVector((*Location)[0]->AsNumber(), (*Location)[1]->AsNumber(), (*Location)[2]->AsNumber()),
            FVector((*Scale)[0]->AsNumber(), (*Scale)[1]->AsNumber(), (*Scale)[2]->AsNumber()));
        AApolloBackgroundMesh* Actor = GetWorld()->SpawnActorDeferred<AApolloBackgroundMesh>(
            AApolloBackgroundMesh::StaticClass(), Transform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Actor)
            continue;
        FString MaterialPackage;
        Object->TryGetStringField(TEXT("materialPackage"), MaterialPackage);
        Actor->SetMeshId(MeshId, MaterialPackage);
        UGameplayStatics::FinishSpawningActor(Actor, Transform);
        Spawned++;
    }
    UE_LOG(LogTemp, Display, TEXT("Apollo background: %d/%d placed meshes spawned"), Spawned, Entries.Num());
}

// The original client's AFortPlayerController::ClientRestart waits until WorldInventory (a replicated AFortInventory
// owned by the controller) is valid, so a pawn is never finished on the client without it (G6). The original server
// creates it during login; until that body is ported, create an empty world inventory before the first restart.
static void EnsureWorldInventory(AController* NewPlayer)
{
    AFortPlayerController* Controller = Cast<AFortPlayerController>(NewPlayer);
    if (!Controller || Controller->WorldInventory || !Controller->GetWorld())
        return;
    FActorSpawnParameters Params;
    Params.Owner = Controller;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AFortInventory* Inventory = Controller->GetWorld()->SpawnActor<AFortInventory>(AFortInventory::StaticClass(), FTransform::Identity, Params);
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldInventory spawn failed for %s"), *Controller->GetName());
        return;
    }
    Inventory->SetReplicates(true);
    Inventory->bOnlyRelevantToOwner = true;
    Inventory->SetOwner(Controller);
    Inventory->InventoryType = EFortInventoryType::World;
    Controller->WorldInventory = Inventory;
    Controller->bHasInitializedWorldInventory = true;
    // Stand-in for the stubbed ServerSetClientHasFinishedLoading_Implementation: the client's SetHasFinishedLoading
    // (0x1423af7xx) only runs OnReadyToStartMatch, which creates ClientQuickBars (0x1423f20a0), once the replicated
    // bHasServerFinishedLoading flips; without it ClientRestart waits forever on "Quickbars are invalid".
    Controller->bHasClientFinishedLoading = true;
    Controller->bHasServerFinishedLoading = true;
    // The client's readiness check (0x1423cc220, which calls SetHasFinishedLoading and so creates ClientQuickBars)
    // runs from a 0.5 s looping timer. That timer is armed only by the client RPC ClientOnGenericPlayerInitialization
    // (client body 0x142371f00 -> controller virtual +0x1540 -> +0x1568). The original server sends it from login code
    // that is not in the client, so send it here (G6, see G6_CONTROLLER_PROBE_FIX.md).
    Controller->ClientOnGenericPlayerInitialization();
    UE_LOG(LogTemp, Display, TEXT("WorldInventory created for %s; sent ClientOnGenericPlayerInitialization"), *Controller->GetName());
}

// The client's pawn customization (0x1423593a0) waits for the player state's HeroType ("Hero:None" in the client log);
// the hero's specialization lists the character parts the client then builds. The original server assigns the hero from
// the player's profile loadout during login; until that is ported every player gets the default Athena hero.
static void EnsureHero(AController* NewPlayer)
{
    AFortPlayerState* PlayerState = NewPlayer ? Cast<AFortPlayerState>(NewPlayer->PlayerState) : nullptr;
    if (!PlayerState || PlayerState->HeroType)
        return;
    static const TCHAR* DefaultHeroPath = TEXT("/Game/Athena/Heroes/HID_560_Athena_Commando_M_RebirthDefaultA.HID_560_Athena_Commando_M_RebirthDefaultA");
    UFortHeroType* Hero = LoadObject<UFortHeroType>(nullptr, DefaultHeroPath);
    if (!Hero)
    {
        UE_LOG(LogTemp, Warning, TEXT("Default hero %s did not load"), DefaultHeroPath);
        return;
    }
    PlayerState->HeroType = Hero;

    // On a client the customization (0x1423593a0) takes the character parts from PlayerState->CharacterData when
    // WasPartReplicatedFlags (its first byte) is non-zero. The parts are the ones HS_560's specialization lists.
    struct FDefaultPart { EFortCustomPartType::Type Slot; const TCHAR* Path; };
    static const FDefaultPart DefaultParts[] = {
        { EFortCustomPartType::Head, TEXT("/Game/Characters/CharacterParts/Male/Medium/Heads/CP_Athena_Head_M_RebithSoldier.CP_Athena_Head_M_RebithSoldier") },
        { EFortCustomPartType::Body, TEXT("/Game/Athena/Heroes/Meshes/Bodies/CP_Athena_Body_M_RebirthSoldier.CP_Athena_Body_M_RebirthSoldier") },
        { EFortCustomPartType::Face, TEXT("/Game/Characters/CharacterParts/FaceAccessories/CP_M_MED_RebirthSoldier.CP_M_MED_RebirthSoldier") },
    };
    for (const FDefaultPart& Part : DefaultParts)
    {
        UCustomCharacterPart* Loaded = LoadObject<UCustomCharacterPart>(nullptr, Part.Path);
        if (!Loaded)
        {
            UE_LOG(LogTemp, Warning, TEXT("Default character part %s did not load"), Part.Path);
            continue;
        }
        PlayerState->CharacterData.Parts[Part.Slot] = Loaded;
        PlayerState->CharacterData.WasPartReplicatedFlags |= (1 << Part.Slot);
    }
    PlayerState->CharacterGender = EFortCustomGender::Male;
    PlayerState->CharacterBodyType = EFortCustomBodyType::Medium;
    PlayerState->ForceNetUpdate();
    UE_LOG(LogTemp, Display, TEXT("Assigned hero %s and character parts (flags %d) to %s"), *Hero->GetName(),
        (int32)PlayerState->CharacterData.WasPartReplicatedFlags, *PlayerState->GetName());
}

// StartWarmupPhase (0x141972c30) sets GamePhase = Warmup and the warmup step; with ?PauseWarmup the countdown end time
// stays unset (UpdateWarmupPhase 0x14197a400 does nothing). The client keeps its loading screen up until the replicated
// phase leaves Setup (G6). Until StartWarmupPhase is ported, enter the paused warmup when the first player spawns.
static void EnsureWarmupPhase(UWorld* World)
{
    AFortGameStateAthena* GameState = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
    if (!GameState || GameState->GamePhase >= EAthenaGamePhase::Warmup)
        return;
    GameState->GamePhase = EAthenaGamePhase::Warmup;
    GameState->GamePhaseStep = EAthenaGamePhaseStep::Warmup;
    GameState->ForceNetUpdate();
    UE_LOG(LogTemp, Display, TEXT("Set AthenaGamePhase to [Warmup] (paused warmup stand-in)"));
}

// While the game phase is Setup or Warmup the client keeps the loading screen up with the reason "Waiting for time of
// day manager" until its GameState's FortTimeOfDayManager (AFortGameStateBase +0x2d0) is set (loading-screen decision
// 0x141f062e0). The client's own manager actor registers itself with the GameState when it replicates, so the server
// has to spawn one. The original server spawns the Blueprint from GetTimeOfDayManagerClassNative; the native class
// stands in until that is ported.
// The client derives its in-game UI state tag from GameState+0x3d0 (the replicated WorldManager); with no manager the tag
// stays None and the client's loading screen manager reports "UIDirector has no valid state" forever.
static void EnsureWorldManager(UWorld* World)
{
    AFortGameStateAthena* GameState = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
    if (!GameState || GameState->WorldManager)
        return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AFortWorldManager* Manager = World->SpawnActor<AFortWorldManager>(AFortWorldManager::StaticClass(), FTransform::Identity, Params);
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldManager spawn failed"));
        return;
    }
    Manager->SetReplicates(true);
    Manager->bAlwaysRelevant = true;
    GameState->WorldManager = Manager;
    GameState->ForceNetUpdate();
    UE_LOG(LogTemp, Display, TEXT("Spawned %s as the GameState world manager"), *Manager->GetName());
}

static void EnsureTimeOfDayManager(UWorld* World)
{
    AFortGameStateAthena* GameState = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
    if (!GameState || GameState->FortTimeOfDayManager)
        return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    // Apollo_Terrain names its manager Blueprint in WorldTimeOfDayManager (data: TODM_BR_s11_C); the client spawns whatever
    // class the server replicates, so the Blueprint carries the day phases, sky light and fog the native class lacks.
    UClass* ManagerClass = nullptr;
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
    {
        ManagerClass = StaticLoadClass(AFortTimeOfDayManager::StaticClass(), nullptr, TEXT("/Game/TimeOfDay/TODM/BR/s11/TODM_BR_s11.TODM_BR_s11_C"));
        if (!ManagerClass)
            UE_LOG(LogTemp, Warning, TEXT("TimeOfDay: TODM_BR_s11_C not loadable; using the native class"));
    }
    AFortTimeOfDayManager* Manager = World->SpawnActor<AFortTimeOfDayManager>(ManagerClass ? ManagerClass : AFortTimeOfDayManager::StaticClass(), FTransform::Identity, Params);
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("TimeOfDayManager spawn failed"));
        return;
    }
    Manager->SetReplicates(true);
    Manager->bAlwaysRelevant = true;
    GameState->FortTimeOfDayManager = Manager;
    GameState->ForceNetUpdate();
    UE_LOG(LogTemp, Display, TEXT("Spawned %s as the GameState time of day manager"), *Manager->GetName());
}

void AFortGameModeAthena::RestartPlayer(AController* NewPlayer)
{
    EnsureWorldInventory(NewPlayer);
    EnsureHero(NewPlayer);
    EnsureTimeOfDayManager(GetWorld());
    EnsureWorldManager(GetWorld());
    if (!FortAthenaMatchFlow::IsActive())
        EnsureWarmupPhase(GetWorld());
    Super::RestartPlayer(NewPlayer);
    if (NewPlayer && NewPlayer->GetPawn())
        UE_LOG(LogTemp, Display, TEXT("Athena pawn spawned: class=%s authority=%d"),
            *NewPlayer->GetPawn()->GetClass()->GetName(), NewPlayer->GetPawn()->HasAuthority() ? 1 : 0);
    FortAthenaInventory::SetupPlayer(this, Cast<AFortPlayerControllerAthena>(NewPlayer));
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteNoHealth")))
        FortAthenaHealth::SetupPlayer(this, Cast<AFortPlayerControllerAthena>(NewPlayer));
    // Vanilla-client tests. FortniteEmptyDedicated has no ground: PositionPawnOnTerrain places the pawn on the replicated
    // test floor. On Apollo_Terrain it places the pawn on the lobby island (no PlayerStart actors load). In both cases the
    // match flow runs afterwards: for a pending bus exit it moves the pawn out of the aircraft and starts the skydive.
    if (NewPlayer && NewPlayer->GetPawn() && GetWorld() && FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
    {
        PositionPawnOnTerrain(NewPlayer);
        FortAthenaMatchFlow::OnPlayerRestarted(this, NewPlayer);
        return;
    }
    if (!NewPlayer || !NewPlayer->GetPawn() || !GetWorld() ||
        !GetWorld()->GetMapName().EndsWith(TEXT("Apollo_Terrain")))
        return;

    PositionPawnOnTerrain(NewPlayer);

    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteProbePawnGround")))
    {
        TWeakObjectPtr<APawn> WeakPawn(NewPlayer->GetPawn());
        FTimerHandle Handle;
        GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakPawn]()
        {
            if (ACharacter* Character = Cast<ACharacter>(WeakPawn.Get()))
                UE_LOG(LogTemp, Display, TEXT("Apollo pawn ground probe: grounded=%d mode=%s location=%s velocity=%s"),
                    Character->GetCharacterMovement()->IsMovingOnGround() ? 1 : 0,
                    *Character->GetCharacterMovement()->GetMovementName(),
                    *Character->GetActorLocation().ToString(),
                    *Character->GetVelocity().ToString());
        }), 5.0f, false);
    }
}

void AFortGameModeAthena::PositionPawnOnTerrain(AController* Player)
{
    if (!Player || !Player->GetPawn() || !GetWorld())
        return;
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")) && GetWorld()->GetMapName().EndsWith(TEXT("Apollo_Terrain")))
    {
        // Real map: the pre-bus island is the Apollo_Terrain_LS_Lobby landscape level (no PlayerStart actors are
        // loaded for it, FindPlayerStart reports none). Trace down onto that level's landscape collision and stand there.
        FBox LobbyBounds(ForceInit);
        for (TActorIterator<ALandscapeProxy> It(GetWorld()); It; ++It)
        {
            const FString LevelName = It->GetLevel() && It->GetLevel()->GetOutermost() ? It->GetLevel()->GetOutermost()->GetName() : FString();
            if (LevelName.Contains(TEXT("Apollo_Terrain_LS_Lobby")))
                LobbyBounds += It->GetComponentsBoundingBox(true);
        }
        if (LobbyBounds.IsValid)
        {
            const FVector Centre = LobbyBounds.GetCenter();
            FHitResult LobbyHit;
            FCollisionQueryParams Query(NAME_None, false);
            if (GetWorld()->LineTraceSingleByChannel(LobbyHit, FVector(Centre.X, Centre.Y, LobbyBounds.Max.Z + 20000.0f),
                    FVector(Centre.X, Centre.Y, LobbyBounds.Min.Z - 20000.0f), ECC_WorldStatic, Query))
            {
                const FVector Spawn = LobbyHit.ImpactPoint + FVector(0.0f, 0.0f, 200.0f);
                Player->GetPawn()->SetActorLocation(Spawn, false, nullptr, ETeleportType::TeleportPhysics);
                UE_LOG(LogTemp, Display, TEXT("Lobby island: pawn positioned at %s (landscape bounds %s)"), *Spawn.ToString(), *LobbyBounds.ToString());
                return;
            }
            UE_LOG(LogTemp, Warning, TEXT("Lobby island: trace at %s hit nothing (bounds %s)"), *Centre.ToString(), *LobbyBounds.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Lobby island: no landscape proxy found in Apollo_Terrain_LS_Lobby"));
        }
        return;
    }
    const FVector Start(16000.0f, -16000.0f, 10000.0f);
    const FVector End(16000.0f, -16000.0f, -10000.0f);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility) &&
        Hit.GetActor() && Hit.GetActor()->IsA<AApolloLandscapeTile>())
    {
        const FVector Spawn = Hit.ImpactPoint + FVector(0.0f, 0.0f, 150.0f);
        Player->GetPawn()->SetActorLocation(Spawn, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Display, TEXT("Apollo pawn positioned above terrain at %s"), *Spawn.ToString());
        return;
    }
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")))
        return;
    // FortniteEmptyDedicated has no ground, so a pawn falls forever. For the vanilla-client tests spawn one wood floor
    // building piece scaled to about +-51200 units. stand-in: test ground, not original behaviour. A StaticMeshActor
    // cannot be used: the client rejects our engine UStaticMeshComponent (NetChecksumMismatch, run 29). The building
    // class is a Fortnite class with matching checksums, its mesh comes from the class defaults on both sides, and the
    // spawn scale reaches the client through SerializeNewActor.
    static TWeakObjectPtr<AActor> Floor;
    UWorld* World = GetWorld();
    if (!Floor.IsValid())
    {
        UClass* FloorClass = StaticLoadClass(AActor::StaticClass(), nullptr,
            TEXT("/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_Floor.PBWA_W1_Floor_C"));
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        const FTransform FloorTransform(FRotator::ZeroRotator, FVector(-51200.0f, -51200.0f, -1000.0f), FVector(200.0f, 200.0f, 1.0f));
        AActor* Actor = FloorClass ? World->SpawnActor<AActor>(FloorClass, FloorTransform, Params) : nullptr;
        if (!Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Test floor not created (floor class %s)"), FloorClass ? TEXT("found") : TEXT("missing"));
            return;
        }
        Actor->SetActorScale3D(FVector(200.0f, 200.0f, 1.0f));
        Actor->bAlwaysRelevant = true;
        Actor->ForceNetUpdate();
        Floor = Actor;
        // The piece's pivot is not known here; move it so its bounds are centred on (0, 0) with the top at z=-950.
        FBox Bounds = Actor->GetComponentsBoundingBox(true);
        if (Bounds.IsValid)
        {
            const FVector Shift(-Bounds.GetCenter().X, -Bounds.GetCenter().Y, -950.0f - Bounds.Max.Z);
            Actor->SetActorLocation(Actor->GetActorLocation() + Shift, false, nullptr, ETeleportType::TeleportPhysics);
            Bounds = Actor->GetComponentsBoundingBox(true);
        }
        UE_LOG(LogTemp, Display, TEXT("Test floor %s created for the original client, bounds %s"), *Actor->GetName(), *Bounds.ToString());
    }
    Player->GetPawn()->SetActorLocation(FVector(0.0f, 0.0f, -800.0f), false, nullptr, ETeleportType::TeleportPhysics);
}

AApolloLandscapeTile::AApolloLandscapeTile()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(false);
    GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
}

void AApolloLandscapeTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AApolloLandscapeTile, TileId);
}

void AApolloLandscapeTile::SetTileId(const FString& NewTileId)
{
    TileId = NewTileId;
    LoadTileMesh();
}

void AApolloLandscapeTile::OnRep_TileId()
{
    LoadTileMesh();
}

void AApolloLandscapeTile::LoadTileMesh()
{
    const FString Name = FString::Printf(TEXT("LandscapeStreamingProxy_%s"), *TileId);
    const FString Path = FString::Printf(
        TEXT("/Game/Athena/Apollo/Maps/Landscape/Converted/%s.%s"), *Name, *Name);
    UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *Path));
    if (Mesh)
    {
        GetStaticMeshComponent()->SetStaticMesh(Mesh);
        GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    }
    UE_LOG(LogTemp, Display, TEXT("Apollo terrain tile %s ready: authority=%d mesh=%s"),
        *TileId, HasAuthority() ? 1 : 0,
        GetStaticMeshComponent()->GetStaticMesh() ? TEXT("yes") : TEXT("no"));
}

AApolloBackgroundMesh::AApolloBackgroundMesh()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(true);
    GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetStaticMeshComponent()->SetCastShadow(false);
}

void AApolloBackgroundMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AApolloBackgroundMesh, MeshId);
    DOREPLIFETIME(AApolloBackgroundMesh, MaterialPackage);
}

void AApolloBackgroundMesh::SetMeshId(const FString& NewMeshId, const FString& NewMaterialPackage)
{
    MeshId = NewMeshId;
    MaterialPackage = NewMaterialPackage;
    LoadMesh();
}

void AApolloBackgroundMesh::OnRep_MeshId()
{
    LoadMesh();
}

void AApolloBackgroundMesh::LoadMesh()
{
    if (MeshId.IsEmpty() || MeshId.Contains(TEXT("/")) || MeshId.Contains(TEXT(".")))
        return;
    const FString Path = FString::Printf(
        TEXT("/Game/Athena/Apollo/Maps/Background/Converted/%s.%s"), *MeshId, *MeshId);
    UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *Path));
    if (Mesh)
        GetStaticMeshComponent()->SetStaticMesh(Mesh);
    // -FortniteHideBackground=<MeshId> hides one background mesh type to identify it in captures.
    FString Hidden;
    if (FParse::Value(FCommandLine::Get(), TEXT("FortniteHideBackground="), Hidden) && Hidden == MeshId)
        GetStaticMeshComponent()->SetVisibility(false);
    int32 Rebuilt = 0;
    // Prefer rebuilt materials stored at the original asset paths: the per-component
    // override first, then each mesh slot's material by name.
    auto LoadRebuilt = [](const FString& Package) -> UMaterialInterface*
    {
        if (!Package.StartsWith(TEXT("/Game/")) || Package.Contains(TEXT(".")) ||
            !FPackageName::DoesPackageExist(Package))
            return nullptr;
        return Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
            *FString::Printf(TEXT("%s.%s"), *Package, *FPackageName::GetShortName(Package))));
    };
    // -FortniteNoRebuiltBackground keeps the importer's placeholder materials for before/after captures.
    if (Mesh && !FParse::Param(FCommandLine::Get(), TEXT("FortniteNoRebuiltBackground")))
    {
        for (int32 Slot = 0; Slot < Mesh->StaticMaterials.Num(); ++Slot)
        {
            UMaterialInterface* Replacement = nullptr;
            if (Slot == 0 && !MaterialPackage.IsEmpty())
                Replacement = LoadRebuilt(MaterialPackage);
            // The glTF importer names the slot by index; the placeholder material keeps the original name.
            const FStaticMaterial& Entry = Mesh->StaticMaterials[Slot];
            const FString SlotMaterial = Entry.MaterialInterface ? Entry.MaterialInterface->GetName()
                                                                  : Entry.MaterialSlotName.ToString();
            if (!Replacement && SlotMaterial.StartsWith(TEXT("M_")))
                Replacement = LoadRebuilt(TEXT("/Game/Athena/Apollo/Environments/Landscape/Background/") + SlotMaterial);
            if (Replacement)
            {
                GetStaticMeshComponent()->SetMaterial(Slot, Replacement);
                Rebuilt++;
            }
        }
    }
    UE_LOG(LogTemp, Display, TEXT("Apollo background mesh %s ready: authority=%d mesh=%s rebuilt_materials=%d actor=%s"),
        *MeshId, HasAuthority() ? 1 : 0,
        GetStaticMeshComponent()->GetStaticMesh() ? TEXT("yes") : TEXT("no"), Rebuilt, *GetName());
}
