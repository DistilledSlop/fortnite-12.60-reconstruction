# Fortnite 12.60 static mesh conversion

Directly loading recovered Windows cooked static meshes in the UE 4.25 Linux
runtime fails during render-data deserialization. The sampled
`S_Fortnite_Hexmaps_Tree_cine` load sought far beyond its `.uexp` size.

The local CUE4Parse checkout also initially failed to export a sampled mesh.
Its object deserializer swallowed an invalid-bool error, leaving `RenderData`
null. Enabling fatal object errors identified the first mismatch. Inspection
of `SM_HB_Rock_Cliff`'s three LODs showed this build does not serialize the
`bForceOpaque` section flag despite the package's rendering custom version.
Its index buffers also omit `bShouldExpandTo32Bit`, and its mesh buffers omit
the ray-tracing geometry array. Those two behaviors are selected with
CUE4Parse version options in `cue4inspect`; the section flag needs the
one-line local patch in `patches/cue4parse-fortnite-12.60-static-mesh.patch`.

Apply the patch to the adjacent FModel/CUE4Parse checkout, then build the
inspector:

```sh
bash porting/prepare_cue4parse.sh
dotnet build porting/cue4inspect/Cue4Inspect.csproj -c Release
dotnet run --no-build -c Release --project porting/cue4inspect -- \
  FortniteGame/Content/Environments/Homebase/Meshes/SM_HB_Rock_Cliff --export-glb
```

The key is entered through the hidden prompt. `--export-glb` writes only to
ignored `porting/generated/converted/`. It exported this 3-LOD rock mesh to a
120,924-byte GLB and `S_Fortnite_Hexmaps_Tree_cine` to a 37,184-byte GLB.
These are geometry conversion probes; material textures and shader behavior
have not been reconstructed.

UE 4.25 includes a glTF importer, but its stock factory does not support
commandlet import because it always opens an options window and its
`FactoryCanImport` returns false. `prepare_ue_gltf.sh` applies a local patch
that accepts existing `.glb`/`.gltf` files and skips the window in commandlets.
The new project enables `GLTFImporter` for the editor. Rebuild the editor
target, then import a converted mesh with absolute paths:

```sh
bash porting/prepare_ue_gltf.sh
/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FortniteLinuxEditor Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
/opt/fortnite/UnrealEngine/Engine/Binaries/Linux/UE4Editor-Cmd \
  /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -run=ImportAssets \
  -source=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/converted/FortniteGame/Content/Environments/Homebase/Meshes/SM_HB_Rock_Cliff.glb \
  -dest=/Game/Environments/Homebase/Meshes \
  -factoryname=GLTFImportFactory -FortniteMatchAssetPath \
  -nosourcecontrol -unattended -nullrhi -nop4
```

The first headless import, without `-FortniteMatchAssetPath`, exited 0 with no
errors and saved a 96,356-byte static mesh package plus a small material
package. The importer placed them
under `/Game/Environments/Homebase/Meshes/SM_HB_Rock_Cliff/`, naming the mesh
`0_SM_HB_Rock_Cliff`. This path does not match the original asset reference;
the `-FortniteMatchAssetPath` option in the patch now makes a single imported
mesh use the GLB basename and destination folder instead. That saved
`/Game/Environments/Homebase/Meshes/SM_HB_Rock_Cliff` at the original path.
The option also sets `CTF_UseSimpleAsComplex` before the editor builds the
mesh. This avoids a PhysX RTree recursion crash seen while importing the
larger center-command mesh, but drops its original collision behavior.

The matched rock mesh cooked for `LinuxNoEditor`. After its cooked files
replaced the sampled Windows package in the ignored staged runtime, the
`-FortniteProbeAsset` probe loaded it as `StaticMesh`, reported one LOD with
920 vertices, and exited 0. The source package had three LODs; CUE4Parse
exports only the highest LOD in the current GLB path, and UE's glTF importer
builds a single LOD. This is a geometry milestone, not full mesh parity.

The first Vulkan map probe exposed invalid bounds on
`SM_HB_Center_Command`. The GLB position accessor itself contained only
finite coordinates, but its vertex data is interleaved with a 108-byte
stride. UE 4.25's glTF importer copied float vector arrays as if each entry
were tightly packed, ignoring `byteStride`. The updated local importer patch
copies each strided vector and handles strided normalized attributes. All
four meshes were reimported and recooked. The resulting Vulkan run has no
invalid mesh bounds. After each cook, the staged shader archives also need to
be copied from `Saved/Cooked/LinuxNoEditor/FortniteLinux/Content/`; otherwise
the staged runtime reports missing shader hashes for the newly cooked assets.

All four static meshes in the sampled `Skybox_Outpost_HomeBase` dependency
set exported to GLB, imported under their original `/Game` paths, and cooked
for Linux. The next level load advanced past the earlier mesh failure and
stopped in Windows cooked material `M_Melee_Flare`, whose `.uexp` triggered a
seek past EOF. Material conversion and reconstruction are still required.
Generated editor assets, cooked assets, and runtime logs remain ignored by Git.

Both patches are committed here as patch files. They modify the adjacent
source checkouts only when the preparation scripts run; no supplied source or
game asset is committed to this repository.

The inspector also supports `--material-report` and `--export-png`. A report
for `M_Melee_Flare` found an unlit translucent material that imports
`T_Starburst`; the latter exported as a 512×512 PNG using managed ImageSharp
encoding. The stock UE texture importer accepted that PNG at its original
`/Game` path. These steps recover source pixels and basic material metadata;
the material graph is the next reconstruction step.

## First Linux material replacements

The editor-only `FortniteMaterialRebuild` commandlet creates simple UE
materials at original `/Game` package paths. It accepts `-Material`,
`-Blend=Translucent|Additive`, and an optional `-Texture` object path. With a
texture it connects RGB to emissive and, for translucent materials, alpha to
opacity. Without one it uses white emissive. Both forms are **approximations**;
they do not reproduce Fortnite's material expression graph or timing.

`M_Melee_Flare` is reported as unlit/translucent and imports `T_Starburst`.
Its 512×512 PNG was imported with UE's stock texture factory. The commandlet
saved `M_Melee_Flare` at its original path, and the Linux cook produced both
texture and material packages. Replacing those Windows cooked packages in the
ignored stage moved the map failure to `M_Slowfield_Flare_01`.

That second material is reported as unlit/additive with no imported texture.
The commandlet saved an emissive approximation and cooked it. With both
replacements staged, `/Game/Maps/Skybox/Skybox_Outpost_HomeBase` reached
`Bringing ... up for play` in the headless Linux runtime and exited 0 after
`-ExecCmds=Quit`. A navigation-system ensure was handled during load, and the
map has no suitable player start. This validates deserialization and level
startup, not visual parity or Fortnite gameplay. With corrected meshes and
current shader archives, the map also enters play under Vulkan on Mesa
Lavapipe via Xvfb, creates a swapchain, and exits 0. The runtime diagnostic
`-FortniteProbeMapMeshes` logs actors whose static-mesh bounds contain NaNs.
