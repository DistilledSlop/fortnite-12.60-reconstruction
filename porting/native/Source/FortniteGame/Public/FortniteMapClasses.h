#pragma once

#include "Engine/LevelScriptActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NavAreas/NavAreaMeta.h"
#include "NavigationSystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FFortClientEventName.h"
#include "FFortDialogExternalLatentActionHandle.h"
#include "GameFramework/OnlineReplStructs.h"
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
    UFUNCTION(BlueprintPure)
    static bool IsAssetNull(TSoftObjectPtr<UObject> Asset);

    UFUNCTION(BlueprintPure)
    static bool IsValid_UniqueNetIdRepl(const FUniqueNetIdRepl& InUniqueNetIdRepl);

    UFUNCTION(BlueprintCallable)
    static void InvalidateLatentActionHandle(UPARAM(ref) FFortDialogExternalLatentActionHandle& Handle);

    UFUNCTION(BlueprintPure)
    static bool IsLatentActionHandleValid(const FFortDialogExternalLatentActionHandle& Handle);

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool DeviceSupportsForceFeedback();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsUsingBattlEye();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsCellConnection();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsConsolePlatform();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsPhoneGame();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsTabletGame();

    UFUNCTION(BlueprintPure, Category="Fortnite|Platform")
    static bool IsMobilePlatform();

    UFUNCTION(BlueprintCallable, Category="Fortnite|Math")
    static float GetAngleDegrees(const FVector& Source, const FVector& Target);

    UFUNCTION(BlueprintPure, Category="Fortnite|Math")
    static FVector K2_GetClosestAxisXY(FVector InVector);

    UFUNCTION(BlueprintPure, Category="Fortnite|Math")
    static FVector NormalizedAngleInDegreesToVector(float A);

    UFUNCTION(BlueprintPure, Category="Fortnite|Math")
    static float VectorToNormalizedAngleInDegrees(FVector V);

    // Original Win64 shipping contract: context is marshaled but not inspected.
    UFUNCTION(BlueprintPure, Category="Fortnite|Platform", meta=(WorldContext="WorldContextObject"))
    static bool IsMobileGame(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext="WorldContextObject"))
    static void ExecuteClientEvent(UObject* WorldContextObject, FFortClientEventName EventType, UObject* EventSource, UObject* EventFocus);
};
