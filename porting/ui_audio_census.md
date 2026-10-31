# Frontend/lobby UI and audio census

Analysis as of 2026-10-31 (rerun after the AES-key closures; the 2026-10-30 figures
are superseded where they differ). No UE build, no staged-game run, no pak read by
this script, and no change under `porting/native/`. Regenerate with:

```sh
python3 porting/census_ui_audio.py                    # full run, about 40 s
python3 porting/census_ui_audio.py --skip-tool        # reuse cached package imports
```

Outputs stay ignored: `porting/generated/ui-audio-census.json` (summary and all
tables) and `porting/generated/ui-audio/package-imports.jsonl` (per-package
import rows from the loose cue4inspect pass). The script reads only local files
and never prompts for a key.

## Scope and limits

- **Maps:** 3,694 maps, native (`/Script`) imports only. A map's `/Game` asset
  imports are counts, not names. The primary-key scan covers 28 paks.
- **Packages:** 4,680 recovered `.uasset` files after the round-2 closures (457
  before the key was used). 4,494 parsed; 186 did not parse and are excluded from the
  type counts below.
- **Pakchunk 1000-1002:** the census's primary-key scan still skips these GUID-keyed
  paks. Per-pak attribution comes from `porting/generated/pak-sources.tsv` (450,978
  rows, one per `.uasset` path, from the all-keys mount; 27 packages appear in both
  `pakchunk0` and `pakChunkEarly`). The provider holds 8 packages for `pakchunk1000`,
  47 for `1001` and 140 for `1002`. None of the 4,680 recovered packages comes from
  these three paks, so the closures reached no content there.
- **Widgets:** the registry has 2,175 WidgetBlueprints. 300 recovered packages are
  widget-related under the census heuristic (under `UI/`, or importing UMG, CommonUI
  or FortniteUI); this includes some gameplay Blueprints. 20 export a
  `UWidgetBlueprintGeneratedClass`. The rest of the registry is outside every closure.
- **Sounds:** the registry has 106,362 SoundWave assets. 641 sound packages are
  recovered (census `recovered_sound_packages`). 30 export a `USoundWave`, so SoundWave
  payloads are almost absent.
- **Import counts:** this census excludes `Package` and `Default__` rows, so the
  BP_BannerEditorTile count (52) differs from the 77 in `asset_compatibility.md`.

## Key findings

Findings 1 to 6 are the 2026-10-30 readings. The 2026-10-31 rerun changes counts and
the unresolved-native list: see "Native classes the Frontend graph still needs" at the
end of this file.

1. **The entry Frontend map has no UI natives of its own.** `Maps/Frontend.umap`
   has 48 native imports, none from UMG, CommonUI or FortniteUI. Its 290 asset
   imports, which hold the actual menu widgets and cues, need the key (below).
   This is the largest gap in the census.
2. **CommonUI is the frontend's main missing native module.** Its plugin is enabled
   in `FortniteGame.uproject` but absent from the engine tree. It is used by
   `BP_BannerEditorTile` (11 imports) and `Maps/FortniteTransition.umap`, which
   imports `CommonActivatableWidget` and `ActivateWidget`.
3. **FortniteUI and the frontend game classes have no recovered source.** Only
   `porting/native/Source/FortniteGame` has any classes. The five frontend
   FortniteGame classes already there are FortWorldSettings, FortLevelScriptActor,
   FortStaticMeshActor, FortMetaNavArea_Obstacles and FortKismetLibrary. None of the
   frontend-only project classes listed below is reconstructed yet.
4. **Engine UMG is present.** `UserWidget`, `WidgetBlueprintLibrary`,
   `WidgetTree`, `Overlay`, `SizeBox` and the list-entry classes are in
   `Engine/Source/Runtime/UMG`. `WidgetComponent` appears in 119 maps.
