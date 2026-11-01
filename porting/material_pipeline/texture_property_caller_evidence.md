# Original TextureProperty direct-vector caller

The original `FUniformExpressionSet::FillUniformBuffer` zeroes a complete
float4 before **each** vector expression call. Therefore a direct texture-size
root that writes only R/G leaves positive zero in B/A. Missing texture/resource
leaves all four zeros. This establishes the direct-root contract; nested
expression callers still require separate evidence.

The original `STAT_FUniformExpressionSet_FillUniformBuffer` literal is at
`0x146ac2670`, referenced by `0x144353955`. Its chained exception-directory
entry begins at `0x14435394a` and links to the actual entry `0x144353900`.
The vector loop at `0x144353b10` loads the array at `this+0x10`, zeroes the
16-byte temporary at `RBP+8`, passes its address in R8, invokes virtual `+0x18`,
copies all four result lanes to the destination and advances it by 16 bytes.
The backwards branch repeats initialization for every vector. TextureProperty
vtable `0x146abfe78 + 0x18` points to the original evaluator `0x14433e4b0`.
Its existing dimension/partial-write proof is documented in
[original_texture_context.md](original_texture_context.md).

`generated/material_pipeline/texture-property-caller-proof-2/receipt.json`,
SHA-256 `03a19ce546da78b2a0229d909d107c5e8f76af51fc05d78a440c839101535c13`,
pins the original memory-image identity, six raw code/table/unwind windows and
the frozen inspector source. Proof 1 is retained; proof 2 clarifies that the
reported successful dimension lanes refer to the size-property mode.

```sh
python3 -I porting/material_pipeline/test_texture_property_caller.py
python3 -I porting/material_pipeline/inspect_texture_property_caller.py --output porting/generated/material_pipeline/<fresh-directory>
```

All three tests pass, including actual-image replay and rejection of missing
zeroing, a changed virtual slot, a partial result copy and a missing loop
backedge. The inspector reads the original memory layout using
`VA=0x140000000+fileoffset`, verifies its full SHA-256 and follows the original
chained unwind metadata; it does not open or modify Ghidra. Earlier exploratory
objdump outputs are retained under `social-border-original-caller-{1,2}`.

SocialUserItemBorder_Master's source vector expression 2 is a direct
TextureProperty mode 0 with a regular Texture child at index 0. Its actual
atlas identity, reference order, source dimensions/resource and native runtime
evaluation must be proven separately. This receipt adds no runtime or widget
acceptance credit.
