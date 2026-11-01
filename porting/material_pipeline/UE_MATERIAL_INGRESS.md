# Original material ingress

## Current frontend batch (2026-11-01)

Engine `c13bb7dd2` and native `8f1d95f` preserve the serialized owner's provenance
on the final material resource, including after normal identical-map reuse.
Actual AngledBox and SimpleFillColor owners pass seven required/zero missing
shaders, game/render attachment and numeric cases. The bounded adapter accepts
shorter shader declarations within the exact original numeric buffer; it retains
the complete source layout, including AngledBox's 736 numeric/752 total bytes.
Exact live Max/Saturate bit checks cover signed-zero ties for finite values.

The current immutable batch-11 manifest and work in progress are recorded in
[HANDOFF](../HANDOFF.md); [batch instructions](FRONTEND_NUMERIC_BATCH.md) explain
source versus resource owner, inheritance, proof inputs and validation. Current
source selection has 118 eligible owners; the runtime suite and matching server
build are not yet finished. All 65 owned frontend resources now decode structurally,
while texture/property runtime conversion remains incomplete. Both actual root
classes next fail on the original Paper2D TranslucentUnlitSpriteMaterial. Current
stock source has not been established as equivalent to that cooked reference.
All four product milestones, real widget construction and original pixels remain
unverified. The checkpoints below retain their historical scopes.

## Donut texture checkpoint (2026-11-01)

Native `46b4675` and Engine `08ebac023` now load the original Donut material
with its original indexed texture and sampler. The client build and Vulkan
probe pass. All 19 original records remain retained, the actual owner requires
seven shaders with zero missing, and the map attaches on both game and render
threads. Five graph and five owner uniform cases pass. Independent validation
passes all seven emitted shader payloads, including their original D3D resource
table links. The 128-byte Material layout has hash 92801271 and resource offsets
96/104/112/120 with types texture/sampler/sampler/sampler.

The source default is `T_UI_Donut_PC_Icon`; the real `M_UI_DonutIcons_App2`
instance overrides `Icon` with `T_UI_Donut_App_Icon`. Both original textures are
128×128 BC1, sRGB, eight mips. Their source metadata and CPU decodes are retained;
these are separate from GPU output. The actual override changes 1,290 pixels in
the controlled 64×64 quad, independently counted from the RGBA readbacks. Exact
command, client digest and 24 hashed evidence files are in
`generated/material_pipeline/donut-independent-checkpoint-results.json`.
The pinned v3 bundle is `donut-ingress-bundle.json`, SHA-1
`e794ad691e278ecf653dfef34c0ae07375f32d7d`.

The Engine restores source-indexed texture references on the real owner before
completeness validation. This does not reconstruct the full original cached
expression data. Its synchronous game-thread texture load during PostLoad emits
a possible-streaming-hitch warning; async dependency handling remains work.
The portable ingress patch includes Engine `08ebac023` and reverse-checks clean.
Matching server build and broad joins are tracked separately from this client
probe.

Actual RootLayout and LoginScreen still fail after the full hard-import closure
is staged. Gdb identifies the next declined original material instance as
`/Game/UI/Foundation/Border/Materials/M-Border-MainL.M-Border-MainL` immediately
before the incompatible current frozen shader-map reader asserts. Evidence:
`generated/frontend-probes/gdb-donut-root-owner-3.log`. Recover and convert its
actual resource next, then retry real widget loading and construction. All four
product gates and original-client pixel parity remain open. Earlier checkpoints
below retain their historical scope.

## MenuButton and final numeric checkpoint

Native `ebea44e` and Engine `6925baad3` pass final client4/server2 builds,
Locker/PowerToast/MenuButton probes and default/Athena joins. The complete
command/source/binary/artifact receipt is `generated/material_pipeline/numeric-menu-checkpoint-results.json`.
MenuButton's real owner requires seven shaders with zero missing, preserves all
19 original records and attaches on both threads. Eight numeric cases and four
View scenarios pass. All 21 emitted payloads across the three probes independently
validate.

The observed original GameTime field at 2264 matches compiled current metadata;
RealTime is separately at 2268. Real current View layout: 4032 bytes, 61 resources,
hash 130419238. Menu Material layout hash: 20447487. The original View struct
size is 3952; this proves only the observed field access, not full ABI equality.
The adapter uses the original typed D3D b1 View/b2 Material roles and the real UE
FSceneView uniform construction, with no shader instruction changes beyond the
entry-point name. Controlled GameTime 0/3.25 changes 1851 pixels; RealTime 10/20
alone leaves both corresponding images identical. A separate fixed-time Primary
Color red/green draw changes 4096 pixels. Current artifacts and root verifier
receipt are `generated/frontend-probes/legacy-numeric-menu-artifacts-4/` and
`legacy-numeric-menu-artifacts-4-root-validation.json`.

