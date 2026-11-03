# Native 74 controller runtime lift

`rough.cpp` connects both complete decompiler bodies: `0x1419f5920` SetupClientRespawnTimerAndLocation and `0x1419fb550` StartCreativeMatchmakingReadyCheck. It is a behavioral reconstruction, not canonical engine source or a drop-in ABI. All external functions are unresolved adapters and require real implementations; none are defined as empty effects. No compiler, tests, generator, promotion, or Ghidra was run.

Respawn retains world/class gating, the player-state reread, 12-byte location/rotation handling, both distance checks and the playlist reread, direct callback, and the timer callback/delegate allocation, clone, auxiliary cleanup, destruction, reallocation and free sequence. Three identical timer blocks are factored into `schedule_respawn`; the temporary delegate's internal representation and hidden ABI are necessarily compressed into named adapters. Stack-cookie checking is compiler/runtime scaffolding and not represented. The value copied from stack `uStack_11c` into the delegate was not initialized in the decompiled body: it is explicitly supplied by `unresolved_stack_11c`, not invented as zero. The output of `141936680` is carried as raw 32-bit bits: `1419f5a53` initializes the full dword at `[rsp+0x40]`, `1419f5a72` reads it with `MOVSS`, and `1419f5acd` passes those bits through XMM3; adapter signatures and padding of `VecBits` need ABI reconciliation. Only the clone pointer and count are initialized before cloning; the timer temporary adapter must preserve the cloned delegate while initializing only the separate observed weak/auxiliary fields and expose the actual inline storage address. Location XY is captured before rotation writes; location Z is captured after them and after rotation XY is read, preserving the observed order for aliased inputs. Native virtual calls and allocator semantics must be bound faithfully.

Ready-check preserves the party-leader gate, log thresholds, ready path and matchmaking attempt, fallback party update, state reset, class gate and duplicate delegate check. The first `14237f3b0` call has unrecovered argument setup and is deliberately a distinct adapter; the second explicitly passes the controller. `14167fb70` has an additional hidden temporary argument, represented by its adapter. Ancestry comparisons are represented by `is_a_raw`, with the +0x10/+0x30/+0x38 checks required of the adapter. Playlist offsets +0x5c9, +0x5cc and +0x640 were checked against the assembly. Static-class adapters must return the actual class objects; helper signatures and implicit argument registers remain unresolved. The native stack-cookie epilog is omitted.

Target addresses: `0x1419f5920`, `0x1419fb550`. Callback targets: `0x1419ebd40`, `0x1419e2cc0`.

Source inventory (SHA-256):

```
0e358656909457f5b26c2b9a50a6ebf50208b591603590bf55dabeb5393b21f2  porting/generated/native-broad74/0x1419f5920.c
9e61afe41af3c31f8d9025146c3cb2f4d7c35f9b7b5e218fe61af606c4485266  porting/generated/native-broad74/0x1419f5920.json
2e88359baa7c583b20b28b416ddde67fec3a23df33967c71a5e445d8617b05c7  porting/generated/native-broad74/0x1419f5920.asm
6d2dcda8abd709df4335470207f4f35e1c6dc854de052d90494505f94c8e4e4a  porting/generated/native-broad74/0x1419fb550.c
d02f84242d7efcadbde95f7cca014431eb85eebbfcbf024517ba953d32776388  porting/generated/native-broad74/0x1419fb550.json
71ed623efc25db026b594473d3b2be7e2708359019708d9e9e2d1f8c5bfd5455  porting/generated/native-broad74/0x1419fb550.asm
```
