# AFortGameStateAthena replication handlers

The three manifest candidates are parameterless multicast broadcasts. Native bodies at 0x1419a7440, 0x1419a8250, 0x1419a8260 add the corresponding reflected delegate offset, zero the argument pointer and tail-call multicast dispatcher 0x1406a9dc0. The decompiled wrapper inlines dispatcher iteration, invalid binding pruning and ProcessEvent; `.Broadcast()` expresses those semantics. Pin windows cover the three execs and native bodies. No canonical manifest or builds changed.

The three incomplete, uncounted drafts retain explicit unresolved calls and pseudo types:

- `OnRep_Aircraft.draft.cpp`: placement iteration and team-index handling; uses existing `AFortGameStateAthena/GetAircraft.inc`. Iterator, virtual check, and several call identities/arguments remain unresolved.
- `OnRep_CurrentPlaylistInfo.draft.cpp`: listener updates, analytics assignment, playlist-derived state and final notification. Object and helper identities remain unresolved; the observed null dereference is preserved.
- `OnRep_PlaylistEndTime.draft.cpp`: timer and countdown delegates. Timer callback and precise first-delay rounding remain unresolved.

These are behavioral sketches, not canonical source or manifest candidates. No dependencies were expanded.
