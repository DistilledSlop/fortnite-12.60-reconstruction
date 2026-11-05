# Original Paper2D translucent material: recovered pixel semantics

The original cooked **TranslucentUnlitSpriteMaterial static permutation** has executable shader evidence for the stock sprite multiplication graph. This supports the current stock **parent graph's** equivalence for the material color and opacity inputs. The separate [legacy override proof](paper2d_legacy_override_evidence.md) now establishes the current stock instance's effective translucent override. Successful Linux cooking/loading, all-pass equivalence and original pixel parity remain unverified.

Evidence lives in `porting/generated/paper2d-shader-semantics/`. No original/source asset, engine file, cook output, or prior evidence was changed.

## Identity and bounded recovery

`mi-export-map.txt` fixes the single original instance export at `.uexp` bytes `[0,8474)`. `mi-resource.json` identifies the unique exact-end material wrapper and its resource at `[1852,8474)`, 6622 bytes. The serialized friendly name and path identify the **parent** (`DefaultSpriteMaterial`, `/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial`); this is expected inside the instance's own static permutation resource and is not evidence that its blend mode equals the parent's.

`paper2d_shader_prefix.py` reuses the verified legacy expression/record readers, with a copied prefix grammar accepting `/Paper2D/`. It decodes the uniform expressions and ten shader records, enforcing each known shader's declared end, map hash, resource identity, target and code size. Every recovered record associates with the original FortniteGame archive by exact shader output hash. All four base-pass pixel variants (simple/no-lightmap, with/without skylight) use the same output hash:

- PS: `de0821b9b3a3001a2e8ab773c0c380db3608db38`, archive index 3378, 2449 wrapper bytes, 2260 DXBC bytes.
- VS: `ffd918ad5be5d564212cbd3fb0925b301d43a137`, archive index 857, 6684 wrapper bytes, 6476 DXBC bytes.

The original receipt below predates HitProxy support. A later bounded decode
(`mi-hitproxy-prefix.json`) reads all 14 instance records through resource offset
6618, leaving the four-byte cooking marker unconsumed. `FHitProxyVS` spans
5012..5434 and requires one four-byte resource parameter; `FHitProxyPS` spans
5434..5824 and requires one six-byte scalar parameter. Their map hashes, record
boundaries, shader targets and archive identities agree; the corresponding
current UE shader declarations support the parameter types, but their original
semantic names have not been independently established. Alternative zero-byte
parameter lengths fail the map-hash check. The parent resource is independently bounded at `[2084,14476)`. The later
`parent-shadow-prefix.json` decodes four shadow-depth vertex records at resource
offsets 604..2336, with archive indices 1510..1513. Each has six bytes between
the LocalVF parameters and the shader output hash, decoded as one scalar
parameter. The current `FShadowDepthVS` declares one `FShaderParameter` named
`LayerId`; the original field name is not independently verified. All four
records pass declared-end, map-hash, target, resource identity and exact archive
output-hash association checks. Their scalar values are all zero.

`parent-shadow-ps-prefix.json` continues through three shadow-depth pixel
records at resource offsets 2336..3632, with archive indices 2467..2469. Each
contains 32 zero bytes between LocalVF parameters and the output hash. The
current `FShadowDepthBasePS` declares four `FRWShaderParameter` fields, each
serialized as SRV and UAV resource parameters (four bytes each), which supports
this layout; the original field names and individual semantics remain unverified.
The same record-end, identity, target and exact archive checks pass.

`parent-density-prefix.json` continues through the no-lightmap density vertex
and pixel records at offsets 3632..4068 and 4068..4536, associated with archive
indices 859 and 2470 respectively. Both contain three six-byte, unbound uniform
buffer parameters after the LocalVF payload. The current
`FUniformLightMapPolicyShaderParametersType::Serialize` declares three such
buffers; `TUniformLightMapPolicy<LMP_NO_LIGHTMAP>` supplies this policy to the
density shaders. The pixel record then contains three scalar parameters, with
base/size/buffer values `(32,12,0)`, `(20,8,0)`, and `(48,16,0)`; the current
`TLightMapDensityPS` declares three scalar fields in that order. Original field
names remain independently unverified. Both records pass the declared-end,
map-hash, target, resource identity, and exact archive output-hash checks.

