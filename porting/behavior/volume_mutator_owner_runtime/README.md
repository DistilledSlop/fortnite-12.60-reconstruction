# Volume and minigame mutator owners

Both classes implement the assigned `IFortMutatorOwner::NativeAppendOwnerMutators` contract. Each clears the output, uses the actor's virtual `FindComponentByClass` to find the first `UFortMutatorListComponent`, and appends that component's `Mutators` array in order, retaining duplicates. The context argument is unused. This differs from the GameMode and GameState callbacks, which use a stored component pointer. The class inheritance must also be added in `mutator_provider_bindings_runtime/source_policy.py` for the overrides to be reachable.

Original image-125 evidence (base 0x140000000):

* AFortVolume class params 0x1465647c0 point to two interface entries at 0x146564780. The first entry is owner class getter 0x1429f89f0 at object offset 0x290. Class constructor thunk 0x142b355e0 invokes constructor 0x141c58290, which stores owner vtable 0x145f62360 at that offset. Its slot +0x20 is 0x141c79ac0.
* AFortMinigame class params 0x146446ef0 point to three interface entries at 0x146446e98. The first entry is the same owner getter at object offset 0x218. Constructor thunk 0x1429b83a0 invokes 0x14207d6d0, which stores owner vtable 0x146085b10 there. Its slot +0x20 is 0x142092410.
* Both callback bodies reset the output, call 0x1429f8d40 (class getter naming `FortMutatorListComponent`), dispatch actor virtual +0x610, and copy the returned component's array at +0xe0. The Volume primary vtable's +0x610 identity is documented in `mutator_context_runtime/README.md`. The context is not read. The returned pointer is used without a further runtime type check.

Proposed generator policy additions: `AFortVolume: ('0x1465647c0', 'IFortMutatorOwner')` and `AFortMinigame: ('0x146446ef0', 'IFortMutatorOwner')`. No build or runtime test has been run.
