# Original material ingress

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

The provider remains separate from Vulkan adaptation: Engine owns neutral IR and map construction; the registered native adapter owns validated Vulkan outputs. The current factory is Locker-only and rejects other slot layouts.

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

The MenuButton's five Slate PS records use library indices 6853, 6854, 6855, 6856 and 6854 (Border shares Default). Reflection of the four unique translated modules shows `View` as `cb1_1`, set 0 binding 1, with a 2,272-byte span; their only read is vec4 element 141 lane 2, byte offset 2,264, using constant indices. The four original DXBC modules contain ISGN, OSGN and SHEX but no RDEF. This establishes an original-buffer offset, not the semantic identity of the value or its mapping to a current UE View field; neither GameTime nor RealTime should be assumed without further evidence.

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

Next, extend numeric conversion to PowerToast (8 vector/15 scalar slots, 208-byte
layout), preserving the actual material-instance/parent shader identity. MenuButton
needs its 304-byte numeric layout plus a verified View mapping; Donut needs original
texture parameter/reference/sampler conversion. These require actual-owner loading
and completeness checks before root-widget integration and original visual tests.
