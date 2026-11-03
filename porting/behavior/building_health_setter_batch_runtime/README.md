# Building health setter batch handoff

No bodies are ready for integration: both native setters depend on lazy creation, and the creator remains unresolved. In particular, substituting a null check or omitting creation would alter normal authority behavior. The existing setter drafts and dependency-blocked manifest in `building_vitals_runtime` remain applicable.

Evidence pins:

- `porting/generated/callee_probe/building_vitals_dependencies_1/0x141bc4a90.asm`: SetHealth calls get-or-create 0x141b9b750, constructs the Health attribute, gets the ASC from the set, and conditionally calls SetNumericAttributeBase. The set is passed to the accessor without a null check.
- `porting/generated/callee_probe/building_vitals_dependencies_1/0x141b990d0.asm`: ForceBuildingHealth checks authority, inlines the get-or-create condition, retains one set pointer, then constructs and applies MaxHealth and Health in that order, with a fresh ASC lookup for each.
- `porting/generated/callee_probe/building_vitals_dependencies_2/0x141b9b750.c` and `building_vitals_dependencies_3/0x141b94070.c`: creation is reached when the set is absent, the actor is not a template, the policy is Lazy, engine byte +0xc7 is set, and the actor has authority. Creation has several additional side effects and cannot be safely replaced by allocation alone.
- `porting/generated/callee_probe/building_vitals_dependencies_6/0x143f72040.c` and `UnrealEngine/Engine/Source/Runtime/Engine/Private/Actor.cpp:2046`: creator's 0x143f72040 is AActor::FlushNetDormancy. The scope name, net-mode and dormancy guards, Initial-to-DormantAll transition, replication check and world/net-driver operations all agree. This closes one previously unknown creator call, but not the creator.
- Remaining blockers include the three component flag clears, the engine byte, the registration/list target and creator's virtual initialization calls, including the nontrivial 0x141b8f710 (`building_vitals_dependencies_4/0x141b8f710.c`). The exact engine byte is absent from the current generated UFortEngine declaration. These are observable paths, so no ready manifest or body-only inc is emitted.
