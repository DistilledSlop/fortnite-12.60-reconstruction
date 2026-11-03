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
  1. The UFortTeamMember native multicast at +0x78. Its type and name are not in any header.
  2. The game-instance acquisition helper `0x140f930b0`, applied to the team member.
  3. The UFortRegisteredPlayerInfo native multicast at +0x250.
  4. The UMcpProfile native multicast at +0xe8. Both profile pointers use it.
- The `NativeTeamMember` offset (+0x290) comes from the decompilation. It has not been checked against the
  generated layout, because the generated headers carry no offsets.
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
