# Fortnite 12.60 Linux reconstruction

This repository tracks new porting code and notes only. The supplied Windows
client and the separate Unreal Engine source checkout remain outside Git.
`../Fortnite/` is a read-only input to this work.

## Current evidence

- The client is `FortniteClient-12.60-CL-13477524`, a packaged Win64 build.
- The adjacent engine checkout is Epic's public UE 4.25.0 release at Git
  revision `f8f4b403eb682ffc055613c7caf9d2ba5df7f319`.
- All 31 game paks use pak version 10 and have encrypted indexes. Version 10
  is Unreal's path-hash index format. The public UE 4.25 source only defines
  pak versions through 9, so its `UnrealPak` cannot simply read these files.
- Some paks contain a nonzero encryption-key GUID. A GUID identifies a key;
  it is not the key itself. No keys are stored in this repository.
- The supplied primary key and three per-chunk keys successfully decrypted and
  validated all 31 pak indexes. Together they list 967,504 entries and 963,427
  unique paths, including 450,951 `.uasset` files and 3,694 `.umap` files.
  Keys are entered through hidden prompts and are not written to Git or
  generated files.
- The index includes project metadata, plugins, configuration, maps, and
  cooked assets. The static client scan found 59 API path templates; this is a
  partial view of the server interface.
- A staged Linux UE 4.25 runtime successfully loaded one recovered Windows
  cooked `SoundClass` after its referenced packages were supplied. It also
  loaded a 64×64 UI texture and reported its mip data. A UI Blueprint failed
  because Fortnite and CommonUI native classes were absent. These are focused
  compatibility probes, not a playable game. A software Vulkan run also
  initialized a swapchain and loaded the texture without drawing it.
- All 3,694 cooked map headers parsed in a CUE4Parse scan. A recovered
  `FortniteEmptyDedicated` map enters play in the staged Linux runtime after
  adding three initial Fortnite class definitions. A more complex obstacle
  map still fails on missing native classes and content dependencies.
- Four static meshes converted from the recovered skybox map load on Linux
  with two reconstructed materials. The map enters play under Vulkan. A new
  Linux dedicated server and client complete a local UE join into that map.
  This proves the native build and join path, not original Fortnite gameplay.
- Apollo's persistent map loads in a partial state without four direct
  Blueprint packages. Three of those packages independently crash during
  cooked script field-path loading. A UE 4.26 field-path version backport did
  not resolve the crashes; many map and gameplay dependencies are still absent.
- The embedded `FortniteGame.uproject` was extracted to ignored output. It
  declares six project modules and 123 enabled plugins. Sixty-three of those
  enabled plugins are absent from the public UE 4.25 engine tree, so the
  project still requires substantial game and plugin code reconstruction.
- The supplied client does not include the Fortnite project source, editor
  assets, or a Linux executable.
- An adjacent FModel/CUE4Parse checkout can serve as a reference for pak and
  asset parsing. The user's `/opt/fortnite/assetextract.md` is also local
  reference material; neither checkout nor the article belongs in this Git
  repository. The article's embedded-key pattern is an example, not a
  guaranteed layout for this client.

Run `python3 porting/inspect_install.py > porting/generated/inventory.json` to
regenerate the local inventory. The generated file is ignored by Git.

Run `python3 porting/inspect_client_routes.py > porting/generated/routes.json`
to inventory API path templates embedded in the client. This is only a static
route list; it does not determine the full network protocol or server behavior.

Run `python3 porting/probe_pak_key.py` to test a candidate AES-256 key. The
script prompts with hidden input and prints only validation results. Never put
keys in source files, shell arguments, Git, or generated reports.

Run `python3 porting/list_pak_assets.py > porting/generated/assets.json` to
list package filenames from the indexes unlocked by a key. The output is
generated metadata and stays outside Git. The script does not extract files.

Run `python3 porting/extract_small_file.py` to extract the embedded project
descriptor into ignored output. This handles small uncompressed or zlib files;
FModel/CUE4Parse is needed for the general case.

