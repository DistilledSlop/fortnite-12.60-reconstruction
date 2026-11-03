# Player-state message behavior, batch69

Four uncounted C++ control-flow lifts in `messages.rough.cpp`, from existing
improved AFortPlayerStateAthena decompilation; source paths and hashes are in
`sources.json`. These are not build inputs or canonical manifest bodies.

OnRep_ThankedBusDriver (exec142a8dea0) and OnRep_DidNotThankBusDriver
(exec142a8d9d0) share the world/game-state guard, optional local Athena-controller
name provider, team style selection, optional season-level suffix, named text
formatting and game-state virtual+a68 dispatch. They differ in localized source
and key. Neither checks the replicated thanked boolean. Native printf loses a
variadic integer argument in this decompilation: the draft deliberately keeps
that call unresolved rather than supplying an assumed level. Their default
FText is constructed before the argument map, unlike the reboot path.

OnRep_RebootCounter (exec142a8dc30) additionally returns for role byte3, creates
the map before the strings, inserts style before resolving the name, omits the
level suffix, formats RebootedPlayerName and dispatches the same virtual+a68.
Native temporary/reference-count and map destructor order still need closure.

Client_OnNewLevel_Implementation (native141a3d950) rejects nonpositive levels,
gets the name through virtual830, formats VictimStyle/PlayerName/Level, resolves
the configured global+24c8 template and only then obtains the world/game state
if the native text predicate is false. The final game-state read is not a cast.
It calls141a2d0f0 before dispatch. No null guards were added for global/service
providers that the original dereferences. Number-format options/culture, soft
reference resolution, intermediate FText lifetime and virtual signatures remain
explicit unresolved boundaries. UE FString/FText/named-map source types are
provisional interpretations, not proved ABI bindings.

Root inspected the complete meaningful algorithm windows and the repeated
container/reference-count mechanics, retaining calls/effects through unresolved
declarations. No original-byte or runtime correctness claim is made by hashing
the decompiler files. No tests, compiler, build or generation ran. Canonical
count stays2,074+180fields; last tested2,040, next tests3,040. Root retains sole
integration ownership. Keep bounded Terra6.1 medium/low fork-none delegation,
rotation and economical reads; preserve unrelated graphics edits.
