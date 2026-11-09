# Direct client exit trace (2026-11-09)

A fresh Wine server and the direct shipping executable were run with the local
backend, the in-memory TLS patch enabled, and the AppES diagnostic disabled.
GDB attached after frontend load and installed hardware breakpoints at the
unsafe-environment handler (`0x142121930`), exit callback (`0x142120970`), and
the confirmation-message function and timer registration (`0x1423b66d0` and
`0x1423b6948`). Live bytes at those locations were checked against the runtime
image before tracing. The client logged in, then the trace hit the unsafe
handler and the exit callback. It recorded no hit on either confirmation
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
