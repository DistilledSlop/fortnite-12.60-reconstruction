# Legacy binding row provider: original-image contract

Static original-image evidence, no source or build edits. Run `python3 /mnt/vibrato/fortnite-work/action-binding-row-provider-1/replay.py`: `PASS six original-image windows and six literals`. This hashes constructor, row-handle and FindRow code, struct getter/name and empty context, and verifies the actual diagnostic literals. It checks captured bytes, not live execution.

The record constructor at 0x1407959d9 passes its copied `FDataTableRowHandle` and the UTF-16 empty string at 0x1459a4b30 to helper 0x14078c640. The helper sets its FindRow `bWarnIfRowMissing` argument to true (0x14078c6e9). Struct getter 0x140817c80 names `CommonInputActionData` and registers size 0x428. The exact UE 4.25 source-level operation is:

```cpp
const FCommonInputActionData* row = handle.GetRow<FCommonInputActionData>(TEXT(""));
```

Source cross-reference: `/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes/Engine/DataTable.h:195` and `:415`. The helper has these ordered outcomes:

1. Null DataTable: return null; if RowName is not `NAME_None`, emit `LogDataTable` Warning for no table, including row name and empty context. Both null table and `NAME_None` return null silently.
2. Nonnull table but null RowStruct: `LogDataTable` Error for no row structure and return null. This check precedes name checking and is unconditional with respect to the warning flag.
3. RowStruct not `IsChildOf(FCommonInputActionData::StaticStruct())`: `LogDataTable` Error for incorrect type and return null. Derived structs are accepted. The actual call enables the warning flag, which gates this error in the generic FindRow routine.
4. `RowName == NAME_None`: `LogDataTable` Warning and return null.
5. No exact FName key in the table row map: `LogDataTable` Warning with context, row and table path, and return null. Otherwise return the map's stored row pointer. Source has `check(RowData)`; shipping image returns the stored pointer directly. Logging at each branch is additionally governed by the existing category verbosity, and neither row iteration nor a default row is substituted.

At 0x1407959ec–0x140795a0c the constructor saves the result; with empty display override it uses row+0x08 for display, and regardless of override then reads row+0x51 for keyboard state. A failed lookup is therefore an invalid consumer precondition, not a valid empty binding or substitute row. This applies even when a display override is supplied. UE's normal lookup diagnostics occur before that invalid consumption; do not suppress or invent a context if parity is intended.

For the adjacent gamepad adapter, vtable +0x290 is the `UWidget::GetOwningLocalPlayer` virtual declared in `/opt/fortnite/UnrealEngine/Engine/Source/Runtime/UMG/Public/Components/Widget.h:789`, overridden by `UUserWidget` in `.../UMG/Public/Blueprint/UserWidget.h:334`. Dynamic dispatch is preserved. Previous proof `/mnt/vibrato/fortnite-work/action-binding-gamepad-provider-1/HANDOFF.md` gives the router and subsystem chain.

For manual reinspection use `objdump -D -b binary -m i386:x86-64 -M intel --adjust-vma=0x140000000 --start-address=0x14078c640 --stop-address=0x14078c701 /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`; use 0x14078bb10..0x14078bdcf for FindRow and 0x1407959d9..0x140795a10 for the callsite. Runtime wiring, valid actual row data, and lifecycle tests remain outside this source proof.