Use the immutable `menu-button-ingress-bundle-active-time.json`, SHA-1
`0b49a605763b907dbdf1f0041715208a81d4744a`. Earlier 0/1-second scenarios both
missed the inspected original shine window (threshold 3.0, period 4.5); their failed
runs are retained. Source-derived active phase 3.25 exercises it. Original-client
pixel comparison, full View layout compatibility, Donut textures and root-widget
integration remain unverified. All four product milestones remain open.

## Numeric PowerToast checkpoint

Engine `6925baad3` and native `25bf1c4` passed the non-editor Linux Vulkan checkpoint recorded in `generated/material_pipeline/numeric-power-checkpoint-results.json`, including exact commands, source and artifact hashes. The original GradientPanel base and PowerToastGlow material instance load through two pinned bundles. For each original resource all 19 shader records are retained, seven converted and zero required shaders missing; the map attaches on both game and render threads. Eight graph cases and eight real-owner uniform cases pass, and an attached quad changes 3,952 pixels. Five source instance parameter override identities were verified. Independent verification passes all seven PowerToast shader artifacts and all seven Locker artifacts. Client and server builds and fresh default and Athena joins pass. Six missing graph dependency packages were recovered verbatim; owner-package import warnings are absent, while preexisting Water startup warnings remain.

Pass multiple absolute bundle paths as one unquoted comma-separated argument, e.g. `-FortniteLegacyMaterialIngress=/absolute/gradient-panel-ingress-bundle.json,/absolute/power-toast-ingress-bundle.json`. Select the actual owner exactly with `-FortniteLegacyMaterialProbeOwner=/Game/UI/Foundation/Border/Materials/M-Border-PowerToastGlow.M-Border-PowerToastGlow` alongside `-FortniteProbeLegacyOriginalMaterial`. The material instance owner and parent shader identity are distinct; both source identities must be preserved. The existing single Locker bundle and pin remain supported.

Numeric layouts include PowerToast 208 bytes, MenuButton 304 bytes and Locker 336 bytes, including their sampler resources. MenuButton original View byte 2264 is now identified as GameTime by original metadata helpers and current declarations (`view_original_identity.md`); native offsets and the observed GameTime runtime binding are now verified. Donut texture conversion, widget integration, Editor rebuild behavior and original-client pixel comparison also remain open. All four product gates remain open.

The bounded, opt-in Locker ingress described below is implemented in Engine `e6e962a43` and native `c8ec113`; its actual-owner runtime probe passed in the staged Linux Vulkan client. The previously verified detached map at Engine `757e0087a` and native `afb32b7` evaluates live preshaders and draws an original Slate quad. The subsequent actual-owner probe establishes those properties for the tested Locker resource and current non-editor layout. All four product dashboard gates remain open.

The Linux client registers the pinned provider with `-FortniteLegacyMaterialIngress=<absolute path to porting/generated/material_pipeline/locker-ingress-bundle.json>`. Add `-FortniteProbeLegacyOriginalMaterial` to load the original Locker owner, check the attached game-thread and render-thread map and actual-owner `IsComplete`, exercise reader rejection cases, and draw its quad. These switches are diagnostic and require the matching staged original asset and Vulkan environment. The staged software Vulkan run exited 0 in 4.785 seconds; its command, environment and artifacts are in `porting/generated/frontend-probes/legacy-material-ingress-owner-3.json` and the corresponding `.log`. It reported domain 5 (UI), blend 2 (translucent), `MSM_Unlit`, quality 1, feature 3, packed flags 2816, seven required shaders, zero missing and complete=1. The actual game-thread and rendering-thread resources held the same map; its original Slate quad changed 2,832 pixels between red and green overrides. Four malformed-reader cases were rejected. The earlier owner-1 run reported missing SmoothStep; owner-2 staged that dependency. The owner-2 log still reports missing Water assets and a handled Vulkan vendor ensure.

The bundle SHA-1 is `0f750204f6008e6e3219d0b002d8409e28c22426`. It pins `/Game/UI/Foundation/Materials/Athena_LockerTabMaster.Athena_LockerTabMaster`, local names and the complete 12,513-byte original resource (SHA-1 `bc2d2f509b6909a4820f65188b85485fce989a2a`). In the `.uexp` framing the wrapper starts at 1037 and the selected resource is `[2797,15310)`; those offsets are not archive `Tell` coordinates. The single location has relative offset 0, feature level 3 and quality level 1. The 46 local names include seven Slate shader types and `FLocalVertexFactory` plus its original shader and uniform names; the bundle records the full ordered list. The source property report `material-locker.json` identifies the original as UI domain, unlit and translucent, consistent with this runtime report.