`parent-depth-prefix.json` decodes `TDepthOnlyVS<false>` at offsets 4536..4954,
with archive index 1520. There are no additional bytes between its LocalVF
parameters and the shader output hash, consistent with the current
`TDepthOnlyVS` declaration. Its declared end, map hash, target, resource
identity and archive association pass. Five previously supported base-pass
records follow at offsets 4954..7092 (archive indices 1521, 1522, 2471, 2471,
2472). No depth-only pixel record occurs in this decoded prefix, so its original
payload is not inferred from the current declaration.

`parent-velocity-prefix.json` continues through `FVelocityVS` at 7092..7510
and `FVelocityPS` at 7510..7904, associated with archive indices 1528 and 2473.
Neither has bytes between its LocalVF parameters and output hash, consistent
with the current UE declarations of both velocity shader types. Both pass
declared-end, map-hash, target, resource-identity and archive-association checks.
`parent-pointshadow-prefix.json` continues through `FOnePassPointShadowDepthGS`
at 7904..8294 (archive index 812), with no additional serialized fields,
consistent with its current UE declaration. The existing HitProxy decoders then
reach offset 9128.

`parent-depth-pixel-prefix.json` decodes `FDepthOnlyPS<false>` at 9128..9528
(archive index 2475). It contains one six-byte unbound scalar parameter,
consistent with the current `MobileColorValue` declaration; its original field
name remains unverified. Existing uniform-mesh VS/GS decoders then reach offset
10318. All 23 standalone parent records pass declared boundaries, map/resource
identity, target and exact archive association. `parent-pipelines-prefix.json`
continues from offset 10318 through three mesh shader pipelines and five full
stage records. UE 4.24's pre-freeze `TShaderMap::SerializeInline` writes a
pipeline type, stage count and full shader record for each stage. The original
bytes match that grammar, and current renderer declarations corroborate each
pipeline's stage membership; stage order below is serialized order:

- `DepthNoColorOutputPipeline` (10322..11152): `FDepthOnlyPS<false>` and
  `TDepthOnlyVS<false>`, archive indices 2476 and 2477.
- `DepthNoPixelPipeline` (11152..11564): `TDepthOnlyVS<false>`, index 668.
- `VelocityPipeline` (11564..12388): `FVelocityPS` and `FVelocityVS`, indices
  2478 and 2479.

All five stages pass the same record checks, including each stage's embedded
pipeline identity and archive hash/target/size. The pipeline count and stage
membership are validated against these observed declarations; unknown pipeline
types are not decoded. UE 4.24 `FMaterialShaderMap::Serialize` reads `bCooked`
after the mesh maps, corroborating the final four-byte true marker. The parent
and instance reports now consume their bounded resources exactly: 28 and 14
records respectively, including pipeline stages. Complete serialized metadata
does not establish runtime loading or parity.
Neither asset has been attached to the runtime using this decode; mesh shader-map construction also remains
necessary.

`extraction.json` records selected exact archive entries. `wrappers.json` verifies the complete D3D wrapper framing, resource-table bindings, DXBC extent and optional-data extent. The assembly files are vkd3d 2.0's direct DXBC disassembly; no engine shader compiler or cook was invoked.

## What the original shader actually computes

In `mi-basepass-ps.asm`, the input signature explicitly declares `COLOR` in `v2` and `TEXCOORD` in `v3.xy` (lines 5–6). The wrapper's texture map binds material uniform-buffer resource 0 to `t2`, and sampler resource 1 to `s1`. The decoded material has exactly one 2D expression: `TextureParameter(SpriteTexture)`, texture index 0, sampler source 0. Thus this identifies the sampled texture by its serialized active uniform binding, not by an unused import or node name.

Let `S = sample_bias(SpriteTexture, v3.xy, ViewMipBias)` and `V = interpolated COLOR`. Instructions 36–40 directly show:

```
P = S * V                          // all four channels
RGB = lerp(P.rgb, cb2[1].rgb, cb2[2].x)
A = saturate(P.a)
```

