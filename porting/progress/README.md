# Port progress dashboard

A generated dashboard for the Fortnite 12.60 Linux reconstruction: a squarified treemap of
the whole effort (area = estimated effort weight, fill level = percent complete), a
per-component table, `progress.json`, and a history page with one point per commit.

**Definition of 100%:** the entire game can be launched into the menu, joined into
a game and matchmade on the reconstructed server, and played with sound and visuals
100% working. In other words, the client — the exact same client as the 12.60
original, feature-complete and unstripped, rebuilt as a UE project — reaches the
lobby, matchmakes to the reconstructed UE dedicated server (which carries every
structure and feature the original dedicated server would have) and plays a match
with all visuals and audio. The supplied 12.60 build is the completeness
reference; the original binary under Wine is an analysis oracle, not the
deliverable. See `../../README.md` for the full goal statement.

The overall figure is an effort-weighted estimate of reconstruction work. It is not
a measure of that definition, which is far from met. The page also lists four binary
milestones toward the definition in `components.json`.

The dashboard splits the overall so the subjective part is always visible. Every leaf
has a basis, on a ladder from most to least objective: `git` (computed from tracked files,
replayable per commit) > `local` (computed from git-ignored outputs, read from disk) >
`documented` (a number typed in `components.json`) > `manual` (a typed percentage).

- **Objective** (`objective_pct`): the share of the overall from `git` and `local` leaves.
  Nothing typed contributes to it.
- **Evidence-based** (`measured_pct`, the name it has always had): the share from `git`,
  `local` and `documented` leaves. It includes typed counts.
- **Manual estimates** (`estimated_pct`): the share from `manual` leaves, with
  their weight share (`manual_weight_pct`) and the overall if all of that work
  were complete (`manual_full_pct`).
- **Weight by basis** (`weight_by_basis`): each basis's share of the total effort weight,
  listed under the treemap legend. It describes the weights, not progress.

`progress.json` and every history entry carry the same fields; the history chart
plots the evidence-based series as a dashed line under the overall.

## Files

| File | Tracked | Purpose |
|------|---------|---------|
| `progress.py` | yes | Generator. Stdlib only, offline, runs in under a second for HEAD. |
| `components.json` | yes | The model: categories, components, weights, measures, rationale, milestones. |
| `totals.json` | yes | Denominators counted from local extraction data by `refresh_totals.py`. Tracked, so every commit can be replayed. |
| `evidence.json` | yes | Evidence ledger of check results written by `record_evidence.py`. Tracked. |
| `refresh_totals.py` | yes | Regenerates `totals.json` (stdlib, deterministic). |
| `record_evidence.py` | yes | Runs or judges one check and records it in `evidence.json`. |
| `serve.sh` | yes | Static LAN server for `out/` on port 8765. |
| `README.md` | yes | This file. |
| `../../.githooks/post-commit` | yes | Records a history point and regenerates `out/` after each commit. |
| `out/` | no | `index.html`, `progress.svg`, `progress.json`, `history.html`. |
| `history.json` | no | One entry per commit (commit, subject, date, overall, per-leaf %). |

## Running

```sh
python3 porting/progress/progress.py            # record HEAD and render (what the hook runs)
python3 porting/progress/progress.py --seed     # replay every commit and rebuild history.json
python3 porting/progress/progress.py --no-record # render HEAD without touching history.json
bash porting/progress/serve.sh                  # http://192.168.4.186:8765/ and localhost
```

`--seed` replays every commit and rebuilds all historical measurements. A normal
run measures HEAD and replays any missing commit points while preserving the
previously recorded values. Replayed points are marked in the history. The
post-commit hook finishes this refresh before returning; failures are logged in
`generated/progress-hook.log` and do not fail the Git commit.

## Editing estimates

Open `components.json`. Each leaf has a `weight` (its treemap area and its share of
the overall figure) and a `measure`. Weight estimates total implementation and
verification effort, completed plus remaining. Importance, urgency, and milestone
criticality do not affect weight. Define exclusive effort allocations before
reweighting overlapping scopes, and state uncertainty when estimates lack evidence.
Measures, from most objective to least:

- `tracked_count`: done = tracked files at the commit matching `globs` (and `name_regex`,
  searched in the file name without extension); total from `totals.json` (`total_key`,
  `total_sub`) or a literal `total`. `also_report` lists other groups in the evidence only.
  Basis `git`.
- `tracked_regex`: done = the integer captured by `regex` (group `group`, default 1) from one
  tracked `file`; total as above. Basis `git`.
- `ledger`: `checks` lists check ids. Done = listed checks whose latest entry in the tracked
  `evidence.json` has `result: pass`. A missing entry is not a pass. The evidence text shows
  each check's result, exit code and age in commits since it was recorded. Basis `git`.
- `checks`: a list of checks. Each is `file` (tracked at the commit), `local` (an ignored path
  under `porting/generated/`), or `pattern` plus `in` (a glob over tracked files). Optional
  `stub_credit` caps a present-but-unexercised check.
- `routes`: client route templates from `porting/generated/routes.json` whose literal path appears in
  committed `porting/server/**`. Use `include`, `exclude` and `stub_credit`.
- `census`: `/Script/FortniteGame` imports from `porting/generated/map-native-imports.jsonl` whose
  names match `match`, counted as done when a native class of that name is declared under
  `porting/native/Source`. `stub_credit` (0.2) keeps an empty declared stub from counting as
  implemented.
