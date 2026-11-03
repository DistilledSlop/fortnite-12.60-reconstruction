# Pawn movement-state control flow

MovementState.draft.cpp lifts the complete observed SetInVortex branch structure
and HandleInStasis transitions. It remains incomplete and uncounted: U_* accesses
and callbacks are explicit unresolved effects, not implementation stubs. The
Vortex loop/reentry timing, movement-mode exclusions, absolute width bit mask,
ordered terminal/launch clamps (including NaN behavior), and flag-update order
are retained. Stasis restores the previous guard byte after reverse native
multicast dispatch, and preserves previous-mode cleanup before new-mode effects.

Evidence: existing generated/decomp/AFortPlayerPawn/HandleInStasis.c and
/tmp/pawn-state68/0x14234d650.{c,asm,json}. The original AFortPlayerPawn virtual
slot +0x1340 at vtable146116ac8 identifies the vortex target. Constructor wrapper
142a75370 forwards to1422fe830; no constructor recovery or initialization claim
is made. The separate thrown-trajectory query/write bodies show a HitResult
at+272c, but its original construction has not been integrated. No tests/builds.
