#pragma once

// Census stubs for /Script/FortniteGame classes that the recovered maps import
// (porting/generated/map-native-imports.jsonl, Class kind) and that the module did not
// declare. STUBS ONLY: no Fortnite behavior, no serialized properties, and unknown
// layouts. Every parent is guessed from the naming convention and UE engine parents
// unless the note says otherwise. Function and enum imports are not declared here.

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Volume.h"
#include "GameFramework/PhysicsVolume.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "AI/NavigationSystemConfig.h"
#include "NavSystemConfigOverride.h"
#include "ABuildingFoundation.h"
#include "FortniteMapStubs.generated.h"

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UAthenaNavOctTreeInclusionBounds : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UNavigationSystemConfig. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UAthenaNavSystemConfig : public UNavigationSystemConfig
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: ANavSystemConfigOverride. NavSystemConfigOverride actor, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API AAthenaNavSystemConfigOverride : public ANavSystemConfigOverride
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UBuildingEditModeSupport_Floor : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UBuildingEditModeSupport_Roof : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UBuildingEditModeSupport_Wall : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: ABuildingFoundation. Size variant of BuildingFoundation, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingFoundation3x3 : public ABuildingFoundation
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: ABuildingFoundation. Size variant of BuildingFoundation, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingFoundation5x5 : public ABuildingFoundation
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UComplexPawnOverlapComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UCreativeDeferredTaskComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UAIPerceptionComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortAIPerceptionComponent : public UAIPerceptionComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UBlueprintAsyncActionBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortAsyncAction_CurrentPlaylistReady : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UBlueprintAsyncActionBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortAsyncAction_WaitForClientAnnouncement : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortAthenaPatrolPoint : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortBangCheckComponent_Customization : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortBangCheckComponent_Donut : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortBangCheckComponent_Elevator : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortBangCheckComponent_Faction : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortBangCheckComponent_Map : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UAudioComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortCollisionAudioComponent : public UAudioComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortCreativeCostComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortEnemySpawn : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AVolume. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortForcedLODZone : public AVolume
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AGameModeBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortGameModeFrontEnd : public AGameModeBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AGameModeBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortGameModeMinimal : public AGameModeBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AStaticMeshActor. Static mesh actor, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API AFortHLODSMActor : public AStaticMeshActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortHealthBarComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortInspectorCameraCreative : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortIslandLocalizationComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortJumpDownLink : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMinigameVolumeComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UBlueprintFunctionLibrary. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMissionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortNavMesh : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: ACameraActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPOICameraActor : public ACameraActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerController. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerControllerFrontEnd : public APlayerController
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortPoi_DiscoverableComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortPropertyOverrideReplComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerStart. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPvPPlayerStart : public APlayerStart
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AVolume. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortUndergroundVolume : public AVolume
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortWaypointVolumeComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFrontEndSettings : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AVolume. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AGameplayEffectVolume : public AVolume
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UInfiltrationStatusComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UPlaysetLevelStreamComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API URegisterWithMutatorComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APhysicsVolume. Water volume, physics volume is a guess; unverified.
UCLASS()
class FORTNITEGAME_API AWaterVolume_Athena : public APhysicsVolume
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APhysicsVolume. Water volume, physics volume is a guess; unverified.
UCLASS()
class FORTNITEGAME_API AWaterVolume_Deep : public APhysicsVolume
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APhysicsVolume. Water volume, physics volume is a guess; unverified.
UCLASS()
class FORTNITEGAME_API AWaterVolume_Shallow : public APhysicsVolume
{
    GENERATED_BODY()
};
