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
errors for them. The reconstructed meshes have one mesh LOD and baked visual
proxy materials described below. Gameplay actors,
props, foliage, water, navigation, and Fortnite rules are still missing. This
grid milestone does not make the island playable.

The inspector's `--actor-report` mode records exported actor classes and
static mesh component references with saved transforms. On the cooked
`Apollo_Background` streaming level it found 83 exports, including 27
`FortStaticMeshActor` instances and 32 static mesh components. Those
components refer to 11 distinct `/Game` mesh packages: cloud sheets,
backdrops, and auroras. The level also contains Blueprint actors and a
`MeshNetwork` component absent from the current Linux project. This report
is a placement inventory for reconstructing the background, not a Linux
load of that streamed level.

The background recovery now exports the 11 referenced `/Game` static mesh
packages as ignored GLBs. `python3 porting/prepare_apollo_background_import.py`
turns the actor report into an ignored UE import settings file and a placement
manifest. It retains 27 components whose mesh object name matches the package
name. Five components remain unresolved: two lighthouse beam cylinders stored
as imports in `S_BackDrop_4`, a built-in engine plane, a `MeshNetwork` plane,
and one empty reference. The 27 placements retain their saved component world
position, rotation, scale, and first per-component material override. Import
these meshes with `ImportAssets` and
`-FortniteMatchAssetPath -nosourcecontrol`, then cook LinuxNoEditor and run
`python3 porting/stage_apollo_background.py`. The native game mode spawns the
placements as replicated static mesh actors. Their original materials,
translucency, Blueprint behavior, and the rest of the streamed level still need
reconstruction.

The background GLB import also creates dependent material and per-mesh
packages. The staging script copies the full cooked `Converted` tree, not just
the 11 top-level mesh packages. A Linux Vulkan capture confirmed the placed
cloud geometry, while showing its placeholder material as solid white. The
native map now spawns a replicated UE sky atmosphere and marks its directional
light as the atmosphere sun; a second Vulkan capture showed a blue sky gradient
instead of the previous black background. This atmosphere is a rendering
baseline, not a recovered Fortnite sky or cloud shader. The original cloud
override `M_Volcano_Cloud_Cove` is an instance of a 111-expression translucent
unlit material, so the cloud appearance still needs a dedicated reconstruction.

The inspector now records each material's expression export types, retained
property values, expression references, and root input names. For
`M_Volcano_Cloud`, the cooked package declares 111 expression references, but
65 are null and only 46 expression objects remain. Its `EmissiveColor` input
names `MaterialExpressionMultiply_4`, which is absent from those exports.
This is incomplete source graph data, even though some parameters, function
calls, and a cloud detail texture remain readable. A reversible editor probe
of the original extracted package first failed because it is unversioned.
Allowing unversioned and cooked content through the editor gates reached the
Windows material shader map, then failed at `MaterialShader.cpp:405`
(`IsContentValid`). The temporary editor settings and package copy were
removed. The original package cannot currently be recooked directly on
Linux; recovering its appearance requires reconstructing missing graph
behavior or translating the cooked shader data.

## Rebuilt cloud and backdrop materials

The 65 null expression references in `M_Volcano_Cloud` are not corruption.
The 46 surviving expression exports are exactly 21 scalar parameters, 13
vector parameters, 7 material function calls, 2 collection parameters, 2
static switches, and 1 texture sample. Every math node (the missing
`Multiply_4` and the rest) is gone, and the base `UMaterialExpression` reports
`IsEditorOnly() == true` in both the 4.25 and 4.26 sources. That is consistent
with the cooker stripping nodes the runtime does not need; the arithmetic then
lives only in the Windows shader map. I did not locate per-class overrides, so
treat this as a strong match rather than a proof. It also means the original
cannot be recooked, whatever engine version reads it.