- `fraction`: `done` over `total`, from a documented count (basis `documented`), or from a local
  JSON file (`count_file`, `count_key`; basis `local`). Totals may come from `totals.json`.
- `jsonl_rows` and `local_glob`: counts from ignored local outputs (basis `local`). `local_glob`
  takes `sources` (`root`, `suffix`, `name_regex`) and `distinct` (count distinct names).
- `manual`: `pct`, your estimate (basis `manual`).

### Applied Claude effort review (2026-11-02)

Claude's report and research are retained in [effort_review](effort_review/README.md).
The original +78 UI proposal is not applied. The reviewed model has **2,019 relative
points and 133 leaves**: UI is 199 (formerly 170), with CommonUI/UMG 44 (formerly
30). CommonUI's behavior estimate rises from 18 to 32 before splitting out two
one-point foundations; frontend/menu flow rises from 25 to 40. FortniteUI and HUD
budgets retain their previous values. The original units are rough relative effort,
not measured hours or a conversion from agent runtime. Importance, urgency and a
desired score play no part in the revision.

Four one-point foundations are carved from residual budgets: actual CommonButton
query/defaults; original input-row/table and current-type accessor runtime; and
observed startup class selections; and engine-backed action-binding data/storage.
The last uses controlled resolvers and leaves production providers and routing open.
The data/storage point was carved from the existing CommonUI residual after the review,
leaving all category and total budgets unchanged. Their ledger entries replay pinned source and
runtime receipts. They do not complete selection/routing, interactive startup or
any product milestone. This recognizes verified work without calling an entire
manual UI leaf complete.

Seven census allocations retain their combined 265 points but now expose their
former 20% declaration share and 80% unfinished feature integration separately.
The declaration share can reach 100% of its own scope; header presence cannot
complete feature behavior. Existing contributions are preserved for the same
coverage, aside from the intentionally narrowed world-manager name heuristic.
Native-body recovery/transcription remains a separate cross-module allocation;
its manifest misses some maintained generator bodies and is not an inventory of
all recovered code. A twelve-case sample supports the ownership distinction, not
a new numeric extrapolation.

Settings, localization, accessibility options and input/control effort become
6, 5, 4 and 5 points. This is **+15**, correcting the review's +14 arithmetic.
Additional scopes required by the existing full-original-feature goal are now
explicit: world materials/textures/particle runtime 40 (range 20–60), animation
and character/cosmetic assembly 40 (25–60), physics/world-system integration 20
(10–30), full-match scale 12 (6–18), and replay/spectating 8 (4–12). These are
low-confidence scope estimates, all at 0% pending actual evidence; inventory
counts do not establish implementation. All original modes remain in scope.
Refine these estimates through component inventories rather than treating them
as calibrated engineer-weeks.

Accounting fixes separate Wine staging from original-client protocol acceptance,
remove SM_/SK_ names from general package coverage with a matching distinct-name
denominator, and exclude voice service routes from general game-profile route
coverage. Raw package recovery, conversion, loading and runtime parity remain
distinct. The implementation log records ownership for geometry/levels/managers,
reflection/replication, comparison infrastructure and launcher/network/backend
security. Shared effort is allocated once; a common receipt may support distinct
deliverables. Historical snapshots retain their original model. Changes caused
by revised weights or scope are model changes, not runtime work or lost work.
Older allocation notes below describe their checkpoint's model and denominator.

### Material pipeline accounting

The frontend allocation includes five separately evidenced material deliverables and
remaining attachment and visual parity work. The four ledger IDs are
`material_original_maps`, `material_shader_translation`, `material_uniform_evaluation`,
and `material_locker_draw`, and `material_locker_asset_ingress`: missing or failing entries earn no credit. Their source
receipts are available from the dashboard as tracked evidence documents copied from
the measured commit. The allocation moves 20 of the existing 60 frontend weight units
into material work, including 10 units for unfinished integration and two for the bounded original Locker actual-owner ingress; total project weight
is unchanged. A completed detached draw is not a working UI root or visual parity.

Coverage-only terrain, background, gameplay and CommonUI leaves are separate from
their unfinished implementation, using the earlier cap shares as explicit weights.
A partial coverage count now earns that fraction of its share instead of reaching a
plateau early. Historical recorded overall values are retained; model changes make
successive history points estimates under different models, not directly comparable.

The scope also reserves unfinished work for all shipped plugins and project modules,
worlds and modes, unclassified original features, native and UI lifecycle, content
conversion and cooking, security and entitlements, input, settings, localization,
accessibility, account progression, backend services, custom serializers, network
relevancy, audio, visuals and full original-behavior comparison. These are scope
allocations, not claims that an exhaustive feature inventory is complete. The documented original plugin count has its own small tracked share; inventory
of individual plugins, implementation, and runtime loading remain separate. Class-header coverage is likewise separated from behavior
and runtime verification (one quarter of the previous reflection leaf weights is
allocated to declarations, three quarters to unfinished implementation). These
allocation choices are judgment, not measured engineering hours. Source-reference
checks use half of their former broad backend leaf weight, with the other half for
behavior and integration; the reference count no longer claims functional progress.

### Frontend source-stage allocation (2026-11-01)

