# Estimated AI water for Fortnite

Refreshed **2026-11-06T23:38:30+00:00**. Recorded inference usage only; missing usage remains unquantified.

Request counts are not reported by ccusage; usage spans **12 model identifiers**. Token volume counts repeated cache reads across calls, not unique text. Excluded local models (by name, not counted): deepseek-v4.1-flash-1017-coder-iq2xxs-remote 0.5M tokens; ds41 0.1M tokens; qwen3.8-27b 65.0M tokens.

**Central scenario: 350 US gallons. Scenario range: 0.59–10,000 US gallons.**

This is an assumption-driven scenario range, not a measurement, confidence interval, or bound on the entire project.

| Component | Low | Central | High |
|---|---:|---:|---:|
| Serving energy (kWh) | 22 | 280 | 3,500 |
| Cooling water (L) | 0 | 83 | 4,400 |
| Electricity generation water (L) | 2.2 | 1,300 | 35,000 |
| Total consumption (L) | 2.2 | 1,300 | 39,000 |

Known token overlaps are removed: output excludes separately reported reasoning. Antigravity input/cache separation remains an explicit assumption; see coverage and methodology.

| Provider / model | Client | Proxy | Input | Cache read | Cache write | Output | Reasoning | Central gal | Low–high gal |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| anthropic / claude-haiku-5-5 | claude | small | 17,206 | 1,203,263,641 | 26,265,334 | 12,516,954 | 0 | 27 | 0.060–690 |
| anthropic / claude-opus-5-5 | claude | large | 351 | 12,045,353 | 817,443 | 78,413 | 0 | 1.6 | 0.0038–39 |
| anthropic / claude-sonnet-5 | claude | standard | 26 | 2,066,984 | 351,476 | 5,829 | 0 | 0.21 | 0.00056–4.4 |
| anthropic / claude-sonnet-5-5 | claude | standard | 7,856 | 514,317,647 | 6,133,529 | 2,468,099 | 0 | 23 | 0.038–650 |
| google / gemini-3.8-flash-low | antigravity | small | 28,252,154 | 92,293,564 | 0 | 363,962 | 479,707 | 5.3 | 0.018–89 |
| google / gemini-3.8-flash-medium | antigravity | small | 4,136,441 | 97,268,453 | 0 | 173,298 | 263,298 | 2.0 | 0.0044–53 |
| openai / gpt-6-luna | codex | small | 5,828,022 | 82,001,152 | 0 | 730,965 | 0 | 2.3 | 0.0059–51 |
| openai / gpt-6-sol | codex | standard | 8,317,318 | 1,264,925,184 | 0 | 2,590,348 | 0 | 47 | 0.062–1,500 |
| openai / gpt-6.1-sol | codex | standard | 40,472,646 | 3,287,191,296 | 0 | 10,163,931 | 0 | 130 | 0.21–4,000 |
| openai / gpt-6.1-terra | codex | standard | 48,674,826 | 2,526,706,048 | 0 | 8,431,607 | 0 | 110 | 0.19–3,200 |
| unknown / deepseek-v4.1-flash-free | opencode | unknown | 266,605 | 8,703,744 | 0 | 25,500 | 41,665 | 0.47 | 0.00041–24 |
| unknown / model_placeholder_m322 | antigravity | unknown | 3,064,990 | 35,267,485 | 0 | 46,599 | 167,373 | 2.4 | 0.0026–110 |

## Collection coverage

```json
{
  "ccusage": {
    "agents": [
      "antigravity",
      "claude",
      "codex",
      "opencode"
    ],
    "command": "ccusage daily -j --since 2026-10-30 --until 2026-11-06 --by-agent --offline --no-cost -z UTC",
    "daily_rows": 8,
    "local_excluded_tokens": {
      "deepseek-v4.1-flash-1017-coder-iq2xxs-remote": 538387,
      "ds41": 141932,
      "qwen3.8-27b": 65044577
    },
    "local_excluded_total": 65724896,
    "local_models": [
      "deepseek-v4.1-flash-1017-coder-iq2xxs-remote",
      "ds41",
      "qwen3.8-27b"
    ],
    "opencode_db": "read",
    "project_scoping": "none: all agent usage detected on this machine in the window",
    "request_counts": "not reported by ccusage",
    "unattributed_residual_tokens": 0,
    "usage_records": 41,
    "window": [
      "2026-10-30",
      "2026-11-06"
    ]
  }
}
```

Retained records from earlier refreshes whose source is no longer present: 0.

See [methodology and assumptions](WATER.md) and [machine-readable snapshot](water.json).
