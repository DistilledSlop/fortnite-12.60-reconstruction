# Original Max and Saturate uniform expressions

The recovered 12.60 runtime image proves these serializers and evaluators;
the neutral IR already has these operations, but native serialized and JSON
readers need separate support. This is not proof of runtime material loading.
The local evidence receipt is `porting/generated/material_pipeline/original-max-saturate-evidence.json`,
SHA-256 `a37af29dbbd18572ed194e57f02bdf6ffff8289e3cf38f5319cb619c95b94203`.
The receipt identifies the runtime image and checks type-name, constructor,
vtable, type getter, serializer and evaluator pointers. The retained
`expression-newtypes-proof-4/` and `expression-newtypes-proof-5/` directories
contain the disassembly and decompilation. The dump is a memory image: its
virtual addresses use image base plus offset, not PE raw section translation.

- Max serializer `0x14433d350` recursively serializes two expression children,
  fields `0x10` and `0x18`, through `0x144344b40`. Evaluator `0x14433d5e0`
  takes the component-wise maximum using `MAXSS`; equal values select the
  second operand, including signed zero.
- Saturate serializer `0x14433ce10` serializes one child at `0x10`. Evaluator
  `0x14433da80` clamps each component to zero through one. Finite negative
  zero is preserved. Nonfinite inputs are rejected by the Python evaluator.

Fresh `frontend-probes/frontend-material-inventory-max-saturate/summary.json`
retains the earlier inventory and updates three source records. Its SHA-256 is
`4ac3d1500324d7af0000a6be2056e36d0c78ace27899682df646e49c99cdb956`.
Original extracted `.uasset` and `.uexp` bytes match the closure copies by
SHA-256 for all three. `M_UI_SimpleFIll_color` decodes to exact resource EOF
12452, with all 19 records associated against the original shader archive.
The two ComicBubble resources now stop at unsupported Texture expressions at
resource-relative offsets 7837 and 2294. The overlay records 55 completely
decoded own resources and 10 fail-closed resources; prior reports are retained.

Validation: `python3 -I porting/material_pipeline/test_legacy_shader_map.py`
(18), `test_evaluate_uniforms.py` (7), `test_export_runtime_fixture.py` (4),
and `test_export_frontend_numeric_batch.py` (14, two skipped). Run with
`ulimit -c 0`; only the first command's full path is abbreviated for the
other scripts. Tests cover nested parsing and truncation, finite float32 and
signed-zero semantics, parameter identity, and flat postorder IR mapping.
