#pragma once

#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FFortClientEventName.h"
#include "FFortDialogExternalLatentActionHandle.h"
#include "GameFramework/OnlineReplStructs.h"
#include "FortniteMapClasses.generated.h"

class UFortWorldItemDefinition;
class AFortPlayerController;

// Classes that are not in the promoted native set (porting/native_class_set.txt) stay hand-written here.
// AFortWorldSettings, AFortAthenaMapInfo, AFortPoiManager and AFortWorldManager are generated in
// Generated/ from the reflection tables and are not declared here.

// AFortLevelScriptActor, AFortStaticMeshActor and UFortMetaNavArea_Obstacles are generated in Generated/.
// Only the shadow proxy stays hand-written: it has a constructor creating DefaultSceneRoot.
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

