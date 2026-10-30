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
// Parent guessed: UAIPerceptionStimuliSourceComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UAthenaTrapAIPerceptionStimuliSourceComponent : public UAIPerceptionStimuliSourceComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingActor : public AActor
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
class FORTNITEGAME_API UBuildingEditModeSupport_Stair : public UObject
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
// Parent guessed: AActor. Building base, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingFoundation : public AActor
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
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingGameplayActor : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UBuildingLayoutRequirement : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AStaticMeshActor. Static mesh actor, name inference only; unverified.
UCLASS()
class FORTNITEGAME_API ABuildingSMActor : public AStaticMeshActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UCaptureComponent : public UActorComponent
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
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UCreativeQuestComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UEdgeNavLinks : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APawn. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortAIPawn : public APawn
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
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortActorOptionsComponent : public UActorComponent
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
class FORTNITEGAME_API UFortAsyncAction_SendQuestStatEvent : public UBlueprintAsyncActionBase
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
class FORTNITEGAME_API AFortAthenaPatrolPath : public AActor
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortClientAnnouncement : public UObject
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
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortCreativeCreatureManagerInfoComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortCreativeVolumeLinkComponent : public UActorComponent
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortFeedbackManager : public UObject
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
// Parent guessed: UGameInstance. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortGameInstance : public UGameInstance
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
// Parent guessed: AGameModeBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortGameModePvE : public AGameModeBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AGameStateBase. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortGameStateAthena : public AGameStateBase
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortGameplayMessageDispatcherComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortGameplayReceiverMessageComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortGameplayTriggerMessageComponent : public UActorComponent
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortHealthSet : public UObject
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
class FORTNITEGAME_API UFortLevelSaveComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortLinkToActorComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMiniMapComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMinigameLogicComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMinigameObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMinigamePlayerStartComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMinigameProgressComponent : public UActorComponent
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortMission : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Interface guessed from the census Class import name; unverified.
UINTERFACE()
class FORTNITEGAME_API UFortMissionInterface : public UInterface
{
    GENERATED_BODY()
};
class FORTNITEGAME_API IFortMissionInterface
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortNavigationGraph : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortNavigationVisibilityComponent : public UActorComponent
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
// Parent guessed: APawn. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPawn : public APawn
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlacementActor : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerController. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerController : public APlayerController
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
// Parent guessed: APawn. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerMannequin : public APawn
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerStart. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerStart : public APlayerStart
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerStart. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerStartCreative : public APlayerStart
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: APlayerStart. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPlayerStartWarmup : public APlayerStart
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortPlaylist : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortPoiCollisionComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AVolume. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortPoiVolume : public AVolume
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
class FORTNITEGAME_API UFortPortalComponent : public UActorComponent
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
// Parent guessed: UObject. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortQuestManager : public UObject
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Interface guessed from the census Class import name; unverified.
UINTERFACE()
class FORTNITEGAME_API UFortSpawnableByPlacementSystemInterface : public UInterface
{
    GENERATED_BODY()
};
class FORTNITEGAME_API IFortSpawnableByPlacementSystemInterface
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortSpectateClickableMapIcon : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UAudioComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortSplineWaterAudioComponent : public UAudioComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortTimeOfDayManager : public AActor
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
class FORTNITEGAME_API UFortVehicleAudioVoice : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortVisibilityComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortVolumeTimeOfDayComponent : public UActorComponent
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: AActor. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API AFortWaterBodyActor : public AActor
{
    GENERATED_BODY()
};

// STUB: layout unknown, replace with generated class
// Parent guessed: UActorComponent. Naming convention; unverified.
UCLASS()
class FORTNITEGAME_API UFortWaterInteractionComponent : public UActorComponent
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
class FORTNITEGAME_API UFortWorldItemDefinition : public UObject
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
class FORTNITEGAME_API UQuestInteractableComponent : public UActorComponent
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