Three one-point shares were split from existing zero-valued work: one from the
14-point lifecycle bucket for the root-reviewed generic context source/generator
candidate, and two from the 24-point CommonUI behavior bucket for isolated base
activation and input dependency candidates. `record_frontend_source_stages.py`
pins and validates their frozen receipt hashes, rehashes their archived source
and original-window bindings, and records the narrowly stated
source-stage scope. The context source is now applied, but the subsequent unity
build failed; the two CommonUI candidates are off-workspace. Their isolated UHT
and syntax checks do not prove shared integration, link or runtime. Concrete
contexts, authentic startup, the Panel queue, input dispatch, interactive UI and
visual parity retain separate unfinished allocations. The fixed event-signature
and material receipts cover different work and are not counted again here.

Total weight remains 1,855. At the same HEAD and local data, this model revision
changes the estimate from 21.010% to 21.172%, of which 0.162 percentage points
are newly enumerated source work, not a measured elapsed-time gain. The one-point
shares are judgment about the bounded work relative to remaining full-game work,
not measured hours. Recorded historical points retain their original models; a
model change does not imply that work occurred at that commit. External receipts
are pinned but must remain available for independent re-verification; the
tracked ledger alone is an attestation, not the underlying evidence.

The 142-owner material closure has two points because the shader/map decoding,
uniforms, detached draw, named ingress and remaining integration/parity are
already separately allocated within the same 20-point material budget. Its small
per-owner increment should not be interpreted as the total value of the material
pipeline. Further weight changes require a total-scope effort comparison,
not the number of hours recently spent or a target percentage.

A bounded metadata audit on 2026-11-02 screened all 117 leaves (1,855 weight
units). It found no explicit importance-based weight rationale, but did not
establish that relative estimates are calibrated to effort. Potential accounting
overlaps need review, including native function bodies (150) and feature implementation.
Package coverage now excludes SM_/SK_ basenames from both its count and denominator,
and Wine oracle stages and original-client join acceptance use separate ledger checks. A shared result can support distinct
deliverables; the associated effort must be allocated only once. No numeric
changes were justified by this metadata review. Before reweighting these scopes,
define their distinct deliverables and estimate their relative total effort.

### Raw pak extraction and package coverage

