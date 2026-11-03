# +0x304 (bNativeInActivationStack) writers and activation-stack pieces

Source: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`. Addresses are VA (file offset = VA - 0x140000000). Nothing built or run.

## 1. Runtime writers of panel+0x304 (verified by bytes)

| Site (VA) | Encoding | Effect | Containing function | Legacy source match |
|---|---|---|---|---|
| 0x1407fc713 | `c6 83 04 03 00 00 01` `movb $1,0x304(%rbx)`, then `call *0x4e8(%rax)` | sets flag true, calls vtable +0x4e8 (NativeAddedToActivationStack) | Push path. Preceded by a Contains-then-Add loop over an array at `(%rbx)` with count at +0x8 and max at +0xc. Start address not established. Inferred: `UCommonInputManager::NativeStartNextOperation` Push case, or `NativeInternalPushActivatablePanel`. | `manager-definitions.inc` lines 155-165 (Push case: Add, then `if (!bNative) { bNative=true; NativeAddedToActivationStack(); }`) |
| 0x1407ae30a | `c6 87 04 03 00 00 00` `movb $0,0x304(%rdi)`, `call 0x8091f0`, `call *0x4f0(%rax)` | clears flag, calls vtable +0x4f0 (NativeRemovedFromActivationStack) | Reverse loop over `manager+0xb8` from index `manager+0xc0 - 1` down to 0. Function start 0x1407ae2c0 (verified prologue). Taken as `UCommonInputManager::NativeResetPanelOperations`. | `manager-definitions.inc` lines 224-236 (`for I = Num-1 .. 0`, clear, Removed) |
| 0x1407dc450 | `c6 86 04 03 00 00 00` `movb $0,0x304(%rsi)`, `call 0x8091f0`, `call *0x4f0(%rax)` | clears flag, calls vtable +0x4f0 | Function starting 0x1407dc280 (prologue seen). Inferred, not verified: the store at 0x7dc438 writes `manager+0xc0` (`mov %r12d,0xc0(%r13)`) just before the clear, so the function is a Remove-from-stack path. | `manager-definitions.inc` lines 188-199 (`NativePrePanelDeactivatedForPop`: Remove, then clear, then Removed) |

Ordering in the binary matches the source: the count write (Remove) at 0x7dc438 precedes the flag clear at 0x7dc450.

Helper `0x1408091f0` is called between the flag write and the vtable call in both clear sites. Its name is not verified. The `0x8090b0` call after the set site (0x7fc723) is also unnamed.

## 2. Non-runtime or unverified sites

- 0x140788c41: `mov %al,0x304(%rsi)`. Context: `xor` setup, no call nearby. Not analysed further. Inferred: not a runtime toggle.
- 0x14079175c: `mov %sil,0x304(%rdi)`, after `mov %esi` to +0x2e8 and +0x2f8. Pattern looks like a constructor or field initialiser. Inferred: init, not runtime.
- 0x140796609: `mov %cl,0x304(%rbx)`, followed by `mov %ecx,0x308(%rbx)`. Inferred: init.
- 0x140807e6b: `mov %al,0x304(%rbx)`, after writes to +0x2e8, +0x2f0, +0x2f8, then `mov %eax,0x308`. Inferred: init.
- Other scan hits (about 180 `disp32=0x304` store-shaped encodings across the image) were not checked. The raw scan matches any ModRM with mod=10, so most are false positives.

## 3. Pop path (confirms NOTES.md)

`PopActivatablePanel` (0x1407e0c80) does not write +0x304. The flag is cleared only in the Remove path (site 0x1407dc450) and in the reset loop (site 0x1407ae30a), and set only in the Push path (site 0x1407fc713). Those paths are reached from operation processing, not from Pop itself.

## 4. Vtable slot +0x4c0 (0x1407e1130)

- Body (from NOTES.md): `this+0x3b0` IsValid and Get on the weak `NativeInputManager`, then `UCommonInputManager::PopActivatablePanel(manager, this)`.
- Vtable neighbourhood (file offset 0x59edb10 base, the first of several identical neighbourhoods): +0x4a8 = 0x1407ce810, +0x4b0 = 0x1407bb630, +0x4b8 = 0x1407bb5f0, +0x4c0 = 0x1407e1130, +0x4c8 = 0x140680300, +0x4d0 and +0x4d8 = 0x1406801c0. The same entries recur at three other vtable bases (0x5bc59c8, 0x6243298 and more).
- The generated headers (`UCommonActivatablePanel.h`, `UCommonActivatableWidget.h`) do not list slot indexes, so no declaration order maps onto +0x4c0.
- Name: **inferred, unverified.** Working name `PopSelfFromInputManager` (virtual, body calls PopActivatablePanel(this)). Keep it as a placeholder until a vtable slot index is matched to a declaration.

## 5. Activation-stack pieces

- `UCommonInputManager::ActivatablePanelStack`: `manager+0xb8` (data), `manager+0xc0` (count). Verified by 0x7ae2f0 and 0x7ae327 (`mov 0xb8(%rsi)`, `movslq 0xc0(%rsi)`).
- Operation queue: `manager+0xe8`, with count at +0xf0 (see NOTES.md).
- Push/add to stack: flag set true and `NativeAddedToActivationStack` (+0x4e8). Site 0x1407fc713 (verified by bytes, matches source).
- Remove from stack: flag cleared and `NativeRemovedFromActivationStack` (+0x4f0). Sites 0x1407dc450 and 0x1407ae30a (verified by bytes, match source).
- Reset: `NativeResetPanelOperations`, reverse loop over the committed stack. Site 0x1407ae30a (inferred name from structure, matches legacy source).

## Unresolved

- Start of the function containing 0x1407fc713 (not established).
- Confirm the function start 0x1407dc280 covers 0x1407dc450 (no end-of-function check done).
- Name the helper at 0x1408091f0 and the call at 0x1408090b0.
- Match vtable +0x4c0 to a declared virtual.
