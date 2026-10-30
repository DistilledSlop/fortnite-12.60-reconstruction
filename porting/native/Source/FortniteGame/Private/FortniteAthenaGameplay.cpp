#include "FortniteAthenaGameplay.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

AFortPlayerPawn::AFortPlayerPawn()
{
    bReplicates = true;
    SetReplicateMovement(true);
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment(GetCapsuleComponent());
    PlayerCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    PlayerCamera->bUsePawnControlRotation = true;
}

void AFortPlayerPawn::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Display, TEXT("FortPlayerPawn ready: %s authority=%d"),
        *GetPathName(), HasAuthority() ? 1 : 0);
}

void AFortPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFortPlayerPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFortPlayerPawn::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
}

void AFortPlayerPawn::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
        AddMovementInput(GetActorForwardVector(), Value);
}

void AFortPlayerPawn::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
        AddMovementInput(GetActorRightVector(), Value);
}

AFortGameModeAthena::AFortGameModeAthena()
{
    DefaultPawnClass = AFortPlayerPawnAthena::StaticClass();
}

void AFortGameModeAthena::StartPlay()
{
    Super::StartPlay();
    if (!GetWorld() || !GetWorld()->GetMapName().EndsWith(TEXT("Apollo_Terrain")))
        return;

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

void AFortGameModeAthena::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
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
