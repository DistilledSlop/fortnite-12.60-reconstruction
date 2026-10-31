# Native class promotion: status

Scope: 168 classes generated from the reflection tables into `porting/native/Source/FortniteGame/Generated/` (164) and
`porting/native/Source/PlayspaceFramework/Generated/` (4), compiled into `FortniteLinux`, `FortniteLinuxServer` and
`FortniteLinuxEditor`. The closure is `porting/native_class_set.txt` (21 round 1 classes, then 146 round 2 classes in
dependency order, plus `AGameplayVolume`). The join smoke test (`porting/server/smoke_join.sh`, direct mode) passes on the
final binaries.

**Wire compatibility: not yet.** 767 of 3,424 properties and 121 of 658 replicated properties are skipped, and 97 of 428
RPCs are not emitted. Both peers in our build agree with each other. Against the original client the `ClassChecksum`
differs for each class with a skipped replicated property, and skipped RPCs shift that class's RPC indices. See
"What blocks wire compatibility".

## Results by round

| Measure | Round 1 (21 classes) | Final (168 classes) |
|---|---|---|
| Properties emitted / total | 1,220 / 1,721 (884 skipped before this pass) | 2,657 / 3,424 (767 skipped) |
| Replicated emitted / total | 349 / 445 (171 skipped before) | 537 / 658 (121 skipped) |
| RPCs emitted / reflection | 271 / 376 (178 skipped before) | 331 / 428 (97 skipped) |
| Structs / enums / delegates emitted | 71 / 56 / 118 | 158 / 110 / 157 |

Round 1 skipped counts are the "before" values from the earlier pass (884 properties, 171 replicated, 178 RPCs) and the
round 1 closure after the type fixes (501 properties, 96 replicated, 105 RPCs). The final denominators are larger
because the closure is larger.

## Regenerate

```sh
porting/promote_native_classes.sh
```

Inputs are git-ignored under `porting/generated/reflection/`. Outputs are tracked under `native/Source/*/Generated/`.
The closure is `porting/native_class_set.txt`. `--promote` in `porting/gen_native_classes.py` checks it: each parent is
listed or is in `/Script/Engine` or `/Script/CoreUObject`. Engine parent headers are found under
`Runtime/Engine/Classes`, `Runtime/Engine/Public` and `Runtime/CoreUObject/Public`.

## Module path decision

The class path is part of the wire format, so the module name matters. The package of each class comes from
`class_parent_map.json` (the `FPackageParams` record that each block points to). It is not guessed.

- 164 classes are in `/Script/FortniteGame`, the module this project owns.
- 4 classes are in `/Script/PlayspaceFramework` (`APlayspaceGameState`, `APlayspacePlayerController`, `APlayspace`,
  `AGameplayVolume`). That package is a separate module, `native/Source/PlayspaceFramework`, and depends on `GameplayTags`
  for the gameplay tag types.

## Naming

UHT keeps the `A`/`U` prefix in the C++ name, but UObject names drop it. So `AFortPawn` is `/Script/FortniteGame.FortPawn`.
Clash checks between hand-written code and generated code must compare the stripped name.

## Classes

Columns: emitted / total. RPCs are net UFUNCTIONs (Server, Client, NetMulticast). Properties and replicated columns count
the class's own table. Other UFUNCTIONs are not emitted: the RPC indices come from the net fields, and `ClassChecksum`
covers replicated properties only (`CoreNet.cpp` 160-240).