The reader retains original bytes, decoded records and library associations for all 19 original shaders: seven Slate and 12 LocalVF. The current factory converts only the seven Slate records. Preservation of the remaining records is not conversion; the runtime gate established that the tested current non-editor owner requires no additional shaders.

## Existing serialization and attachment path

Paths below are relative to the Engine checkout.

| Stage | Source |
| --- | --- |
| Original owner serialization | `Engine/Source/Runtime/Engine/Private/Materials/Material.cpp:3190`, `UMaterial::Serialize` |
| Inline resource framing and selection | `Material.cpp:662`, free `SerializeInlineShaderMaps` |
| Resource-local names and selected resource offset | `Engine/Source/Runtime/Engine/Private/Materials/MaterialShared.cpp:3958` and `:4006`, `FMaterialResourceProxyReader` constructor / `Initialize` |
| Cooked / valid flags and temporary shader map | `MaterialShared.cpp:920`, `FMaterial::SerializeInlineShaderMap` |
| Current shader-map deserializer | `Engine/Source/Runtime/Engine/Private/Materials/MaterialShader.cpp:2393`, `FMaterialShaderMap::Serialize` |
| Game-thread resource processing | `Material.cpp:3426`, `UMaterial::PostLoad`, then `Material.cpp:750`, `ProcessSerializedInlineShaderMaps` |
| Inline attachment | `Engine/Source/Runtime/Engine/Public/MaterialShared.h:1760`, `FMaterial::SetInlineShaderMap` |
| Cooked ID reuse, registration and completeness | `MaterialShared.cpp:1754`, `FMaterial::CacheShaders` |

`FMaterialShaderMap::Serialize` immediately reads the **current** `ShaderMapId` and then calls the current shader-map memory-image deserializer. It is too late to attempt recovery after feeding incompatible legacy records into that path.

## Bounded legacy provider

The implementation places an explicit opt-in provider call in `FMaterial::SerializeInlineShaderMap`, after reading the cooked and valid booleans and **before** allocating and invoking the current `LoadedShaderMap->Serialize`. The provider returns a tri-state result:

1. **Not applicable:** archive position and error state are unchanged; execute the existing reader.
2. **Converted:** return a validated, finalized `TRefCountPtr<FMaterialShaderMap>` plus retained original-data provenance; the member function can assign its own `GameThreadShaderMap` legitimately.
3. **Failed:** set the archive error and report the precise failure. Do not fall through to the current reader after consuming legacy bytes.

Match only explicitly enabled original data with validated package/resource identity, archive-version information and source digests. Broad version matching must not intercept the working cooked Engine overlay. Pass the original owner path from `UMaterial::Serialize` / the outer helper; the temporary resource is not yet bound to its owning material.

The resource proxy retains validated selected `[begin,end)` bounds derived from locations and `NumBytes` and passes those bounds and local names in an explicit context to `SerializeInlineShaderMap`, including the reload path. The provider checks the owner path, pinned bundle, ordered names, exact bytes and source digest before decoding. It does not infer the resource end from the remaining export.

The bounded original reader decodes the original representation: compilation output, recursive expressions, all shader records and library associations. It retains raw/decoded records and their provenance. Locker's original 12 mesh records must remain accounted for alongside its seven Slate records, even if the current runtime layout does not require their conversion. Successful decoding requires exact end consumption. Seeking past shader data, setting it invalid, or substituting a synthetic `UMaterial` is not this proposal.

The provider remains separate from Vulkan adaptation: Engine owns neutral IR and map construction; the registered native adapter owns validated Vulkan outputs. The validated factory supports the tested numeric Locker, PowerToast and MenuButton layouts; other resource layouts require their own conversion and runtime verification.

## Determine completeness from the actual owner

`FMaterialShaderMap::IsComplete` (`MaterialShader.cpp:1936`) is the canonical check. It derives `FMaterialShaderParameters` from the supplied resource, acquires the current layout, and checks required shader types, permutations, pipelines and vertex-factory maps with the resource's `ShouldCache` predicates. Its layout implementation is in `MaterialShader.cpp:1793` onward and its private data types are in `Private/Materials/MaterialShaderMapLayout.h`.

There is a concrete reason seven Slate shaders might suffice in a non-editor UI runtime: `FLocalVertexFactory::ShouldCompilePermutation` (`Engine/Source/Runtime/Engine/Private/LocalVertexFactory.cpp:189`) returns `!!WITH_EDITOR` when `MaterialDomain == MD_UI`. The current Slate VS and PS predicates require `MD_UI` (`Engine/Source/Runtime/SlateRHIRenderer/Private/SlateMaterialShader.cpp:28` and `:62`). The actual-owner runtime gate now establishes completeness for the tested Locker resource and current non-editor layout; other owners and configurations still require their own checks. Neither the owner domain nor the required set may be rewritten to force success.

