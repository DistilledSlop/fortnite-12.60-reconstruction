# Original Slate shaders into the local UE 4.25 runtime

The detached Locker bridge is implemented and exercised in the non-editor Linux client. Engine `757e0087a` adds neutral material IR/API, live current preshaders, registered Slate shader construction and checked known-output-hash finalization through the normal freeze/resource initialization path. The portable patch is `patches/ue425-legacy-slate-material-bridge.patch` (main repo `4a95007`); its reverse check was tested. Preserve the five pre-existing dirty Engine files. Native `2144eba` supplies the opt-in fixture caller. The sections below retain the construction rationale and describe the implemented path; the detached construction details below are historical; later actual-owner results are summarized in [UE_MATERIAL_INGRESS.md](UE_MATERIAL_INGRESS.md).

The actual Vulkan run `generated/frontend-probes/legacy-material-runtime-1.log` exited 0 under Xvfb: eight live `FillUniformBuffer` cases passed, the detached map froze, and seven RHI shaders were constructed. An independent verifier checked the 14 emitted files and all seven SPIR-V/payloads (`generated/frontend-probes/legacy-material-artifacts-1-validation.json`): instructions differ only in the entry-point name; header offsets match actual descriptors; layout hash 22544639 is computed from the original shape; SourceHash, CRC and embedded code match. Three verifier tests pass. That first gate did not test a draw or attachment. A later Vulkan SM5 run used original VSfalse/PSDefault shaders and drew a 64×64 RGBA8 Locker quad: eight pre-freeze and eight post-freeze live uniform cases passed; two `RBGA1` overrides changed 2,832 of 4,096 pixels, with red and green center pixels respectively (`generated/frontend-probes/legacy-material-runtime-draw-1.log`, `generated/frontend-probes/legacy-material-artifacts-2/LockerQuad.json` and RGBA files). It exited 0. The later actual-owner probes test normal attachment; original visual comparison remains unverified.

## Detached Locker scope

Build an opt-in, detached `FMaterialShaderMap` containing the **seven original Locker Slate records**. Locker has no material texture expressions; it still exercises live parameter lookup, all supported recursive numeric expression forms, both Slate VS variants, and the five Slate PS types. Preserve its 12 vector/30 scalar output slots: 320 numeric bytes, 336-byte complete `Material` layout, sampler resources at offsets 320/328. Do not attach this detached seven-shader map to a material or claim `IsComplete(Material)` until the current platform's required shader layout has been checked. The original 12 mesh records remain required whenever that layout requests them.

Use two small Engine/RenderCore interfaces plus a native diagnostic caller. Engine owns material IR conversion and map construction; the caller prepares Vulkan compiler outputs. Engine does not need to depend on VulkanRHI or include private Slate shader classes.

## Construct the real current shader classes

The required interfaces already exist:

```cpp
FShaderType* Type = FindShaderTypeByName(FHashedName(*Record.TypeName));
FMaterialShaderType::CompiledShaderInitializerType Init(
    Type, Record.PermutationId, VulkanOutput, UniformSet,
    OriginalMaterialMapHash, nullptr, nullptr, DebugDescription);
FShader* Shader = Type->ConstructCompiled(Init);
Content->AddShader(Type->GetHashedName(), Record.PermutationId, Shader);
```

This illustrates construction inside the factory, with validated IR/outputs as inputs. Check `GetMaterialShaderType()`, expected frequency, permutation 0, null pipeline/VF, and exact type-name allowlist before construction. Ensure SlateRHIRenderer is loaded so registration exists. `ConstructCompiled` is available outside editor builds; it dispatches the registered constructor without exposing `SlateMaterialShader.h` across module boundaries.

Actual local constructors bind these names:

