# Original Google login-icon material

The source owner is `/Game/UI/Login/Art/M_TempIcon_Login_Google`.
Its exact named texture parameter is **Texture**, number0/global2/index-1;
its original indexed default is `T_TempIcon_Login_Google`. The original graph,
all19 shader records and seven Slate programs remain intact. The80-byte uniform
layout has resources48/56/64/72, with the original asset sampler at resource1.

The source512-square sRGB BC1 texture has ten mips: three external bulk payloads
and seven inline payloads. The framer verifies original file hashes, signed bulk
offsets and all ten compressed payloads. Final logical2-square/1-square mips
retain physical4-square blocks. No substituted texture or recook is involved.

`export_google_icon_ingress.py` rereads the original material through the frozen
inspector and checks the complete compiled expressions/source shader mappings.
`google_frame.py` independently frames the original compressed source files.
The runtime verifier checks actual named/indexed lookup, full residency, sampler
binding, original-source versus exact-copy draws and a controlled BC1 checker.
Default checker effect is observational; the combined numeric profile requires
positive effect. Valid frames are retained before sampling-policy rejection.
There is no raw compressed-GPU readback or original-client pixel-parity claim.

Final-path source replays `google-promotion-2-a` and `google-promotion-2-b` match:

- Bundle SHA-1: `7e127c308659a4a87be659d9d17829ceb691a4dc`.
- Bundle SHA-256: `3697ae77e1e20dde264403d0a5559e8ebfc2ea85ba0f3d872fa65ad33bdf4a44`.
- Runtime fixture SHA-256: `21a60f0dc74358cc8801bd16875af8a4b1c2e47ee7355e7533476c1e6fc87325`.

Producer/verifier location discovery is now relative to the maintained script.
The changed exporter provenance requires the new bundle pin; the freshly
regenerated runtime fixture remains byte-identical to the reviewed candidate.
All8 producer tests and10 verifier tests pass at the final paths. The manual
Probe patch remains separate pending its coordinated Native phase; tools and
source replays alone establish no runtime material acceptance.

```sh
python3 -I porting/material_pipeline/export_google_icon_ingress.py \
  --output porting/generated/material_pipeline/google-fresh/bundle.json
python3 -I -m unittest discover -s porting/material_pipeline -p 'test_export_google_icon_ingress.py'
python3 -I -m unittest discover -s porting/material_pipeline -p 'test_verify_google_icon_runtime_artifacts.py'
```

The producer requires a new output directory. Original assets, inspector and
shader evidence remain external inputs. Frozen promotion receipt:
`/tmp/google-tool-promotion-2/receipt.json`, SHA-256
`39d7b621151f6e2fad5c45e60c10d508c75df1a44491bdae6b4976050d0ba7db`;
independent root audit is
`generated/material_pipeline/google-root-review-1/promoted-source-root-audit-2.json`,
SHA-256 `ad704fde1ac3bf8905beffd688eb0240795b1ed7d46d405de64f9692ad216a1b`.