5. **Sound classes and mixes need no native reimplementation.** Every audio class
   imported by the 311 recovered sound packages (`SoundClass`, `SoundMix`,
   `SoundCue`, `SoundNodeWavePlayer`, `SoundAttenuation`, `AudioComponent`, and
   the sound-node types) is in the engine tree. The gap is content (SoundWave
   payloads) and the audio backend, not classes.
6. **No audio output is possible on this host yet.** The HDA card is registered
   but exposes no PCM device, and no sound server is running. Without `-nosound`,
   UE 4.25 should disable audio and keep running (by code reading; see Audio backend).

## Native classes needed by the frontend

Frontend maps: 44 (`porting/generated/ui-audio-census.json`, `frontend_maps`).
Native imports across them: 171 unique. Unresolved (no engine, plugin or porting
source), ranked by map count:

| Rank | Import | Kind | Maps | Home | Status |
|---|---|---|---|---|---|
| 1 | FortniteGame.FortNavigationVisibilityComponent | Class | 11 | FortniteGame | not recovered |
| 2 | FortniteGame.FortGameModeFrontEnd | Class | 2 | FortniteGame | not recovered |
| 3 | FortniteGame.FortPlayerController | Class | 2 | FortniteGame | not recovered (gameplay) |
| 4 | FortniteGame.FortQuestManager | Class | 2 | FortniteGame | not recovered |
| 5 | FortniteGame.FortAsyncAction_SendQuestStatEvent | Class | 2 | FortniteGame | not recovered |
| 6 | FortniteGame.SendClientStatEvent | Function | 2 | FortniteGame | not recovered |
| 7 | FortniteGame.FortBangCheckComponent_{Map,Customization,Donut,Elevator,Faction} | Class | 1-2 each | FortniteGame | not recovered |
| 8 | FortniteGame.FortPlayerControllerFrontEnd | Class | 1 | FortniteGame | not recovered |
| 9 | FortniteGame.FortPlayerMannequin | Class | 1 | FortniteGame | not recovered |
| 10 | FortniteGame.FrontEndSettings | Class | 1 | FortniteGame | not recovered |
| 11 | FortniteGame.EFrontEndCamera | Enum | 1 | FortniteGame | not recovered |
| 12 | FortniteGame.FortGameModeMinimal | Class | 1 | FortniteGame | not recovered |
| 13 | FortniteUI.FortFrontEndContext | Class | 1 | FortniteUI | not recovered |
| 14 | FortniteGame: GetFirstLocalFortPlayerController, GetLocalFortPlayerControllers, GetPartSkeletalMeshComponent, OnMatchStarted, RunFacialAnimationTest | Function | 1 each | FortniteGame | not recovered |

Resolved frontend natives are all engine classes (Engine, CoreUObject, UMG,
NavigationSystem, Foliage, MovieScene, LevelSequence, CinematicCamera,
ClothingSystemRuntimeNv, ChaosCloth, Niagara) or the five porting classes above.

## UI native modules in all maps

| Rank | Import | Maps | Status |
|---|---|---|---|
| 1 | UMG.WidgetComponent | 119 | present (engine UMG) |
| 2 | CommonUI.CommonActivatableWidget, CommonUI.ActivateWidget | 1 (FortniteTransition) | CommonUI plugin absent |
| 3 | FortniteUI.FortActivatableMovieWidget_Monolithic, FortniteUI.LoadStreamingVideo | 1 (FortniteTransition) | FortniteUI source absent |
| 4 | FortniteUI.FortGlobalUIContext, FortniteUI.SetRatingWidgetFeedbackTitle | 1 (Zone_Survival_Winter_Temperate) | FortniteUI source absent |
| 5 | FortniteUI.FortFrontEndContext | 1 (Frontend_HeroLoadout) | FortniteUI source absent |
| 6 | UMG.UserWidget, UMG.WidgetBlueprintLibrary, UMG.Create, UMG.AddToViewport | many | present (engine) |

## Widget Blueprint imports (recovered)

`BP_BannerEditorTile` (`FortniteGame/Content/UI/Foundation/Profile/`) has 52 native
class, function, struct and enum imports:

