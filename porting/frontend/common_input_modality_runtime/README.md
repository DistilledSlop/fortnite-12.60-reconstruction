# CommonInput modality runtime candidate

Source fragments recover input-method producers 0x14081c970 and 0x14081cc60,
lock updater 0x14081aa90, broadcaster 0x14081adb0, initialization
0x14081bbe0, deinitialization 0x14081b1d0 and preprocessor-order ticker
0x14081d270. They need generator integration and a UE build; no runtime
validation has been performed. The reflected SetCurrentInputType and
SetGamepadInputType execs (0x14081f060 and 0x14081f0e0) invoke these producers.
The generator should use policy.py to validate the pinned reflection and bounded
original code windows, apply setter metadata, configure C++ includes and forward
declarations, append the declaration and definition fragments, and copy both
preprocessor headers to the CommonInput include path. NativeInputMethodChanged is
already supplied by common_analog_cursor_runtime; avoid duplicating it or its
older broadcaster definition. Settings::InitializeNativeInputDataCache must be
public for the original direct initialization call.

The producer uses the cached platform, rejects unsupported input, tracks
thrashing with the InputMethodThrashing lock, and refuses a new last type
while locks exist. Lock updates choose the largest enum byte; they compare the
resulting current type with LastInputType, not its previous current value.
Gamepad changes broadcast even if the value is unchanged. The broadcaster skips
null or tearing-down worlds, broadcasts native then reflected delegates, then
sets the timestamp. World flag identity is supported independently: original
0x144687460 ORs mask 0x20 into world+0x11d, then logs the exact text
`BeginTearingDown for %s` at 0x146b562d0 and broadcasts two delegates; the
matching UWorld::BeginTearingDown implementation in Engine/Private/World.cpp
sets bIsTearingDown before that log and those two broadcasts.

The actual input preprocessor vtable at 0x145a0dc38 identifies key down
0x14081b8d0, analog 0x14081b7e0, mouse move 0x14081bb00, and mouse down and
double-click 0x14081ba60. Key up, mouse up, wheel and motion are the shared
false-return implementation; tick 0x14081d230 is a no-op apart from shared
pointer destruction. Events require an active Slate application and the
local player's controller ID. Key down and pointer events choose and submit
the input type unless blocked; analog input only returns the block state.
Mouse movement ignores one marked event and otherwise requires either absolute
component to exceed 0.0001. Initialization installs the processor at index 0,
registers a 0.1 second order-maintenance ticker and the existing ShowKeys CVar
callback; deinitialization unregisters the processor and ticker. The mouse
movement and ticker constants were read from 0x1459c03a4 and 0x1459a86e8.

The four original Android-key candidate globals are 0x148265858, 0x1482658a0,
0x148265888 and 0x148265870. Their captured FName indices are 422657, 422685,
422674 and 422664 respectively; their literal names remain independently
unverified. The processor files adapt the earlier frozen research at
/tmp/common-input-native-dependencies-1/frozen-input-lifecycle-stage-2. Its
filter method copies each map before updating and reducing it, matching the
copy-and-reduce shape of 0x14081ccc0. Event-to-key mapping uses the four
Android system keys in that candidate; their original global key identities
should receive independent verification. The processor's IgnoreNextMove hook
is present; the original 0x14081cbc0 marks it after a separate operation, whose
external callers are not wired here. Those are remaining limitations.
