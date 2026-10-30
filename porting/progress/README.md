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

## Files

| File | Tracked | Purpose |
|------|---------|---------|
| `progress.py` | yes | Generator. Stdlib only, offline, runs in under a second for HEAD. |
| `components.json` | yes | The model: categories, components, weights, manual %, measures, rationale, milestones. |
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

`--seed` replays every commit (about six seconds for 59 commits). It is the only way to
refresh history for old commits. A normal run measures only HEAD.

## Editing estimates

Open `components.json`. Each leaf has a `weight` (its treemap area and its share of
the overall figure) and a `measure`:

- `{"kind": "manual", "pct": N}`: your estimate, with `rationale`.
- `checks`: a list of checks. Each is `file` (tracked at HEAD), `local` (an ignored
  path under `porting/generated/`), or `pattern` plus `in` (a glob over tracked files).
- `routes`: client route templates from `porting/generated/routes.json` whose literal
  path appears in committed `porting/server/**`. Use `include`, `exclude` and `stub_credit`.
- `census`: `/Script/FortniteGame` imports from `porting/generated/map-native-imports.jsonl`
  whose names match `match`, counted as done when a native class of that name is declared
  under `porting/native/Source`. `stub_credit` (0.2) keeps an empty declared stub from
  counting as implemented.
- `fraction`: `done` over `total`, from a documented count or from a local JSON file
  (`count_file`, `count_key`).
- `jsonl_rows` and `local_glob`: counts from ignored local outputs.

A leaf may have `"ceiling": N`, which caps its value where the measure overstates
progress, with the reason in `rationale`. Groups take the weight-averaged value of
their children. Set `"done": true` on a milestone when it is actually reached.

Edit, commit, and the post-commit hook regenerates the dashboard.

## Measurement rules

- **Commit-only.** Text sources are read with `git archive` at the commit. Untracked or
  uncommitted files, such as work in progress under `porting/server/`, do not count.
- **Local outputs.** Ignored files under `porting/generated/` are read from disk. In a
  history replay they are marked `carried` because they show today's state, not the
  state at that commit. Manual estimates are also carried into replayed points.
- **Pre-porting commits.** A commit with no `porting/` tree scores 0%.
- **Basis labels.** `git` (committed sources), `local` (ignored outputs), `documented`
  (a count from the porting docs), `manual` (an estimate), and `carried` (replay only).

## Colour

Five bins from red to green: `#a8102a`, `#e0573a`, `#f2b705`, `#3b9a3f`, `#006b3c`.
They were checked for colour-vision separation with OKLab deltaE: adjacent pairs
stay at or above 14.9 under simulated protan and deutan vision, and at or above 15.4
under normal vision. Every tile also prints its percentage, so colour is never the
only cue. The page follows the OS light or dark setting.

## Git hygiene

`out/` and `history.json` are git-ignored in the top-level `.gitignore`. The
generator never writes into tracked paths. The hook appends output to
`porting/generated/progress-hook.log`, and it always exits 0.
