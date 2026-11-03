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

## VERIFIED (round 2): find (0x141e62c80), id compare (0x140d7c5a0), slot 0x268 out-buffer, wrapper locals
Evidence: asm = image-125.runtime.bin (file offset = VA - 0x140000000); reflection = fortnite_game_reflection.json,
class_name_map.json, class_ref_map.json; decompile = decomp-imports/Imports/FindToastRegisteredPlayer__141e62c80.c and
UniqueNetIdCompare140d7c5a0__140d7c5a0.c (matches the asm).

### 0x141e62c80 FindToastRegisteredPlayer(GI in rcx, IdRef in rdx)
1. Returns 0 unless IdRef+0x8 is non-null and `(*(void**)(IdRef+8))->vslot[5]()` returns true (asm 0x141e62c9a-0x141e62caf).
2. Container: GI+0x710 (TArray data) and GI+0x718 (int32 count), iterated forward (asm 0x141e62cb1-0x141e62cc3).
   Reflected: UFortGameInstance (block 0x1463cbc30) property RegisteredPlayers, kind Array, offset 1808 (0x710).
   Element class: inner property class ref 0x142abf9e0 -> block 0x146502df0 = UFortRegisteredPlayerInfo (class_ref_map.json, resolved).
3. For element e (skip null): idx = int32 at e+0xc (asm 0x141e62cd0). If idx >= global at 0x148263a2c, item = NULL.
   Else item = [0x148263a18 + (idx>>16)*8] + (idx & 0xffff)*0x18 (signed split, asm 0x141e62cd3-0x141e62d05).
   Skip e if `(item+0x8) & 0x30000000` is set (asm 0x141e62d0d). Bit names not bound to engine source here.
4. Match: call 0x140d7c5a0(e+0x38, IdRef) (asm 0x141e62d16-0x141e62d1d). e+0x38 is UFortRegisteredPlayerInfo::PlayerId
   (reflected Struct, offset 56 = 0x38; struct ref 0x1447a2020 not resolved in class_ref_map.json).
5. Return e (rax = rdi, asm 0x141e62d46) on the first match; otherwise return 0 (asm 0x141e62d2f).

### 0x140d7c5a0 id compare (A = PlayerId, B = IdRef)
- valid(X) = X+0x8 non-null and `(*(void**)(X+8))->vslot[5]()` true. If valid(A) != valid(B), return 0.
- If both invalid, return 1 (asm 0x140d7c5eb-0x140d7c600).
- Else return `(*(void**)(A+8))->vslot[0](*(void**)(B+8))` (asm 0x140d7c5ef-0x140d7c5fe).
- Consequence in find: since IdRef was validated in step 1, a match requires PlayerId+8 valid and vslot0 true.
- The vslot0 target and the object type of +0x8 are UNKNOWN (not decompiled).

### Wrapper 0x142292260 locals (items 3 and 8)
- Locals at wrapper rsp+0x20 (out buffer, lea 0x142292284), rsp+0x30 (plStack_28 read at 0x1422922c2), rsp+0x38 (lStack_20
  read at 0x1422922a4). The wrapper has no store to rsp+0x30, rsp+0x38 (full listing 0x142292260-0x1422922fc).
- Call 0x268 at 0x142292293 with rdx = rsp+0x20. Callee return rax = rdx (rbx at 0x142292222 epilogue) is the IdRef passed to find.
- Callee 0x142292160 writes to out (rbx = rdx at 0x142292172):
  - out+0x0 = vtable 0x145a95878 (paths A, B, zero-path), then the wrapper overwrites it with 0x145a95888 (0x1422922b3).
  - Path A (IsValid(this+0x90), 0x142292183): out+0x8 = [Get(this+0x90)+0x238]; out+0x10 = [Get(this+0x90)+0x240]
    (asm 0x14229219b-0x1422921ad).
  - Path B (this+0x90 invalid, this+0x28 null, IsValid(this+0x38) at 0x1422921e8): out+0x8 = [Get(this+0x38)+0x250];
    out+0x10 = [Get(this+0x38)+0x258] (asm 0x1422921f5-0x142292216, shares the 0x1422921ad store).
  - Path C (this+0x90 invalid, this+0x28 non-null, asm 0x1422921bd-0x1422921e2): calls 0x140f96040(this+0x28, out, 0) and
    returns out without the common epilogue. Its effect on out+0x10/+0x18 is UNKNOWN (0x140f96040 not analysed).
  - Path zero (both invalid, 0x142292218): out+0x8 = out+0x10 = 0.
  - Common epilogue (0x142292222-0x142292235, paths A, B, zero): out+0x20 = 0 and out+0x18 = 0.
