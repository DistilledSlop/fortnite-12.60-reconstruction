# GetMaxDurability and normalized durability

The base definition virtual is recoverable. `prepare.py` verifies the pinned image SHA-256, the entire reflected exec (including both bytecode and compiled-in property decode paths, stack advancement, and output write), reflected signature and flags, GPSC/thunk/constructor relationship, and the original-this vtable store. The proof gate ran successfully. Exec 0x142b5d270 decodes one `int32 ItemLevel`, calls slot 0x4d8 with that value, and stores the returned float. Constructor 0x142042090 writes primary table 0x14606f8f8 to its original this; that table's slot 0x4d8 is 0x1407c46d0, which reads a verified IEEE 1.0 constant and returns. The table scan covers the slot contiguously, and the constant lies in initialized, readable, non-writable PE storage. The wrapper calls the existing FFrame `Step` (0x142ed9cf0) and `StepExplicitProperty` (0x142ed9d20) helpers into a zero-initialized four-byte local; reflected metadata identifies the parameter as int32. The reflected function is const. The proposed virtual base body ignores the argument, but virtual dispatch must be retained; derived implementations are not recovered. `manifest-entries.json` and `GetMaxDurability.inc` are ready for review and integration.

WorldItem normalized body 0x14205a980 casts the virtual `GetItemDefinitionBP()` result to UFortWorldItemDefinition, tests reflected `bItemHasDurability` at byte 0x343 mask 1, and returns 1.0 on failure. Otherwise it snapshots the definition's vtable, invokes item virtual `GetDurability()` then `GetLevel()`, calls the saved definition slot 0x4d8 with that level, divides, and performs `COMISS` against +0 followed by unsigned-below branch to +0 and `MINSS` against 1.0. NaN and negatives become +0, negative zero is preserved, and values above 1 become 1. The proposed body explicitly uses `FMath::IsNaN` before the negative comparison; the local engine generic implementation is bit-based, while Windows uses `_isnan` when vector intrinsics are enabled. Compilation must retain IEEE division and comparisons (no fast-math assumptions).

The finite inventory in `subclass_inventory.py` closes the saved-vptr question for the reflected native WorldItem class hierarchy in this image. The parent map has exactly three members with complete chains: WorldItem, CreativeWorldItem, and PlaysetWorldItem. Their primary tables are respectively 0x14607a630, 0x1463af658, and 0x1464e8cd8. The two derived constructor thunks pass original this to the WorldItem constructor and then install their own primary tables on that same object. All three tables select the same `GetLevel` (0x14205a560), `GetDurability` (0x142059c00), `HasDurability` (0x14205ca30), and `GetItemDefinitionBP` (0x14205a2f0). These bodies, including the transitive calls in GetDurability, read item/definition state and do not replace the definition. Thus typed virtual dispatch after those calls preserves the original dispatch for these original native classes. The inventory is limited to this reflected native hierarchy and does not establish behavior for arbitrary future native overrides. Reflected item method flags have neither FUNC_Event nor FUNC_BlueprintEvent; Blueprint subclasses cannot override these C++ virtual methods through Blueprint events. The proof scanner passed.

The following typed implementation is a candidate under strict floating-point compilation, with the calls kept as separate full expressions:

```cpp
const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());
if (!Definition || !Definition->bItemHasDurability) return 1.0f;
const float Durability = GetDurability();
const int32 Level = GetLevel();
const float Maximum = Definition->GetMaxDurability(Level);
const float Value = Durability / Maximum;
if (FMath::IsNaN(Value) || Value < 0.0f) return 0.0f;
return Value > 1.0f ? 1.0f : Value;
```

The normalized manifest entry and source fragment are proposed for root review and integration. Typed dispatch naturally admits future derived definition `GetMaxDurability` overrides, which remain unrecovered. No tests or builds were run.
