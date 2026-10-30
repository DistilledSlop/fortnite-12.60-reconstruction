# Recovered map loading

`cue4inspect --scan-maps` parsed the package headers of all 3,694 `.umap`
files in the supplied pak set. Each imports `/Script/FortniteGame`. The
smallest candidate, `FortniteEmptyDedicated`, has 31 imports, no `/Game`
asset imports, and 15 exports. It imports `FortLevelScriptActor`,
`FortWorldSettings`, and `FortMetaNavArea_Obstacles` from the missing game
module.

Three initial UCLASS definitions with those names were added to the new
`FortniteGame` module. They supply compatible UE base classes so the map can
deserialize; they do not implement Fortnite behavior or all original
properties. The recovered `.umap` and `.uexp` were staged under
`FortniteLinux/Content/Maps/` in ignored output. The staged Linux executable
loaded `/Game/Maps/FortniteEmptyDedicated`, brought its world up for play,
used `GameModeBase`, and exited with code 0. This worked both with `-nullrhi`
and with a Vulkan swapchain under Xvfb and Mesa Lavapipe. A warning says the
saved `World.DefaultPhysicsVolume` property was skipped as no longer
serializable; its effect on gameplay remains untested.

`PG_Obstacle_S_3_Tower_A` was the next candidate. Its direct imports include
two building Blueprints. A bounded `/Game` dependency walk recovered 365
packages, then the Linux runtime reported missing native classes including
`AIHotSpotConfig`, `BuildingActorHotSpotConfig`, and
`BuildingEditModeMetadata_Wall`/`Floor`. It also reported missing engine
material functions and ended with an async loading assertion (exit code 139).
This map has not entered play. The result shows why package extraction alone
does not restore a Blueprint-based level.

All recovered map and asset bytes, scans, and logs are ignored generated data;
the repository contains only the new code and this analysis.

`Skybox_Outpost_HomeBase` is another candidate with only ten direct asset
imports. Its recovered Windows cooked materials and meshes failed during
Linux deserialization. All four sampled mesh dependencies now export to GLB,
import at their original `/Game` paths, and cook for Linux; see
`mesh_conversion.md`. The Linux runtime directly loaded one converted mesh.
After all four converted meshes and two reconstructed materials were staged,
the map entered play in the headless Linux runtime and exited 0. The materials
are visual approximations, and no Fortnite gameplay is present. The recovered
config names `/Script/FortniteGame.FortNavSystem`; defining an initial native
subclass of UE's `UNavigationSystemV1` removed the navigation-system ensure.
The map now also enters play under Vulkan with corrected mesh imports and
current shader archives. See `mesh_conversion.md`.

## Apollo persistent map probe

`cue4inspect --scan-native` parsed all 3,694 map headers and listed native
class, struct, enum, and function imports per map in ignored JSONL. Across
those maps it found 465 distinct native imports, including 146 under
`/Script/FortniteGame`. `Apollo_Terrain` directly imports `FortPoiManager`,
`FortWorldManager`, and `WorldTileManager` in addition to the map classes
already present. The world manager's serialized default subobject is named
`MySceneComponent`, and the level contains a `WorldTileManager_0` subobject.

Initial reflected classes with these names and subobject shapes were added
to the native module. A headless load of the recovered `Apollo_Terrain` map
then reduced `Could not find class` errors from seven to four. The remaining
four refer to cooked Blueprint classes in unstaged packages:
`BP_DisplayShadowProxy_01_C`, `BP_SafeZoneStorm_C`,
`BP_SetupNamedPoiColors_C`, and `DefaultMapInfo_C`. The map enters play in
this partial state, but 108 package lookup errors remain, including streaming
landscape levels, virtual textures, materials, and gameplay data. It does not
yet represent a working Apollo island.

The four direct Blueprint packages were then recovered and staged. This
exposed their native parents and functions. Cooked function metadata confirms
that `BP_DisplayShadowProxy_01_C` overrides zero-parameter
`ShowShadowProxy` and `HideShadowProxy` events from
`FortSkydivingShadowProxy`; the other parents are `FortSafeZoneStorm` and
`FortAthenaMapInfo`. Initial native parent classes and those two event
declarations were added. The load now reaches further into the Blueprints but
still fails on further native classes and methods, including
`FortKismetLibrary`, `FortAsyncAction_AuthorityMatchReady`, and imported
default scene components. These declarations are load compatibility work,
not recovered Fortnite behavior.

The next probe identified two imported default scene components. Adding
`DefaultSceneRoot` to `FortSkydivingShadowProxy` and `RootComponent0` to
`FortAthenaMapInfo` removed those import warnings. The safe-zone Blueprint
also imports `FortKismetLibrary.IsMobilePlatform`; the new native function
returns the platform result (false for Linux), and its missing-class warning
disappeared. Loading now reaches deeper Blueprint data but still fails with a
UE linker assertion while `FortAsyncAction_AuthorityMatchReady` and other
game-state classes/functions are absent. A local UE patch can log unresolved
field paths if the related field-path assertion recurs; it does not suppress
either assertion. The full Blueprint and gameplay logic remain unimplemented.

`python3 porting/probe_apollo_blueprints.py` temporarily stages each of the
four direct Blueprint packages alone, then restores all staged files. Its
logs stay under ignored `porting/generated/apollo-blueprint-isolation/`.
Without these packages, the partial map enters play but reports four missing
classes. `DefaultMapInfo` alone also enters play. Each of the shadow, storm,
and POI Blueprints fails independently while its cooked script is loaded.
The shadow package exposes an unresolved `InstancedStaticMesh` field path in
its construction script. The other two reached an invalid export index while
deserializing `FFieldPath` from `UStruct::SerializeExpr`; with the original UE
4.25 code, both reported index 1024 despite different package export counts.

UE 4.26's `FieldPath.cpp` additionally checks the Fortnite branch custom
version before reading the serialized owner. The adjacent 4.26 source and
CUE4Parse agree on the version sequence. `prepare_ue_fieldpath.sh` now
backports that check and the two intervening version constants to the local
4.25 checkout. This changed the invalid index values but did not remove the
crashes; the shadow field path assertion also remains. The mismatch therefore
needs further cooked script and custom version inspection. The linker patch
is a diagnostic that prints the invalid index and export count; it does not
make an invalid index valid. Crash stacks were verified with the matching
`.sym` file for the staged binary.