- Therefore:
  - lStack_20 = rsp+0x38 = out+0x18 = 0 on paths A, B and zero (0x142292235). The free call 0x142bee710 is never reached on
    those paths (test/je at 0x1422922a4-0x1422922bb).
  - plStack_28 = rsp+0x30 = out+0x10 = a pointer with +0x8 refcount incremented (incl at 0x1422921b6) on paths A and B,
    or 0 on the zero path. The wrapper's release (0x1422922c7-0x1422922ee: subl [p+8]; vtbl[0]; subl [p+0xc]; vtbl[8](p,1))
    matches that shape. Release semantics are shape-only (item 10 still open).
- The wrapper's rbx restore (0x142292222 path) and the callee's saved rbx/rsi are consistent with the frame sizes.

### TeamMember vtable (item 2, partial, NOT closed)
- Virtual call in the wrapper: `mov (%rdi),%rax; call *0x268(%rax)` at 0x142292290-0x142292293, rdi = TeamMember.
- Ctor 0x142af85c0 calls base ctor 0x142afa270 (0x142af85d4), then stores vtable 0x1465318e0 to [rbx] (0x142af85e0).
  Ctor 0x142afa270 stores vtable 0x146531640 to [rbx] (0x142afa285).
- Vtable slot 0x268 read from image-125 file data:
  - [0x1465318e0+0x268] = 0x142292160 (matches the wrapper's callee).
  - [0x146531640+0x268] = 0x1422920c0 (different).
- Reflection (native_class_index.json): UFortTeamMemberCampaign vtable_candidate 0x146531640, status "unverified", shared with
  UFortTeamMemberAthena. This conflicts with HANDOFF.md ("campaign constructor 0x142af85c0 ... vtable"). No ctor-to-class
  record exists: construct_fn_to_block.json has no entry for 0x142af85c0, 0x142afa270, 0x14652f900 or 0x14652f830.
- Raw store scan: 596 `mov r64, [reg+0x290]` encodings in the image (109 in 0x144-0x145). Not attributed to the toast; not used.
- Conclusion: the TeamMember class is UNKNOWN. Only vtable 0x1465318e0 has slot 0x268 = 0x142292160 among the two checked.

### Reflected facts used
- UFortPlayerPowerRatingToast (block 0x146f9f600): members end at 0x288 (Text_Description) so NativeTeamMember at 0x290 is not reflected.
- Functions SetToast and ShowToast take an Object param (ToastNotification class ref 0x142b26c60); no team-member param reflected.

## UNKNOWN (after round 2)
2. TeamMember class: UNKNOWN (see above). UFortTeamMemberCampaign binding conflicts between the HANDOFF and the reflection index.
5. Game-instance type for find: the array at GI+0x710 is reflected on UFortGameInstance; the arg's type is still UNKNOWN (the
   wrapper's Hop2->Outer chain is not tied to a class).
6. Out-buffer size beyond +0x20 and the vtable 0x145a95888 type name: UNKNOWN. Path C writes: UNKNOWN.
8. lStack_20 / plStack_28: CLOSED for paths A, B and zero (see above). Path C UNKNOWN.
9. Second argument (0) of 0x1406a9dc0: UNKNOWN (unchanged).
10. NativeCtrlRelease142292260 semantics: UNKNOWN (shape only).
- Closed in round 2: item 4 (return type). The find returns an element of UFortGameInstance::RegisteredPlayers, class
  UFortRegisteredPlayerInfo (reflection class_ref_map resolved 0x142abf9e0 -> 0x146502df0; asm 0x141e62d46).
