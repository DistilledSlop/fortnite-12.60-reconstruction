# Port progress dashboard

A generated dashboard for the Fortnite 12.60 Linux port: a squarified treemap of
the whole effort (area = estimated effort weight, colour = percent complete), a
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
| `evidence.json` | yes | Evidence ledger of check results written by `record_evidence.py`. Tracked. Starts empty. |
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

`--seed` replays every commit (about sixteen seconds for 98 commits). It is the only way to
refresh history for old commits. A normal run measures only HEAD.

## Editing estimates

Open `components.json`. Each leaf has a `weight` (its treemap area and its share of
the overall figure) and a `measure`. Measures, from most objective to least:

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

Denominators (`total_key` and optional `total_sub`) are read from `porting/progress/totals.json`.
If a key is missing the leaf shows 0 and says why. A leaf may have `"ceiling": N`, which caps its
value where the measure overstates progress, with the reason in `rationale`. Groups take the
weight-averaged value of their children. Set `"done": true` on a milestone when it is reached.

Edit, commit, and the post-commit hook regenerates the dashboard. The totals and the ledger are
measured from the commit, so commit `totals.json` and `evidence.json` in the same commit that
uses them.

### Refreshing the totals

```sh
python3 -I porting/progress/refresh_totals.py      # rewrite porting/progress/totals.json
```

Run it after the extraction outputs change (reflection tables, asset registry, pak index,
`porting/map_compatibility.md`). It reads only local files, takes about three seconds, and fails
(leaving the file unchanged) if a source is missing. Each entry records its `source` file, its
`method`, and a `generated_utc` taken from the source file's modification time, so unchanged data
gives identical bytes. Commit the result.

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
| `wine_entitlement_passed` | as `wine_starts`, `--grep 'LogAppES: .*([Ee]ntitlement).*(passed|granted|success)'` | expected to fail: the AppES gate is not bypassed |
| `wine_lobby_reached` | as `wine_starts`, `--grep 'LogLoad: LoadMap: .*Lobby'` | expected to fail: the entitlement gate stops the client first |
| `checksum_tool_selftest` | `python3 -I porting/class_checksum.py --self-test` | exit 0 |

The client log for the Wine checks is
`porting/generated/wine-prefix/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log`.
`--log` judges whatever log is on disk, so record the Wine checks right after a run.

There is no `generator_selftest`: `gen_native_classes.py` has no self-test, so it is not in the
suite until one exists.

## Objectivity audit

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

