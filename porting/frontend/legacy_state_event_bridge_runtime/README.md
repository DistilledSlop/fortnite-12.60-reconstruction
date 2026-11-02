# Legacy state event and Athena override recovery

Unregistered generator policy and source fragments; no build or runtime claim.
The policy uses the type gate to emit the original enum, supplies the reflected
events, and supplies source aliases of both native virtual slots.
The enum descriptor 147f53560 names EFortUIState; its fourteen name/value pairs
start at 147019fd0. Function constructors 145316c40 and 145316c70 are linked
from class block 147013c40 via 147013b30/b40 to OnEnterState and OnExitState.
Their FFunctionParams records at 147f4fc20 and 147f4fd00 both have size 1,
flags 08020800 and no return. The parameter arrays point to the enum properties
at 147f4fb88 and 147f4fc68, respectively: PreviousUIState and NextUIState,
both using enum constructor 14531e170 and byte underlying type. The flags are
public, Event and BlueprintEvent, with no BlueprintCallable or native flag.
Enum editor metadata was not recovered; UHT integration must validate the
generator's enum rendering with these Blueprint events.

Widget vtable 146e386e0 slots 510/518 go through 145160ac0/145160ec0 to
1453181c0/145318200. Both wrappers resolve a UFunction by FName and call
ProcessEvent with a one-byte parameter buffer. The names are looked up through
distinct runtime FNames at 1483c1118/1483c1120 (captured comparison indices
000b8ce5/000b8cec). Initializer 14066a080 constructs the first from UTF-16
literal 147014758, `OnEnterState`; initializer 14066a0a0 constructs the second
from literal 147014778, `OnExitState`. Each passes find-or-add mode 1 to the
FName constructor 142cd4ab0. This independently proves slot 510 enters and
slot 518 exits. The root supplies the previous and next selectors respectively.

Athena slot 480, 14514ace0, has the following exact control flow:

1. Return unless selector is 3 and the default UFortRuntimeOptions from
   1426fdde0 has bForceApolloIntroSkipSubgameSelect true. Descriptor
   147b9df48 names this property and its set-bit routine 142acbdd0 writes cec.
2. Call root virtual 288, cast to FortPlayerController (class getter 142a3d9f0),
   return if cast fails, then load CommonCoreMcpProfile at 1af0 and return if
   null. Its descriptor 147b3bfe8 names the field and records offset 1af0.
3. Emit guarded diagnostic at verbosity 5, call 1421dfc50 on that pointer,
   and return if true. This predicate returns true if bEnableApolloIntro is
   false (descriptor 147b9df10, set-bit 142acb3b0 writes ced), or if the
   profile's boolean attribute `intro_game_played` is true, or if console bool
   `Eleven.Intro.Seen` is true. The attribute helper 1410fd930 leaves its out
   byte unchanged when missing or not a bool. Initializer 14030ab30 copies the
   literal at 1460d26e8 to the attribute FString at 148153340; initializer
   140309c20 registers the console bool at 1481523f0 with its named literal.
4. Emit another guarded diagnostic, obtain FortGameInstance through 144c10fc0
   (world game instance followed by cast); it is subsequently dereferenced
   without an explicit null guard. If subgame getter 141e5f160 is not 1, call
   141e7ec30 with 1 and then 140684de0 on the game instance member at 848.
   The setter does substantially more than a byte store; the latter function
   operates on a multicast-delegate-style invocation list and its exact member
   identity and semantic operation remain unresolved.
5. Call the configuration singleton 144db86e0 and virtual 260 for class 4;
   write that class to the output and finally write 4 to the selector.

The diagnostic literals are `APOLLO INTRO - Should we show the apollo intro?`
at 145f8cec0 and `APOLLO INTRO - skip the subgame selection` at 145f8cf20.
Both use LogFortUI, category storage 14838e2e0; its initializer is established
in the sibling ui_config_singleton_runtime/RECOVERY.md.
Required joins before implementing the override are a typed implementation of
the profile attribute query, the full setter 141e7ec30 and the member 848
operation. Do not introduce opaque stand-in fields,
skip those calls, or substitute a direct subgame byte assignment. The original
base slot is a genuine return-only function, 1406801c0, already represented in
root_legacy_state_runtime.
