# panel_pop_stack_runtime: PopActivatablePanel path recovery

Status: disassembly and layout checks done. Fragments written: `panel-pop-declarations.inc`, `panel-pop-definitions.inc`. Nothing built or run. Nothing committed.

Source of bytes: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`. Addresses below are the binary-relative addresses that objdump prints (`objdump -D -b binary -mi386:x86-64 --start-address=<VA-0x140000000>`), so they equal the VA minus 0x140000000.

## 1. Vtable slot +0x4c0 (`0x1407e1130`)

Leaf thunk, no stack locals stored. Control flow:

1. `call 0x2ee7880` with `rcx = this+0x3b0`. This is `FWeakObjectPtr::IsValid` (serial check against the global object table, reads index at +0 and serial at +4).
2. If false, return.
3. `call 0x2ee5dd0` with `rcx = this+0x3b0`. This is `FWeakObjectPtr::Get`, which returns `UObject*` or null.
4. `call 0x7e0c80` with `rcx = manager`, `rdx = this`. That is `UCommonInputManager::PopActivatablePanel(this)`.

The weak pointer at `+0x3b0` matches `TWeakObjectPtr<UCommonInputManager> NativeInputManager` declared in `porting/native/Source/CommonUI/Generated/UCommonActivatablePanel.h` (line 90). The offset itself is confirmed by usage only, because the generated header has no byte offsets.

## 2. PopActivatablePanel body (`0x1407e0c80`, 290 lines, includes callees)

Control flow as the disassembly shows it:

- Local predicted stack is built by `0x7ad3f0`. The name `NativePredictedPanelStack` is taken from the legacy source candidate, not from the binary.
- Committed stack: scan `manager+0xb8` (`ActivatablePanelStack.Data`) with count at `manager+0xc0`. `r12b = !found`.
- Predicted stack: scan the local copy. `sil = !found`.
- If the panel is not in the predicted stack (`sil != 0`), the function only does verbose logging and returns.
- Otherwise it walks the predicted stack from the top down. For each entry it calls `0x2c9ff10`, then builds an `FOperation` with `Operation = Pop (3)` and `Panel = entry`, then calls `0x7a95e0` (the clean-operation helper that scans `manager+0xe8`). The loop breaks when `0x7a95e0` returns the target panel.
- If the target's predicted index is greater than 0, it appends an `Intro (0)` operation for `predicted[Index-1]` at the end of `manager+0xe8` (`Num` at `+0xf0`, `Max` at `+0xf4`, grow via `0x69ea30`). Entry stride is 0x28.
- If `manager+0xe0` (`bNativeProcessingOperations`) is clear, it calls `0x7ab920` (`NativeBeginOperationProcessing`). In the binary this call is not gated on a suspension flag, which differs from the legacy source candidate.

**Byte `panel+0x304` is not written anywhere in this function, or in any callee dumped: `0x7a95e0`, `0x7ab920`, `0x2c9ff10`, `0x69ea30`, `0x2ee7880`, `0x2ee5dd0`.** `PopActivatablePanel` therefore only enqueues Pop/Intro operations and kicks processing. The membership byte must be written later, by operation processing or by the panel's `NativeAddedToActivationStack` (`+4E8`) and `NativeRemovedFromActivationStack` (`+4F0`) hooks. That writer is not yet recovered. The `+0x304` claim in HANDOFF.md is only a membership description, not a write site from this path.

## 3. Owning classes and reflected members

- `UCommonInputManager` owns `ActivatablePanelStack` at `+0xb8` (reflected `offset 184`) and `Operations` at `+0xe8` (reflected `offset 232`). Both match `porting/native/Source/CommonUI/Generated/UCommonInputManager.h`. `FOperation` is 0x28 bytes (`Operation` at 0, `Panel` at 8). Source: `porting/native/Source/CommonUI/Generated/FOperation.h`.
- `PopActivatablePanel` is `UCommonInputManager::PopActivatablePanel(UCommonActivatablePanel*)`. The reflection record `porting/generated/reflection/fortnite_game_reflection.json` (name PopActivatablePanel, 1 Object param `ActivatablePanel`) has `owning_class: null` and points at a different image address (`0x14080ede0`). `porting/generated/decomp_batches/state.json` maps the UCommonInputManager name to `0x140810760`. The 0x1407e0c80 body is attributed to the manager by usage (`+0xb8`, `+0xe8`), not by reflection.
- `+0x3b0` (`NativeInputManager`) belongs to `UCommonActivatablePanel`. It is not reflected, because it is a `TWeakObjectPtr` and has no `UPROPERTY`.
- `+0x304` (`bNativeInActivationStack`) belongs to `UCommonActivatablePanel`. It is declared in `porting/frontend/legacy_panel_operation_queue_runtime/panel-declarations.inc` (line 5, "original +304"). It is not in the generated header, and it is not reflected.
- `+0x288` (`bIsActive`) belongs to `UCommonActivatableWidget` (`porting/native/Source/CommonUI/Generated/UCommonActivatableWidget.h` line 68). It is a bitfield, so the reflection data has no byte offset for it. The `+0x288` mapping is taken from HANDOFF.md and is not verified.
- `UFortRestartClientModal` derives from `UFortActivatablePanel`, which derives from `UCommonActivatablePanel`.

## Unresolved

- Names for `0x2c9ff10`, `0x7a95e0`, `0x7ab920` (named here as NativeBeginOperationProcessing only by the legacy source), `0x7ad3f0`, `0x2ca88a0`.
- The byte writer for `+0x304`. Next step: find the op-processing completion path that reaches `NativeAddedToActivationStack` and `NativeRemovedFromActivationStack`.
- Vtable slot `+0x4c0` has no name yet, so the fragment uses a placeholder member name.
- Break condition for the Pop loop (return value of `0x7a95e0`) is inferred from one compare, not from the source.
