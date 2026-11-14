# Inventory query leaves

Weapon virtual +820 `142698590` repeats the dynamic ammo-definition query, rejects null data/result, then returns true for a non-ammo definition or the ammo definition's reflected full-byte bIsConsumed. Definition virtual +380 `14202a5f0` returns reflected bAllowMultipleStacks, distinct from stack-size-based IsStackable. Constructor projections are documented with the corresponding generator policy. The 23 constructor-proven weapon tables retain dynamic +820 dispatch; indexed definition descendants are recorded in ignored `generated/agents/reload-inventory209/`. Independent review: `inventory-leaf-review216`.

The reserve-ammo body and inventory interface resolver remain unintegrated: actual pawn interface bindings and concrete world/profile lookup implementations are required. These two complete leaves do not imply full reload behavior. Build/runtime acceptance is pending.


## Concrete inventory-owner queries

The PC, AI pawn, deployable plot and Athena AI bot controller now carry complete native single/all-item lookup bodies, presence virtuals, and the bot quickbar getters. They use the existing inventories and profile backend rather than shadow indexes. The subsequent integration extends the same IFortInventoryOwnerInterface after recovering the bot's distinct infinite-ammo implementation and actual mutable state contracts. The concrete methods can be source-complete before that shared binding is available; no reserve/reload or walking repair is claimed.

Evidence: existing singleitem215/allitems214 captures for PC14237aa80/14237b1d0; fresh bounded captures765545c34e314b1880098d9ee457169d for AI1422c5d10/1422c5e80 and plot141c37980; retained plot140680300 bytes; getters223 captures ee6d2f67063e41e2885247cd10c805ae and c50b51475a034724a99c7abcda0f0de0 for bot methods. Plot presence141663080 is proven by concrete interface table145f44288 and its direct pointer comparison. PC presence1406801d0 returns true; AI1422dcd90 and bot1416a0ae0 require a begun-play inventory actor.

Review preserves plot's virtual presence call before its inventory read, inventory reads after casts/output reset, world profile capture before the virtual definition-key call, account profile reread after that call, borrowed group records, and original temporary destruction scopes. Plot all-items returns zero without clearing output. Single plot lookup forwards a failed/null world-definition cast to the existing inventory helper. Indexed subtype tables preserve these bodies; DeployableBase controller14253e6a0 is a direct jump to PC14237aa80. Unknown/unindexed subtype behavior and runtime Blueprint behavior remain unverified. Complete source bodies are build/runtime pending.

Fourteen new method bindings include reused constant-return originals; they must not be described as fourteen newly recovered distinct original functions. Independent semantic review225 found the inventory-load ordering mismatch, which was corrected before generation.


## Reload predicates

Base1426930a0 and ranged1426c49c0 CanReload828 are complete ordered predicates. Capacity is computed even with missing WeaponData; all dynamic action, reserve, magazine/local-magazine, reload-active and firing calls remain dynamic. Optional failure tags use original native-tag singleton offsets and AddTag. The elapsed test keeps the positive comparison to reject NaN, reads the prior reload timestamp before GetWorld, and retains the original valid-world precondition. Ranged checks actual data/double-pump/cock state before a qualified nonvirtual base call. Existing capacity/cache/firing/reserve helpers now close all predicate callees; broader producer state remains separately incomplete. Cached evidence and original constructor table inventory are in reload-predicate196 and reload-predicates231 reports. UE build/runtime pending.
