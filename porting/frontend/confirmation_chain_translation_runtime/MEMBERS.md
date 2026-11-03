# UFortConfirmationWindow members +0x480 and +0x490

Scope: identifies the types of the window members at +0x480 and +0x490. Evidence: reflection JSON, Ghidra pseudo-C (decompile_targets.py to a scratch dir, read-only), and flat disassembly of porting evidence image image-125.runtime.bin (VA = 0x140000000 + file offset). Nothing built, run, or committed.

Naming: reflected names as-is. Unreflected members and functions are project-local, address-suffixed.

## VERIFIED

### Window identity and constructor
- Class: /Script/FortniteUI.FortConfirmationWindow, size 0x5e0. StaticClass getter NativeClassGetter145215a90 calls FUN_142d698b0 with those strings and size (decompile of 0x145215a90, args: L"/Script/FortniteUI", L"FortConfirmationWindow", size 0x5e0, ConfigName L"Engine", ClassConstructorFn &LAB_145217060).
- LAB_145217060 (0x145217060) is a thunk: loads *rcx, then `jmp 0x144f4d810`.
- Constructor NativeCtor144f4d810 (0x144f4d810): sets vtable 0x146dd2d98, calls FUN_141f7ce20(this+0x490), sets this+0x498 and this+0x4a0 to 0, calls FUN_142b1bea0(this+0x4b0) (FFortDialogDescription_NUI ctor, matches NOTES.md), sets this+0x5c8, +0x5d0 to 0, +0x5d8 to 2.
- The ctor writes no store to this+0x480.
- Not the window ctor: 0x144f4d550 (vtable 0x146dcc730) and 0x144f4ec40 (vtable 0x146dcccb0, reached via thunk 0x145337110) are other classes. Both stores to 0x480/0x490 in 0x144f4d550 are zero-initializations of that class's own fields.

### +0x480 = Image_MainIcon, type UImage (/Script/UMG.Image)
- Reflection (fortnite_game_reflection.json, class properties, window entry): Image_MainIcon, kind Object, offset 1152 = 0x480, type_refs.class 0x143daaa70.
- 0x143daaa70 is `jmp 0x14080f790`. 0x14080f790 is `jmp 0x143dac430`.
- 0x143dac430 is a StaticClass getter. It references L"Image" (0x14683e2aa), L"/Script/UMG" (0x146839fd8), and L"Engine" (0x1459a4c08), and calls FUN_142d698b0 at 0x143dac4ce. So the class is UImage in /Script/UMG.
- The ctor does not write 0x480, so the member is assigned outside the ctor. Writer functions are not identified here.

### +0x490 = unreflected 8-byte slot, type FFortDialogExternalLatentActionHandle (reflected struct, size 4)
- Reflection: no property at offset 0x490 in the window class. The gap between reflected CommonLoadGuard_LatentSpinner (0x488) and ConfirmationButtons (0x498). Name is address-suffixed: Member490.
- Ctor NativeCtor144f4d810 calls FUN_141f7ce20 with rcx = this+0x490 (see constructor above).
- FUN_141f7ce20 (0x141f7ce20) is `*param_1 = 0xffffffff; return param_1;` (decompile). Its address is the target of the thunk 0x142b1d600 (`mov %rdx,%rcx; jmp 0x141f7ce20`), which is the Construct slot (slot 3 per NOTES.md ICppStructOps order) of struct_ops vtable 0x146557098.
- Struct record (struct_blocks.json, index 1374): FortDialogExternalLatentActionHandle, size 4, align 4, struct_ops vtable_va 0x146557098, struct_ops_fn 0x142b1b3e0.
- So the member is the struct's native Construct applied in place. Size 4 matches the single dword store. The Dispatch read of only the first dword (NOTES.md) is consistent.
- Name of the struct is reflected as-is. Member name is address-suffixed.

### Corrections to NOTES.md
- NOTES.md item 14 (W+0x490 type UNKNOWN) and item 10's W+0x480 class identity: now resolved as above.
- The 0x141f7ce20 call in the ctor is a Construct call on a reflected struct, not a generic helper.

## UNKNOWN (not in scope of this pass)
- Writers of +0x480 (which widget object is stored and where): not identified.
- Vtable 0x270 target on the UImage receiver (u2 at 0x144f7601e): not resolved here.
- Meaning of 0x142b1d600's other callers: not checked.
