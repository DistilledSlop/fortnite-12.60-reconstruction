# Complete jump, sprint refresh and parachute-open callers

The integrated bodies are PlayerPawn CheckJumpInput `0x14230d2f0`, Athena
CheckJumpInput `0x141a0e190`, and PlayerPawn OpenParachute `0x14233e6e0`.
Existing original captures and assembly are indexed in the retained
`pawn-jump-input145`, `jump-closure245` and parachute dependency evidence.
Independent review confirmed their complete algorithms and branch ordering.

Player CheckJumpInput refreshes local sprint state only outside saved-move
replay; it compares the current movement style and refreshes it before handling
jump/parachute input. This is inline original behavior, not a separate function.
The original second GetWorld call, unordered-rejecting cooldown test, and the
timestamp write after virtual Open or Close are preserved. Athena clamps its
remaining jump penalty with the original MAXSS behavior, then calls Player.

Open preserves feedback, game-state consumption, the Blueprint consume event,
movement mode transition, local-controller callback, action state, both virtual
owned-tag queries, gameplay event and tag-map update. The actual pawn interface
and authority-side faction lookup are integrated, including their lazy side
effects; ASC-only tags are not substituted.

Constructor-confirmed primary tables for Player, Athena, CharacterVehicle,
Ostrich and RemoteControlledPawnAthena were independently read at slots +838,
+1328 and +1330. Player uses `0x14230d2f0`; the four Athena descendants use
`0x141a0e190`. Every Open target is `0x14233e6e0`. CanDeploy is `0x14230bcb0`
except Ostrich, whose `0x1406801e0` constant-false override is restored too.
That shared constant address receives no new distinct-function credit.

These are complete algorithms under their original typed state preconditions.
Saved-move save/encode/combine/replay parity, controller timer producers, profile
backend installation/GC, ability initialization and whole movement-class behavior
remain product limitations. UE compilation and fresh walking/collision runtime
acceptance are pending. Source recovery does not establish that jitter is fixed.
