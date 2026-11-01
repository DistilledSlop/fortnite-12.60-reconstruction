# Native build harness

This UE project rebuilds the `/Script/FortniteGame` module for Linux against
the recovered 12.60 classes and cooked assets. The latest offline checksum
model matches 4,617 of 4,617 generated classes, and 251 recovered
behavior bodies are spliced into the build. Both native client/server direct
joins pass. Most game behavior, interactive UI and usable content remain
unfinished, so the project is not playable yet. Launcher, anti-cheat and
entitlement compatibility remain required for the finished project.

After the UE 4.25 source checkout is set up, build a Linux Development target:

```sh
/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FortniteLinux Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
```

The project disables unused SteamVR and UdpMessaging plugins. Build the editor
and shader worker, then cook the tiny engine Entry map:

```sh
cd /opt/fortnite/UnrealEngine
Engine/Build/BatchFiles/Linux/Build.sh FortniteLinuxEditor Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
Engine/Build/BatchFiles/Linux/Build.sh ShaderCompileWorker Linux Development -NoHotReload
Engine/Binaries/Linux/UE4Editor-Cmd \
  /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -run=Cook -TargetPlatform=LinuxNoEditor -Map=/Engine/Maps/Entry -unattended -nop4
```

Stage the cooked project with `RunUAT.sh BuildCookRun` using `-skipbuild
-skipcook -stage -archive -platform=Linux -clientconfig=Development`. A staged
headless run with `-nullrhi -nosound -unattended -ExecCmds=Quit` loaded
`/Engine/Maps/Entry` and exited with code 0. The staged output stays ignored
by Git. That historical probe validates only the empty engine map; later
recovered-map and direct-join probes are recorded in `porting/HANDOFF.md`.

The optional `-FortniteProbeAsset=/Game/Path/Asset.Asset` switch asks the game
module to load one asset after engine initialization and logs its resolved
class or `LOAD_FAILED`. Recovered asset files must be copied into the matching
`FortniteLinux/Content/` path in the ignored staged build before this test.
For `Texture2D`, the probe also logs dimensions, mip count, and first-mip byte
count. It does not validate visible rendering when run with `-nullrhi`.

`Generated/` (FortniteGame) and `PlayspaceFramework/Generated/` hold the classes promoted from
the reflection tables (`porting/promote_native_classes.sh`; do not hand-edit). They cover the
Athena join chain (`FortPlayerPawnAthena`, `FortPlayerStateAthena`, `FortGameStateAthena`,
`FortPlayerControllerAthena`, `FortWorldSettings`, and the Playspace parents). Their properties and
RPCs are partial, and bodies are stubs. See `porting/native_classes_status.md`.

`FortniteMapClasses.h` still holds the hand-written classes outside that set that the
`FortniteEmptyDedicated` map needs. After staging its recovered `.umap` and `.uexp` under
`FortniteLinux/Content/Maps/`, launch the staged executable with
`/Game/Maps/FortniteEmptyDedicated` as its map argument. The map enters play, and these are starting
points for full gameplay reconstruction.

`FortniteAthenaGameplay.h` defines the native `FortGameModeAthena` (server-only, still on
`AGameModeBase`). It selects the generated `FortPlayerPawnAthena` as its default pawn and keeps the
Apollo stand-in spawns. `Config/DefaultInput.ini` still binds movement, view, and jump controls, but the
generated pawn has no input bindings. These are placeholders, not a reconstruction of Fortnite movement
or gameplay rules.

`AApolloLandscapeTile` is a temporary replicated static mesh actor for the 61
converted Apollo landscape tiles. `AFortGameModeAthena` reads their transforms
from the ignored staged `Config/ApolloLandscapeGrid.json`; each client loads
the corresponding cooked mesh when it receives the replicated tile ID.
`AApolloSunLight` is a temporary replicated directional light because the
original streamed lighting levels do not load. It exposes the pale converted
terrain under software Vulkan; it does not recreate Apollo's sky or materials.
`python3 porting/stage_apollo_grid_meshes.py` stages the Linux packages and
shader archives. All generated meshes, materials, and the manifest are ignored
by Git.

