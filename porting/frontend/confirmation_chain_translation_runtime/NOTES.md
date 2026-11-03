# Confirmation chain translation (bot click) - notes

Scope: translation of porting/generated/decomp-imports/Imports/{BotClick__144f54030, ConfirmClicked__144f5fbc0, DeclineClicked__144f5fce0, Dispatch__144ce33d0, Present__144f55c60, WeakPin__143d66ef0, OuterWalk__144ade110, u2__144f75f90, u3__144f55ae0}.c. Not built, not run, not committed. Only this folder was touched.

Naming: BotClickButton is reflected (exec 145218540) and used as-is. All other functions and members are address-suffixed (NativeXxx<VA>). Reflected members (ConfirmationButtons, DeclineButton, ConfirmationDescription, EntryBox_SimpleButtons, CommonLoadGuard_LatentSpinner) are used as-is. ConfirmClicked, DeclineClicked, Present, WeakPin, OuterWalk, Dispatch, u2, u3 are not in the reflection JSON (exact-name search returned 0 hits).

Evidence method: image is /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin, a flat memory image with VA = 0x140000000 + file offset. Disassembly was done by extracting bytes at the VA with dd and running `objdump -D -b binary -mi386:x86-64 --adjust-vma=<VA>`. Do not run objdump directly on the PE file: it reads on-disk section layout, not the memory image.

## VERIFIED (this pass)

### Window +0x4b0 is ConfirmationDescription (FFortDialogDescription_NUI, inline)
- Reflection (fortnite_game_reflection.json, window class owning ConfirmationDescription): ConfirmationDescription offset 1200 = 0x4b0, kind Struct. DeclineButton 1192 = 0x4a8, ConfirmationButtons 1176 = 0x498, CommonLoadGuard_LatentSpinner 1160 = 0x488, Image_MainIcon 1152 = 0x480, EntryBox_SimpleButtons 1096 = 0x448.
- struct_blocks.json FortDialogDescription_NUI: size 280 = 0x118. 0x4b0 + 0x118 = 0x5c8, the TArray that ConfirmClicked/u3 use at W+0x5c8.
- Therefore W+0x596 = description +0xe6 (0x596 - 0x4b0 = 0xe6), inside the struct.

### Description +0xe6 is a bool, stored false by the struct constructor
- Vtable 0x146556f68 (struct_ops_fn of FortDialogDescription_NUI, reflection): slot 3 (Construct, per engine Class.h ICppStructOps order: dtor, HasNoopConstructor, HasZeroConstructor, Construct, HasDestructor, Destruct) = 0x142b1d5f0.
- 0x142b1d5f0 is `mov rcx,rdx; jmp 0x142b1bea0`.
- 0x142b1bea0 (constructor body, flat disassembly) stores: `movl $0,0xe0` (DisplayTime 0.0), `movw $1,0xe4` (byte e4=1, byte e5=0), `mov %dil,0xe6` (byte e6=0), `mov %rdi,0xe8` (NotificationHandler null), `mov %rdi,0xf0` (ShowSound null), `mov %rdi,0xd0`/`0xd8` (AdditionalContent, LeftAdditionalContent null). Also FSlateBrush-style ctor at +0 and FText ctors at +0x88 and +0xa0, TArray at +0xb8, Name at +0xc8.
- The header FFortDialogDescription_NUI.h (generated) gives defaults Dismissable(true), Cancelable(false), bShouldWaitForLatentActionOnConfirmAction(false), which match the stores: the only true default is at e4.
- Name of e6 as bShouldWaitForLatentActionOnConfirmAction: NOT closed by a direct store or read. Layout evidence only: reflection table indices fall monotonically with offset for all 10 reflected offset members (13@0, 12@136, 11@160, 9@184, 8@200, 7@208, 6@216, 5@224, 1@232, 0@240). Bools are indices 4, 3, 2, i.e. between DisplayTime (224) and NotificationHandler (232), so the header order Dismissable(e4), Cancelable(e5), bShouldWait(e6) follows. This is INFERRED from ordering, so the name stays UNKNOWN (see item 15).
- Dispatch does not read desc bytes 0xe4..0xe6 (Dispatch__144ce33d0.c). ConfirmClicked passes literal 1 as Dispatch's param_5.

### W+0x490 (8-byte slot): owner and access verified, type UNKNOWN
- Reflection: no property at 1168 = 0x490 in the window class. The gap lies between reflected CommonLoadGuard_LatentSpinner (0x488) and ConfirmationButtons (0x498). It is an unreflected native member.
- ConfirmClicked 0x144f5fc48 `lea 0x490(%rdi),%rcx`, then stored as Dispatch's 6th stack argument (`mov %rcx,0x28(%rsp)`).
- Dispatch reads only the first dword (`uVar1 = *param_6`) and passes it on. Its handle operations are 141f8f0a0 and 141f87900.
- u4 (141f7ce20) initializes a handle as `*p = 0xffffffff` (one dword). FFortDialogExternalLatentActionHandle.h has one int32 member, Handle, at offset 0. This matches the access pattern but is not proof that W+0x490 holds that type, so the type stays UNKNOWN.

