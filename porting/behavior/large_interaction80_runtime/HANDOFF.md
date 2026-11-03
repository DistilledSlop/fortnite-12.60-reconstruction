# 0x14208b4f0 rough lift

Connected source-level draft of the pinned body, including gating, state and
float updates, both timer branches, three callback allocations/clones and
cleanup, final related-object checks, and the immediate and delayed actions.
It is uncounted and unvalidated; it is not a canonical implementation or parity
claim. The event, callback and registration stack layouts, omitted native call
arguments, vtable/allocator ABI, weak handles, stack-residue word, float return
ABI, and ownership contracts require resolution before integration. The compiler
security-cookie epilogue is not gameplay behavior and is not lifted.

Commands: `cat AGENTS.md`; bounded reads of `porting/README.md`,
`porting/HANDOFF.md`, `porting/master_plan.md`, and the complete selected C body;
`sha256sum porting/generated/callee_probe/blocking/0x14208b4f0.c` yielded the
hash in `sources.json`. No compiler, tests, runtime, generator, Ghidra,
promotion, staging or commit was run. Parent owns review and integration.

Review correction: split callback construction from clone/registration-frame
initialization so the second callback is constructed before manager and delay
loads, and those loads precede clone/init. One frame models the observed stack
slots from 108 through 80; the gaps at 107..101, fc..f9, e0, cc..c9, b0,
and a0 remain unwritten/unknown. The ec word is read through an unresolved
adapter only when allocation succeeds, and the frame is reused; its actual
value, exact ABI and possible hidden helper mutations remain open. No validation
was run for this correction.