| Leaf | Old basis, pct | New basis, pct | How it is computed now | Remaining subjectivity |
|------|----------------|----------------|------------------------|------------------------|
| `gameplay.reflection_exact` | documented, 42.2% (1200/2842) | git, 46.0% (1307/2842) | tracked `Source/FortniteGame/Generated/[AU]*.h` over named `/Script/FortniteGame` classes (totals) | A header is not behaviour. Other generated modules are listed, not counted. |
| `net.checksum_parity` | documented, 18.9% | git, 18.9% (1317/6957) | regex on the Parity-yes column of the tracked checksum report, over named native classes (totals) | Offline model of the checksum; no live comparison with the original client. |
| `ui.fortniteui_classes` | documented, 0% | git, 0% (0/1681) | tracked `FortniteUI/Generated` headers over named `/Script/FortniteUI` classes | None beyond the denominator. |
| `assets.reflection_recovery` | documented, 99.9% | local, 99.9% (6957/6967) | named blocks in `class_name_map.json` over block total (totals) | Name accuracy is not re-verified here. |
| `assets.blueprint_load` | documented, 4 of 22211 | documented, 4 of 22211 | denominator from the registry's Blueprint count (totals) | Numerator (4) still typed. |
| `maps.enter_play` | documented, 2 of 3694 | documented, 2 of 3694 | denominator from the registry's World count (totals) | Numerator (2) still typed. |
| `maps.streamed_levels` | documented, 0 of 87 | documented, 0 of 87 | denominator from the regex on `map_compatibility.md` (totals) | Numerator (0) still typed. |
| `audio.sound_classes` | documented, 1 of 306 | documented, 1 of 317 | denominator: sound class or mix package paths (path heuristic, totals) | Numerator typed; 317 vs the documented 306 is a heuristic. |
| `assets.map_headers` | local, 100% | local, 100% | denominator from the registry's World count | None. |
| `assets.package_coverage` | local, 0.16% | local, 0.16% (712/450783) | denominator is the cooked `.uasset` entry count (totals), was 450,951 typed | Local recovered or converted files are counted by folder, not by package. |
| `assets.static_meshes` | manual, 8% | local, 1.6% (77 glb + SM_ in native/Content, distinct names / 4719) | counts converted meshes; denominator is SM_-named cooked packages (naming heuristic) | Naming heuristic; a conversion is not spawn-verified. |
| `assets.materials` | manual, 6% | local, 0.3% (83 / 24961) | M_, MI_, MF_ packages rebuilt in native/Content; denominator naming heuristic | Naming heuristic; disabled originals and recovered packages are excluded. |
| `assets.textures` | manual, 25% | local, 1.3% (386 / 30523) | PNG conversions under `generated/converted` plus T_ packages in native/Content | Naming heuristic; the PNG count is masks and atlases only. |
| `assets.skeletal_anim` | manual, 0% | local, 0% (0 / 1719) | SK_ packages converted in native/Content | Naming heuristic; animation (A_, 7 names) not counted. |
| `platform.wine_launch` | manual, 2% | git, 0% (ledger 0 of 6) | six Wine stage checks in `evidence.json` | Needs a real run and recording. Two stages are expected to fail. |
| `platform.eac_removal` (renamed) | manual, 2% | git, 50% (2 of 2 checks, stub credit 0.5) | `-nobe` and `-noeac` present in `run_original_client.sh`, credited at 50% | The credit is a judgement. The flags are not seen working; AppES is not bypassed. |
| `net.native_join` | manual, 40% | git, 0% (ledger 0 of 3) | `smoke_default`, `smoke_athena`, `net_version_checksum` in the ledger | Needs the suite run and recorded. |
| `net.original_join` | manual, 0% | git, 0% (ledger 0 of 2) | `wine_entitlement_passed`, `wine_lobby_reached` in the ledger | Expected to stay 0: the entitlement gate is not bypassed. |
| `gameplay.weapons` | manual, 0% | git, 57% (27 of 47) | tracked reflection headers whose name matches `Weapon` over named FortniteGame classes matching the same regex | Counts headers, not firing. It measures the same work as `reflection_exact` and reads higher than the behaviour warrants. |
| `gameplay.storm` | manual, 2% | git, 41% (11 of 27) | tracked headers matching `Storm\|SafeZone` | Same caveat as weapons. |
| `gameplay.battle_bus` | manual, 0% | git, 50% (8 of 16) | tracked headers matching `BattleBus\|Aircraft\|Parachute\|Glider` | Same caveat as weapons. |
| `gameplay.native_behavior` (new, weight 150) | none | git, 0.2% (36 of 20574) | committed `porting/behavior/**/*.inc` bodies over native functions with an exec address | None. Small by design. |
| `ui.commonui_umg` | manual, 2% | git, 34.5% (58 of 168) | tracked CommonUI and UMG headers over named classes in those packages | Same caveat as weapons: headers, and no UMG headers exist. |
| `audio.voice` | manual, 0% | git, 0% (0 of 2 routes, stub credit 0.5) | voice routes from the static client scan, present in committed server code | Route presence only. |
| `visuals.vfx` | manual, 0% | local, 0% (0 / 171) | NS_ Niagara packages converted (none) over NS_ cooked names | Particle (Cascade) assets not counted. |

### Still manual

Each keeps its typed percentage as a ceiling, so it cannot exceed the estimate without a metric.