| Class | Package | Parent | Properties emitted / total | Replicated emitted / total | RPCs emitted / reflection |
|---|---|---|---|---|---|
| AFortGameStateAthena | /Script/FortniteGame | AFortGameStateZone | 172 / 197 | 80 / 91 | 1 / 1 |
| AFortPlayerStateAthena | /Script/FortniteGame | AFortPlayerStateZone | 61 / 66 | 39 / 41 | 8 / 8 |
| AFortPlayerControllerAthena | /Script/FortniteGame | AFortPlayerControllerZone | 128 / 161 | 15 / 19 | 98 / 122 |
| AFortPlayerPawnAthena | /Script/FortniteGame | AFortPlayerPawn | 78 / 97 | 12 / 14 | 3 / 5 |
| AFortWorldSettings | /Script/FortniteGame | AWorldSettings | 43 / 49 | 0 / 0 | 0 / 0 |
| AFortAthenaMapInfo | /Script/FortniteGame | AActor | 29 / 35 | 0 / 0 | 0 / 0 |
| AFortPoiManager | /Script/FortniteGame | AActor | 17 / 17 | 4 / 4 | 0 / 0 |
| AFortWorldManager | /Script/FortniteGame | AInfo | 43 / 50 | 2 / 3 | 0 / 0 |
| AFortPlayerStateZone | /Script/FortniteGame | AFortPlayerState | 20 / 27 | 14 / 18 | 1 / 2 |
| AFortPlayerState | /Script/FortniteGame | APlayerState | 47 / 55 | 33 / 37 | 2 / 4 |
| AFortGameStateZone | /Script/FortniteGame | AFortGameState_InGame | 68 / 74 | 42 / 44 | 2 / 5 |
| AFortGameState_InGame | /Script/FortniteGame | AFortGameState | 0 / 0 | 0 / 0 | 0 / 0 |
| AFortGameState | /Script/FortniteGame | AFortGameStateBase | 41 / 48 | 24 / 27 | 7 / 7 |
| AFortGameStateBase | /Script/FortniteGame | APlayspaceGameState | 1 / 2 | 1 / 2 | 0 / 0 |
| AFortPlayerControllerZone | /Script/FortniteGame | AFortPlayerControllerGameplay | 23 / 29 | 2 / 3 | 22 / 26 |
| AFortPlayerControllerGameplay | /Script/FortniteGame | AFortPlayerController | 53 / 59 | 12 / 13 | 15 / 15 |
| AFortPlayerController | /Script/FortniteGame | APlayspacePlayerController | 215 / 266 | 28 / 31 | 103 / 135 |
| AFortPlayerPawn | /Script/FortniteGame | AFortPawn | 228 / 309 | 54 / 65 | 22 / 31 |
| AFortPawn | /Script/FortniteGame | ACharacter | 124 / 173 | 29 / 33 | 11 / 15 |
| APlayspaceGameState | /Script/PlayspaceFramework | AGameState | 3 / 5 | 0 / 0 | 0 / 0 |
| APlayspacePlayerController | /Script/PlayspaceFramework | APlayerController | 1 / 2 | 0 / 0 | 0 / 0 |
| AFortReplaySpectatorPawnBase | /Script/FortniteGame | ASpectatorPawn | 1 / 2 | 0 / 0 | 0 / 0 |
| ABattleMapPawnLive | /Script/FortniteGame | AFortReplaySpectatorPawnBase | 0 / 8 | 0 / 0 | 0 / 0 |
| ABuildingActor | /Script/FortniteGame | AActor | 126 / 146 | 17 / 22 | 0 / 0 |
| ABuildingGameplayActor | /Script/FortniteGame | ABuildingActor | 10 / 18 | 0 / 0 | 0 / 0 |
| ABuildingGameplayActorPlayerPropAttachment | /Script/FortniteGame | ABuildingGameplayActor | 0 / 0 | 0 / 0 | 0 / 0 |
| ABuildingItemCollectorActor | /Script/FortniteGame | ABuildingGameplayActor | 32 / 39 | 6 / 9 | 0 / 1 |
| ABuildingItemWeaponUpgradeActor | /Script/FortniteGame | ABuildingItemCollectorActor | 17 / 25 | 1 / 1 | 0 / 0 |
| ABuildingSMActor | /Script/FortniteGame | ABuildingActor | 106 / 136 | 21 / 25 | 12 / 12 |
| ABuildingTrap | /Script/FortniteGame | ABuildingSMActor | 21 / 34 | 2 / 5 | 0 / 0 |
| ABuildingTrapFloor | /Script/FortniteGame | ABuildingTrap | 0 / 0 | 0 / 0 | 0 / 0 |
| ABuildingTrapDefender | /Script/FortniteGame | ABuildingTrapFloor | 3 / 5 | 1 / 3 | 0 / 0 |
| AFortClientOnlyActor | /Script/FortniteGame | AActor | 0 / 0 | 0 / 0 | 0 / 0 |
| ABuildingWeakSpot | /Script/FortniteGame | AFortClientOnlyActor | 9 / 10 | 0 / 0 | 0 / 0 |
| ACustomCosmeticModifierBase | /Script/FortniteGame | AActor | 16 / 17 | 0 / 0 | 0 / 0 |
| ACustomCharacterPartModifier | /Script/FortniteGame | ACustomCosmeticModifierBase | 2 / 3 | 0 / 0 | 0 / 0 |
| AFortAIDirectorDataManager | /Script/FortniteGame | AActor | 3 / 4 | 0 / 0 | 0 / 0 |
| AFortAircraft | /Script/FortniteGame | AActor | 3 / 4 | 1 / 1 | 0 / 0 |
| AFortAthenaAircraft | /Script/FortniteGame | AFortAircraft | 18 / 23 | 8 / 8 | 0 / 0 |
| AFortAthenaCreativePortal | /Script/FortniteGame | ABuildingGameplayActor | 29 / 32 | 11 / 11 | 0 / 0 |
| AFortGameplayMutator | /Script/FortniteGame | AInfo | 4 / 4 | 1 / 1 | 0 / 0 |
| AFortAthenaMutator_AI | /Script/FortniteGame | AFortGameplayMutator | 0 / 1 | 0 / 0 | 0 / 0 |
| AFortBroadcastRemoteClientInfo | /Script/FortniteGame | AActor | 10 / 14 | 10 / 14 | 10 / 13 |
| AFortBroadcastSpectatorInfo | /Script/FortniteGame | AInfo | 4 / 4 | 4 / 4 | 0 / 0 |
| AFortCarriedObject | /Script/FortniteGame | AActor | 7 / 10 | 1 / 3 | 0 / 0 |
| AFortClientAnnouncementManager | /Script/FortniteGame | AActor | 1 / 4 | 1 / 2 | 0 / 0 |
| AFortCreativePortalManager | /Script/FortniteGame | AInfo | 1 / 2 | 0 / 0 | 0 / 0 |
| AFortCreativeRealEstatePlotManager | /Script/FortniteGame | AInfo | 0 / 1 | 0 / 1 | 0 / 0 |
| AFortDamageNumbersActor | /Script/FortniteGame | AFortClientOnlyActor | 29 / 33 | 0 / 0 | 0 / 0 |
| AFortDecoHelper | /Script/FortniteGame | AActor | 16 / 20 | 0 / 0 | 0 / 0 |
| AFortEmitterCameraLensEffectDirectional | /Script/FortniteGame | AEmitterCameraLensEffectBase | 4 / 5 | 0 / 0 | 0 / 0 |
| AFortFXManager | /Script/FortniteGame | AFortClientOnlyActor | 2 / 5 | 0 / 0 | 0 / 0 |
| AFortFeedbackManager | /Script/FortniteGame | AActor | 3 / 6 | 0 / 2 | 0 / 0 |
| AFortGlobalAbilityTargetingActor | /Script/FortniteGame | AInfo | 0 / 0 | 0 / 0 | 0 / 0 |
| AFortGlobalEnvironmentAbilityActor | /Script/FortniteGame | AInfo | 0 / 2 | 0 / 0 | 0 / 0 |
| AFortInGameMapManager | /Script/FortniteGame | AActor | 33 / 36 | 2 / 2 | 1 / 1 |
| AFortInventory | /Script/FortniteGame | AActor | 7 / 10 | 1 / 3 | 0 / 0 |
| AFortLocalizationService | /Script/FortniteGame | AInfo | 3 / 3 | 0 / 0 | 0 / 0 |
| AFortMarkActor | /Script/FortniteGame | AActor | 5 / 7 | 4 / 6 | 0 / 0 |
| AFortMissionManager | /Script/FortniteGame | AActor | 4 / 14 | 0 / 7 | 0 / 0 |
| AFortMissionState | /Script/FortniteGame | AActor | 3 / 5 | 0 / 0 | 0 / 0 |
| AFortMusicManager | /Script/FortniteGame | AActor | 2 / 5 | 0 / 0 | 0 / 0 |
| AFortObjectiveBase | /Script/FortniteGame | AFortMissionState | 15 / 18 | 6 / 6 | 0 / 0 |
| AFortPhysicsPawn | /Script/FortniteGame | APawn | 2 / 2 | 2 / 2 | 2 / 4 |
| AFortPlayerCharm | /Script/FortniteGame | AActor | 2 / 6 | 0 / 0 | 0 / 0 |
| AFortPlayerMarkerBase | /Script/FortniteGame | AActor | 2 / 5 | 0 / 0 | 0 / 0 |
| AFortPlayerParachute | /Script/FortniteGame | AActor | 12 / 23 | 0 / 2 | 0 / 0 |
| AFortPlayerPet | /Script/FortniteGame | AActor | 1 / 7 | 0 / 0 | 0 / 0 |
| AFortPlayerPetRepState | /Script/FortniteGame | AActor | 1 / 3 | 1 / 3 | 0 / 0 |
| AFortPlayerStartWarmup | /Script/FortniteGame | APlayerStart | 0 / 0 | 0 / 0 | 0 / 0 |
| AFortPoiVolume | /Script/FortniteGame | AVolume | 5 / 7 | 0 / 0 | 0 / 0 |
| AFortPropertyOverrideReplShared | /Script/FortniteGame | AInfo | 0 / 1 | 0 / 1 | 0 / 0 |
| AFortQuickBars | /Script/FortniteGame | AFortClientOnlyActor | 0 / 7 | 0 / 0 | 0 / 0 |
| AFortRemoteControlledPawnAthena | /Script/FortniteGame | AFortPlayerPawnAthena | 9 / 17 | 5 / 7 | 0 / 0 |
| AFortSafeZoneIndicator | /Script/FortniteGame | AActor | 46 / 51 | 17 / 17 | 0 / 0 |
| AFortSafeZoneStorm | /Script/FortniteGame | AActor | 1 / 1 | 0 / 0 | 0 / 0 |
| AFortScriptedActionManager | /Script/FortniteGame | AActor | 2 / 5 | 0 / 0 | 0 / 0 |
| AFortSkyTube | /Script/FortniteGame | AActor | 31 / 32 | 0 / 0 | 0 / 0 |
| AFortSpecialActorReplicationInfo | /Script/FortniteGame | AInfo | 1 / 2 | 0 / 1 | 0 / 0 |
| AFortSprayDecalInstance | /Script/FortniteGame | ABuildingSMActor | 4 / 5 | 0 / 1 | 0 / 0 |
| AFortTeamInfo | /Script/FortniteGame | AInfo | 3 / 5 | 3 / 4 | 0 / 0 |
| AFortTeamPrivateInfo | /Script/FortniteGame | AInfo | 2 / 4 | 1 / 3 | 0 / 0 |
| AFortThreatParticleActor | /Script/FortniteGame | AActor | 0 / 0 | 0 / 0 | 0 / 0 |
| AFortThreatVisualsManager | /Script/FortniteGame | AActor | 19 / 22 | 2 / 4 | 0 / 1 |
| AFortTimeOfDayManager | /Script/FortniteGame | AInfo | 66 / 80 | 5 / 6 | 1 / 2 |
| AFortVehicleManager | /Script/FortniteGame | AActor | 2 / 3 | 0 / 0 | 0 / 0 |
| AFortVisibilityManager | /Script/FortniteGame | AActor | 0 / 1 | 0 / 1 | 0 / 0 |
| AGameplayVolume | /Script/PlayspaceFramework | AActor | 4 / 4 | 0 / 0 | 0 / 0 |
| AFortVolume | /Script/FortniteGame | AGameplayVolume | 14 / 34 | 4 / 7 | 1 / 1 |
| AFortVolumeManager | /Script/FortniteGame | AInfo | 6 / 18 | 1 / 2 | 0 / 0 |
| AFortWindManager | /Script/FortniteGame | AActor | 20 / 25 | 0 / 0 | 0 / 0 |
| APhysicsObject | /Script/FortniteGame | AFortPhysicsPawn | 26 / 37 | 4 / 8 | 3 / 5 |
| AUnicornDriver | /Script/FortniteGame | AInfo | 30 / 32 | 0 / 0 | 0 / 0 |
| UAthenaMarkerComponent | /Script/FortniteGame | UActorComponent | 7 / 11 | 0 / 1 | 1 / 2 |
| UAthenaPlayerMatchReport | /Script/FortniteGame | UObject | 10 / 12 | 0 / 0 | 0 / 0 |
| UAthenaResurrectionComponent | /Script/FortniteGame | UActorComponent | 4 / 6 | 1 / 2 | 2 / 4 |
| UBaseStatManager | /Script/FortniteGame | UObject | 1 / 3 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata | /Script/FortniteGame | UDataAsset | 4 / 6 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata_BinaryToggle | /Script/FortniteGame | UBuildingEditModeMetadata | 1 / 1 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata_Floor | /Script/FortniteGame | UBuildingEditModeMetadata_BinaryToggle | 0 / 0 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata_Roof | /Script/FortniteGame | UBuildingEditModeMetadata_BinaryToggle | 0 / 1 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata_Stair | /Script/FortniteGame | UBuildingEditModeMetadata | 1 / 1 | 0 / 0 | 0 / 0 |
| UBuildingEditModeMetadata_Wall | /Script/FortniteGame | UBuildingEditModeMetadata_BinaryToggle | 0 / 0 | 0 / 0 | 0 / 0 |
| UBuildingStructuralSupportSystem | /Script/FortniteGame | UObject | 0 / 3 | 0 / 0 | 0 / 0 |
| UCreativeQuestManager | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UCreativeQuickbarComponent | /Script/FortniteGame | UActorComponent | 9 / 12 | 0 / 0 | 0 / 3 |
| UCreativeUserContentManager | /Script/FortniteGame | UActorComponent | 1 / 3 | 0 / 0 | 0 / 0 |
| UCustomCharacterPart | /Script/FortniteGame | UPrimaryDataAsset | 18 / 24 | 0 / 0 | 0 / 0 |
| UCustomColorComponent | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UCustomColorSwatch | /Script/FortniteGame | UPrimaryDataAsset | 2 / 2 | 0 / 0 | 0 / 0 |
| UCustomPlayerComponent | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortItem | /Script/FortniteGame | UObject | 4 / 4 | 0 / 0 | 0 / 0 |
| UFortPersistableItem | /Script/FortniteGame | UFortItem | 4 / 5 | 0 / 0 | 0 / 0 |
| UFortAccountItem | /Script/FortniteGame | UFortPersistableItem | 8 / 8 | 0 / 0 | 0 / 0 |
| UFortAmbientAudioController | /Script/FortniteGame | UActorComponent | 3 / 9 | 0 / 0 | 0 / 0 |
| UFortAnimNotifyState_RootMotionInterrupt | /Script/FortniteGame | UAnimNotifyState | 2 / 2 | 0 / 0 | 0 / 0 |
| UFortCameraMode | /Script/FortniteGame | UObject | 8 / 9 | 0 / 0 | 0 / 0 |
| UFortCharacter | /Script/FortniteGame | UFortAccountItem | 2 / 2 | 0 / 0 | 0 / 0 |
| UFortCharacterCosmeticManager | /Script/FortniteGame | UWorldSubsystem | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortCinematicCamera | /Script/FortniteGame | UFortCameraMode | 0 / 1 | 0 / 0 | 0 / 0 |
| UFortClientBotManager | /Script/FortniteGame | UObject | 41 / 45 | 0 / 0 | 0 / 0 |
| UFortControllerComponent_LocalizationService | /Script/FortniteGame | UActorComponent | 0 / 0 | 0 / 0 | 2 / 2 |
| UFortCreativeDeviceButtonComponent | /Script/FortniteGame | UActorComponent | 6 / 7 | 0 / 0 | 0 / 0 |
| UFortCreativeObjectTrackingComponent | /Script/FortniteGame | UActorComponent | 0 / 0 | 0 / 0 | 1 / 1 |
| UFortCreativeOptionsBundle | /Script/FortniteGame | UDataAsset | 0 / 1 | 0 / 0 | 0 / 0 |
| UFortCustomizationAssetLoader | /Script/FortniteGame | UObject | 0 / 3 | 0 / 0 | 0 / 0 |
| UFortFeedbackBank | /Script/FortniteGame | UPrimaryDataAsset | 0 / 2 | 0 / 0 | 0 / 0 |
| UFortFootstepAudioBank | /Script/FortniteGame | UDataAsset | 9 / 11 | 0 / 0 | 0 / 0 |
| UFortGamepadSettings | /Script/FortniteGame | UDataAsset | 33 / 34 | 0 / 0 | 0 / 0 |
| UFortGameplayMessageComponentBase | /Script/FortniteGame | UActorComponent | 6 / 7 | 1 / 1 | 0 / 0 |
| UFortGenericRecord | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortIndicator | /Script/FortniteGame | UObject | 4 / 4 | 0 / 0 | 0 / 0 |
| UFortSlateHUDIndicator | /Script/FortniteGame | UFortIndicator | 2 / 2 | 0 / 0 | 0 / 0 |
| UFortHealthBarIndicator | /Script/FortniteGame | UFortSlateHUDIndicator | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortWorker | /Script/FortniteGame | UFortCharacter | 8 / 9 | 0 / 0 | 0 / 0 |
| UFortHero | /Script/FortniteGame | UFortWorker | 6 / 10 | 0 / 0 | 0 / 0 |
| UFortIndicatorManager | /Script/FortniteGame | UObject | 1 / 2 | 0 / 0 | 0 / 0 |
| UFortLevelOverlayManager | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortMiniMapIndicator | /Script/FortniteGame | UFortIndicator | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortMiniMapChallengeIndicators | /Script/FortniteGame | UFortMiniMapIndicator | 0 / 2 | 0 / 0 | 0 / 0 |
| UFortMiniMapTeamIndicators | /Script/FortniteGame | UFortMiniMapIndicator | 14 / 17 | 0 / 0 | 0 / 0 |
| UFortMissionGenerator | /Script/FortniteGame | UPrimaryDataAsset | 8 / 16 | 0 / 0 | 0 / 0 |
| UFortMobileInteractionComponent | /Script/FortniteGame | UStaticMeshComponent | 1 / 1 | 0 / 0 | 0 / 0 |
| UFortMovementComp_Character | /Script/FortniteGame | UCharacterMovementComponent | 51 / 52 | 0 / 0 | 0 / 0 |
| UFortMusicManagerBank | /Script/FortniteGame | UDataAsset | 0 / 2 | 0 / 0 | 0 / 0 |
| UFortMutatorListComponent | /Script/FortniteGame | UActorComponent | 3 / 9 | 0 / 1 | 0 / 0 |
| UFortPawnScriptedBehavior | /Script/FortniteGame | UObject | 5 / 5 | 0 / 0 | 0 / 0 |
| UFortRegisteredPlayerInfo | /Script/FortniteGame | UObject | 20 / 36 | 0 / 0 | 0 / 0 |
| UFortSkinWeightOverrideManager | /Script/FortniteGame | UObject | 0 / 0 | 0 / 0 | 0 / 0 |
| UFortStatEventManager | /Script/FortniteGame | UObject | 0 / 3 | 0 / 0 | 0 / 0 |
| UFortStormShieldComponent | /Script/FortniteGame | USceneComponent | 4 / 4 | 1 / 1 | 0 / 0 |
| UFortSupplyDropInfo | /Script/FortniteGame | UDataAsset | 24 / 25 | 0 / 0 | 0 / 0 |
| UFortSwimmingAudioBank | /Script/FortniteGame | UDataAsset | 3 / 5 | 0 / 0 | 0 / 0 |
| UFortUnderwaterDamageComponent | /Script/FortniteGame | UActorComponent | 7 / 9 | 0 / 0 | 0 / 0 |
| UFortUserCloudHelperComponent | /Script/FortniteGame | UActorComponent | 1 / 1 | 0 / 0 | 0 / 0 |
| UFortWeaponAnimSet | /Script/FortniteGame | UDataAsset | 1 / 3 | 0 / 0 | 0 / 0 |
| UFortWorldRecord | /Script/FortniteGame | UFortGenericRecord | 4 / 5 | 0 / 0 | 0 / 0 |
| UFortZoneRecord | /Script/FortniteGame | UFortGenericRecord | 9 / 9 | 0 / 0 | 0 / 0 |
| UFortZoneTheme | /Script/FortniteGame | UPrimaryDataAsset | 20 / 29 | 0 / 0 | 0 / 0 |
| UHeartbeatManager | /Script/FortniteGame | UObject | 4 / 5 | 0 / 0 | 0 / 0 |
| UPegasusGameEventCollector | /Script/FortniteGame | UActorComponent | 8 / 8 | 0 / 0 | 0 / 0 |
| UQuickHealItemPicker | /Script/FortniteGame | UObject | 3 / 3 | 0 / 0 | 0 / 0 |
| UStatManager | /Script/FortniteGame | UBaseStatManager | 0 / 0 | 0 / 0 | 0 / 0 |
| UUnicornAthenaPawnSampler | /Script/FortniteGame | UActorComponent | 1 / 1 | 0 / 0 | 0 / 0 |
| UWeaponHitNotifyAudioBank | /Script/FortniteGame | UDataAsset | 0 / 11 | 0 / 0 | 0 / 0 |
| UWorldTheme | /Script/FortniteGame | UDataAsset | 1 / 16 | 0 / 0 | 0 / 0 |
| UWorldTileManager | /Script/FortniteGame | UObject | 0 / 1 | 0 / 0 | 0 / 0 |
| APlayspace | /Script/PlayspaceFramework | AInfo | 5 / 9 | 0 / 2 | 0 / 0 |

