# Overlaps and missing leaves — second pass (all categories)

Follow-up to `report.md`, 2026-11-02. This pass is deliberately not UI-centred. Recommendation IDs refer to `suggestions/recommendations.json`; open questions to `suggestions/questions.json`. Nothing here is applied.

## Correction to the first report: attention was uneven

Yes, the first pass was shaped by the brief. The README asked for special attention to the UI proposal, and I spent my source-level effort there: 41 leaves got tier A scrutiny (all 26 UI leaves, plus 15 elsewhere that I touched while chasing overlaps) and 76 got only a rationale-and-measure read. My weight recommendations were +29 UI and +14 platform, and **zero elsewhere**. That zero reflects "I could not size it", not "the weight is right". Two consequences:

- The UI got the only gap analysis in the first pass, so only UI leaves had a chance to be raised. Non-UI categories had no equivalent chance, which biases the *relative* allocation toward UI by omission. In this pass I found plausible candidate additions outside the UI (EFF-018, 019, 020, 012) that are comparable in size to the +29 I recommended.
- I did reduce the UI proposal (78 -> 29), but that reduction came from the same lopsided method. Treat the +29 as provisional until the items below are decided together.

## Confirmed overlaps (measurement; fix without moving weights)

| Rec | Leaves | Overlap | How verified |
|---|---|---|---|
| EFF-001 | `platform.wine_launch`, `net.original_join` | Same two ledger IDs credit both (65 pts). Latent: both show 0% at `dcc14c3`. | Compared check lists in `components.json:72-73, 816-817`; checked history entry |
| EFF-010 | `assets.package_coverage`, `static_meshes`, `skeletal_anim` | Same `native/Content` `.uasset` files counted up to 3 times | Read the three measure specs |
| EFF-015 | `backend.game_profile`, `audio.voice` | Two voice routes selected by both | Replayed the route measure over `generated/routes.json` (2 of 59 double-selected, 0 unowned) |
| EFF-013 (census part) | `gameplay.world_volumes` vs `maps.pois`, landscape leaves | Census regex `Water|Volume|Poi|POI|Manager|WorldTile|Landscape|Tile` names the same subject as the map leaves | Read regex at `components.json:585` |

## Probable overlaps (effort scope; boundary text needed before any weight move)

| Rec | Leaves (weights) | Why it looks like double funding | Size of possible overlap |
|---|---|---|---|
| EFF-003 | `ui.lifecycle` (12), `ui.commonui_umg.implementation` (18) | Both claim the operation queue | 5-10 of the proposal |
| EFF-008 | `native_behavior` (150), `reflection_exact.implementation` (45), `weapons/storm/bus.implementation` (48/12/20) | Function-body effort named in five leaves | Unknown; needs sampling |
| EFF-013 | `assets.landscape` (25), `apollo_terrain` (45), `streamed_levels` (28), `world_volumes` (30), `pois` (30) | Terrain/landscape/level assembly in five places | 10-30 pts |
| EFF-014 | `reflection_recovery` (15), `reflection_exact` (60), `checksum_parity` (20), `replication_layout` (30), `serializers` (12) | Same recovered reflection tables; `replication_layout` text looks stale | 10-20 pts, low confidence |
| EFF-016 | `audio/visuals.original_comparison` (5/8), `end_to_end_parity` (6), `platform.rendering` (20) | Shared capture/compare harness funded in each | Small |
| EFF-017 | `launch_security`, `eac_removal`, `wine_launch`, `encryption_token`, `original_join`, `oauth_account.behavior`, `version_lightswitch.behavior` | Entitlement/anti-cheat gate described in seven leaves | Small to moderate |
| EFF-006 | `ui.frontend_lobby` (25), `gameplay.blueprint_logic` (35), `assets.blueprint_load` (30) | Widget-Blueprint logic and loading have no stated owner | 5-10 pts |
| EFF-007 | Seven census leaves (265) | Empty rationales, so no scope to compare | Unknown |

Sum of the probable overlaps is plausibly tens of points downward, but only EFF-013 and EFF-014 have a numeric estimate and both are low-to-medium confidence. I do not recommend reducing any weight until the boundaries are written.

## Candidate missing leaves

| Rec | Gap | Evidence | Plausible size | Confidence |
|---|---|---|---:|---|
| EFF-018 | Character animation and cosmetics (AnimBP, sequences, montages, emotes, customization assembly) | 2,379 AnimBlueprints, 3,691 CustomCharacterParts, 952 characters; `skeletal_anim` counts SK_ only and says animation "too few to matter" | 25-60 | low (gap is real; size is not) |
| EFF-012 | 3D-world materials, shader translation, texture conversion; Cascade VFX | 24,995 materials, 30,554 textures; frontend materials alone take 20 pts for 142 owners | 20-60 | low |
| EFF-019 | Chaos destruction, geometry collections, field systems, foliage, water, vehicle physics | About 45 + 13 + 17 + 7 named classes, no model mention | 10-30 | low |
| EFF-020 | 100-player scale/stability, replay/spectate | No mention; only mechanisms (relevancy 8, graph 10) | 10-30 | low |
| Q-01/05 | Other modes/worlds, STW, Linux cook pipeline | See `questions.json` | unknown | low |

Together these are 65-180 points of plausible missing scope, i.e. 3.5-10% of the 1,855-point model, with the largest uncertainty in how much already sits inside leaves that are named differently. They are scope candidates, not corrections. If even half are real, the UI's relative share (170 or 199 of ~2,000) falls, and the UI case for +29 should be judged against them.

## Leaves that look suspiciously small or placeholder-like

`platform.settings/localization/accessibility_options` (1 each), `platform.accessibility` (2), `gameplay.unknown_inventory` (4), `backend.progression` (4), `gameplay.end_to_end_parity` (6), `gameplay.authority` (10), `assets.other_content` (3), `assets.plugins` (4), `assets.cinematics` (5), `audio.original_comparison` (5). EFF-009 sizes only the first four. The others are either true residual placeholders or scope that is hidden elsewhere; Q-02 asks which.

## What I could not check

- Per-module split of the 20,574 native functions (`generated/native_funcs.json` absent), so no overlap between `native_behavior` and any module leaf is sized.
- Whether `maps.streamed_levels` contains the POIs and Blueprints counted elsewhere.
- Any claim of the form "this leaf is correctly sized": that would need independent decomposition per leaf, which neither pass did.