`FortniteMaterialRebuild` accepts `-Blend=Opaque`, `-UVChannel=1`, and an
optional `-Mesh=/Game/...` to build a lit atlas material and assign it to a
converted terrain mesh. D4's `Terrain_D4` atlas came from
`python3 porting/bake_apollo_tile.py D4`; its imported editor texture, material,
mesh edit, cooked packages, and staged payloads remain ignored.
`-AllApolloTiles` creates the remaining tile materials in one editor run after
the atlases are imported. The current Linux cook and stage contain 61 tile
materials and 61 terrain textures.

`-FortniteProbeTerrainCollision` performs one vertical line trace through D4
after Apollo's converted grid spawns. The editor commandlet
`-run=FortniteTerrainCollision -Tile=D4` rebuilds one imported tile with
triangle collision before a Linux recook. Use `-All` in place of `-Tile=D4`
to process all 61 imported grid meshes. `-FortniteProbePawnGround` logs whether
the server pawn settles into Walking movement on D4 five seconds after it
spawns. The source and cooked packages remain ignored.

`-FortniteProbeMapMeshes` logs any static mesh actor with invalid component
bounds after a map begins play. It helped identify a UE glTF importer stride
bug while testing the recovered skybox map under Vulkan.

## Apollo Blueprint replacements

Run the editor commandlet once to create four editable Blueprints at the
original Apollo package paths. It refuses to overwrite any existing source
asset unless `-Update` is explicitly supplied:

```sh
/opt/fortnite/UnrealEngine/Engine/Binaries/Linux/UE4Editor-Cmd \
  /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -run=FortniteApolloBlueprint -unattended -nop4 -nullrhi -nosound
```

`Config/DefaultGame.ini` lists these directories for the Linux cook. Run the
editor cook command above, then `python3 porting/stage_native_blueprints.py`.
The generated `.uasset` and cooked files stay ignored. The commandlet gives
the storm actor the component names required by the recovered map, but its
material, mesh, and gameplay behavior still need reconstruction. The shadow
actor's instanced mesh is intentionally absent because its saved Windows
component buffer does not deserialize in the stock Linux runtime.

## Rebuilt Apollo cloud and backdrop materials

`python3 porting/prepare_apollo_background_materials.py` turns the material
reports under ignored `generated/` into `apollo-background-materials.json`
(parameter defaults, instance overrides) and texture import settings. After
importing those textures with `ImportAssets`, run
`UE4Editor-Cmd FortniteLinux.uproject -run=FortniteBackgroundMaterials
-Spec=<that json>` to create `M_Volcano_Cloud` and its `M_Volcano_Cloud_Cove`
instance, plus `M_BackDrop_P` and its `M_BackDrop_1/3/4` instances, at the
original `/Game` paths. `-SkipCloud` and `-SkipBackdrop` limit a run. The
commandlet logs every instance parameter the simplified parent has no
counterpart for. `Config/DefaultGame.ini` always-cooks those folders and the
background mesh folder; `stage_apollo_background.py` stages them.

## Network staged plan step 0

`Config/DefaultEngine.ini` sets `net.MagicHeader=1110`, `net.MaxChannelSize=65535`,
`AESHandlerComponent` encryption, and disables Oodle with `bEnableOodle=False`.
`FortniteGame.cpp` reports network version checksum 683914758 and accepts any
remote version, logging the first value seen. The Oodle handler cannot be created
without the Oodle Net SDK, so it reserves no packet bit on either peer.

Encryption keys are never stored in the repository. The server answers
`?EncryptionToken=` requests with a 32-byte key from the environment:

