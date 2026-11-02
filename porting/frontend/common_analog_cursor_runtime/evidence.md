# Router-created analog cursor evidence

`1407d12a0` forwards to `1407896b0`, which allocates 0xd8 bytes, calls
`FAnalogCursor` constructor `142ffa9b0`, stores the raw router at +0x50 and
installs vtable `1459fc448`. It then initializes the state at +0x58 through
+0xd6, and returns a shared pointer. In particular +0xd4 and +0xd5 start at
zero, +0xd6 at one. +0xb8 and +0xc4 start as float 1.0.

Vtable offsets relative to `1459fc448`:

| Offset | Address | Observed role |
| --- | --- | --- |
| 08 | 1407fef70 | Tick, including cursor positioning and base Tick branch |
| 10 | 1407c99c0 | Key down |
| 18 | 1407c9b20 | Key up |
| 20 | 1407c6690 | Analog input |
| 28 | 1406801e0 | Mouse move returns false |
| 30 | 1407c9ef0 | Mouse button down |
| 38 | 1407ca320 | Mouse button up |
| 58 | 1407c4500 | Owner user index; finds router local player in game instance's local player array |
| 68 | 1407cff60 | Event eligibility |
| 70 | 1407cffa0 | Key eligibility |

Both key handlers gate on virtual +0x70. The special virtual-accept key is
routed through router `1407e3b40` with pressed/repeat or released; return value
zero consumes it. Otherwise key down temporarily sets input-subsystem state
through `14081d0e0`, calls the base key-down handler and clears that state;
key up calls the base handler. Key down has an additional local-player
viewport-console `ConsoleActive()` gate before routing. The analog handler calls base analog input
only when virtual +0x68 succeeds, and returns its result only when +0xd5 is
set. The key eligibility path allows a special exception when cursor eligibility
fails, conditional on router configuration bytes +0x81 and +0x83.

Mouse down/up do not forward to base. They check base relevance, examine the
effecting button, exclude left and right mouse buttons, and use a +0x58 key container, synthesize key events and
dispatch them through Slate. Mouse up returns the synthetic dispatch result
only for a key present in the container; other eligible synthetic releases
are dispatched but return false. Down requires the viewport widget in the
widget path under the pointer; untracked releases additionally require that
the Slate user does not hold mouse capture. The excluded keys are proven by
registrations pairing LeftMouseButton literal `142ef4f51` with global
`1482642c8`, and RightMouseButton `142ef50b9` with `1482642e0`.

After shared construction, `1407cd650` copies the analog cursor settings,
copies the current Slate pressed mouse buttons and removes those two keys,
binds the input subsystem's native method-changed delegate and applies the
current input method. Callback `1407c9450` stores the entire input-method
byte at +0xd4 and clears weak cursor target +0xa8 only for Gamepad (value 1).
The byte is not a boolean: Touch (2) is distinct from both other methods.
The subsystem broadcast `14081adb0` invokes native then dynamic delegate and
updates the input-change timestamp when the world permits it. Its producer
at `14081c970` includes availability, thrash and lock/override logic and is
not present in the existing native source.

Settings defaults are written by standalone struct constructor `140813b50`
and repeated inline by `UCommonUIInputSettings` class constructor `140813610`.
The struct contains priority 2, acceleration enabled, acceleration 1500,
maximum speed 2200, dead zone 0.25, hover slowdown 0.4, scroll dead zone 0.2,
scroll period 0.1 and scroll multiplier 2.5. The class constructor also sets
link-cursor-to-gamepad-focus true and UI action processing priority 10000.

Tick starts by calling `1407ec060`, which updates the Slate user's cursor
visibility from +0xd4, +0xd5 and router policy `1407fb160`. Tick proceeds only
if +0xd4 equals one and `1407cf210` succeeds. When +0xd5 is set it invokes
the base analog Tick; otherwise it follows a separate focus/hit-test/cursor
positioning path. The full positioning path and `1407cf210` must be recovered
before replacing this class with source. Merely calling base Tick changes
behavior and is not a faithful implementation.
