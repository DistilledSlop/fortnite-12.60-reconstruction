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

The dashboard splits the overall so the subjective part is always visible:

- **Evidence-based** (`measured_pct`): the share of the overall from leaves whose
  basis is `git`, `local` or `documented` — counts from committed sources, ignored
  local outputs, or documented totals. This is the objective lower bound.
- **Manual estimates** (`estimated_pct`): the share from `manual` leaves, with
  their weight share (`manual_weight_pct`) and the overall if all of that work
  were complete (`manual_full_pct`).

`progress.json` and every history entry carry the same fields; the history chart
plots the evidence-based series as a dashed line under the overall.

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
