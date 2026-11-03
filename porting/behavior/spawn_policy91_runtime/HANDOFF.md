# Spawn policy recovery (draft, uncounted)

`spawn_policy.cpp` reconstructs both native 14198bb80 and its actual policy callee 14198ec00, with opaque ABI adapters for unresolved dependencies. It is behavioral source, not a manifest entry or a buildable integration claim. `Word` arguments intentionally retain unknown types. No canonical files, generation, tests or builds were changed/run.

The top body obtains class +130, invokes class virtual +390 once if missing, then rereads it. A null class or still-null default produces false, a separately allocated UTF-16 `Invalid_Class` including its terminator (14 code units), and a copied shared default metadata value from 1407c2c80. Otherwise it passes the default object and the other three opaque arguments to the collection at game-state +20f8.

The callee scans 16-byte entries in order, using entry +8 as a presence indicator. It invokes the selected object's virtual +48 with the four original policy arguments. A false result transfers the string allocation, copies/retains metadata, releases the temporary metadata reference and returns immediately. A true temporary is destroyed before the next iteration. An empty collection or all-true results yield true, empty string and retained default metadata. A zero presence indicator sets the receiver null and still performs virtual dispatch; the reconstruction intentionally does not invent a skip. Strong and weak release operations and vtable destructor calls are described in the source adapter contract.

Integration still needs identification of the reflected argument and result types, policy entry type, actual virtual implementations, string allocator bindings, controller ownership helpers and precise vtable/ABI integration. `evaluate_rule` must retain the original null receiver behavior, and the result is constructed into uninitialized output storage. This draft does not claim recovery of the individual rule implementations.

Evidence (SHA-256):
- `porting/generated/native91-fresh/0x14198bb80.c`: ce50f9647b131675b37de8e89e7a41c0f7d9c3263d49c13b8e12341cd9d40c5c
- `porting/generated/native91-fresh/0x14198bb80.asm`: 1940f53f8a832f1b0bdc8e4ed316e3aec165cf99b8d32325d9078f4e1a5b88a8
- `porting/generated/native91-body/0x14198ec00.c`: 74c56deba453beb7a4929ffc5e7f31ca1cbf66251c29df500fd7d18ee78863ed
- `porting/generated/native91-body/0x14198ec00.asm`: c8a48dab35eb9dc7b7f1951130d4b9f5cdc0ed89139e5a332c508e2d7202696c

Root reflection lookup resolves the return struct constructor1428df090 to EvaluationResult (48 bytes): bSucceeded, ErrorCode FString+8 and ErrorMessage FText+24. Arguments are InstigatorActor, ActorClassToSpawn, Location and Rotation. This identifies the opaque layout; binding the native rule/delegate storage and allocator/lifetime adapters remains open. Evidence: native_funcs.json AFortGameStateAthena/CanSpawnActorOfClass and struct_blocks.json EvaluationResult.
