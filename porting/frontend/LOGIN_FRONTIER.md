# Login screen load frontier (2026-11-08)

## Later observations (2026-11-09)

The snapshot below predates the login-art and TrackingLlama ingress work in
commits `a69ff6c8` and `9e9e7304`. The probe runner now includes bundles for
the four materials listed below; they are not the current first unconverted
login materials. Do not infer an interactive frontend from class loading.

In `generated/frontend-probes/logs/r3.log`, a diagnostic run with unconverted
legacy shader maps skipped loaded `LoginScreen_C` and created a widget with 25
tree widgets. It also reported missing native imports, missing DataTable row
structs and Blueprint calls to null imports. A separate diagnostic run,
`logs/lobby2.log`, loaded and created `AthenaLobby_C` and completed the scoped
creation/ownership probe. Neither demonstrates the normal login-to-lobby
transition, correct rendering, or a usable lobby.

The broader `logs/ai1.log` run begins loading `/Game/Maps/Frontend` with the
skip diagnostic enabled. It reports many unconverted materials, including
`SettingsDefaultSlider_M`, `M_UI_SettingBorder_Master` and
`M_UI_SettingsPanelBG`, and ends in an async-loading `RF_NeedLoad` assertion.
The nearby synchronous texture-load warning is a clue, not a proven cause;
the recorded crash stack does not reliably identify it. The diagnostic skip
itself permits missing shader maps, so this run cannot establish that normal
frontend loading works. The captured stage predates later source changes.

The numeric exporter accepted `SettingsDefaultSlider_M` and produced a bundle
with seven Slate records backed by six validated translated shaders. To add
that generated bundle to the existing diagnostic manifest for a future probe:

```sh
python3 porting/material_pipeline/merge_ingress_manifests.py \
  --output porting/generated/material_pipeline/frontend-settings-combined.json \
  porting/generated/material_pipeline/comicbubble-ingress-3/combined-manifest.json \
  porting/generated/material_pipeline/settings-numeric-discovery-20261110/manifest.json
FORTNITE_MATERIAL_MANIFEST=porting/generated/material_pipeline/frontend-settings-combined.json \
  bash porting/frontend/run_frontend_map.sh /Game/Maps/Frontend /tmp/frontend-probe.log /tmp/frontend-probe.png
```

The merger checks bundle hashes and rejects duplicate owners; the runtime
still validates resource provenance and compatibility. The merged manifest
has not been exercised in a client and does not resolve the broader map load.

Probe: `porting/frontend/run_frontend_probe.sh /Game/UI/Foundation/StateLayouts/LoginScreen.LoginScreen_C OUT.log`
(matched stage `generated/frontend-probes/paper2d-matched-stage-3`, the 2026-11-08 client with the default-on legacy
field-path and Sequencer v11 readers).

- Passes now: `SplashScreenWidget` (Sequencer v11 channels), 25+ numeric/texture material ingress bundles
  (Locker, GradientPanel, PowerToast, MenuButton, Donut, MainL, Keybind, FadedFlogo, MediaButton, Hollow progress,
  Google icon, social atlas, EpicLogo).
- Does not load yet: materials with no pinned bundle fall to the stock reader and crash. Listed with
  `-FortniteSkipUnconvertedLegacyShaderMaps` (lines `UNCONVERTED_LEGACY_MATERIAL`):
  `M_CornerBackArrow`, `M-Icon-ExternalLink-64`, `M_SkewButtonShape_DF`, `M_TempIcon_Login_Facebook`
  (the list stops at the first crash caused by the diagnostic itself; more follow).
- Each needs: an original-resource bundle (`material_pipeline/export_*_ingress.py`), its DXBC->SPIR-V shaders and texture
  report, a native handler and a pin in `LegacyMaterialShaderMapProbe.cpp`. The exporters are per-material today.
- Not reached: `RootLayout_Athena_C`, `/Game/Maps/Frontend`, any displayed menu.
