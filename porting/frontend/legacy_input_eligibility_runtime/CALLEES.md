# Shared helper callees of the legacy CommonUI input translations

Scope: the 8 most-shared callees from the UNKNOWN lists of
`legacy_input_eligibility_runtime/NOTES.md` (U-list lines 100-130),
`legacy_input_tick_runtime/NOTES.md` and `legacy_input_dispatch_runtime/NOTES.md`.
Decompiles: `porting/generated/decomp-imports/Imports/`. Disassembly: `image-125.runtime.bin`
(file offset = VA - 0x140000000). Engine source: `/opt/fortnite/UnrealEngine/Engine/Source`.

Bodies for the resolved entries are in `CALLEES.inc` (same folder).

## Results

| VA | Lists | Result | Basis |
|---|---|---|---|
| 142ee5dd0 | elig U1, disp U3, tick | Identity: `FWeakObjectPtr::Get()` (bEvenIfPendingKill=false) | Control flow and field offsets match `WeakObjectPtr.h` `Internal_Get`/`Internal_GetObjectItem`, `UObjectArray.h` `IsValid`. FUObjectItem: Object +0, Flags +8, SerialNumber +0x10, stride 0x18. Pending-kill mask 0x30000000 (Unreachable|PendingKill). |
| 142ee7880 | disp U5, tick | Identity: `FWeakObjectPtr::IsValid()` | Same checks as above, returns bool. Object pointer not read. |
| 142ee3b30 | disp U1, tick | Identity: `FWeakObjectPtr::FWeakObjectPtr(const UObject*)` / `operator=` | Asm 2ee3b30-2ee3b6a: null -> index -1, serial 0 (`Reset()`); else index = `[obj+0xc]` (`UObjectBase::InternalIndex`), serial = call 2ec00d0 with rcx=0x148263810 (`GUObjectArray`), edx=index. The callee's own identity (`AllocateSerialNumber`) is not separately verified. |
| 143d5a140 | elig U12, disp U6, tick | NOT `UWidget::GetCachedWidget`. Project-local `NativeSharedAccessor143d5a140` | Returns a `TSharedPtr`-shaped `{Object, Ctrl}` by sret. Strong member `+0xe8/+0xf0` is tried first, then a weak pin of `+0xd8/+0xe0`. UE 4.25 `GetCachedWidget` tries `MyGCWidget` (weak) then `MyWidget` (weak), and has no strong member path. Controller layout (vtbl +0, Shared +8, Weak +0xc, vtbl[0] DestroyObject, vtbl[1] deleting dtor) matches `FReferenceControllerBase`. |
| 142bee710 | elig U10, disp U10, tick | NOT identified. Project-local `NativeGMallocSlot30142bee710` | Verified: `if (p) { if (G==0) init(); G->vtbl[0x30](G, p); }`. In the 4.25 `FMalloc` source, slot 0x30 is `TryRealloc` (3 args), and `Free` is at 0x38. The binary's allocator class layout is not verified, so no identity is claimed. |
| 142bfd200 | tick, disp U10 | NOT identified. Project-local `NativeGMallocSlot20142bfd200` | Verified: `if (G==0) init(); G->vtbl[0x20](G, a, b, c)` (three args after this). Slot 0x20 does not match a 4.25 `FMalloc` slot with the same arity, so no identity is claimed. |
| 142c54b70 | elig U9, tick | NOT identified (UNRESOLVED) | Log emitter with `STAT_FMsgLogf` / `FMsgLogfCount` strings, so it is the 4.25 `FMsg::Logf_InternalImpl` family. Non-fatal branch matches source: verbosity {2,3,4,0x40} routes to GWarn, else GLog. Fatal branch differs: binary copies 0xffe TCHARs and zeroes index 0xffe, while source copies `UE_ARRAY_COUNT(Message)-1` (4095) and zeroes index 4095. Not declared. |
| 14078c640 | tick U2-area, disp U2 | NOT identified (UNRESOLVED) | Body matches `FDataTableRowHandle::GetRow` (DataTable.h 415-425): if table `[+0]` non-null, `FUN_14078bb10(table, row name [+8], ctx, 1)`; else warn "FDataTableRowHandle::GetRow : No DataTable for row %s (%s)." and return 0. The warn gate's `FUN_142cd8ed0` is an unnamed helper, so its equivalence to `RowName != NAME_None` is unverified. Verbosity gate `2 < DAT_148329e08` is consistent with `Warning <= category verbosity`. |

## Counts
- Resolved: 6 (3 identities, 3 project-local names with verified semantics).
- Not resolved: 2 (142c54b70, 14078c640).

## Verified facts not turned into names
- Globals: 0x148263a2c (element count bound used by the weak-pointer checks), 0x148263a18 (chunk table), 0x148263810 (GUObjectArray, used as `this` in the serial allocation call), 0x14823d250 (allocator pointer). Their names are not verified.
- 142bee820 is the lazy allocator-init helper called when 0x14823d250 is null.

## Not done
- No build, no run, no commit. Edits limited to this folder.
