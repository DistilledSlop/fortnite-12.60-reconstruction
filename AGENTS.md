# Project instructions

Read `porting/README.md` for the goal, then the latest snapshot at the top of `porting/HANDOFF.md` and `porting/master_plan.md` before resuming work. The four dashboard milestones are the acceptance gates; the weighted progress percentage is an effort estimate.

The primary agent plans, orchestrates, and reviews. Delegate substantive work to `gpt-6-sol` subagents with `fork_turns: "none"`, using medium reasoning for complex investigation and implementation and low reasoning for simple bounded tasks. At most three subagents work at once. Assign one owner to `porting/native/`, promotion, staging, and UE builds; avoid concurrent generation/build operations.

Preserve existing edits. Stage specific paths, review and commit completed validated changes, and never force push. No remote is currently configured. Keep credentials, recovered game data, and generated runtime artifacts out of Git. Never print or log key material. Do not hand-edit generated native code; change generator inputs and regenerate.

Record exact commands, outcomes, and artifact paths in the handoff. Distinguish a previous green baseline from current unvalidated edits, offline checksum parity from runtime compatibility, and extracted assets from usable content. Update the handoff before stopping so the next session can resume without reconstructing the history.

Use `gpt-6-luna` with medium reasoning for read-only investigation and review; use `gpt-6-sol` with medium or low reasoning for implementation, tests, documentation changes, and operations. Keep `fork_turns: "none"` and preserve this configuration in compaction summaries and handoffs.
