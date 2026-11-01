# ComicBubble ingress exporter

`export_comicbubble_ingress.py` is bounded to `M_UI_ComicBubble` and
`M_UI_ComicBubble_Inst`. It emits either a source checkpoint or two generic v2
material ingress bundles with a v1 manifest. Bundle conversion is offline;
actual owner attachment, resource creation, rendering and pixel parity remain
unverified.

The exporter retains each original resource and its 19 shader wrappers, all
seven original Slate records, complete numeric graphs, six serialized instance
numeric overrides, and the two-entry reference array on both owners. The parent
uniform order is Tail index 1/source 2 then Clouds index 0/source 0. The instance
has one Clouds uniform, no texture override, and still inherits both references.
All 20 original DXT1 mip payloads retain their original logical and stored
sizes, signed bulk offsets and byte hashes. No resizing or recooking occurs.

The bundle path revalidates the exact 14 original wrapper/DXBC/SPIR-V chains,
tool identities, SPIR-V validation, fresh reflection and per-shader View GameTime
access evidence. Material resource-table entries are distinct from the loose
ElementTexture maps: every parent pixel shader has texture resources 0/2 and
sampler resources 3/5; every instance pixel shader has texture 0 and sampler 1.
Font permutations also have loose ElementTexture descriptors. The per-owner
Material layouts are respectively 896 and 368 bytes.

Source texture metadata is retained with its evidence limits. Effective sampler
and sRGB fields are deliberately unresolved in the texture proof objects. The
framing inspector's defaults do not prove effective runtime sampling. Root's
separate sampler investigation may provide additional evidence; this exporter
does not silently promote inspector defaults into established engine behavior.

Local prerequisites include the original extraction, texture inventory,
parent context proof 4, instance context proof 1, mapped shader directory
`porting/generated/material_pipeline/comicbubble-shaders-1`, original View
metadata under `view-identity`, and the pinned texture framing receipt initially
at `/tmp/comicbubble-texture-framing-1/receipt.json`. The framing receipt is a
scratch dependency, not a checked-in fixture. `--frame-receipt` can relocate its
exact bytes; referenced source paths must remain accessible. Context proof input
hashes and texture package hashes are independently rechecked.

Run from the repository root, choosing a fresh generated output directory:

```sh
python3 -I porting/material_pipeline/export_comicbubble_ingress.py \
  --bundles \
  --frame-receipt /tmp/comicbubble-texture-framing-1/receipt.json \
  --shader-directory porting/generated/material_pipeline/comicbubble-shaders-1 \
  --output porting/generated/material_pipeline/comicbubble-ingress-3
python3 -I porting/material_pipeline/test_export_comicbubble_ingress.py
```

The reviewed candidate output is
`porting/generated/material_pipeline/comicbubble-ingress-3/manifest.json` plus
`M_UI_ComicBubble.json` and `M_UI_ComicBubble_Inst.json`. The manifest pins both
bundle hashes and marks `requires_view=true`. Graph-default uniform cases are
separate from actual owner cases; unmatched parameter identity perturbations
retain the proper baseline.

Without `--bundles`, `--output` names a single source-checkpoint JSON file. That
schema explicitly has `runtime_ready=false`; it is not accepted as a runtime
manifest. No native code, engine code, shared generated headers or source
promotion is performed by either mode.