| Class | Named allocations to recover |
| --- | --- |
| `FSlateMaterialShaderVS` | `ViewProjection`, `SwitchVerticalAxisMultiplier` |
| `FSlateMaterialShaderPS` | `GammaAndAlphaValues`, `ShaderParams`, `DrawFlags`, `ElementTextureSampler`, `ElementTexture` |
| `FMaterialShader` | `Material`, `MaterialCollectionN`, scene texture binding |

The local PS already has **three** scalar parameters, matching the recovered original serializer. The old public 4.23 PS source with two parameters is not the construction reference. Omit allocations for original unbound parameters. The originals' Slate scene-texture, collection and VT-feedback bindings are unbound; do not add new bindings for them. `FShader` builds its own current `ParameterMapInfo` and automatically bound metadata from the supplied new parameter map. Do not copy the original serialized parameter-map arrays or old object memory into current instances.

Source: `Runtime/SlateRHIRenderer/Private/SlateMaterialShader.cpp` (constructors and registration); `Runtime/Renderer/Private/ShaderBaseClasses.cpp:68`; `Runtime/Engine/Public/MaterialShaderType.h:61`; `Runtime/RenderCore/Private/Shader.cpp:353,500`.

## Convert the graphs into live preshaders

The converter is inside an `FMaterialShaderMap` member/static factory. That class is already a friend of `FUniformExpressionSet` and `FMaterialShaderMapContent`, giving legitimate access to their protected/private arrays and compilation output. An ordinary game-module helper cannot write those fields. The neutral Engine API has this shape (consult the header for the exact declaration):

```cpp
// New API, taking bounded neutral IR rather than JSON or cooked object memory.
static ENGINE_API TRefCountPtr<FMaterialShaderMap> CreateLegacySlateMap(
    const FLegacyMaterialIR& Material,
    const TArray<FLegacySlateShaderInput>& Shaders,
    FString& Error);
// Each shader input contains the original record identity and a validated
// FShaderCompilerOutput whose Target is the active Vulkan SM5 platform.
```

For each distinct numeric parameter, append current `FMaterialScalarParameterInfo` or `FMaterialVectorParameterInfo`, assigning full `FHashedMaterialParameterInfo`, display name, association/index and original default. Preserve `FName` number as part of identity. Reject conflicting defaults for the same identity in this first implementation. Construct temporary current expression objects using their public constructors:

* scalar/vector parameters take `(FMaterialParameterInfo, ParameterIndex)` rather than an old default value;
* constant takes `(FLinearColor, uint8 ValueType)`;
* folded math takes `(A, B, uint8 Op, uint32 ValueType)`;
* append takes `(A, B, uint32 ComponentsA)`;
* swizzle takes `(X, int8 R, G, B, A)`; validate the serialized element count against the constructor's derived count first;
* trig takes `(X, Y, ETrigMathOperation)`; current `WriteNumberOpcodes` emits Y only for atan2, matching the old evaluation's use of Y.

Then reproduce `HLSLMaterialTranslator.cpp:1064`: allocate one header per original top-level scalar/vector slot, set `OpcodeOffset = UniformPreshaderData.Num()`, call the expression's current `WriteNumberOpcodes`, and set `OpcodeSize` from the resulting difference. Do not deduplicate or reorder output slots. Temporary graph objects can then be released: runtime evaluation reads bytecode and parameter tables.

This avoids hand-authoring opcode widths, stack order, swizzle operands, or the extra value-type byte used by dot/cross. Current division also retains the safe-divisor behavior. `MaterialUniformExpressions.cpp:554,590,859` demonstrates that scalar/vector opcodes consult the current `MaterialRenderProxy` and material interface before using the stored default. Static default-byte uploads cannot replace this path.

For later Donut support, append its `FMaterialTextureParameterInfo` into `UniformTextureParameters[Standard2D]`, preserving full parameter identity, `TextureIndex`, and sampler source; its original virtual flag is false and layer/page-table values are -1. The current texture getter performs the live texture lookup. Reject other texture/VT cases until explicitly converted.