- `FORTNITE_NET_ENCRYPTION_KEY`: 64 hex characters or base64 of 32 bytes, or
- `FORTNITE_NET_ENCRYPTION_KEY_FILE`: a file containing one such value.

Without either variable the direct connection still works. Passing
`-FortniteOriginalClient` skips the invented Apollo stand-in actors in
`AFortGameModeAthena::StartPlay`.


## Water plugin (UE 4.26 backport)

`Plugins/Water/` is Epic's 4.26 Experimental/Water plugin adapted to 4.25 and stays out of Git.
Recreate it with `bash porting/prepare_water_plugin.sh` (copies from `/opt/fortnite/UnrealEngine4260`,
then applies `patches/ue425-water-plugin-*`). It provides `/Script/Water` classes such as
`WaterSplineComponent` for the cooked Apollo maps. GPU Gerstner wave data, ocean collision and
exclusion volumes are stubbed (see `map_compatibility.md`, "Water plugin backport").

## Editor target and cook check

Cause of the editor exit 1. `UE4Editor-Cmd FortniteLinux.uproject -run=Cook ...` (and any unattended run) stopped on a message dialog:
`Plugin 'Oodle' failed to load because module 'OodleCompressionFormat' could not be found`. The Oodle, Landmass, GeometryProcessing and Water plugins were enabled in the uproject after the editor binaries were last built (07:56), so their modules had never been compiled. The Oodle SDK is not the cause: `OodleCompressionFormat` builds with `HAS_OODLE_DATA_SDK=0` and needs no SDK.

Fix. The uproject is unchanged: Oodle and AESHandlerComponent stay enabled, and the server config still drops OodleHandlerComponent. Rebuilding `FortniteLinuxEditor` compiled the plugin modules, but it first failed on editor-only code in the gitignored Water copy. The copy now has these shims for 4.26-only APIs:

- `Plugins/Water/Source/Runtime/Private/ConvertWaterBodyActorsCommandlet.cpp`: `ULevel::GetPackage()` becomes `GetOutermost()` (lines 104, 189), and the `GetLoadedExternalActorPackages()` append is removed.
- `Plugins/Water/Source/Runtime/Private/WaterVertexFactory.cpp`: line 100 uses the local `WATER_IMPLEMENT_TEMPLATE_VERTEX_FACTORY_TYPE_EX` macro, and the macro takes its type name from `TEXT(#FactoryClass)` (line 80). The engine macro does the same. The earlier hard-coded `"TWaterVertexFactory"` gave the `<false>` and `<true>` instances the same name, and the cook then failed with `MaterialShader.cpp:1597` (`!MeshShaderMap->HasShader(...)`).

Note: `porting/patches/ue425-water-plugin-backport.patch` (outside `porting/native/`) does not yet contain these three edits, so `prepare_water_plugin.sh` would recreate the broken copy. It needs regenerating.

Evidence (logs in `/tmp/fn-editor-check/`, not tracked):