| Leaf | Pct (ceiling) | Why no objective metric exists yet |
|------|---------------|-----------------------------------|
| `platform.rendering` | 30 | Needs captures of the client compared with the original; none are compared. |
| `assets.landscape` | 15 | Cooked landscape component packages are not enumerated; 'Landscape' names (69) are not comparable to the 221 decoded components. |
| `assets.cinematics` | 0 | The registry has no level-sequence count; cooked 'Sequence' names are mostly curves. |
| `maps.sky` | 20 | No sky or cloud render is captured and compared. |
| `maps.pois` | 1 | No POI map enters play, so there is nothing to count. |
| `gameplay.blueprint_logic` | 1 | No Blueprint behaviour is reproduced, and no metric counts it. |
| `net.replication_layout` | 1 | Layout parity needs per-property offsets; the checksum leaf covers only the hash. |
| `net.replication_graph` | 0 | Nothing about the server replication graph is reproduced or measured. |
| `backend.session_beacon` | 0 | No beacon or session-discovery route is in the static route scan. |
| `audio.playback` | 0 | Every probe ran with `-nosound`; no audio output exists to count. |
| `audio.cues_music` | 2 | No cue or music asset has been played; no cue count is converted. |
| `visuals.lighting` | 10 | Needs captures compared with the original. |
| `visuals.postprocess` | 3 | No post-process setting is captured or compared. |
| `ui.frontend_lobby` | 0 | No menu or lobby widget is reproduced; widget Blueprints are not mapped to menus. |
| `ui.hud` | 0 | No HUD widget is reproduced or captured. |

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

Five bins from red to green: `#a8102a`, `#e0573a`, `#f2b705`, `#3b9a3f`, `#006b3c`.
They were checked for colour-vision separation with OKLab deltaE: adjacent pairs
stay at or above 14.9 under simulated protan and deutan vision, and at or above 15.4
under normal vision. Every tile also prints its percentage, so colour is never the
only cue. The page follows the OS light or dark setting.

## Interactive view

`index.html`'s treemap is interactive; `progress.svg` stays a static picture of
the same layout. All of it is one inline script plus CSS — no external assets,
and with JavaScript disabled the page falls back to the static treemap.

- **Zoom.** Click any tile (category or component) and a camera zooms into it.
  The percentage shows on every tile at all zoom levels; name labels are hidden
  at the root level and revealed when zoomed in, where the tiles are large
  enough to hold them, so the root view stays readable. Breadcrumbs, the *Fit
  all* button or `Esc` go back. Hovering shows a tooltip with the name,
  percent, done/total, weight, basis and evidence; clicking fills a detail
  panel below the treemap (with the component's children as further drill-down
  links for categories).
- **Sort.** The *Sort tiles by* toggle re-lays out the whole treemap by percent
  complete instead of by area. Both layouts are precomputed by the generator
  (two layers inside one camera group); the script only swaps which layer is
  visible, so no layout math runs in the browser.
- **Geometry.** Zoom is a CSS transform on the camera group (`vector-effect:
  non-scaling-stroke` keeps borders crisp); the tile rectangles themselves are
  never modified by the script.

## Treemap labels

Labels are never cut off. Group headers and tile names wrap at word boundaries; a
tile too small for its name shows only its percentage, and every tile's full label,
done/total, source and evidence are in the hover title.

On the dashboard page every tile shows its percentage, and sizes stay uniform
unless a tile genuinely cannot hold them: the bases are an 11px name and a 12px
percentage (`fit_tile_label` in `progress.py`). A cramped tile first shrinks the
percentage, then the name (which also reduces the number of wrapped lines), and
only then drops the name altogether, leaving the percentage alone. The fit model
is the same font metrics the geometry checker validates against: the first
baseline sits 4px+size down, lines advance size+2 px, and text occupies 0.756em
above and 0.2358em below its baseline. The static `progress.svg` keeps the
conservative one-size behaviour: a label only when the tile fits one.

Wrap widths come from a per-character advance table in `progress.py` (`CHAR_W`):
the maximum advance across DejaVu Sans Bold and Liberation Sans Bold (the two
widest common sans families a viewer is likely to use, and wider than SF Pro,
Segoe UI, Arial or Helvetica for every character in the set), measured at each
font's native 2048 upem with a 5% margin for the synthesized 600 weight. The
table bounds the bold and regular faces alike, so no emitted line can overflow
its tile on those fonts; characters outside the set default to the widest entry.

## Git hygiene

`out/` and `history.json` are git-ignored in the top-level `.gitignore`. The
generator never writes into tracked paths. The hook appends output to
`porting/generated/progress-hook.log`, and it always exits 0.