## Blocked classes (47 targets not generated)

These are referenced by a skipped property or RPC, but their module or an ancestor is not available:

- `McpProfileSys` (not in the engine tree; `UMcpItemDefinitionBase`, `UMcpProfile`, `UMcpProfileGroup`): 17 targets, including
  the item-definition chain (`UFortHeroType`, `UAthenaGliderItemDefinition`, `UAthenaBattleBusItemDefinition`,
  `UFortPlaysetItemDefinition`, `UFortAwardItemDefinition`, and others) and the `UFortMcpProfile*` classes.
- `ModularGameplay` (`UControllerComponent`, `UGameFrameworkComponent`, `UGameStateComponent`): 13 controller and game
  state components (`UFortControllerComponent_*`, `UFortGameStateComponent_*`, `UFortPlayerControllerAthenaXPComponent`).
- `GameplayAbilities` and `GameplayTasks` (engine plugin, enabled in the project, not promoted): `UFortAbilitySystemComponent`,
  `UFortAdvancedMovementSet`, `UFortControlResistanceSet`, `UFortDamageSet`, `UFortMovementSet` (`UAttributeSet` parent),
  `UGameplayAbility` and `UGameplayEffect` (references only). The generator does not yet accept these packages as parents.
- `Water` (`AWaterBody`): `AFortWaterBodyActor`. The plugin is enabled but the generator does not accept its package.
- `ClientPilot`, `Lobby`, `MediaAssets`, `Niagara`, `AnimationBudgetAllocator`: not in the project's module set
  (`UFortClientPilot_Base`, `AFortLobbyBeaconState`, `UMediaSource`, `UMediaPlayer`, `UNiagaraSystem`,
  `USkeletalMeshComponentBudgeted`).
