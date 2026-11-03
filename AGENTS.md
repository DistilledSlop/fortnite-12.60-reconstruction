# Project instructions

Read `porting/README.md` for the goal, then inspect current Git state and the relevant existing manifests/evidence before resuming work. Historical handoff and plan Markdown are reference material when needed, not routine batch status. The four dashboard milestones are the acceptance gates; the weighted progress percentage is an effort estimate.

Dashboard task weights estimate total implementation and verification effort, completed plus remaining. Importance, urgency, milestone criticality, and a desired progress percentage must not affect weights. Define accounting boundaries and avoid allocating the same effort to multiple leaves; report uncertain estimates explicitly.

The primary agent plans, orchestrates, and reviews, and works directly when useful. Use `gpt-6-luna` medium for small read-only tasks, `gpt-6.1-terra` medium or high for regular delegated work, and `gpt-6.1-sol` medium for really complex delegated tasks. Always set `fork_turns: "none"`. At most three subagents work at once. Assign one owner to `porting/native/`, promotion, staging, and UE builds; avoid concurrent generation/build operations.

Preserve existing edits. Stage specific paths, review and commit completed validated changes, and never force push. Private origin: https://github.com/DistilledSlop/fortnite-12.60-reconstruction. Use Codex <codex@openai.com> for new commits. The user-authorized identity-only history rewrite is complete; resume ordinary pushes. Keep credentials, recovered game data, and generated runtime artifacts out of Git. Never print or log key material. Do not hand-edit generated native code; change generator inputs and regenerate.

Use brief agent messages, Git commits, and existing evidence files to track routine progress. Do not create or update handoff Markdown for routine batches; create a handoff document only when necessary or requested. Distinguish a previous green baseline from current unvalidated edits, offline checksum parity from runtime compatibility, and extracted assets from usable content.

Preserve the current Luna/Terra/Sol model configuration in compaction summaries and handoffs. It supersedes the earlier `gpt-6-sol` implementation default.

The primary agent also performs difficult implementation, debugging, and problem solving directly when that best advances the goal, while continuing to plan, orchestrate, and review delegated work.

# Input and tool-output efficiency

Keep input-token use economical. Start with bounded searches and small head/tail or matched-context excerpts; cap result counts and tool output. Process large generated files with scripts and return only the needed fields or summaries. Read short log tails and relevant failure windows instead of repetitive full logs. Expand the read window only when the smaller view is insufficient; reading a reasonably sized full file is acceptable when needed. Preserve this preference alongside the subagent configuration in handoffs and compaction summaries.

# Bounded subagent tasks and rotation

Use subagents as bounded function or task runners, implementers, explorers, or researchers. Do not give one long-lived subagent an entire milestone or broad group of dashboard tiles. Follow-up turns are allowed while the bounded task still benefits from its context. At a reasonable stopping point where much prior context is no longer relevant, have the agent freeze a concise handoff and finish, then start a new subagent for the next bounded task. The primary agent retains planning, integration, review, and overall ownership. Preserve this preference alongside the model configuration and input-efficiency rules in handoffs and compaction summaries.

# Validation cadence

Optimize for implementing features as fast as possible without compromising any
quality or parity. Use that as the governing rule for deciding whether a test
resolves real correctness/parity uncertainty or wastes resources. Speed must come
from focused work and proportionate validation, never omitted behavior, fabricated
fallbacks, weakened requirements, or unsupported claims of completion.

Test according to changed risk and coherent integration batches, not each feature,
helper, or commit. Documentation/status commits need no build or runtime suite.
Keep source/reflection/generation invariants fail-closed, but batch their checks
with the implementation they protect. Do not add tests that merely mirror code.

Carry forward prior passing results when new work cannot invalidate them. Do not
recheck each previously recovered body after adding another independent body.
Repeat only checks affected by changed dependencies or shared contracts; checks
that are effectively instant may still run when convenient. Apply this rule to
gameplay and frontend work, and preserve it in handoffs and compaction summaries.

For a frontend batch, default to a client build and the real affected frontend
flow. Use existing focused checks for new ownership/GC, delegate mutation/reentry,
bitfield, serialization, or other difficult reconstructed contracts. Simple
accessors and ordinary calls usually need source review and coverage in the
combined flow, not separate probe frameworks. After a fixture correction, repeat
only the affected build/check unless a new failure justifies broader testing.

Build/test the server and run joins when networking, backend, replicated/shared
runtime contracts or server behavior changes, and at meaningful integration
checkpoints. UI-only changes do not automatically require them. Run broad retained
regressions for cross-cutting Engine/generator changes or completed integration
batches before accepting a new baseline/milestone; do not repeat unchanged suites
because another commit exists. Investigate actual crashes or uncertain ABI/lifetime
behavior with a debugger rather than proving ordinary calls one by one.

Keep source/build/run identifiers and useful failure logs. Full binary archival,
independent provenance audits and exhaustive evidence packages belong at meaningful
baseline/milestone checkpoints or when evidence is suspect, not every helper.
If verification repeatedly dominates without advancing the product path, consolidate
the implementation batch and reassess the check selection. Preserve this preference
in compaction summaries and handoffs alongside the delegation and input-use rules.

# Rough implementation passes

Prefer broad feature passes from batch-decompiled, typed/named Ghidra C to an
initial C++ lift, then use compilation and affected runtime flows to prioritize
refinement. Use assembly for specific ambiguities, not as the default starting
point for every missing helper. Neutral address-based names are acceptable; do
not spend time polishing names that do not affect correctness. Preserve explicit
unresolved calls/effects and concise reusable blocker notes; do not count drafts
or placeholders as verified bodies or claim their behavior is implemented.
Integrate completed gameplay code for the UI collaborator to use rather than
holding it back; the user authorized this shared Native integration.

For bulk native recovery, use Terra 6.1 low/medium effort workers for brute-force
work that automation cannot handle. Claude is also authorized as a workhorse;
give it bounded independent source batches with clear file ownership. Keep one
Native promotion/build stream and skip visual work until the user resumes it.

User checkpoint cadence: run builds/runtime/tests only after roughly 1,000 new
native function bodies. Last tested baseline: 2,040 bodies; next about 3,040.
Keep source generation fail-closed, commit and push completed source immediately,
and explicitly label bodies added since the tested baseline as build pending.

# External model workers

Antigravity CLI (`agy`) is available. Use Gemini 3.8 Flash for narrow, explicit bulk tasks with complete implementation standards and evidence review; reject stubs, omitted behavior, and unsupported defaults. Reserve Claude Opus 5.5 for short, tightly scoped hard problems: it consumes Antigravity Claude quota very quickly. Use Terra, Flash, and Sonnet for bulk implementation and long source-recovery runs. Keep authorized Claude work moving while its provider has usage. Do not interrupt an existing useful Sonnet run merely to change models.
