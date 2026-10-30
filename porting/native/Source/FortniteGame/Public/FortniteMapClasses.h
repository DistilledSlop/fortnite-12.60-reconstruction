#pragma once

#include "Engine/LevelScriptActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/WorldSettings.h"
#include "NavAreas/NavAreaMeta.h"
#include "NavigationSystem.h"
#include "FortniteMapClasses.generated.h"

// Initial reflection-compatible bases for loading a recovered map. Gameplay
// behavior and serialized Fortnite properties still need reconstruction.
UCLASS()
class FORTNITEGAME_API AFortLevelScriptActor : public ALevelScriptActor
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortWorldSettings : public AWorldSettings
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortStaticMeshActor : public AStaticMeshActor
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API UFortMetaNavArea_Obstacles : public UNavAreaMeta
{
    GENERATED_BODY()
};

// The recovered map's navigation config names this native class. Its full
// Fortnite-specific behavior still needs reconstruction.
UCLASS()
class FORTNITEGAME_API UFortNavSystem : public UNavigationSystemV1
{
    GENERATED_BODY()
};

// The Apollo persistent level serializes these manager objects. Their
// Fortnite gameplay responsibilities are still to be recovered.
UCLASS()
class FORTNITEGAME_API AFortPoiManager : public AActor
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API UWorldTileManager : public UActorComponent
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortWorldManager : public AActor
{
    GENERATED_BODY()

public:
    AFortWorldManager();

    UPROPERTY(VisibleAnywhere)
    USceneComponent* MySceneComponent;

    UPROPERTY(VisibleAnywhere)
    UWorldTileManager* WorldTileManager;
};

UCLASS()
class FORTNITEGAME_API AFortSkydivingShadowProxy : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void ShowShadowProxy();
    virtual void ShowShadowProxy_Implementation();

    UFUNCTION(BlueprintNativeEvent)
    void HideShadowProxy();
    virtual void HideShadowProxy_Implementation();
};

UCLASS()
class FORTNITEGAME_API AFortSafeZoneStorm : public AActor
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortAthenaMapInfo : public AActor
{
    GENERATED_BODY()
};