- Before: exit 1 at the Oodle dialog (`repro1.log`).
- `FortniteLinuxEditor` rebuild before the shims: exit 6, 6 errors in the Water sources listed above (`build-editor-1.log`).
- After the shims: `FortniteLinuxEditor`, `FortniteLinux`, `FortniteLinuxServer` and `ShaderCompileWorker` each built with exit 0, Development.
- `UE4Editor-Cmd ... -ExecCmds=Quit -nullrhi`: exit 0, `Mounting plugin Oodle` with no Oodle dialog. The only dialog left is the default-property warning for missing `/Water/...` content (`repro2.log`).
- Cook, `-run=Cook -TargetPlatform=LinuxNoEditor -Map=/Engine/Maps/Entry -unattended -nop4`: exit 0 with Water enabled, `Cooked packages 408 Packages Remain 0 Total 408` (`cook-final.log`). Before the vertex factory name fix it crashed (`cook.log`). With Water disabled temporarily it passed (`cook-nowater.log`), which isolated the Water shim as the cause. The uproject was restored from a backup and verified.
- Stage: `RunUAT.sh BuildCookRun -skipbuild -skipcook -stage -platform=Linux -clientconfig=Development -stagingdirectory=<new dir>`, no `-archive`: `STAGE_EXIT=0`, 2.7 GB (`stage.log`). The stage was deleted afterwards.
- Staged `FortniteLinux.sh /Engine/Maps/Entry -nullrhi -nosound -unattended -ExecCmds=Quit`: exit 0, `LogLoad: LoadMap: /Engine/Maps/Entry` (`stage-run.log`).
- Join smoke, direct mode: `porting/server/smoke_join.sh` hardcodes `porting/generated/linux-stage`, which was not modified. The same steps ran against a hardlinked copy of that stage with the new `FortniteLinux` and `FortniteLinuxServer` binaries. Result: `Local network version checksum: 683914758`, `IpNetDriver listening on port 7777`, `ss -lun` shows `UNCONN 0.0.0.0:7777` while the server runs, `Join succeeded:`, and `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated, Game: /Script/FortniteGame.FortGameModeAthena)`.
- The existing `porting/generated/linux-stage` still holds the old executables until it is restaged.

## Staging the client: CEF3 libraries

Since the plugin set grew (OnlineSubsystemUtils and friends), `FortniteLinux` links `libcef.so`. The stage
(`porting/generated/linux-stage/LinuxNoEditor`) is assembled by copying binaries, so add the engine's CEF3
directory where the client's RPATH looks for it. Hard links keep it free on the same filesystem:

```sh
mkdir -p porting/generated/linux-stage/LinuxNoEditor/Engine/Binaries/ThirdParty/CEF3
cp -al /opt/fortnite/UnrealEngine/Engine/Binaries/ThirdParty/CEF3/Linux \
   porting/generated/linux-stage/LinuxNoEditor/Engine/Binaries/ThirdParty/CEF3/Linux
```

`ldd FortniteLinux` must then report no missing libraries. The server does not need it.

## NetSerialize self-test

`Source/FortniteGame/Private/NetSerializeSelfTest.cpp` round-trips the five recovered `struct_net` bodies
(`FSharedRepMovement`, `FFortAISharedRepMovement`, `FReplicatedPhysicsPawnState`, `FFortAIAttributeReplicationProxy`,
`FCreateBuildingActorData`) through `FBitWriter` and `FBitReader`, and checks the bit counts the layout fixes: 432 for the
physics state, 177 for the building data (176 for handle 255, see the file). It needs no server, level or RHI:

```sh
env -u LD_LIBRARY_PATH stdbuf -oL -eL porting/generated/linux-stage/LinuxNoEditor/FortniteLinux.sh \
  -NetSerializeSelfTest -nullrhi -nosound -unattended -log
```

It logs one `NetSerializeSelfTest: <Struct> <case> PASS` or `FAIL` line per case, then `NetSerializeSelfTest: ALL PASS`
(exit 0) or `FAILED n` (exit 1). The flag is ignored by any other run.

## Game instance selection

`DefaultEngine.ini` selects `/Script/FortniteGame.FortGameInstance`, matching the
extracted 12.60 configuration. The generated native class is present, but its
methods are largely stubs; this setting alone does not implement the original
frontend or its initialization lifecycle. The Entry map and Athena game mode
remain the defaults for the working native direct-join path.

A November 2026 isolated hardlinked copy of the existing Linux stage, with the
updated config copied over, loaded `/Engine/Maps/Entry` under `-nullrhi` and
`obj list class=FortGameInstance` listed one live instance under the game engine.
The process exited normally after `Quit`. A dedicated server on port 17777 and a
client using that stage completed a direct join on `FortniteEmptyDedicated`
with `FortGameModeAthena`; the client was stopped by the test timeout. The
startup still reported missing Water plugin assets. This smoke test does not
validate the original frontend or UI behavior.
