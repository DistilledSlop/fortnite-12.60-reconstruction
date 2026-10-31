# Frontend/lobby UI and audio census

Analysis as of 2026-10-30. No UE build, no staged-game run, no pak read, and no
change under `porting/native/`. Regenerate with:

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
- **Packages:** 457 recovered `.uasset` files were classified by the loose
  cue4inspect pass (`--imports` plus export summary, no key).
- **Unmeasured paks:** `pakchunk1000`, `1001` and `1002` are GUID-keyed and not
  indexed by the primary key. Any frontend or sound content in them is unknown.
- **Widgets:** the registry has 2,175 WidgetBlueprints. Four recovered packages are
  widget-related: they sit under `UI/` or import UMG, CommonUI or FortniteUI. The
  other 2,171 are unseen.
- **Sounds:** the registry has 106,362 SoundWave assets. 311 sound-class, mix and
  cue packages are recovered. No SoundWave payload is recovered.
- **Import counts:** this census excludes `Package` and `Default__` rows, so the
  BP_BannerEditorTile count (52) differs from the 77 in `asset_compatibility.md`.

## Key findings

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

The loose pass cannot discover `/Game` dependencies, because `--imports` prints
only `/Script` roots. These commands recover them and need the primary key, plus three GUID keys because of
`--all-keys`. Agents may run them unattended by adding `--key-file=<path>` (see `HANDOFF.md`, "Still to do"):

```sh
# 1. Frontend entry map closure (highest priority)
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/Frontend --save-closure --max-packages=400 --all-keys

# 2. Transition map with CommonUI natives
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/FortniteTransition --save-closure --max-packages=200 --all-keys

# 3. Default lobby (102 asset imports)
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/UI/Frontend_Lobby_Default --save-closure --max-packages=400 --all-keys

# 4. Largest lobby, if closure 1 is not enough
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/Maps/UI/Frontend_Lobby_StW_Season11 --save-closure --max-packages=600 --all-keys
```

Closures write to `porting/generated/extracted/`. A closure that stops at the cap
prints `pending` above zero, so rerun with a higher `--max-packages`. Afterwards,
`python3 porting/census_ui_audio.py` runs with no key and picks up the new packages.

## Blockers

- **Key-gated:** the Frontend asset graph, the 2,171 unseen WidgetBlueprints,
  106,362 SoundWave payloads, and the three GUID-keyed paks.
- **Host audio:** no PCM device and no sound server. Real audio output needs a
  host-level decision: a working HDA PCM device, or a user-session PipeWire or
  PulseAudio null sink. Nothing here starts a sound server.
- **Tooling:** `--imports` prints only `/Script` roots. Printing non-`/Script`
  roots would let the loose pass see `/Game` dependencies without a key. This is
  a small `porting/cue4inspect` change and was left alone under the analysis-only
  scope.