What the cooked data does keep was enough for a simplified rebuild. The
commandlet `FortniteBackgroundMaterials` creates an unlit translucent
`M_Volcano_Cloud` at its original path, using the recovered parameter names
and defaults (`Tile_N_X/Y`, `FlattenDetailNormal`, `SoftenShadow`,
`LightColorDay`, `ShadowColorDay`, `AngleFadeSensitivity` and `...U`, `Opacity`,
`OpacityUnder`, `CameraDistanceFade`) and the decoded `T_StormCloudMobile`
(64x64; its RG channels are a detail normal and its alpha is noise). The shade
is a sun-wrapped lerp from shadow to light color, and the opacity is a
view-angle fade with near-camera fade and the "Under" values when seen from
below. The shading and opacity formulas are my reconstruction. The original
function calls (`Color_Time_of_Day`, `FadeBasedOnViewAngle`,
`Near_Camera_Fade`, `FlattenNormal`) are not extracted, the time-of-day
variants other than Day are unused, and the vertex offset animation
(`WorldPositionOffset`, `Speed`, `Offset`) is omitted. The `M_Volcano_Cloud_Cove`
instance is recreated with its parent link: of its 9 recovered overrides,
7 apply; `RefractionDepthBias` and `Offset` have no counterpart.

`M_BackDrop_P` is an opaque unlit material whose report predates the
expression dump, so only its texture and function names are known. A simplified
parent samples the already recovered `T_Apollo_Cliff_D_01`, mixes
Grass Low, Grass and Grass2 by height and patch noise, blends to cliff by
slope, and applies wrapped sun shading. The grass colors, thresholds and
contrast values from the `M_BackDrop_1`, `M_BackDrop_3` and `M_BackDrop_4`
instances are applied (22 parameters applied across the 7 instances in this
batch; 8 had no counterpart: 4x `RefractionDepthBias`, `Rocks 1 Scale`,
`Rocks2 Scale`, `Contrast rocks`, `Offset`). Parent defaults the cook did not
report (grass tone height, waterline height and blur) are my assumptions. The
`BackDrop3_N` and `BackDrop4_N` normal textures are not extracted and unused.
`M_BackDrop_2`, `M_BackDrop_6`, `M_BackDrop_7`, `M_SnowTops`, the `1B` and `4B`
overrides and the aurora materials still use importer placeholders: their
packages are not on disk and extracting them needs the hidden-key prompt, for
example `--material-report` on each path.

At run time `AApolloBackgroundMesh` replicates the component's original
override package and, per mesh slot, looks for a rebuilt material at the
original path before falling back to the placeholder. A software-Vulkan join
(`capture_apollo_visual.sh`) logged 27/27 placements on server and client, with
rebuilt materials on all 16 cloud actors and on 9 backdrop actors
(`S_BackDrop_1`, `_3`, `_4`). In a fixed capture, the cloud region's share of
pixels darker than 80/255 fell from 22.7% to 0.0% and its mean brightness rose
from 165 to 211: the dark flat-lit slabs became soft white clouds
(`apollo-cloud-before.png`, `apollo-cloud-after.png`). The backdrops were
invisible behind foreground hills and, from 2 million units away, washed out
by default aerial perspective. The sky atmosphere's aerial perspective scale
is now 0.1, and the capture's `-FortniteProbeLook=yaw,pitch[,lift]` raises
the local camera. From 800 m up, the previously white `S_BackDrop_4` island
renders green with grey rock patches. `-FortniteNoRebuiltBackground` restores
the placeholders for comparison and `-FortniteHideBackground=<MeshId>` hides
one mesh type. The captures stay under ignored `generated/`. This is a
visual approximation, not a match to the original shaders.

## Terrain collision probe

The imported meshes initially had `CTF_UseSimpleAsComplex` with no box,
sphere, or convex collision shapes. A Linux asset probe confirmed that D4
therefore had no usable ground collision. The editor commandlet
`-run=FortniteTerrainCollision -Tile=D4` changed its body setup to
`CTF_UseComplexAsSimple`, rebuilt one PhysX triangle mesh, and saved the
ignored editor asset. After a Linux recook and restage, the staged runtime
reported the new trace flag. A line trace through the center of D4, from
`(16000, -16000, 10000)` to `(16000, -16000, -10000)`, hit the terrain actor
at Z approximately `-1001`.

