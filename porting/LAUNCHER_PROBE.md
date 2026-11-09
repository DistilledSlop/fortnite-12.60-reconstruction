# Supplied launcher probe (2026-11-09)

A Wine run used the supplied `FortniteLauncher.exe`, the local TLS backend, a
copy of the DXVK software-rendering prefix, and `-nobe -noeac`. The launcher
started the shipping client with a launcher token and also started the BattlEye
wrapper. The TLS verification patch was enabled; the AppES diagnostic patch
was disabled. No executable was modified on disk.

The client signed in to the local backend and reached subgame selection. It
then logged `LogAppES: Display: closing code 3` and requested exit. In this run,
frontend load was logged at 14:27:58 UTC, closing code 3 at 14:28:24, and
`RequestExit(0)` at 14:28:51. The launcher and its token therefore did not
establish a persistent session in this environment. This observation does not
identify the underlying cause of the AppES result or prove why the later exit
was requested.

The first attempt also exposed that the runtime patchers only recognized a
bare executable name, while this launcher starts the shipping client using a
Windows path. They now identify the exact shipping executable within their own
network namespace. With that fix, the TLS patch attached and the second run
successfully signed in. The local screenshot and request evidence are under
`generated/wine-investigation/20261109-launcher/` (ignored by Git).

Static inspection of the unsafe-environment handler shows reason values 2, 3,
and 4 select the shared `UnsafeEnvironment_Body` message, whose English source
warns about a cheat or reverse-engineering tool. If the launcher's logged code 3
is the same value passed to that handler, it would select that message. The
launcher run did not capture the handler argument or dialog; this mapping does
not establish what the client detected or why it closed.
