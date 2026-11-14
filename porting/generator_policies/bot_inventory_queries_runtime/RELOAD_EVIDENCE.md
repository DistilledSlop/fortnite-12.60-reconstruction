# Inventory reload notification and weapon predicates

The original inventory-owner interface slot `+0x78` takes the weapon actor.
Constructor-proven interface tables select these complete bodies:

| Provider | Interface table | Target |
| --- | --- | --- |
| Player controller | `14612a110` | `1406801c0` |
| Athena player controller | `145e1d478` | `1406801c0` |
| AI pawn | `1461088b8` | `1406801c0` |
| Deployable plot | `145f44288` | `1406801c0` |
| Athena AI bot controller | `145d43478` | `1416a6100` |

`1406801c0` is the exact empty void body, already counted elsewhere. The bot
body gets weapon data through `1426a0960`, dispatches the same interface's
infinite-ammo query at `+0x60`, and increments the actual optional telemetry
component's `TotalAmmoCheats` (`+0xc4`) with 32-bit wrap. Actor component storage
is the reflected field at `+0x900`; there is no auxiliary telemetry counter.
The preserved capture is
`local-model-package/captures/dc236dec63fb410b9e20fea3c3fc3432/0x1416a6100`.

Weapon predicate `1426a5af0` calls virtual stats `+0x6a8`, then virtual GetWorld
even for a null row, and checks World, row, and the existing exact charge test
`1426786a0` in that order. Capture:
`local-model-package/captures/2f45a58d05894c02b550f8834796803e/0x1426a5af0`.
Predicate `1426a5ab0` checks WeaponData, then its original trigger getter
`142671d20`, comparing with Automatic (`1`). Cached C:
`porting/generated/native-cap-helpers30/0x1426a5ab0.c`.

These are complete invoked algorithms under the original object preconditions.
They do not establish the full reload action, bot lifecycle, or runtime parity.
