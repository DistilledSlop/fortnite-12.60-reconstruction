# Aircraft initial source pass (unintegrated)

`Tick.cpp.in` is a control-flow lift of `0x1418d7fb0`; `Cosmetics.cpp.in`
retains the complete split-unwind `0x1418b99f0` C as a C++-shaped direct lift,
including the loaded-asset cast, weak pointer resolution and serial-number
updates, pending-handle cancellation, load arrays and delegate allocation,
shared-pointer transfer/release, synchronous spawning, attachment and cosmetic
application. It retains raw Ghidra operations where ABI or type recovery is
unfinished. Neither file is a compilable production body, tested, promoted, or
counted as implementation. No independent complete body was identified.

Integration dependencies:

* Reconcile illustrative tick fields with true reflected/native storage, use
  actual embedded multicast delegate objects and bind them through their real
  types; preserve mutation and reentry ordering. `+0x2f0` and `+0x2f1` are
  separate bytes. See `../aircraft_tick_runtime/README.md` and
  `../aircraft_dropzone_runtime/README.md` for mapped contracts.
* Recover `0x1419918d0` default cosmetic lookup, player-interface and fallback
  providers; `+0x5f0` cosmetic virtual; event signatures and registration;
  mutable `0x1478c2360` and `0x1478c2400` globals and their lifetime.
* Resolve cosmetic loader `0x141df62a0` + virtual `0x340`, completion callback
  `0x1418bc2f0`, pending-handle cancellation operations, weak/soft reference
  internals, object-class checks, and shared-pointer ownership and ABI. The
  stack argument `in_stack_ffffffffffffff08` is unresolved: recover its caller
  contract rather than substituting a value. Name/declare externals with actual
  signatures and reconstruct Unreal types instead of copying raw casts.
* Establish exact spawn parameter and attachment APIs and constants; preserve
  both the requested and fallback cosmetic, any in-flight request, and all
  cleanup/refcount paths. The direct lift has no invented fallback or no-op.

The decompiled C's initial nonnegative test rejects NaN, while the targeted
assembly at `0x1418d80db` uses `COMISS zero,start; JA return`, so the tick draft
preserves the assembly's NaN pass. The authority interval/end comparisons are
strict and reject unordered values as documented in the existing investigation.
Review float behavior and the calling convention again during integration.

Inputs: `porting/generated/callee_probe/gameplay_rough_pass_1/0x1418d7fb0.c`,
`0x1418b99f0.c` and the targeted `0x1418d7fb0.asm` window. Commands: read both
C files and the two existing aircraft READMEs; `rg -n -C 3 '1418d80c6|1418d8272|1418d80d' .../0x1418d7fb0.asm`; generated the cosmetic
transliteration with a short Python address-renaming script; wrote the tick
and this dependency map. No build or test was run for the draft.