- `PlayspaceFramework` components with a `ModularGameplay` parent: `UPlayspaceControllerComponent_PlayerSpawning`,
  `UPlayspaceGameStateComponent_PlayerSpawningManager`, `UPlayspaceManagerComponent`.

## Skipped items

Skipped properties are recorded per class and per property in `porting/generated/reflection/skipped_properties.json`
(fields: `class`, `property`, `kind`, `category`, `replicated`, `reason`, `tokens`).

Final skipped properties by category (767 total, 121 replicated): `class_unnamed` 332, `class_outside_native_set` 209,
`struct_member_unresolved` 133, `delegate_member_unresolved` 78, `struct_custom_netserialize` 9, `container_element` 3,
`interface` 2, `enum_not_available` 1. Skipped RPCs (97 total): `class_unnamed` 62, `struct_member_unresolved` 16,
`class_outside_native_set` 14, `struct_custom_netserialize` 4, `struct_not_available` 1.

Custom NetSerialize structs (Fort package): `AthenaPawnReplayData`, `CreateBuildingActorData`,
`FortAIAttributeReplicationProxy`, `FortAISharedRepMovement`, `FortGameplayAbilityTargetData_SingleTargetHit`,
`FortGameplayEffectContext`, `FortRootMotionSource_FallingBoost`, `ReplicatedPhysicsPawnState`, `SharedRepMovement`. A replicated
property or RPC that uses one of them, directly or inside another struct, is skipped (category
`struct_custom_netserialize`). Their bodies are not recovered.

