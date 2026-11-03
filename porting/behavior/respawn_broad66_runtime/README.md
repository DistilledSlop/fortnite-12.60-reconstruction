# Respawn/controller broad recovery draft

Six inlined gameplay bodies recovered from the generated controller exec decompiles. `rough.cpp` is intentionally non-buildable typed pseudocode, not a source-complete candidate or manifest proposal. Unknown functions, class tests, fields, timer ownership, parameter layouts, and call effects retain U_/address labels; log and allocation/destruction mechanics are summarized. Timer interpretation follows the set-timer arguments but needs ABI verification. In particular the packed vtable d10 argument and camera transform helper require deeper recovery before promotion. No canonical changes or tests.

Original reflected exec addresses: RespawnTurnCameraBackOn 142a5a1d0; CheckIfSafeToTurnOnCamera 142a56500; CheckIfSafeToRespawn 142a564e0; StartRespawnLoading 142a5d5f0; CheckIfClientLoadedLTM 142a564a0; OnMinigameStarted 142a596c0. All take no reflected arguments; each advances the script frame before its inlined body.
