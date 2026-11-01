# Bounded regular-Texture Keybind ingress

`export_keybind_ingress.py` prepares the original
`/Game/UI/InGame/KeybindWidget/Materials/M_Buttonborder_DF.M_Buttonborder_DF`
for a future fixed-pin adapter. It does not modify the numeric manifest, Engine,
native sources, packages or stage, and does not execute UE.

The original parent has one **regular Texture**, not TextureParameter. Its
fixture entry explicitly has `expression_kind: "Texture"` and `parameter: null`.
The source has no ParameterInfo to rename or synthesize. The future adapter must
preserve absent identity and use indexed default lookup; named texture overrides
are forbidden. Fixture/bundle schema v4 makes this distinction explicit.

The original PostLoad reference algorithm is pinned by the existing immutable
texture-context v3 receipt. Expressions position 0 references the actual
T_Buttonborder_DF. The complete function closure contains AAThreshold,
MF_Smoothstep, ScaleUVsByCenter and BreakOutFloat2Components. Its retained
FunctionExpressions are null or function calls with false texture-reference
capability, so functions append no textures. The complete ordered reference
array is therefore:

```text
/Game/UI/Foundation/Textures/Icons/Input/Misc/T_Buttonborder_DF.T_Buttonborder_DF
```

The producer checks parent export index/name pairs, function root export
index/type/name, complete dependency coverage and cycles. Unsupported retained
classes, texture-bearing functions, TextureParameter, TextureProperty, other
texture arrays, virtual stacks and parameter collections fail closed.

Fresh original package inspection/framing/decode is under
`generated/material_pipeline/keybind-inventory-2/`. The entire 8,857-byte
resource and all 19 original records remain present. Seven Slate records use
archive indices 2480, 2481 and 7128–7132; twelve mesh records remain in the
bundle. The source layout is 176 bytes, with a 144-byte numeric prefix and
resource offsets 144/152/160/168, types 5/8/8/8. Six vector and ten scalar
roots flatten to 29 numeric nodes; comparison covers 136 meaningful bytes.

`keybind-shaders-1/` retains all seven original wrappers, exact DXBC slices,
D3D resource tables, mapped SPIR-V and reflection. The producer compares wrappers
to the original archive again, freshly inspects D3D tables and SPIR-V types, and
runs the pinned `spirv-val` on every module. Material texture/sampler links come
from typed original table entries, rather than SPIR-V names. Unknown table
register kinds fail immediately; Material and Element sampled images must match
the original float32, 2D, non-arrayed, single-sample declaration exactly. The current header
resource-link checker remains `verify_runtime_artifacts.py`; actual emitted UE
headers can only be checked after the future native adapter runs.

The isolated Donut inspector decoded the original T_Buttonborder_DF into
`keybind-texture-1/`: 64×64, PF_B8G8R8A8, sRGB, UI group, default filter, wrap
addressing, one forced-inline mip. The producer independently reads its native
platform frame, validates logical/mip dimensions and non-virtual data, and
compares its exact BGRA payload after the declared RGBA swizzle to the decoded
pixels. No resize or replacement occurs. Source-stored and reader-adjusted bulk
offsets are retained separately. The canonical RGBA SHA-256 is
`804c4bcf49353b3fe938fe3dd9bb96b0b6d3b826937b3754c0785250b906fad4`.

The fixture declares a future scratch-buffer sampling control: actual source
versus an independent upload of those pinned pixels must produce identical
readbacks; an asymmetric checker must differ. All three use identical source
shaders, numeric profile, layout and samplers, without owner-cache mutation or
named texture lookup. Profiles `defaults` and diagnostic `combined` are provided;
the latter changes genuine numeric parameters and is not an owner-default claim.
These requirements are not completed runtime evidence.

Run from the repository root after producing the pinned inspection and shader
inputs:

```sh
python3 -I porting/material_pipeline/test_export_keybind_ingress.py
python3 -I porting/material_pipeline/export_keybind_ingress.py --output porting/generated/material_pipeline/keybind-ingress-3/bundle.json
```

The producer requires fresh bundle/fixture destinations. Its sibling
`runtime-fixture.json` retains the seven shader programs and source RGBA path/hash.
`keybind-ingress-1/` was an incomplete development output. Candidate 2 and its
71 frozen inputs remain immutable history; candidate 3 adds portable negative
contract tests, exact sampled-image validation and an unmatched FName.number
numeric case alongside association/index isolation. None of the synthetic
negative tests claims original-runtime success. Actual-owner source eligibility,
game/render-thread attachment, RHI
sampling controls, RootLayout construction and original visual parity remain
separate runtime gates.