## What blocks wire compatibility

1. **Class references that are unnamed (`class_unnamed`, 332 properties, 62 RPCs).** `resolve_class_refs.py` resolves
   2,326 of 2,622 distinct class references. The 296 unresolved targets are singleton builders with no class-block
   reference. They are the largest block.
2. **Classes outside the set (`class_outside_native_set`, 209 properties, 14 RPCs).** Mostly the blocked classes above.
   Each new class also adds references, so this number moved while the set grew.
3. **Struct member types (`struct_member_unresolved`, 133 properties, 16 RPCs).** Mostly structs that use the unnamed or
   blocked classes, and the 9 custom NetSerialize structs.
4. **Delegate parameters (`delegate_member_unresolved`, 78 properties).** Delegates do not replicate, so they change the class
   layout only.
5. **Replication conditions.** `DOREPLIFETIME` is used without conditions. `COND_*` is not recovered.
6. **Bodies.** No function bodies or constructor logic are recovered. Each emitted RPC `_Implementation` and each RepNotify
   is an empty stub.

## Hand-written code changed

- Stubs removed from `FortniteMapStubs.h` because they are now generated: `ABuildingActor`, `ABuildingGameplayActor`,
  `ABuildingSMActor`, `AFortPlayerStartWarmup`, `AFortPoiVolume`, `AFortTimeOfDayManager`, `UFortFeedbackManager` (and earlier
  `AFortGameStateAthena`, `AFortPlayerController`, `AFortPawn`).