The decoded uniform expression order contains two vector expressions (SelectionColor, SelectionColor.rgb), followed by scalar expressions (SelectionColor.a, RefractionDepthBias). The material binding is `cb2`; its debug layout has 48 constant bytes before resources. This matches vector registers 0–1 followed by packed scalar register 2. The current source's `MaterialUniformExpressions.cpp` lines 361–372 independently corroborate this vector-then-packed-scalar layout. Therefore the material stage is:

```
E = max(lerp(S.rgb * V.rgb, SelectionColor.rgb, SelectionColor.a), 0)
A = saturate(S.a * V.a)
```

SelectionColor's original default is `(0,0,0,0)`, so the ordinary default material result is `E = max(S.rgb * V.rgb, 0)` and `A = saturate(S.a * V.a)`. There is no alpha threshold/discard instruction in this recovered base-pass PS. The shader does not premultiply RGB by opacity here. RefractionDepthBias is present in metadata but does not contribute to this base-pass output.

This formula describes the material contribution, not the final framebuffer pixel. Lines 41–59 compute an optional sampled 3D contribution and interpolated contribution; line 82 forms `o0.rgb = E * transmission + contribution`. A view-controlled branch at lines 61–77 can replace RGBA with a position/bounds diagnostic pattern. These branches are retained in the evidence and must not be erased when asserting whole-program equivalence. Their interpretation as fog and bounds visualization is consistent with engine shader source, whereas the algebra and branch existence are directly visible in DXBC. `o0.a` remains the saturated product outside that diagnostic replacement.

The recovered VS independently writes `o2 = r1.zyxw` to the COLOR semantic (line 217), after fetching `r1` from a vertex buffer (line 42). This confirms an interpolated four-channel color path; the PS does not synthesize color from a material constant.

## Comparison with the current stock graph

The immutable `paper2d-tool/final-current-DefaultSpriteMaterial.json` supplies actual connected expression indices and masks:

- Multiply_4 uses SpriteTextureSampler RGB and VertexColor RGB; EmissiveColor points to Multiply_4.
- Multiply_3 uses SpriteTextureSampler alpha and VertexColor alpha; both Opacity and OpacityMask point to Multiply_3.
- SpriteTextureSampler Coordinates points to TextureCoordinate_0; its absent tagged coordinate/tile overrides require the engine's expression defaults for UV0/unit tiling.

Those are serialized current edges, not guesses from node names. They agree with the **original instance's compiled** color/opacity multiplication. The original parent's cooked four missing expression exports cannot establish the same edges by themselves, and its masked-pass behavior has not been recovered here. The current source parent is masked, unlit and two-sided; the original instance overrides blend mode to translucent. The immutable original instance report explicitly contains `bOverride_BlendMode=true` and `BLEND_Translucent`.

The separate legacy native-tail decoder now proves the current stock instance's active translucent override, with disabled shading, two-sided and mask-clip overrides inheriting the verified parent values. Its exact export consumption, version and source hashes are recorded in `paper2d-native-override-evidence/receipt-v2.json`. Combined with the compiled multiplication and opacity evidence above, this makes the stock source a justified candidate for a faithful representation cook; the resulting cook, all-pass behavior and pixels still require validation. Copying the parent as masked, declaring parity solely from matching names, or silently substituting a different material would not satisfy this evidence.

## Reproduction and validation

1. Produce export maps with the immutable Cue4Inspect DLL named in `paper2d-tool/receipt.json`, using the original isolated overlay and `--loose --export-map-json`.
2. Frame each first export with `inspect_material_resource.scan_resource(raw, 0, serialSize)`; retain exact raw/export hashes.
3. Run `paper2d_shader_prefix.py --uexp <original .uexp> --resource-report <mi-resource.json> --shader-archive <original FortniteGame archive> --report <mi-prefix.json>`.
4. Run `shader_archive.py` with `--shader-index 3378 --shader-index 857 --max-shaders 2`, then `inspect_d3d_code.py` on its extraction report.
5. Run the existing `vkd3d2-compiled/bin/vkd3d-compiler -x dxbc-tpf -b d3d-asm --formatting=signatures,offsets` on each extracted DXBC, setting LD_LIBRARY_PATH to that tool prefix's lib directory.

