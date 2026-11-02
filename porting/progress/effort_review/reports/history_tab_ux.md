# History tab — usability analysis

2026-11-02. Source: `progress/progress.py:1884-2112` (`history_html`, chart script, table), `progress/history.json` (409 entries), `progress/out/history.html` (352 KB, 409 table rows). **Limit:** no browser is installed here, so I did not see the page rendered. Judgments come from the code and the data; someone should open it and confirm the visual points marked (visual). Nothing was modified.

## What the page does today

- A line chart of overall % per commit (solid) and an "evidence-based" % (dashed), opening on the latest 50 commits with a fitted y-axis, or all commits on a zero baseline.
- Six range buttons (All, Recent 50, Older, Newer, Zoom in, Zoom out), a slider and hover to pick a commit, and a one-line grey readout.
- A table of all 409 commits: date, hash, subject, overall, nine category columns, and a "Source" column.
- An explanatory paragraph under the chart.

## What the data shows (why it feels unhelpful)

| Fact | Value | Source |
|---|---|---|
| Entries | 409 | `history.json` |
| Replayed ("seeded") rather than recorded at commit time | 215 (53%) | `seeded` flag |
| Commits where overall changed | 67 of 408 | diff of `overall_pct` |
| The last 14 commits (all of the 2026-11-02 UI work, 04822b2 and neighbors) | overall 21.458% on every one; zero leaf changed | diff of `leaves` |
| Last-50 window | 21.226-21.458%, 7 changes | `overall_pct` |
| Five largest single-commit jumps | 6.12, 5.4, 5.4, 4.43, 4.32 points, mostly seeded entries or a model rescope ("Account for reconstruction scope...") | `history.json` |
| Leaf count at first vs latest entry | 61 vs 117 | `leaves` |

So the main story a reader wants ("what did we get done?") is invisible: a day of real UI work is a flat line, while the biggest movements are artifacts of replay or of rescoping the model. This is partly inherent to the scoring (credit appears only when a measured leaf moves), and it is the same issue behind the effort-weight review: any weight change I recommend will itself create a step in this chart.

## Problems, ordered by impact

1. **The page cannot answer "what changed?"** Each entry already stores every leaf's percentage, yet the page shows only the nine category totals. There is no per-commit delta, no list of leaves that moved, and no way to find the commits that did change the score among 409. (code: table rows at `progress.py:2065-2070`; leaves saved but unused)
2. **Model changes and replays look like progress.** Replayed points (53%) are drawn identically to recorded ones (same line, same dots), and model revisions are not marked at all; the only warning is a sentence of small grey text at the bottom. Entries do not store the model hash or total weight, so even the table cannot tell which jumps are rescoping. (code: single `<polyline>` for all entries; `seeded` appears only in the last table column)
3. **Zoomed scale exaggerates noise.** The default view (recent 50) uses a fitted axis, so a 0.2-point span fills the chart, and the fact that the scale is fitted is stated only in the small status line. A reader can take a 21.226 to 21.458 wobble for substantial movement. (visual)
4. **Commit index, not time, on the x-axis.** 227 commits happened on 2026-11-01 and 13 on 11-02, so equal spacing per commit distorts time; axis labels are bare hashes (`ea68ee4`), meaningless to readers. (code: labels use `short`)
5. **Control language is technical.** "Zoom in / Zoom out / Older / Newer" halve or double a window of commit counts; "Recent 50" is the only preset. There are no time presets (today, last 7 days), no "since last model change".
6. **The table is a 409-row wall.** No search, filter, sort, sticky header or pagination; nine percentage columns with one decimal each repeat mostly identical values; the "Source" column repeats a 54-character sentence ("replayed: manual estimates and local outputs as of now") 215 times. The table and the chart are not linked: picking a point does not highlight its row, and clicking a row does nothing. (code: table build loop)
7. **Detail readout is one long grey sentence.** `hash date: overall x%; evidence-based y% | subject` in a `min-height: 1.5em` line gives no deltas, no categories, no explanation of what "evidence-based" means. (code: `inspect()`)
8. **No legend on the chart itself.** Solid versus dashed is explained only in the paragraph below; `objective_pct` is stored but never plotted.
9. **Accessibility and touch.** The chart is `role="img"` with a one-line label and no textual summary; pointer inspection uses `pointermove` on an overlay with no `touch-action` handling, so on touch screens it competes with page scrolling (visual/untested); color plus dash are the only distinctions between series; hover tooltips exist only on dots, which are hidden when the series is dense.

