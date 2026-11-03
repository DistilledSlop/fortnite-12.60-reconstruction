# Aircraft rough native pass

Unintegrated C++-shaped control-flow lifts, not production ABI declarations or
runtime-validated implementations. No canonical manifest changes are proposed.
The existing manifest was checked for both native addresses; neither is listed.

* Existing `../aircraft_rough_runtime/Tick.cpp.in` (linked, not duplicated): 0x1418d7fb0, available in the prior aircraft rough draft, which
  already contains the substantial tick control flow. The simulated-proxy path
  selects and loads cosmetics, then the flight path handles authority timing,
  client drift, movement, drop-zone transitions and flight completion. The
  field layout and external interfaces remain illustrative. The assembly's
  initial COMISS/JA test allows NaN flight start times, unlike the C decompile.
* `CosmeticSelection.cpp.in`: 0x1419918d0, a typed lift of the state override,
  embedded controller interface and default-provider fallback. The interface's
  +0x18 virtual is deliberately called twice on the successful branch. Provider,
  interface, data and cosmetic types and ABI are unresolved; the source does not
  add null guards absent from the binary.

Source evidence: `porting/generated/callee_probe/gameplay_rough_pass_1/0x1418d7fb0.{c,asm}`,
`porting/generated/callee_probe/blocking/0x1419918d0.c`, and
`porting/behavior/aircraft_rough_runtime/README.md`. The cosmetic loader called
by Tick is still the separate split-unwind draft in that directory. No build,
execution or tests were performed. These files are rough evidence, not counted
as completed production bodies.
