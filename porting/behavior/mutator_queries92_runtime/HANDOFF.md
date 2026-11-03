# Mutator value query source proposal

`Proposal.cpp` recovers four native bodies as typed rough C++. It depends on the iterator source proposal in `../infinite_ammo_iterator_runtime/Proposal.cpp.in`, not a global mutator list. No canonical manifest entries are claimed. No build or runtime tests were run.

| Reflected wrapper | Native body | Callback vtable slot |
| --- | --- | --- |
| GetOverridenSiphonHealthValue 0x14291ef50 | 0x141995ba0 | +0xaf0 |
| GetOverridenSiphonGoldValue 0x14291eeb0 | 0x141995b20 | +0xb10 |
| GetOverridenDamageSelfDamageToDeal 0x14291ebc0 | 0x141995900 | +0xb18 |
| GetOverriddenVehicleTrickScore 0x14291eaf0 | 0x141995820 | +0xb98 |

All four use active-only iterator constructor 0x1419b90a0, advance 0x1419f3d90 and destructor 0x1419bb7e0. For the three pawn queries, initialize one float to positive zero and pass its address to each callback in traversal order; any nonzero low-byte callback result stops. Writes by callbacks returning zero persist into the next callback and the eventual return. The destructor clears the selected mutator's context even on early termination. The virtual callback binding declarations are contracts, not implementations: preserve dynamic dispatch, byte return, and callback side effects.

Vehicle starts each callback with the original score, so rejected callback writes do not feed the next callback. A nonzero result returns that candidate; exhaustion returns the original score. Even null/failed context still traverses the game-state owner. Its context is the raw pointer at controller +0x220 if non-null and passing the class test using StaticClass 0x1447c8410; otherwise null. The exact class name and portable field/accessor corresponding to that offset are unresolved. Reflected parameters identify AFortPlayerController*, float, and float result; the other three accept APawn* and return float. The slot and context binding must be resolved before integration.

Iterator construction gathers context owners through native interface dispatch or volume fallback, then appends the game state uniquely. Owner arrays and per-owner candidate snapshots retain their native order and duplicates. Candidate selection filters for active Athena mutators and sets context; advancing and destruction clear it through virtual +0x6b8, including repeated clears at owner boundaries. See the referenced proposal and its README for weak-context lifetime, owner/provider dispatch, active predicate, fallback and unresolved dependencies. These effects cannot be dropped or replaced by direct array iteration. Callback invocations can have side effects; traversal and context setters must retain their original order.

Evidence: fresh wrappers in `porting/generated/native92-broad/`; fresh bodies and assembly in `porting/generated/native92-query/`; iterator in `porting/generated/callee_probe/infinite_ammo_rule/`. Original capture base 0x140000000, SHA-256 338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680. Reflection types are in `porting/generated/reflection/fortnite_game_reflection.json`.
