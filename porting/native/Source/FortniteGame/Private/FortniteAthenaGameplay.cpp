#include "FortniteAthenaGameplay.h"

#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
