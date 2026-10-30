#!/usr/bin/env python3
"""Generate FortniteMapStubs.h/.cpp from the census-derived table (scratch tool, not in repo)."""
import sys

# parent key -> (C++ parent name, include path or None for in-module)
PARENTS = {
    'A':    ('AActor', 'GameFramework/Actor.h'),
    'AS':   ('AStaticMeshActor', 'Engine/StaticMeshActor.h'),
    'AV':   ('AVolume', 'GameFramework/Volume.h'),
    'AP':   ('APhysicsVolume', 'GameFramework/PhysicsVolume.h'),
    'ACAM': ('ACameraActor', 'Camera/CameraActor.h'),
    'APAWN': ('APawn', 'GameFramework/Pawn.h'),
    'APC':  ('APlayerController', 'GameFramework/PlayerController.h'),
    'AGM':  ('AGameModeBase', 'GameFramework/GameModeBase.h'),
    'AGS':  ('AGameStateBase', 'GameFramework/GameStateBase.h'),
    'PS':   ('APlayerStart', 'GameFramework/PlayerStart.h'),
    'ANSO': ('ANavSystemConfigOverride', 'NavSystemConfigOverride.h'),
    'UO':   ('UObject', 'UObject/Object.h'),
    'UAC':  ('UActorComponent', 'Components/ActorComponent.h'),
    'UAUD': ('UAudioComponent', 'Components/AudioComponent.h'),
    'UAS':  ('UBlueprintAsyncActionBase', 'Kismet/BlueprintAsyncActionBase.h'),
    'UBFL': ('UBlueprintFunctionLibrary', 'Kismet/BlueprintFunctionLibrary.h'),
    'UAIP': ('UAIPerceptionComponent', 'Perception/AIPerceptionComponent.h'),
    'UAIS': ('UAIPerceptionStimuliSourceComponent', 'Perception/AIPerceptionStimuliSourceComponent.h'),
    'UGI':  ('UGameInstance', 'Engine/GameInstance.h'),
    'UNSC': ('UNavigationSystemConfig', 'AI/NavigationSystemConfig.h'),
    'ABF':  ('ABuildingFoundation', None),
    'INTERFACE': (None, 'UObject/Interface.h'),
}

# census name (no /Script/FortniteGame prefix) -> parent key, optional note
T = {}
def add(key, names, note=''):
    for n in names.split():
        T[n] = (key, note)

add('A', 'BuildingActor BuildingGameplayActor FortAthenaPatrolPath FortAthenaPatrolPoint FortEnemySpawn FortInspectorCameraCreative FortNavMesh FortPlacementActor FortSpectateClickableMapIcon FortTimeOfDayManager FortWaterBodyActor')
add('A', 'BuildingFoundation', 'building base, name inference only')
add('ABF', 'BuildingFoundation3x3 BuildingFoundation5x5', 'size variant of BuildingFoundation, name inference only')
add('AS', 'BuildingSMActor FortHLODSMActor', 'static mesh actor, name inference only')
add('UO', 'BuildingEditModeSupport_Floor BuildingEditModeSupport_Roof BuildingEditModeSupport_Stair BuildingEditModeSupport_Wall BuildingLayoutRequirement FortClientAnnouncement FortFeedbackManager FortHealthSet FortMission FortNavigationGraph FortPlaylist FortQuestManager FortWorldItemDefinition FrontEndSettings AthenaNavOctTreeInclusionBounds')
add('UAC', 'CaptureComponent ComplexPawnOverlapComponent CreativeDeferredTaskComponent CreativeQuestComponent EdgeNavLinks FortActorOptionsComponent FortBangCheckComponent_Customization FortBangCheckComponent_Donut FortBangCheckComponent_Elevator FortBangCheckComponent_Faction FortBangCheckComponent_Map FortCreativeCostComponent FortCreativeCreatureManagerInfoComponent FortCreativeVolumeLinkComponent FortGameplayMessageDispatcherComponent FortGameplayReceiverMessageComponent FortGameplayTriggerMessageComponent FortHealthBarComponent FortIslandLocalizationComponent FortJumpDownLink FortLevelSaveComponent FortLinkToActorComponent FortMiniMapComponent FortMinigameLogicComponent FortMinigameObjectiveComponent FortMinigamePlayerStartComponent FortMinigameProgressComponent FortMinigameVolumeComponent FortNavigationVisibilityComponent FortPoiCollisionComponent FortPoi_DiscoverableComponent FortPortalComponent FortPropertyOverrideReplComponent FortVehicleAudioVoice FortVisibilityComponent FortVolumeTimeOfDayComponent FortWaterInteractionComponent FortWaypointVolumeComponent InfiltrationStatusComponent PlaysetLevelStreamComponent QuestInteractableComponent RegisterWithMutatorComponent')
add('UAUD', 'FortCollisionAudioComponent FortSplineWaterAudioComponent')
add('UAS', 'FortAsyncAction_CurrentPlaylistReady FortAsyncAction_SendQuestStatEvent FortAsyncAction_WaitForClientAnnouncement')
add('UBFL', 'FortMissionLibrary')
add('UAIP', 'FortAIPerceptionComponent')
add('UAIS', 'AthenaTrapAIPerceptionStimuliSourceComponent')
add('ANSO', 'AthenaNavSystemConfigOverride', 'NavSystemConfigOverride actor, name inference only')
add('UNSC', 'AthenaNavSystemConfig')
add('APAWN', 'FortAIPawn FortPawn FortPlayerMannequin')
add('APC', 'FortPlayerController FortPlayerControllerFrontEnd')
add('AGM', 'FortGameModeFrontEnd FortGameModeMinimal FortGameModePvE')
add('AGS', 'FortGameStateAthena')
add('UGI', 'FortGameInstance')
add('AV', 'FortForcedLODZone FortPoiVolume FortUndergroundVolume GameplayEffectVolume')
add('AP', 'WaterVolume_Athena WaterVolume_Deep WaterVolume_Shallow', 'water volume, physics volume is a guess')
add('ACAM', 'FortPOICameraActor')
add('PS', 'FortPlayerStart FortPlayerStartCreative FortPlayerStartWarmup FortPvPPlayerStart')
add('INTERFACE', 'FortMissionInterface FortSpawnableByPlacementSystemInterface')

