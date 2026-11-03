# Player-state broad batch71

Five uncounted control-flow drafts in `behavior.rough.cpp`, with ignored improved
decompiler source hashes in `sources.json`. No build, tests, Ghidra, generation,
or canonical edits were performed. Hashes identify decompiler inputs, not
original binary bytes or parity evidence. The four RPC implementation entries in
the manifest are skipped with null inc_path, not ported; no matching existing
behavior body was found. OnRep has no matching manifest name. Root owns review
and integration; tested baseline 2040, canonical 2074, next tests ~3040.

OnRep_ChangeTeamInfo covers the validity and class guards, victim/killer names,
optional level formatting, all four ordered named text arguments, lookup/format,
world/game-state dispatch and cleanup. Sparse-map insert and text ownership are
explicit adapters; adapters must reproduce allocation, duplicate/hash behavior,
strong/weak release and cleanup ordering. The decompiler's printf call does not
show its integer variadic operand; it remains unresolved. Field and type layout,
text conversion temporary aliasing, string empty representation, native map
allocation and destruction, and actual callback signatures remain unresolved.

The RPCs preserve cast checks, report call order and rereads, optional runtime
flag, downstream notification, kill-feed 0x30 iteration and reference release.
The 36-byte output is opaque; array Data indirection and refcount control-block
layout require ABI confirmation. No draft is proposed as a completed candidate.
