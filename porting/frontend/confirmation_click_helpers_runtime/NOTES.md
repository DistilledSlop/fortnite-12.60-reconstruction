# Confirmation click helpers: VA 0x144f5fbc0 (confirm) and 0x144f5fce0 (decline)

Status: bodies recovered from disassembly (control flow verified), callee semantics NOT verified. Not built, not run, not committed.
Source: /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (file offset = VA - 0x140000000). Callee addresses below are file offsets as printed by objdump (VA = offset + 0x140000000).

## Verified (disassembly)
- Confirm helper (rcx = W, rdx = B): null B -> ret. Gate call 0x3d66ef0(W) false -> ret. Scans ConfirmationButtons (ptr +0x498, count +0x4a0, stride 0x10, Button at +0x0) for B; the matched element's +0x8 becomes the value passed on. Byte +0x596 == 0 -> call 0x4f55c60(W, 0, &value), return. Else: vtable +0x268 on B (result unused), controller = 0x4ade110(W), null -> ret, call 0x4ce33d0(controller, 0, value, &ConfirmationDescription, W+0x490, 1), call 0x3d852b0(EntryBox_SimpleButtons, 1), tail-jump 0x7f7e20(CommonLoadGuard_LatentSpinner, 1).
- Decline helper (rcx = W, rdx unused): gate 0x3d66ef0(W) false -> ret. Copies ConfirmationDescription (+0x4b0) via 0x4f4cfd0, calls 0x4f75f90(W) and 0x4f55ae0(W), controller = 0x4ade110(W), null -> cleanup/ret. Builds local at rsp+0x170 with 0x1f7ce20, then 0x4ce33d0(controller, 1, [rsp+0x178], &copy, &local170, 0), cleanup 0x4ae20d0(copy), ret.
- Both helpers call 0x4ce33d0 with a bool in dl and a bool on the stack that are opposite (confirm: 0/1, decline: 1/0). That pattern fits a confirm/decline flag pair.
- Field mapping (window, reflection class 5769, verified against UFortConfirmationWindow.h): +0x448 EntryBox_SimpleButtons, +0x488 CommonLoadGuard_LatentSpinner, +0x498 ConfirmationButtons data, +0x4a0 its count, +0x4a8 DeclineButton, +0x4b0 ConfirmationDescription.
- Decline helper ignores rdx (verified: rdx is not read before the first call).

## Inferred (not verified)
- 0x4ce33d0 may be the confirmation dispatch (reflected UFortConfirmationWindow-adjacent ShowAdvancedLatentActionConfirmation has params ConfirmButtonInputActions, DeclineButtonInputAction, AdditionalContent, LeftAdditionalContent, TargetPlayer). Name match only.
- 0x4ade110 is a class-tree cast helper (performs an IsA-style check on the object returned by 0x3dbe060). It may be the "Athena controller" cast from the BotClickButton notes. Not confirmed.
- 0x3d66ef0 calls 0x3d5a140 and checks flags at +0x1b8 and pointer at +0x1b0 of the result, so it is a world/owner gate. Exact meaning unknown.

## Blockers
1. Window +0x596 byte: owner class and meaning unknown (not in reflected window props, which end past +0x4b0).
2. Window +0x490 slot: unreflected; passed to 0x4ce33d0. Type unknown.
3. ConfirmationButtons element +0x8 value: element type beyond the one-field FFortConfirmationButtonInfo header (only Button at +0x0 is declared). Need the reflected struct layout for FFortConfirmationButtonInfo.
4. Button vtable +0x268 (called on the clicked UCommonButton): slot owner not confirmed.
5. Helpers 0x3d66ef0, 0x4ade110, 0x4ce33d0, 0x4f55c60, 0x3d852b0, 0x4f75f90, 0x4f55ae0, 0x4f4cfd0, 0x1f7ce20, 0x4ae20d0: names and signatures unrecovered beyond the above. Each needs a one-level read before its fragment can be bound.
6. Decline local at rsp+0x178 is set by 0x1f7ce20; its meaning is unknown.
7. Confirm helper's 0x7f7e20 tail call and 0x3d852b0 setter semantics unverified.

## Files
- definitions.inc: fragments with blk_ placeholders (not compilable).
- declarations.inc: signatures and blocker list.