- `assets.pak_extraction` (basis `local`) is a `fraction` whose `count_file` is
  `porting/generated/extract-full-summary.json` and `count_key` is `verification.treeFiles`
  (the files the verify pass found in `/mnt/vibrato/fortnite-extracted-full`). The total is
  `pak_full_distinct_paths` in `totals.json` (963,427, the summary's `uniquePaths`). The count is
  read from that small summary, so the hook walks no files. Regenerate the summary with the
  extraction's verify mode, then run `refresh_totals.py`.
- `assets.package_coverage` (basis `local`) counts distinct recovered or converted `.uasset`
  basenames excluding SM_ and SK_. Its denominator applies the same naming rule to the full
  cooked inventory, using distinct basename counts rather than paths. Naming is a heuristic;
  raw recovery also contributes to pak extraction and does not prove conversion or usability.
  The measurement change applies going forward; recorded history is not rewritten.
- `backend.game_profile` excludes voice routes, which are assigned to `audio.voice`. The latter
  measures voice service route references only; client voice, audio transport and parity remain
  unverified.

Denominators (`total_key` and optional `total_sub`) are read from `porting/progress/totals.json`.
If a key is missing the leaf shows 0 and says why. Current leaves use explicit weighted work breakdowns instead of percentage ceilings: narrow coverage and the remaining implementation are separate leaves. Historical model versions may still use `ceiling`; the generator retains support for them. Stub credit for source references and generated declarations is partial because these do not establish behavior. Groups take the weight-averaged value of their children. Set `"done": true` on a milestone only when its end-to-end acceptance evidence is recorded. Route literal matches can include tests and documentation, so they do not establish served-route coverage (see `../server/EVIDENCE.md`).

Edit, commit, and the post-commit hook regenerates the dashboard. The totals and the ledger are
measured from the commit, so commit `totals.json` and `evidence.json` in the same commit that
uses them.

### Refreshing the totals

```sh
python3 -I porting/progress/refresh_totals.py      # rewrite porting/progress/totals.json
```

Run it after the extraction outputs change (reflection tables, asset registry, the verified full
extraction summary, `porting/map_compatibility.md`). The name-prefix counts (`SM_`, `SK_`, `NS_`,
`A_`, material and texture prefixes), the distinct basename count and the sound-path count come from
a walk of the full raw extraction at `/mnt/vibrato/fortnite-extracted-full`, not from the old 28-pak
index. That walk is run once, by hand, and writes `porting/generated/cooked-name-counts-full.json`
(ignored):

```sh
python3 -I porting/progress/refresh_totals.py --count-tree   # walks ~963k files once (about 7 s)
```

The post-commit hook never walks files; a normal refresh reads only that count file and the summary,
and it fails if the count file's `.uasset` total differs from the summary. It reads only local files,
takes about three seconds, and fails (leaving the file unchanged) if a source is missing. Each entry records its `source` file, its
`method`, and a `generated_utc` taken from the source file's modification time, so unchanged data
gives identical bytes. Commit the result.

### Latest regression provenance

Native `907e1dc` has separate build and runtime receipts. Build `generated/regress/20261101T002525Z-2242896/result.json` passes the corrected incremental-build gate; `generated/regress/20261101T002339Z-2232871/result.json` passes restage and seven checks (4,617 offline classes, 251 behavior snippets, 71 backend tests). The earlier wrapper failure at `20261101T002007Z-2216551` was its mtime rule after Build.sh exit0, not a compiler failure. Engine source remains `f716e0cb6`. The smoke ledger judges existing logs with separate wrapper exits and source provenance. UIManager class loading and the local version getter are limited probes, not milestones. Shader receipts verify 44 structurally valid SPIR-V candidates with both vkd3d 1.2 (12 diagnostic jobs) and official Wine vkd3d 2.0 (zero); descriptor set, binding and storage-class inventories match, with no semantic or rendering proof. The later party-roster test brings backend tests to 72, with local admission policies and no client end-to-end verification. An isolated stage with native FortGameInstance selected verified Entry, a live instance and direct join; UI remains stubbed. The strict pre-freeze material uniform-expression prefix parser (`2014363`) stops at documented unknown fields in all four original resources; shader runtime semantics and draws remain unverified. Original client build metadata reports CL 13315662, distinct from the workspace folder CL 13477524. All four product gates remain open.

### Recording evidence

```sh
python3 -I porting/progress/record_evidence.py CHECK_ID [--timeout S] [--grep REGEX] [--log PATH] -- COMMAND [ARG...]
python3 -I porting/progress/record_evidence.py CHECK_ID --log PATH --grep REGEX   # judge an existing log
python3 -I porting/progress/record_evidence.py --list
python3 -I porting/progress/record_evidence.py --remove CHECK_ID
```

The command runs in the repository root with a timeout (default 900 s; exit 124 on timeout).
`result` is `pass` when the exit code is 0 (or no command ran) and, with `--grep`, at least one
line matches. The entry stores the exit code, the match count, up to five key lines, the HEAD
commit and a UTC time, and replaces the previous entry for that id. Commit `evidence.json` after
recording. A check that has not been recorded counts as not passed.

### Standard suite

These ids are the ones `components.json` uses. Run them from the repository root and record each
with `record_evidence.py`. Smoke and Wine runs bind ports or start the original client: run them
only when the orchestrator says so.

| Check id | Command | Pass condition (how it is recorded) |
|----------|---------|-------------------------------------|
| `smoke_default` | `bash porting/server/smoke_join.sh` | the script exits 0 |
| `smoke_athena` | `bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'` | the script exits 0 |
| `net_version_checksum` | none; `--log porting/generated/dedicated-server-client-test.log --grep 'Local network version checksum: 683914758'` | a server log line with that checksum (run `smoke_default` first) |
| `wine_starts` | run `porting/run_original_client.sh`, then `--log` the client log below with `--grep 'LogInit: Display: Game Engine Initialized'` | the engine starts under Wine |
| `wine_pak_mount` | as `wine_starts`, `--grep 'LogPakFile: Display: Mounting pak file'` | paks mount |
| `wine_frontend_map` | as `wine_starts`, `--grep 'LogLoad: LoadMap: /Game/Maps/Frontend'` | the Frontend map loads |
| `wine_login_ui` | as `wine_starts`, `--grep 'LoginScreen_C_'` | the login screen appears |
| `wine_entitlement_passed` | as `wine_starts`, `--grep 'LogAppES: .*([Ee]ntitlement).*(passed|granted|success)'` | no passing record; the observed run stopped at the AppES access gate |
| `wine_lobby_reached` | as `wine_starts`, `--grep 'LogLoad: LoadMap: .*Lobby'` | no passing record; the observed run stopped before this stage |
| `original_client_hello`, `original_client_login`, `original_client_welcome`, `original_client_join` | record stage-specific evidence from an original-client connection to the reconstructed server | each stage is pending; the observed run stopped at AppES |
| `checksum_tool_selftest` | `python3 -I porting/class_checksum.py --self-test` | exit 0 |

The client log for the Wine checks is
`porting/generated/wine-prefix/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log`.
`--log` judges whatever log is on disk, so record the Wine checks right after a run.

There is no `generator_selftest`: `gen_native_classes.py` has no self-test, so it is not in the
suite until one exists.

## Objectivity audit

The following audit and every table and ceiling below are a historical baseline, superseded by the current model described above; they are not a current dashboard snapshot.

Changed by the objectivity pass (`components.json` is the source of truth; each leaf's `rationale`
has the same facts). Baseline: HEAD 9e6c524, before the change. Values for "after" assume
`totals.json` and `evidence.json` are committed with the change; before that, HEAD reads both
as missing and the totals-based leaves show 0.

### Weight by basis

| Basis | Before (weight 1,725) | After (weight 1,875) |
|-------|-----------------------|----------------------|
| `git` | 417 (24.2%) | 947 (50.5%) |
| `local` | 108 (6.3%) | 253 (13.5%) |
| `git+local` | 105 (6.1%) | 105 (5.6%) |
| `documented` | 250 (14.5%) | 115 (6.1%) |
| `manual` | 845 (49.0%) | 455 (24.3%) |

Objective (`git` + `local`) weight share: 36.6% before, 69.6% after.

### Overall

| Figure | Before | After |
|--------|--------|-------|
| Overall | 15.31% | 17.30% |
| Objective (`git` + `local`) | 10.50% | 16.24% |
| Evidence-based (`measured_pct`) | 13.07% | 16.25% |
| Manual share of overall | 2.25% | 1.05% |
| Overall if all manual work were complete | 62.05% | 40.52% |

Movement of the overall (+1.99 points): the new 150-weight `gameplay.native_behavior` leaf dilutes
every other leaf by 1,725/1,875, which alone costs 1.23 points. The leaf changes add 3.22 points.
The largest of them: `gameplay.weapons` +1.84 (27 of 47 weapon class headers), `gameplay.battle_bus`
+0.67, `platform.eac_removal` +0.64 (credit 50%), `ui.commonui_umg` +0.52 (58 of 168 class headers),
`gameplay.storm` +0.31. Down: `net.native_join` -0.46 (ledger empty, so 0 until the suite is
recorded), `assets.textures` -0.28, `assets.static_meshes` -0.11, `assets.materials` -0.08.

### Converted leaves

Current basis of each leaf that was converted or re-measured in the objectivity pass, as in
`components.json` and `out/progress.json` at HEAD. Percentages are the leaf's `pct`, which is
capped by any `ceiling`. Figures for `assets.static_meshes` and `visuals.vfx` are at HEAD;
after the next commit the denominators are 4,727 and 173 (see "Refreshing the totals").

| Leaf | Basis, pct | Done / total or check count | How it is computed now | Remaining subjectivity |
|------|------------|-----------------------------|------------------------|------------------------|
| `gameplay.reflection_exact` | git, 99.5% | 2828 / 2842 | tracked `Source/FortniteGame/Generated/[AU]*.h` over named `/Script/FortniteGame` classes (totals) | A header is not behaviour. Other generated modules are listed, not counted. |
| `net.checksum_parity` | git, 65.7% | 4572 / 6957 | regex on the Parity column of the tracked checksum report, over named native classes (totals) | Offline model of the checksum; no live comparison with the original client. |
| `ui.fortniteui_classes` | git, 97.6% | 1641 / 1681 | tracked `FortniteUI/Generated` headers over named `/Script/FortniteUI` classes | None beyond the denominator. |
| `assets.reflection_recovery` | local, 99.9% | 6957 / 6967 | named blocks in `class_name_map.json` over block total (totals) | Name accuracy is not re-verified here. |
| `assets.blueprint_load` | documented, 0.02% | 4 / 22211 | denominator from the registry's Blueprint count (totals) | Numerator (4) still typed. |
| `maps.enter_play` | documented, 0.05% | 2 / 3694 | denominator from the registry's World count (totals) | Numerator (2) still typed. |
| `maps.streamed_levels` | documented, 0% | 0 / 87 | denominator from the regex on `map_compatibility.md` (totals) | Numerator (0) still typed. |
| `audio.sound_classes` | documented, 0.3% | 1 / 317 | denominator: sound class or mix package paths (path heuristic, totals) | Numerator typed; the path rule is a heuristic. |
| `assets.map_headers` | local, 100% | 3694 / 3694 | denominator from the registry's World count | None. |
| `assets.package_coverage` | local, 24.2% | 108,956 / 450,951 | denominator is the cooked `.uasset` entry count of the full extraction (totals, `cooked_uasset_entries`); weight 50 | Local recovered or converted files are counted by folder, not by package. |
| `assets.static_meshes` | local, 1.6% | 77 / 4719 at HEAD, 77 / 4727 after the commit | converted `.glb` stems plus `SM_` names in native/Content, distinct names; denominator is `SM_`-named cooked packages in the full tree (naming heuristic) | Naming heuristic; a conversion is not spawn-verified. |
| `assets.skeletal_anim` | local, 0% | 0 / 1719 | `SK_` packages converted in native/Content; denominator `SK_`-named cooked packages in the full tree | Naming heuristic; animation (`A_`, 7 names) not counted. |
| `platform.wine_launch` | git, 0% | ledger 0 of 6 | six Wine stage checks in `evidence.json` | Needs runs and recording; the last two stages have no passing record. |
| `platform.eac_removal` | git, 0% | ledger 0 of 1 (`wine_starts`) | the ledger check listed in `components.json` | Needs the check run and recorded. |
| `net.native_join` | git, 100% | ledger 3 of 3 | `smoke_default`, `smoke_athena`, `net_version_checksum` in the ledger | Recorded checks only; they run on the server. |
| `net.original_join` | git, 0% | ledger 0 of 2 | `wine_entitlement_passed`, `wine_lobby_reached` in the ledger | See `components.json` rationale. |
| `audio.voice` | git, 50% | 2 of 2 routes, stub credit 0.5 | voice routes from the static client scan, present in committed server code | Route presence only. |
| `visuals.vfx` | local, 0% | 0 / 171 at HEAD, 0 / 173 after the commit | `NS_` Niagara packages converted (none) over `NS_` cooked names in the full tree | Particle (Cascade) assets not counted. |
| `gameplay.weapons` | git, 20% | 47 of 47 headers, ceiling 20 | tracked reflection headers whose name matches `Weapon` over named FortniteGame classes matching the same regex | Counts headers, not firing. Capped at 20. |
| `gameplay.storm` | git, 20% | 27 of 27 headers, ceiling 20 | tracked headers matching `Storm\|SafeZone` | Same caveat as weapons. Capped at 20. |
| `gameplay.battle_bus` | git, 20% | 16 of 16 headers, ceiling 20 | tracked headers matching `BattleBus\|Aircraft\|Parachute\|Glider` | Same caveat as weapons. Capped at 20. |
| `gameplay.native_behavior` | git, 1.2% | 249 / 20574 | committed `porting/behavior/**` bodies over native functions with an exec address | None. Small by design. |
| `ui.commonui_umg` | git, 20% | 58 of 168 headers, ceiling 20 | tracked CommonUI and UMG headers over named classes in those packages | Headers, not behaviour. Capped at 20. |

### Still manual

Each keeps its typed percentage as a ceiling, so it cannot exceed the estimate without a metric.
These are the 14 `manual` leaves in `out/progress.json`.

| Leaf | Pct (ceiling) | Why no objective metric exists yet |
|------|---------------|-----------------------------------|
| `platform.rendering` | 30 | Needs captures of the client compared with the original; none are compared. |
| `assets.landscape` | 15 | 221 components are decoded only in the A1-H8 grid export; the cooked tree stores components inside 67 tile packages, not one package each, so no component denominator exists. See its rationale. |
| `assets.cinematics` | 0 | The registry has no level-sequence count; cooked 'Sequence' names are mostly curves. |
| `maps.sky` | 20 | No sky or cloud render is captured and compared. |
| `maps.pois` | 1 | No POI map enters play, so there is nothing to count. |
| `gameplay.blueprint_logic` | 1 | No Blueprint behaviour is reproduced, and no metric counts it. |
| `net.replication_layout` | 1 | Layout parity needs per-property offsets; the checksum leaf covers only the hash. |
| `net.replication_graph` | 0 | Nothing about the server replication graph is reproduced or measured. |
| `audio.playback` | 0 | Every probe ran with `-nosound`; no audio output exists to count. |
| `audio.cues_music` | 2 | No cue or music asset has been played; no cue count is converted. |
| `visuals.lighting` | 10 | Needs captures compared with the original. |
| `visuals.postprocess` | 3 | No post-process setting is captured or compared. |
| `ui.frontend_lobby` | 0 | No menu or lobby widget is reproduced; widget Blueprints are not mapped to menus. |
| `ui.hud` | 0 | No HUD widget is reproduced or captured. |

`backend.session_beacon` is not manual: it is `git` at 10% (3 of 3 routes, see `components.json`).

Also still typed but not manual: the denominators of `maps.apollo_terrain` (61) and
`maps.background` (32) are literals, and their numerators are counts of the same local lists,
so the ceilings (35, 30) are their only limit.

## Measurement rules

- **Commit-only.** Text sources are read with `git archive` at the commit. Untracked or
  uncommitted files, such as work in progress under `porting/server/`, do not count.
- **Local outputs.** Ignored files under `porting/generated/` are read from disk. In a
  history replay they are marked `carried` because they show today's state, not the
  state at that commit. Manual estimates are also carried into replayed points.
- **Pre-porting commits.** A commit with no `porting/` tree scores 0%.
- **Totals and ledger are committed data.** `totals.json` and `evidence.json` are read from
  each commit like any tracked file. A commit that has no `totals.json` uses HEAD's copy and
  says so in the evidence text. Until the orchestrator commits both files, HEAD reads them as
  missing, so the totals-based leaves show 0.
- **Basis labels.** `git` (committed sources, the tracked totals and ledger), `local` (ignored
  outputs), `documented` (a typed count from the porting docs), `manual` (a typed estimate), and
  `carried` (replay only, local data read from disk).
- **Objective share.** `objective_pct` is the share of the overall from leaves whose basis is
  neither `manual` nor `documented`: only `git` and `local` count.

## Colour

A leaf's colour is not a hue. Each leaf is a neutral tile, and its blue fill rises from the
bottom edge to its percent complete: the fill is pct/100 of the tile height, clipped to the
tile's rounded corners, with a 2px edge line along its top when the fill is partial. 0% is
empty and 100% fills the tile. Unit-square tiles overlay squares on the same fill (see Unit squares).
The percentage is printed on every tile, so colour is never the only cue. The treemap key
is an empty-to-full scale under the chart.

The palette is CSS custom properties (`--tm-*`), defined once in `TREEMAP_PALETTE_CSS` in
`progress.py` and used by the page and by `progress.svg`:

- light: surface `#fcfcfb` (also the gutter), neutral tile `#ecebe8`, fill `#b7d3f6`,
  fill edge and unit-done `#2a78d6`, unit-todo `#d6d5d0`, ink `#0b0b0b`, secondary ink and
  group labels `#52514e`;
- dark: surface `#1a1a19`, neutral tile `#2b2b29`, fill `#184f95`, fill edge and unit-done
  `#3987e5`, unit-todo `#3d3d3a`, ink `#f0efec`, secondary ink and group labels `#c3c2b7`.

The page and `progress.svg` follow the OS light or dark setting, and `data-theme="dark"`
selects the dark palette on the page. Ink is the same colour on the neutral surface and on
the fill, which is why the fill is a light tint in light mode and a dark blue in dark mode:
the text reads on both.

## Interactive view

`index.html`'s treemap is interactive; `progress.svg` stays a static picture of
the same layout. All of it is one inline script plus CSS — no external assets,
and with JavaScript disabled the page falls back to the static treemap.

- **No zoom.** The treemap is always shown at full size: there is no camera,
  drill-down or breadcrumbs. Hovering a tile shows a tooltip with the name,
  percent, done/total, weight, basis and evidence; clicking fills a detail
  panel below the treemap (with the component's children as links for
  categories).
- **Sort.** The *Sort tiles by* toggle re-lays out the whole treemap by percent
  complete instead of by area. Both layouts are precomputed by the generator
  (two sibling layers inside the SVG); the script only swaps which layer is
  visible, so no layout math runs in the browser.

## Treemap labels

Labels are never cut off. Group headers and tile names wrap at word boundaries; a
tile too small for its name shows only its percentage, and every tile's full label,
done/total, source and evidence are in the hover title.

Leaf tiles (`leaf_label` in `progress.py`) use an 11px name, falling back to 10px
when 11px would need more than four lines, and a 12px percentage (11px when 12px
is too wide). Text is never below 10px. A tile that cannot hold the name at 10px
in at most four lines shows its percentage alone; a tile too small even for that
shows nothing. Names are never truncated, and the full name is in the hover title
and the detail panel. The same labels appear in the static `progress.svg`.

Group labels are plain 12px text on the surface above their children, with no header
strip: weight 600, with a lighter "· pct" suffix. The name is shown in full on one
line when it fits; otherwise a trailing parenthetical such as " (Apollo island)"
is dropped for the display; otherwise the name wraps to at most two lines; else
only the percentage. The full name is always in the hover title.

Every tile, group or leaf, is inset by 1px, so sibling tiles have a 2px gutter.
A group's children sit 3px inside its frame. The page and `progress.svg` follow
the light or dark OS setting. Tiles have 4px corners and no stroke; top-level groups have a
faint 1px hairline in the neutral tile colour, and sub-groups none. Hovering a tile outlines
it in 2px ink, and the clicked tile keeps a 2px blue outline across both sort layers.

Unit-square tiles use the same label rules (`unit_label` calls `leaf_label`), so
they also never show text below 10px. Their squares sit below the label strip, and
at least 1px separates neighbouring squares. The smallest square pitch is 3px (a
2px side), so a dense grid that would read as one solid block falls back to the
fill level instead.

Wrap widths come from a per-character advance table in `progress.py` (`CHAR_W`):
the maximum advance across DejaVu Sans Bold and Liberation Sans Bold (the two
widest common sans families a viewer is likely to use, and wider than SF Pro,
Segoe UI, Arial or Helvetica for every character in the set), measured at each
font's native 2048 upem with a 5% margin for the synthesized 600 weight. The
table bounds the bold and regular faces alike, so no emitted line can overflow
its tile on those fonts; characters outside the set default to the widest entry.

## Unit squares

Decomposable leaves additionally render one square per unit, drawn under the
tile's label, over the same bottom-up progress fill used by other leaves. Each unit is its own square of identical size, with no merging into
stripes or stretching: done is the `unit-done` blue and not-done the `unit-todo` neutral. Both are palette
variables (see Colour), so they swap with the page theme. Units are grouped by
asset type, package or class; groups are contiguous, with a gap row between groups
that starts at a row boundary. Done units come first within each group.

`unit_grid_layout` picks the largest integer pitch `s` (at least 2, at most 9 SVG
units, `UNIT_PITCH_MAX`) at which every unit fits in the grid region: the tile minus a label strip at the top (sized from
the fitted name and percentage, `label_strip`) and a 3px inset on the other sides.
A square is `s` wide, minus a 1px gap when `s >= 4`. The grid is left and top
aligned, filling rows left to right, so a tile with few units shows a small block in its
top-left corner; the background shows the tile's progress fill. Squares carry no labels; the
done/total is in the hover summary and the detail panel.

A grid is kept only when it reconciles with its leaf: the grid's done and total must
equal the leaf's own measured done and total (`build_unit_grids`). A grid that does not
reconcile is dropped and the leaf shows its fill level alone, which is driven by the leaf's
pct, so a grid never contradicts its tile's percentage. Currently `assets.static_meshes`
(71 of its 77 done items are converted `.glb` stems outside the SM_ unit list),
`gameplay.native_behavior` (126 ported functions have no exec address, so they are
outside the 20,574 units) and `gameplay.reflection_exact` (one tracked header,
`UFortMatchmakingKnobsDataSource`, is not in the named-class denominator) are dropped.
Adding those items as units would change the leaf total, which the measure fixes.

A tile shows its fill level without squares when its grid is dropped by the
reconciliation check, when its units cannot fit at pitch 2 or more, or when it has more
than `UNIT_GRID_MAX` (6000) units. `assets.package_coverage` (about 450k cooked packages)
has no grid and is a plain fill-level tile. The done set of a grid never exceeds the
tile's numerator: it is the leaf's own done signal intersected with the unit list.

## Git hygiene

`out/` and `history.json` are git-ignored in the top-level `.gitignore`. The
generator never writes into tracked paths. The hook appends output to
`porting/generated/progress-hook.log`, and it always exits 0.

### Generic context runtime allocation (2026-11-01)

One point was split from the 13-point zero-valued lifecycle remainder for the
verified generic context diagnostic runtime, leaving 12 unfinished lifecycle
points. Five checks cover exact original Native metadata in both targets, three
measured loaded properties, lookup selection, registry/GC/removal/deinit, and
world travel/deferred timer/copied-map dispatch. This is distinct from the
previously credited generic source and 41 non-Native event signatures. Concrete
context methods, authentic startup and playable UI remain unverified; all four
product milestones remain false. Root and Login still stop at OnBeginIntro and
the Google material respectively.

Total weight remains 1,855. This judgment-based model revision adds one point,
changing the estimate from 21.172% to 21.226% at the same HEAD and local data;
it is not a retroactive historical gain or measured engineering time. The
importer pins a green checkpoint, independent full strict report and independent
integrity audit, and checks their source, binary, metadata and runtime bindings.
The audit covers 122 committed source bindings, 1,416 artifacts and 18 binary
rows. These receipts and the audited artifacts are external to the ledger; the
integrity audit currently lives under /tmp and must remain available at its pinned
path for re-verification. The ledger alone is an attestation.

### Google material runtime credit (2026-11-01)

The existing Google owner now passes, bringing the bounded material closure to
126/142. `record_frontend_google_runtime.py` pins the successful Native
`5d45ea2` / Engine `d7e04d4b6` checkpoint, two fresh strict Google reports,
the full context report, independent integrity audit and root aggregate audit.
The profiles verify named texture binding, diagnostic ten-mip sampling controls
and seven shader payloads. Raw BC1 GPU readback, original-client pixel parity,
widget integration and interactive UI remain unverified. The audit covers 202
committed source bindings, 1,693 artifact rows and 18 binary rows; the pinned
external receipts and their audited artifacts must remain available for
re-verification. This changes no weights, leaves or acceptance gates.

The first observed Root and Login boundaries are now M_UI_ComicBubble and
M_TempIcon_Login_Facebook respectively, both at MemoryImage.cpp:1064; async
load order may change which appears first. Native OnBeginIntro remains
unimplemented. Earlier history points are preserved.

### CommonUI internal button runtime allocation

One point is allocated from the remaining CommonUI behavior share to the
controlled internal button runtime diagnostic, leaving 20 points for unfinished
behavior. The diagnostic exercises real UObject and Slate factory, lifecycle,
core input and callbacks in a controlled fixture; it does not establish the main
CommonButton, Panel, queue, full UI, gesture or parity behavior. Both targets
were rebuilt, four live processes and two joins passed, and the source and
artifacts are pinned for independent verification. This phase did not repeat an
exhaustive original UHT/ELF audit or the material checks. The denominator remains
1,855 and the milestone gates remain false.

### CommonUI base activation runtime allocation

One point of the 22-point unfinished CommonUI behavior share is reserved for the
integrated base activation diagnostic runtime; the remaining 21 points cover
Panel and operation queue, input dispatch and held actions, buttons and other
widgets, integration and behavior parity. These are rough relative effort shares,
not a count of methods or elapsed hours. The prior one-point activation source
candidate and one-point input source candidate remain separate. Header and event
signature coverage and the material pipeline measure different work.

The check now passes on the pinned phase-5 checkpoint, independent full strict
report, integrity audit and separate 18-row inherited-binary audit. Both target
builds are inherited from phase 2; three new runtime processes pass after the
retry epoch, including 40 activation rows and the inherited context checks.
Both targets pass original UHT metadata and linked implementation verification.
The audit covers 639 new artifacts, 42 inherited metadata files and 18 binary
rows. Pinned external receipts and their audited artifacts must remain available
for re-verification. This one check covers base activation only; Panel/queue,
input, full CommonUI, authentic startup, interactive UI and original-client
parity remain unverified. All four milestones remain false. The allocation
preserves total weight 1,855 and prior history; the one point adds about 0.054
percentage points at the same remaining evidence.

At the base-activation checkpoint, the Root probe first encountered missing CommonActivatablePanel.OnBeginIntro
at AsyncLoading.cpp:3126; Login first encountered M_TempIcon_Login_Facebook at
MemoryImage.cpp:1064. Asynchronous load order can change the first boundary.

### CommonUI Panel intro runtime allocation

One point is allocated from remaining CommonUI behavior to the controlled Panel
intro diagnostic, leaving 19 points for unfinished behavior. It exercises the
real intro, EndIntro and full activation path, including input and priority in a
fixture and C++ ProcessEvent interception. It does not establish Blueprint VM,
the whole Panel, outro, deactivation, input producer, manager, full UI or parity.
The phase includes two builds, six processes and two joins; it does not repeat
the broader original metadata, link, material or pixel checks. The denominator
remains 1,855, history is preserved and all four milestone gates remain false.

### CommonUI Panel outro runtime allocation

One point is allocated from remaining CommonUI behavior, leaving 18 points, for
the controlled Panel intro, outro, reacquisition and listener lifecycle diagnostic.
It includes held-action map, NaN duration, callback manager relookup and structural
interface reset fixtures with C++ ProcessEvent interception. Blueprint VM, full
interface routing, native input producer, queue, main button, authentic UI and
parity remain unverified. Interface restoration source work uses the existing
reflection budget. Two builds, seven processes and two joins are covered by pinned
evidence and an independent audit. The denominator remains 1,855, history is
preserved and all four milestone gates remain false.

### Confirmation runtime allocation

One point is split from the existing 30-point FortniteUI behavior budget, leaving
29 points there. Four equally weighted checks cover description construction/copy,
the authentic async action, queued presentation and user interaction. The pinned
`record_frontend_confirmation_description_runtime.py` importer credits only the
first check: actual UE defaults, original size/alignment, and typed native delegate
ownership through C++ and UScriptStruct copying. The family therefore shows 25%.
The denominator stays 1,855; no milestone is marked complete.

At Native `00e1279`, both builds, eight diagnostic processes and both joins pass.
The original Root class loads with no unresolved bytecode-field warnings; earlier
intro/outro load failures are resolved. No original Root widget instance or
interactive presentation has been established. The importer rechecks archived
artifacts and source pairs; stage binary fields are cross-checked without
rehashing binary contents. The original build/stage/freeze receipts carry those
binary hashes. Frozen external evidence must remain available for re-verification.

Native body coverage reads committed `MANIFEST.json` entries with tracked `.inc`
files directly, excluding member fields. It updates after each source commit
without rerunning native tests. `behavior/COUNTS.txt` retains the last checker
checkpoint; source coverage does not establish build or runtime parity.
