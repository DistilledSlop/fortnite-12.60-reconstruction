# Aircraft dependency investigation: mode time-of-day override

No implementation or manifest entry is proposed: the dependencies below are
not yet source-closed. This investigation is useful for the mode's larger
helper graph, but the setter is not itself aircraft-specific.

`AFortGameModeAthena::SetTimeOfDayManagerGameplayOverride` exec
`0x1428f08f0` passes the reflected `TimeOfDayManagerClass` argument to native
`0x1419642e0`. Its complete original decompile and assembly are in
`porting/generated/callee_probe/blocking/0x1419642e0.{c,asm}`. The mode's
`+0x278` GameState is checked against `AFortGameStateAthena::StaticClass`
`0x14290f690`; `0x14199e430` rejects the operation in playground or creative
and logs the original message at verbosity 3 if enabled. It short circuits
before changing the override or looking up WorldSettings.

Otherwise, each of the existing mode `+0x1370` value and the argument is
individually checked as a UClass child of `AFortTimeOfDayManager::StaticClass`
`0x142b057f0`; the checked values (null on failure) are compared. On inequality
it stores the *original argument*, gets the world by virtual `+0x150`, calls
`0x144690740(world, false, true)`, casts its result to `AFortWorldSettings`
(`0x142b5dca0`), and calls `0x141fd72d0` if the cast succeeds. Do not compare
raw incoming pointers or change the order of the store and callbacks. The
reflected field is `TimeOfDayManagerGameplayOverride`.

The new bounded probes in `porting/generated/callee_probe/aircraft_larger_recovery{,2}/`
show:

* `0x14199e430` fetches the active playlist (override first, then base), calls
  `0x141a49590`, and only on false reloads the active playlist and calls
  `0x141a4b0d0`. Each predicate returns false for null playlist, obtains the
  object from `0x141ed1800`, reads FNames at `+0x1748` and `+0x1768`
  respectively, rejects None, and searches both playlist FName arrays at
  `+0xd0` and `+0xe0`. The provider, field names and array types require
  further identification; do not cache the active playlist across calls.
* `0x141fd72d0` uses the WorldSettings manager at `+0x800`. If null, it calls
  `0x141fbd1f0` and discards the return. Otherwise it destroys the existing
  actor with `(false, true)`, calls `0x141fbd1f0`, and stores the return at
  `+0x800`. This asymmetry and reentry ordering are observable.
* `0x141fbd1f0` is a substantial spawn/selection body (273 lines of decompile,
  with authority, world state and class/actor iteration). Its complete source
  binding and return ABI require more work before this call can be represented.
  `0x144690740` also has world and level selection semantics beyond a generic
  guessed getter; its decompile is `porting/generated/callee_probe/extra/0x144690740.c`.

Additional dispatch evidence: `0x1419331a0`, called by the exec for
`AddAIClassToReplicationGraph`, dispatches the process-global native delegate
at `0x1478c2670`. It increments its reentry counter at `+0x14`, captures the
initial last index and iterates backwards while reloading the live allocation.
Each 16-byte entry requires a nonzero word at `+8` and a nonnull delegate
instance; it calls instance virtual `+0x50` with the new class. A failed call
or invalid entry requests cleanup. After decrement, cleanup runs only if the
counter is <= 0: entries invalid by that test or instance virtual `+0x20`
are removed with `0x140690780`; count is re-read after each removal. It then
sets the threshold at `+0x10` to max(2, 2*count) and calls `0x1406911b0` when
that is below the pre-cleanup count and capacity differs from count. The
registration owner and concrete native delegate type are still unidentified;
replacing it with a fresh delegate would lose registrations. Assembly:
`porting/generated/callee_probe/blocking/0x1419331a0.asm`.

All addresses are from the original late-capture image at
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
(base `0x140000000`). These are static observations, not runtime validation.
No build or tests were run.
