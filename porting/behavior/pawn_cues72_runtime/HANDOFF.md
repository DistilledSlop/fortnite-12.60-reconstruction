# Pawn cues batch72 rough lifts

Two uncounted, non-build-input drafts cover the full recovered exec body of StopBatchedLoopingWeaponActivation (142a20ea0) and the separate RPC implementation NetMulticast_Athena_BatchedDamageCues_Implementation (142334fc0). This is not a repeat of batch70 helper 142345360: the RPC has its own two-dispatch and input-mutation logic.

Stop preserves signed gap8[51a] gating, live ASC reread after global lookup, globals +1518 cue argument, event type3, native c8..14 stack descriptor initializers, four frees in 68/78/88/98 stack-slot order, descriptor vtable reset, then non-atomic shared-control strong/weak release. Actual descriptor types, global relocation and all helper/vtable prototypes remain unresolved. The contiguous storage representation expresses observed stack layout only; no engine type/ABI claim follows.

Damage cue RPC preserves mode1 exclusion, complete 16/68-byte copies, the first callback, live input reread, conditional second actor substitution, flag-byte mutations at 1e then1d from41 then40, complete recopy and second callback. Improved decompiler formal names contradict the widths used; raw positional names deliberately retain this uncertainty. Struct layouts, mode receiver/calling convention, helper contracts and potential aliased inputs need native reconciliation before promotion. memcpy expresses the observed fixed loads/stores at a behavioral level; overlapping native arguments or callbacks exposing local storage require ABI review.

sources.json fingerprints ignored generated decompiler inputs, not original binary proof. No tests/compiler/Ghidra/generation or canonical changes. Canonical counts unchanged; next tests remain3040 after last tested2040.
