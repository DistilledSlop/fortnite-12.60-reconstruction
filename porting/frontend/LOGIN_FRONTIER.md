# Login screen load frontier (2026-11-08)

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
