#pragma once

#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StaticMeshActor.h"
#include "FortniteAthenaGameplay.generated.h"

// Initial native player hierarchy. Movement is UE character movement until
// the recovered Fortnite abilities, inventory, and cosmetics are rebuilt.
UCLASS()
class FORTNITEGAME_API AFortPlayerPawn : public ACharacter
{
    GENERATED_BODY()

public:
    AFortPlayerPawn();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
};

UCLASS()
class FORTNITEGAME_API AFortPlayerPawnAthena : public AFortPlayerPawn
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortGameModeAthena : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFortGameModeAthena();
    virtual void StartPlay() override;
};

// Linux cooked geometry reconstructed from Apollo's landscape components.
UCLASS()
class FORTNITEGAME_API AApolloLandscapeTile : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AApolloLandscapeTile();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void SetTileId(const FString& NewTileId);

private:
    UPROPERTY(ReplicatedUsing=OnRep_TileId)
    FString TileId;

    UFUNCTION()
    void OnRep_TileId();

    void LoadTileMesh();
};
