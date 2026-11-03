# Typed infinite-ammo rule

`NativeQueryInfiniteAmmoRule` binds original `0x1419b0920` with assigned static
source names. It retains the original iterator's provider order, per-provider
candidate array, active-only selection, weak context resolution at each callback,
virtual query, bitwise accumulation, control-byte stop at 2, and final context
cleanup. Returning true never skips subsequent callbacks by itself.

The local iterator specializes constructor `0x1419b90a0` to this rule's actual
`AFortPlayerStateAthena*` input. That class implements `IFortMutatorContext`, as
pinned in `mutator_provider_bindings_runtime`. Thus a non-null context always
uses the interface callback; null skips context providers. The generic
`UObject*` iterator fallback `0x141c7c5c0` remains unported and receives no credit.
The extra GameState owner remains nullable and is appended uniquely.

GameState, Volume, and Minigame owner callbacks are integrated with their original
native interfaces. Other providers still dispatch through the interface or the
original actor virtual `FindComponentByClass` fallback. Array duplicates and
callback context writes are preserved, including the second clearing when
crossing from an exhausted nonempty provider to another nonempty provider.
Empty providers recurse without overwriting the previous current mutator;
exhaustion clears the pointer, and early termination leaves destructor cleanup.
Both original lightweight iterator/interface stat scopes are retained.

`native-evidence.json` pins the rule, constructor, owner selector, candidate
selector, destructor, and active predicate in image-125. Only the outer rule is
credited as a canonical body; the embedded specialized methods receive no extra
function credit. This does not establish controller inventory-owner interface
integration, all mutator overrides, full ammo behavior, or runtime parity.
No build or test was run; the next batch remains at 2,040 bodies.