### W+0x596 read path
- ConfirmClicked 0x144f5fc22 `cmp %dl,0x596(%rdi)`, then je 0x144f5fc90 (Present path) if zero. Otherwise dispatch path. Confirmed by disassembly.

### 0x1407f7e20 (LatentSpinner setter, u1): behavior verified, meaning UNKNOWN
- ConfirmClicked 0x144f5fc78 loads rcx = [W+0x488] (reflected CommonLoadGuard_LatentSpinner), 0x144f5fc81 `mov 0x40(%rsp),%rbx`, and 0x144f5fc8b is `jmp 0x1407f7e20` (a TAIL CALL, not a call). dl = 1.
- 0x1407f7e20 (flat disassembly): `mov 0x1f0(%rcx),%rcx; test; je ret; mov %dl,0x408(%rcx); jmp 0x1408033f0`. So it stores the flag byte at [[obj+0x1f0]+0x408] and tail-jumps to 0x1408033f0 (callee UNKNOWN).

### jmp *0x270 sites: two different functions
- 0x144f7601e (in u2, NativeHelper144f75f90): `lea 0x4b0(%rdi),%rdx; mov 0x480(%rdi),%rcx; call *0x2c0(%rax)` at 0x144f75fee (Obj=[W+0x480], Desc=W+0x4b0). Then `cmpq $0,0x4f8(%rdi)`; dl = ([W+0x4f8]==0) ? 1 : 0; `mov 0x480(%rdi),%rcx; mov (%rcx),%rax; rex.W jmp *0x270(%rax)` at 0x144f7601e. So u2 tail-calls vtable slot 0x270 of [W+0x480] with bFlag = (W+0x4f8 == 0). This matches definitions.inc.
- 0x144f5fcc0 (a separate function, NOT part of ConfirmClicked; its bytes start after a ret at 0x144f5fca7): `mov 0x2c0(%rcx),%rcx; test; je ret; mov (%rcx),%rax; test %dl,%dl; mov $4,%dl; jne +2; mov $1,%dl; jmp *0x270(%rax)`. Receiver is [arg+0x2c0], dl = (arg!=0) ? 4 : 1. Its caller is not identified.
- Vtable slot 0x270 targets: UNKNOWN for both sites (receiver classes not resolved to a vtable in the image).

### W+0x480 receiver class
- Reflection: window member at 0x480 is Image_MainIcon (Object), type_refs class = 0x143daaa70. The class name is not resolved in class_name_map.json / class_ref_map.json (no hit for that VA).

## Corrections to the earlier notes
- The earlier claim that the call to 0x1407f7e20 does not appear in 0x144f5fbc0..0x144f5fce0 was wrong: it is the tail `jmp` at 0x144f5fc8b.
- The earlier claim that `jmp *0x270` at 0x144f5fcd7 explains u2's tail was wrong: u2's tail is at 0x144f7601e, and 0x144f5fcd7 is in the separate 0x144f5fcc0 helper.
- The earlier "+0x596 inside the description is INFERRED" is now VERIFIED (see above). Only the bool's name remains open.

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
9. Present and DeclineClicked: 144f4cfd0 (copy into temp, argument order as pseudo-C), 144ae20d0 (destroy), 141f7ce20 (handle init; body read from pseudo-C as `*p = 0xffffffff`, not disassembled).
10. u2 (NativeHelper144f75f90): 142b1bea0 (the struct constructor, now identified; see VERIFIED), 144f4f6a0 (argument direction as pseudo-C), 144f555e0, 144f7b690, 144f7b730. Vtable 0x2c0 and 0x270 target identity. W+0x4f8 meaning (tested == 0). W+0x480 class identity (see VERIFIED).
11. u3 (NativeHelper144f55ae0): 144f555e0, 1407b7ed0, 140685370. Element struct at +0x5c8: pointer at +0x0 and int32 Count at +0x8 are read by the pseudo-C; bytes +0xc..+0xf UNKNOWN. Vtable 0x50 on the element pointer. W+0x5dc counter semantics beyond the increment/decrement shape.
12. 143d852b0 (EntryBox setter) and 1407f7e20 (LatentSpinner setter): meanings UNKNOWN (CALLEES.md says "probably SetVisibility" and "loading state", both inferred). 1407f7e20 body verified (see VERIFIED); its callee 0x1408033f0 is UNKNOWN.
13. W+0x570 mapping: ConfirmationDescription.ConfirmButtonInputActions.ArrayNum per LAYOUT.md. ArrayNum at +8 is inferred.
14. W+0x490 type and owner class (see VERIFIED): the slot is an unreflected native member; its declared type is UNKNOWN.
15. Name of description +0xe6 (bShouldWaitForLatentActionOnConfirmAction vs Cancelable at e5): the byte location and the false default are verified; the name is INFERRED from reflection table-index ordering. No direct store or read in the set distinguishes e5 from e6.
16. The 0x270 receiver vtable targets (both sites, see VERIFIED): UNKNOWN. The 0x144f5fcc0 helper's caller is UNKNOWN.

