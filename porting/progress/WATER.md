# Fortnite AI water estimate

The README badge is an estimate of **operational freshwater consumption for AI
inference used on the Fortnite reconstruction**. It includes on-site cooling and
water consumed to generate electricity, reported separately. It does not measure
water withdrawals or local water scarcity. Training, hardware manufacturing,
networks, developer devices, compilation, and game runtime are outside the scope.

See [the current backfill and model breakdown](WATER_REPORT.md),
[the JSON snapshot](water.json), and [editable scenario assumptions](../water_profiles.json).

## What the range means

Low, central, and high are explicit **scenarios**, not a confidence interval.
The actual result can lie outside the range. Missing usage is unquantified; the
high scenario does not put an upper bound on calls whose logs no longer exist.
Unknown model identity and unknown token categories are coverage gaps, not known
zero consumption. The report retains original model identifiers and identifies
the workload proxy used for each one.

Provider APIs generally expose usage, not the energy, hardware utilization,
serving region, or water intensity of individual calls. Consequently these
coefficients are transparent engineering assumptions, **not measured rates for
the named proprietary models**. Exact token accounting improves the estimate;
it cannot make those missing physical measurements exact.

## Energy assumptions

All rates below are **kWh per million tokens**, inclusive of serving infrastructure
overhead once. Input and output use different rates. Generated reasoning uses the
output rate, and is separated from visible output when the log exposes it.

| Workload proxy | Typical name matches | Input low / central / high | Output or reasoning low / central / high |
|---|---|---|---|
| Small | GPT Luna, Claude Haiku, Gemini Flash | 0.02 / 0.1 / 0.5 | 0.1 / 0.5 / 2.5 |
| Standard | GPT Sol/Terra, Claude Sonnet, Gemini Pro, local Qwen | 0.05 / 0.25 / 1.25 | 0.25 / 1.25 / 6.25 |
| Large | GPT Astra, Claude Opus | 0.1 / 0.5 / 2.5 | 0.5 / 2.5 / 12.5 |
| Unknown | Unrecognized or unrecorded model | 0.02 / 0.25 / 2.5 | 0.1 / 1.25 / 12.5 |

These assignments describe assumed serving workloads, not known parameter counts.
Version names within a family share a proxy until better evidence is available.
`model_overrides` in `water_profiles.json` can set an exact model's `tier`,
`input_kwh_per_million`, and/or `output_kwh_per_million` arrays without changing
collection code. A rate should only be called measured when supporting telemetry
for that model and workload exists.

Cache reads use **0.02 / 0.1 / 0.3 times the uncached input rate**; cache writes
use **1.0 / 1.2 / 1.5 times** that rate. These are uncertain compute proxies, not
API price ratios. A cache write replaces the corresponding uncached input cost;
it is not added a second time. Reads are not free: cache access and subsequent
attention still require serving work. Separate cache retention/storage lifetime,
nonlinear long-context attention, routing/helper models, and unreported speculative
work are not explicitly modeled. Unknown thinking counts are not invented or
added on top of an output total that already includes thinking.

The local-Qwen proxy is especially uncertain: it lacks wall-meter telemetry and
can vary strongly with quantization, CPU/GPU offload and throughput. Local serving
energy includes the host and has zero assumed on-site cooling water (air cooling).
Its electricity-generation water remains included. A future wall-meter estimate
should replace this proxy, not be added to it.

## Water assumptions

For cloud models:

```text
cooling liters = serving kWh / assumed PUE × on-site WUE
power-generation liters = serving kWh × electricity water intensity
total liters = cooling liters + power-generation liters
US gallons = total liters / 3.785411784
```

| Assumption | Low | Central | High |
|---|---:|---:|---:|
| Cloud PUE (facility energy / IT energy) | 1.2 | 1.2 | 1.2 |
| On-site consumption (L / IT kWh) | 0 | 0.36 | 1.5 |
| Electricity-generation consumption (L / facility kWh) | 0.1 | 4.52 | 10 |

PUE 1.2 and the low/high intensities are scenario choices. The central water
factors use historical US data-center averages as proxies, not the location of
any specific provider request. In particular, the electricity mix may differ
substantially for Google's, OpenAI's, Anthropic's, or this machine's actual supply.
No water-replenishment or carbon-credit offsets are subtracted. These are
consumption factors, not withdrawal factors, and should not be mixed with them.

Sources and their limits:

