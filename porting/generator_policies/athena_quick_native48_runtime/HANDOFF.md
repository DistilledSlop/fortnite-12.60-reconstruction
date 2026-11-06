# Athena quick native 48

Two high-confidence reflected implementations are ready for review in `manifest-entries.json`. No canonical files, build, runtime, or tests were changed or run. `native-evidence.json` contains the original image hash and complete raw windows for all five execs, their native bodies, and the directly investigated helpers. Windows follow pdata spans or the full disassembled body; 0x14070fe00's window includes the cold continuation and intervening bytes.

`GetPlaylistEnableBots` uses reflected `PlaylistEnableBots` at original +0xbe0. Its level-zero scalable-float value must be strictly greater than zero (including the original behavior for negative values and NaN).

`GetAthenaPlaylistContextTags` copies `UFortPlaylist.GameplayTagContainer` (+0xd0) from the current playlist. Reflection identifies `MaxTeamSize` (+0x5c), and `bool_bits.json` identifies `bIgnoreDefaultQuests` (+0x552, native byte). The latter removes all five team size tags in order, with parent updates after each successful removal; otherwise team sizes 1, 2, 3, 4, and greater than 4 add respective Solo, Duo, Trios, Squad, and LargeTeam tags, with other values adding nothing. Native 0x143e96610 names itself `STAT_FGameplayTagContainer_RemoveTag`, and 0x143e8b3b0 is the previously identified AddTag. The already integrated singleton supplies those tags and preserves native registration. Native reads playlist pointer +0x1df8 first, then +0x1df0; integration must ensure canonical `GetCurrentPlaylistAthena()` retains this precedence and returns the same object.

Three targets remain blocked rather than substituting guessed behavior:

* `AreSkyTubesEnabled`: 0x141989c50 checks reflected `bSkyTubesDisabled` (+0x11f3), then 0x1426f5f90 reads global int 0x1478df4c8. Its owner, initialization and runtime mutation have not been identified.
* `CanUseSlowMotionOnVictory`: 0x14198bce0 first consults complex predicate 0x14199e4e0 and, if true, compares reflected `GamePhase` (+0x1cb0) to 5. Otherwise it iterates mutators and dispatches virtual slot +0x920, returning the first handled result or true; iterator and virtual contracts are not closed.
* `GetEndGameReturnToMatchmakingOriginDelay`: 0x1419920d0 forwards the preferred current playlist to 0x141a42830. That helper performs an unresolved global provider/predicate and checks reflected `bAllowReturnToMatchmakingOriginOnMatchEnd` (+0x464), returning global float 0x1478c29b8 when permitted and zero otherwise. The predicate and global's owner/lifecycle must be recovered.

Existing full probe C/assembly: `porting/generated/callee_probe/blocking` for the five targets, `porting/generated/callee_probe/extra` for 0x14070fe00 and 0x141a42830, and `/tmp/athena-quick48/probe` for 0x143e96610 and 0x1426f5f90. Reflection function flags and parameter records come from `porting/generated/reflection/fortnite_game_reflection.json` mapped via `native_class_index.json`.
