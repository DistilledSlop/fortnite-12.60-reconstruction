# Confirmation click helpers: callee and field identification

Source: /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (file offset = VA - 0x140000000).
Reflection: porting/generated/reflection/fortnite_game_reflection.json (class block 0x146f02cc0 = UFortConfirmationWindow).
Not built, not run, not committed. Names below are proposed labels, not symbols from the shipping exe.

## Corrections to NOTES.md

- Byte +0x596 is compared against dl, which is 0 on the confirm path (`cmp %dl,0x596(%rdi)` at 0x144f5fc22). So the branch is "byte == 0 goes to 0x4f55c60 path", as NOTES.md says. Verified.
- The description at W+0x4b0 is inline and larger than the reflected header implies (see +0x596 below).

## Callees

### 0x143d66ef0 (gate on window W)
- Evidence: calls 0x143d5a140(W, &out). 0x143d5a140 reads W+0xe8 (object ptr) and W+0xf0 (shared ref block), increments the refcount if > 0, and writes (ptr, block) to out. That is the TWeakPtr::Pin / TSharedPtr-from-weak pattern. Gate then reads pinned+0x1b8 (int) and pinned+0x1b0 (ptr), calls vtable+0x28 (bool query) and vtable+0x48 (writes a byte into pinned+0x1a8), and returns `(byte at pinned+0x1a8) & 0x19 != 0`.
- Best name: `UFortConfirmationWindow::IsDialogInputAccepting` (a pinned-widget state gate). Name is a guess.
- Confidence: verified for the pin and flag shape; inferred for the meaning of mask 0x19 and the pinned type.

### 0x144ade110 (typed outer lookup, W)
- Evidence: calls 0x143dbe060 (no-argument static getter, see below). Compares the class depth/parent array of `W->ClassPrivate` (+0x10) against that class (UE IsChildOf-style depth check). If no match, walks `W->Outer` (+0x20) and then `Outer->Outer`. On a match, reads a second cached class global (0x1481134c8, lazily filled via 0x142cbc9d0 and 0x142cc0600) and calls vtable+0x290 on the matched object, then tail-jumps to 0x144ade1e0. Returns null if nothing matches.
- Best name: `GetTypedOuterOrOwnerFromWidget` (walk self/outer/outer-outer for a class, then convert). Not "Athena controller": the code never compares against a controller-specific class that I could read.
- Confidence: verified for the walk shape; inferred for the class identity. The class-name FName lookup (0x148382580 holds FName index 0x44d) is not decoded.

### 0x143dbe060 (static class getter)
- Evidence: ignores its rcx argument, uses a guard word at 0x148321628 and lazy init, the usual UE StaticClass() pattern.
- Best name: `UClass* <T>::StaticClass()` for the class T used by 0x144ade110. T is unidentified.
- Confidence: inferred.

### 0x144ce33d0 (dispatch to controller-side presenter)
- Verified args: rcx = controller (from 0x144ade110), dl = bool, r8 = matched value (the 8-byte element +0x8), r9 = &W+0x4b0 (ConfirmationDescription), 5th arg = bool, 6th arg = &W+0x490.
- Evidence: calls 0x141f8f0a0 with r8 and 0x141f87900 with the 6th arg (W+0x490). Iterates an array in the description (+0xf8 data, +0x100 count) and calls vtable+0x28 on the element. Looks like a list of pending dialog entries being presented.
- Best name: `ControllerPresentConfirmationDialog` (unknown exact owner).
- Confidence: verified for argument order; inferred for behavior and name.

### 0x144f55c60 (present wrapper, called from confirm path when byte 0x596 == 0)
- Evidence: gate 0x143d66ef0(W). Copies ConfirmationDescription (W+0x4b0) via 0x144f4cfd0 (copy-construct FFortDialogDescription_NUI: out = rcx, src = rdx). Calls 0x144f75f90(W) and 0x144f55ae0(W), gets controller from 0x144ade110(W), builds a local with 0x141f7ce20 at rsp+0x178, then calls 0x144ce33d0(controller, dl, value, &local, 0, &W+0x490). The value is passed by pointer and dereferenced at entry (`mov (%rdi),%r8`).
- Best name: `UFortConfirmationWindow::PresentConfirmation` (copy the description, reset state, dispatch).
- Confidence: verified for the call sequence; inferred for the name.

### Other callees (unresolved, args only)
- 0x144f4cfd0: copy-construct FFortDialogDescription_NUI (out, src). Inferred.
- 0x144f75f90(W), 0x144f55ae0(W): called once each before the controller lookup, no args except W. Unknown; likely state reset. Unverified.
- 0x141f7ce20(local): builds a value at rsp+0x178 used as the 6th arg. Unknown.
- 0x143d852b0(EntryBox_SimpleButtons, dl=1): setter with a bool. Probably SetVisibility. Inferred.
- 0x1407f7e20(CommonLoadGuard_LatentSpinner, dl=1): tail call with bool. Probably a loading-state setter. Inferred.
- 0x141f8f0a0 and 0x141f87900: operate on the value and on W+0x490. Unknown.

## Field owners

### W+0x490 (8-byte slot)
- Owner: UFortConfirmationWindow, native (unreflected) member. Verified layout gap: reflected CommonLoadGuard_LatentSpinner is at 0x488, ConfirmationButtons at 0x498, nothing at 0x490 in reflection.
- Use: passed by address (`lea 0x490(%rdi),%rcx` at 0x144f5fc48) as the 6th argument to 0x144ce33d0, and 0x141f8f0a0 / 0x141f87900 act on it. It is a handle-like object (for example a weak/shared ref).
- Confidence: owner verified (layout gap). Type inferred.

### W+0x596 (byte)
- Owner: most likely inside the inline ConfirmationDescription (FFortDialogDescription_NUI, at W+0x4b0), at local offset +0xe6. Reason: the description is read at +0xf8 and +0x100 in 0x144ce33d0 (so it spans at least 0x108 bytes, to 0x5b8), and 0x596 falls inside that span. Reflection has no property for it (the struct's reflected fields are not present in the JSON), and the generated header does not include FFortDialogDescription_NUI.h.
- Meaning: 0 selects the 0x144f55c60 present path, nonzero selects the dispatch path (calls the vtable+0x268 on the button, then 0x144ade110 and 0x144ce33d0).
- Confidence: access and branch verified; owner inferred from layout. Field name unknown.

## FFortConfirmationButtonInfo (element stride 0x10)
- Stride: verified. Loop computes `count << 4` (`shl $0x4` at 0x144f5fbef) over data ptr +0x498 and count +0x4a0.
- +0x0: `UCommonButton* Button`. Verified: reflected inner property "ConfirmationButtons" at offset 0x0, and generated header `FFortConfirmationButtonInfo.h` declares Button only.
- +0x8: 8-byte value, passed by value as r8 to 0x144ce33d0 and by pointer to 0x144f55c60. Its type is unreflected and unknown (pointer-sized). Confidence: verified size and use; type unknown.
- Element size 0x10 is consistent with the generated struct (only Button is declared; the rest of the 0x10 is unreflected).

## Open items
- Decode the FName at 0x148382580 (value 0x44d) to name the class looked up by 0x144ade110.
- Resolve 0x3d852b0, 0x1407f7e20, 0x144f75f90, 0x144f55ae0, 0x141f7ce20 with a one-level read each.
- Get FFortDialogDescription_NUI's size and fields to confirm the +0xe6 owner for 0x596.
