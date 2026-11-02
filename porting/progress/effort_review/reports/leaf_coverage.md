# Coverage of all 117 leaves

Generated from `inventory.json` (captured at HEAD `5e939e5`) plus the review verdicts. Tier A = examined against source, commit contents or sibling leaves in this review. Tier B = reviewed by rationale, measure and denominator only; "no change" for a B leaf means no defect was visible, not that the weight was validated. Weights are the committed values; the five proposed UI changes are in `report.md`.

| Leaf | Cat | Wt | Measure | Tier | Verdict | Recs |
|---|---|---:|---|:-:|---|---|
| `platform.linux_build` | platform | 20 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `platform.wine_launch` | platform | 25 | ledger | A | No weight change; measurement overlap | EFF-001 |
| `platform.eac_removal` | platform | 5 | ledger | A | No change; wine_starts dedup confirmed applied (5871766) |  |
| `platform.launch_security` | platform | 10 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `platform.rendering` | platform | 20 | manual | B | Scope question (world materials/shaders) | EFF-012 |
| `platform.accessibility` | platform | 2 | manual | B | Raise 2 -> 5 (3-8) | EFF-009 |
| `platform.settings` | platform | 1 | manual | B | Raise 1 -> 6 (3-10) | EFF-009 |
| `platform.localization` | platform | 1 | manual | B | Raise 1 -> 5 (2-8) | EFF-009 |
| `platform.accessibility_options` | platform | 1 | manual | B | Raise 1 -> 4 (2-6) | EFF-009 |
| `assets.pak_index` | assets | 10 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.map_headers` | assets | 8 | jsonl_rows | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.package_coverage` | assets | 50 | local_glob | A | No weight change; exclude SM_/SK_ from measure | EFF-010 |
| `assets.pak_extraction` | assets | 15 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.static_meshes` | assets | 30 | local_glob | A | No weight change; measure overlap | EFF-010 |
| `assets.landscape` | assets | 25 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.skeletal_anim` | assets | 35 | local_glob | A | No weight change; measure overlap | EFF-010 |
| `assets.blueprint_load` | assets | 30 | fraction | B | Boundary vs ui.frontend_lobby | EFF-006 |
| `assets.cinematics` | assets | 5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.plugins` | assets | 4 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.plugin_inventory` | assets | 1 | tracked_regex | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.other_content` | assets | 3 | manual | B | Scope question (textures, materials) | EFF-012 |
| `assets.frontend_bundle` | assets | 2 | ledger | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `assets.reflection_recovery` | assets | 15 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.enter_play` | maps | 30 | fraction | B | Scope question (world set) | Q-01 |
| `maps.apollo_terrain` | maps | 15.75 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.apollo_terrain.implementation` | maps | 29.25 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.background` | maps | 7.5 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.background.implementation` | maps | 17.5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.sky` | maps | 20 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.streamed_levels` | maps | 28 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `maps.other_worlds` | maps | 12 | manual | B | Scope question (modes/worlds) | Q-01 |
| `maps.pois` | maps | 30 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `gameplay.framework` | gameplay | 60 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.weapons` | gameplay | 12 | tracked_count | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `gameplay.weapons.implementation` | gameplay | 48 | manual | B | Boundary text only | EFF-008 |
| `gameplay.building` | gameplay | 50 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.inventory` | gameplay | 45 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.storm` | gameplay | 3 | tracked_count | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `gameplay.storm.implementation` | gameplay | 12 | manual | B | Boundary text only | EFF-008 |
| `gameplay.battle_bus` | gameplay | 5 | tracked_count | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `gameplay.battle_bus.implementation` | gameplay | 20 | manual | B | Boundary text only | EFF-008 |
| `gameplay.abilities` | gameplay | 30 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.ai_nav` | gameplay | 30 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.world_volumes` | gameplay | 30 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.other` | gameplay | 20 | census | A | No numeric change; add rationale/boundary text | EFF-007 |
| `gameplay.unknown_inventory` | gameplay | 4 | manual | B | Boundary question | Q-02 |
| `gameplay.end_to_end_parity` | gameplay | 6 | manual | B | Boundary question | Q-02 |
| `gameplay.blueprint_logic` | gameplay | 35 | manual | B | Boundary vs ui.frontend_lobby | EFF-006 |
| `gameplay.shipped_features` | gameplay | 15 | manual | B | Scope question (modes/vehicles/STW) | Q-01 |
| `gameplay.authority` | gameplay | 10 | manual | B | Boundary question | Q-02 |
| `gameplay.reflection_exact` | gameplay | 15 | tracked_count | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `gameplay.reflection_exact.implementation` | gameplay | 45 | manual | A | No numeric change; define boundary | EFF-008 |
| `gameplay.native_behavior` | gameplay | 150 | tracked_regex | A | No numeric change; define boundary and sample | EFF-008 |
| `net.version_check` | net | 10 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.magic_header` | net | 8 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.packet_handlers` | net | 12 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.encryption_token` | net | 12 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.native_join` | net | 20 | ledger | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.original_join` | net | 40 | ledger | A | No weight change; own checks needed | EFF-001 |
| `net.replication_layout` | net | 30 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.serializers` | net | 12 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.relevancy` | net | 8 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.checksum_parity` | net | 20 | tracked_regex | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `net.replication_graph` | net | 10 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.version_lightswitch` | backend | 7.5 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.version_lightswitch.behavior` | backend | 7.5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.oauth_account` | backend | 12.5 | routes | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.oauth_account.behavior` | backend | 12.5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.matchmaking` | backend | 20 | routes | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.matchmaking.behavior` | backend | 20 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.game_profile` | backend | 15 | routes | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.game_profile.behavior` | backend | 7 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.progression` | backend | 4 | manual | B | Boundary question | Q-02 |
| `backend.local_tests` | backend | 4 | ledger | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.public_catalog` | backend | 12.5 | routes | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.public_catalog.behavior` | backend | 12.5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.party_xmpp` | backend | 15 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.party_xmpp.behavior` | backend | 15 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.session_beacon` | backend | 6 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.session_beacon.implementation` | backend | 24 | manual | B | Possible overlap with net.* | Q-03 |
| `backend.stats_cloud` | backend | 10 | checks | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `backend.stats_cloud.behavior` | backend | 10 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `audio.sound_classes` | audio | 15 | fraction | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `audio.playback` | audio | 20 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `audio.original_comparison` | audio | 5 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `audio.cues_music` | audio | 25 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `audio.voice` | audio | 10 | routes | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `visuals.lighting` | visuals | 20 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `visuals.postprocess` | visuals | 12 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `visuals.original_comparison` | visuals | 8 | manual | B | No change recommended (rationale and measure reviewed; not independently sized) |  |
| `visuals.vfx` | visuals | 20 | local_glob | B | Scope question (Cascade omitted) | EFF-012 |
| `ui.commonui_umg` | ui | 6 | tracked_count | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.commonui_umg.implementation` | ui | 18 | manual | A | Raise 18 -> 32 (22-44); reject 48 | EFF-002, EFF-003, EFF-011 |
| `ui.commonui_panel_outro_runtime` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.commonui_panel_intro_runtime` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.commonui_button_internal_runtime` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.commonui_activation_runtime` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_original_maps` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_shader_translation` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_uniforms` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_detached_draw` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_locker_ingress` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_named_ingress` | ui | 6 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_frontend_closure` | ui | 2 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_widget_integration` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.material_visual_parity` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.frontend_lobby` | ui | 25 | manual | A | Accept 40 (30-60), conditional on boundary | EFF-006 |
| `ui.frontend_event_registrations` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.lifecycle` | ui | 12 | manual | A | Hold 12 (10-18); reject +8 until queue owner set | EFF-003, EFF-011 |
| `ui.hud` | ui | 40 | manual | A | Hold 40 (30-65); +10 unsupported | EFF-005 |
| `ui.fortniteui_classes` | ui | 10 | tracked_count | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.fortniteui_classes.implementation` | ui | 29 | manual | A | Hold 29 (24-38); reject 44 | EFF-004 |
| `ui.confirmation_runtime` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.context_source_stage` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.context_runtime_stage` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.panel_source_stage` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
| `ui.input_source_stage` | ui | 1 | ledger | A | No change; bounded 1-2 pt allocation carved from a broad budget, consistent with progress/README.md |  |