| Module | Count | Status | Names |
|---|---|---|---|
| UMG | 18 | present | Image, NamedSlot, Overlay, SizeBox, WidgetSwitcher, UserObjectListEntry, WidgetTree, ... |
| CommonUI | 11 | CommonUI absent | CommonActionWidget, CommonBorder, CommonButton, CommonLoadGuard, CommonTextBlock, CommonWidgetSwitcher, LoadGuardSlot; functions BP_OnHovered, BP_OnSelected, BP_GuardAndLoadAsset; delegate OnAssetLoaded |
| FortniteGame | 9 | not recovered | FortAccountItem, FortBannerUtilities, FortHomebaseBannerColorItem, FortHomebaseBannerIconItem; banner color and icon functions |
| Engine | 8 | present | KismetMathLibrary, KismetSystemLibrary, MaterialInstanceDynamic, Texture2D, ... |
| FortniteUI | 3 | not recovered | FortBangWrapper_NUI, FortPlayerProfileBannerEditorTile, SetBangStateBP |
| CoreUObject | 3 | present | Object, Function, LinearColor |

The other three recovered packages (`SolidWhite`, `T_UI_Icon_Mic_64`, and the
landscape Blueprint `BP_SetupNamedPoiColors`) are not frontend widgets.
`BP_SetupNamedPoiColors` imports UMG in a gameplay Blueprint.

## Audio natives and content

Recovered sound packages: 311 (`FortSoundClassesAndModes` 278 + `Fort_BossFight_SoundClass`
14, `Athena/Sounds/Events/SoundClasses` 14, `Athena/Sounds/Events/RiskyReels` 3,
`Sounds/Attenuation` 1, `Sounds/Fort_Doors` 1).

| Class | Packages importing | Status |
|---|---|---|
| Engine.SoundClass | 188 (export type USoundClass in 188) | present |
| Engine.SoundMix | 119 (121 exports typed UObject, probably mixes) | present |
| Engine.SoundCue | 2 | present |
| Engine.SoundNodeWavePlayer | 2 | present |
| Engine.SoundNodeRandom, SoundNodeModulator | 1 each | present |
| Engine.SoundAttenuation, AudioComponent | 1 each | present |

Gameplay audio natives, outside the frontend: FortCollisionAudioComponent (40 maps),
FortVehicleAudioVoice (40), FortSplineWaterAudioComponent (7), and
AmbientAudio.AmbientVolume (2 maps, plugin absent).

## Plugins required for UI and audio

Of 123 enabled plugins, 63 are absent from `/opt/fortnite/UnrealEngine/Engine/Plugins`.
This matches the README. The rows below are the UI and audio relevant ones,
plus the absent plugins that maps import.

| Plugin | Engine | Evidence | Relevance |
|---|---|---|---|
| CommonUI | absent | BP_BannerEditorTile (11), FortniteTransition (1 map) | required for frontend |
| NetUI | absent | none observed | likely widget networking, verify after closure |
| SocialUMG | absent | none observed (UI/SocialPanel textures recovered) | social panel, verify |
| SubtitlesWidgets | absent | none observed | subtitles, defer |
| AmbientAudio | absent | Apollo_Audio, Apollo_Papaya_Audio (2 maps) | gameplay audio |
| FortSoundCueTemplates | absent | none observed | sound templates, defer |
| VoiceChatManager, GVoiceChat, LiveStreamVoice, VoiceEmbeddedCommunication | absent | none observed | voice, defer |
| Water | absent | 76 maps | not UI or audio; map blocker |
| MeshNetwork | absent | 9 maps | not UI or audio |
| BlueprintContext | absent | Zone_Survival (1 map) | not frontend |
| ResonanceAudio, AudioSynesthesia, SoundCueTemplates, Synthesis, TimeSynth | present | none observed | available if needed |
| ImagePlate, Paper2D, MediaCompositing, MfMedia | present | none observed | available |

## Audio backend on this Linux box