`test_paper2d_shader_prefix.py` passes four meaningful fail-closed checks: unknown pipelines preserve their unconsumed boundary; an unknown record does not consume or seek through even a malicious end offset; a known record cannot exceed the resource; and an unsupported vertex factory fails before records. Artifact/tool/input SHA256 values are in `receipt.json`, with the full archive hash additionally in both prefix reports. Neither DXBC execution nor a UE draw has been tested.

## Complete original shader conversion

The 28 parent and 14 instance shader-record occurrences, including every nested
pipeline stage, reference 33 distinct original archive shaders. All 33 were
extracted with `shader_archive.py`, inspected with `inspect_d3d_code.py`, converted
with `translate_mapped_dxbc.py`, and reflected with `reflect_mapped_spirv.py`.
Every converted shader passes `spirv-val --target-env vulkan1.0`. The conversion
retains 163 explicit descriptors with zero unmatched original resource-table
entries: 63 uniform buffers, 62 uniform texel buffers, 19 sampled images and 19
samplers. All 42 original records have empty serialized parameter-binding arrays;
their uniform-buffer, vertex-factory and pass bindings still require runtime
construction.

Generated receipts and exact code artifacts are under
`porting/generated/paper2d-shader-semantics/conversion/`: `source.json`, `d3d.json`,
`mapped/mapped-translation-report.json`, and `reflection.json`. Their cross-hashes
bind the extracted wrappers, original DXBC, conversion tools and resulting
SPIR-V. This establishes offline conversion and descriptor association, not
compatible runtime uniform values, owner completeness, texture binding or a UE
draw. The original material reader and provider still need the complete LocalVF
and pipeline path; the existing seven-Slate-shader provider cannot load these
resources.

`python3 porting/material_pipeline/export_paper2d_shader_manifest.py` reparses
both original resources and exports `shader-manifest.json`. It preserves the
42 occurrences and three ordered pipeline stage lists, links them to all 33
converted outputs, and verifies framing, archive bytes, report cross-hashes,
SPIR-V reflection and original descriptor associations. The focused rejection
test passes after a changed conversion input is rehashed into its report chain.

The source-only Engine bridge patch
`porting/patches/ue425-legacy-localvf-material-factory.patch` adds a separate
`CreateLegacyLocalVertexFactoryMap` entry point. It constructs actual material
and mesh shader objects and shared or unique shader pipelines, checks current
registered type/permutation/target and complete binding consumption, and retains
temporary mesh and unique-stage ownership through freezing or rejection. The
existing strict seven-record Slate entry point remains separate. Patch
reapplication and source review passed; compilation and runtime checks are
pending. This factory is not yet connected to the original-material provider.

The native `LegacyOriginalMaterialReader` now has the corresponding source
decoder for the exact Paper2D shader-map identity, observed shader bodies and
three pipelines. It retains bindings, resource identities and ordered stage
indices alongside every record and the raw resource bytes. Source review
matches the verified Python body layouts; native execution remains pending.
The provider still needs to construct converted outputs with the original
View, LocalVF, pass and texture bindings before calling the new factory.

`LegacyLocalVFAdapter` now implements the output-construction kernel for those
bindings. Given an original record, its verified converted shader and explicit
original uniform/loose parameter contracts, it checks recursive member layouts,
current metadata registration, original resource tables and complete descriptor
consumption before emitting the Vulkan header, parameter map and shader code.
It handles uniform texel buffers, resource-only uniform buffers, packed loose
parameters and the observed graphics frequencies. The caller's output remains
unchanged on rejection. The existing Slate adapter shares its SPIR-V entry-point
rewriter; the Slate mapping policy is unchanged.

Source/schema review covers the 42 occurrences, 33 converted shaders and 163
descriptors. Compilation and execution remain pending. This kernel is not
connected to the provider: the [full View comparison](view_original_identity.md)
now proves a whole-buffer mismatch, so supplying the current View by name would
correctly fail its contract. Runtime work must establish compatible accessed
members/resources or implement a faithful remapping, along with independently
recovered LocalVF and pass contracts.
