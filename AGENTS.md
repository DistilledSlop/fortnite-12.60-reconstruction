# Project instructions

Read `porting/README.md` for the goal, then the latest snapshot at the top of `porting/HANDOFF.md` and `porting/master_plan.md` before resuming work. The four dashboard milestones are the acceptance gates; the weighted progress percentage is an effort estimate.

The primary agent plans, orchestrates, and reviews, and works directly when useful. Use `gpt-6-luna` medium for small read-only tasks, `gpt-6.1-terra` medium or high for regular delegated work, and `gpt-6.1-sol` medium for really complex delegated tasks. Always set `fork_turns: "none"`. At most three subagents work at once. Assign one owner to `porting/native/`, promotion, staging, and UE builds; avoid concurrent generation/build operations.

Preserve existing edits. Stage specific paths, review and commit completed validated changes, and never force push. No remote is currently configured. Keep credentials, recovered game data, and generated runtime artifacts out of Git. Never print or log key material. Do not hand-edit generated native code; change generator inputs and regenerate.

Record exact commands, outcomes, and artifact paths in the handoff. Distinguish a previous green baseline from current unvalidated edits, offline checksum parity from runtime compatibility, and extracted assets from usable content. Update the handoff before stopping so the next session can resume without reconstructing the history.

Preserve the current Luna/Terra/Sol model configuration in compaction summaries and handoffs. It supersedes the earlier `gpt-6-sol` implementation default.

The primary agent also performs difficult implementation, debugging, and problem solving directly when that best advances the goal, while continuing to plan, orchestrate, and review delegated work.

# Input and tool-output efficiency

Keep input-token use economical. Start with bounded searches and small head/tail or matched-context excerpts; cap result counts and tool output. Process large generated files with scripts and return only the needed fields or summaries. Read short log tails and relevant failure windows instead of repetitive full logs. Expand the read window only when the smaller view is insufficient; reading a reasonably sized full file is acceptable when needed. Preserve this preference alongside the subagent configuration in handoffs and compaction summaries.
