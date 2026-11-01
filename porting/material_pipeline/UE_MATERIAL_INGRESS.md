# Original material ingress: proposed next patch

This is a source-checked plan, not an implemented asset-loading bridge. It was checked against `/opt/fortnite/UnrealEngine` at `757e0087a` and the native diagnostic at `afb32b7`. The detached Locker map, live preshaders, registered constructors and original Slate quad readback work. Original `UMaterial` shader-map deserialization, owner attachment and completeness have **not** been validated by that diagnostic.

## Existing serialization and attachment path

Paths below are relative to the Engine checkout.

| Stage | Source |
| --- | --- |
| Original owner serialization | `Engine/Source/Runtime/Engine/Private/Materials/Material.cpp:3177`, `UMaterial::Serialize` |
| Inline resource framing and selection | `Material.cpp:661`, free `SerializeInlineShaderMaps` |
| Resource-local names and selected resource offset | `Engine/Source/Runtime/Engine/Private/Materials/MaterialShared.cpp:3904` and `:3950`, `FMaterialResourceProxyReader` constructor / `Initialize` |
| Cooked / valid flags and temporary shader map | `MaterialShared.cpp:905`, `FMaterial::SerializeInlineShaderMap` |
| Current shader-map deserializer | `Engine/Source/Runtime/Engine/Private/Materials/MaterialShader.cpp:2393`, `FMaterialShaderMap::Serialize` |
| Game-thread resource processing | `Material.cpp:3410`, `UMaterial::PostLoad`, then `Material.cpp:740`, `ProcessSerializedInlineShaderMaps` |
| Inline attachment | `Engine/Source/Runtime/Engine/Public/MaterialShared.h:1756`, `FMaterial::SetInlineShaderMap` |
| Cooked ID reuse, registration and completeness | `MaterialShared.cpp:1712`, `FMaterial::CacheShaders` |

`FMaterialShaderMap::Serialize` immediately reads the **current** `ShaderMapId` and then calls the current shader-map memory-image deserializer. It is too late to attempt recovery after feeding incompatible legacy records into that path.

## Proposed bounded legacy provider

Add an explicit opt-in provider call in `FMaterial::SerializeInlineShaderMap`, after reading the cooked and valid booleans and **before** allocating and invoking the current `LoadedShaderMap->Serialize`. The provider returns a tri-state result:

1. **Not applicable:** archive position and error state are unchanged; execute the existing reader.
2. **Converted:** return a validated, finalized `TRefCountPtr<FMaterialShaderMap>` plus retained original-data provenance; the member function can assign its own `GameThreadShaderMap` legitimately.
3. **Failed:** set the archive error and report the precise failure. Do not fall through to the current reader after consuming legacy bytes.

Match only explicitly enabled original data with validated package/resource identity, archive-version information and source digests. Broad version matching must not intercept the working cooked Engine overlay. Pass the original owner path from `UMaterial::Serialize` / the outer helper; the temporary resource is not yet bound to its owning material.

The outer resource reader reads local names, resource locations and a payload byte count. Its current implementation discards local `Locs` after initialization and does not expose a public checked selected-resource range. The next patch should retain validated selected `[begin,end)` bounds derived from those locations and `NumBytes`, then pass an explicit read context into `SerializeInlineShaderMap`. Cover `ReloadMaterialResource` (`MaterialShared.cpp:194`) as well. Do not cast an arbitrary `FArchive` to the private proxy or guess bounds from the remaining export size.

The bounded original reader must decode the complete original representation: compilation output, recursive expressions, all shader records and library associations. Preserve immutable raw/decoded records and their digests. Locker's original 12 mesh records must remain accounted for alongside its seven Slate records, even if the current runtime layout does not require their conversion. Validate exact end consumption after successful decoding. Seeking past shader data, setting it invalid, or substituting a synthetic `UMaterial` is not this proposal.

