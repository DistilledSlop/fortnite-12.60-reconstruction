#include "FortniteAthenaGameplay.h"
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

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
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

void AFortGameModeAthena::StartPlay()
{
    Super::StartPlay();
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

void AFortGameModeAthena::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    if (NewPlayer && NewPlayer->GetPawn())
        UE_LOG(LogTemp, Display, TEXT("Athena pawn spawned: class=%s authority=%d"),
            *NewPlayer->GetPawn()->GetClass()->GetName(), NewPlayer->GetPawn()->HasAuthority() ? 1 : 0);
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
    const FVector Start(16000.0f, -16000.0f, 10000.0f);
    const FVector End(16000.0f, -16000.0f, -10000.0f);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility) &&
        Hit.GetActor() && Hit.GetActor()->IsA<AApolloLandscapeTile>())
    {
        const FVector Spawn = Hit.ImpactPoint + FVector(0.0f, 0.0f, 150.0f);
        Player->GetPawn()->SetActorLocation(Spawn, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Display, TEXT("Apollo pawn positioned above terrain at %s"), *Spawn.ToString());
    }
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
