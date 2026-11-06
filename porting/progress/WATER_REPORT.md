# Estimated AI water for Fortnite

Refreshed **2026-11-06T23:20:11+00:00**. Recorded inference usage only; missing usage remains unquantified.

Recorded **86,845 requests** across **13 model identifiers**. Token volume counts repeated cache reads across calls, not unique text.

**Central scenario: 320 US gallons. Scenario range: 0.55–9,300 US gallons.**

This is an assumption-driven scenario range, not a measurement, confidence interval, or bound on the entire project.

| Component | Low | Central | High |
|---|---:|---:|---:|
| Serving energy (kWh) | 21 | 250 | 3,100 |
| Cooling water (L) | 0 | 76 | 3,900 |
| Electricity generation water (L) | 2.1 | 1,200 | 31,000 |
| Total consumption (L) | 2.1 | 1,200 | 35,000 |

Known token overlaps are removed: output excludes separately reported reasoning. Antigravity input/cache separation remains an explicit assumption; see coverage and methodology.

| Provider / model | Proxy | Input | Cache read | Cache write | Output | Reasoning | Central gal | Low–high gal |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| anthropic / claude-haiku-5-5 | small | 17,084 | 1,196,682,172 | 26,082,196 | 4,827,961 | 7,570,180 | 27 | 0.059–680 |
| anthropic / claude-opus-5-5 | large | 351 | 12,045,353 | 817,443 | 59,558 | 18,855 | 1.6 | 0.0038–39 |
| anthropic / claude-sonnet-5 | standard | 26 | 2,066,984 | 351,476 | 5,509 | 320 | 0.21 | 0.00056–4.4 |
| anthropic / claude-sonnet-5-5 | standard | 7,676 | 506,991,647 | 6,054,274 | 1,590,014 | 853,761 | 22 | 0.038–640 |
| google / gemini-3.8-flash-n | small | 32,979,726 | 189,796,277 | 0 | 542,180 | 711,915 | 7.4 | 0.023–140 |
| google / gemini-3.8-flash-tiered | small | 3,064,990 | 35,267,485 | 0 | 46,599 | 199,238 | 1.00 | 0.0026–22 |
| local / deepseek-v4.1-flash-1017-coder-iq2xxs-remote | unknown | 39,815 | 494,039 | 0 | 4,533 | 0 | 0.033 | 0.000038–1.4 |
| local / qwen3.8-27b | standard | 920,245 | 52,968,139 | 0 | 637,297 | 0 | 2.8 | 0.0068–66 |
| openai / gpt-6-luna | small | 5,793,786 | 81,822,464 | 0 | 551,332 | 178,116 | 2.2 | 0.0059–51 |
| openai / gpt-6-sol | standard | 3,512,119 | 444,664,320 | 0 | 548,577 | 267,041 | 17 | 0.022–520 |
| openai / gpt-6.1-sol | standard | 40,325,939 | 3,275,991,552 | 0 | 5,393,954 | 4,724,363 | 130 | 0.21–4,000 |
| openai / gpt-6.1-terra | standard | 48,494,476 | 2,519,572,480 | 0 | 7,517,705 | 997,129 | 110 | 0.19–3,100 |
| unknown / deepseek-v4.1-flash-free | unknown | 267,913 | 8,873,600 | 0 | 27,804 | 41,665 | 0.48 | 0.00042–24 |

## Collection coverage

```json
{
  "antigravity": {
    "files": 500,
    "google_cache_exceeds_input": 5504,
    "google_output_components_match": 6717,
    "limitations": [
      "Only retained local conversations with a project workspace are included; missing databases are unknown, not zero usage.",
      "Generation-only records lack timestamps; step creation time is not necessarily request time.",
      "Workspace attribution is conversation-wide; mixed-workspace activity cannot be separated.",
      "Input and cache fields are treated as separate; their provider-specific semantics are not fully verified.",
      "Generator-only fallback without an identifier is excluded when step usage exists."
    ],
    "malformed_metadata": 0,
    "missing_databases": 336,
    "missing_ids": 3,
    "missing_usage_files": 0,
    "scoped_files": 500,
    "unreadable_files": 0,
    "usage_records": 6717
  },
  "claude": {
    "excluded_zero_records": 10,
    "files": 216,
    "limitations": [
      "Local session logs only; calls without usage or stable message IDs are excluded."
    ],
    "malformed_lines": 0,
    "missing_message_ids": 0,
    "missing_usage_files": 5,
    "scoped_files": 215,
    "unreadable_files": 0,
    "usage_records": 11807
  },
  "codex": {
    "fallback_files": 1374,
    "files": 1382,
    "limitations": [
      "Local session logs only; cumulative fallback cannot identify every request."
    ],
    "malformed_lines": 0,
    "missing_usage_files": 1,
    "scoped_files": 1375,
    "unaccounted_events": 0,
    "unreadable_files": 0,
    "usage_records": 67590
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
    "scoped_files": 11,
    "unreadable_files": 0,
    "usage_records": 677
  }
}
```

Retained records from earlier refreshes whose source is no longer present: 54.

See [methodology and assumptions](WATER.md) and [machine-readable snapshot](water.json).
