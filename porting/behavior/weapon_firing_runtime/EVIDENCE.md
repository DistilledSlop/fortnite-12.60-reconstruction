# Weapon firing predicates

Primary virtual +708 resolves to `1426a5c50` on eight base-family constructors, `1426a5c40` on five Deco constructors, and `1426d24d0` on ten ranged constructors. Base reads reflected ActiveAbility, calls original IsActive `1406ce750`, re-reads ActiveAbility, and compares its current spec handle unequal to ReloadAbilitySpecHandle. Deco returns reflected bButtonDown. Ranged nonvirtually calls base, then checks its two timer handles in order with one timer manager lookup. Original ranged constructor `1426bd2a0`, stores `1426bd436`/`1426bd43d`, initializes both handles to zero, matching default FTimerHandle construction.

The accessor for the protected ability spec handle is synthetic support and receives no original-function credit. It preserves the original direct field read without the target Engine getter's development-only instancing assertion. The same bridge is used by the already recovered reload-ability predicate.

Cached source and constructor inventory are recorded in ignored `generated/agents/weapon-firing202/`, `weapon-target150/`, and `reload-predicate196/`. Timer scheduling/clearing and broader firing producers remain open. These complete predicates do not claim full firing, reload, or movement acceptance; build/runtime verification is pending.
