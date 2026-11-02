# Effort-weight audit — final report

Reviewer: Claude (Sonnet 5.5), 2026-11-02. Scope: all 117 dashboard leaves, committed model (HEAD `dcc14c3`, inventory captured at `5e939e5`, 1,855 pts) and the unapplied UI proposal (1,933 pts). Nothing outside `effort_review/` was modified.

Weights below mean total effort (completed plus remaining) in the model's unit, "roughly engineer-weeks". Importance, urgency and target percentage were not used. Evidence, measured progress and weight are kept apart. Detail and line citations are in `research/evidence_notes.md` and `suggestions/recommendations.json`.

## Bottom line

1. **Do not apply the 78-point UI proposal as written.** Directionally part of it is right (CommonUI is under-sized), but only about 29 of the 78 points survive review, and none of the five raises comes with a decomposition.
2. **Two real defects are independent of the weights:** two ledger checks credit both `platform.wine_launch` and `net.original_join` (EFF-001), and the `.uasset` files in `native/Content` credit up to three asset leaves (EFF-010).
3. **The largest uncertainty is outside the UI:** 265 points in seven unexplained census leaves and 150 points in `native_behavior` overlap in intent with several other implementation leaves (EFF-007, EFF-008). I cannot size this without the per-function split, so I propose boundary text, not numbers.
4. **Several leaves look too small** (platform settings, localization, accessibility, 1-2 pts each) and some scopes may be missing (3D-world materials, Cascade VFX, alternative modes). These need owner decisions on scope before any numbers.

## Proposed UI changes, one by one

| Leaf | Current | Proposed | My recommendation | Range | Confidence | Rec |
|---|---:|---:|---:|---|---|---|
| `ui.commonui_umg.implementation` | 18 | 48 | **32** | 22-44 | low | EFF-002 |
| `ui.frontend_lobby` | 25 | 40 | **40** (conditional) | 30-60 | low | EFF-006 |
| `ui.lifecycle` | 12 | 20 | **12** | 10-18 | medium | EFF-003 |
| `ui.hud` | 40 | 50 | **40** | 30-65 | low | EFF-005 |
| `ui.fortniteui_classes.implementation` | 29 | 44 | **29** | 24-38 | low | EFF-004 |
| **UI total** | 170 | 248 | **199** | | | |

Why:

- **CommonUI up, but less.** The plugin is not in the engine tree, so it must be rebuilt from decompilation, and the recent evidence shows cost beyond the 18 points: the Root attach crash in `IconTextButton.UpdateTextStyle` (`a93a1c0`), `CommonButton.GetCurrentTextStyleClass` and `CommonActivatableWidgetQueue.RebuildWidget` still missing, 22 skipped imports, input routing open. My count of about eight clusters at 3-5 engineer-weeks each gives 22-44, so 48 is above the range.
- **Queue is funded twice.** `components.json:1283` and `:1717` both claim the operation queue; raising both would double-count it. Give it to CommonUI, leave lifecycle at 12.
- **FortniteUI is function-light.** 1,681 classes (59% of FortniteGame) but 37 vs 1,780 UFUNCTION declarations in the generated headers. The proposal would size FortniteUI behavior (44) almost equal to FortniteGame's reflection implementation (45), while function bodies are already funded by `native_behavior` and widget behavior by frontend and HUD. This is a proxy, not a count (see U-02).
- **HUD +10 has no evidence behind it.** Recent UI work is CommonButton, input and startup. The HUD may well exceed 40, but a raise needs a component list.
- **Frontend/lobby +15 is plausible** (menus, lobby, party, locker, shop, matchmaking UI), but its boundary with `gameplay.blueprint_logic` (35) and `assets.blueprint_load` (30) is undefined.

Native checkpoint claims checked: `04822b2` is supported by its commit contents (11 files, +284/-3) and `HANDOFF.md:26-30`; `dcc14c3` is correctly described as an unvalidated candidate (`HANDOFF.md:12-17`: UE builds and runtime pending). The model's milestone text does not overstate either. I did not open the frozen receipts under `/mnt/vibrato` (U-06).

