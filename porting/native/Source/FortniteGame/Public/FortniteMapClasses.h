#pragma once

#include "Engine/LevelScriptActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NavAreas/NavAreaMeta.h"
#include "NavigationSystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FortniteMapClasses.generated.h"

// Classes that are not in the promoted native set (porting/native_class_set.txt) stay hand-written here.
// AFortWorldSettings, AFortAthenaMapInfo, AFortPoiManager and AFortWorldManager are generated in
// Generated/ from the reflection tables and are not declared here.

// Initial reflection-compatible bases for loading a recovered map. Gameplay
// behavior and serialized Fortnite properties still need reconstruction.
UCLASS()
class FORTNITEGAME_API AFortLevelScriptActor : public ALevelScriptActor
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

UCLASS()
class FORTNITEGAME_API AFortSkydivingShadowProxy : public AActor
{
    GENERATED_BODY()

public:
    AFortSkydivingShadowProxy();

    UPROPERTY(VisibleAnywhere)
    USceneComponent* DefaultSceneRoot;

    UFUNCTION(BlueprintNativeEvent)
    void ShowShadowProxy();
    virtual void ShowShadowProxy_Implementation();

    UFUNCTION(BlueprintNativeEvent)
    void HideShadowProxy();
    virtual void HideShadowProxy_Implementation();
};

UCLASS()
class FORTNITEGAME_API UFortKismetLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsMobilePlatform();
};
