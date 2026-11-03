# UFortWorldItem query overrides

`python3 -I porting/behavior/world_item_queries_runtime/prepare.py` produces
five proposed helper entries and snippets. Run completed with five. They depend
on the matching virtual UFortItem base declarations already promoted by root.
This is source recovery, not a build or runtime validation. The helper kind emits
plain C++ virtual declarations; inherited reflected UFunctions dispatch through
them. Overrides are not separately reflected UFunctions.

Image-125 SHA-256 is checked before decoding. Hardened existing original-this
vptr proof and `build_vtable_slots` establish GPSC 0x142b5bd80 -> thunk
0x142b59740 -> constructor 0x142041fc0 -> vtable 0x14607a630. The proof
checks exact complete body bytes, table slots, source reflection and current
storage representation, and preexisting body/declaration exclusions.

| Override | Slot | Target | Behavior |
|---|---|---|---|
| GetDurability | 0x2c8 | 0x142059c00 | Virtual HasDurability; if true return ItemEntry.Durability, else -1.0f. |
| HasDurability | 0x2c0 | 0x14205ca30 | Cast the virtual GetItemDefinitionBP to UFortWorldItemDefinition; return its bItemHasDurability or false. |
| GetNumInStack | 0x368 | 0x14205ac70 | Return ItemEntry.Count. |
| GetNumberOfSlotsTaken | 0x2e8 | 0x14205af00 | Cast virtual item definition to UFortWorldItemDefinition; return NumberOfSlotsToTake or 1. |
| GetItemDefinitionBP | 0x3e8 | 0x14205a2f0 | Return ItemEntry.ItemDefinition. |

The two accessors tail-jump to exact leaf functions: Count at 0x14202c360 and
ItemDefinition at 0x1407c1e20; Durability true branch jumps to 0x14202d340.
Reflection places ItemEntry at 0xe0 and its Count, ItemDefinition, Durability
members at 0xc, 0x18, 0x24; snippets use generated named members, not native
byte offsets. WorldItemDefinition class static GPSC 0x142b5be40 matches the
original cast, reflected bit decoder maps bItemHasDurability to 0x343 bit 1,
and reflected NumberOfSlotsToTake is 0x734. Cast uses UE's original object
subclass test contract. The -1.0f constant source is checked in a readable,
initialized, non-writable section. Preserve these five together because the
GetDurability override requires the HasDurability override.

The remaining two requested bodies are blocked, not proposed: normalized
0x14205a980 casts the virtual item definition and calls GetDurability, another
item virtual +0x268, and definition virtual +0x4d8 before float division and
clamping; both virtual identities and exact float edge semantics remain to be
proven. Remaining ammo 0x14205b6b0 contains multiple inventory/owner/world
virtual calls, class tests and other dependencies and has not been recovered.
It cannot be replaced by a constant or by ItemEntry.LoadedAmmo.

Evidence is at ignored `porting/generated/callee_probe/world_item_queries_1/`;
run used `porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input
porting/behavior/world_item_queries_runtime/targets.json --out
porting/generated/callee_probe/world_item_queries_1 --timeout 30`. Probe
completed, rolling back its transaction; project metadata files changed. No
canonical manifest, Native, generator, build or test changes in this slice.

Review follow-up: `prepare.py` additionally asserts fallback leaf 0x1406801d0
is exactly `b0 01 c3` and pins the complete 0x142b5be40 GPSC function bytes
and its `FortWorldItemDefinition` class-name literal. Original subtype checks
call that GPSC only for non-null objects, then inspect the UE struct ancestry
array. Local UE4.25 `Templates/Casts.h` UObject-to-UObject `Cast` checks non-null
then `IsA<To>()`; `UObjectBaseUtility.h` implements that via `To::StaticClass()`
and `IsChildOf`, with no other operation in the cast. StaticClass may lazily
initialize its class, matching the original GPSC call rather than being a pure
operation. Both generated fields occur under public access and the included
`UFortWorldItemDefinition.h` declares both and inherits `UFortItemDefinition`.
Re-running prepare completed with five proposals after these checks.

Integration note: explicit `includes` accepts indexed engine headers only; generated same-module UFortWorldItemDefinition inclusion is inferred by generator body_refs. The proposal omits that explicit key.
