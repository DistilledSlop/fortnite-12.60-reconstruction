# Pickup native rough batch75

Two uncounted connected C++ drafts cover native14203cda0 TossPickup and
native1420279f0 BlueprintSetPickupTarget. sources.json pins the ignored source
files by SHA-256. Their reflected exec wrappers142a2bad0 and142a2a600 were also
probed but are not counted as additional recovered behavior.

Toss retains the authority gate, predicate-dependent setup, live related-object
lookup after virtual720, class check, helper ordering, flag updates and final
virtual5e8. Target setup retains the early null exit, raw8+4-byte temporary copy,
and vtable capture before helper14202dc10. No vector or reflected type is inferred.

Blockers: virtual720, virtual640 and virtual5e8 targets remain unresolved. All
externs require real ABI/effect recovery. The missing arguments of143f7abb0 and
14203e6d0, original class-index lookup layout, scratch initialization/overlap,
and type of global14813b798 remain open. GlobalBits preserves a raw32-bit value
without asserting a float calling convention. These drafts are neither canonical
native inputs nor proof of gameplay parity. No compilation, generation or tests.