- **Engine config:** `Config/Unix/UnixEngine.ini` sets `AudioDeviceModuleName=AudioMixerSDL`
  and `AudioMixerModuleName=AudioMixerSDL`. Linux therefore uses SDL2, not OpenAL.
- **SDL in the binary:** `Source/ThirdParty/SDL2/SDL-gui-backend/` is SDL 2.0.10,
  linked statically. `FortniteLinux` has no `NEEDED` entry for libSDL2 or
  libasound. Its strings include `SDL_OpenAudioDevice`, `SDL_AUDIODRIVER` and
  `SDL_DISKAUDIOFILE`.
- **Drivers built in:** `libSDL2.a` contains ALSA, PulseAudio, dummy and disk
  drivers. It contains no PipeWire or JACK driver.
- **OpenAL:** the engine bundles `libopenal.so` for Linux. No config or
  `.uproject` entry selects `ALAudio`, so it is unused.
- **Host:** `/proc/asound/cards` lists card 0 (HDA-Intel), but `/proc/asound/pcm` is empty
  and `/dev/snd` has no `pcm*` node. No pulseaudio, pipewire or wireplumber process
  is running. Host libraries exist (libasound 1.2.15, libpulse, libpipewire 1.6,
  system SDL2 2.32.10, libopenal 1.25.1), but there is nothing to talk to.
- **Failure mode:** `FAudioDeviceManager::Initialize` fails when SDL cannot open a
  device. The engine logs "Audio device could not be initialized" and shuts the audio
  manager down (`AudioDeviceManager.cpp:385`, `UnrealEngine.cpp:2918-2925`). The run then
  continues without audio. By code reading (not yet run here), a missing audio device
  does not fail a probe.

Headless options:

| Option | Works here | Use |
|---|---|---|
| `-nosound` | yes | explicit no-audio load tests (all earlier probes) |
| no flag, no device | no, silent fallback | do not rely on it; check the log for "Audio device could not be initialized" |
| `SDL_AUDIODRIVER=dummy` | yes | mixer runs, no output; checks init and timing |
| `SDL_AUDIODRIVER=disk SDL_DISKAUDIOFILE=<scratch>/out.raw` | yes | captures mixed PCM for verification; delete the file afterwards |
| `alsa`, `pulseaudio` | no | no PCM device; no server |
| a PipeWire or PulseAudio null sink | no, not started | needs a user-session change, so it is a human decision |

## Recommended implementation order

1. **Frontend widget graph (key-gated).** Recover the closures of `Maps/Frontend`
   and the lobby maps (commands below), then rerun the census. This turns the 290
   asset imports into named widgets, sounds and classes, and is the prerequisite
   for every later item.
2. **CommonUI native module.** Reconstruct the classes and functions listed above as
   a reflection-compatible module. Start with `BP_BannerEditorTile`, which previously
   stopped at `CommonButton:BP_OnHovered` with exit code 139. Then `FortniteTransition`.
3. **FortniteUI natives.** FortFrontEndContext, FortGlobalUIContext,
   FortActivatableMovieWidget_Monolithic, FortBangWrapper_NUI,
   FortPlayerProfileBannerEditorTile, and their functions.
4. **Frontend game classes.** FortGameModeFrontEnd, FortPlayerControllerFrontEnd,
   FrontEndSettings, EFrontEndCamera, FortPlayerMannequin, FortGameModeMinimal, plus
   FortNavigationVisibilityComponent, which all 11 frontend maps that import it
   need (ten BP12 hero rooms and Frontend_Lobby_Background).
5. **Audio verification, no key.** Load all 311 recovered sound packages under
   `-nullrhi -nosound` (proven for one). Then repeat with
   `SDL_AUDIODRIVER=disk` and check the captured file has non-zero samples. This
   fixes `audio.sound_classes` before any content is added.
6. **Sound content.** SoundWave payloads and cues referenced from the frontend
   closure. Lobby music and menu cues come first.
