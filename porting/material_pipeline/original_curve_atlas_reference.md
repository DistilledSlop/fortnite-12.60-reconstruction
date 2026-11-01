# Original CurveAtlasRowParameter texture references

The original `UMaterialExpressionCurveAtlasRowParameter` participates in the
material reference collector and returns its reflected `Atlas` object. This
proves the node's reference behavior; the material's complete expression and
function traversal must still establish the ordered texture array.

Primary evidence is frozen at
`generated/frontend-native-evidence/original-curve-atlas-reference-proof-1/receipt.json`,
SHA-256 `f007855d75984f806382e05b2b79a1cc2899b28edc28ee032fc1372569e2a202`.
It contains 23 content-addressed immutable inputs, 19 original memory windows,
metadata decoding, and the original decompiler/assembly outputs. Its original
image SHA-256 is
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`;
this is memory layout, with `VA = 0x140000000 + fileoffset`.

The original class getter `0x144788ae0` names
`/Script/Engine.MaterialExpressionCurveAtlasRowParameter`, registers size
`0x88`, and passes constructor callback `0x144789ec0`. The callback calls
`0x1443050b0` on the initializer's non-null object. That constructor calls
its parent, then loads `0x146ab40f0` and stores it at the object's primary
vtable pointer. Thus this is the installed derived vtable, rather than a
candidate inferred from neighboring symbols. The registered parent thunk
`0x14478a440` leads to `0x144791ca0`, whose original class literal is
`MaterialExpressionScalarParameter`.

| Installed slot | Original target | Complete leaf bytes | Behavior |
| --- | --- | --- | --- |
| `+0x268` | `0x1409bebd0` | `48 8b 41 68 c3` | `mov rax,[rcx+0x68]; ret` |
| `+0x270` | `0x1406801d0` | `b0 01 c3` | `mov al,1; ret` |

There are no branches or additional calls in either leaf. Original property
metadata at class block `0x146bd7600` binds `Atlas` to an Object property at
`0x68`, with referenced-class thunk `0x1446ef940`. That thunk resolves to
`0x1446f0b70`, whose original registration literal names
`/Script/Engine.CurveLinearColorAtlas`. The neighboring `Curve` object is at
`0x60`, and `InputTime` is a struct at `0x70`; the getter therefore returns
the atlas, rather than the curve or expression input.

The previously proven original collector in
[original_texture_context.md](original_texture_context.md) calls these slots.
Its parent direct-expression branch adds unique non-null texture pointers;
its function branch preserves its separate append behavior. Two direct atlas
nodes pointing to the same atlas consequently contribute one first-occurrence
reference in that direct branch. This checkpoint does not assert the final
index of `CA_SocialPanel`: the SocialUserItemBorder producer must preserve the
complete parent order and recursively inspect `GetUserInterfaceUV` and every
other retained function/layer dependency. It must reject an unsupported class
or missing closure rather than infer ordering from imports.

Reproduction from the repository root:

```sh
porting/.venv/bin/python -I porting/material_pipeline/inspect_original_curve_atlas_reference.py --output /absolute/fresh/directory
```

The helper verifies the image hash, metadata identity, both vtable pointers,
and exact leaf bytes, then freezes its own source, decoding/decompilation
helpers, all four analysis batches, and raw windows. The recorded Ghidra
commands used `view_original_probe.py --functions` for batches 1/3/4 and
`decompile_targets.py --only ... --create-missing --timeout 30 --max-seconds 180`
for batch 2. Batch 2 created missing function definitions only within a
transaction that was rolled back. No program was saved or symbols renamed.
Ghidra's five bookkeeping file changes are recorded in the frozen integrity
reports. The original client was not executed; this is source evidence, with
no rebuilt runtime or material draw claim.
