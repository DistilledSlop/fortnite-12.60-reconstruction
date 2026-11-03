# Pawn vehicle 70 handoff

Two uncounted, unvalidated control-flow drafts in PROPOSAL.cpp: ServerSetAttachment_Implementation and ApplyWrapToSaveTheWorldHoverboard. Neither is source-complete or suitable for promotion. Both retain unresolved native calls, field bindings and ABI. The attachment logging adapter encompasses formatted pawn-name lifetime; component destruction's decompiled arguments and GUObjectArray lookup/mark must be recovered. The copied 16-byte attachment may carry ownership or reference semantics not yet established. Hoverboard's context, variant entries and apply arguments need exact layout, constructors/destructors, copy semantics and native signatures. The deep-copy adapter covers only the shown reserve and 0x48-byte entry copies; cleanup adapters preserve the decompiled free order. Inlined null mesh/pawn diagnostics are unreachable after the initial dereference and null mesh return. Material selector uses only the low-byte boolean from v450, with null material yielding false.

Dedicated follow-up passes completed OnRep_InVehicle and OnRep_CosmeticLoadout
as separate full control-flow drafts, with their own handoffs. Vehicle source
connects all paths through source103–1177 and remains unvalidated/uncounted.
Cosmetic source covers all requests and final dispatch; root review corrected
loadout-relative handle bases and cleanup when returned pointer aliases destination.
All four inputs and hashes are in sources.json; no parked input remains here.
The manifest has an existing ServerSetAttachment_Implementation metadata entry;
none of this directory is a promotion-ready implementation.

No compiler, tests, build, generator, Ghidra, canonical edits or commits were run. Baseline per assignment: tested 2,040; canonical 2,074; next tests about 3,040. Parent owns integration and review. Preserve Luna medium small read-only, Terra medium/high regular, Sol medium complex; fork none and economical bounded reads.

All bounded workers finished. Root reviewed control flow and unresolved contracts;
no source-complete or runtime-parity claim is made.

Batch87 follow-up: hoverboard_wrap87_runtime/ expands the previously opaque record append and destruction adapters, preserves byte-sized initialization with unwritten gaps, and records the complete0xf0 context extent. Its call/layout contracts remain open; use that more explicit draft for the next hoverboard pass.
