# HideToast translation: resolution round 3

Evidence: image-125.runtime.bin (file offset = VA - 0x140000000), disassembly by objdump -D -b binary -mi386:x86-64.
Reflection: porting/generated/reflection/fortnite_game_reflection.json, struct_blocks.json, class_ref_map.json.
Engine: /opt/fortnite/UnrealEngine/Engine/Source (CoreOnline.h, OnlineReplStructs.h). Decompiles are not used as evidence.
Nothing here is a guess. Anything not listed as VERIFIED stays UNKNOWN.
No definitions-final.inc / declarations-final.inc is written: UNKNOWN items remain.

## Item 1: virtual slot 0x268 (index 77) on the team member

### VERIFIED
- Vtable 0x1465318e0 (UFortTeamMemberCampaign) at +0x268 holds 0x142292160 (image bytes at file offset 0x1465318e0+0x268 - 0x140000000).
- 0x142292160 (asm): `this` = rcx (TeamMember); rdx = out buffer; `rbx = rdx` (0x142292172).
  - IsValid(this+0x90) (call 0x142ee7880), then Get(this+0x90) (call 0x142ee5dd0). Path A: out+0x0 = vtable 0x145a95878;
    out+0x8 = [obj+0x238]; out+0x10 = [obj+0x240] with `incl 0x8(%rcx)` on that pointer (0x1422921ad-0x1422921b6). Returns rax=0.
  - Other paths as recorded in NOTES.md round 2 (path B uses this+0x38, path C calls 0x140f96040, zero path).
- The wrapper 0x142292260 uses the returned rax as the IdRef (rax = rdx = out buffer; wrapper epilogue, rbx at 0x142292222).

### Out-buffer type: FUniqueNetIdRepl (VERIFIED by layout match)
- Reflection struct record UniqueNetIdRepl (block 0x147d99840): size 40, align 8, construct_fns [0x1447a2020].
  Its ReplicationBytes is an Array at offset 24 (0x18); the record's offset-0 "Byte" entry is the array element type.
- The same getter 0x1447a2020 is the type of UFortRegisteredPlayerInfo::PlayerId (Struct, offset 56 = 0x38),
  so PlayerId is FUniqueNetIdRepl (VERIFIED by the struct ref).
- Engine source: FUniqueNetIdWrapper (CoreOnline.h:197-215) has a virtual destructor (vtable at +0), and
  `TSharedPtr<const FUniqueNetId> UniqueNetId` (object at +0x8, controller at +0x10). FUniqueNetIdRepl derives from it and adds
  `TArray<uint8> ReplicationBytes` (OnlineReplStructs.h:104) at +0x18, total 0x28 = 40.
- Match with the asm: out+0x0 vtable, out+0x8 object, out+0x10 controller, out+0x18 ReplicationBytes data (zeroed on
  paths A, B, zero: lStack_20 = out+0x18 = 0), out+0x20 Num/Max (zeroed in the common epilogue). Size and offsets match
  the reflected 40-byte struct.
- Wrapper cleanup (NOTES.md round 2): lStack_20 (out+0x18, ReplicationBytes data) is freed by 0x142bee710 and plStack_28
  (out+0x10, controller) is released by the inline sequence. This is the shape of the FUniqueNetIdRepl destructor.
  Destructor binding to a name: NOT VERIFIED (UNKNOWN).

### Type of the returned object
- Slot 0x268 returns `FUniqueNetIdRepl*` (the out buffer, passed by reference to the find).
- Vtable values 0x145a95878 and 0x145a95888 (buffer+0x0): bound to FUniqueNetIdRepl? NOT VERIFIED (no RTTI). UNKNOWN.

