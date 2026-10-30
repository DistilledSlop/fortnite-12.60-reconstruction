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
`python3 porting/stage_apollo_grid_meshes.py` stages the Linux packages and
shader archives. All generated meshes, materials, and the manifest are ignored
by Git.

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
