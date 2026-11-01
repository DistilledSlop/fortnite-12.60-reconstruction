# Frontend numeric material batch

`export_frontend_numeric_batch.py` consumes the bounded frontend closure inventory,
compares every inventory package's `.uasset`/`.uexp` to the immutable extraction,
reparses bounded original resource bytes, and verifies all original shader archive
associations. It writes only to a **new** directory below `porting/generated`.
Earlier exporters, bundles and runtime pins remain separate.

```sh
python3 -I porting/material_pipeline/test_export_frontend_numeric_batch.py
python3 -I porting/material_pipeline/export_frontend_numeric_batch.py \
  --inventory porting/generated/frontend-probes/frontend-material-inventory/summary.json \
  --output porting/generated/material_pipeline/frontend-numeric-batch-9 \
  --view-access-report porting/generated/material_pipeline/frontend-view-access-all-1.json \
  --view-original-metadata porting/generated/material_pipeline/view-identity/evidence.json
python3 -I porting/material_pipeline/test_export_frontend_numeric_batch.py \
  --batch porting/generated/material_pipeline/frontend-numeric-batch-9
```

Choose a fresh output path when repeating the command. An existing directory is
rejected before generation. A failed run leaves its partial directory for
inspection; it is never silently resumed. Tool subprocess output goes to bounded
per-chunk log files. Unique Slate shader selections are chunked at 256; mapped
modules are structurally validated and independently reflected. A tool-level
failure aborts the batch instead of producing an apparently successful manifest.

`report.json` covers every material owner, including explicit rejection reasons.
It keeps mapped reflection for numeric candidates rejected at the binding stage,
including View users. Texture expressions/overrides, collections/virtual textures,
unsupported expression nodes, incomplete or different shader-record shapes,
unclassified constant-buffer spans and unproven View bindings fail closed.
Slate's own element texture and sampler remain supported independently of original
material texture expressions. Every included module has explicit typed bindings.

`manifest.json` uses schema `fortnite.legacy-material-manifest.v1`. Each entry has:

- `owner_path`, `owner_type`, and `parent_path` for the actual owner and direct MI parent;
- `shader_identity` for the ultimate base material and `resource_owner_path` for the
  nearest ancestor carrying its own serialized permutation;
- `resource_sha1`/`resource_sha256`, absolute `bundle_path`, `bundle_schema`,
  `bundle_sha1`/`bundle_sha256`, and `requires_view` indicating an explicit proven
  GameTime-only View contract.

The native consumer must receive the manifest SHA-256 explicitly and check it
before trusting entries; then verify bundle identities, live bounded source
bytes/names and owner identity. Own-resource entries receive resource serialization
callbacks. Nonstatic inherited owners reuse an ancestor map, with exact resource
and base-identity cross-references, and require separate actual-owner inheritance
checks. A successful export does not prove any callback, owner completeness,
attachment, live uniform bytes or rendering. The real owner completeness gate
must decide the required shader set.

Bundles use `fortnite.legacy-material-ingress.v2`, retaining raw original resource
bytes, local names, the complete parsed map, all 19 original shader records and
all 19 library code receipts. Only the actual seven Slate records are adapted.
`runtime_fixture.uniform_cases` are graph defaults/perturbations;
`owner_uniform_cases` start with cumulative parent-chain plus child overrides.
`owner_parameter_expectations` contains effective numeric overrides even if
optimized out of this graph; native checks must query inherited values.
`unused_owner_parameter_expectations` identifies those absent from the graph.
`serialized_override_chain` retains every original per-owner numeric array,
full FName number/association/index identities and source instance parameter
arrays, while `graph_effective_parameter_expectations` contains only identities
evaluated by this graph. Numeric cases include wrong association, index and FName
number isolation whenever parameters exist.

The no-View batch (retained as batch-5) has 147 owners: 44 new eligible owner entries (21 own
resources and 23 inherited owners), four eligible owners already covered by frozen
pins, and 99 rejected owners. The two other frozen owners are rejected here for
texture/View contracts; all six exact existing owner paths are excluded from the
manifest. The batch selects 167 unique Slate library programs, retaining complete
19-record provenance separately. Forty-five owners require unproven View bindings,
26 fail the current exact Material buffer span contract, 16 depend on resources
that the inventory did not completely decode, ten have texture expressions, and
two have unsupported/incomplete record shapes. These categories describe the
first failed check for each owner, not an exhaustive list of all its limitations.


The optional View arguments must be supplied together. They allow a shader only
when its exact library index and fresh mapped SPIR-V SHA-256 match the supplied
access proof. The exporter checks the proof inspector/grammar identities and
reruns the strict access inspector on each fresh module. Original metadata must
identify float32 GameTime at byte 2264 in the 3,952-byte original View struct.
The shader contract requires View b1 span 2272 and Material b2 fitting the numeric
buffer. `view_contract` retains the original metadata path/hash plus
`access_proof_path`, `access_proof_sha256`, and matched `shader_access_records`.
The native consumer must verify those external proof hashes and every PS proof.
Some physical float4 loads cover 2256..2271; the proof establishes that every use
selects GameTime at 2264. This does not establish full View ABI compatibility.

