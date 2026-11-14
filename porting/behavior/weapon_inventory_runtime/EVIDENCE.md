# Inventory query leaves

Weapon virtual +820 `142698590` repeats the dynamic ammo-definition query, rejects null data/result, then returns true for a non-ammo definition or the ammo definition's reflected full-byte bIsConsumed. Definition virtual +380 `14202a5f0` returns reflected bAllowMultipleStacks, distinct from stack-size-based IsStackable. Constructor projections are documented with the corresponding generator policy. The 23 constructor-proven weapon tables retain dynamic +820 dispatch; indexed definition descendants are recorded in ignored `generated/agents/reload-inventory209/`. Independent review: `inventory-leaf-review216`.

The reserve-ammo body and inventory interface resolver remain unintegrated: actual pawn interface bindings and concrete world/profile lookup implementations are required. These two complete leaves do not imply full reload behavior. Build/runtime acceptance is pending.