## Other consequential findings

| Rec | Summary | Numeric effect |
|---|---|---|
| EFF-001 | `wine_entitlement_passed` and `wine_lobby_reached` credit both `platform.wine_launch` (25) and `net.original_join` (40); original_join has no Hello/Login/Welcome/Join check | None to weights; fixes overstated progress |
| EFF-007 | Seven census leaves (265 pts) have empty rationales | None; text only |
| EFF-008 | Function-body effort named in five leaves (150/45/48/12/20) | Unknown, needs sampling |
| EFF-009 | Settings, localization, accessibility options and accessibility total 5 pts | +14 (3-10, 2-8, 2-6, 3-8) |
| EFF-010 | `package_coverage`, `static_meshes`, `skeletal_anim` credit the same files | None to weights |
| EFF-011 | Verified `04822b2`-era work (CommonButton defaults, 421 input rows, startup classes) has no ledger leaf; broad manual leaves stay at 0% | Carve 3 leaves of 1-2 pts from existing budgets |
| EFF-012 | No sized leaf for 3D-world materials/shaders/textures; VFX counts Niagara only | 20-60 pts possible, scope decision first |

Open questions (low confidence, not actionable): scope of Save the World and other modes/worlds (Q-01), whether 10/6/4/4-point residual leaves are integration-only (Q-02), beacon overlap with net (Q-03), definition of the effort unit under agent throughput (Q-04).

## Combined model impact

| Scenario | Total | UI | Platform | Overall at unchanged evidence |
|---|---:|---:|---:|---|
| Committed | 1,855 | 170 | 85 | 21.458% (`progress/README.md:117`) |
| Proposal as saved | 1,933 | 248 | 85 | about 20.592% (same line) |
| This review (EFF-002, 006, 009; others no weight change) | 1,898 | 199 | 99 | about 20.97% (my arithmetic: 21.458 x 1,855/1,898, new weight earning zero credit) |

The last figure is arithmetic, not a dashboard run (I did not execute `progress.py`, which writes outside this workspace). Any applied change makes later history points estimates under a different model; earlier points keep their original models and must not be recomputed. EFF-001 and EFF-010 are measurement fixes whose effect is a lower, not higher, reading when the double counts are removed.

## Residual uncertainty

- No engineer-hour calibration; ranges are relative-scale judgment on proxies (class counts, UFUNCTION/UPROPERTY counts, LOC, cooked-name counts). Commit cadence (409 commits over four days) reflects agent throughput and was not used for weights.
- 76 of 117 leaves are tier B: reviewed by rationale, measure and denominator only. "No change" there means no visible defect, not validated weight (`reports/leaf_coverage.md`).
- The inventory is at `5e939e5`; live HEAD is `dcc14c3`. The only change that matters here (`wine_starts` dedup, `5871766`) was checked against live `components.json`. `ui_audio_census.md:54` is stale (FortniteUI now has 2,259 generated headers).
- Per-module native function counts were unavailable (`generated/native_funcs.json` absent), which limits EFF-004 and EFF-008.

## Suggested order for the primary agent

1. EFF-001, EFF-010 (measurement fixes, no weights): small separate tasks.
2. EFF-003 then EFF-002 (assign the queue, then raise CommonUI to 32 with a second independent estimate).
3. EFF-007 boundary text; EFF-011 ledger carve-outs.
4. EFF-009 platform placeholders after one inventory pass.
5. Owner decisions on Q-01 and EFF-012 before any change to maps, gameplay shipped features, materials or VFX.
6. EFF-006 only with its boundary list; hold EFF-004, EFF-005 until decomposed.

## Files

- `reports/leaf_coverage.md` — all 117 leaves with tier, verdict and recommendation IDs.
- `research/evidence_notes.md` — method, scale proxies, evidence trail, unknowns U-01..U-06.
- `suggestions/recommendations.json` — 12 actionable items (schema-conformant); `suggestions/questions.json` — 4 low-confidence questions.
