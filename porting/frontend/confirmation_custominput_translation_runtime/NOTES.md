# ShowSimpleConfirmationDialog_CustomInput translation: status

Not built, not run, not committed. Fragments only: `declarations.inc`, `definitions.inc`.
Evidence: `porting/generated/reflection/native_funcs.json`, `porting/generated/decomp-imports/Imports/ShowSimpleConfirmationDialog_CustomInput__1451f76d0.c`,
`CustomInputShowDialog__144b1aeb0.c`, `ReadParamA__142ed9cf0.c`, `ReadParamB__142ed9d20.c`, `FNameOrText__142c0c240.c`.
Disassembly: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (file offset = VA - 0x140000000).

## VERIFIED
- Reflection: exec 0x1451f76d0, flags 0x4022401, 10 params plus ReturnValue (Object, offset 240).
  Kinds: InWorldContextObject Object, Icon Struct (type ref 0x142fdb590), Title/Message Text, ConfirmAction/DeclineAction Name,
  AdditionalContent/LeftAdditionalContent/TargetPlayer Object, Cancelable Bool (bitfield, size_of_outer 248).
- The exec does NOT tail-jump. Its body is the full function (prologue, reads, call, epilogue, `ret` at 0x1451f7c2e).
  It calls 0x144b1aeb0 directly at 0x1451f7b49.
- Entry: rdx = FFrame (saved to rbx), r8 = result pointer (saved to r14), frame size 0x1f0.
- Each read checks FFrame+0x20 (code pointer) for zero, then calls 0x142ed9d20 (zero path) or 0x142ed9cf0 (non-zero path).
- Read order (10 reads, through disassembly): InWorld (slot rsp+0x68, zero-initialized), Icon, Title, Message, ConfirmAction, DeclineAction,
  AdditionalContent, LeftAdditionalContent, Cancelable (4-byte slot, tested != 0), TargetPlayer. Order matches reflected index 10 down to 1.
- Title and Message are each preceded by a call to 0x142c0c240 (copies a value from 0x1407c2c80).
- Call to 0x144b1aeb0: rcx = InWorldContextObject (rsp+0x68), rdx = Icon, r8 = Title, r9 = Message, and stack arguments
  covering ConfirmAction, DeclineAction, AdditionalContent, LeftAdditionalContent, Cancelable, TargetPlayer. The stack slot order was read from disassembly.
- Result: the callee's return value is stored with `mov %rax,(%r14)`, i.e. to the ReturnValue pointer.
- 0x144b1aeb0 checks its 5th argument (ConfirmAction) against zero as a 64-bit value, consistent with an FName.
- 0x144b1aeb0 copies Title and Message (20-byte ptr/ptr/int shape), builds an element with an out-of-line allocator, and calls 0x144b1a460.

## UNKNOWN (kept out of the fragments as bodies)
1. Icon struct name at 0x142fdb590. Declared only as `FUnknownIcon_142fdb590`. The sibling fragment's FSlateBrush is not confirmed here.
2. Concrete classes for AdditionalContent, LeftAdditionalContent, TargetPlayer (class refs 0x144786230 and others). Declared as UObject*.
   The sibling fragment's UWidget/ULocalPlayer are not confirmed here.
3. Mapping of the stack-argument slots to the individual ConfirmAction/DeclineAction names at the call site (Ghidra naming only).
4. Helper bodies: 0x142ed9cf0 (table dispatch via DAT_148262b60), 0x142ed9d20 (property path), 0x142c0c240, 0x1407c2c80.
   The fragment's non-zero/zero FFrame branches are collapsed into one helper call. Their behavior is not modelled.
5. Body of 0x144b1aeb0 beyond the verified argument checks: the allocator 0x14083c8d0, 0x142b1bd20, 0x140795480, 0x14079a760, 0x142bee710, and the
   factory 0x144b1a460 are not translated. The factory's registration and activation behavior is UNKNOWN.
6. Reference-count release sequences in the disassembly (lock decrement, vtable slot 0 and slot 8 on zero) are verified as a pattern only. Their
   owning helper is not named, and they are not written in the fragment.
7. Whether reflected index order equals UE declaration order (used as the C++ parameter order, which matches the sibling declaration).
8. Cancelable bitfield mask: the reflection marks it a bitfield, but the mask value is not verified. The read is a full 4-byte slot.
