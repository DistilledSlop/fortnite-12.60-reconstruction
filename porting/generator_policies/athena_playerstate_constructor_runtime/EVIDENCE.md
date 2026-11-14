# Athena player-state constructor projection

Original constructor `0x141a2e060` calls parent `0x1423fa740` at `0x141a2e0c0`, then runs a straight-line body ending at `0x141a2e7d2`. Evidence is the decrypted runtime image, disassembled using Capstone, joined with `generated/reflection/fortnite_game_reflection.json` and `bool_bits.json`. This is a projection of directly verified reflected writes, not an implementation of all original constructor behavior. Offsets are original binary offsets, not generated C++ offsets.

| Store | Offset | Reflected assignment |
| --- | --- | --- |
| `0x141a2e24c` | e78 | SeasonLevelUIDisplay = 0 (dword from EBX) |
| `0x141a2e5d3` | 1288 | SecondsAlive = 0, TimeOfPawnCreation = 0 (qword from RBX) |
| `0x141a2e6b3` | 1388 | bResurrectionChipDisabled = false (byte from BL) |
| `0x141a2e6df` | 222 | bShouldUpdateReplicatedPing = false; AND byte with 0xfe clears **only mask 0x01**, preserving masks 0x02, 0x04, 0x08, 0x20, 0x40 and remaining bits |
| `0x141a2e6f5` | 13a4 | bResurrectingNow = false (byte from BL) |
| `0x141a2e6fc` | 13a8 | RebootCounter = 0 (dword from EBX) |
| `0x141a2e703` | 13ac | bHoldsRebootVanLock = false (byte from BL) |
| `0x141a2e72d` | 13e0 | bPreserveSquad = false (byte from BL) |
| `0x141a2e74d` | 668 | inherited bSkipReplicatedStats = true (whole byte 1; reflected native bool) |
| `0x141a2e755` | 108 | inherited NetUpdateFrequency = 1.0f (0x3f800000) |
| `0x141a2e779` | 13b6 | bIsAnAthenaGameParticipant = true; OR byte with 0x01 preserves all other bits |
| `0x141a2e79c` | d2c | PersonalLobbyAction = 0 (dword from EBX) |
| `0x141a2e7aa` | 13b4 | MatchAbandonState = underlying byte 0 |
| `0x141a2e7c1` | e68 | TeamScore = -1 (dword 0xffffffff) |

`xor ebx, ebx` at `0x141a2e0d9` establishes all the EBX/RBX/BL zero stores above; Windows x64 calls preserve RBX. The inherited private ping bit is set through the engine's public `SetShouldUpdateReplicatedPing(false)` accessor, whose implementation performs the corresponding typed bitfield assignment. Other direct writes touch arrays, delegates, structs, pointers, vtables, or unknown native locations; no unsupported identities are inferred. A component byte store and virtual call at `0x141a2e760..0x141a2e773` are not projected. As with any constructor projection, calls and native subobject side effects are not reproduced here.

Reflection locates TeamIndex at `0xe60`, SquadId at `0xfdc`. No direct constructor store overlaps either byte, including a zero store. This finding does not establish their values after base calls, initialization, or later team assignment. No values are assigned to them by this policy. Values inherited from allocation, templates, or other code require separate evidence. The class-local enum, integer and boolean direct stores listed above are the reflected ones identified by inspecting the complete body, including zero stores omitted by the older lossy default extraction.

The generated class ancestry is `AFortPlayerStateAthena : AFortPlayerStateZone : AFortPlayerState : APlayerState`; the policy checks the immediate parent and the generated code checks inherited scalar types at compile time. The policy preserves an existing compatible constructor and initializer list, rejects competing uses of projected fields, validates own declarations and inherited scalar types, and creates a constructor only when none exists. It does not modify generation integration or authored templates.
