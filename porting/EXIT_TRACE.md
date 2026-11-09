# Direct client exit trace (2026-11-09)

A fresh Wine server and the direct shipping executable were run with the local
backend, the in-memory TLS patch enabled, and the AppES diagnostic disabled.
GDB attached after frontend load and installed hardware breakpoints at the
unsafe-environment handler (`0x142121930`), exit callback (`0x142120970`), and
the confirmation-message function and timer registration (`0x1423b66d0` and
`0x1423b6948`). Live bytes at those locations were checked against the runtime
image before tracing. The trace hit the unsafe handler and the exit callback
after frontend load. The client log for this run was subsequently overwritten,
so login in this specific trace is not independently confirmed. It recorded no hit on either confirmation
breakpoint between attachment and exit. GDB detached with a thread bookkeeping
error during shutdown; this does not prove what scheduled the callback or
exclude a call before attachment.

Attaching during pak loading in an earlier attempt produced a pak index hash
mismatch and a fatal exit, so that attempt is not evidence of the original
post-login exit. The trace output is under
`generated/wine-investigation/20261109-exit-trace/` (ignored by Git).

A separate retry exposed a stale Wine server left in a previous network
namespace: the new client could not reach its local backend. The runner now
waits briefly for the prefix's existing server to exit and refuses to start if
it is still in use, rather than killing other Wine applications.

A follow-up run attached immediately after the FortniteBR content bundle
finished mounting and before the frontend `LoadMap` entry. With TLS patching
on and the AppES diagnostic off, it hit the unsafe-environment handler, logged
`LogAppES: closing code 1`, and hit the exit callback approximately 20 seconds
later. The confirmation-message function and its timer registration did not
hit between attachment and exit. This narrows the observation window but does
not exclude an earlier call, and static inspection found other delegate vtables
that reference the callback. The run reached authenticated account endpoints
before shutdown. Its trace, client log and redacted request metadata are under
`porting/generated/wine-investigation/20261109-early-exit-trace/` (ignored).

Another run captured the timer's delegate at the exit callback. Its vtable is
`0x1459a9ed8`, and its method pointer is `0x142120970`; the target object's
vtable `0x1461e7520` maps to `AFortPlayerControllerFrontEnd`. The timer data
contains a 20-second rate. A statically identified confirmation-message timer
uses a 15-second rate, so the delegate type and target alone do not prove that
path created this timer. The captured trace and client log are under
`porting/generated/wine-investigation/20261109-delegate-vtable/` (ignored).

A conditional breakpoint on timer setup subsequently caught registration of
this exact delegate at `0x1446278d0`, returning to `0x14211ed8c`; the timer
rate was 20.0 seconds. The callback later fired from the timer dispatcher. This
identifies the registration caller independently of the confirmation-message
path. The trace and client log are under
`porting/generated/wine-investigation/20261109-timer-register/` (ignored).
