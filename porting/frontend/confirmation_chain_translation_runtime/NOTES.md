# Confirmation chain translation (bot click) - notes

Scope: translation of porting/generated/decomp-imports/Imports/{BotClick__144f54030, ConfirmClicked__144f5fbc0, DeclineClicked__144f5fce0, Dispatch__144ce33d0, Present__144f55c60, WeakPin__143d66ef0, OuterWalk__144ade110, u2__144f75f90, u3__144f55ae0}.c. Not built, not run, not committed. Only this folder was touched.

Naming: BotClickButton is reflected (exec 145218540) and used as-is. All other functions and members are address-suffixed (NativeXxx<VA>). Reflected members (ConfirmationButtons, DeclineButton, ConfirmationDescription, EntryBox_SimpleButtons, CommonLoadGuard_LatentSpinner) are used as-is. ConfirmClicked, DeclineClicked, Present, WeakPin, OuterWalk, Dispatch, u2, u3 are not in the reflection JSON (exact-name search returned 0 hits).

## Byte identities

### +0x596 (window byte, NativeFlag596)
- Verified by the C: ConfirmClicked reads `*(char*)(W+0x596)` and branches on == 0 (Present path) vs != 0 (dispatch path). Disassembly confirms `cmp %dl,0x596(%rdi)` at 0x144f5fc22 (rdi = W, dl = the bool argument).
- Verified by the C: no writer in the nine translated files.
- NOT verified: that the byte is inside ConfirmationDescription (would be description +0xe6). The C accesses it via the window base only. The description starts at W+0x4b0 (verified as the Dispatch description argument), but its sizeof is not shown. The 288-byte copy buffer is an allocation size, and the TArray at W+0x5c8 suggests sizeof is at most 0x118, which would put 0x596 inside. This is INFERRED only.
- Field name and type: UNKNOWN.

### +0x490 (8-byte slot, NativeMember490)
- Verified by the C: `lea 0x490(%rdi),%rcx` at 0x144f5fc48 (ConfirmClicked). It is passed as Dispatch's 6th argument only on that path. Present and DeclineClicked pass a local handle instead. Dispatch passes it to 141f8f0a0 and 141f87900, then reads its first dword (`*param_6`).
- NOT verified: type, owner class, and meaning. "Weak or shared ref" is inference only. Layout placement (between reflected 0x488 and ConfirmationButtons 0x498) comes from CALLEES.md and was not re-checked against the reflection JSON.
- The grep hit for 0x490 in OutroDefault__1407bb630.c is a vtable call `*0x490(param_1)`, not this member.

## UNKNOWN items (bodies or meanings not in the translated set)

1. Window vtable 0x288 (NativeDispatchSlot288): return object type and identity. The result is read as a UObject (+0x10 ClassPrivate) in BotClick.
2. NativeClassGetter142a3d9f0 and NativeClassGetter143dbe060: class identity (the pseudo-C shows only a call returning a pointer).
3. Object flags at +0x1838 and +0x1818 on the vtable-0x288 result: owner and meaning.
4. NativeDispatchSlot268 (vtable 0x268 on the button): result unused, meaning UNKNOWN.
5. NativeWeakPin143d66ef0 internals: NativeWeakPinOut143d5a140 body. Its out pair layout {object, block} is INFERRED (the pseudo-C only passes &lStack_18 and separately reads plStack_10). Pinned-object type. Meanings of slots 0x28 and 0x48 and fields +0x1b0, +0x1b8, +0x1a8. The mask 0x19 meaning. CALLEES.md claims W+0xe8 and W+0xf0 are read inside 143d5a140; not visible in the pseudo-C.
6. Shared block vtable slots 0 and 8: identity UNKNOWN (release pattern matches the pseudo-C).
7. NativeOuterWalk144ade110: cached FName at 0x148382580, thunk_FUN_142cbd480, FUN_142cc0600, FUN_144ade1e0, vtable 0x290. Class identity UNKNOWN. The walk checks W and W->Outer->Outer only (one step, no loop).
8. Dispatch: 141f8f0a0, 141f87900 (handle operations), vtable 0x28 and 0x50 on the desc+0xf8 and desc+0x108 objects, and the field names and types at desc+0xf8, +0x100, +0x108, +0x110. The first parameter (Controller) is never read in the pseudo-C.
   - Disassembly check: the 0x28 call receiver is the object in rcx (`mov 0xf8(%rsi),%rcx; mov (%rcx),%rax; call *0x28(%rax)`). The pseudo-C shows `()` with no arguments, so the receiver is inferred from this.
   - Disassembly check: Dispatch has no backward loop. CALLEES.md's "iterates an array" claim is not supported.
9. Present and DeclineClicked: 144f4cfd0 (copy into temp, argument order as pseudo-C), 144ae20d0 (destroy), 141f7ce20 (handle init).
10. u2 (NativeHelper144f75f90): 142b1bea0, 144f4f6a0 (argument direction as pseudo-C), 144f555e0, 144f7b690, 144f7b730. W+0x480 object type. Vtable 0x2c0 and 0x270 meanings. W+0x4f8 meaning (tested == 0).
11. u3 (NativeHelper144f55ae0): 144f555e0, 1407b7ed0, 140685370. Element struct at +0x5c8: pointer at +0x0 and int32 Count at +0x8 are read by the pseudo-C; bytes +0xc..+0xf UNKNOWN. Vtable 0x50 on the element pointer. W+0x5dc counter semantics beyond the increment/decrement shape.
12. 143d852b0 (EntryBox setter) and 1407f7e20 (LatentSpinner setter): meanings UNKNOWN (CALLEES.md says "probably SetVisibility" and "loading state", both inferred).
13. W+0x570 mapping: ConfirmationDescription.ConfirmButtonInputActions.ArrayNum per LAYOUT.md. ArrayNum at +8 is inferred.
14. sizeof(FFortDialogDescription_NUI): not shown in the set (see +0x596 above).

## Cross-check results (disassembly, image-125.runtime.bin)

- Pseudo-C calls `thunk_FUN_143d853b0` at the EntryBox setter. The disassembly `call 0x3d852b0` at 0x144f5fc73 gives VA 0x143d852b0. The translation uses 0x143d852b0 (CALLEES.md matches). The pseudo-C thunk name is unexplained.
- Call sites in 0x144f5fbc0..0x144f5fce0: 0x3d66ef0 (WeakPin), `call *0x268` on the button, 0x4ade110 (OuterWalk), 0x4ce33d0 (Dispatch), 0x3d852b0, 0x4f55c60 (Present), and `jmp *0x270(%rax)` at 0x144f5fcd7. The pseudo-C's call to 0x1407f7e20 (LatentSpinner setter) does NOT appear as a direct call in that range, and the `jmp *0x270` is not explained by the pseudo-C. Unresolved; the translation keeps the pseudo-C call.
- BotClick's virtual call at 0x288 is on `this` (the window) per the pseudo-C pattern. Not separately disassembled.
