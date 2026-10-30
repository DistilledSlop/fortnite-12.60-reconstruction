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
  -factoryname=GLTFImportFactory -nosourcecontrol -unattended -nullrhi -nop4
```

The first headless import exited 0 with no errors and saved a 96,356-byte
static mesh package plus a small material package. The importer placed them
under `/Game/Environments/Homebase/Meshes/SM_HB_Rock_Cliff/`, naming the mesh
`0_SM_HB_Rock_Cliff`. This path does not match the original asset reference;
the packages still need relocation and validation in a Linux cook before the
recovered level can use them. Generated editor assets remain ignored by Git.

Both patches are committed here as patch files. They modify the adjacent
source checkouts only when the preparation scripts run; no supplied source or
game asset is committed to this repository.
