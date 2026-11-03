# Item owner and world resolution

Five original bodies plus one supporting weak field. These form the owner and
world dependency of `UFortWorldItem::GetRemainingAmmo`; the full ammo query is
not yet complete. Run `python3 -I porting/behavior/item_owner_runtime/prepare.py`
before promotion. The finite gate checks the original image SHA, complete
wrapper/native bytes in `evidence.json`, constructor-installed primary tables,
reflected parameter types, property identity, and supporting flag bit.

`UFortItem` constructor `0x142024010` initializes the weak storage at `+0x78`
to index `INDEX_NONE`, serial zero. UE's default `FWeakObjectPtr::Reset` does
exactly that. The assigned field name `NativeOwningController` represents
`FWeakObjectPtr` with statically typed controller access: the reflected getter returns that class
and the reflected temporary-item setter takes it. Original `0x142ee7880`
validates nonzero serial, nonnegative in-range index, matching serial and absence
of the unreachable/pending-kill flags. `0x142ee5dd0` repeats those checks before
returning the object. `0x142ee3b30` assigns object index/allocated serial or resets
the weak handle. These correspond to the existing UE weak pointer contracts.

Base getter `0x142030a60` calls IsValid then Get, otherwise returns null.
Temporary setter `0x142038aa0` calls IsValid even though the result is discarded,
sets reflected `bTemporaryItemOwningController` (`+0x80`, mask 2), and assigns
the weak pointer. Both calls and ordering remain explicit in source.

WorldItem getter `0x14205b080` first uses that weak pointer. If invalid, it calls
native slot `+0x460` once to test, calls it again, reads inventory `+0xe0`, and
casts to FortPlayerController. The slot target `0x14205b110` returns reflected
`OwnerInventory` (`+0xd8`); its source helper name `NativeGetOwnerInventory` is
assigned, not claimed as the original native identifier. Engine Actor reflection
identifies `+0xe0` as `Owner`, so the lift uses `Inventory->GetOwner()`.
It preserves the two virtual inventory calls and adds no extra null guard.

WorldItem GetWorld slot `+0x150` targets `0x1410cf280`: read original OuterPrivate
at `+0x20`, call its virtual GetWorld when present, otherwise return null. The
source uses the existing GetOuter accessor and typed virtual dispatch. All three
original reflected native WorldItem classes select the same `+0x150/+0x460/+0x4a0`
targets; the derived final-table construction was independently established by
`item_max_normalized_runtime/subclass_inventory.py`.

The later ammo fallback `0x14463f780` matches local engine
`UEngine::GetFirstLocalPlayerController(World)` (UnrealEngine.cpp): checked world
context, optional OwningGameInstance, then its first local controller. The inner
helper matches GameInstance.cpp's world-controller iterator / IsLocalController
branch and null-world local-player branch. Use that engine API, retaining its
invalid-context behavior; do not replace it with an arbitrary controller index.

Some class initializers and other item state remain unported. This slice restores
the identified weak field's default and writer; it does not claim recovery of the
whole item constructor. No builds, runtime runs, or tests were performed.

Integration uses the existing engine FWeakObjectPtr storage type and static_cast to the proved controller type; no runtime cast is added on weak resolution. This avoids treating the controller template argument as a by-value class in the current field gate. GetWorld explicitly requests indexed Engine/World.h.
