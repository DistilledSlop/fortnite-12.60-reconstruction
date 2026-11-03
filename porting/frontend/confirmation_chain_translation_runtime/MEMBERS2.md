# Confirmation chain: element +0x8, weak-pin gate, helper 0x144f5fcc0

Scope: three UNKNOWNs from NOTES.md / definitions.inc. Evidence: reflection JSON, generated header, decomp-imports pseudo-C, flat disassembly of image-125.runtime.bin (VA = 0x140000000 + file offset), UE 4.25 headers. Nothing built, run, or committed. Names: reflected names as-is, others project-local address-suffixed.

## 1. FFortConfirmationButtonInfo element +0x8

### VERIFIED
- Reflection (porting/generated/reflection/struct_blocks.json, record block_va 0x147e8fa00, name FortConfirmationButtonInfo): size 16, align 8. Only property: Button, offset 0, kind Object, class 0x140807800. No reflected member at +0x8. Checked with a script that printed only the matching record.
- Generated header porting/native/Source/FortniteUI/Generated/FFortConfirmationButtonInfo.h: one UPROPERTY, UCommonButton* Button. No +0x8 member.
- Read of element +0x8: ConfirmClicked (decomp-imports/Imports/ConfirmClicked__144f5fbc0.c, loop over W+0x498 with count W+0x4a0): `alStackX_10[0] = plVar2[1]` when plVar2[0] == Button. This is the 8-byte value passed on to Present and Dispatch.

### UNKNOWN
- Writer of element +0x8: no store found. A grep of all 65 files in decomp-imports/Imports for 0x498 finds only ConfirmClicked. The window constructor 0x144f4d810 does not touch 0x498 except the zeroing noted in MEMBERS.md.
- Type and meaning of the value. Name to use for the element member: Member8 (project-local, same convention as Member490).

## 2. Weak-pin gate 0x143d66ef0

### VERIFIED (disassembly, gate at file offset 0x3d66ef0)
- Out pair: the gate calls 0x143d5a140 with rdx = &out at rsp+0x20. out.obj = [rsp+0x20], out.block = [rsp+0x28].
- If out.obj != 0 and [obj+0x1b8] (int32) != 0 and P = [obj+0x1b0] != 0: call vtbl[0x28] of P (bool query, 0x143d66f26). If true: call vtbl[0x48] of P with &byte (0x143d66f49), store that byte to [obj+0x1a8].
- Return value: ([obj+0x1a8] & 0x19) != 0 (0x143d66f57), or false when out.obj == 0.
- Block release (0x143d66f67..0x143d66f90): dec int32 at block+0x8; if zero, call vtbl[0] with rcx = block; dec int32 at block+0xc; if zero, call vtbl[0x8] with rcx = block, edx = 1.
- Layout match: UE 4.25 FReferenceControllerBase (Core/Public/Templates/SharedPointerInternals.h:71-93) has a vptr at +0, SharedReferenceCount at +8, WeakReferenceCount at +0xc. The release order matches ReleaseSharedReference / ReleaseWeakReference (SharedPointerInternals.h:351, 365-375), where DestroyObject is the first virtual (line 93). Slot identities in the concrete block vtable are not resolved, so no engine name is given.

### VERIFIED (disassembly of 0x143d5a140, out pair writer)
- Primary path: if W+0xe8 != 0 and block = W+0xf0 != 0 and block[+8] > 0: block[+8] is incremented, out = {W+0xe8, block}.
- Fallback path: block = W+0xe0 (incremented if its count is nonzero), object = W+0xd8.
- The primary path resembles TWeakPtr::Pin (SharedPointer.h:1219, Pin() returns TSharedPtr(*this)). The fallback pair W+0xe0/W+0xd8 is not in that UE function, so no engine name is assigned. Keep NativeWeakPinOut143d5a140.

### UNKNOWN
- Pinned-object type (the object at W+0xe8). Writers were searched with find_field_refs.py (0xe8@0x144f5fbc0 and 0xf0@0x144f5fbc0, window 0x200000, --all-access; output in /tmp/claude-1000/confirm_members2/refs_e8_f0.json). Qword stores to +0xe8 and +0xf0 in the same function were found in 18 functions. None is the window constructor 0x144f4d810. The candidates 0x144f4cfd0 and 0x144f4f6a0 are the description copy path, since desc+0xe8 is NotificationHandler (NOTES.md, 0x142b1bea0). No other candidate was verified as a window writer.
- Meaning of obj+0x1b0, +0x1b8, +0x1a8, and the identities of P's vtable slots 0x28 and 0x48.
- Block vtable slot 0 and slot 8 targets.

## 3. Helper 0x144f5fcc0 (slot 0x270 receiver)

### VERIFIED
- Decompile (decomp-imports/Imports/NativeHelper144f5fcc0__144f5fcc0.c): FUN_144f5fcc0(param_1 = obj, param_2 = bool). Receiver = [obj+0x2c0]; if null, return. Calls vtbl[0x270](receiver, dl), dl = 4 if param_2 != 0, else 1.
- Caller scan: a scan of the whole image for E8/E9 rel32 to 0x144f5fcc0 finds one site, 0x14528e4fd. An abs64 scan finds no table reference. The site is inside the function at 0x14528e490 (decomp: NativeCaller14528e490__14528e490.c). That function calls the helper with (param_1, aiStack[0] != 0).
- 0x14528e490 is a registered native function: the abs64 scan finds it at 0x146f7b578. The name pointer at 0x146f7b570 is 0x146f7aaf8, the string "HandleDebugHUDObjectiveHeightChanged". The table at 0x146f7b570 is listed as native_arrays of block 0x146f79a80 in porting/generated/reflection/native_class_index.json (native_count 5).
- Owning class: class_name_map.json maps 0x146f79a80 to UFortMissionTracker. Its reflection record (classes_in_groups_with_fort_names index 6117) has function HandleDebugHUDObjectiveHeightChanged (create_func 0x14528c7c0).
- Receiver member: UFortMissionTracker reflected property DebugHeightEstimate, offset 704 = 0x2c0, kind Object, type class 0x14080eff0. class_ref_map.json maps 0x14080eff0 to 0x145a05e80, and class_name_map.json names 0x145a05e80 UCommonNumericTextBlock.
- So the helper operates on UFortMissionTracker.DebugHeightEstimate (not a UImage). The slot 0x270 target for UCommonNumericTextBlock is not resolved (native_class_index vtable_addr null). UWidget::SetVisibility is NOT claimed for this receiver.

### UNKNOWN
- That param_1 of 0x14528e490 is the UFortMissionTracker instance. This follows from the native-table convention, not from the body; the body reads param_2+0x18, +0x20, +0x80, which were not matched to a known struct.
- Meaning of the flag: the caller's aiStack[0] comes from 0x142ed9d20 / 0x142ed9cf0 (out-param). Its meaning, and what the 4 vs 1 choice means for DebugHeightEstimate, are not verified.
- Vtable slot 0x270 target for the receiver's class.
