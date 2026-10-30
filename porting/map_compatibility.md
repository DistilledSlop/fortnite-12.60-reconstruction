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
Linux deserialization. A first static mesh conversion path now exports two of
its meshes to GLB and imports one as a UE 4.25 editor mesh; see
`mesh_conversion.md`. That editor mesh has not yet been cooked and does not
yet have the original asset path, so this map still does not enter play.
