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

## UNKNOWN
1. Layout of UFortPlayerPowerRatingToast: whether NativeTeamMember is at +0x290 (follows last reflected +0x288, not proven).
2. Owner class and type of the multicast at TeamMember+0x78 (existing evidence uses UFortTeamMemberCampaign; not re-verified).
3. Type of NativeMulticast250 and NativeMulticastE8 (UMcpProfile for E8 per existing evidence).
4. Type of the wrapper return. The pseudo-C shows undefined8; the UFortRegisteredPlayerInfo* type is from the existing header.
5. Game-instance type for 0x141e62c80 (existing header says UFortGameInstance; the third Outer hop is not confirmed).
6. Layout and type of the wrapper out-buffer (NativeToastIdOut142292260) and its vtable 0x145a95888.
7. Meaning of the ObjectFlags bit 0x10 check (no name bound).
8. Sources of lStack_20 and plStack_28 in the wrapper (no visible assignment).
9. Second argument (0) to the broadcast 0x1406a9dc0.
10. Whether NativeCtrlRelease142292260 has the same semantics as the CancelInner sequence (it is the non-atomic shape of the same release).

## Observations (not fixed)
- Wrapper: when Hop1's ObjectFlags bit 0x10 is set, OuterOrNull receives 0 and reads 0+8 (no guard in the pseudo-C).
  The same holds for GameInstance = Hop2->Outer when Hop2 is null. The pseudo-C has no guard on either.
- The existing definition (power-hidetoast-definition.inc) uses the names UNRECOVERED_*; this folder's names replace them
  but are not integrated. Do not merge without naming review.
