# Original Social curve-atlas defaults

The original `CurveLinearColorAtlas` constructor is reached by class getter
`0x1446ef940 -> 0x1446f0b70`, callback `0x1446f0e00`, constructor
`0x1441589a0`, and the UTexture2D/UTexture constructor chain. Original
reflection and enum records bind its final stores to sRGB false, bilinear
filtering, HDR compression, and clamp addressing on both axes. The pinned
constructor receipt is `social-atlas-defaults-proof-1/receipt.json`, SHA-256
`787cd43f8289a5ddad33905c4ef9e8c22bad628f6d1a0ee690a45d26fd0b6815`.

The independent inspector decodes the package summary, name map, sole export,
class import, and complete tagged-property stream ending in `None`. The four
tags are TextureSize, GradientCurves, ImportedSize and LightingGuid; none of
the six relevant texture properties is present. Its pinned receipt is
`social-atlas-independent-tags-1/receipt.json`, SHA-256
`c33b34e17d6fea309b682e4c15ad6cf57999fdb561ac9e1cb1f3d878b48edd94`.
The decoding is bounded to this pinned package and rejects unknown tag types.

The original reflected LODGroup field is `+0x5f`. No store to that byte was
identified in the traced constructor chain; earlier object initialization and
post-load migration are not established. The fixture retains the earlier
parser-reported `TEXTUREGROUP_World` value as an inference, not as a proven
original constructor default. The producer rejects explicit tags for all six
fields instead of treating overrides as defaults.

Reproduce the independent byte inspection from the repository root with a new
output directory:

```
python3 porting/material_pipeline/inspect_social_atlas_defaults.py --repo "$PWD" --output /tmp/social-atlas-inspection-new
```

The inspector verifies the original image hash, each pinned source/output and
raw window, constructor stores, reflected fields, enum mappings, and package
tags. The producer repeats these checks before writing a fixture. This is
source evidence; no original-client execution, runtime acceptance, or visual
parity is asserted.
