# Evidence trail and assumptions (effort review)

Reviewer: Claude (Sonnet 5.5), 2026-11-02. Read-only on everything outside `effort_review/`.
All paths are relative to `FortniteClient-12.60-CL-13477524/porting/` unless noted. Nothing was copied from generated or private data; counts below were re-derived with small read-only scripts.

## 0. Provenance and staleness

- `effort_review/inventory.json` was captured at HEAD `5e939e5`. Live HEAD is now `dcc14c3` (two later commits: `5871766` "Prepare Claude effort review workspace and remove duplicate startup accounting", `dcc14c3` GetCurrentInputType accessor). `git status` shows `progress/components.json` clean, so the live model is the committed 1,855-point model; the 1,933 proposal exists only in `effort_review/proposals/ui-effort-model.json`. I did not rebuild the inventory, to keep the original capture comparable.
- Consequence: the inventory's `platform.wine_launch` rationale ("six ledger checks") is stale. Live `progress/components.json:63-73` has five checks, and `wine_starts` appears only in `platform.eac_removal`. The startup dedup is therefore applied (`5871766`), and I confirmed it.
- `ui_audio_census.md:51-54` (dated 2026-10-31) says CommonUI is "absent from the engine tree" and FortniteUI has "no recovered source". The first is still useful: CommonUI has to be reconstructed rather than copied from UE. The second is stale, since `native/Source/FortniteUI/Generated` now holds 2,259 headers.

## 1. Method

1. Read every leaf's weight, measure kind and rationale from the inventory (117 leaves).
2. Pulled scale proxies from `progress/totals.json` and the tracked tree (class counts, UFUNCTION/UPROPERTY counts per generated module, native function counts, LOC per work area).
3. Compared adjacent allocations for overlap by (a) shared ledger check IDs, (b) overlapping measure specs, (c) overlapping rationale text.
4. Verified the UI claims against commits `04822b2` and `dcc14c3` and the `HANDOFF.md` resume point.
5. Judged weights by *total effort in the model's unit* ("roughly engineer-weeks", `components.json` `_about`). Importance, urgency and target percentage were excluded.

No leaf was independently re-sized by building it. Weights are therefore compared on relative scale proxies and decomposition judgment, not on measured hours. The tier column in `reports/leaf_coverage.md` separates leaves I examined against source (A) from leaves reviewed only by rationale and measure (B).

## 2. Scale proxies (observed this session)

| Proxy | Value | Source |
|---|---|---|
| Total model | 117 leaves, 1,855 pts; UI 170 (9.2%) | `inventory.json` committed_head |
| Named classes: FortniteGame / FortniteUI / CommonUI / UMG | 2,842 / 1,681 / 58 / 110 | `progress/totals.json` `uht_classes_named_by_package` |
| Generated headers: FortniteGame / FortniteUI / CommonUI | 5,535 / 2,259 / 103 | `ls native/Source/<mod>/Generated/*.h` |
| UFUNCTION lines in generated headers: FortniteGame / FortniteUI / CommonUI | 1,780 / 37 / 25 | `grep -c UFUNCTION` over the same headers |
| UPROPERTY lines: FortniteGame / FortniteUI / CommonUI | 26,839 / 7,602 / 275 | same |
| Native functions with an exec address | 20,574; 251 ported (1.2%), 188 skipped | `totals.json native_functions_with_exec`; `behavior/COUNTS.txt:1-2` |
| WidgetBlueprints in registry | 2,175; 300 recovered packages are widget-related, 20 export a widget class (as of 2026-10-31) | `ui_audio_census.md` scope bullets |
| Cooked names: materials / textures / SoundWave / SM_ / SK_ / NS_ | 24,995 / 30,554 / 106,362 / 4,727 / 1,719 / 173 | `totals.json` |
| LOC (tracked files): server / frontend / behavior / material_pipeline | 9.7k / 14.1k / 10.7k / 15.9k | `git ls-files <dir> \| xargs wc -l` |
| Repo age | 409 commits over 2026-10-30..11-02 (85/84/227/13 per day) | `git log` |

Interpretation limits:

- The UFUNCTION grep counts declarations in generated headers only. Native, non-reflected functions are not visible here, so it is a rough proxy for "how much function logic a module owns", not a count.
- The commit cadence reflects agent throughput. It says nothing about engineer-weeks and was not used to set any weight, consistent with `README.md` ("not the number of hours recently spent").
- `generated/native_funcs.json` is not present in this checkout, so the per-module split of the 20,574 native functions could not be computed. Unknown U-02.

## 3. Findings behind each recommendation

### EFF-001 shared ledger credit (wine_launch / original_join)
`progress/components.json:72-73` (platform.wine_launch checks) and `:816-817` (net.original_join checks) list the identical IDs `wine_entitlement_passed` and `wine_lobby_reached`. One recorded result credits both leaves (25 and 40 pts). `net.original_join`'s own rationale says the Hello/Login/Welcome/Join stages have not been verified, so its deliverable (original client joins the reconstructed server) is not what those two checks measure.