At game-thread `ProcessSerializedInlineShaderMaps`, the implementation gates converted candidates **before** its initial `RegisterInlineShaderMap` loop and subsequent `SetInlineShaderMap` calls. A temporary deserialized `FMaterialResource` has no bound owner, so invoking its domain/material getters is unsafe. It allocates a validation resource from the actual owner and binds it with `FMaterialResource::SetMaterial` (`MaterialShared.h:2285`), supplying actual base material, quality usage, feature level and optional actual material instance. This is a CPU resource for the real asset, not a replacement material object.

The gate reports the real domain, blend/shading flags, packed material flags, quality and feature level. Existing public `GetDependentShaderAndVFTypes` (`MaterialShared.cpp:2783`, declaration `MaterialShared.h:1910`) provides a basic required-type/VF report. The Engine-owned diagnostic enumerates the exact private layout for every type/permutation/pipeline/VF and lists all missing records. It requires `Candidate->IsComplete(ValidationResource, false)` and no missing enumerated entries before allowing normal attachment. If any current mesh requirement exists, convert the needed original mesh/VF/View bindings; do not override `ShouldCache`, omit the missing records or assume completeness.

## IDs, output metadata and threading

Before attachment, retain the original map identity and quality/feature selection, active target platform, current type-layout parameters, validated live uniform layout and parameter tables, and correctly converted compilation-output flags. The factory fills runtime `CookedShaderMapIdHash`, quality, feature and current `LayoutParams`, and maps the decoded compilation flags, scene-texture mask, editor statistics and runtime virtual texture mask. These mappings still need runtime validation beyond the Locker probe.

`SetInlineShaderMap` sets `bContainsInlineShaders` and `bLoadedCookedShaderMapId` and queues the rendering-thread assignment. `FMaterial::GetShaderMapId` (`MaterialShared.cpp:515`) then returns the inline candidate ID. Later `CacheShaders` reuses/registers that ID and checks completeness. Preserve those normal paths after validation. The Editor `bIsCookedId` / editor dependency identity path remains unverified and must not be inferred from the successful non-editor diagnostic.

Serialization may execute on an async loading thread. Register the provider and preload required shader-type modules on the game thread before loading original assets. Provider publication is atomic; each invocation parses a private JSON tree from the pinned bundle string, and retained provenance uses plain data rather than a shared JSON tree. Deserialize/convert with bounded CPU work; do **not** copy the diagnostic callback's synchronous render-command flush into async loading. Defer owner-dependent completeness, material validation and attachment to game-thread `PostLoad`, preserving normal resource initialization and render-command ordering.

The original Locker owner passed its current non-editor requirements with all original records retained in the staged probe. Other material layouts, texture parameter/reference/sampler conversion and mesh/View uniform-buffer conversion remain separate unfinished work.


## Next View-buffer investigation

The MenuButton's five Slate PS records use library indices 6853, 6854, 6855, 6856 and 6854 (Border shares Default). Reflection of the four unique translated modules shows `View` as `cb1_1`, set 0 binding 1, with a 2,272-byte span; their only read is vec4 element 141 lane 2, byte offset 2,264, using constant indices. The four original DXBC modules contain ISGN, OSGN and SHEX but no RDEF. Subsequent original metadata helper evidence identifies this field as GameTime; see `view_original_identity.md`. Native offsets and the observed GameTime runtime binding are now verified; full View ABI parity is unproven.

## Final revision and reload coverage

Engine `6750d43f5` adds the provider and owner gate; `e6e962a43` binds both
real owners to temporary resources before the guarded reload call. Native
`c8ec113` supplies the reader, Vulkan adapter and actual-owner probe. Final
client/server builds, owner-3 and both native joins pass; receipts are under
`generated/frontend-probes/legacy-ingress-reload-joins-20261101T030500Z/`.
The three affected translation units also pass standalone syntax checks with
`STORE_ONLY_ACTIVE_SHADERMAPS=1`, using fresh copied header definitions rather
than a PCH built with zero. Runtime reload and alternate async reload remain
unverified; the tested Linux build uses zero. The portable two-commit patch is
`patches/ue425-legacy-material-asset-ingress.patch`.

MenuButton actual-owner ingress and its observed View binding are verified; next Donut needs original texture parameter/reference/sampler conversion. These require actual-owner loading and completeness checks before root-widget integration and original visual tests.
