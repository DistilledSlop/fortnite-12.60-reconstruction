# Native class coverage: exe reflection vs porting tree

Analysis as of 2026-10-31, read-only. No UE build, no run of `promote_native_classes.sh`, no edit under `porting/native/`, `native_class_set.txt` or the generator, no `git add`. Machine-readable output: `porting/generated/native_class_coverage.json` (git-ignored; one record per reflected type with its evidence paths).

Scope: 1 reflected type in a package that is out of scope for this analysis is excluded from every count and table (the task's scope rule; not described here).

## 1. Answer

**Are we porting all native classes the exe reflects? No.**

- Reflected types: 15049 in total (6966 class blocks, of which 210 are `UINTERFACE`; 4512 structs; 2056 enums; 1515 delegates). 10 class blocks have no name in the name map (2 classes, 8 interfaces) and cannot be matched.
- **Game classes** (module is our project or a module absent from the UE 4.25 tree; 4763 named classes and interfaces): 4572 generated (a), 7 hand-written stand-ins (c), 2 blocklisted (d), **182 missing (e)**. So 96.0% have a generated source.
- **Engine and plugin classes** (2193 named): 2004 engine source in an enabled module (b-enabled), 151 engine source in a plugin that is not enabled (b-not-enabled: declared in the 4.25 tree, not compiled into our build), 14 adapted porting-plugin source (Water, c), 24 missing (e).
- **Missing named classes and interfaces: 206** (plus 10 unnamed blocks). **Missing structs, enums and delegates: 1657** (1350 of them are not referenced by any promoted class; they are outside the promotion closure).
- MISSING count by cause category (all kinds, incl. the 10 unnamed blocks): C1 297, C2 19, C3 94, C4 103, C5 1360.

Cause categories (section 6 has the table): **C1** missing plugin/runtime module in our project; **C2** not in `native_class_set.txt` (add it); **C3** blocked by a generator limitation; **C4** engine class not present in the UE 4.25 source; **C5** other (unnamed block, or a struct/enum/delegate outside the promotion closure).

Top actions (full ordered list in section 7):

1. Generate the C1 modules as runtime modules (35 packages absent from both the UE 4.25 and 4.26 trees, for example SocialUMG, LauncherSocialDefaults, FortSoundCueTemplates, SmartObjectsModule). That unblocks 154 classes and 2 interfaces, plus their structs and enums. Needs a decision on the third-party SDK package (NVIDIAGfeSDK, 17 non-class types).
2. Add the 19 FortniteUI interfaces (C2) to `native_class_set.txt`. No blocker.
3. Decide on the hand-written stand-ins (AFortGameModeAthena, AFortStaticMeshActor, AFortHLODSMActor, AFortLevelScriptActor). They block 7 generated children (C3).
4. Address the 87 C3 structs and delegates: some need out/return delegate parameter support, the rest need their other-module types (step 1).

## 2. Method and matching rules

- **Reflected types.** Classes and interfaces: `porting/generated/reflection/class_parent_map.json` (name, package, parent) joined to `fortnite_game_reflection.json` `classes_in_groups_with_fort_names` by block address. Interfaces are blocks with `CLASS_Interface` (0x4000, UE 4.25 `ObjectMacros.h:201`). Structs, enums and delegates: `struct_blocks.json`, `enum_blocks.json`, `delegate_blocks.json` (their `package` field). Counts: 6,967 class blocks (6,957 named), 4,512 structs, 2,056 enums, 1,515 delegate signatures. The task's 6,958 index entries include a `_summary` key. The out-of-scope exclusion leaves 6,966 class/interface blocks.
- **C++ names.** Class and interface: the reflected name already carries its A/U prefix. Struct: `F` + name. Enum: name, or `E` + name; namespaced enums (`X::Type`) use `X`. Delegate: `F` + name without `__DelegateSignature`.
- **Declarations.** A regex over `.h` files finds `class`, `struct`, `enum`, `namespace` and `DECLARE_*DELEGATE` declarations in four trees: (a) `porting/native/Source/*/Generated` (generated, class/struct/enum/delegate); (c) hand-written `porting/native/Source` (not Generated) and `porting/native/Plugins` (the adapted Water plugin); (b) UE 4.25 `Engine/Source` and `Engine/Plugins` (ThirdParty skipped); and UE 4.26 names only, as a hint (`/opt/fortnite/UnrealEngine4260`).
- **Module of a declaration** is the module whose `*.Build.cs` sits in the nearest ancestor directory (the UBT rule). A match needs the declaring module to equal the reflected package (`/Script/X` = module `X`). A name found only in another module is not a match: under the task's rule it needs name plus package. Those cases are MISSING with cause C4 and the other module recorded.
- **Status precedence:** a = generated in our tree; c = hand-written or adapted porting source; b = engine source with matching module; d = blocklisted (`native_class_blocklist.txt`, 8 entries; the file has 10 lines including comments); e = none of these. Within b, **b-enabled** means the module is usable in our build (see Origin and usable rules below); **b-not-enabled** means the source exists but the module is not usable in our build.
- **Origin.** `game` = module is a project module or absent from the UE 4.25 tree. `engine` = engine Runtime, engine plugin, or the porting Water plugin.
- **Usable in our build.** A module is b-enabled when it is an engine Runtime module, a plugin enabled in `native/FortniteLinux.uproject`, a project module, or named in a native `*.Build.cs` dependency list (for example `Hotfix`, `RHI`, `WebBrowser`, which are compiled although their plugin is not in the uproject). The `Build.cs` rule is an approximation of UBT; it does not follow transitive engine dependencies.
- **Counts check.** `native_class_set.txt` has 4,572 entries; the 4,617 in the task brief is the file's line count including comments. Every set entry has a generated header, and every emitted class in `native_classes_index.json` (4,463) is declared in the generated tree.

## 3. Counts

### 3a. By kind and status (named types; the 10 unnamed blocks are reported in section 4)

| Kind | a generated | b-enabled | b-not-enabled | c hand/porting | d blocklisted | e missing | Total |
|---|---:|---:|---:|---:|---:|---:|---:|
| class | 4463 | 1935 | 150 | 21 | 0 | 185 | 6754 |
| interface | 109 | 69 | 1 | 0 | 2 | 21 | 202 |
| struct | 1628 | 1521 | 435 | 4 | 0 | 924 | 4512 |
| enum | 629 | 820 | 97 | 1 | 0 | 509 | 2056 |
| delegate | 1006 | 253 | 30 | 2 | 0 | 224 | 1515 |
| **all named** | **7835** | **4598** | **713** | **28** | **2** | **1863** | 15039 |

### 3b. By package (all kinds; unnamed blocks shown as `(none)`)

| Package | Types | a | b-enabled | b-not-enabled | c | d | e | Classes+ifaces e |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| /Script/FortniteGame | 6515 | 5482 | 0 | 0 | 7 | 2 | 1024 | 6 |
| /Script/FortniteUI | 2602 | 2172 | 0 | 0 | 0 | 0 | 430 | 40 |
| /Script/Engine | 2143 | 0 | 2122 | 0 | 0 | 0 | 21 | 5 |
| /Script/ControlRig | 377 | 0 | 0 | 372 | 0 | 0 | 5 | 1 |
| /Script/AIModule | 273 | 0 | 271 | 0 | 0 | 0 | 2 | 0 |
| /Script/GameplayAbilities | 230 | 0 | 230 | 0 | 0 | 0 | 0 | 0 |
| /Script/UMG | 214 | 0 | 196 | 0 | 0 | 0 | 18 | 0 |
| /Script/Niagara | 211 | 0 | 195 | 0 | 0 | 0 | 16 | 3 |
| /Script/MovieSceneTracks | 141 | 0 | 133 | 0 | 0 | 0 | 8 | 4 |
| /Script/MovieScene | 112 | 0 | 108 | 0 | 0 | 0 | 4 | 3 |
| /Script/AnimGraphRuntime | 110 | 0 | 109 | 0 | 0 | 0 | 1 | 1 |
| /Script/CommonUI | 107 | 93 | 0 | 0 | 0 | 0 | 14 | 0 |
| /Script/Synthesis | 106 | 0 | 0 | 101 | 0 | 0 | 5 | 1 |
| /Script/SlateCore | 91 | 0 | 91 | 0 | 0 | 0 | 0 | 0 |
| /Script/InteractiveToolsFramework | 89 | 0 | 88 | 0 | 0 | 0 | 1 | 1 |
| /Script/AugmentedReality | 74 | 0 | 74 | 0 | 0 | 0 | 0 | 0 |
| /Script/CoreUObject | 71 | 0 | 71 | 0 | 0 | 0 | 0 | 0 |
| /Script/Landscape | 71 | 0 | 70 | 0 | 0 | 0 | 1 | 1 |
| /Script/OnlineSubsystemUtils | 60 | 0 | 60 | 0 | 0 | 0 | 0 | 0 |
| /Script/LiveLinkInterface | 56 | 0 | 49 | 0 | 0 | 0 | 7 | 0 |
| /Script/NavigationSystem | 53 | 0 | 53 | 0 | 0 | 0 | 0 | 0 |
| /Script/Paper2D | 44 | 0 | 0 | 44 | 0 | 0 | 0 | 0 |
| /Script/EditableMesh | 42 | 0 | 0 | 42 | 0 | 0 | 0 | 0 |
| /Script/Slate | 39 | 0 | 38 | 0 | 0 | 0 | 1 | 0 |
| /Script/GameplayTags | 32 | 0 | 32 | 0 | 0 | 0 | 0 | 0 |
| /Script/GeometryCollectionEngine | 32 | 0 | 32 | 0 | 0 | 0 | 0 | 0 |
| /Script/Party | 31 | 0 | 31 | 0 | 0 | 0 | 0 | 0 |
| /Script/MovieSceneCapture | 28 | 0 | 28 | 0 | 0 | 0 | 0 | 0 |
| /Script/Water | 27 | 0 | 0 | 0 | 21 | 0 | 6 | 3 |
| /Script/FieldSystemEngine | 25 | 0 | 25 | 0 | 0 | 0 | 0 | 0 |
| /Script/MediaAssets | 25 | 0 | 23 | 0 | 0 | 0 | 2 | 1 |
| /Script/LiveLink | 24 | 0 | 0 | 23 | 0 | 0 | 1 | 0 |
| /Script/McpProfileSys | 24 | 19 | 0 | 0 | 0 | 0 | 5 | 0 |
| /Script/Foliage | 23 | 0 | 23 | 0 | 0 | 0 | 0 | 0 |
| /Script/KairosSceneCapture | 23 | 0 | 0 | 0 | 0 | 0 | 23 | 7 |
| /Script/LevelSequence | 23 | 0 | 23 | 0 | 0 | 0 | 0 | 0 |
| /Script/SocialUMG | 23 | 0 | 0 | 0 | 0 | 0 | 23 | 19 |
| /Script/GameSubCatalog | 22 | 0 | 0 | 0 | 0 | 0 | 22 | 0 |
| /Script/AnimationCore | 21 | 0 | 21 | 0 | 0 | 0 | 0 | 0 |
| /Script/PlayspaceFramework | 21 | 19 | 0 | 0 | 0 | 0 | 2 | 0 |
| /Script/AudioMixer | 20 | 0 | 19 | 0 | 0 | 0 | 1 | 0 |
| /Script/SmartObjectsModule | 20 | 0 | 0 | 0 | 0 | 0 | 20 | 10 |
| /Script/TimeSynth | 20 | 0 | 0 | 20 | 0 | 0 | 0 | 0 |
| /Script/MantisRuntime | 19 | 0 | 0 | 0 | 0 | 0 | 19 | 9 |
| /Script/MeshNetwork | 19 | 17 | 0 | 0 | 0 | 0 | 2 | 0 |
| /Script/PhysXVehicles | 19 | 0 | 19 | 0 | 0 | 0 | 0 | 0 |
| /Script/ReplicationGraph | 19 | 0 | 19 | 0 | 0 | 0 | 0 | 0 |
| /Script/GameplayTasks | 17 | 0 | 17 | 0 | 0 | 0 | 0 | 0 |
| /Script/NVIDIAGfeSDK | 17 | 0 | 0 | 0 | 0 | 0 | 17 | 0 |
| /Script/HeadMountedDisplay | 16 | 0 | 16 | 0 | 0 | 0 | 0 | 0 |
| /Script/LauncherSocialDefaults | 16 | 0 | 0 | 0 | 0 | 0 | 16 | 16 |
| /Script/ChaosSolverEngine | 15 | 0 | 15 | 0 | 0 | 0 | 0 | 0 |
| /Script/MIDIDevice | 15 | 0 | 0 | 15 | 0 | 0 | 0 | 0 |
| /Script/EpicCMSUIFramework | 14 | 0 | 0 | 0 | 0 | 0 | 14 | 7 |
| /Script/FortSoundCueTemplates | 14 | 0 | 0 | 0 | 0 | 0 | 14 | 13 |
| /Script/Account | 13 | 9 | 0 | 0 | 0 | 0 | 4 | 0 |
| /Script/ClothingSystemRuntimeCommon | 13 | 0 | 13 | 0 | 0 | 0 | 0 | 0 |
| /Script/EngineSettings | 13 | 0 | 13 | 0 | 0 | 0 | 0 | 0 |
| /Script/HoagieRuntime | 13 | 0 | 0 | 0 | 0 | 0 | 13 | 6 |
| /Script/RigVM | 13 | 0 | 13 | 0 | 0 | 0 | 0 | 0 |
| /Script/AnimationSharing | 12 | 0 | 12 | 0 | 0 | 0 | 0 | 0 |
| /Script/CinematicCamera | 12 | 0 | 12 | 0 | 0 | 0 | 0 | 0 |
| /Script/ClothingSystemRuntimeInterface | 12 | 0 | 12 | 0 | 0 | 0 | 0 | 0 |
| /Script/GameplayBehaviorsModule | 12 | 0 | 0 | 0 | 0 | 0 | 12 | 9 |
| /Script/LauncherSocialTypes | 12 | 0 | 0 | 0 | 0 | 0 | 12 | 0 |
| /Script/Qos | 12 | 0 | 12 | 0 | 0 | 0 | 0 | 0 |
| /Script/ResonanceAudio | 12 | 0 | 0 | 12 | 0 | 0 | 0 | 0 |
| /Script/AudioExtensions | 11 | 0 | 11 | 0 | 0 | 0 | 0 | 0 |
| /Script/AudioSynesthesia | 11 | 0 | 0 | 11 | 0 | 0 | 0 | 0 |
| /Script/CommonInput | 11 | 10 | 0 | 0 | 0 | 0 | 1 | 0 |
| /Script/Landmass | 11 | 0 | 11 | 0 | 0 | 0 | 0 | 0 |
| (none) | 10 | 0 | 0 | 0 | 0 | 0 | 10 | 0 |
| /Script/ChaosNiagara | 10 | 0 | 0 | 10 | 0 | 0 | 0 | 0 |
| /Script/ImagePlate | 10 | 0 | 0 | 10 | 0 | 0 | 0 | 0 |
| /Script/MeshDescription | 10 | 0 | 10 | 0 | 0 | 0 | 0 | 0 |
| /Script/AssetRegistry | 9 | 0 | 9 | 0 | 0 | 0 | 0 | 0 |
| /Script/EpicGameplayStatsRuntime | 9 | 0 | 0 | 0 | 0 | 0 | 9 | 2 |
| /Script/LiveLinkMovieScene | 9 | 0 | 0 | 9 | 0 | 0 | 0 | 0 |
| /Script/OnlineSubsystem | 9 | 0 | 9 | 0 | 0 | 0 | 0 | 0 |
| /Script/SubtitlesWidgets | 9 | 0 | 0 | 0 | 0 | 0 | 9 | 4 |
| /Script/Serialization | 8 | 0 | 8 | 0 | 0 | 0 | 0 | 0 |
| /Script/AmbientAudio | 7 | 6 | 0 | 0 | 0 | 0 | 1 | 0 |
| /Script/EngineMessages | 7 | 0 | 7 | 0 | 0 | 0 | 0 | 0 |
| /Script/FieldSystemCore | 7 | 0 | 7 | 0 | 0 | 0 | 0 | 0 |
| /Script/LiveLinkComponents | 7 | 0 | 0 | 6 | 0 | 0 | 1 | 0 |
| /Script/LiveLinkMessageBusFramework | 7 | 0 | 5 | 0 | 0 | 0 | 2 | 0 |
| /Script/Lobby | 7 | 0 | 7 | 0 | 0 | 0 | 0 | 0 |
| /Script/SoundCueTemplates | 7 | 0 | 0 | 7 | 0 | 0 | 0 | 0 |
| /Script/BuildPatchServices | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/ClothingSystemRuntimeNv | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/GeometryCollectionCore | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/GeometryCollectionSimulationCore | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/InputCore | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/LiveStreamAnimation | 6 | 0 | 0 | 0 | 0 | 0 | 6 | 3 |
| /Script/MRMesh | 6 | 0 | 6 | 0 | 0 | 0 | 0 | 0 |
| /Script/ModularGameplay | 6 | 6 | 0 | 0 | 0 | 0 | 0 | 0 |
| /Script/ProceduralMeshComponent | 6 | 0 | 0 | 6 | 0 | 0 | 0 | 0 |
| /Script/ChaosCloth | 5 | 0 | 0 | 5 | 0 | 0 | 0 | 0 |
| /Script/EventModeRuntime | 5 | 0 | 0 | 0 | 0 | 0 | 5 | 2 |
| /Script/GeometryCollectionTracks | 5 | 0 | 0 | 5 | 0 | 0 | 0 | 0 |
| /Script/Hotfix | 5 | 0 | 5 | 0 | 0 | 0 | 0 | 0 |
| /Script/LocalNotifications | 5 | 0 | 0 | 0 | 0 | 0 | 5 | 1 |
| /Script/NiagaraShader | 5 | 0 | 5 | 0 | 0 | 0 | 0 | 0 |
| /Script/SessionMessages | 5 | 0 | 5 | 0 | 0 | 0 | 0 | 0 |
| /Script/SidecarSys | 5 | 0 | 0 | 0 | 0 | 0 | 5 | 3 |
| /Script/Solaris | 5 | 0 | 0 | 0 | 0 | 0 | 5 | 5 |
| /Script/EyeTracker | 4 | 0 | 4 | 0 | 0 | 0 | 0 | 0 |
| /Script/ImageWriteQueue | 4 | 0 | 4 | 0 | 0 | 0 | 0 | 0 |
| /Script/MaterialShaderQualitySettings | 4 | 0 | 4 | 0 | 0 | 0 | 0 | 0 |
| /Script/Overlay | 4 | 0 | 4 | 0 | 0 | 0 | 0 | 0 |
| /Script/PurchaseFlow | 4 | 0 | 0 | 0 | 0 | 0 | 4 | 1 |
| /Script/ScheduledEvents | 4 | 0 | 0 | 0 | 0 | 0 | 4 | 0 |
| /Script/TimeManagement | 4 | 0 | 4 | 0 | 0 | 0 | 0 | 0 |
| /Script/AndroidPermission | 3 | 0 | 0 | 3 | 0 | 0 | 0 | 0 |
| /Script/AnimationBudgetAllocator | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/AudioAnalyzer | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/ChaosSolvers | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/ClientPilot | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/Gauntlet | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/MediaUtils | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/NetUI | 3 | 0 | 0 | 0 | 0 | 0 | 3 | 3 |
| /Script/NiagaraCore | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/PhysicsCore | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/VectorVM | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/WebBrowser | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 |
| /Script/AnalyticsBlueprintLibrary | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| /Script/AssetTags | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| /Script/AudioPlatformConfiguration | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/BlueprintContext | 2 | 2 | 0 | 0 | 0 | 0 | 0 | 0 |
| /Script/CableComponent | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| /Script/EventModeUI | 2 | 0 | 0 | 0 | 0 | 0 | 2 | 2 |
| /Script/ForwardingChannels | 2 | 0 | 0 | 0 | 0 | 0 | 2 | 2 |
| /Script/GameFeedback | 2 | 0 | 0 | 0 | 0 | 0 | 2 | 1 |
| /Script/ImageWrapper | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/JsonUtilities | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/LiveLinkCurveDebugUI | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| /Script/LiveStreamVoice | 2 | 0 | 0 | 0 | 0 | 0 | 2 | 2 |
| /Script/MoviePlayer | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/NetCore | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/NiagaraAnimNotifies | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/OodleHandlerComponent | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/PacketHandler | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/PropertyPath | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/Rejoin | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/WidgetCarousel | 2 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| /Script/AutomationUtils | 1 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| /Script/AvfMediaFactory | 1 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| /Script/Crashlytics | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/DiscordRPC | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/F5Player | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/F5PlayerFactory | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/FacialAnimSystem | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/FortniteEngineLoadingScreen | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/HoagieUI | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/Renderer | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
| /Script/SignificanceManager | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
| /Script/SolarisUi | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/SoundFields | 1 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
| /Script/StaticMeshDescription | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
| /Script/UACBase | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| /Script/WmfMediaFactory | 1 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |

`e` in the package table includes the unnamed blocks (package `(none)`).

## 4. MISSING classes and interfaces (named 206, unnamed 10)

Cause code and short reason per row; the parent is the reflected parent. `ancestor` in the reason means the blocking type is a parent.

#### /Script/FortniteUI (40)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AFortFrontendLevelScriptActor | class | AFortLevelScriptActor | C3 | parent AFortLevelScriptActor is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| UAthenaPlayerInfoBaseWidgetInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortAthenaMOTDNewsChildren | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortBangInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortBangWrapperContentInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortChatContainer | class | USocialChatContainer | C1 | ancestor USocialChatContainer: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortChatMessageEntry | class | USocialChatMessageEntry | C1 | ancestor USocialChatMessageEntry: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortConnectionsUserListEntry | class | UFortSocialListUserEntryBase | C1 | ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting pl... |
| UFortGlobalActionDetailsDataSource | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortHUDObjectiveSizeInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortHUDShutdownTimerInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortHomeCMSScreenBase | class | UEpicCMSScreenBase | C1 | ancestor UEpicCMSScreenBase: package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortItemListEntryWidgetInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortItemListViewInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortItemManagementMulchPanel | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortItemViewContextInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortJoinablePartyActionMenu | class | USocialInteractionMenu | C1 | ancestor USocialInteractionMenu: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortJoinablePartyListEntry | class | USocialListInteractableEntryBase | C1 | ancestor USocialListInteractableEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortLeavePartyListEntry | class | USocialListInteractableEntryBase | C1 | ancestor USocialListInteractableEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortLeavePartyMenu | class | USocialActionMenu | C1 | ancestor USocialActionMenu: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMissedInvitesActionMenu | class | USocialActionMenu | C1 | ancestor USocialActionMenu: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMissedInvitesListEntry | class | USocialListInteractableEntryBase | C1 | ancestor USocialListInteractableEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMobileActionButtonInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortNodeCanvasEntityInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortNodeCanvasInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortPartyMemberListEntry | class | UFortPresenceUserListEntry | C1 | ancestor UFortPresenceUserListEntry: ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no mod... |
| UFortPresenceUserListEntry | class | UFortSocialListUserEntryBase | C1 | ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting pl... |
| UFortPrimaryContentInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortPrioritizedWidget | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortSocialInteractionMenu | class | USocialInteractionMenu | C1 | ancestor USocialInteractionMenu: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortSocialListUserEntryBase | class | USocialListUserEntryBase | C1 | ancestor USocialListUserEntryBase: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortSocialNameTextBlock | class | USocialNameTextBlock | C1 | ancestor USocialNameTextBlock: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortSocialUserListEntry | class | UFortPresenceUserListEntry | C1 | ancestor UFortPresenceUserListEntry: ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no mod... |
| UFortSocialUserTreeView | class | USocialUserTreeView | C1 | ancestor USocialUserTreeView: package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortTabButtonInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortTeamMemberListEntry | class | UFortPresenceUserListEntry | C1 | ancestor UFortPresenceUserListEntry: ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no mod... |
| UFortTimespanDataSource | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |
| UFortUserSearchResultListEntry | class | UFortPresenceUserListEntry | C1 | ancestor UFortPresenceUserListEntry: ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no mod... |
| UFortVoiceChatMemberListEntry | class | UFortPresenceUserListEntry | C1 | ancestor UFortPresenceUserListEntry: ancestor UFortSocialListUserEntryBase: ancestor USocialListUserEntryBase: package /Script/SocialUMG has no mod... |
| UHUDLayoutToolInterface | interface | UInterface | C2 | usable module FortniteUI, parent UInterface generated or engine; add to native_class_set.txt |

#### /Script/SocialUMG (19)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UDesignerPreviewChatChannel | class | USocialChatChannel | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialActionMenu | class | UCommonActivatableWidget | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatChannelTab | class | UCommonButton | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatContainer | class | UUserWidget | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatEditableText | class | UWidget | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatMessageEntry | class | UUserWidget | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatMessageList | class | UListViewBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialGroupListEntry | class | UCommonButton | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialGroupTreeView | class | UTreeView | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialInteractionButton | class | UCommonButton | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialInteractionMenu | class | USocialActionMenu | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialListEntryBase | class | UCommonButton | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialListInteractableEntryBase | class | USocialListEntryBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialListUserEntryBase | class | USocialListInteractableEntryBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialNameTextBlock | class | UCommonRichTextBlock | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialUserListHeaderEntry | class | USocialListEntryBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialUserListView | class | USocialUserListViewBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialUserListViewBase | class | UListViewBase | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialUserTreeView | class | USocialUserListView | C1 | package /Script/SocialUMG has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/LauncherSocialDefaults (16)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UChatColorSchemeDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialActionButtonStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatChromeColorSchemeDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatChromeMarginsDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatChromeStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatMarkupStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialChatStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialCheckBoxStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialFontStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialListMarginsDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialListStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialProfanityDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialRadioBoxStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialScrollBarStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialSoundSchemaDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USocialStyleDataAsset | class | UDataAsset | C1 | package /Script/LauncherSocialDefaults has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/FortSoundCueTemplates (13)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UEmoteBase | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEmoteFoley | class | UEmoteBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEmoteMusic | class | UEmoteBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEmoteMusic3P | class | UEmoteBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortSoundCueTemplateBase | class | USoundCueTemplate | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGliderClose | class | UGliderOpenClose | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGliderOpen | class | UGliderOpenClose | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGliderOpenClose | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGliderThrustLoop | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGliderThrustStart | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UPickaxeImpactEnemy | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UPickaxeReady | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UPickaxeSwing | class | UFortSoundCueTemplateBase | C1 | package /Script/FortSoundCueTemplates has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/SmartObjectsModule (10)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AGenericSmartObject | class | AActor | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UAITask_UseSmartObject | class | UAITask | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UBTTask_FindAndUseSmartObject | class | UBTTaskNode | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UBTTask_UseSmartObject | class | UBTTask_BlackboardBase | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| URichSmartObjectInterface | interface | UInterface | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USmartObjectBlueprintFunctionLibrary | class | UBlueprintFunctionLibrary | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USmartObjectComponent | class | UActorComponent | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USmartObjectConfig | class | UDataAsset | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USmartObjectManager | class | UObject | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USmartObjectRenderingComponent | class | UPrimitiveComponent | C1 | package /Script/SmartObjectsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/GameplayBehaviorsModule (9)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UBTTask_StopGameplayBehavior | class | UBTTaskNode | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehavior | class | UObject | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehaviorConfig | class | UObject | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehaviorConfig_Animation | class | UGameplayBehaviorConfig | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehaviorConfig_BehaviorTree | class | UGameplayBehaviorConfig | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehaviorManager | class | UAISubsystem | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehavior_AnimationBased | class | UGameplayBehavior | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehavior_BehaviorTree | class | UGameplayBehavior | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayBehaviorsBlueprintFunctionLibrary | class | UBlueprintFunctionLibrary | C1 | package /Script/GameplayBehaviorsModule has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/MantisRuntime (9)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFortAbilityTask_ApplyRootMotionMantisForce | class | UAbilityTask_ApplyRootMotion_Base | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortAnimNotifyState_Mantis | class | UAnimNotifyState | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortAnimNotify_Mantis | class | UAnimNotify | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortGameplayAbility_Mantis | class | UFortGameplayAbility | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortItemLayerAnimInstance_UncleBrolly | class | UFortItemLayerAnimInstance | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMantisData | class | UDataAsset | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMantisPawnComponent | class | UPawnComponent | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortMantisWeaponComponent | class | UFortWeaponComponent | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UUncleBrollyWeaponAnimInstance | class | UAnimInstance | C1 | package /Script/MantisRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/EpicCMSUIFramework (7)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UEpicCMSImage | class | UCommonLazyImage | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSLayoutBase | class | UUserWidget | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSManager | class | UObject | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSScreenBase | class | UCommonActivatablePanel | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSSimpleMessage | class | UCommonUserWidget | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSTileBase | class | UCommonButton | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UEpicCMSTileCarousel | class | UUserWidget | C1 | package /Script/EpicCMSUIFramework has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/KairosSceneCapture (7)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AKairosGameModeBase | class | AGameModeBase | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| AKairosHeroBase | class | AActor | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| AKairosPawn | class | APawn | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| AKairosPlayerController | class | APlayerController | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| AKairosSceneManagerActor | class | AActor | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UKairosCaptureStatics | class | UBlueprintFunctionLibrary | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UKairosConfig | class | UDataAsset | C1 | package /Script/KairosSceneCapture has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/FortniteGame (6)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AFortAthenaTutorialMarker | class | AFortStaticMeshActor | C3 | parent AFortStaticMeshActor is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| AFortAthenaTutorialShootingAutofireTarget | class | AFortStaticMeshActor | C3 | parent AFortStaticMeshActor is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| AFortGameModeBR | class | AFortGameModeAthena | C3 | parent AFortGameModeAthena is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| AFortHLODSMSectionActor | class | AFortHLODSMActor | C3 | parent AFortHLODSMActor is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| AFortInstancedStaticMeshActor | class | AFortStaticMeshActor | C3 | parent AFortStaticMeshActor is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |
| AFortPlayspaceGameMode | class | AFortGameModeAthena | C3 | parent AFortGameModeAthena is a hand-written stand-in (generated child would clash; blocklisted or hand-written) |

#### /Script/HoagieRuntime (6)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AFortHoagieAudioController | class | AActor | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| AFortHoagieVehicle | class | AFortAthenaSKVehicle | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortCameraMode_Hoagie | class | UFortCameraMode_AthenaVehicle | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortHoagieDriverAnimInstance | class | UFortVehicleOccupantAnimInstance | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortHoagieVehicleAnimInstance | class | UFortVehicleAnimInstance | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortHoagieVehicleConfigs | class | UFortPhysicsVehicleConfigs | C1 | package /Script/HoagieRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/Engine (5)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UAnimCompress_Automatic | class | UAnimCompress | C4 | module Engine is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; absent from UE 4.26 tree too |
| UAudioBus | class | UObject | C4 | module Engine is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UDeviceProfileFragment | class | UObject | C4 | module Engine is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UVirtualTexture2D | class | UTexture2D | C4 | module Engine is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UVirtualTextureBuilder | class | UObject | C4 | module Engine is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/Solaris (5)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| USolarisArrayLibrary | class | UObject | C1 | package /Script/Solaris has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USolarisGameSettings | class | UObject | C1 | package /Script/Solaris has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USolarisMathLibrary_Bool | class | UObject | C1 | package /Script/Solaris has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USolarisMathLibrary_Int | class | UObject | C1 | package /Script/Solaris has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USolarisMathLibrary_Real | class | UObject | C1 | package /Script/Solaris has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/MovieSceneTracks (4)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UMovieSceneCameraShakeSourceShakeSection | class | UMovieSceneSection | C4 | module MovieSceneTracks is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UMovieSceneCameraShakeSourceShakeTrack | class | UMovieSceneNameableTrack | C4 | module MovieSceneTracks is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UMovieSceneCameraShakeSourceTriggerSection | class | UMovieSceneSection | C4 | module MovieSceneTracks is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |
| UMovieSceneCameraShakeSourceTriggerTrack | class | UMovieSceneTrack | C4 | module MovieSceneTracks is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/SubtitlesWidgets (4)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFortMediaSubtitlesPlayer | class | UObject | C1 | package /Script/SubtitlesWidgets has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USubtitleDisplay | class | UWidget | C1 | package /Script/SubtitlesWidgets has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USubtitleDisplayOptions | class | UDataAsset | C1 | package /Script/SubtitlesWidgets has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USubtitleDisplaySubsystem | class | UGameInstanceSubsystem | C1 | package /Script/SubtitlesWidgets has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/LiveStreamAnimation (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| ULiveLinkTestSkelMeshTrackerComponent | class | UActorComponent | C1 | package /Script/LiveStreamAnimation has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| ULiveStreamAnimationChannel | class | UChannel | C1 | package /Script/LiveStreamAnimation has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| ULiveStreamAnimationSubsystem | class | UGameInstanceSubsystem | C1 | package /Script/LiveStreamAnimation has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/MovieScene (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UMovieSceneBoolSection | class | UMovieSceneSection | C4 | module MovieScene is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree; same name declared in U... |
| UMovieSceneSpawnSection | class | UMovieSceneBoolSection | C4 | module MovieScene is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree; same name declared in U... |
| UMovieSceneSpawnTrack | class | UMovieSceneTrack | C4 | module MovieScene is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree; same name declared in U... |

#### /Script/NetUI (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UBadNetworkIndicator | class | UUserWidget | C1 | package /Script/NetUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UNetDebugWidget | class | UUserWidget | C1 | package /Script/NetUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UNetGraph | class | UUserWidget | C1 | package /Script/NetUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/Niagara (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UNiagaraShaderStageBase | class | UNiagaraMergeable | C4 | module Niagara is in the UE 4.25 tree (engine-plugin:Niagara, enabled) but no source file declares it; absent from UE 4.26 tree too |
| UNiagaraShaderStagePlaceholderA | class | UNiagaraShaderStageBase | C4 | module Niagara is in the UE 4.25 tree (engine-plugin:Niagara, enabled) but no source file declares it; absent from UE 4.26 tree too |
| UNiagaraShaderStagePlaceholderB | class | UNiagaraShaderStageBase | C4 | module Niagara is in the UE 4.25 tree (engine-plugin:Niagara, enabled) but no source file declares it; absent from UE 4.26 tree too |

#### /Script/SidecarSys (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| USidecarDss | class | UObject | C1 | package /Script/SidecarSys has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USidecarInventorySys | class | UObject | C1 | package /Script/SidecarSys has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| USidecarSys | class | UObject | C1 | package /Script/SidecarSys has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/Water (3)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AWaterBodyProxy | class | AActor | C4 | module Water is in the UE 4.25 tree (porting-plugin:Water, enabled) but no source file declares it; absent from UE 4.26 tree too |
| UNiagaraDataInterfaceWater | class | UNiagaraDataInterface | C4 | module Water is in the UE 4.25 tree (porting-plugin:Water, enabled) but no source file declares it; present in UE 4.26 tree |
| UNiagaraWaterFunctionLibrary | class | UBlueprintFunctionLibrary | C4 | module Water is in the UE 4.25 tree (porting-plugin:Water, enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/EpicGameplayStatsRuntime (2)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UBlueprintGameplayStatsLibrary | class | UBlueprintFunctionLibrary | C1 | package /Script/EpicGameplayStatsRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UGameplayTagTableManager | class | UDataAsset | C1 | package /Script/EpicGameplayStatsRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/EventModeRuntime (2)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| AFortWeapon_EventMode | class | AFortWeapon | C1 | package /Script/EventModeRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortGameFrameworkComponent_EventMode | class | UGameFrameworkComponent | C1 | package /Script/EventModeRuntime has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/EventModeUI (2)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFocusButton | class | UBacchusActionButton | C1 | package /Script/EventModeUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UFortEventModeEmotesWidget | class | UFortHUDElementWidget | C1 | package /Script/EventModeUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/ForwardingChannels (2)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UForwardingChannelFactory | interface | UInterface | C1 | package /Script/ForwardingChannels has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| UForwardingChannelsSubsystem | class | UGameInstanceSubsystem | C1 | package /Script/ForwardingChannels has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/LiveStreamVoice (2)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| ULiveStreamVoiceChannel | class | UChannel | C1 | package /Script/LiveStreamVoice has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |
| ULiveStreamVoiceSubsystem | class | UGameInstanceSubsystem | C1 | package /Script/LiveStreamVoice has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/AnimGraphRuntime (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UAnimCustomInstance | class | UAnimInstance | C4 | module AnimGraphRuntime is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; absent from UE 4.26 tree too |

#### /Script/ControlRig (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UControlRigLibrary | class | UBlueprintFunctionLibrary | C4 | module ControlRig is in the UE 4.25 tree (engine-plugin:ControlRig, not enabled) but no source file declares it; absent from UE 4.26 tree too |

#### /Script/Crashlytics (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UCrashlyticsBlueprintLibrary | class | UBlueprintFunctionLibrary | C1 | package /Script/Crashlytics has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/DiscordRPC (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UDiscordRuntimeSettings | class | UObject | C1 | package /Script/DiscordRPC has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/F5Player (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UF5PlayerSettings | class | UDeveloperSettings | C1 | package /Script/F5Player has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/F5PlayerFactory (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UF5PlayerFactorySettings | class | UObject | C1 | package /Script/F5PlayerFactory has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/FacialAnimSystem (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFacialLiveLinkRemapAsset | class | ULiveLinkRetargetAsset | C1 | package /Script/FacialAnimSystem has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/FortniteEngineLoadingScreen (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFortniteUserInterfaceSettings | class | UDeveloperSettings | C1 | package /Script/FortniteEngineLoadingScreen has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/GameFeedback (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| ULogOptions | class | UObject | C1 | package /Script/GameFeedback has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/HoagieUI (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UFortHoagieVehicleReticle | class | UFortHUDElementWidget | C1 | package /Script/HoagieUI has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/InteractiveToolsFramework (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UMeshSurfacePointToolMouseBehavior | class | UAnyButtonInputBehavior | C4 | module InteractiveToolsFramework is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; absent from UE 4.26 tree too |

#### /Script/Landscape (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| ULandscapeLODStreamingProxy | class | UStreamableRenderAsset | C4 | module Landscape is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/LocalNotifications (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| ULocalNotificationsSettings | class | UDeveloperSettings | C1 | package /Script/LocalNotifications has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/MediaAssets (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UMediaTimeStampInfo | class | UObject | C4 | module MediaAssets is in the UE 4.25 tree (engine-runtime, enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/PurchaseFlow (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UPurchaseFlowJSBridge | class | UObject | C1 | package /Script/PurchaseFlow has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/SolarisUi (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| USolUserWidget | class | UUserWidget | C1 | package /Script/SolarisUi has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### /Script/Synthesis (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| USubmixEffectStereoDelayPreset | class | USoundEffectSubmixPreset | C4 | module Synthesis is in the UE 4.25 tree (engine-plugin:Synthesis, not enabled) but no source file declares it; present in UE 4.26 tree |

#### /Script/UACBase (1)

| Name | Kind | Parent | Cause | Reason |
|---|---|---|---|---|
| UUACNetworkComponent | class | UActorComponent | C1 | package /Script/UACBase has no module in the UE 4.25 tree or porting plugins (4.26 has module: False) |

#### (none): unnamed reflection blocks (10)

These 10 class blocks have no name in `class_name_map.json` and no package. They need a name (for example from the runtime dump) before they can be matched. Cause C5.

## 5. MISSING structs, enums and delegates (1657 named)

| Package | C1 | C3 | C4 | C5 | Total |
|---|---:|---:|---:|---:|---:|
| /Script/FortniteGame | 0 | 17 | 0 | 1001 | 1018 |
| /Script/FortniteUI | 0 | 68 | 0 | 322 | 390 |
| /Script/GameSubCatalog | 22 | 0 | 0 | 0 | 22 |
| /Script/UMG | 0 | 0 | 18 | 0 | 18 |
| /Script/NVIDIAGfeSDK | 17 | 0 | 0 | 0 | 17 |
| /Script/Engine | 0 | 0 | 16 | 0 | 16 |
| /Script/KairosSceneCapture | 16 | 0 | 0 | 0 | 16 |
| /Script/CommonUI | 0 | 1 | 0 | 13 | 14 |
| /Script/Niagara | 0 | 0 | 13 | 0 | 13 |
| /Script/LauncherSocialTypes | 12 | 0 | 0 | 0 | 12 |
| /Script/SmartObjectsModule | 10 | 0 | 0 | 0 | 10 |
| /Script/MantisRuntime | 10 | 0 | 0 | 0 | 10 |
| /Script/LiveLinkInterface | 0 | 0 | 7 | 0 | 7 |
| /Script/EpicGameplayStatsRuntime | 7 | 0 | 0 | 0 | 7 |
| /Script/HoagieRuntime | 7 | 0 | 0 | 0 | 7 |
| /Script/EpicCMSUIFramework | 7 | 0 | 0 | 0 | 7 |
| /Script/SubtitlesWidgets | 5 | 0 | 0 | 0 | 5 |
| /Script/McpProfileSys | 0 | 0 | 0 | 5 | 5 |
| /Script/Synthesis | 0 | 0 | 4 | 0 | 4 |
| /Script/ControlRig | 0 | 0 | 4 | 0 | 4 |
| /Script/Account | 0 | 0 | 0 | 4 | 4 |
| /Script/ScheduledEvents | 4 | 0 | 0 | 0 | 4 |
| /Script/LocalNotifications | 4 | 0 | 0 | 0 | 4 |
| /Script/MovieSceneTracks | 0 | 0 | 4 | 0 | 4 |
| /Script/SocialUMG | 4 | 0 | 0 | 0 | 4 |
| /Script/PurchaseFlow | 3 | 0 | 0 | 0 | 3 |
| /Script/EventModeRuntime | 3 | 0 | 0 | 0 | 3 |
| /Script/GameplayBehaviorsModule | 3 | 0 | 0 | 0 | 3 |
| /Script/LiveStreamAnimation | 3 | 0 | 0 | 0 | 3 |
| /Script/Water | 0 | 0 | 3 | 0 | 3 |
| /Script/AIModule | 0 | 0 | 2 | 0 | 2 |
| /Script/MeshNetwork | 0 | 1 | 0 | 1 | 2 |
| /Script/LiveLinkMessageBusFramework | 0 | 0 | 2 | 0 | 2 |
| /Script/PlayspaceFramework | 0 | 0 | 0 | 2 | 2 |
| /Script/SidecarSys | 2 | 0 | 0 | 0 | 2 |
| /Script/AmbientAudio | 0 | 0 | 0 | 1 | 1 |
| /Script/FortSoundCueTemplates | 1 | 0 | 0 | 0 | 1 |
| /Script/MovieScene | 0 | 0 | 1 | 0 | 1 |
| /Script/AudioMixer | 0 | 0 | 1 | 0 | 1 |
| /Script/LiveLinkComponents | 0 | 0 | 1 | 0 | 1 |
| /Script/CommonInput | 0 | 0 | 0 | 1 | 1 |
| /Script/Slate | 0 | 0 | 1 | 0 | 1 |
| /Script/MediaAssets | 0 | 0 | 1 | 0 | 1 |
| /Script/LiveLink | 0 | 0 | 1 | 0 | 1 |
| /Script/GameFeedback | 1 | 0 | 0 | 0 | 1 |
| **total** | **141** | **87** | **79** | **1350** | 1657 |

C5 rows (not referenced by any promoted class, so not emitted) are listed only in the JSON. C1, C3 and C4 rows are listed below.

### C1: module absent from the UE 4.25 and 4.26 trees (generate the module) (141)

- **/Script/GameSubCatalog** (17 struct, 5 enum): CatalogDownload, CatalogDynamicBundle, CatalogDynamicBundleItem, CatalogGiftInfo, CatalogItemPrice, CatalogItemSalePrice, CatalogKeyValue, CatalogMetaAssetInfo, CatalogOffer, CatalogOfferRequirement, CatalogPurchaseInfo, CatalogPurchaseInfoGift, CatalogPurchaseNotification, CatalogReceiptInfo, EAppStore::Type, ECatalogOfferType, ECatalogRequirementType, ECatalogSaleType::Type, EStoreCurrencyType::Type, ItemQuantity, KeychainDownload, Storefront
- **/Script/NVIDIAGfeSDK** (12 struct, 5 enum): EGfeSDKHighlightSignificance, EGfeSDKHighlightType, EGfeSDKPermission, EGfeSDKReturnCode, EGfeSDKScope, GfeSDKCreateInputParams, GfeSDKCreateResponse, GfeSDKHighlightCloseGroupParams, GfeSDKHighlightConfigParams, GfeSDKHighlightDefinition, GfeSDKHighlightGroupView, GfeSDKHighlightOpenGroupParams, GfeSDKHighlightScreenshotParams, GfeSDKHighlightSummaryParams, GfeSDKHighlightVideoParams, GfeSDKPermissionsChangedData, GfeSDKRequestPermissionsParams
- **/Script/KairosSceneCapture** (11 struct, 5 enum): EKairosAppMode, EKairosAvatarCaptureState, EKairosHeroAnimationState, EKairosHeroAnimationType, EKairosHeroSkeletonType, KairosAnimationFramingInfo, KairosAvatarAttributesMcpData, KairosAvatarDisplayInfo, KairosAvatarItemMcpData, KairosAvatarMcpData, KairosBobbleTimeParams, KairosCaptureAvatarParams, KairosCaptureParams, KairosSceneInfo, KairosSnippetParams, KairosSnippetPlayRecord
- **/Script/LauncherSocialTypes** (12 struct): ChatChromeColorScheme, ChatChromeMargins, ChatChromeStyle, ChatColorScheme, ChatMarkupStyle, ChatStyle, ProfanityData, SocialFontStyle, SocialListMargins, SocialListStyle, SocialSoundSchema, SocialStyle
- **/Script/MantisRuntime** (5 struct, 4 enum, 1 delegate): ApplyRootMotionMantisForceDelegate, EFortMantisBranchPath, EFortMantisBranchRule, EFortMantisNotifyEvent, EFortMantisNotifyWindow, FortMantisMontageData, FortMantisTargetData, FortMantisTechniqueBranch, FortMantisTechniqueData, FortRootMotionSource_Mantis
- **/Script/SmartObjectsModule** (6 struct, 4 enum): ESOReferenceDrawingMode, ESmartObjectRequiredTagsPolicy, ESmartObjectSlotState, ESmartObjectTagPolicy, SmartObjectExecutionSlot, SmartObjectID, SmartObjectRuntime, SmartObjectSlot, SmartObjectUse, SmartObjectUseConfig
- **/Script/HoagieRuntime** (4 struct, 1 enum, 2 delegate): CachedSeatCollision, EHoagieBoostState, FortHeliFlightModel, HoagieAltimeterTraceUpdatedDelegate, HoagieBoostStateDelegate, ReplicatedHeliControlState, RotorHit
- **/Script/EpicCMSUIFramework** (4 struct, 1 enum, 2 delegate): EDateType, EpicCMSLayoutTypeMapping, EpicCMSTileTypeMapping, OnCMSTileConfirmed, SlotDescription, TileDefinition, onLoadingCompleted
- **/Script/EpicGameplayStatsRuntime** (4 struct, 3 enum): EEpicLeaderboardDataType, EEpicLeaderboardTimeWindow, EEpicLeaderboardUpdateFunction, GameplayStatMetadataTableRow, GameplayStatTag, ManagedGameplayTagDataTableItem, TagTableManagerHelper
- **/Script/SubtitlesWidgets** (1 struct, 4 enum): ESubtitleDisplayBackgroundOpacity, ESubtitleDisplayTextBorder, ESubtitleDisplayTextColor, ESubtitleDisplayTextSize, SubtitleFormat
- **/Script/ScheduledEvents** (4 struct): CalendarDownload, EventChannelState, EventRecord, EventsTimeline
- **/Script/LocalNotifications** (1 struct, 3 enum): ELocalNotificationEventType, ELocalNotificationParams, ELocalNotificationType, LocalNotificationData
- **/Script/SocialUMG** (2 struct, 2 delegate): SocialChatMessageEntryStyle, SocialChatMessageEntryTextStyle, SocialChatTextChanged, SocialChatTextCommitted
- **/Script/GameplayBehaviorsModule** (2 struct, 1 enum): AgentGameplayBehaviors, EGameplayBehaviorInstantiationPolicy, MontagePlaybackData
- **/Script/EventModeRuntime** (2 struct, 1 enum): EFortEventModeEmoteToPlay, EventModeFocusActor, EventModeWidgetCachedData
- **/Script/LiveStreamAnimation** (2 struct, 1 enum): ELiveStreamAnimationRole, LiveStreamAnimationHandle, LiveStreamAnimationLiveLinkSourceOptions
- **/Script/PurchaseFlow** (3 struct): PurchaseFlowItem, PurchaseFlowOffer, PurchaseFlowReceiptParam
- **/Script/SidecarSys** (2 struct): InventoryFlushPayload, SidecarFileInfo
- **/Script/FortSoundCueTemplates** (1 struct): GliderThrustData
- **/Script/GameFeedback** (1 struct): LogSubmitOptions

### C3: generator limitation (out/return/reference delegate parameters, or an unresolved type that a later module or class change resolves) (87)

- **/Script/FortniteUI** (31 struct, 37 delegate): ActionBindingIconPicker, AthenaCustomizationParams, AthenaItemShopSectionDisplayData, AthenaPlaylistEntry, BarrierObjectState, BuildingCategoryOptionData, CardPackOffer, CmsJsonMessages, CreateGridWidget, CreativeAdData, CreativeAdJsonObject, CreativeAdSource, FortAdvancedLatentConfirmationDialogResultDelegate_NUI, FortAthenaTutorialStandaloneStepInfo, FortConfirmationResultDelegate_NUI, FortFocusedBuildingInfo, FortPlayerSurveyAnswerWidgetDataTableEntry, FortReceivedItemLootInfo, FortSquadSlotSortChanged, FortStateStyle, FortTabNavigateRequest, FortTopBarTabButtonInfo, FortUINavigationOperation, FortUINavigationRequest, FortUIPerk, FortUIPerkTier, FortUIStatStyle, FortUISurvivorSquadStatMatch, FortVaultTabNavigateRequest, GGCenterDisplayEntry, GetSortKeyFor, GiftingErrorText, HudLayoutToolProxyPropertyData, IconPicker, ItemCategoryOptionData, ItemListCategoryArray, ItemShopCurrencyDislayData, OnAthenaGamePhaseChanged, OnCameraChangedDelegate, OnCraftItemFailed, OnEndOfDayRecap, OnEquipSelectionDelegate, OnFireModeChange, OnFocusedBuildingStateChanged, OnGetItemToCompare, OnHordeTierCompleteDelegate, OnHordeTierInitializedDelegate, OnHordeWaveCompleteDelegate, OnInventoryFiltersWithUnseenItemsChanged, OnItemTypesWithUnseenItemsChanged, OnLocalPlayerLost, OnPlayerDeath, OnQuickTimeEventFeedbackWidget, OnQuickbarContentsChanged, OnQuickbarFullUpdate, OnRequestPromptDisplayDelegate, OnResearchItemAction, OnScoreStatChanged, OnSelectionChangedDelegate, OnShowPickerDelegate, OnSlotItemConfirmed, OnSubGameChanged, PTTStateChange, QuickbarFocusChangedDelegate, SelectItemToSpawnDelegate, SetWorldResourceWidgetVisibility, SquadQuickChatOptionData, UpdateTutorialAnnouncementDelegate
- **/Script/FortniteGame** (11 struct, 6 delegate): AnimTagProperty, BuildingFoundationStreamingData, FortCosmeticLockerSlotData, FortFinderProperty, FortOnCameraFrameTargetSettingsChangedDelegate, FortPartyMemberRepData, FortPartyRepData, FortPlacementActorFinderEntry, FortPlacementActorFinderInfo, FortSourDropperColorData, HUDMessageData, ObjectCostContainer, OnGamePhaseStepChanged, OnPartyTypeChanged, OnRejoinCheckCompletedDelegate, OnShowHUDMessage, OnUIGameplayCueEvent
- **/Script/MeshNetwork** (1 struct): AggregatedFunction
- **/Script/CommonUI** (1 delegate): OnCustomNavigationEvent

Reasons (count): return, out or non-const reference parameter (4); parameter type unresolved (TODO_StructOtherModule_14295cb30) (3); parameter type unresolved (TODO_EnumOtherModule_142abec50) (3); member Sprite: unresolved type: TODO_ClassOutsideNativeSet_140842200 (2); member BangType: unresolved type: TODO_EnumOtherModule_142b24020 (2); parameter type unresolved (TODO_EnumOtherModule_142b636e0) (2); parameter type unresolved (TODO_StructOtherModule_142b25640) (2); member Function: unresolved type: TODO_Class_142e49b80 (1)

### C4: engine or plugin class not in the UE 4.25 source (79)

- **/Script/UMG** (1 struct, 17 delegate): CustomWidgetNavigationDelegate, GenerateWidgetForObject, GenerateWidgetForString, GetBool, GetCheckBoxState, GetFloat, GetInt32, GetLinearColor, GetMouseCursor, GetSlateBrush, GetSlateColor, GetSlateVisibility, GetText, GetText, GetWidget, OnPointerEvent, OnReply, RadialBoxSettings
- **/Script/Engine** (3 struct, 10 enum, 3 delegate): CompressedSegment, EAudioBusChannels, EAudioSpectrumBandPresetType, EAudioSpectrumType, EConcurrencyVolumeScaleMode, EFFTPeakInterpolationMethod, EFFTSize, EFFTWindowType, EGainParamMode, EMIDCreationFlags, ETextureDownscaleOptions, OnAudioFadeChangeSignature, OnSubmixSpectralAnalysis, OnSubmixSpectralAnalysisBP, SkinWeightProfileManagerTickFunction, SoundSubmixSpectralAnalysisBandSettings
- **/Script/Niagara** (11 struct, 2 enum): ENiagaraFastPathInputType, ENiagaraFastPathMode, NiagaraFastPathAttributeNames, NiagaraFastPathFloatInput, NiagaraFastPathIntInput, NiagaraFastPath_Module_EmitterLifeCycle, NiagaraFastPath_Module_EmitterScalability, NiagaraFastPath_Module_SpawnBurstInstantaneous, NiagaraFastPath_Module_SpawnPerUnit, NiagaraFastPath_Module_SpawnRate, NiagaraFastPath_Module_SystemLifeCycle, NiagaraFastPath_Module_SystemScalability, NiagaraScriptInstanceParameterStore
- **/Script/LiveLinkInterface** (7 struct): LiveLinkCurveElement, LiveLinkFrameData, LiveLinkFrameRate, LiveLinkInterpolationSettings, LiveLinkRefSkeleton, LiveLinkTimeCode, LiveLinkTimeSynchronizationSettings
- **/Script/ControlRig** (2 struct, 2 enum): ControlRigIOVariable, ControlRigOperator, EControlRigOpCode, EUnitExecutionType
- **/Script/MovieSceneTracks** (4 struct): MovieSceneCameraShakeSourceShakeSectionTemplate, MovieSceneCameraShakeSourceTrigger, MovieSceneCameraShakeSourceTriggerChannel, MovieSceneCameraShakeSourceTriggerSectionTemplate
- **/Script/Synthesis** (2 struct, 2 enum): ESourceEffectFilterParam, EStereoDelayFiltertype, SourceEffectFilterAudioBusModulationSettings, SubmixEffectStereoDelaySettings
- **/Script/Water** (3 struct): WaterStreamingCell, WaterStreamingGrid, WaterWaveParams
- **/Script/AIModule** (1 struct, 1 delegate): ActorPerceptionInfoUpdatedDelegate, ActorPerceptionUpdateInfo
- **/Script/LiveLinkMessageBusFramework** (2 struct): LiveLinkSubjectDataMessage, LiveLinkSubjectFrameMessage
- **/Script/LiveLink** (1 struct): LiveLinkRetargetAssetReference
- **/Script/MovieScene** (1 struct): MovieSceneSpawnSectionTemplate
- **/Script/AudioMixer** (1 enum): EMusicalNoteName
- **/Script/Slate** (1 enum): ETextTransformPolicy
- **/Script/MediaAssets** (1 enum): MediaTextureOutputFormat
- **/Script/LiveLinkComponents** (1 delegate): LiveLinkTickSignature

## 6. Cause categories

| Code | Category | Classes+ifaces | Structs+enums+delegates | Total | Meaning and action |
|---|---|---:|---:|---:|---|
| C1 | missing plugin/runtime module in our project | 156 | 141 | 297 | The module is absent from the UE 4.25 tree and the UE 4.26 tree, so no source exists. Generate it as a runtime module (as CommonUI and McpProfileSys were), add it to the uproject and the game Build.cs, then add its classes to the set. |
| C2 | not in native_class_set.txt (just add it) | 19 | 0 | 19 | Usable module, and the parent is generated or engine. |
| C3 | blocked by a generator limitation | 7 | 87 | 94 | Classes: a hand-written stand-in parent (a generated child would clash with it). Structs and delegates: `not_emittable` (out/return/non-const reference delegate parameter, or a type from another module). |
| C4 | engine class not present in UE 4.25 source | 24 | 79 | 103 | The reflected package is an engine or plugin module, which the generator cannot write into. Some names exist in the UE 4.26 tree (hint). |
| C5 | other | 10 | 1350 | 1360 | Unnamed blocks (no name), or a struct, enum or delegate outside the promotion closure (not referenced by any promoted class). |

Unnamed blocks are counted under C5 in the classes column (10).

## 7. Ordered action list (for the build owner)

1. **Generate the C1 modules.** Add these packages to `RUNTIME_MODULE_PACKAGES` in `porting/gen_native_classes.py`, register them in `porting/native/FortniteLinux.uproject`, list them in the game Build.cs, then add their classes and interfaces (and non-engine ancestors) to `native_class_set.txt`. Modules (count = C1 items rooted in the module, classes and non-class types): SocialUMG (42), KairosSceneCapture (23), GameSubCatalog (22), SmartObjectsModule (20), MantisRuntime (19), NVIDIAGfeSDK (17), LauncherSocialDefaults (16), EpicCMSUIFramework (15), FortSoundCueTemplates (14), HoagieRuntime (13), GameplayBehaviorsModule (12), LauncherSocialTypes (12), SubtitlesWidgets (9), EpicGameplayStatsRuntime (9), LiveStreamAnimation (6), EventModeRuntime (5), SidecarSys (5), Solaris (5), LocalNotifications (5), PurchaseFlow (4), ScheduledEvents (4), NetUI (3), EventModeUI (2), LiveStreamVoice (2), ForwardingChannels (2), GameFeedback (2), HoagieUI (1), FacialAnimSystem (1), Crashlytics (1), DiscordRPC (1), UACBase (1), F5Player (1), F5PlayerFactory (1), SolarisUi (1), FortniteEngineLoadingScreen (1). Non-class types of a module enter the emitted closure only when a promoted class uses them.
   - Decision: NVIDIAGfeSDK (17 non-class types) is a third-party SDK package; generate it only if a class of ours needs it.
   - Modules that only carry non-class types (no class in the list above) need no set entry; their structs and enums enter the closure only when a promoted class uses them.
2. **Add the 19 FortniteUI interfaces (C2) to `native_class_set.txt`:** UAthenaPlayerInfoBaseWidgetInterface, UFortAthenaMOTDNewsChildren, UFortBangInterface, UFortBangWrapperContentInterface, UFortGlobalActionDetailsDataSource, UFortHUDObjectiveSizeInterface, UFortHUDShutdownTimerInterface, UFortItemListEntryWidgetInterface, UFortItemListViewInterface, UFortItemManagementMulchPanel, UFortItemViewContextInterface, UFortMobileActionButtonInterface, UFortNodeCanvasEntityInterface, UFortNodeCanvasInterface, UFortPrimaryContentInterface, UFortPrioritizedWidget, UFortTabButtonInterface, UFortTimespanDataSource, UHUDLayoutToolInterface. Each has parent `UInterface`, which the generator emits.
3. **Resolve the hand-written stand-ins (C3, 7 classes).** `AFortGameModeAthena` (Apollo stand-in, blocklisted), `AFortStaticMeshActor`, `AFortHLODSMActor`, `AFortLevelScriptActor` (hand-written in `native/Source/FortniteGame/Public/`) block AFortGameModeBR, AFortPlayspaceGameMode, AFortAthenaTutorialMarker, AFortAthenaTutorialShootingAutofireTarget, AFortInstancedStaticMeshActor, AFortHLODSMSectionActor, AFortFrontendLevelScriptActor. Replacing a stand-in is a decision (it changes Apollo behaviour); until then these 7 stay missing.
4. **Generator: support out and return delegate parameters, and the `TODO_*Other*Module` types (C3 structs and delegates, 87 rows).** Most `TODO_StructOtherModule`/`TODO_EnumOtherModule` entries resolve once their module is added (step 1). Out/return delegate parameters are already listed as a next step in `native_classes_status.md`.
5. **Engine and plugin classes (C4, 24 classes, 79 structs/enums/delegates).** Cannot be generated into an engine module. Options per item: backport from the UE 4.26 tree (47 names are present there by name, for example UVirtualTexture2D, UAudioBus, UVirtualTextureBuilder, UMovieSceneSpawnTrack), or leave as is. The adapted Water plugin also lacks 3 classes (AWaterBodyProxy and two others) that the exe reflects.
6. **Package-name mismatches (9 items, marked in the C4 detail).** The exe's package for these names differs from the 4.25 module that declares the same name: MovieScene vs MovieSceneTracks (UMovieSceneBoolSection, UMovieSceneSpawnSection, UMovieSceneSpawnTrack, MovieSceneSpawnSectionTemplate), Engine vs AudioMixer (EAudioSpectrumType, EFFTPeakInterpolationMethod, EFFTSize, EFFTWindowType), LiveLinkComponents vs LiveLink (LiveLinkTickSignature). Whether the exe or the 4.25 tree is right is not established here; check the exe's package path before generating anything for them.
7. **Enable plugins only if a promoted class needs them.** 151 engine classes/interfaces and 562 structs/enums/delegates are in engine source but in plugins that are not enabled (b-not-enabled). They are not compiled into our build. The largest by type count: ControlRig (372), Synthesis (101), Paper2D (44), EditableMesh (42), LiveLink (23).
8. **C5 structs, enums and delegates (1,350 rows):** not referenced by any promoted class. Nothing to do unless a promoted class starts to use them; the closure brings them in.
9. **Census.** Re-run `python3 porting/census_ui_audio.py` (not done here). Its table 1 is stale (section 8).
10. **Stale documents.** `native_classes_status.md` says Niagara, AnimationSharing, PhysXVehicles, AnimationBudgetAllocator and SignificanceManager are not enabled; `native/FortniteLinux.uproject` now enables them. The blocklist now has 8 entries, not 18, and no plugin-based entries. Update when the next promotion runs.

## 8. Census cross-check (`ui_audio_census.md`)

Two ranked tables in the census. The census was built 2026-10-31 18:06 (`generated/ui-audio-census.json`); the generated headers it reports as missing were written at 20:57. So its first table is out of date. Its second table, which also reads the reflection tables and the set, agrees with this analysis wherever it has a class-kind row.

| Import | Census status | Our status | Our cause | Agree |
|---|---|---|---|---|
| FortniteGame.FortNavigationVisibilityComponent | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortGameModeFrontEnd | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortPlayerController | not recovered (gameplay) | a_generated |  | NO (stale census) |
| FortniteGame.FortQuestManager | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortAsyncAction_SendQuestStatEvent | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortBangCheckComponent_Map | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortBangCheckComponent_Customization | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortBangCheckComponent_Donut | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortBangCheckComponent_Elevator | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortBangCheckComponent_Faction | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortPlayerControllerFrontEnd | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortPlayerMannequin | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FrontEndSettings | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.EFrontEndCamera | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortGameModeMinimal | not recovered | a_generated |  | NO (stale census) |
| FortniteUI.FortFrontEndContext | not recovered | a_generated |  | NO (stale census) |
| FortniteGame.FortKismetLibrary | hand-written in porting/native source, not in set | c_hand |  | yes |
| Paper2D.PaperSprite | present in engine plugin, not in set | b_engine |  | yes |
| FortniteUI.FortSocialNameTextBlock | missing from set and source | e_missing | C1 | yes |
| GameplayAbilities.AbilitySystemBlueprintLibrary | present in engine plugin, not in set | b_engine |  | yes |
| SubtitlesWidgets.FortMediaSubtitlesPlayer | missing from set and source (plugin absent) | e_missing | C1 | yes |
| AnimationBudgetAllocator.SkeletalMeshComponentBudgeted | present in engine plugin, not in set | b_engine |  | yes |
| ChaosCloth.ChaosClothingSimulationFactory | present in engine plugin, not in set | b_engine |  | yes |
| FortniteGame.FortStaticMeshActor | hand-written in porting/native source, not in set | c_hand |  | yes |
| GameplayAbilities.AbilitySystemComponent | present in engine plugin, not in set | b_engine |  | yes |
| Niagara.NiagaraComponent | present in engine plugin, not in set | b_engine |  | yes |
| Niagara.NiagaraSystem | present in engine plugin, not in set | b_engine |  | yes |
| Paper2D.PaperSpriteAtlas | present in engine plugin, not in set | b_engine |  | yes |

Not compared: 20 rows are subobject names (instance names such as `FortniteUI.Root`, not `/Script` classes), and one row is a function (`FortniteGame.SendClientStatEvent`).

The first table lists 16 rows with census status "not recovered". Every one of them has a generated header now (for example `UFortNavigationVisibilityComponent.h`, `AFortPlayerController.h`, `EFrontEndCamera.h`). That is a stale census output, not a disagreement in the analysis.

**Verdict.** The 16 disagreements are a stale census output, not an error in this analysis: this analysis matches the current tree, and the census was run before the generated headers existed. In the census second table (12 class rows), 2 names are in our MISSING set (FortSocialNameTextBlock and FortMediaSubtitlesPlayer, both C1: the module is absent from both trees). The other 10 are not missing: 2 hand-written (FortKismetLibrary, FortStaticMeshActor, c) and 8 engine-source classes (b).

## 9. Caveats

- Name matching only (plus module). No runtime or behaviour check. A `b` status means the declaration exists in the 4.25 source; it does not mean it builds in our project.
- The UE 4.26 presence (47 of the C4 items) is a name match in another engine version, a hint only.
- The package-name mismatches (section 7, item 6) are kept as missing, per the task's rule; they are reported in each row's C4 detail.
- Delegates: 13 (package, name) pairs appear twice in `delegate_blocks.json` (sparse delegates shared by owners). Each block is counted once.
- The 10 unnamed class blocks have no package. They are counted once, under C5.
- A `game` vs `engine` origin label is the module's location, not a judgement about ownership.
- Out of scope: one out-of-scope package is excluded from every table and count (see the scope note at the top).

Scratch analysis scripts (not committed): `/tmp/claude-1000/-opt-fortnite/cb6b2fd6-af2b-4311-9c18-8992e33f0be4/scratchpad/coverage.py`, `/tmp/claude-1000/-opt-fortnite/cb6b2fd6-af2b-4311-9c18-8992e33f0be4/scratchpad/census_check.py`, `/tmp/claude-1000/-opt-fortnite/cb6b2fd6-af2b-4311-9c18-8992e33f0be4/scratchpad/finalize.py`, `/tmp/claude-1000/-opt-fortnite/cb6b2fd6-af2b-4311-9c18-8992e33f0be4/scratchpad/report.py`.