The Feature-enabled batch-6 has 50 new eligible owner entries: 22 own resources
and 28 inherited owners. Its six View entries are FeatureButton_Master and five
nonstatic descendants using the exact same five proven pixel shader programs.
The remaining 39 View owners have no matching per-shader proof and remain
explicitly rejected. There are 93 total rejected owners and four eligible frozen
existing owners. The same 167 unique Slate programs are mapped and validated.
The optional artifact audit passes alongside all eleven isolated tests: 50
bundle/source identities, 350 mapped shader references/typed contracts, and 900
numeric graph/owner cases. Batch-6 contains no draw parameter, time window or original pixel oracle.
Live owner and controlled GPU time checks remain work for the native provider
and runtime probe.


Batch-7 preserves batch-6 and adds controlled source-derived View cases for the
six exact Feature program profiles. The original default PS library index 3559
(mapped module SHA-256 `4fc49bec9850cc07fe798e555f9313e2b9d2d6cb9de0c03c56ec818046cfbd80`)
observes GameTime at `cb1[141].z`, uses the period at `cb2[24].z`, gates the
threshold at `cb2[24].y`, and reads UseShineSwipe at `cb2[24].w`.
Each profile sets the exact serialized scalar identity `UseShineSwipe` (FName
number 0, association 2, index -1) to 1 and retains all effective owner numeric
overrides. The exporter evaluates CPU constants, derives threshold and period
from numeric indices 97/98, requires a gap greater than 0.25, and chooses GameTime
0 and threshold+0.25 with RealTime 10/20 cross-pairs. Missing parameter/program or
invalid source window yields an explicit unsupported profile while retaining the
otherwise valid owner bundle.

Master, Base, Disabled and Text_Base derive 3.25 as the active time. Hovered and
Pressed have inherited source overrides and derive approximately 1.583333373.
`view_cases` stores all four complete override lists/expected numeric buffers and
controlled times. `view_draw_expectations` records equal pairs (0,2)/(1,3),
different pairs (0,1)/(2,3), register slots, source shader identity and timing
rationale. These are source-derived diagnostic expectations, not successful GPU
readbacks. The batch-7 audit passes 12 tests covering 50 bundles, 350 shader
references and 924 numeric cases (including all 24 View cases).

The preserved batch-7 manifest SHA-256 is
`5a77ff5ee75f769785fcb434f6713671fa37fb1f4ca7da40f93c6d4bd73eae56`.
Frozen Donut/Menu exclusions in the batch report describe this exporter's scope;
they do not invalidate those owners' separately preserved runtime checkpoints.


Batch-9 preserves all earlier batches, including the inherited Base draw failure.
The actual master diagnostic changed 1,864 pixels and passed RealTime isolation;
the actual inherited Base diagnostic changed zero pixels despite passing its
source identity and numeric checks. Base's original ShineRGBA is
(0, 0, 0, 0.498039). Selecting an active timer window therefore does not establish
that every inherited owner's source appearance produces a visible time effect.
No source color or other appearance override is invented to force visibility.

Every Feature profile now has an explicit boolean
`view_draw_expectations.require_time_delta`. It is true only for the observed
master diagnostic, with different pairs (0,1)/(2,3). Inherited profiles set it to
false and have an empty `different_image_pairs` list. Their mandatory equal pairs
(0,2)/(1,3) still verify RealTime isolation. Their GameTime changes remain
observations; zero changed pixels does not establish original-client pixel parity.
All original effective overrides, CPU expected numeric cases and source-derived
thresholds remain unchanged, including Hovered/Pressed's approximately
1.583333373 active time.

Batch-9 consumes the complete 49-program View access proof at
`generated/material_pipeline/frontend-view-access-all-1.json`. It includes 88
eligible owners (34 own resources and 54 inherited), excludes five eligible
frozen existing owners, and explicitly rejects 54 owners. The 167 selected unique
Slate programs remain structurally validated. This expands proved access
coverage without inventing draw profiles for other programs.

```sh
python3 -I porting/material_pipeline/test_export_frontend_numeric_batch.py \
  --batch porting/generated/material_pipeline/frontend-numeric-batch-9 \
  --baseline porting/generated/material_pipeline/frontend-numeric-batch-8
python3 -I porting/material_pipeline/test_export_frontend_numeric_batch.py \
  --batch porting/generated/material_pipeline/frontend-numeric-batch-9 \
  --baseline porting/generated/material_pipeline/frontend-numeric-batch-7
```

Both commands pass 14 tests. The artifact audit validates all 88 bundles, 616
shader references and 1,604 numeric cases. Baseline comparisons confirm all 1,604
batch-8 numeric cases and all 924 batch-7 numeric cases are unchanged. Only the
visible-delta policy changes; controlled native runtime validation remains a
separate gate.

Batch-9 manifest SHA-256:
`3c6effeedd6851ba71fc6ffb8d176cca31c35118564daecd2de50b40d0445b51`.
