# Estimated AI water for Fortnite

Refreshed **2026-11-05T21:14:35+00:00**. Recorded inference usage only; missing usage remains unquantified.

Recorded **80,959 requests** across **12 model identifiers**. Token volume counts repeated cache reads across calls, not unique text.

**Central scenario: 310 US gallons. Scenario range: 0.53–8,900 US gallons.**

This is an assumption-driven scenario range, not a measurement, confidence interval, or bound on the entire project.

| Component | Low | Central | High |
|---|---:|---:|---:|
| Serving energy (kWh) | 20 | 240 | 3,000 |
| Cooling water (L) | 0 | 72 | 3,700 |
| Electricity generation water (L) | 2.0 | 1,100 | 30,000 |
| Total consumption (L) | 2.0 | 1,200 | 34,000 |

Known token overlaps are removed: output excludes separately reported reasoning. Antigravity input/cache separation remains an explicit assumption; see coverage and methodology.

| Provider / model | Proxy | Input | Cache read | Cache write | Output | Reasoning | Central gal | Low–high gal |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| anthropic / claude-haiku-5-5 | small | 13,684 | 1,023,492,207 | 21,741,574 | 4,060,089 | 6,521,071 | 23 | 0.050–580 |
| anthropic / claude-opus-5-5 | large | 351 | 12,045,353 | 817,443 | 59,558 | 18,855 | 1.6 | 0.0038–39 |
| anthropic / claude-sonnet-5 | standard | 26 | 2,066,984 | 351,476 | 5,509 | 320 | 0.21 | 0.00056–4.4 |
| anthropic / claude-sonnet-5-5 | standard | 7,348 | 497,074,557 | 5,738,651 | 1,541,212 | 839,580 | 22 | 0.036–630 |
| google / gemini-3.8-flash-n | small | 32,412,315 | 176,968,180 | 0 | 527,594 | 691,391 | 7.2 | 0.022–140 |
| local / deepseek-v4.1-flash-1017-coder-iq2xxs-remote | unknown | 39,815 | 494,039 | 0 | 4,533 | 0 | 0.033 | 0.000038–1.4 |
| local / qwen3.8-27b | standard | 920,245 | 52,968,139 | 0 | 637,297 | 0 | 2.8 | 0.0068–66 |
| openai / gpt-6-luna | small | 5,618,205 | 80,369,408 | 0 | 538,111 | 174,842 | 2.2 | 0.0057–49 |
| openai / gpt-6-sol | standard | 3,512,119 | 444,664,320 | 0 | 548,577 | 267,041 | 17 | 0.022–520 |
| openai / gpt-6.1-sol | standard | 40,244,467 | 3,263,568,384 | 0 | 5,371,026 | 4,710,085 | 130 | 0.21–4,000 |
| openai / gpt-6.1-terra | standard | 45,128,434 | 2,334,691,840 | 0 | 7,049,815 | 911,792 | 100 | 0.17–2,900 |
| unknown / deepseek-v4.1-flash-free | unknown | 6,132 | 1,664 | 0 | 12 | 17 | 0.0021 | 0.0000033–0.050 |

## Collection coverage

```json
{
  "antigravity": {
    "files": 500,
    "google_cache_exceeds_input": 4791,
    "google_output_components_match": 5925,
    "limitations": [
      "Only retained local conversations with a project workspace are included; missing databases are unknown, not zero usage.",
      "Generation-only records lack timestamps; step creation time is not necessarily request time.",
      "Workspace attribution is conversation-wide; mixed-workspace activity cannot be separated.",
      "Input and cache fields are treated as separate; their provider-specific semantics are not fully verified.",
      "Generator-only fallback without an identifier is excluded when step usage exists."
    ],
    "malformed_metadata": 0,
    "missing_databases": 323,
    "missing_ids": 0,
    "missing_usage_files": 0,
    "scoped_files": 500,
    "unreadable_files": 0,
    "usage_records": 5925
  },
  "claude": {
    "excluded_zero_records": 9,
    "files": 176,
    "limitations": [
      "Local session logs only; calls without usage or stable message IDs are excluded."
    ],
    "malformed_lines": 0,
    "missing_message_ids": 0,
    "missing_usage_files": 5,
    "scoped_files": 175,
    "unreadable_files": 0,
    "usage_records": 10021
  },
  "codex": {
    "fallback_files": 1279,
    "files": 1289,
    "limitations": [
      "Local session logs only; cumulative fallback cannot identify every request."
    ],
    "malformed_lines": 0,
    "missing_usage_files": 1,
    "scoped_files": 1280,
    "unaccounted_events": 0,
    "unreadable_files": 0,
    "usage_records": 64458
  },
  "gemini": {
    "files": 0,
    "limitations": [
      "No native Gemini session files found means usage is unavailable, not measured zero; sessions without attributable project roots are excluded.",
      "JSONL session format and unrecorded calls are not included; tool token overlap is unverified."
    ],
    "missing_usage_files": 0,
    "scoped_files": 0,
    "unaccounted_tool_tokens": 0,
    "unreadable_files": 0,
    "unscoped_files": 0,
    "usage_records": 0
  },
  "opencode": {
    "files": 1,
    "limitations": [
      "OpenCode normalizes stored output to exclude reasoning and input to exclude cache; older versions may differ.",
      "Only locally retained OpenCode messages are included; direct fnrunner/llama-server calls do not retain usage here."
    ],
    "missing_usage_files": 2,
    "scoped_files": 8,
    "unreadable_files": 0,
    "usage_records": 555
  }
}
```

Retained records from earlier refreshes whose source is no longer present: 0.

See [methodology and assumptions](WATER.md) and [machine-readable snapshot](water.json).