Call `UniformSet.CreateBufferStruct()` after filling all arrays. Compare size, resource offsets and base types with the original decoded debug layout before proceeding. `CreateBufferStruct` copies its computed layout into the set. Its generated metadata uses `DataDrivenUniformBuffer`, which initializes its layout immediately and is not added to the global auto-binding list (`ShaderParameterMetadata.cpp:82`). The explicit `FMaterialShader` constructor binding supplies `Material`; do not add a duplicate global metadata registration. Retain the temporary metadata through layout/output construction and define its ownership in the bridge. The numeric layout remains vectors then scalars rounded up to float4; preshader-bytecode emission order does not change that output layout.

Source: `Runtime/Engine/Private/Materials/MaterialUniformExpressions.h` (current constructors/opcode writers); `.cpp:340,554,590,1083`; `HLSLMaterialTranslator.cpp:1064,2899`; `Runtime/Engine/Public/MaterialShared.h:255,349,501,550`.

## Build Vulkan outputs and allocations together

The mapped SPIR-V descriptor receipt, original D3D resource-table receipt, and decoded named fields must produce a single consistent `FVulkanShaderHeader` **and** `FShaderParameterMap`. Original D3D `BufferIndex` values are not Vulkan header array indices.

| Original use | Actual current Vulkan representation |
| --- | --- |
| Loose cb0 values | `PackedUBs[0]` backed by the mapped cb0 descriptor; retain original scalar byte offsets/sizes in `LooseData` allocations with buffer index 0. Set packed size from the reflected block span, preserving unused gaps. Populate packed-global metadata for those allocations as the compiler does. |
| Material/other genuine UBs | A `UniformBuffers` entry plus matching `UniformBufferSpirvInfos`. Add named allocation `(HeaderUBIndex, FVulkanShaderHeader::UniformBuffer, 1, UniformBuffer)`. `FShaderUniformBufferParameter::Bind` obtains its runtime base index from allocation **BufferIndex**. |
| Explicit font texture/sampler | `Globals` entries and matching `GlobalSpirvInfos`; add named allocation `(FVulkanShaderHeader::Global, HeaderGlobalIndex, original resource count, SRV/Sampler)`. Use the reflected descriptor type. |
| Resources carried by a Material UB | `FUniformBufferInfo.ResourceEntries`: original source resource-table index -> verified current layout resource index -> mapped global descriptor. `UBBaseType` comes from the checked current resource layout. |

Use real UBs (`bHasRealUBs=true`) for material constants. `ConstantDataOriginalBindingIndex`/global original binding values come from the mapped binding receipt, not an assumption that D3D registers equal bindings. Populate all SPIR-V descriptor-set/binding **word offsets** from reflection after the entry-point rename. Header UB/global array lengths must match their corresponding SPIR-V-info arrays.

For a UB resource entry, do not confuse a resource-table index with its CPU byte offset. Require the current layout entry at that index to match the recovered original byte offset/base type. Populate its `GlobalIndex` from the mapped resource descriptor. `SetShaderUniformBufferResources` uses this exact resource-array index and verifies the current layout hash. Put the **current computed layout hash** in the Vulkan header after the shape comparison; keep the old D3D hash separately as provenance. Do not manufacture a zero layout hash to suppress validation.

Derive `InOutMask` from reflected VS input attribute locations and PS target locations/depth output; zero is not valid metadata for the Slate draw. Local compiler code sets attribute/target bits and bit 15 for depth. Reuse the proven UE entry-point convention `main_%08x_%08x`, then re-reflect: a longer `OpEntryPoint` changes later word offsets. Keep the original cooked source hash as recorded (often zero); any additional artifact digest is bridge provenance, not a recovered HLSL source hash.

