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
#include "AFortHLODSMActor.h"
