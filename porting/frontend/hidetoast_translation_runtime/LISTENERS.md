# HideToast listener-adding function 0x144e22b70: multicast delegates

Scope: the Add calls in 0x144e22b70 (called last by SetToast body 0x144e1cd20), for the three multicast containers HideToast (0x144e0c9c0) removes itself from.

Evidence: image = /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (file offset = VA - 0x140000000). Disassembly via `objdump -D -b binary -mi386:x86-64`. Decompiles: porting/generated/decomp-imports/Imports/*.c (names below).

## Summary
| Container | Callback bound (verified) | Callback params (decompile) | Owner class |
|---|---|---|---|
| TeamMember +0x78 (`NativeMulticast78`) | HideToast__144e0c9c0 (0x144e0c9c0) | 1 (`longlong`) | UNKNOWN |
| Player +0x250 (`NativeMulticast250`) | TeamPlayerCb__144e00850 (0x144e00850) | 2 (`undefined8`, `longlong`) | UNKNOWN |
| Profile +0xe8 (`NativeMulticastE8`), profile = Player +0xd8 | ProfileCb__144e07780 (0x144e07780) | 4 (`undefined8` x3, `longlong`) | UNKNOWN |

Parameter types are not verified beyond the counts above (Ghidra shows `undefined8`/`longlong`).

## Add sites in 0x144e22b70 (verified by disassembly)
- **+0x78 (TeamMember)**: callback `lea 0x4e0c9c0` at 0x144e22bbf. Weak get `lea 0x290(%r14)` at 0x144e22bc8 (decompile lines 99-104). Then `call 0x140685370` (0x144e22c47, arg 1), `call 0x140690dd0` (grow, 0x144e22c6f), entry written. The 0x30-byte entry is built inline with vtables 0x1459a9e80 / 0x1459a9ed8 (decompile lines 105-129). No contains check before this add.
- **+0x250 (Player)**: callback `lea 0x4e00850` at 0x144e22d50. Weak get at 0x144e22cf7. `call 0xd97b10` contains check (0x144e22d3b, decompile line 139). Then `call 0x140685370` (0x144e22dbb) and `call 0x140690dd0` (0x144e22de3). Entry built inline with vtables 0x1459cb3b8 / 0x1459cb410 (decompile lines 141-168). The object stored in entry +0x20 is the Player-side object (`lVar9`).
- **Profile +0xe8**: callback `lea 0x4e07780` at 0x144e22e6e. Contains check `call 0xd97b10` (0x144e22e93) on `lea 0xe8(%rbx)`. Then `call 0x144de7130` (add helper, 0x144e22ec8). A second pair is at 0x144e22eef / 0x144e22f28 (the Player +0xe8 profile path).
- **Remove (not an add)**: 0x144e22f3b weak get, then `call 0x140690620` on `lea 0x78(%rax)` at 0x144e22f4e (decompile lines 191-192, inside `if (bVar3)`).
- 0x144e22f78 and 0x144e230ba are weak gets before non-delegate calls (decompile lines 195, 256). No delegate Add in them.

## Helpers
- 0x140685370: called with arg 1 before every add. Decompile body (Helper685370__140685370.c) checks entries with vtable +0x20 and compacts them via 0x140690780. Name: MulticastCompact__140685370. Body not fully read.
- 0x140690dd0: grow, called when count+1 > capacity (decompile line 123, 0x144e22c6f). Not decompiled.
- 0xd97b10: contains check. Not decompiled.
- 0x144de7130 (ProfileAdd__144de7130): allocates a 0x30 entry via 0x142bfd200 and writes the callback record. Decompile in Helper4de7130__144de7130.c, partly read.
- 0x140690620 (MulticastRemove__140690620): per decompile, the callbacks call it to remove before re-adding (Helper690620__140690620.c). Its loop was partly read.

## Callbacks (decompile)
- HideToast__144e0c9c0 (HideToast__144e0c9c0.c): removes from +0x78, +0x250, and +0xd8 +0xe8 / +0xe8 +0xe8 via 0x140690620, then clears this+0x290.
- TeamPlayerCb__144e00850 (CbTeam250__144e00850.c): `0x140690620(param_2+0x250, param_1)` then `0x144e22b70(param_1)`.
- ProfileCb__144e07780 (CbProfileE8__144e07780.c): `0x140690620(param_4+0xe8, param_1)` then `0x144e22b70(param_1)`.

## Owner classes (UNKNOWN)
- No generated header declares a delegate at these offsets. Only 7 headers under porting/native/Source/FortniteGame/Generated contain "MulticastDelegate" (none is a team-member, registered-player, or profile class that owns these offsets).
- UFortTeamMember (porting/native/Source/FortniteGame/Generated/UFortTeamMember.h:14-22) declares only `SocialUser`. The subclasses UFortTeamMemberCampaign.h and UFortTeamMemberAthena.h are not checked for offset 0x78.
- Player +0x250 and profile +0xe8: owner UNKNOWN (see DELEGATES.md; no reflected property at these offsets).