Write `Header` then SPIR-V with `FMemoryWriter` into `Output.ShaderCode.GetWriteAccess()`. Set actual target/frequency, instruction/sampler metadata from validated inputs, and success only after validation. Call `Output.GenerateOutputHash()` **after** building the converted code and parameter map. This creates the current code identity; retain the original DXBC library hash only as a separate origin association. Add each output with `Map->GetResourceCode()->AddShaderCompilerOutput(Output)`.

Source: `Developer/VulkanShaderFormat/Private/VulkanShaderCompiler.cpp:575,652,898,1213,1498`; `Runtime/VulkanRHI/Public/VulkanShaderResources.h:60`; `Runtime/VulkanRHI/Private/VulkanCommands.cpp:331,420,554`; `Runtime/RenderCore/Private/ShaderParameters.cpp:84`; `ShaderCore.cpp:442`.

## Runtime finalization

The actual Linux client defines `WITH_EDITORONLY_DATA=0` (`native/Intermediate/Build/Linux/B4D820EA/FortniteLinux/Development/SlateRHIRenderer/Definitions.SlateRHIRenderer.h:26`). In that configuration, compiled `FShader` instances do not store `OutputHash`; `GetOutputHash()` returns `ShaderSourceDefaultHash`. Existing `FShader::Finalize(Code)` looks up that value. Thus compiled-constructor + ordinary `FinalizeContent()` alone cannot build this runtime map.

The bridge adds a checked overload `FShader::Finalize(const FShaderMapResourceCode*, const FSHAHash& KnownOutputHash)` in `RenderCore/Public/Shader.h` and `Private/Shader.cpp`. It resolves the code index, checks existence/frequency and any prior index, and assigns private `ResourceIndex`. Preserve existing editor behavior. In the non-editor branch of the existing overload, require an already resolved, in-range index with matching frequency instead of attempting the unavailable output-hash lookup. Do not expose an unchecked index setter.

**Add all seven outputs before resolving any indices:** `AddShaderCode` inserts hashes in sorted order and can shift earlier indices. Then construct the seven shaders, finalize each with its known converted output hash, and add them to the content. Ordinary `FMaterialShaderMap::FinalizeContent()` can now sort/hash the content and execute the existing memory-image freeze and `FShaderMapResource_InlineCode` initialization. This preserves the engine's normal finalization path.

The Engine factory assigns a new `FMaterialShaderMapContent(TargetPlatform)` through public `AssignContent`, fills its compilation output, and uses the original cooked map identity plus quality/feature with current layout parameters. `FMaterialShaderMap` owns its private ID and already starts with finalized/successful flags in its constructor; keep the object private until the construction/validation has succeeded. Use `TRefCountPtr`, not direct deletion. Do not register this detached first test in the global material cache or set a material's inline map yet.

Source: `Runtime/RenderCore/Private/Shader.cpp:500,550,653`; `ShaderResource.cpp:156,161`; `ShaderMap.cpp:57,65,681`; `Runtime/Engine/Private/Materials/MaterialShader.cpp:1263,2303`; `Runtime/Engine/Public/MaterialShared.h:726,969,1258`.

## Implementation and remaining gate

The patch changes the two RenderCore finalization files above and adds the neutral Engine IR/API header and factory implementation in `Runtime/Engine/Private/Materials`; native supplies an opt-in caller. No changes to Slate shader layouts are needed. Keep Vulkan header construction in the native caller/bridge module so Engine acquires no Vulkan dependency. Current Slate constructors are sufficient.

The first detached Locker gate demonstrated these before material attachment:

