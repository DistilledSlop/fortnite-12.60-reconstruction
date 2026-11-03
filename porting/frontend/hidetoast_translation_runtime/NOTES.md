# UFortPlayerPowerRatingToast::HideToast: translation notes

Body VA is 0x144e0c9c0 (not 0x142292...). The wrapper is 0x142292260. Decomp for the body was generated with
decompile_targets.py (--only 0x144e0c9c0:Imports.HideToast --create-missing --no-index), output
decomp-imports/Imports/HideToast__144e0c9c0.c. HideWrapper__142292260.c and OuterRead__140f930b0.c were already present.
Disassembly (image-125.runtime.bin, file offset = VA - 0x140000000) matches the body pseudo-C, including the call
at body +0x4d (rcx = TeamMember) and the tail jump (jmp) into 0x1406a9dc0 at the end of the body.

## Flow (from pseudo-C, consistent with the existing definition)
1. IsValid(NativeTeamMember at this+0x290). If valid:
2. Get -> RemoveAll on TeamMember+0x78 with listener this.
3. Get again -> wrapper 0x142292260 -> Player.
4. If Player: RemoveAll on Player+0x250; then on Player+0xd8 (CommonPublicProfile) +0xe8 and Player+0xe8 (AccountProfile) +0xe8.
5. Weak reset: this+0x290 = -1, this+0x294 = 0.
6. Unconditional broadcast of OnToastFinished (this+0x258), second argument 0.

## Wrapper 0x142292260
- Hop1 = TeamMember->Outer (+0x20) unless TeamMember+0x8 bit 0x10 is set (then null).
- Hop2 = OuterOrNull(Hop1) (0x140f930b0): 0 when Hop1+0x8 bit 0x10 is set, else Hop1->Outer.
- GameInstance candidate = Hop2->Outer. Passed to FindToastRegisteredPlayer (0x141e62c80).
- PlayerId from vtable slot 0x268 (index 77) with a hidden return buffer (rdx). Its return value (rax) is the
  buffer address and is passed as the reference argument.

## Naming applied
- Reflected: HideToast, NativeTeamMember is project-local (existing evidence), OnToastFinished (reflected at +0x258).
- Address-suffixed natives: NativeWeakPtrIsValid142ee7880, NativeWeakPtrGet142ee5dd0, NativeMulticastRemoveAll140690620,
  NativeDynamicMulticastBroadcast1406a9dc0, NativeToastFindRegisteredPlayer142292260, NativeOuterOrNull140f930b0,
  NativeFindToastRegisteredPlayer141e62c80, NativeCtrlRelease142292260, NativeFreeData142bee710.
- Multicast members: NativeMulticast78, NativeMulticast250, NativeMulticastE8.
- Virtual slot 0x268 (wrapper) is the vtable entry for implementation 0x142292160 (existing evidence); no name bound.

## VERIFIED
Evidence: asm = image-125.runtime.bin (file offset = VA - 0x140000000), decompiled pseudo-C is not used as evidence.
1. (was item 1) NativeTeamMember is the weak slot at this+0x290 (8 bytes).
   - Asm 0x144e0c9d4 and 0x144e0c9e1: HideToast passes this+0x290 to the IsValid call (0x142ee7880) and the Get call.
   - Asm 0x144e0ca64 and 0x144e0ca6e: movl -1 to 0x290 and movl 0 to 0x294, the same ObjectIndex/ObjectSerialNumber
     fields as TWeakObjectPtr (WeakObjectPtr.h:235-236, `int32 ObjectIndex; int32 ObjectSerialNumber;`).
   - Reflection (fortnite_game_reflection.json, classes_in_groups_with_fort_names, block_va 0x146f9f600):
     OnToastFinished (InlineMulticastDelegate) at 0x258, Text_PowerDifference 0x268, Banner_MemberBanner 0x270,
     Text_PowerRating 0x278, Text_Title 0x280, Text_Description 0x288 (last, Object = 8 bytes). Contiguous; ends at 0x290.
   - Still unverified: the pointee UFortTeamMemberCampaign of the weak pointer (existing evidence only).
