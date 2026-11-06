# Pawn gameplay-cue RPC forwarding

Ten source-complete implementations forward to the live AbilitySystemComponent
when present. They invoke its virtual `_Implementation` method, preserving local
prediction handling in the existing engine implementation, rather than sending
a second RPC. Do not replace the direct field read with the virtual ASC getter.
Argument copies and destruction use the actual UE types. The decompiler imported
several parameter names/types in reverse order; reflected wrapper marshaling and
matching ASC exec virtual slots establish the source argument order.

Probes: /tmp/pawn-cues66 and /tmp/pawn-cues66b. Each implementation's original
body bytes, pawn virtual slots and matching ASC exec windows are pinned. Complete
RPC reflection records for both owners are guarded. Source recovered; no build,
runtime or tests run. Original source ABI details remain compiler-managed.
