#include "FortniteAthenaGameplay.h"

#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AFortPlayerPawn::AFortPlayerPawn()
{
    bReplicates = true;
    SetReplicateMovement(true);
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
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
    if (GetWorld() && GetWorld()->GetMapName().EndsWith(TEXT("Apollo_Terrain")))
    {
        // The original cooked A1 tile cannot be loaded on Linux yet. Stage a
        // reconstructed mesh at its saved root transform instead.
        const FTransform TileTransform(
            FRotator::ZeroRotator,
            FVector(113792.0f, -130048.0f, 100.0f),
            FVector(128.0f, 128.0f, 100.0f));
        GetWorld()->SpawnActor<AApolloLandscapeTileA1>(AApolloLandscapeTileA1::StaticClass(), TileTransform);
    }
}

AApolloLandscapeTileA1::AApolloLandscapeTileA1()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TerrainMesh(
        TEXT("/Game/Athena/Apollo/Maps/Landscape/Converted/LandscapeStreamingProxy_A1.LandscapeStreamingProxy_A1"));
    if (TerrainMesh.Succeeded())
        GetStaticMeshComponent()->SetStaticMesh(TerrainMesh.Object);
}

void AApolloLandscapeTileA1::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Display, TEXT("Apollo A1 mesh ready: %s authority=%d mesh=%s"),
        *GetPathName(), HasAuthority() ? 1 : 0,
        GetStaticMeshComponent()->GetStaticMesh() ? TEXT("yes") : TEXT("no"));
}