- `AFortSafeZoneStorm` and `UWorldTileManager` removed from `FortniteMapClasses.h`. `FortniteApolloBlueprintCommandlet.cpp`
  includes the generated `AFortSafeZoneStorm.h`.
- `gen_stubs.py` `GENERATED_NATIVE` lists every generated class name without its prefix letter, so a regeneration
  does not stub them again.
- `FortniteGame.Build.cs` depends on `GameplayAbilities`, `GameplayTags`, `SlateCore`, `UMG` and `JsonUtilities`.
  `PlayspaceFramework.Build.cs` depends on `GameplayTags`. `FortniteLinux.uproject` enables `GameplayAbilities`.
- `AFortPlayerPawn` lost its hand-written camera, input bindings and visual probe. The generated pawn has no input or camera.
- `AFortGameModeAthena` keeps its Apollo stand-in behaviour and still derives from `AGameModeBase`.

## Build and smoke

- Builds: `FortniteLinux`, `FortniteLinuxServer` and `FortniteLinuxEditor` all exit 0 on the final sources. 14 build
  iterations were used: 1-3 failed in UHT (engine plugin not enabled, delegate headers without `UDELEGATE()`), 4 failed in C++
  (enum RPC parameter spelling), 5-8 in round 2 (stub clash, and UHT/C++ module dependencies), 9-10 succeeded
  (round 2 client and server), 11 failed in the editor (commandlet include, PlayspaceFramework GameplayTags link), 12 editor
  success, 13-14 rebuilt the client and server (no relink needed).
