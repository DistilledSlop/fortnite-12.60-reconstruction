# Profile inventory group filters

`NativeFindProfileItems` (141ff83f0) and `NativeFindAccountItems1421c1800` (1421c1800) preserve the original dynamic group query before output reset, capacity release/reservation, source order, borrowed non-null shared records and virtual definition eligibility. The former accepts UFortItem; the latter UFortAccountItem. Neither compares the returned definition to the requested key or filters quantity.

Reviewed against cached C and assembly in `porting/generated/callee_probe/profile_inventory_lookup/0x141ff83f0.{c,asm}` and `0x1421c1800.{c,asm}`, and bounded recovery reports inventory-allitems214/inventory-singleitem215. Both helpers live in the Fortnite profile subclass so McpProfileSys does not acquire a reverse dependency on FortniteGame. The inherited wrapper retains actual base/overlay dispatch through the same backend pointer; no replacement index is introduced.

These are complete bodies under the original installed-backend precondition. Backend installation, mutation, destruction and GC remain incomplete, and usable runtime inventory/profile behavior is not established. Build pending.