7. **Lobby flow.** `Frontend_Lobby_Default` (102 asset imports, UMG natives) first,
   then the season lobbies, which have the largest asset graphs.
8. **Voice and absent audio plugins.** Defer. No frontend map imports them.

## Commands that need the AES key

The commands below need the primary key plus three GUID keys (`--all-keys`). Keys
are passed only through `--key-file=<path>`, never printed or copied.

```sh
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/Frontend --save-closure --max-packages=6000 --all-keys --key-file=<path>
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/FortniteTransition --save-closure --max-packages=3000 --all-keys --key-file=<path>
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/UI/Frontend_Lobby_Default --save-closure --max-packages=6000 --all-keys --key-file=<path>
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/UI/Frontend_Lobby_StW_Season11 --save-closure --max-packages=6000 --all-keys --key-file=<path>
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll --pak-sources --all-keys --key-file=<path>   # writes generated/pak-sources.tsv
python3 porting/census_ui_audio.py                    # reads extracted/ and pak-sources.tsv; no key
```

Closures write to `porting/generated/extracted/` (git-ignored; new closures merge
into the same tree and overwrite identical paths). A closure that stops at its cap
prints `pending` above zero and exits 1. `--pak-sources` writes one row per recovered
`.uasset` path with the pak whose entry the provider serves. It is a read-only
index and writes a derived file under `porting/generated/`.

Round 1 (caps 400, 200, 400, 600) left pending frontiers of 949, 209, 2,003 and 1,806.
Round 2 (2026-10-31) raised the caps and closed all four:

| Closure | Cap | Visited | Failed | Pending |
|---|---|---|---|---|
| `Maps/Frontend` | 6,000 | 3,632 | 0 | 0 |
| `Maps/FortniteTransition` | 3,000 | 290 | 0 | 0 |
| `Maps/UI/Frontend_Lobby_Default` | 6,000 | 3,198 | 0 | 0 |
| `Maps/UI/Frontend_Lobby_StW_Season11` | 6,000 | 3,947 | 0 | 0 |

`porting/generated/extracted/` before round 2: 3,166 files, 219 MB. After: 10,499
files, 1.5 GB. Of the 10,499 files, 7,333 paths are new, 2,972 existing paths were
written again by the round-2 closures (content not compared), 194 existing paths were
not touched, and none were removed. Free space on `/` stayed at 79 to 80 GB.

## Blockers

- **Key-gated, now closed for the Frontend graph:** the four closures close with no
  pending frontier. 300 widget-related packages are recovered, 20 of them as
  `UWidgetBlueprintGeneratedClass`; the other registry widgets are outside the closures.
  30 packages export a `USoundWave`, out of 106,362 registry SoundWave assets.
  `pakchunk1000-1002` contribute no package to the closures.
- **Closure walk gap:** the walk follows only `/Game/` and `/Engine/` roots. Ten
  referenced packages live under `Engine/Plugins/...` (roots `/Water/`, `/Niagara/`,
  `/Paper2D/`) and were never recovered (listed below). Mapping plugin roots to their
  `Engine/Plugins/**/Content` folder is a small `porting/cue4inspect` change; it was
  not made here.
- **Soft references:** object paths stored as soft references in data are not
  followed by the walk. Only hard import roots are measured.
- **Host audio:** no PCM device and no sound server. Real audio output needs a
  host-level decision: a working HDA PCM device, or a user-session PipeWire or
  PulseAudio null sink. Nothing here starts a sound server.
- **Tooling:** `--imports` prints only `/Script` roots. Printing non-`/Script` roots
  would let the loose pass see `/Game` dependencies without a key.

## Native classes the Frontend graph still needs

Method: `widget_blueprints.native_imports` in `porting/generated/ui-audio-census.json`
(300 widget packages). Filtered to class-kind rows (Class and component-typed
imports; Function, struct, enum and delegate rows excluded). Each row is cross-checked
against `porting/native_class_set.txt` (A/U prefix variants) and the reflection index
`porting/generated/reflection/native_class_index.json` (6,958 classes). Ranked by the
number of widget packages that import the name. Engine-owned imports are excluded
because the set holds project closure classes, not engine classes.

