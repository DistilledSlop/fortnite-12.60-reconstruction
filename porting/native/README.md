# Native build harness

This is a newly authored UE project used to test Linux compilation and to
reconstruct the game module incrementally. Its module is named `FortniteGame`
so that future classes can use the `/Script/FortniteGame` path found in cooked
assets. It contains no Fortnite gameplay code, no supplied assets, and no EAC
integration. It is not playable yet.

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
by Git. This validates only the empty engine map; Fortnite gameplay, content,
and server integration remain to be implemented.

The optional `-FortniteProbeAsset=/Game/Path/Asset.Asset` switch asks the game
module to load one asset after engine initialization and logs its resolved
class or `LOAD_FAILED`. Recovered asset files must be copied into the matching
`FortniteLinux/Content/` path in the ignored staged build before this test.
For `Texture2D`, the probe also logs dimensions, mip count, and first-mip byte
count. It does not validate visible rendering when run with `-nullrhi`.

`FortniteMapClasses.h` contains initial Fortnite class definitions
needed to deserialize the supplied `FortniteEmptyDedicated` map. After staging
its recovered `.umap` and `.uexp` under `FortniteLinux/Content/Maps/`, launch
the staged executable with `/Game/Maps/FortniteEmptyDedicated` as its map
argument. This map enters play, but the class definitions are only starting
points for full gameplay reconstruction.

`FortniteAthenaGameplay.h` defines initial native `FortGameModeAthena`,
`FortPlayerPawn`, and `FortPlayerPawnAthena` classes. The game mode selects the
Athena pawn, which uses standard UE character movement and replicates on a
dedicated server. `Config/DefaultInput.ini` binds movement, view, and jump
controls. These are newly authored placeholders for the original class names,
not a reconstruction of Fortnite movement or gameplay rules.

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