`dotnet run --project porting/cue4inspect -- <asset-path>` inspects one cooked
package through the adjacent CUE4Parse checkout. It prompts for the primary
key with hidden input and prints export types and names. The tool has parsed
both a sound class and a cooked UI Blueprint using `GAME_UE4_25_Plus` without
a mapping file. Use `--all` to list every export, or `--pseudo` to ask
CUE4Parse to decode Blueprint bytecode into pseudocode. Redirect pseudocode
to `porting/generated/`; it is extracted game data. The pseudocode is partial
and can contain unresolved expressions, so it is a reconstruction aid rather
than buildable source. Package compatibility still needs to be tested asset by
asset. The CUE4Parse checkout is a local dependency, not vendored.
In one sampled UI Blueprint, 5 of 9 function scripts decoded.
Add `--save-package` to write the selected package and its payload files into
ignored `porting/generated/extracted/`. This was verified on a UI Blueprint:
both its `.uasset` and `.uexp` were recovered. The output remains cooked data;
it is not an editor source asset, and runtime compatibility must be tested per
asset and dependency graph.
Use `--save-folder` with a package folder prefix to recover a subtree with one
key prompt. It recovered 292 packages from the sound-class folder in a local
test. All recovered files stay ignored by Git.
Use `--save-list` with a newline file of package paths to recover selected
packages with one key prompt. `python3 porting/stage_recovered.py <same-list>`
copies complete recovered packages to the ignored Linux stage, skipping any
package whose staged files already exist. This avoids mixing Windows cooked
payloads with converted Linux packages at the same path.
Use `--imports` on a package to list its native `/Script/` imports, including
classes and functions required by cooked Blueprints. Redirect large results
to ignored generated output.
Use `--scan-maps` to inventory import counts and script modules for every
mounted `.umap` package. `--save-closure --max-packages=250` recovers one
package plus reachable `/Game` imports, stopping at the package cap. It does
not synthesize missing native classes or replace engine content.
Use `--scan-native` to list class, struct, enum, and function imports for
every map as JSON lines. It found 146 distinct `/Script/FortniteGame` imports
across the 3,694 maps; the generated full census stays outside Git.
Add `--all-keys` when inspecting packages in the three GUID-keyed chunks;
the tool prompts for those keys without saving them. It mounted all 31 paks
and parsed a material package from `pakchunk1002` in a local test.
`dotnet run --project porting/cue4inspect -- --registry` parses the embedded
asset registry and reports asset classes and dependency-node counts. The main
registry reports 191,666 asset records, including 22,211 Blueprints, 2,379
animation Blueprints, 2,175 widget Blueprints, and 3,694 worlds.
Use `--function-report` on a cooked Blueprint to report its native superclass,
function overrides, flags, and serialized parameter types. The report is a
reconstruction aid; it does not include native C++ implementations.

`bash porting/prepare_ue_fieldpath.sh` backports UE 4.26's Fortnite branch
field-path owner version check into the adjacent UE 4.25 checkout and logs a
cooked field path before UE's unresolved-path assertion. The script is
idempotent. `bash porting/prepare_ue_linker.sh` optionally adds invalid export
index details to a linker assertion. These patches help diagnose Blueprint
compatibility; the three sampled Apollo gameplay Blueprints still crash on
load. The repository stores only patches; the supplied UE source remains
outside Git.

`python3 porting/probe_apollo_blueprints.py` tests the four staged Apollo
Blueprint dependencies separately and restores the stage after each run.
Its per-case logs stay ignored. The staged executable and `.sym` file must
match to get an accurate crash stack.

See `server_surface.md` for the observed backend dependencies and
`server/README.md` for the local HTTP probe.
See `asset_compatibility.md` for the first runtime package-loading probe.
See `map_compatibility.md` for the first recovered map launches.
See `mesh_conversion.md` for the first static mesh conversion and editor import.

## Work needed for the requested result

1. **Asset access:** obtain the needed keys and mappings, read every pak index,
   and inventory package dependencies. All indexes are now readable; mappings,
   package decoding, and dependency analysis remain. Backport the version 10
   path-hash reader from UE 4.26 or repack recovered packages for the runtime.
2. **Linux client:** reconstruct game classes and content in a new UE project;
   convert or rebuild platform-specific assets; build and cook for Linux.
   The empty native project now builds, cooks, and launches a staged Linux
   executable. It has no EAC integration.
3. **Local server:** identify the client protocols and state machine, then
   implement the minimum local services for login, lobby, matchmaking, and a
   game session. Expand service coverage as client behavior requires.
4. **Full coverage:** verify maps, modes, UI, audio, inventory, progression,
   and gameplay against the supplied 12.60 build. The result must be tested
   end to end; file presence alone does not establish feature parity.

The first Linux executable now loads recovered levels and converted meshes,
and joins a new local UE dedicated server. A recovered Fortnite character,
original gameplay rules, account services, and full island content have not
yet been reconstructed.

`porting/native/` is a minimal build harness with the `/Script/FortniteGame`
module name. It has no EAC integration and contains no supplied assets. It
must not be described as a playable port until it actually loads content and
implements gameplay. See its README for the build command.

UE 4.25's original `Setup.sh` manifest failed because Epic's old dependency
URL returned HTTP 403. The locally supplied replacement manifest let `Setup.sh`
finish. The Linux Development game and editor targets compiled. The editor
cooked the engine Entry map for `LinuxNoEditor`, and Unreal's staging tool
assembled a Linux distribution. In a network-isolated headless smoke test, the
staged executable loaded `/Engine/Maps/Entry` with `GameModeBase` and exited
with code 0. This proves the build and packaging path for the newly authored
module. It is not a playable Fortnite client and contains no supplied game
content, reconstructed Fortnite classes, or local server.

If you have Git access to `EpicGames/UnrealEngine`, run
`bash porting/fetch_ue425_manifest.sh` to fetch the updated `4.25` branch
dependency manifest into `/opt/fortnite/ue425-Commit.gitdeps.xml`. The script
uses Git only, validates the XML, and does not change either engine checkout.
The `4.26.0` source checkout at `/opt/fortnite/UnrealEngine4260` is already
available for studying the v10 pak index code.

## Format references

- UE 4.25 `FPakInfo`: `../../UnrealEngine/Engine/Source/Runtime/PakFile/Public/IPlatformFilePak.h`
- [UEViewer pak version definitions](https://github.com/gildor2/UEViewer/blob/master/Unreal/FileSystem/UnArchivePak.h)
