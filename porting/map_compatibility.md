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

The new package-version report shows that the shadow, storm, and POI Blueprint
headers all declare UE4 file version 518, licensee version 0, and **zero
serialized custom versions**. Their Fortnite branch and release versions
therefore both report `-1`. The UE 4.26 branch-version check cannot select a
new owner format from these headers. The invalid indexes are read during
`UStruct::SerializeExpr`, after `FFieldPath` script data, so a bytecode or
field-path layout mismatch remains possible.

## Linux cooked Blueprint replacements

The `FortniteApolloBlueprint` editor commandlet now creates four new editable
Blueprint packages at the original paths, inheriting the native parent
classes. The source packages are ignored under `porting/native/Content/`; the
supplied cooked packages are retained separately and were not overwritten.
UE 4.25 cooks the new packages for Linux when their directories are listed in
`DefaultGame.ini`. The original packaging section was in `DefaultEngine.ini`,
where this UE version did not read it; moving it made all four packages cook.

After staging those four Linux packages, Apollo entered play with zero
missing class errors and no Blueprint script assertion. The first replacement
set still had 38 unresolved component imports and four component dependency
errors. The commandlet now reconstructs the safe-zone Blueprint's named SCS
components, using the class and component names observed in the cooked
package. This reduced unresolved component imports to two and component
dependency errors to one. Both remaining imports point into the shadow
Blueprint; its original `InstancedStaticMeshComponent` instance buffer
triggered a Linux array element-size assertion when the template was present.
The native replacement leaves that component absent until its instance data
can be decoded and rebuilt for Linux. The map still has 104 missing package
lookups and 87 absent streaming levels, so entering play is only a loading
milestone, not an island or gameplay port.

The 104 unique missing package paths are present in the primary-key paks and
sum to about 181 MiB of indexed payload, although their dependency closure is
larger. A bounded probe recovered and staged
`Apollo_Terrain_LS_Parent` (8.8 MiB). Its map load exposed 29 distinct missing
classes, including water spline components, generators, and waterbody
Blueprints, and then hit an invalid cooked export index (158789 with 1208
exports). The staged copy was moved to ignored holding output; the extracted
copy remains available for reconstruction. The base Apollo map still enters
play after this probe. Directly staging every missing map would not yet give
a playable island.

`Apollo_Terrain_LS_A1` is a smaller landscape tile whose map header imports
only engine and already reconstructed Fortnite classes. Staging it still
failed: UE 4.25 lacks `/Script/Landscape.LandscapeLODStreamingProxy`, which is
present in the adjacent UE 4.26 source, and loading a Windows cooked inline
material shader map overran the Linux async archive. The three staged tile
files were moved into ignored holding output. A fresh load of the base Apollo
map succeeded again. The remaining landscape work requires a compatible
landscape class and Linux material reconstruction, not just package copying.

## Apollo landscape mesh reconstruction

`python3 porting/probe_apollo_streaming.py A1` temporarily stages the original
A1 tile, runs the Apollo map, and restores the held files even if the runtime
crashes. Its ignored log reproduces the two independent blockers: the tile
imports `/Script/Landscape.LandscapeLODStreamingProxy`, absent from UE 4.25,
and its inline Windows material shader map drives `FAsyncArchive` past the
end of the `.uexp`. The crash stack reaches `FShaderMapBase::Serialize` through
`UMaterialInstance::Serialize`. Copying the original cooked tile cannot fix
those platform and class mismatches.

`cue4inspect --landscape-report` reads the actor and component transforms from
the original package. `--export-landscape` decodes the heightmap and writes a
GLB under ignored `porting/generated/converted/`, without loading the Windows
shader map in UE. A1 yielded one 127×127 quad component and a 16,384 vertex
mesh. Its saved root transform is location `(113792, -130048, 100)` and scale
`(128, 128, 100)`. D4 yielded four components and a GLB with varying vertex
heights, so the conversion path is not limited to flat terrain.

The A1 GLB first imported as
`/Game/Athena/Apollo/Maps/Landscape/Converted/LandscapeStreamingProxy_A1`,
then cooked for Linux. A headless asset probe loaded it with 16,384 vertices.
The initial `AApolloLandscapeTileA1` actor placed it at the saved root
transform and replicated it to a joining client. This established the
single tile conversion path before expanding to the grid.

The inspector's `--export-landscape-grid` mode processes all 64 regular grid
positions with one hidden key prompt and writes an ignored transform manifest.
It exported 61 nonempty landscape meshes, about 434 MiB of GLB data. A7 and
A8 have no package in this build; B8 has a landscape actor with zero
components. All 61 mesh exports completed without a CUE4Parse error. These
are source geometry for reconstruction.

`python3 porting/prepare_apollo_grid_import.py` creates ignored Unreal import
settings for the meshes that have no editor source asset yet. The editor
`ImportAssets` commandlet imported all 61 GLBs under `/Game/.../Converted/`.
It was run with `-importsettings=porting/generated/import-apollo-grid.json`,
`-FortniteMatchAssetPath`, `-nosourcecontrol`, `-unattended`, and `-nullrhi`.
The Linux cook produced 61 static mesh packages and four generated materials.
`python3 porting/stage_apollo_grid_meshes.py` stages those packages, current
shader archives, and the ignored transform manifest. The native
`AApolloLandscapeTile` class reads each tile ID replicated by
`AFortGameModeAthena`, loads its Linux cooked mesh, and places it at the saved
transform. A headless standalone run logged 61/61 meshes, and a dedicated
server join logged all 61 loaded on both server and client. A software Vulkan
run created a swapchain, loaded all 61 meshes, advanced to frame 1, and
exited 0. Its visible appearance was not inspected.

The original streamed level packages are still absent, so Apollo logs lookup
errors for them. The reconstructed meshes have only approximate generated
materials, one mesh LOD, and unverified landscape collision and texture
layers. Gameplay actors, props, foliage, water, navigation, and Fortnite rules
are still missing. This grid milestone does not make the island playable.
