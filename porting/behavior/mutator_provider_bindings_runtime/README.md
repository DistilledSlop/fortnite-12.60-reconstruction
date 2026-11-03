# Mutator provider bindings

The independently recovered owner overrides are supplied in `OwnerAppend.inc` and `manifest-entries.json`. Both reset the supplied array and copy the component's mutators in order, retaining duplicates; neither uses the context argument. Install the same assigned virtual `NativeAppendOwnerMutators(TArray<AFortGameplayMutator*>&, UObject*)` on `IFortMutatorOwner` and override it in each class. This is an ABI-compatible source contract for the observed calls, not a recovered original name or proof of the original static context type. The generator must emit the interface declaration and real class overrides, including both classes' native interface inheritance; inherited empty interface declarations are insufficient.

Evidence from image-125 (base 0x140000000):

* AFortGameModeAthena FClassParams 0x1463cf250 has owner entry at 0x1463cf238, offset 0xa18. Constructor 0x14192db5b installs interface vtable 0x145dfc1f0; its +0x20 entry is 0x141948340. That body resets the output then reads component at interface-this +0x990 = object +0x13a8 and appends its array at component +0xe0. This corresponds to generated `MutatorListComponent`.
* AFortGameStateAthena FClassParams 0x1463ea150 has owner entry at 0x1463ea120, offset 0xf20. Constructor 0x14197f7fb installs table 0x145e0b700; +0x20 points to 0x141995780, with the same body and component at interface-this +0x1120 = object +0x2040, `MutatorListComponent`.
* AFortPlayerStateAthena FClassParams 0x1464d3e50 has context entry 0x1464d3e38, offset 0xd20, whose class thunk 0x1429f8a30 jumps to context class builder 0x1429f87b0. Constructor 0x141a2e10d installs table 0x145e368c0, with +0x10 pointing to 0x141a45030. It calls 0x141c74820 on the player, then 0x141c7d420 on the result and player; if non-null, it calls 0x141c5f860 with the result and output and then adds the result uniquely. These helper operations are not yet recovered, so this context override is not ported. The iterator's fallback selector 0x141c7c5c0 is not interchangeable without evidence.

The iterator's owner +0x610 fallback is consistent with local UE 4.26 `AActor::FindComponentByClass(TSubclassOf<UActorComponent>) const` (Actor.h:2915), which is virtual; `GetComponentByClass` is a separate nonvirtual wrapper. Original slot identity and owner actor type still require verification before binding. Other provider implementations and the context methods remain integration dependencies. No runtime, build, or tests were run.

`python3 porting/behavior/mutator_provider_bindings_runtime/verify.py` passed all 12 capture-window checks; this checks evidence identity only, not generated code or runtime behavior.

Batch28 integrates both owner bodies, the native interface declaration, and original GameState inheritance through source_policy.py. Typed component local ensures its generated header is included. Full iterator and PlayerState provider remain incomplete.