1. Current preshader evaluation through `FillUniformBuffer` matches `evaluate_uniforms.py` for original defaults and several overrides of `Rotation_U`, `Offset_U`, `RBGA1` and `SelectionColor`; verify association/index isolation, preserved output-slot positions and padding. Use the existing reference evaluator's float behavior/declared tolerance for transcendental operations rather than asserting universal cross-platform bit identity.
2. Checked layout equals 336 bytes with sampler offsets 320/328; all seven constructors resolve their recovered allocations, and current `ParameterMapInfo` reflects those **Vulkan** allocations.
3. Frozen detached map resolves all seven types/permutation 0 through `GetShader`; each resource index points to the matching converted output hash/frequency; current frozen-content validation passes in the actual non-editor Linux build.
4. VS/PS RHI objects were requested on the rendering thread. Header/factory success is a construction milestone only. A subsequent gate created a Slate pipeline, bound the material uniform buffer and loose parameters, drew the original Locker quad and read pixels back as described above. The later actual-owner gate described below checks `IsComplete(Material)` and mesh requirements before normal attachment.

The constructed Vulkan parameter/header pair was exercised by the Locker quad pipeline; other material and vertex interfaces remain unverified. The eight fixture cases establish the tested live-parameter behavior only; the detached seven-shader check alone did not establish the actual owner’s requested layout.


## Original asset ingress

The current Engine/native working trees add a bounded opt-in provider for the original Locker resource and a game-thread completeness gate bound to its actual owner. `-FortniteLegacyMaterialIngress=<absolute path to porting/generated/material_pipeline/locker-ingress-bundle.json>` registers the pinned provider; `-FortniteProbeLegacyOriginalMaterial` requests the attachment, completeness, reader-negative and attached-map quad checks. The staged software Vulkan probe passed with exit 0: the actual owner reported seven required shaders, zero missing and complete=1; the same map was attached on game and render threads and drew the quad with 2,832 changed pixels. Four malformed-reader cases were rejected. Independent verification of all seven shader artifacts passed (`porting/generated/frontend-probes/legacy-material-ingress-artifacts-3-validation.json`); both RGBA readbacks were byte-identical to the earlier detached draw. The receipt and log are `porting/generated/frontend-probes/legacy-material-ingress-owner-3.json` and `legacy-material-ingress-owner-3.log`. The earlier owner-1 run lacked SmoothStep; owner-2 staged that dependency. Its log still reports missing Water assets and a handled Vulkan vendor ensure.

The provider retains all 19 original shader records and their library provenance, while converting only seven Slate shaders. The runtime gate established these seven satisfy the tested actual owner’s current non-editor layout. It logged UI domain, translucent blend and unlit shading, consistent with the source property report. Other layouts, Donut textures, mesh/View bindings and Editor cooked-ID behavior remain open, as does comparison to original-client pixels. See `UE_MATERIAL_INGRESS.md` for the exact resource bounds, hashes, attachment and threading details.

## Generic numeric checkpoint

Engine `6925baad3` supports the tested numeric layouts, with native `25bf1c4` retaining the existing Locker path and loading the original GradientPanel base and PowerToastGlow instance. The complete PowerToast layout is 208 bytes, MenuButton 304 bytes and Locker 336 bytes, including sampler resources; a layout alone does not prove its runtime binding. The PowerToast actual-owner run retains 19 records, converts seven, reports zero missing, checks game/render attachment and passes eight graph and eight owner uniform cases. Its quad changes 3,952 pixels. Five source instance override identities were verified, and all seven PowerToast and Locker artifacts independently pass. Exact build, probe, join and hash receipts are in `generated/material_pipeline/numeric-power-checkpoint-results.json`. See [UE_MATERIAL_INGRESS.md](UE_MATERIAL_INGRESS.md) for owner and parent identity, CLI and remaining work.

The later native `ebea44e` / Engine `6925baad3` checkpoint also verifies the
original MenuButton owner and real View uniform binding. GameTime 0/3.25 changes
1851 pixels with RealTime isolation; separate fixed-time Primary Color overrides
change 4096 pixels. All 21 final emitted shader payloads, both builds and both
joins pass. See `generated/material_pipeline/numeric-menu-checkpoint-results.json`
and [UE_MATERIAL_INGRESS.md](UE_MATERIAL_INGRESS.md). Full View ABI and original
pixels remain unverified; Donut texture resources and actual root widgets are next.