- Class-kind imports not in the set: 162 engine-owned (outside the set by design) and
  31 non-engine (table below; 19 of the 31 are component-typed subobject names).
- Of the 31: 2 are missing from both the set and source (`FortSocialNameTextBlock`,
  `FortMediaSubtitlesPlayer`); 2 are hand-written in `porting/native/Source` but not in
  the generated set (`FortKismetLibrary`, `FortStaticMeshActor`); 8 are classes in an
  engine plugin (Paper2D x2, GameplayAbilities x2, AnimationBudgetAllocator,
  ChaosCloth, Niagara x2); 19 are subobject names. The reflection index contains every
  Class row here.
- Rows marked "subobject" are object names whose import class is a component type,
  for example `FortniteUI.Root` as `SceneComponent`. They are template or subobject
  names resolved from the widget Blueprint, not `/Script` classes. 17 of them have
  no reflection entry (`refl` column `no`).

| Rank | Import | Kind | Widget packages | In reflection | Status |
|---|---|---|---|---|---|
| 1 | FortniteGame.FortKismetLibrary | class | 16 | yes | hand-written in porting/native source, not in set |
| 2 | FortniteGame.CameraComponent | subobject (CameraComponent) | 6 | yes | project module, source not recovered |
| 3 | FortniteGame.SceneComponent | subobject (SceneComponent) | 6 | yes | project module, source not recovered |
| 4 | Paper2D.PaperSprite | class | 4 | yes | present in engine plugin, not in set |
| 5 | FortniteUI.FortSocialNameTextBlock | class | 3 | yes | missing from set and source |
| 6 | FortniteUI.PrimarySceneComponent | subobject (ChildActorComponent) | 2 | no | project module, source not recovered |
| 7 | FortniteUI.Root | subobject (SceneComponent) | 2 | no | project module, source not recovered |
| 8 | FortniteUI.SceneChangeRootComponent | subobject (SceneComponent) | 2 | no | project module, source not recovered |
| 9 | FortniteUI.SecondarySceneComponent | subobject (ChildActorComponent) | 2 | no | project module, source not recovered |
| 10 | FortniteUI.TransitionCameraComponent | subobject (CameraComponent) | 2 | no | project module, source not recovered |
| 11 | GameplayAbilities.AbilitySystemBlueprintLibrary | class | 2 | yes | present in engine plugin, not in set |
| 12 | SubtitlesWidgets.FortMediaSubtitlesPlayer | class | 2 | yes | missing from set and source (plugin absent) |
| 13 | AnimationBudgetAllocator.SkeletalMeshComponentBudgeted | class | 1 | yes | present in engine plugin, not in set |
| 14 | ChaosCloth.ChaosClothingSimulationFactory | class | 1 | yes | present in engine plugin, not in set |
| 15 | FortniteGame.FortStaticMeshActor | class | 1 | yes | hand-written in porting/native source, not in set |
| 16 | FortniteGame.StaticMeshComponent0 | subobject (StaticMeshComponent) | 1 | no | project module, source not recovered |
| 17 | FortniteUI.FocusCameraTargetComponent | subobject (ChildActorComponent) | 1 | no | project module, source not recovered |
| 18 | FortniteUI.PedestalBase | subobject (ChildActorComponent) | 1 | no | project module, source not recovered |
| 19 | FortniteUI.PedestalChildrenRoot | subobject (SceneComponent) | 1 | no | project module, source not recovered |
| 20 | FortniteUI.SceneRoot | subobject (SceneComponent) | 1 | no | project module, source not recovered |
| 21 | FortniteUI.VaultBackgroundComponent | subobject (ChildActorComponent) | 1 | no | project module, source not recovered |
| 22 | FortniteUI.VaultRotatorComponent | subobject (ChildActorComponent) | 1 | no | project module, source not recovered |
| 23 | FortniteUI.WidgetComponent_BattlePassInfo | subobject (WidgetComponent) | 1 | no | project module, source not recovered |
| 24 | FortniteUI.WidgetComponent_CompletedFeatInfo | subobject (WidgetComponent) | 1 | no | project module, source not recovered |
| 25 | FortniteUI.WidgetComponent_FillOpenSlot | subobject (WidgetComponent) | 1 | no | project module, source not recovered |
| 26 | FortniteUI.WidgetComponent_MemberInfo | subobject (WidgetComponent) | 1 | no | project module, source not recovered |
| 27 | FortniteUI.WidgetComponentsRoot | subobject (SceneComponent) | 1 | no | project module, source not recovered |
| 28 | GameplayAbilities.AbilitySystemComponent | class | 1 | yes | present in engine plugin, not in set |
| 29 | Niagara.NiagaraComponent | class | 1 | yes | present in engine plugin, not in set |
| 30 | Niagara.NiagaraSystem | class | 1 | yes | present in engine plugin, not in set |
| 31 | Paper2D.PaperSpriteAtlas | class | 1 | yes | present in engine plugin, not in set |

