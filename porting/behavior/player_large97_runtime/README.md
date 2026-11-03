# player_large97_runtime (UNCOUNTED draft, nothing compiled/promoted; `manifest-entries.json` is empty because no function is fully closed)

Four typed source drafts for the three assigned large functions (one assignment turned out to be two functions). Raw `controller_bulk96_runtime/rough.cpp` was not copied.

| File | Native | Lines (decompile) | Status |
|---|---|---|---|
| `ToDeathCause.inc` + `ToDeathCause_TagRows.inc` | `0x141a5f1c0` `AFortPlayerStateAthena::ToDeathCause(const FGameplayTagContainer&, bool)` | 869 | full decision logic; tag-slot names, override helper and `0x14192b2d0` unresolved |
| `CheckIfLevelStreamingCompleteForHandshake.inc` | `0x1419dbc20` | 355 | full flow; iterator ctor/dtor, 6 virtuals, bounds getter unresolved |
| `AFortQuickBars_SwapItems.inc` | `0x142070440` | 569 | full flow incl. 4 log messages with recovered args; 7 helpers unresolved |
| `CheckGhostModeItemReplicated.inc` | real body `0x1423cbe80` (wrapper `0x142a64930`) | 161 merged | full flow incl. 0.1 s self re-arm timer; 8 virtuals unresolved |
| `Unresolved97.h` | | | declarations only (no bodies, no guesses), each tagged with its native address/vtable slot |
| `gen_death_cause_rows.py` | | | regenerates the DeathCause tables from the decompile (strict-shape parser, aborts on surprises) |

## Important corrections to the assignment
* **`0x142070440` is not CheckGhostModeItemReplicated.** Its log strings read `Bad call to AFortQuickBars::SwapItems ...` and its params are `(AFortQuickBars*, EFortQuickBars, int, EFortQuickBars, int)`. The broad label sits on `0x142a64930`, a 26-byte stub (`inc [Frame+0x20]`-style Code advance, then `JMP 0x1423cbe80`). The decompiles in `native96-broad/0x142a64930.c` and `native96-impl/0x142a64930.c` merge stub + body and **drop the entry** (`if (!GhostModeRepData.bInGhostMode) return;`, the `this+0x598` interface base, the final timer delegate target). I disassembled `0x1423cbe80..0x1423cc21e` and `0x142070440..0x142070f00` from `image-125.runtime.bin` (dd + objdump, literal reads only: floats 0.1 @0x1459a86e8, -1.0 @0x1459a979c, 10000.0 @0x1478c283c, 0.0 @0x148218630) to recover dropped stack args.
* Probe `0x1423cbe80` and `0x142057010`/`0x142059e40` (already in impl) are the natural next function-level probes for CheckGhostMode.

## DeathCause automation
The 44 repeated blocks `FUN_141ed1800()+slot -> scan GameplayTags then ParentTags -> return N` become two ordered tables (28 stage-1 + 16 stage-2 rows) emitted by `gen_death_cause_rows.py`; row order is first-match priority. Special rows are explicit kinds: `0x358` (`DBNO?0x11:1`), `0x360` (`DBNO?0x11:0`), `0xa80` (then `0x638`: Sniper 6 else SniperNoScope 7). Between stages sits the `0xab8` gate (present => 0xe Minigun; checked via helper `0x14192b2d0`, not the inline scan). Last row `0x448`: match => 0x2e, else 0x32. The override provider (`0x141e5b2a0`/`0x141e5f200`) runs first and short-circuits unless it leaves 0x32. Enum names come from generated `EDeathCause.h`. Stage 1 uses the inlined exact (index,number) scan; stage 2 keeps the out-of-line helper because its identity with the inline scan is not proven.

## Concrete bindings (reflected/proven)
`FQuickBar` (CurrentFocusedSlot+0, Slots+0x10, DataDefinition+0x20) in `AFortQuickBars` at 0x220/0x2b0/0x340 (Primary/Secondary/Creative, static_asserts included); `FQuickBarSlot` (stride 0x28: Items, bEnabled bit0, bIsOccupied bit3, UsedBySlotIndex+0x14, UsedByItemGuid+0x18); `FQuickBarSlotData.AcceptedItemTypes`; `FGhostModeRepData` at controller+0x2448 (bInGhostMode, GhostModeItemDef +8); `RespawnCameraActor` +0x2e70; `EFortItemType::WeaponHarvest`=0x17; `FTimerHandle`/`GetWorldTimerManager().SetTimer(Handle, this, &CheckGhostModeItemReplicated, 0.1f, false)` (native: local zero handle not stored, first delay -1.0); `World->GetStreamingLevels()` (+0x98/+0xa0) with FailedToLoad==2 at +0xb8 (state-byte meaning by 4.25 enum, not proven in this build).

## Blockers / unresolved (all declared, none guessed)
* FFortGameplayTags slot-name mapping (`0x141ed1800`): offsets only (names need the singleton initializer).
* `0x14192b2d0`, `0x141e5b2a0`, `0x141e5f200`; QuickBars helpers `0x14204e850 0x14204f2a0 0x142065b00 0x142049220 0x1417cdda0 0x14204dd50 0x14205c5e0 0x1420570e0`, pending-move TSet layout (0x28 elements) and its drain loop; interface vtable slots +0x50/+0xa0/+0xd0 at controller+0x598 and QuickBars vtable +0x628/+0x630/+0x640/+0x648; item vtable +0x2e8/+0x318/+0x468/+0x480; log category `0x1481348e0/e4` (logged at Error).
* Handshake: object-iterator ctor/dtor `0x140894360`/`0x14469a960`, resolvers `0x143f76730`/`0x1413064f0`, classes `0x14273ec00`/`0x14480eb00`, stub, bounds getter `0x141bdf240` (FBox; only X/Y read), `0x1442dd540`, global `0x14811365c`, ULevelStreaming virtuals +0x268/+0x270 and fields +0x128/+0xb8, per-actor streaming object at actor+0xda0, the non-reflected listener array at controller+0x2da0 (16-byte entries; virtuals +0x28/+0x48), actor flag bytes +0x1f4/+0x1f5/+0x248/+0xb8.
* `AActor` raw offsets used (Owner 0xe0, RootComponent 0x130, scene-component location 0x1d0) are decompile offsets, not cross-checked against the 4.25 engine headers.
* Not compiled; `FRawPendingMoveSet`, `FRawObjectIterState` need real layouts before promotion. Push-list `Pad` dword copied from a stale guid word in native (never read as far as recovered).
Suggested next probes: `0x14204e850`, `0x14204f2a0`, `0x142065b00`, `0x142049220`, `0x1417cdda0`, `0x140894360`, `0x141bdf240`, `0x14192b2d0`, `0x141e5f200`, `0x141ed1800`.

Root review: corrected the tag-helper declaration/call spelling and enum member
declarations, and retained one singleton call per stage-1 row rather than
assuming call hoisting is harmless. New dependency capture native98-deps
contains21 helpers, including several blockers listed above; these have not
yet been integrated into the drafts. All four drafts remain uncounted.