## Recommendations (stable IDs; no code written)

Ownership: all changes are in `progress/progress.py` (`history_html`, and the recording path around `reconcile_history`), with tests in `progress/test_history.py`. Each is a separate bounded task.

| ID | Change | Why | Data needed | Priority |
|---|---|---|---|---|
| HIST-01 | Add a delta column and a "what moved" cell (top 3 leaves by change, with ±points) per commit; add a **"Only commits that changed the score"** filter (default on) | Answers the main question; 341 of 408 rows vanish by default | None; `leaves` already stored | P1 |
| HIST-02 | Mark **replayed** points (hollow dots or lighter segment) and show **model revisions** as vertical rules with a label; add legend (solid/dashed/hollow) on the chart | Stops rescoping and replays reading as work done | Record `model_sha256` and `total_weight` per new entry; for old entries derive revision points from `git log -- components.json` | P1 |
| HIST-03 | State the scale in a visible chip ("zoomed: 21.2-21.5%") with a one-click true-scale (0-25%) toggle | Prevents misreading small wobble | None | P1 |
| HIST-04 | Replace the detail line with a card: commit, date, subject, overall with delta, evidence-based, objective, per-category deltas, top leaf changes, replay/model note; link to the commit | Readable, self-explanatory | `categories` and `leaves` already stored | P1 |
| HIST-05 | Table: sticky header, text filter on subject, hide category columns behind a toggle, replace the long Source sentence with a small icon and a tooltip, paginate or lazily render (50 rows, "show more") | 352 KB page and 409 rows are hard to scan | None | P2 |
| HIST-06 | Link chart and table (click a row selects the point; selecting a point scrolls to and highlights the row); keyboard arrows move selection on the chart | Removes two disconnected views | None | P2 |
| HIST-07 | Presets "Today", "7 days", "Since last model change", "All"; label x-axis with dates (with a toggle for commit-index spacing) | Matches how people think about time; 227 commits/day distorts equal spacing | `date` stored | P2 |
| HIST-08 | Optional per-category lines (small multiples or toggles) so a reader can see where progress comes from | Overall alone hides which area moved | `categories` stored | P3 |
| HIST-09 | Touch and accessibility: `touch-action: pan-y` with pointer-drag inspection, text summary of the visible range, visible focus, series distinguished by marker as well as dash | Basic robustness | None | P3 |
| HIST-10 | Put a three-sentence plain-language header above the chart ("This is the overall estimate after each commit. Flat lines mean the work did not yet change a measured leaf. Steps from rescoping are marked") and move the glossary into tooltips | The explanation is currently at the bottom, in one dense paragraph | None | P1 |

## Interaction with the effort review

- Applying any of EFF-002/006/009 (+43 pts) lowers the overall by about 0.5 points in one commit with no work done. Do HIST-02 (model-revision markers) **before** applying weights, so the step is labeled when it appears.
- The zero-movement tail (14 commits) is itself evidence for EFF-011: verified work that no ledger leaf records cannot appear in history. HIST-01's "what moved" cell would make that visible and is a reasonable acceptance check for EFF-011.

## Unknowns

- Visual quality, spacing and mobile behavior (no browser here).
- Whether viewers use the table at all; the usability ranking assumes the chart and the delta question are the main use.
- Whether `history.json` can be extended: it is git-ignored and rebuilt by `--seed`, so adding fields to old entries means a reseed, which marks entries as replayed.