### UI-module class names with no entry in native_class_set.txt

- **CommonUI:** none. Every CommonUI class name the widget packages import has an
  entry in the set (for example CommonButton, CommonTextBlock, CommonActionWidget,
  LoadGuardSlot). CommonUI functions, delegates and the `CommonInputActionData` struct
  are not classes and are outside this check.
- **FortniteUI:** `FortSocialNameTextBlock` (3 packages; missing from set and source,
  present in reflection). The other FortniteUI names outside the set are the 16
  subobject rows above (`PrimarySceneComponent`, `Root`, `SceneChangeRootComponent`,
  `SecondarySceneComponent`, `TransitionCameraComponent`, `FocusCameraTargetComponent`,
  `PedestalBase`, `PedestalChildrenRoot`, `SceneRoot`, `VaultBackgroundComponent`,
  `VaultRotatorComponent`, `WidgetComponent_BattlePassInfo`,
  `WidgetComponent_CompletedFeatInfo`, `WidgetComponent_FillOpenSlot`,
  `WidgetComponent_MemberInfo`, `WidgetComponentsRoot`).
- **UMG:** 38 Class names, all present in engine `Runtime/UMG` (so outside the set by
  design): WidgetBlueprintGeneratedClass, WidgetTree, Overlay, UserWidget, Image,
  OverlaySlot, BorderSlot, SizeBox, Border, HorizontalBox, HorizontalBoxSlot,
  SizeBoxSlot, VerticalBox, VerticalBoxSlot, WidgetSwitcherSlot, BackgroundBlur,
  BackgroundBlurSlot, NamedSlot, ScaleBox, ScaleBoxSlot, MovieScene2DTransformSection,
  MovieScene2DTransformTrack, MovieSceneWidgetMaterialTrack, PanelSlot, PanelWidget,
  UMGSequencePlayer, Widget, WidgetAnimation, WidgetSwitcher, WidgetComponent, Spacer,
  TextBlock, WidgetBlueprintLibrary, DynamicEntryBox, SafeZone, SafeZoneSlot,
  UserObjectListEntry, UserObjectListEntryLibrary. Two engine easing functions
  (EaseInFunction, EaseOutFunction, MovieSceneBuiltInEasingFunction) are also outside
  the set.

### Frontend map natives

171 unique `/Script` natives across the 44 Frontend maps; 7 unresolved, all
FortniteGame functions or one enum: `SendClientStatEvent` (2 maps), `EFrontEndCamera`
(enum), `GetFirstLocalFortPlayerController`, `GetLocalFortPlayerControllers`,
`GetPartSkeletalMeshComponent`, `OnMatchStarted`, `RunFacialAnimationTest`. Other
agents change `porting/native/` during this work, so the status column reflects that
tree at the time of the run.

### Cooked asset counts present locally

