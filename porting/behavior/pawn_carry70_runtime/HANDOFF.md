# Carry, drop and attachment behavior, batch70

Four uncounted full control-flow drafts in carry.rough.cpp. Source paths and
SHA-256 hashes in sources.json identify ignored decompiler inputs, not verified
original-byte or runtime evidence. Not build inputs or canonical proposals.

RestoreDBNOCollisionAfterDrop exec142a7e770 preserves the flag/hoister gates,
two unguarded capsule collision virtuals, optional DBNO interaction component,
and repeated interface/global lookup in tag order520,530,528. Each count change
conditionally calls virtual810; neither the tag map nor that virtual is faked.

LocalThrowCarriedPlayer exec142a7d6d0 checks matching hoister, clears it, updates
action and calls142336990 using repeated hoistee reads. Then it captures the
controller, clears hoistee, broadcasts a reread boolean, and only on role3 with
an ability interface removes the two configured tags. Global/interface providers
are reread in source order. Tag arrays are freed in +10 then +0 order. Extra
opaque tag storage parameter is a sketch allocation mechanism, not native ABI.

ServerHoistDBNOPlayer_Implementation native14234a360 retains controller/type and
target virtual11c8 eligibility, linked state writes, target-state cast, team-byte
comparison, 16-bit stat increment, root/global location copy, callback and force
update order. No new availability or distance check is invented.

RemoveAttachment exec142a7e4c0 retains local/authority guard and warning,
16-byte value clearing, component call/object flag manipulation, saved role,
and post-callback copy to the attachment RPC. The root sketch deliberately keeps
1440d4a00 arguments unknown as in the source, instead of assuming a Destroy API.
GUObjectArray chunk/index/unchecked path must be recovered before replacing its
adapter with UE code. Unknown field offsets remain explicit symbolic constants.

Unresolved: all adapter signatures/types; absolute field binding; interface
receiver/vtable types; gameplay tag update/notification contracts; callback
mutation; object-index flags and null semantics; temporary lifetimes and logging
arguments. FVector/source type is provisional. No source-completeness claim.
Root reviewed corresponding meaningful algorithm windows. No tests/compiler,
build, generation or Ghidra ran for these four. Canonical2074, lasttested2040,
nexttests3040. Root sole integration owner; bounded fork-none Terra6.1 medium/low
workers, rotation and economical reads persist. Preserve unrelated graphics.
