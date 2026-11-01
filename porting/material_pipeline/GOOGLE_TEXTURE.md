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
All8 producer tests and10 verifier tests pass at the final paths. The manual Probe patch was applied as `eefffa9`; its first coordinated runtime
phase failed before provider registration, as recorded below. Tools and source
replays alone establish no runtime material acceptance.

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

## Native runtime phase 1 (failed, source released)

Both fresh builds passed at Native `eefffa9` / Engine `d7e04d4b6`
(client153.04s/server130.36s). Fresh UHT/generated CPP/object/linked binary/debug
provenance passed for both targets. Google defaults then exited1 with
`Legacy material ingress: invalid pinned configuration or manifest`.
The pinned v4 bundle omits the mandatory top-level `parent_path` string;
the existing general `LoadBundle` gate requires its explicit empty value for
this base material. This is a source-contract diagnosis, not shader acceptance.
No provider registration, texture receipts, payloads or failure frames were
produced. Combined, strict runtime verification and all dependent regressions
were stopped. No input was corrected or rerun inside the frozen phase.

Failure checkpoint:
`generated/material_pipeline/frontend-google-runtime-failure-checkpoint-results-1.json`,
SHA-256 `e99774d30ab116dae528b2993bfb4134f5b84a57616dd7d8b93956a121b96f1f`.
It binds202 committed source snapshots,18 binary rows/six immutable MNT binaries,
1,126 runtime inputs and1,099 artifacts. The normal Saved runtime log is retained
as `generated/frontend-native-evidence/google-runtime-1/google-defaults.normal-runtime.log`;
`failure-diagnosis.json` records the exact source gate. Separate explicit release:
`/mnt/vibrato/fortnite-evidence/frozen-runtime/google-frozen-runtime-1/source-release.json`,
SHA-256 `86c56c27e8923109978d4bb6525516e15a228cc3b6a73d1ff7299895e47b37c0`.
All jobs ended. Prior phase3 remains the latest fully green runtime. No new owner
or product milestone credit is established.

All five original material/texture files matched their normal staged copies
byte-for-byte before launch. Original directory:
`/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Login/Art/`;
stage directory:
`generated/frontend-probes/paper2d-matched-stage-3/LinuxNoEditor/FortniteLinux/Content/UI/Login/Art/`.
Receipt `generated/frontend-native-evidence/google-runtime-1/google-original-staged-assets.json`
has SHA-256 `97a328955e1c8e08137dd2fa2eae654eda8489e007bafef8564bc4bcd2f48aa9`.

| File | Bytes | Original and staged SHA-256 |
| --- | ---: | --- |
| M_TempIcon_Login_Google.uasset | 2195 | 0607ac56a1005df98e58dfc5c9a36b65d0ce12cfe59bd6fec11d1333ebae1176 |
| M_TempIcon_Login_Google.uexp | 10833 | 8494d6dbcc82f757d641f910072773cf8a2394516f3d1d7c068e2b9d26d6048b |
| T_TempIcon_Login_Google.uasset | 730 | dd5b879570082e8a9ce2b5422bce4defab5cb4ce33fc1504c8adaf8d05928077 |
| T_TempIcon_Login_Google.uexp | 3310 | d8ccb6760cb477ccca194f1e33596a6b4f9298fabd8ec7e8d8acb7750c5ea4d4 |
| T_TempIcon_Login_Google.ubulk | 172032 | 7e16cf5d986d0d1fc186102733357143beeac6f401850a0e91acd0ef17d0af72 |

The original signed-offset framing and ten BC1 mip hashes remain bound by the
unchanged fixture above and by `ReadGoogleCompressedSource` in the manual Probe.
A future bounded retry must first repair and independently review the producer
contract, regenerate fresh bundle pins, and freeze a new Native phase. Do not
alter or overwrite phase1 evidence or weaken the general loader gate.