Counts are packages with at least one export of the type, among the 4,494 parsed
packages (from 4,680 `.uasset` files; 4,695 `.uexp`; 1,109 `.ubulk`; 15 `.umap`). A
package can hold several types, so rows do not sum.

| Type | Packages | Note |
|---|---|---|
| Textures (`UTexture2D`) | 976 | plus 8 `UTextureCube`, 2 `UTextureRenderTarget2D` |
| Materials (`UMaterial`, `UMaterialInstanceConstant`, `UMaterialFunction`) | 203, 269, 464 | plus 15 `UMaterialParameterCollection` |
| Sounds (`USoundClass`, `USoundCue`, `USoundWave`) | 188, 228, 30 | `USoundWave` payloads are almost absent |
| Fonts (`UFont`) | 3 | |
| Widget blueprints (`UWidgetBlueprintGeneratedClass`) | 20 | `UWidgetBlueprint` has no exports |
| Static meshes (`UStaticMesh`) | 368 | |
| Skeletal meshes (`USkeletalMesh`) | 9 | |

### Cooked widget blueprints now present

Membership is the closure's own saved-file list.

- **Frontend closure (4):** `Athena/UI/Frontend/Art/Takeovers/Donut_fui_AcquisitionOffer`,
  `Athena/UI/Frontend/Art/Takeovers/Donut_fui_AcquisitionRow`,
  `Environments/FrontEnd/Blueprints/BackgroundString/UI_Background_Strings`,
  `UI/InGame/Mission/SpeechBubbleWidget` (also in the lobby closures).
- **Transition closure (3):** `Movies/VideoExitButton`,
  `UI/Foundation/Movie/ActivatableMovieWidget_Monolithic_Native_Min`,
  `UI/Foundation/Movie/VideoPlayerWidget_Min`.
- **Lobby closures (12, both lobby maps):** `Athena/UI/Frontend/Parties/PartySuggestionButton`,
  `Athena/UI/Matchmaking/MatchmakingInputIndicator`, `UI/Foundation/Buttons/PanelButton`,
  `UI/Foundation/Stats/HomebaseRatingBar`, `UI/Foundation/Stats/Moonbeam_PowerRatingWidget`,
  `UI/Foundation/Stats/PowerRatingWidget`, `UI/Frontend/Lobby/LobbyGadgetButton`,
  `UI/Frontend/Lobby/PlayerPad/LobbyPlayerAddPlayer`,
  `UI/Frontend/Lobby/PlayerPad/LobbyPlayerPadGadgets`,
  `UI/Frontend/Lobby/PlayerPad/TeamMemberPedestal_FeatInfo`,
  `UI/Frontend/Lobby/PlayerPad/TeamMemberPedestal_Nameplate`,
  `UI/Frontend/Lobby/PlayerPad/TeamMemberPedestal_XPBoostInfo`.
- **Outside the closures (1):** `UI/Foundation/Profile/BP_BannerEditorTile`, recovered
  earlier and not reached by any closure.

### Referenced but not recovered

- **Hard imports (`/Game/`, `/Engine/`):** none missing. The four closures logged zero
  "Could not recover" failures, so every referenced package that the provider holds
  was recovered.
- **Unfollowed roots:** 10 packages, all under `Engine/Plugins/` and reached through
  roots the walk does not map. Type is inferred from the name only, because the
  packages were not recovered:
  - Water (7): `MPC_Water` (parameter collection), `WaterParticleDisplacement`,
    `MF_Gerstner_Wave_Deep`, `MF_Gerstner_6_Waves`, `GetWaveData`,
    `GerstnerWave_Deep_WithZTrace` (material functions), `T_Water_TilingNormal_Waves_01`
    (texture).
  - Niagara (2): `TilingCurl32` (vector field asset; type unverified), `S_Gnomon`
    (type unverified).
  - Paper2D (1): `TranslucentUnlitSpriteMaterial` (material).
- Fonts, sounds and textures reached only through soft references are not measured.