The commandlet's `-All` mode built and saved one PhysX triangle collision mesh
for each of the 61 imported tiles. The Linux recook and stage include these
updated packages. The game mode now places joining pawns above the D4 ground
hit, and the pawn has a first person camera. In a dedicated server and client
join, all 61 tiles loaded on both peers and the server's five second probe
reported `grounded=1`, `mode=Walking`, and a stationary pawn at
`(16000, -16000, -907.845)`. That verifies the D4 spawn and standing path;
movement across tile boundaries and collision on other tiles still need tests.
An Xvfb screenshot before adding a light was entirely black. A temporary
replicated directional light now illuminates the reconstructed terrain. A
software Vulkan capture shows pale terrain relief from the D4 pawn camera.
The same view was captured after a visible Linux client joined the local
dedicated server; both peers logged the light and D4 mesh, and the server
logged a successful join. The captures stay under ignored `porting/generated/`.
The black sky and mostly flat generated materials expose the missing sky and
landscape texture layers; this is not the original island appearance.

## Landscape layer data

The inspector can read already extracted packages with `--loose`, which scans
the ignored `porting/generated/extracted/` tree and requires no AES prompt.
For example, `--export-png --loose` decoded the original
`T_Apollo_Moss_D_01` texture from its extracted package. The
`--landscape-report --loose` result for A1 identifies `M_CommonLandscape`,
one component, and Sand, Gravel, and Mud layer channels. Its decoded 128×128
weightmap paints the entire component with Sand.

`--export-landscape-weightmaps` writes each component's decoded PNG weightmap
and a `layers.json` mapping channel numbers to original layer names. Its
`--export-landscape-weightmap-grid` form reads the regular A1–H8 tiles with
one hidden primary-key prompt. It exported 61 tiles, 221 components, and 311
weightmap textures with zero failures. A7 and A8 are absent, and B8 has no
components. Across the grid, the masks name nine layers: Gravel, Mud, Sand,
Asphalt, BiomeA, BiomeB, BiomeC, BiomeD, and DataLayer. D4's four components
contain varying Gravel, Mud, Asphalt, and biome weights.

The `M_CommonLandscape` material imports separate common-layer functions.
Its Sand function samples `T_Apollo_Sand_D_01`; Gravel samples both
`T_Apollo_Dirt_D_01` and `T_Apollo_Gravel_D_01`. Those original diffuse
textures were decoded to ignored PNGs. `--material-report` now includes each
material function's texture-sample references so the remaining layer graphs
can be traced.

`python3 porting/bake_apollo_tile.py D4` combines the decoded D4 masks with
the recovered Grass, Gravel, Dirt, Forest Floor, Cliff, and Asphalt diffuse
images. Its output atlas is ignored generated data. The original material
functions use more complex blends, tinting, normals, and masks, so this bake
is a visual proxy. The imported `Terrain_D4` texture and an opaque Linux
material sample the converted mesh's second UV channel. The editor commandlet
assigned that material to the D4 mesh, and the Linux cook produced its mesh,
material, texture, and shader data. A software Vulkan capture showed textured
green and brown D4 terrain beside white neighboring tiles.

The same bake then generated atlases for all 61 tiles. The editor imported 60
more textures and `FortniteMaterialRebuild -AllApolloTiles` created 60 more
materials, reusing D4's existing material. The Linux cook produced 61 terrain
texture packages and 61 tile materials; the stage now includes them and the
updated shader archives. The local join still loaded all 61 meshes and a
grounded pawn. A later Vulkan capture shows the nearby terrain colored across
the D4 tile boundary. These atlases remain approximations of the original
material graph, and most of the grid has not been visually inspected.
