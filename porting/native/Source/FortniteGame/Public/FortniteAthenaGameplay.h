#pragma once

#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StaticMeshActor.h"
#include "FortniteAthenaGameplay.generated.h"

class UCameraComponent;
class UDirectionalLightComponent;
class USkyAtmosphereComponent;

UCLASS()
class FORTNITEGAME_API AApolloSkyAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AApolloSkyAtmosphere(const FObjectInitializer& ObjectInitializer);

private:
    UPROPERTY(VisibleAnywhere)
    USkyAtmosphereComponent* SkyAtmosphere;
};

UCLASS()
class FORTNITEGAME_API AApolloSunLight : public AActor
{
    GENERATED_BODY()

public:
    AApolloSunLight(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UDirectionalLightComponent* SunLight;
};

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
    UPROPERTY(VisibleAnywhere)
    UCameraComponent* PlayerCamera;

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
    virtual void RestartPlayer(AController* NewPlayer) override;

private:
    void PositionPawnOnTerrain(AController* Player);
    void SpawnApolloBackground();
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

// Recovered Apollo_Background placements, with Linux cooked mesh copies.
UCLASS()
class FORTNITEGAME_API AApolloBackgroundMesh : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AApolloBackgroundMesh();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void SetMeshId(const FString& NewMeshId, const FString& NewMaterialPackage);

private:
    UPROPERTY(ReplicatedUsing=OnRep_MeshId)
    FString MeshId;

    // Original per-component material override package, such as a cloud instance.
    UPROPERTY(ReplicatedUsing=OnRep_MeshId)
    FString MaterialPackage;

    UFUNCTION()
    void OnRep_MeshId();

    void LoadMesh();
};