### Registered-player lookup 0x141e62c80 (VERIFIED)
- Signature: `UFortRegisteredPlayerInfo* FindToastRegisteredPlayer141e62c80(GI, const FUniqueNetIdRepl* IdRef)`
  - IdRef: FUniqueNetIdRepl, see above (VERIFIED by the wrapper's use of the 0x268 return).
  - Return: element of GI->RegisteredPlayers, element class UFortRegisteredPlayerInfo (VERIFIED: reflected UFortGameInstance
    property RegisteredPlayers, Array, offset 0x710; inner Object class ref 0x142abf9e0 resolves to block 0x146502df0).
  - Container scan: data at GI+0x710, count int32 at GI+0x718 (asm 0x141e62cb1-0x141e62cc3).
- Element match: element e: idx = int32 at e+0xc (UObjectBase::InternalIndex, asm 0x141e62cd0), resolved through the global
  object table (0x148263a2c, 0x148263a18, 0x18-byte items). Items with flag bits 0x30000000 set are skipped (asm 0x141e62d0d).
  Then `0x140d7c5a0(e+0x38, IdRef)` compares PlayerId (e+0x38) against IdRef. e+0x38 = UFortRegisteredPlayerInfo::PlayerId
  (reflected, offset 56, type FUniqueNetIdRepl as above). The first match is returned.
- Id compare 0x140d7c5a0 (A = PlayerId, B = IdRef): valid(X) = X+0x8 non-null and `(*(X+8))->vslot[5]()`.
  Compare = `(*(A+8))->vslot[0](*(B+8))`. Consistency only: under the MSVC slot order of FUniqueNetId (CoreOnline.h:111-169:
  Compare first virtual, dtor, GetType, GetBytes, GetSize, IsValid, ToString), slot 0 = Compare and slot 5 = IsValid.
  NOT VERIFIED (no concrete vtable bound). UNKNOWN.

### Wrapper signature (VERIFIED where marked)
    UFortRegisteredPlayerInfo* NativeToastFindRegisteredPlayer142292260(UFortTeamMemberCampaign* TeamMember)
  - TeamMember class: UFortTeamMemberCampaign (VERIFIED, see item 3).
  - Return: UFortRegisteredPlayerInfo* (VERIFIED: rax of 0x141e62c80 returned unchanged, asm 0x141e62d46).
  - Argument chain Outer hops and 0x140f930b0: VERIFIED in NOTES.md round 1.
  - Game-instance argument: the array offset (0x710) is verified; that the object reached by Hop2->Outer is a
    UFortGameInstance at runtime is NOT verified. UNKNOWN.

## Item 2: owners of the three RemoveAll'd multicast delegates

### TeamMember +0x78 (NativeMulticast78)
- VERIFIED owner: UFortTeamMember (base class). The base constructor 0x142afa270 (the body of the UFortTeamMember internal
  constructor 0x142af8560, see TEAMMEMBER.md) stores zero to +0x78 and +0x80 (`mov %rax,0x78(%rbx)`, `mov %rax,0x80(%rbx)`),
  and stores +0x30/+0x38 as invalid weak pointers (-1, 0). The Campaign constructor 0x142af85c0 calls 0x142afa270 first
  and only stores its vtable after (TEAMMEMBER.md), so +0x78 is initialized by the UFortTeamMember layout.
  Reflection of UFortTeamMember (block 0x14652f830) lists only SocialUser at 0x28, so +0x78 is not reflected.
- Element type / kind: UNKNOWN. Two inconsistent stride facts exist between the notes: the RemoveAll loop 0x140690620
  is described with 16-byte entries (DELEGATES.md), while the Add sites 0x144e22b70 build 0x30-byte entries (LISTENERS.md).
  Not resolved here.
- Signature: UNKNOWN. The only Broadcast on the HideToast path is OnToastFinished (+0x258), not this delegate.
  The bound callback is HideToast 0x144e0c9c0 (Ghidra shows 1 `longlong` parameter, not a verified signature).

### Player +0x250 (NativeMulticast250)
- Owner: UNKNOWN. UFortRegisteredPlayerInfo has no reflected property at 0x250 (offset 592 falls in the gap between 472 and 760).
  The class constructor storing this delegate was not located: the registration function 0x142ac02f0 passes constructor
  candidate 0x2ac12e0, which is the static-class/registration path (it calls 0x142ac02f0 and many 0x2ac10xx helpers),
  not an instance constructor scan that touches +0x250. UNKNOWN.
- Signature: UNKNOWN. Bound callback TeamPlayerCb 0x144e00850 (Ghidra: 2 parameters, not verified types).

### Profile +0xe8 (NativeMulticastE8), profile = Player+0xd8 (CommonPublicProfile) and Player+0xe8 (AccountProfile)
- Profile classes (VERIFIED, reflected pointers resolved): AccountProfile -> UFortMcpProfileCampaign (block 0x14643d6d0);
  CommonPublicProfile -> UFortMcpProfileCommonPublic (block 0x14643f020).
- Reflected properties: UMcpProfile (block 0x145c00e80) top offset 208; UFortMcpProfileCommonPublic has 0 properties;
  UFortMcpProfileCampaign has OnDifferentHeroLoadoutActivated at 0x2f0 only. No reflected member at 0xe8 in any of them.
- Owner: UNKNOWN (the +0xe8 member is not reflected and no constructor store was located).
- Signature: UNKNOWN. Bound callback ProfileCb 0x144e07780 (Ghidra: 4 parameters, not verified types).

## Item 3: class of the toast's NativeTeamMember pointee: RESOLVED

- VERIFIED (asm, file offsets via VA - 0x140000000): SetToast body 0x144e1cd20 calls 0x142aef8a0 (UFortTeamMemberCampaign
  class getter, block 0x14652f900 noregister 0x142aef8a0), then compares the object's class inheritance entry
  (`cmp 0x38(%rcx),%edx`; `cmp %rax,(%rcx,%rdx,8)`), the fast IsA pattern. Only after this check does it call
  `lea 0x290(%rsi),%rcx; call 0x142ee3b30` to store the object into the weak slot at this+0x290.
- Therefore NativeTeamMember is TWeakObjectPtr<UFortTeamMemberCampaign>, and the class of its pointee is UFortTeamMemberCampaign
  (an IsA check against that class), not UFortTeamMember.
- Note: IsA is checked against the static class. A subclass of UFortTeamMemberCampaign would also pass, but no subclass is
  reflected here. The vtable check in TEAMMEMBER.md (0x1465318e0 for the slot 0x268 target) is consistent.

## Remaining UNKNOWN items (after round 3)
1. Pointee class of this+0x90 in 0x142292160 and the class of [obj+0x238] / [obj+0x240] (fields written into the out buffer).
2. Vtable values 0x145a95878 / 0x145a95888 binding to FUniqueNetIdRepl (no RTTI).
3. Virtual slot bindings used by 0x140d7c5a0 (slot 0 Compare, slot 5 IsValid): consistent with MSVC order, not bound.
4. Game-instance type reached by the wrapper's Hop2->Outer chain (UFortGameInstance at runtime not verified).
5. Path C of 0x142292160 (0x140f96040 effect on out+0x10/+0x18) (from round 2).
6. NativeCtrlRelease142292260 semantics and the FUniqueNetIdRepl destructor name binding (0x142bee710 free).
7. Second argument (0) of 0x1406a9dc0 (OnToastFinished broadcast).
8. +0x78 delegate element type, kind, and the stride conflict between 0x140690620 and 0x144e22b70.
9. Owner class of +0x250 on the registered-player class (no constructor store located).
10. Owner class of +0xe8 on UMcpProfile-derived profile classes; signature of the +0x250 and +0xe8 delegates.

UNKNOWN count left: 10 (numbered items above; the listed owners and signatures are counted once each).