PREFIX_ORDER = None

def cpp_name(census, key):
    parent_name = PARENTS[key][0] if PARENTS[key][0] else 'UInterface'
    if key == 'INTERFACE':
        return 'U' + census
    return parent_name[0] + census

def main(census_list_path, out_h, out_cpp):
    census = [l.strip() for l in open(census_list_path) if l.strip()]
    missing = set(census)
    assigned = set(T)
    if missing != assigned:
        print('MISMATCH only-in-census:', sorted(missing - assigned), 'only-in-table:', sorted(assigned - missing), file=sys.stderr)
        sys.exit(1)

    lines = []
    lines.append('#pragma once')
    lines.append('')
    lines.append('// Census stubs for /Script/FortniteGame classes that the recovered maps import')
    lines.append('// (porting/generated/map-native-imports.jsonl, Class kind) and that the module did not')
    lines.append('// declare. STUBS ONLY: no Fortnite behavior, no serialized properties, and unknown')
    lines.append('// layouts. Every parent is guessed from the naming convention and UE engine parents')
    lines.append('// unless the note says otherwise. Function and enum imports are not declared here.')
    lines.append('')
    lines.append('#include "CoreMinimal.h"')
    lines.append('#include "UObject/Object.h"')
    lines.append('#include "UObject/Interface.h"')
    lines.append('#include "GameFramework/Actor.h"')
    lines.append('#include "Engine/StaticMeshActor.h"')
    lines.append('#include "GameFramework/Volume.h"')
    lines.append('#include "GameFramework/PhysicsVolume.h"')
    lines.append('#include "Camera/CameraActor.h"')
    lines.append('#include "GameFramework/Pawn.h"')
    lines.append('#include "GameFramework/PlayerController.h"')
    lines.append('#include "GameFramework/GameModeBase.h"')
    lines.append('#include "GameFramework/GameStateBase.h"')
    lines.append('#include "GameFramework/PlayerStart.h"')
    lines.append('#include "Engine/GameInstance.h"')
    lines.append('#include "Components/ActorComponent.h"')
    lines.append('#include "Components/AudioComponent.h"')
    lines.append('#include "Kismet/BlueprintAsyncActionBase.h"')
    lines.append('#include "Kismet/BlueprintFunctionLibrary.h"')
    lines.append('#include "Perception/AIPerceptionComponent.h"')
    lines.append('#include "Perception/AIPerceptionStimuliSourceComponent.h"')
    lines.append('#include "AI/NavigationSystemConfig.h"')
    lines.append('#include "NavSystemConfigOverride.h"')
    lines.append('#include "FortniteMapStubs.generated.h"')
    lines.append('')

    for census_name in census:
        key, note = T[census_name]
        cname = cpp_name(census_name, key)
        lines.append('// STUB: layout unknown, replace with generated class')
        if key == 'INTERFACE':
            lines.append('// Interface guessed from the census Class import name; unverified.')
            lines.append('UINTERFACE()')
            lines.append(f'class FORTNITEGAME_API U{census_name} : public UInterface')
            lines.append('{')
            lines.append('    GENERATED_BODY()')
            lines.append('};')
            lines.append(f'class FORTNITEGAME_API I{census_name}')
            lines.append('{')
            lines.append('    GENERATED_BODY()')
            lines.append('};')
            lines.append('')
            continue
        parent = PARENTS[key][0]
        guess = f'// Parent guessed: {parent}. ' + (note[0].upper() + note[1:] + '; unverified.' if note else 'Naming convention; unverified.')
        lines.append(guess)
        lines.append('UCLASS()')
        lines.append(f'class FORTNITEGAME_API {cname} : public {parent}')
        lines.append('{')
        lines.append('    GENERATED_BODY()')
        lines.append('};')
        lines.append('')

    open(out_h, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))

    cpp = [
        '#include "FortniteMapStubs.h"',
        '',
        '// Stub types only. Their layouts and behavior are unknown; see FortniteMapStubs.h.',
        '',
    ]
    open(out_cpp, 'w', encoding='utf-8', newline='\n').write('\n'.join(cpp))
    print('wrote', len(census), 'stub declarations')

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3])
