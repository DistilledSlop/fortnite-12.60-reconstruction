# Original Paper2D translucent material: recovered pixel semantics

The original cooked **TranslucentUnlitSpriteMaterial static permutation** has executable shader evidence for the stock sprite multiplication graph. This supports the current stock **parent graph's** equivalence for the material color and opacity inputs. The separate [legacy override proof](paper2d_legacy_override_evidence.md) now establishes the current stock instance's effective translucent override. Successful Linux cooking/loading, all-pass equivalence and original pixel parity remain unverified.

Evidence lives in `porting/generated/paper2d-shader-semantics/`. No original/source asset, engine file, cook output, or prior evidence was changed.

## Identity and bounded recovery

`mi-export-map.txt` fixes the single original instance export at `.uexp` bytes `[0,8474)`. `mi-resource.json` identifies the unique exact-end material wrapper and its resource at `[1852,8474)`, 6622 bytes. The serialized friendly name and path identify the **parent** (`DefaultSpriteMaterial`, `/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial`); this is expected inside the instance's own static permutation resource and is not evidence that its blend mode equals the parent's.

`paper2d_shader_prefix.py` reuses the verified legacy expression/record readers, with a copied prefix grammar accepting `/Paper2D/`. It decodes the uniform expressions and ten shader records, enforcing each known shader's declared end, map hash, resource identity, target and code size. Every recovered record associates with the original FortniteGame archive by exact shader output hash. All four base-pass pixel variants (simple/no-lightmap, with/without skylight) use the same output hash:

- PS: `de0821b9b3a3001a2e8ab773c0c380db3608db38`, archive index 3378, 2449 wrapper bytes, 2260 DXBC bytes.
- VS: `ffd918ad5be5d564212cbd3fb0925b301d43a137`, archive index 857, 6684 wrapper bytes, 6476 DXBC bytes.

The reader stops **before** unknown `FHitProxyVS` at resource offset 5012 (`.uexp` offset 6864), leaving 1610 bytes uninterpreted. It never reads that unknown record's end offset or skips its body. `complete_serialized_shader_map` is false. The parent resource is independently bounded at `[2084,14476)`; its prefix stops at the first shadow-depth VS at resource offset 604, leaving 11788 bytes uninterpreted. There is no claim that either entire shader map was decoded.

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

`test_paper2d_shader_prefix.py` passes three meaningful fail-closed checks: an unknown record does not consume or seek through even a malicious end offset; a known record cannot exceed the resource; and an unsupported vertex factory fails before records. Artifact/tool/input SHA256 values are in `receipt.json`, with the full archive hash additionally in both prefix reports. Neither DXBC execution nor a UE draw has been tested.
