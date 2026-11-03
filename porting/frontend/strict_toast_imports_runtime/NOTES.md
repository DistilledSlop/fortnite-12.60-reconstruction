# Strict toast imports: fragment status

Evidence: the generated `.c`/`.asm` files under `porting/generated` contain no decompilation for
these addresses. The disassembly was produced with `objdump -D -b binary -m i386:x86-64` from
`porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin` (file offset = VA - 0x140000000).
All 31 windows in `original-windows.json` match their SHA-256 values. Helper bodies were read the same way.

## FortPlayerPowerRatingToast.HideToast (partly implemented, not closed)

Files: `power-hidetoast-declaration.inc`, `power-hidetoast-definition.inc`.

- The control flow is reproduced from the disassembly: IsValid, team multicast RemoveAll, registered-player
  lookup, RemoveAll on the registered-player and both profile multicasts, weak reset, and an unconditional
  OnToastFinished broadcast.
- Blocked, not compilable (marked `UNRECOVERED_*`):
  1. `UNRECOVERED_TEAM_MEMBER_TOAST_MULTICAST`: native multicast at UFortTeamMember +0x78. Reflection record
     `0x14652f830` has one reflected property (SocialUser, +0x28), so +0x78 is a non-UPROPERTY native member.
     Type and name are unknown; only `RemoveAll(this)` is used.
  2. `UNRECOVERED_FIND_REGISTERED_PLAYER`: wrapper `0x142292260`, signature recovered as
     `UFortRegisteredPlayerInfo* (UFortTeamMember* TeamMember)`. Name unknown.
  3. `UNRECOVERED_REGISTERED_PLAYER_MULTICAST`: native multicast at UFortRegisteredPlayerInfo +0x250. Reflected
     properties jump from +0x1d8 to +0x2f8, so the slot is native. Type and name unknown.
  4. `UNRECOVERED_PROFILE_MULTICAST`: native multicast at UMcpProfile +0xe8, used for both profiles. UMcpProfile
     reflected properties end at +0xd0. Type and name unknown.
- Correction: the game-instance helper `0x140f930b0` is not called from HideToast. It is called inside the wrapper
  `0x142292260` (generic flag-gated Outer read, +0x20). The wrapper walks three Outer hops from the team member and
  passes the third to `FindToastRegisteredPlayer`. The third object's type is not confirmed as UFortGameInstance.
- Confirmed by reflection: `UFortRegisteredPlayerInfo` `CommonPublicProfile` is at +0xd8 and `AccountProfile` at +0xe8.
  `UFortPlayerPowerRatingToast` `OnToastFinished` is at +0x258.
- The `NativeTeamMember` offset (+0x290) follows the last reflected property (+0x288). This is consistent with a
  native member but is not a full layout proof.
- SetToast for the power toast (`0x144e1cd20`) and the setup-subscriptions function (`0x144e22b70`) are not
  recovered. Both produce the weak team member and the multicast bindings.

## UFortFriendSubscriptionToastWidget.SetToast (declaration only)

File: `friend-settoast-declaration.inc`. The body is not implemented. The disassembly (1243 bytes) has these
unidentified parts:
- The bHasAction branch and its player/controller input path.
- The MCP friend-subscription operation at `0x140cd1e80`.
- Analytics at `0x1417ef620`.
- The native notification action and the presentation calls (`0x145238060`, `0x145803c70`).
- The virtual slots on the notification and subscription objects (+0x288, +0x2e8, +0xbd8, +0x2c8, +0x270, +0x268, +0x150).

The social action and analytics contracts must be recovered from the callee bodies. No substitute is emitted.