- [LBNL, 2024 United States Data Center Energy Usage Report](https://eta-publications.lbl.gov/sites/default/files/2024-12/lbnl-2024-united-states-data-center-energy-usage-report.pdf): approximately 0.36 L/IT-kWh direct WUE and 4.52 L/facility-kWh electricity water consumption in 2023. Historical US averages support the central water scenario only.
- [Google, Measuring the environmental impact of delivering AI at Google Scale](https://services.google.com/fh/files/misc/measuring_the_environmental_impact_of_delivering_ai_at_google_scale.pdf): 0.24 Wh and 0.26 mL for the median Gemini Apps text prompt in May 2025. The water figure concerns on-site cooling; it is neither an API/model-specific token rate nor a total including electricity-generation water. Its on-site WUE uses IT electricity as denominator.
- [Energy Use of AI Inference, Efficiency Pathways, and Test-Time Scaling](https://arxiv.org/abs/2509.20241): illustrates dependence on throughput, power, overhead and test-time scaling. It does not calibrate these model-specific token proxies.
- [From Prompts to Power](https://arxiv.org/abs/2511.05597): empirical evidence that input/output length, architecture, hardware and batching affect inference energy; it does not measure the project models.

The original badge used [Sam Altman's average ChatGPT query figure](https://blog.samaltman.com/the-gentle-singularity)
divided by an assumed 400 tokens. That conversion is retired. The new result
covers a different accounting boundary and more usage, so changes from 20.6 gal
should not be interpreted as a measured increase in physical water use.

## Collection and refresh

Run on the host where ccusage sees the agent usage (the default source):

```sh
python3 porting/water_badge.py
```

The default source is ccusage, described in [ccusage source](#ccusage-source-default)
below. It is not project-scoped. The project-scoped log collectors remain available
with `--logs` and scope usage to the main checkout, registered reconstruction
worktrees, the Gemini/local-model worker packages, supporting Unreal Engine,
FModel, Ghidra and SDK-dump directories, and sessions whose recorded
workspace is exactly the parent `/opt/fortnite` project directory. Arbitrary
sibling directories under that parent are excluded. The separate `Fortnite14Research`
checkout is excluded. Calls from other projects are excluded, even if their
messages mention Fortnite. Calls made on another host, erased logs, and work
from a projectless directory without recorded project attribution cannot be
recovered by path matching alone.

The usage ledger retains sanitized counts, model/provider identifiers and hashed
record identities so future refreshes do not lose accounting when logs rotate.
It contains no prompts, responses, credentials or source paths. Refresh replaces
matching record identities rather than adding the same call again. Rebuild from
only currently available logs with `--replace-ledger` when intentionally correcting
the accounting boundary. This discards retained history and is not the normal
refresh command. Output location and scenario profiles can be overridden with
`--output-dir` and `--profiles` for review without changing the tracked snapshot.
On another host, use `--project-root` for its Fortnite parent workspace.
`--codex-root`, `--claude-root`, `--gemini-root`, `--antigravity-root` and
`--opencode-db` override local log locations, including archived log copies.

Publishing reads the saved `water.json` snapshot only; it never accesses private
logs. Refreshing the README water badge is separate from unrelated reconstruction
progress publication. The snapshot carries its refresh time and collection gaps.

Use `python3 porting/water_badge.py --publish` to refresh both the snapshot and
only the README water badge. This preserves independently edited reconstruction
progress. `python3 porting/progress/publish.py --water-only --check` verifies that
badge against the saved snapshot without touching logs.

The compact tracked ledger is `water_usage.json.gz` (deterministic gzip of JSON,
not raw session logs). The collector reads:

- Codex per-response usage with model attribution from turn context, or cumulative
  counter deltas as a fallback. Response IDs deduplicate repeated/forked logs.
- Claude assistant-message usage, merging repeated streaming snapshots by message
  identity. Input excludes cache categories; output includes thinking, so reported
  thinking is split from output rather than added again.
- OpenCode's read-only SQLite message usage, including locally served Qwen and
  other local models. Its stored output already excludes separately stored
  reasoning. Direct llama-server/fnrunner runs without retained usage remain a gap.
- Antigravity's read-only conversation metadata, scoped using workspace URIs in
  its summary database. Step and generation copies of the same usage are merged
  by request identity. The decoder reads token/model metadata, not prompts or
  responses. Its wire field numbers come from the installed CLI's embedded schema;
  the decoder records that binary's SHA-256 for provenance. Unknown fields are
  skipped and malformed metadata is reported. A future incompatible schema
  requires revalidation. Missing conversation databases are counted explicitly.
- Native Gemini CLI saved JSON sessions, using workspace metadata or a verified
  project-path hash. No such native sessions were available for the initial
  backfill; the actual Gemini backfill comes from Antigravity.

Antigravity exposes an internal normalized usage format rather than the native
Gemini response. In this backfill, 4,791 of 5,925 requests have cache-read counts
larger than the reported input count. We therefore retain input and cache reads
as separate categories rather than subtracting cache reads as if input were an
inclusive native Gemini total. This is an explicit semantic assumption. All
5,925 requests satisfy total output = response output + thinking output, which
supports splitting reasoning once. The machine-readable coverage records these
checks. Metadata with unknown provider semantics remains flagged rather than
silently claiming a verified split.

The collectors do not invoke paid APIs or resume any stopped provider workers.
Only aggregate usage metadata is published. Official token-accounting references:
[Claude thinking usage](https://platform.claude.com/docs/en/build-with-claude/thinking-steering-and-cost),
[OpenCode normalization](https://github.com/anomalyco/opencode/blob/dev/packages/opencode/src/session/session.ts),
and [Gemini CLI saved sessions](https://github.com/google-gemini/gemini-cli/blob/main/docs/cli/session-management.md).

For another provider or host, export normalized JSONL and pass
`--import-usage /path/to/export.jsonl`. Each record must explicitly include
`"project":"fortnite-12.60-reconstruction"`, a provider and model identifier,
`source` (24–64 lowercase hex characters from a stable request identity hash),
nonnegative integer token fields matching the report, and `requests`. Optional
`timestamp` must include a timezone. Counts must already be disjoint; matching
source hashes replace existing identities rather than adding duplicates. Keep
identities consistent with the native collector when importing overlapping calls.
The importer whitelists published fields and strips any extra private content.
Do not estimate token counts from character counts and present them as measured
usage. Missing history remains a limitation until reliable records are supplied.

## ccusage source (default)

```sh
python3 porting/water_badge.py --publish
```

This is the default. It reads `ccusage daily -j --by-agent --offline --no-cost -z UTC`
for every agent ccusage detects on the machine (Claude, Codex, OpenCode and
Antigravity at the last refresh) from the first commit date (UTC day of
`git log --reverse --format=%cI | head -1`, 2026-10-30) through today, with
local models excluded (below). `--since` and `--until` override the window, and
`--ccusage-bin` selects the executable. `--ccusage` is accepted as a no-op alias.
`--logs` selects the project-scoped log collectors and rejects these three options.

Limitations and choices:

- Not project-scoped. ccusage has no project filter, so the window counts all
  cloud agent usage on this machine, including non-Fortnite work. The snapshot
  `scope`, the report and `limitations` say so. `--logs` remains the
  project-scoped measurement.
- Ledger replacement. Every ledger row inside the window is replaced by ccusage
  rows, whatever produced it. Rows outside the window are kept as history. A
  `--logs` refresh rebuilds from logs and drops ccusage-marked rows (reported on
  stderr), so the two sources never count the same usage twice. The earlier
  log-derived ledger remains in git history.
- Token mapping. ccusage `inputTokens` excludes cache reads and writes, which it
  reports separately, and `outputTokens` is used as reported. Its `totalTokens`
  can exceed those fields on some agent rows (observed on antigravity and
  opencode, about 0.01% of the window). The excess is recorded as reasoning and
  charged at the output rate, so the estimate is unaffected by the split and
  token totals reconcile to ccusage `totalTokens`. ccusage has no reasoning field.
  Excluded local models are removed before this excess is allocated, so their
  tokens never share in it. The residual is allocated across kept models only.
- Requests. ccusage reports none. The report says so and the snapshot records
  `requests_reported: false` with `recorded_requests: null`, never zero.
- Timestamps. Each (day, agent, model) row is stamped 12:00 UTC, which makes
  `usage_first_at` and `usage_last_at` day-resolution.
- Local models. Models whose OpenCode DB rows have providerID `llamacpp` or
  `local` (read-only, table `session_message`), plus `local_models` in
  `porting/water_profiles.json`, are dropped by name together with their
  per-day rows. At the last refresh these were qwen3.8-27b,
  deepseek-v4.1-flash-1017-coder-iq2xxs-remote and ds41. Their token totals are
  listed in the report and `coverage.ccusage.local_excluded_tokens`. A missing or
  unreadable DB leaves the profile list in force (`opencode_db_status` records
  which). Edge: a model name used by both local and cloud providers is excluded
  everywhere, so its cloud usage is dropped too. ccusage rows carry no provider,
  so name matching is the only available key.
- Providers. Model names beginning `claude-` map to `anthropic`, `gpt-` to
  `openai`, and `gemini-` to `google`. Everything else is `unknown`. The
  estimator treats `unknown` as cloud-hosted, so `unknown` rows that are not
  local (for example OpenCode's cloud deepseek-v4.1-flash-free, or Antigravity's
  model_placeholder_m322) are estimated as cloud.
- Live usage. Active sessions keep writing logs, so a fresh ccusage run can
  differ slightly from a snapshot made a few minutes earlier. Totals were checked
  exactly against a captured ccusage payload.