The provider remains separate from Vulkan adaptation: Engine owns neutral IR and map construction; the registered native adapter owns validated Vulkan outputs. The current factory is Locker-only and rejects other slot layouts.

## Determine completeness from the actual owner

`FMaterialShaderMap::IsComplete` (`MaterialShader.cpp:1936`) is the canonical check. It derives `FMaterialShaderParameters` from the supplied resource, acquires the current layout, and checks required shader types, permutations, pipelines and vertex-factory maps with the resource's `ShouldCache` predicates. Its layout implementation is in `MaterialShader.cpp:1793` onward and its private data types are in `Private/Materials/MaterialShaderMapLayout.h`.

There is a concrete reason seven Slate shaders might suffice in a non-editor UI runtime: `FLocalVertexFactory::ShouldCompilePermutation` (`Engine/Source/Runtime/Engine/Private/LocalVertexFactory.cpp:189`) returns `!!WITH_EDITOR` when `MaterialDomain == MD_UI`. The current Slate VS and PS predicates require `MD_UI` (`Engine/Source/Runtime/SlateRHIRenderer/Private/SlateMaterialShader.cpp:28` and `:62`). **This is not proof of completeness for the original owner.** Other factories, material flags, pipelines and permutations still require checking; neither the owner domain nor the required set may be rewritten to force success.

At game-thread `ProcessSerializedInlineShaderMaps`, gate converted candidates **before** its initial `RegisterInlineShaderMap` loop and subsequent `SetInlineShaderMap` calls. A temporary deserialized `FMaterialResource` has no bound owner, so invoking its domain/material getters is unsafe. Allocate a validation resource from the actual owner and bind it with `FMaterialResource::SetMaterial` (`MaterialShared.h:2285`), supplying actual base material, quality usage, feature level and optional actual material instance. This is a CPU resource for the real asset, not a replacement material object.

Report the real domain, blend/shading/usage flags, quality and feature level. Existing public `GetDependentShaderAndVFTypes` (`MaterialShared.cpp:2783`, declaration `MaterialShared.h:1910`) provides a basic required-type/VF report. An Engine-owned diagnostic can enumerate the exact private layout for every type/permutation/pipeline/VF and list all missing records. Require `Candidate->IsComplete(ValidationResource, false)` before allowing normal attachment. If any current mesh requirement exists, convert the needed original mesh/VF/View bindings; do not override `ShouldCache`, omit the missing records or assume completeness.

## IDs, output metadata and threading

Before attachment, retain the original map identity and quality/feature selection, active target platform, current type-layout parameters, validated live uniform layout and parameter tables, and correctly converted compilation-output flags. The present factory fills runtime `CookedShaderMapIdHash`, quality, feature and current `LayoutParams`; it only fills the uniform-expression portion of compilation output. Normal ingress must also map and validate the original relevant scene-texture, world-position-offset and other compilation flags rather than silently accept their current default values.

`SetInlineShaderMap` sets `bContainsInlineShaders` and `bLoadedCookedShaderMapId` and queues the rendering-thread assignment. `FMaterial::GetShaderMapId` (`MaterialShared.cpp:515`) then returns the inline candidate ID. Later `CacheShaders` reuses/registers that ID and checks completeness. Preserve those normal paths after validation. The Editor `bIsCookedId` / editor dependency identity path remains unverified and must not be inferred from the successful non-editor diagnostic.

Serialization may execute on an async loading thread. Register the provider and preload required shader-type modules on the game thread before loading original assets. Deserialize/convert with bounded CPU work; do **not** copy the diagnostic callback's synchronous render-command flush into async loading. Defer owner-dependent completeness, material validation and attachment to game-thread `PostLoad`, preserving normal resource initialization and render-command ordering.

This next patch must first prove one original Locker owner loads and passes its actual current requirements without losing original records. Other material layouts, texture parameter/reference/sampler conversion and mesh/View uniform-buffer conversion remain separate unfinished work.