7. (was item 7) ObjectFlags bit 0x10 is RF_ClassDefaultObject: `RF_ClassDefaultObject = 0x00000010` (ObjectMacros.h:486).
   The bit is tested on TeamMember+0x8 (asm 0x142292273, `shr $0x4; test $0x1`), not on Hop1.
   +0x8 is ObjectFlags and +0x20 is OuterPrivate in UObjectBase (UObjectBase.h:202-214 field order, after the vptr).
   When set, the wrapper calls OuterOrNull with rcx=0 (asm 0x142292277-0x14229227f).
9. (partial) The second argument of 0x1406a9dc0 is the constant 0 (asm 0x144e0ca7f `xor %edx,%edx`, tail jump 0x144e0ca8b).
   Its meaning is still UNKNOWN (remains item 9 below).
- Other facts: OnToastFinished is at this+0x258 (lea 0x144e0ca78). The reset and broadcast are on different paths:
  the reset (0x290/0x294) runs only inside the IsValid branch, the broadcast runs on both paths.
- Wrapper out-buffer (partial, item 6): the buffer is at wrapper rsp+0x20. The slot-0x268 implementation at 0x142292160
  writes buffer+0x0 (vtable 0x145a95878), buffer+0x8 (value copied from object+0x238/0x250), and buffer+0x10 (pointer
  with a refcount increment on [ptr+0x8], asm 0x1422921ad-0x1422921b6). The wrapper then overwrites buffer+0x0 with
  0x145a95888 (asm 0x1422922b3). The vtable 0x145a95888 starts with function pointers (no type name bound).
  Binding of slot 0x268 to 0x142292160 for the TeamMember class is still existing evidence only.
- The wrapper has no store to rsp+0x30 or rsp+0x38 (lStack_20 and plStack_28), so their values come from elsewhere.
  Writes to buffer+0x10 (= rsp+0x30) by the 0x142292160 implementation are the only visible candidate (item 8).
- Release sequence in the wrapper (asm 0x1422922c2-0x1422922ee): non-atomic `subl` on [ptr+0x8], vtable[0] call,
  `subl` on [ptr+0xc], vtable[8] call with edx=1. CancelInner pseudo-C (lines 102-109) has the same field shape (item 10 partial).

## UNKNOWN
2. Owner class and type of the multicast at TeamMember+0x78. UFortTeamMemberCampaign has 0 reflected properties
   (block 0x14652f900), so no offset check is possible. Existing evidence only.
3. Type of NativeMulticast250 and NativeMulticastE8. 0x250 is not a reflected field of UFortRegisteredPlayerInfo.
   0xd8 (CommonPublicProfile) and 0xe8 (AccountProfile) are reflected Object properties, but their class refs
   (0x14299e680, 0x1429a6020) do not resolve in class_name_map.json. UMcpProfile +0xe8 is not reflected.
4. Type of the wrapper return. Asm returns the rax of 0x141e62c80 unchanged; the UFortRegisteredPlayerInfo* type is not verified.
5. Game-instance type for 0x141e62c80 (existing header says UFortGameInstance; the third Outer hop is verified in asm
   0x14229227b-0x14229228c, the type is not).
6. Layout and type of the wrapper out-buffer (NativeToastIdOut142292260) and its vtable 0x145a95888 (partial, see VERIFIED).
   Buffer size beyond +0x10 is not established; vtable type name not bound (no RTTI).
8. Sources of lStack_20 and plStack_28: no store in the wrapper (VERIFIED); their binding to the slot-0x268 implementation is UNKNOWN.
9. Meaning of the second argument (0) to the broadcast 0x1406a9dc0 (its value 0 is VERIFIED).
10. Whether NativeCtrlRelease142292260 has the same semantics as the CancelInner sequence (shape matches; CancelInner
    was not checked in asm, so semantics are not verified).

## Observations (not fixed)
- Wrapper: when TeamMember's ObjectFlags bit 0x10 is set (not Hop1's, corrected), OuterOrNull receives 0 and reads 0x8
  (VERIFIED asm 0x140f930b0: `mov 0x8(%rcx)` with no null check). This is a fault path, not a guard.
  The same holds for GameInstance = Hop2->Outer when Hop2 is null (0x14229228c `mov 0x20(%rax),%rbx`, no test on rax).
- The existing definition (power-hidetoast-definition.inc) uses the names UNRECOVERED_*; this folder's names replace them
  but are not integrated. Do not merge without naming review.
