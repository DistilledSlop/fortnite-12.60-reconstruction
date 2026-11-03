# HideToast multicast delegates: findings

Scope: NOTES.md UNKNOWN items 2 and 3 (the three multicast delegates used by HideToast at 0x144e0c9c0).
Evidence: disassembly of image-125.runtime.bin (file offset = VA - 0x140000000), engine source, and
porting/generated/reflection/fortnite_game_reflection.json (class names via class_name_map.json).
Nothing here is a guess. Anything not shown is UNKNOWN.

## Call sites in HideToast body (0x144e0c9c0)
- Asm 0x144e0c9f2-0x144e0c9f9: `mov %rbx,%rdx`; `lea 0x78(%rax),%rcx`; `call 0x140690620`. Delegate = TeamMember+0x78, listener = this.
- Asm 0x144e0ca1a-0x144e0ca24: `lea 0x250(%rax),%rcx`; `mov %rbx,%rdx`; `call 0x140690620`. Delegate = Player+0x250, listener = this.
- Asm 0x144e0ca29-0x144e0ca3f: `mov 0xd8(%rsi),%rcx`; `add $0xe8,%rcx`; `call 0x140690620`. Delegate = Player+0xd8 profile +0xe8.
- Asm 0x144e0ca44-0x144e0ca5a: `mov 0xe8(%rsi),%rcx`; `add $0xe8,%rcx`; `call 0x140690620`. Delegate = Player+0xe8 profile +0xe8.
- Asm 0x144e0ca78-0x144e0ca8b: `lea 0x258(%rbx),%rcx`; `xor %edx,%edx`; `jmp 0x1406a9dc0`. Broadcast of OnToastFinished (+0x258), rdx = 0.
- No call in HideToast adds a binding (no AddUObject-shaped call). The only Broadcast is on +0x258, not on the three delegates.

## Callee identity (shape match, not exact symbol)
- 0x140690620 takes (delegate in rcx, UObject* in rdx) and loops over 16-byte entries (TArray Data at +0, Num at +8,
  entry = Object at +0, FName index at +8). This matches `FMulticastScriptDelegate::RemoveAll(const UObject*)`
  (Engine Core/Public/UObject/ScriptDelegates.h:384-395; TScriptDelegate layout ScriptDelegates.h:253-256,
  Object then FunctionName, 16 bytes).
  Caveat: the source loop runs downward (`--BindingIndex`), the asm loop runs upward. Identity is by argument shape and
  entry layout only.
- 0x1406a9dc0 takes (delegate in rcx, Parameters in rdx) and loops over the invocation list. This matches
  `FMulticastScriptDelegate::ProcessMulticastDelegate(void* Parameters)` (ScriptDelegates.h:479-500). Shape match only.

## Delegate at TeamMember +0x78 (NativeMulticast78)
- Type: container shape is FMulticastScriptDelegate (16-byte TScriptDelegate entries, see above). Reflected kind: UNKNOWN.
- Signature (parameter count and types): UNKNOWN. No Add or Broadcast on this member; no reflected property at this offset.
- Owner class: UNKNOWN. Reflection of UFortTeamMember (block 0x14652f830, class_name_map.json) lists only
  SocialUser at offset 40. Its generated header porting/native/Source/FortniteGame/Generated/UFortTeamMember.h has only that member.
  UFortTeamMemberCampaign (block 0x14652f900) has 0 reflected properties and is declared
  `: public UFortTeamMember` (porting/native/Source/FortniteGame/Generated/UFortTeamMemberCampaign.h:9-10).
- Verification of the team member as UFortTeamMemberCampaign: NOT VERIFIED. The pointee class of NativeTeamMember is not
  established by any evidence here. The existing guess stays a guess.

## Delegate at Player +0x250 (NativeMulticast250)
- Type: same container shape as above. Reflected kind: UNKNOWN. Signature: UNKNOWN.
- Owner: UNKNOWN. The registered-player class is not verified. The wrapper return type (0x142292260 -> 0x141e62c80) is the
  existing evidence only.
- Consistency only: UFortRegisteredPlayerInfo (block 0x146502df0) reflects CommonPublicProfile at offset 216 (0xd8) and
  AccountProfile at offset 232 (0xe8). The body reads exactly these two pointers at Player+0xd8 and Player+0xe8.
  This matches the class but does not prove the wrapper's return type.
- Reflected properties of UFortRegisteredPlayerInfo have no entry at 592 (0x250). Offset 0x250 falls in the unreflected gap
  between offsets 472 and 760.

## Delegate at UMcpProfile +0xe8 (NativeMulticastE8), used on profile objects
- Profile types (type refs from the reflected pointers): AccountProfile (offset 232) resolves to class getter 0x14299e680
  -> block 0x14643d6d0 = UFortMcpProfileCampaign. CommonPublicProfile (offset 216) resolves 0x1429a6020 -> block
  0x14643f020 = UFortMcpProfileCommonPublic. Source: porting/generated/reflection/class_ref_map.json "resolved".
- Reflected delegate at +0xe8 in these classes: none. UMcpProfile (block 0x145c00e80) lists properties up to offset 208.
  UFortMcpProfile (0x146438d70) and UFortMcpProfileCommonPublic (0x14643f020) have 0 properties.
  UFortMcpProfileCampaign (0x14643d6d0) has one delegate, OnDifferentHeroLoadoutActivated, at offset 752 (0x2f0).
- Signature: UNKNOWN. Owner: UNKNOWN (the +0xe8 member is not reflected in UMcpProfile or the two subclasses).

## Reference: OnToastFinished (+0x258), the one broadcast in HideToast
- Reflected: OnToastFinished, kind InlineMulticastDelegate, offset 600 (0x258), block 0x146f9f600
  (fortnite_game_reflection.json, UFortPlayerPowerRatingToast).
- Signature: OnToastFinished__DelegateSignature, params [] (0 params), flags 0x130000 (Delegate, MulticastDelegate).
  Create func 0x1452a1280. This is consistent with the null Parameters pointer in the broadcast call (rdx = 0).

## Summary of UNKNOWN items after this pass
- Delegate types beyond FMulticastScriptDelegate-shape: UNKNOWN for all three (no reflected property, no Add/Broadcast call).
- Parameter count and types: UNKNOWN for all three.
- Owners: UNKNOWN for all three. UFortTeamMemberCampaign is NOT VERIFIED.
- UFortRegisteredPlayerInfo for the player: consistent with the +0xd8/+0xe8 offsets, not verified as the wrapper return type.