## Cross-check results (disassembly, image-125.runtime.bin)

- Pseudo-C calls `thunk_FUN_143d853b0` at the EntryBox setter. The disassembly `call 0x143d852b0` at 0x144f5fc73 (rcx = [W+0x448], dl = 1) gives VA 0x143d852b0. The translation uses 0x143d852b0 (CALLEES.md matches). The pseudo-C thunk name is unexplained.
- Call sites in 0x144f5fbc0..0x144f5fca7 (ConfirmClicked): 0x143d66ef0 (WeakPin), `cmp`/`call *0x268` on the button, 0x144ade110 (OuterWalk), 0x144ce33d0 (Dispatch), 0x143d852b0 (EntryBox setter), tail `jmp 0x1407f7e20` at 0x144f5fc8b, and 0x144f55c60 (Present) on the zero path.
- BotClick's virtual call at 0x288 is on `this` (the window) per the pseudo-C pattern. Not separately disassembled.

## VERIFIED (pass 3: 0x1408033f0, vtable 0x270 chain, W+0x490)

Sources: porting/generated/decomp-imports/Imports/NativeHelper1408033f0__1408033f0.c (Ghidra pseudo-C), cross-checked by flat disassembly of image-125.runtime.bin (dd + objdump, VA-based).

### 0x1408033f0 (tail target of 0x1407f7e20): body verified, meaning UNKNOWN
- Entry: rcx = P. Called as `jmp 0x1408033f0` from 0x1407f7e20 with P = [obj+0x1f0] (see earlier VERIFIED block).
- Fields read: A = byte [P+0x409], B = byte [P+0x408] (set by the caller from dl), C = qword [P+0x3f8].
- Disassembly: `cmpb $0,0x409(%rcx)` at 0x1408033fa, `je 0x14080346b` at 0x140803404, `cmpq $0,0x3f8(%rcx)` at 0x140803406, `cmpb $0,0x408(%rcx)` at 0x140803414, `movb $0,0x409(%rcx)` at 0x140803421, `call 0x1407bc830` at 0x140803431.
- Logic (pseudo-C, consistent with the disassembly):
  - If A == 0: if C == 0 and B == 0, return. Otherwise set A = 1, call 0x1407bc830(P+0x3e8, 1), send payload with byte DAT_147c522f1 to FUN_142fd17e0([P+0x3b8]), then send payload with byte DAT_147c522f4 to FUN_142fd17e0([P+0x3c8]).
  - If A != 0: if C != 0 return; if B != 0 return; set A = 0, call 0x1407bc830(P+0x3e8, 0), send payload with byte DAT_147c522f4 to [P+0x3b8], then payload with byte DAT_147c522f1 to [P+0x3c8].
  - Payload: 16-byte local {byte b; byte 1; qword 0; dword 0}, passed by pointer.
- So it is a guarded two-state toggle of byte [P+0x409] with two notifications per transition. Meaning of the state, of 0x1407bc830, of 0x142fd17e0, and of the DAT_147c522f1/f4 values: UNKNOWN. Types of [P+0x3b8], [P+0x3c8], [P+0x3e8]: UNKNOWN.

### 0x1408033f0 is not 0x1407f7e20's only consumer
- Imports/u1__1407f7e20.c line 21 calls FUN_1408033f0 (same callee). No other caller was checked.

### Vtable slot 0x270: candidate vtables read, receivers still unresolved
- Candidate vtable 0x1468284a8 (native_class_index "UImage", status unverified; also listed as candidate for UListView, UHorizontalBox, and others, so not class-specific): slot 0x270 = 0x143d91830.
- Constructor 0x143d35365 (Imports/NativeCtor143d35365__143d35365.c): stores vptr 0x1468284a8 at obj+0 (line 20) and later vptr 0x146828780 at obj+0 (line 44). Its class is not identified. Direct e8/e9 call scan and rel32/abs64 scans for 0x143d35365 found no references.
- Vtable 0x146828780: slot 0x270 = 0x143d91830 (same target). Class UNKNOWN.
- Candidate vtable 0x145da35f0 (native_class_index "UFortConfirmationWindow", status unverified): slot 0x270 = 0x1406801e0. Stores of this address: 0x14185ae5f (`lea`, then `mov %rax,(%rcx)` at 0x14185ae69, so a vptr store into the object in rcx; the object class is not identified) and 0x14186ff72 (`mov %rax,0x10(%rbx)`, so NOT a vptr store).
- UImage class-block attempt failed: StaticClass getter 0x143dac0a0 and its registration (Imports/NativeCtorImage143dac5b0__143dac5b0.c) resolve to L"ListViewBase", not Image. The UImage constructor is not found.
- Result: slot 0x270 targets for the receiver at [W+0x480] (u2 at 0x144f7601e) and for the 0x144f5fcc0 helper remain UNKNOWN. The receiver's runtime class is not verified, so none of the three slot-0x270 addresses above is claimed as its target.

### W+0x490 (no new facts)
- Owner and type remain as in the earlier VERIFIED block (unreflected native member, type UNKNOWN). The window constructor was not identified in this pass.
