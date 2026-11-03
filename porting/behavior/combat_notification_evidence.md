# Combat notification RPC investigation

The two RPC implementations remain skipped. Evidence is authoritative raw image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (VA minus `0x140000000` is file offset; image SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`). Disassembly windows are `/tmp/combat-141a3d6c0.asm`, `/tmp/combat-141a3d7c0.asm` and `/tmp/combat-1419eded0.asm`. Generated improved decompilation omits the last call's arguments; the raw instructions below govern.

* Both check Owner at +0xe0 is an AFortPlayerControllerAthena. DBNO calls controller helper `0x1419eded0` with true, obtains controller +0x31e0 through `0x1419e4e30`, and calls `0x1406ba850` with that address and Player. Kill calls `0x14241bc10` with this's virtual +0x150 result and Player; only on false does it call the same controller helper with false and pass Player to `0x1406ba850` on controller +0x31c8 (accessor `0x1419e4e60`). The two offsets are distinct from the prior proven +0x31f8 team-kill delegate.
* The common helper is a material blocker, not an ignorable notification: it obtains a world through controller virtual +0x150, checks a timer-related path and controller +0x2c98/+0x2c9c, may store world +0x540 at +0x2c94, increments +0x2c9c for DBNO or +0x2c98 for Kill, and builds callback/timer state. Its complete behavior and the callback are not yet recovered. Substituting only the delegate broadcast would lose state and scheduling effects.
* Kill also reads the default runtime options byte +0x122 and, if nonzero, calls `0x1423c0800` (a jump thunk) on the controller with two zero arguments. The target and effect are unclosed. Predicate `0x14241bc10` is not a simple null/equality check: it resolves an object from a world/context, casts it, invokes virtual getters on components +0x590 and Player +0x318, and compares their byte returns. Resolve its exact typed contracts before implementing the guard.
* After those paths both invoke virtual +0x28 on the controller subobject +0x598. If the result and its +0x6d8 member exist, `0x14269e270` constructs a 0x1a-byte value from fields +0x8c0/+0x908 and an optional +0x350 object's virtual +0x2a8; otherwise a zero value is used. If Player exists, `0x1419b6d20` reads Player +0x278 and casts to the class returned by `0x142a75f10`. Both pass the result, Player and value to `0x1406801c0`, whose raw bytes begin `c2 00 00` (return with zero stack adjustment): the final call is empty in this capture. The virtual calls before it can have effects and cannot be assumed dead at source level. The shown RPC bodies contain no FText construction or destruction; do not infer text lifetime behavior from the improved decompiler's missing arguments.

Raw image SHA-256 window hashes (length includes body through `ret`, excluding following unwind fragments for the RPCs):

| VA | length | SHA-256 |
|---|---:|---|
| 0x141a3d6c0 | 0xf3 | 39179a0775f0410c4413c7dac379f7cb500cc26bfd10c0a82e93671c6438ff78 |
| 0x141a3d7c0 | 0x12c | fed08470a6f8bb7d79c642deae53df619abdda0eadfda742551884bd1c96ed3c |
| 0x1419eded0 | 0x250 (window) | f7b67c3bf6d24721fa72a91610dac5258268bd1073487407240ed3ea5f017d2f |
| 0x1419e4e30 | 8 | 85133185923231c57d1310c5344cae816e162b69ebbe39b75c08f6ae44e8964f |
| 0x1419e4e60 | 8 | bdae347141bd672b23611315e298d35930d653498aa973f353bc2a803ec8404c |
| 0x14269e270 | 0x6e | 5e92923b0de7102c69dd62bed4d10c8255f425cf65c3384bfedf684202b22393 |
| 0x1406801c0 | 0x10 (includes padding) | 28f844b5972c241e804231b0e41b93e13696e59b98dde74f19029386db5da0c5 |

No source snippets or manifest changes are proposed until these dependencies are closed; `/tmp/gameplay-combat-notification-entries.json` is an empty list. No runtime or build validation was performed.