- Staging: the two binaries are copied into `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/`.
  No cook and no `BuildCookRun`.
- Smoke: `porting/server/smoke_join.sh` in direct mode, with `rg` shimmed by `grep -a` on `PATH` and `stdbuf -oL`. Exit 0.
  `ss -lun` shows `UNCONN 0.0.0.0:7777`. The server logs `Local network version checksum: 683914758 (0x28C3B606)`,
  `Join succeeded: vibrato-43A0C0CF8424` and `Athena pawn spawned: class=FortPlayerPawnAthena authority=1`. The client logs
  `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated, Game: /Script/FortniteGame.FortGameModeAthena)`.
- Not run: the Apollo branch (`Apollo_Terrain`, probes, terrain and background grid counts).

## Next steps

1. Add engine plugin packages that are enabled in the project (`GameplayAbilities`, `GameplayTasks`, `Water`,
   `ModularGameplay`, `MediaAssets`, `Niagara` if enabled) to the generator's accepted packages, with their parent includes and
   module dependencies. This unblocks the `UFortAbilitySystemComponent` RPCs and the controller components.
2. Identify the 296 unresolved singleton builders behind `class_unnamed` (332 properties, 62 RPCs). This is the largest
   remaining block for wire compatibility.
3. Decide the McpProfileSys item-definition chain: it is not in the engine tree, so it needs a source for that plugin or a
   documented permanent skip. The generator must not invent a parent.
4. Recover the bodies of the 9 custom NetSerialize structs, or keep their replicated uses skipped.
5. Recover replication conditions (`COND_*`) and add `DOREPLIFETIME_CONDITION` where needed.
6. Check each class against the original `ClassChecksum` (`original_client_net_protocol.md` section 2).
7. Recover function bodies for the RPC and RepNotify stubs, starting with the Athena join path.

## Files

Tracked and changed: `porting/gen_native_classes.py`, `porting/gen_stubs.py`, `porting/native_class_set.txt`,
`porting/promote_native_classes.sh`, `porting/native_classes_status.md` (this file), `porting/native/FortniteLinux.uproject`,
`porting/native/Source/FortniteGame/` (`Generated/`, `FortniteGame.Build.cs`, `Public/FortniteMapStubs.h`,
`Public/FortniteMapClasses.h`), `porting/native/Source/PlayspaceFramework/` (`Generated/`, `PlayspaceFramework.Build.cs`),
`porting/native/Source/FortnitePortEditor/Private/FortniteApolloBlueprintCommandlet.cpp`.

Git-ignored outputs: `porting/generated/reflection/fortnite_game_reflection.json`, `class_ref_map.json`,
`skipped_properties.json`, `native_classes_index.json`, `type_closure.json`.
