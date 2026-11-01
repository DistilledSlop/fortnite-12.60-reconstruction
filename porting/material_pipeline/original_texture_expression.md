# Original Texture and TextureProperty serialization

Original runtime image evidence is in
`porting/generated/material_pipeline/original-texture-expression-evidence.json`,
SHA-256 `d506041de197e33cbc837ce739ffbbd3057d66a56e9724aa514331aa7b3e5806`.
It checks the type names, type objects, constructors, vtables, getters and
serializers; decompilation and disassembly are retained under
`expression-texture-proof-1/` and `expression-texture-proof-2/`.

`FMaterialUniformExpressionTexture` uses base serializer `0x14433eb60`: a
16-byte payload of int32 texture index, int16 layer index, int16 page layer,
int32 sampler source and uint32 virtual flag. `TextureParameter` serializer
`0x14433f680` first writes ParameterInfo then this same base payload.
`TextureProperty` serializer `0x14433e410` writes one recursive expression
followed by a byte: 0 for size and 1 for reciprocal size. The parser bounds
recursion and validates index, sampler, virtual flag and property values.

The original `TextureProperty` evaluator resolves a texture through the child
and runtime context and reads its resource dimensions. This context and the
texture-index/default mapping are not established. Numeric evaluation and
runtime fixture flattening therefore still reject both new expression types;
structural decoding does not establish usable or converted materials.

The fresh `porting/generated/frontend-probes/frontend-material-inventory-textures/summary.json`
(SHA-256 `06f45720da994337266ac1a5774116675a88aa15f36786554fadbdd31e0c0254`)
updates the ten prior failures without overwriting earlier reports. Each decoded
resource reaches exact EOF and has 19 validated original shader archive
associations. Extracted and closure `.uasset` and `.uexp` hashes agree for all
ten. The overlay contains 147 material owners and 65 completely decoded own
resources, with no structural decode failures. This is not runtime proof.

Validation: with `ulimit -c 0`, `python3 -I` runs of
`porting/material_pipeline/test_legacy_shader_map.py` (21 tests, including all
ten original resources and 190 shader associations), `test_evaluate_uniforms.py`
(7), `test_export_runtime_fixture.py` (4), and
`test_export_frontend_numeric_batch.py` (14, two skipped) pass. The original
resource test skips if the local extraction or overlay is unavailable. The
batch producer's `load_inventory` also accepts all 147 overlay owners.