### EFF-002..006 proposed UI raises
- Deliverable boundaries: `components.json:1283` (CommonUI impl) lists "Panel and operation queue"; `:1717` (lifecycle) lists "operation queue and native helpers". Both proposed raises (+30, +8) therefore may fund the same queue.
- FortniteUI is declaration-heavy and function-light: 1,681 classes (59% of FortniteGame's 2,842) but 37 vs 1,780 UFUNCTION declarations. `gameplay.reflection_exact.implementation` (FortniteGame) is 45; the proposal would give FortniteUI 44. Function bodies are already funded by `gameplay.native_behavior` (150), and widget layout and behavior live in Blueprint widgets (`ui.frontend_lobby`, `ui.hud`, `assets.blueprint_load`).
- HUD +10: the proposal's only rationale for HUD is unchanged text ("No HUD is reproduced"). The recent evidence (`04822b2`, `dcc14c3`, `a93a1c0`) is CommonButton, input and startup work, so no new information bears on HUD scope.
- Checkpoint `04822b2` verified: touches `UCommonButton.cpp/.h`, `MainButtonStateProbe.h`, `common_panel_runtime/policy.py` and the evidence json (11 files, +284/-3). `HANDOFF.md:26-30` says both builds pass, the probe checks 7 defaults on CDO and fresh widget, 8 diagnostics, 421 table rows and both joins pass. `dcc14c3` touches the generator and adds `CommonInputCurrentTypeProbe.cpp`; `HANDOFF.md:12-17` says UE builds/runtime are pending, so it is correctly **not** a validated checkpoint. I found no claim in the dashboard model that overstates either.
- What the evidence does show is forward cost not captured in the 18-pt budget: unresolved `CommonButton.GetCurrentTextStyleClass`, `CommonActivatableWidgetQueue.RebuildWidget`, 22 skipped imports, Root attach crash at `a93a1c0`, and routing/dispatch still open (`HANDOFF.md`). That supports raising CommonUI impl, but not by the proposed 30 points without a decomposition.

### EFF-007/008 gameplay allocation boundaries
- Seven census leaves (framework 60, building 50, inventory 45, abilities 30, ai_nav 30, world_volumes 30, other 20 = 265 pts) have an empty `rationale` (`components.json:455`, `:490`, etc.) and a measure that counts class-stub presence at 0.2 credit.
- Function-body effort is named in `gameplay.native_behavior` (150), `gameplay.reflection_exact.implementation` ("module behavior"), `weapons.implementation` (48), `storm.implementation` (12), `battle_bus.implementation` (20). `progress/README.md:120-127` states the convention (bodies in native_behavior; feature budgets cover adapters/integration), but only the UI leaves quote it in their rationale.

### EFF-009 undersized placeholders
`platform.settings`, `platform.localization`, `platform.accessibility_options` are 1 pt each with the generic text "Inventory, implement and verify original client functionality in this area" (`components.json:121-141`). `platform.accessibility` is 2. Those are the full settings UI and storage, localization, and input remap/accessibility. Existing classes hint at scope (e.g. `SubtitlesWidgets` 4 classes, `EngineSettings` 7 in `totals.json`).

### EFF-010 asset coverage overlap
`assets.package_coverage` (`components.json:181`) counts every `.uasset` under `generated/extracted` and `native/Content` against 450,951 entries. `assets.static_meshes` (`:209`) and `assets.skeletal_anim` (`:243`) count `^SM_`/`^SK_` `.uasset` files under `native/Content`, which are also in package_coverage's numerator. One file is credited up to twice. This is a progress-measurement overlap; the effort overlap is that meshes sit inside the 50-pt "all types" scope as well as in their own 30/35.

### EFF-011 verified work not visible in the ledger
Commits `04822b2` (CommonButton defaults), `29a2c24`/`428a402` (421 input rows), `ea68ee4`/`78c5b61` (startup classes, Athena controller default) are cited in `HANDOFF.md` and the m1 milestone text, but no ledger leaf in `components.json` records them; the host leaves (`:1276`, `:1710`) are manual at pct 0. The existing pattern (`record_frontend_*_runtime.py`, 1-pt leaves carved from the budget) is the right shape. `dcc14c3` GetCurrentInputType is explicitly not UE validated (`HANDOFF.md:12-17`) and gets no credit.

### EFF-012 possible omissions
- 3D-world materials (24,995 name-matched) and textures (30,554) have no sized leaf; `assets.other_content` is 3 and `platform.rendering` is 20 (30% manual). Frontend materials alone have 20 points (`ui.material_*`), which covers 142 owners and 44 DXBC to SPIR-V shaders.
- `visuals.vfx` (20) counts Niagara only (173 `NS_`); its rationale states Cascade is not counted.

## 4. Unknowns

| ID | Unknown | Effect |
|---|---|---|
| U-01 | No engineer-hour calibration exists; unit is "roughly engineer-weeks" but throughput is agent-driven. | All ranges are relative-scale judgment. |
| U-02 | Per-module split of the 20,574 native functions is not available locally. | Cannot size the UI share of `native_behavior`. |
| U-03 | Whether Save the World, Creative/Playground, other worlds (3,694 maps) are in scope. | Drives `maps.other_worlds` (12) and `gameplay.shipped_features` (15). |
| U-04 | Frontend widget Blueprint count that actually needs logic; only 20 widget classes recovered. | Drives `ui.frontend_lobby` / `ui.hud`. |
| U-05 | Whether `native_behavior` bodies include non-UI CommonUI/FortniteUI functions. | Overlap size with `ui.*.implementation`. |
| U-06 | Receipts under `/mnt/vibrato/fortnite-evidence` and `/mnt/vibrato/fortnite-work` were not opened. | UI claims verified from tracked commit contents and HANDOFF only, not from the frozen archives. |
