# Restart modal vtable routines: verified behavior

Source: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`.
Address convention: VA = `0x140000000` + file offset. All disassembly was read with `objdump -D -b binary -mi386:x86-64`. Vtable slots were read with `struct` reads at VA `0x146d700c8 + off`.

Names: the only names used are those that appear as embedded strings in the binary (`Activate: %s`, `Deactivate: %s`, `PopActivatablePanel: begin`, `DeactivateWidget`). Every other routine is described by what it does.

## 0. Slot table (vtable at VA 0x146d700c8)

| Slot | Target VA | File offset | Verified target shape |
|---|---|---|---|
| +0x4a8 | 0x1407ce810 | 0x7ce810 | Section 1 |
| +0x4b0 | 0x1407bb630 | 0x7bb630 | Section 2 |
| +0x4c0 | 0x1407e1130 | 0x7e1130 | Section 4 |
| +0x4d8 | 0x1406801c0 | 0x6801c0 | `ret $0x0` stub (Section 2.1) |
| +0x490 | 0x145160190 | 0x5160190 | Partially described (Section 2.2) |
| +0x4e8 (Added) | 0x1406801c0 | 0x6801c0 | Same stub as +0x4d8 |
| +0x4f0 (Removed) | 0x1406801c0 | 0x6801c0 | Same stub as +0x4d8 |

## 1. Slot +0x4a8, target 0x1407ce810

Verified instruction sequence:

1. `cmpb $0x0,0x303(%rcx)`; `je 0x7ce87a`. If byte `this+0x303` is zero, the function returns with no other effect.
2. `call 0x7ae640` with `rcx = this` (Section 1.1).
3. `call 0x809130` with `rcx = this` (Section 1.2).
4. Log block: `mov 0x18(%rbx),%rax` and store it into a local at `rsp+0x40`. Call `0x2ce9f90(&local40, &local20)`. Read the returned pointer `rax`. If `8(%rax) != 0`, pass `(%rax)` as the `%s` argument. Otherwise pass the address `0x59a4b30`. Call `0x2cc40c0` with `ecx = [0x82675e8]`, `rdx = 0x59ef340`, `r8 = <that pointer>`. The format string at `0x59ef340` decodes as UTF-16 `Deactivate: %s`.
5. If `local20 != 0`, call `0x2bee710` on it (release of a local).
6. Return.

It writes nothing to `this`. It does not write `+0x288` or `+0x303` (the only writes to `+0x303` are listed in Section 5).

Field identity of `this+0x18`: UNKNOWN.

### 1.1 Callee 0x1407ae640 (file 0x7ae640)

Verified behavior:

1. `rbx = this`. Calls `0x2ee7880` with `rcx = this+0x3b0` (a `TWeakObjectPtr`). If it returns false (`al == 0`), the function returns at `0x7ae743`.
2. If `[this+0x3c0] == 0` and `0x2cd8ed0(ecx=[this+0x3c8], edx=0)` returns nonzero and `[this+0x3cc] == 0`, jump to the reset block (step 4).
3. Otherwise, if `0.0f < [this+0x308]` (a float compare with `comiss`), call `0x7bdba0(rcx=this+0x360, rdx=&local60, r8=this+0x3c0)`. Its result is an `int32` index. If the index is not `-1`, the element address is `[this+0x360] + index*0x98`, and when that element is non-null, call `0x7ac590` with `rcx = element+0x10` and `xmm1 = 0`. Meaning of `0x7ac590`: UNKNOWN.
4. Reset block (`0x7ae6fc`): write 16 zero bytes to `this+0x3c0..0x3cf`. Write `0` to dword `this+0x308`. Call `0x2ee5dd0` with `rcx = this+0x3b0` (the weak pointer get; returns the object pointer in `rax`). Store `0` to `[rax+0xa8]` and `0` to `[rax+0xb0]`. There is no null check of `rax` between the call and these stores. The `IsValid` check in step 1 is the only guard.
5. Return at `0x7ae743`.

Byte `+0x303` is not referenced anywhere in this function (no `0x303` displacement in the dumped range).

Unverified: the meaning of `0x2cd8ed0`, `0x7bdba0`, `0x7ac590`, the layout of the `0x98`-byte elements, and whether the function has code paths outside the ones above.

### 1.2 Callee 0x140809130 (file 0x809130)

Verified: this is a thunk.

1. Calls `0x2ec7730` with `rcx = this` and `rdx = [0x80d2260]` (an 8-byte value read from binary offset `0x80d2260`). Result in `rax`.
2. Tail-jumps with `rcx = this`, `rdx = rax`, `r8 = 0` to `*(vtable(this) + 0x210)`.

So it dispatches the virtual slot `+0x210` on `this`. The target of that slot is UNKNOWN. `0x2ec7730` is UNKNOWN.

## 2. Slot +0x4b0, target 0x1407bb630 (file 0x7bb630)

Verified instruction sequence:

1. `mov (%rcx),%rax` (vtable), `mov %rcx,%rbx` (this saved).
2. `movb $0x0,0x303(%rcx)`. It clears byte `this+0x303`.
3. `call *0x4d8(%rax)`, with `rcx = this`. Target is the `ret $0` stub (Section 2.1).
4. Reload vtable from `[this]` and set `rcx = this`.
5. `movb $0x0,0x288(%rbx)`. It clears byte `this+0x288` (`UCommonActivatableWidget` `bIsActive` per HANDOFF/NOTES, not re-verified from reflection here).
6. Epilogue, then `jmp *0x490(%rax)` (tail call with `rcx = this`, `rdx` as passed in).

### 2.1 Slot +0x4d8 (and +0x4e8, +0x4f0), target 0x1406801c0 (file 0x6801c0)

Verified: the first bytes at this address are `c2 00 00` = `ret $0x0`. It does nothing and does not set `rax`. The byte sequence `b0 01 c3` (`mov $1,%al; ret`) lives at `0x6801d0`, which is not the slot entry.

So the same stub is the target of slots `+0x4d8`, `+0x4e8` (Added) and `+0x4f0` (Removed). That means the Added and Removed slots are no-op stubs in this vtable. They have no effect on the object.

### 2.2 Slot +0x490, target 0x145160190 (file 0x5160190)

Partially described. The function is long (about 1040 disassembled lines, 5 `ret` sites at `0x51604f9`, `0x516057c`, `0x516068b`, `0x51607a9`, `0x5160a79`). Verified:

1. Saves `rsi`, `r14`, and reserves stack. `rsi = rcx` (this).
2. `call 0x514f130` with `rcx = this`. If the result `rax` is null, jump to `0x51603c7`, which sets `rcx = this` and tail-jumps to `0x7d26e0` (target UNKNOWN).
3. Otherwise it calls `0x4db9890` with `rcx = rax`, then reads `0x8(%rax)`. It walks a list or array through that pointer (exact structure UNKNOWN).
4. Verified stores through `rsi = this`: `mov %rbp,0x420(%rsi)`, `mov %rdi,0x428(%rsi)`, `mov %rbp,0x430(%rsi)`, `mov %rbx,0x438(%rsi)`. These are 8-byte stores to `this+0x420`, `+0x428`, `+0x430`, `+0x438`. The values they store, and the conditions under which they run, are UNKNOWN.
5. Calls with unknown targets: `0x690620`, `0x3d5c530`, `0x2ee5dd0`, `0x2ee3b30`, `0x2cd4ab0`, `0x2cc0600`, `0x2cbc9d0`, `0x28ea7f0`, `0xf67c80`, `0xd11db0`, `0x7d2950`, `0x7ccee0`, `0x690dd0`, `0x58092ff`, `0x51b3160`, `0x4ce6fd0`, `0x4ade110`, `0x3dbe060`, `0x2cd8ed0`, `0x2bfd200`.

UNKNOWN: the full set of reads and writes to `this`, the meaning of the `+0x420..+0x438` fields, and what each `ret` path returns.

## 3. Cross-check: fallback 0x1407b7ed0 (HANDOFF claim)

Verified (file 0x7b7ed0):

```
cmpb $0x0,0x288(%rcx)
je   0x7b7ee3        ; ret
mov  (%rcx),%rax
jmp  *0x4a8(%rax)    ; tail-call slot +0x4a8
```

So the fallback is exactly `if (this+0x288 != 0) tail-call slot +0x4a8`, with the HANDOFF's shape confirmed.

Does Section 1 support "fallback = DeactivateWidget()"? Only partly:

- The dispatch shape (gate on `+0x288`, tail-call `+0x4a8`) is supported.
- Slot `+0x4a8` logs the embedded string `Deactivate: %s` (Section 1, step 4), and it runs only when `+0x303` is nonzero.
- The embedded ASCII string `DeactivateWidget` exists in the binary at file offset `0x5A01988` (VA `0x145A01988`). Nothing in this analysis links that string to slot `+0x4a8` or to the vtable. UNKNOWN.
- Slot `+0x4a8` does not write `+0x288`. The clear of `+0x288` happens in the `+0x4b0` routine (Section 2 step 5), not in `+0x4a8`.

So the claim "fallback is DeactivateWidget()" is UNKNOWN from this evidence. Only the dispatch shape is verified.

## 4. Slot +0x4c0, target 0x1407e1130 (file 0x7e1130), callee 0x1407e0c80 (file 0x7e0c80)

### 4.1 Slot +0x4c0 (0x7e1130), verified

NOTES.md calls this a "leaf thunk". That is wrong: it makes three calls.

1. `rbx = this`. `call 0x2ee7880` with `rcx = this+0x3b0`. If the result is false (`je 0x7e1164`), return.
2. `call 0x2ee5dd0` with `rcx = this+0x3b0`. Result in `rax`, moved to `rcx`.
3. `rdx = this`. `call 0x7e0c80`.
4. Epilogue and `ret`. There is no null check of `rax` between step 2 and step 3. The `IsValid` check in step 1 is the only guard.

### 4.2 Callee 0x1407e0c80 (`PopActivatablePanel` body), verified claims

Called with `rcx = manager`, `rdx = panel`. The entry logs the embedded string `PopActivatablePanel: begin` (decoded from `0x59f3d48`), gated on the global at `0x80d1cd8` (`cmpb $0x7`).

Verified:

- Calls `0x7ad3f0` with `rcx = manager`, `rdx` = a local. The NOTES name `NativePredictedPanelStack` for this local is not in the binary. UNKNOWN as a name.
- Committed stack scan: `[manager+0xb8]` is the data pointer, `movslq [manager+0xc0]` is the count, 8-byte elements, each compared to the panel (`cmp %r15,(%rcx)`). `r12b = 1` when not found. Verified.
- Predicted stack scan: same pattern over the local array. `sil = 1` when not found. Verified.
- Pop loop: `r13d = count - 1`, index `r12` runs down to 0. Each iteration calls `0x2c9ff10` on `predicted[r12]`, builds an `FOperation` on the stack with `Operation = 3` (`movb $0x3`) and `Panel = predicted[r12]` (at `+8`), and calls `0x7a95e0` with `rcx = manager`, `rdx = &op`. The loop exits when `cmp %r15,%rdi` after that call is equal. Whether `0x7a95e0` preserves `rdi` is UNKNOWN (the `0x7a95e0` return path was not reached in the dump), so the break condition is only partly verified.
- Intro append: if the target index `r13d > 0`, take `predicted[index-1]`, call `0x2c9ff10`, build `FOperation` with `Operation = 0` (`movb $0x0`) and `Panel = predicted[index-1]`. It appends at `[manager+0xe8]`: `Num` is `[manager+0xf0]` (incremented, `mov %eax,0xf0(%rdi)`), `Max` is `[manager+0xf4]` (`cmp 0xf4(%rdi),%eax`), grow via `call 0x69ea30` when `Num > Max`. Element stride is `0x28` (`lea (%rsi,%rsi,4)` then `*8`). Verified.
- Processing kickoff: `cmpb $0x0,0xe0(%rdi)` (manager `bNativeProcessingOperations` per NOTES, not re-verified); if zero, `mov %rdi,%rcx; call 0x7ab920`. There is no check of a suspension flag on this path. Verified.
- Byte `+0x304` is not referenced anywhere in `0x7e0c80` (no `0x304` displacement in its 290-line dump).
- Early-exit path when the panel is not in the predicted stack: UNKNOWN (the exact jump targets were not traced; NOTES claim is not re-verified).

Not re-verified: the bodies of `0x7a95e0`, `0x7ab920`, `0x2c9ff10`, `0x69ea30`, `0x2ee7880`, `0x2ee5dd0` (only the first bytes of `0x7a95e0` and short windows of the others were read). The NOTES claim that `+0x304` is not written by any of these is UNKNOWN beyond the bytes read.

## 5. Byte +0x303 (verified accesses)

Byte-pattern scan for displacement `0x303`. Confirmed by disassembly where marked.

| Access | VA | Instruction | Function |
|---|---|---|---|
| read (`cmpb $0`) | 0x1407ce816 | `cmpb $0x0,0x303(%rcx)` | Section 1 (slot +0x4a8) |
| clear | 0x1407bb63c | `movb $0x0,0x303(%rcx)` | Section 2 (slot +0x4b0) |
| set to 1 | 0x1407ab908 | `movb $0x1,0x303(%rbx)` | Function at 0x1407ab8f0 (verified). Calls `0x2ee7880` on `this+0x3b0` but does not test its result before the store. Ends with `jmp 0x8090f0` (target UNKNOWN). |
| set to 1 | 0x1407ce7d0 | `movb $0x1,0x303(%rbx)` | Function containing the log string `Activate: %s` at `0x59ef320` (verified string). Function entry UNKNOWN (not traced). Ends with `jmp 0x8090f0`. |

Other byte-pattern hits (not disassembled, so their function identity and role are UNKNOWN): `0x1407d6a18` (set 1), `0x1407dd21e` (cmp 0), `0x1407f1b34` (clear 0), `0x1407fc9ad` (set 1), `0x1407fcf5f` (set 1).

Meaning of `+0x303` as a field name: UNKNOWN. It is not in reflection data for this class (not checked beyond the disassembly).

## UNKNOWN summary

- Name of `+0x303`, `+0x304`, `+0x288`-as-`bIsActive` are from HANDOFF/NOTES, not re-verified from reflection.
- Identity of slot `+0x4a8` as `DeactivateWidget` (Section 3).
- Target of slot `+0x210` called via `0x809130`; function `0x2ec7730`; global `0x80d2260`.
- Target `0x8090f0` (tail-jump of the `+0x303` setters).
- Section 2.2 (slot `+0x490`): most of its behavior.
- Break condition in `0x7e0c80` (`rdi` preservation through `0x7a95e0`) and the early-exit path for a panel not in the predicted stack.
- Bodies of `0x7a95e0`, `0x7ab920`, `0x2c9ff10`, `0x69ea30`, `0x2ee7880`, `0x2ee5dd0`, `0x2cd8ed0`, `0x7bdba0`, `0x7ac590`.
