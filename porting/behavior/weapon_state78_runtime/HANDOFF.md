# Weapon state helper drafts

`rough.cpp` is a connected, unvalidated behavioral lift of 0x1426a07c0,
0x1426aeed0 and 0x142694dd0, reached from the batch75 weapon virtual drafts.
`sources.json` pins the three decompiled C files and SHA-256 digests. All
source-level branches, repeated class getters, live loop bounds, state writes,
cleanup calls and return paths are represented. The nested 0x142694dd0 call is
connected to its draft. These are not accepted native bodies or proven parity.

Unresolved: actual object/class and weak-pointer layout, iterator construction,
virtual-call ABI and implicit register arguments, helper signatures and ownership,
and the address-specific effect of the named stub. In particular, the manager
call dispatches from the vtable pointer at owner+0x4c8; the source supplies no
explicit receiver. It must not be treated as a call on that pointer as an object.
The `weak14_outer_indirect` and `indirect_class_private` adapters preserve the
decompiler's typed dereferences until UObject layout is recovered. The iterator
is not zero-initialized. The call to 0x1423d4240 preserves the class-index-derived
upper bits of its low-byte-one argument; assembly is needed to resolve its true
ABI. The three zero-argument owner/global calls retain omitted arguments rather
than fabricating them. Low-byte predicates are explicit. No compiler, build,
test, Ghidra, generation, canonical promotion, staging or commit was run.
