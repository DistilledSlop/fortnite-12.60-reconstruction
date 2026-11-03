# Aircraft and playlist state evidence

Authoritative original: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`; read at VA minus `0x140000000`. All windows below match the older `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` byte for byte. The hashes are of the stated windows, not necessarily entire functions.

| VA | Window | SHA-256 |
|---|---:|---|
| 0x14291e920 | 0x60 | c03c29fac312680c73af742a1be76688d3e2d25c7ddc2d4bbeb2795674a3d056 |
| 0x141995550 | 0x6f | 3d6813d88697cbbb47436ef1dea2eab159edfa5e67d051c6c2135629734021b4 |
| 0x141990d90 | 0x127 | 37b70d46b05badeb3490d61296cea15b398d5adb497ee8c29d5aa7fce32b1702 |
| 0x141990ec0 | 0x130 | 59070e2fb5bf837ce7680616d03383bb12e4e90f1a5e598083438421ce6148f2 |
| 0x1419918d0 | 0x90 | b39f013dfae59457e72e737bc41fcd84a79ce7c516f01b5d8d262fa93d832291 |
| 0x1419520f0 | 0xa0 | 185bf504644534a027f74016fd4eb09fb585dc48339961ae2b6387e61e2b57db |
| 0x1419adfe0 | 0x150 | 5aa490d1ac61e5ee2c742c7fc2d4caee3f8646033d0236e71e149f4827049ea6 |

`GetMaxPlaylistPlayers` exec calls 0x141995550. That body calls the object's virtual `GetWorld`, reads the world's game state, checks its type against AFortGameStateAthena, selects its override playlist or base playlist, and returns playlist +0x44; it returns -1 if any required object is absent or of the wrong type. Reflection identifies +0x44 as UFortPlaylist.MaxPlayers; existing `GetCurrentPlaylistAthena` implements playlist selection. It is significant that the query uses the world's game state rather than directly reading `this`. This query is now in the canonical manifest; scratch generation and generated game-state translation-unit syntax compilation pass (see HANDOFF.md). Shared Native promotion and runtime validation remain pending.

`GetAircraft` exec 0x14291e730 calls 0x141990d90. In phase values 1 and 2 it returns the indexed Aircrafts element if the signed index is in bounds, otherwise null. Other phases additionally log invalid indices at verbosity 6, and when no aircraft is found invoke virtual GetWorld then 0x144693a30 before conditionally logging at verbosity 3. The category and predicate are now closed in `aircraft_lookup_evidence.md`: LogFort and UWorld::IsPlayingReplay. The full canonical body preserves both log paths and replay suppression; scratch generation and syntax compilation pass, with shared Native promotion/runtime validation pending. See `porting/generated/callee_probe/blocking/0x141990d90.{c,asm}`.

`GetBattleBusForPlayer` exec 0x14291e820 calls 0x1419918d0, not 0x141990ec0. The body takes the default battle bus at +0x13a8, optionally consults a player interface at +0x5a8 when +0x13b0 permits user choice, and if no bus is resolved calls 0x141d46180 and reads its +0x218. Interface method identities and default provider closure remain unresolved. See `porting/generated/callee_probe/blocking/0x1419918d0.c`.

0x141990ec0 instead computes an aircraft index for a player state: it checks the player class, obtains an interface value at +0x318, selects playlist, examines game mode at +0x14a0 and subtracts playlist +0xc8, then checks index against phase and aircraft count. Its invalid paths emit logs. Exact interface, playlist field, category and full logging contracts remain unresolved. No approximation is implemented.

`OnAircraftEnteredDropZone` 0x1419520f0 iterates `Aircrafts` via 0x141990d90, checking each non-null aircraft's byte +0x2f0, then calls 0x1419adfe0 with false only when all are ready. 0x1419adfe0 is substantive: when +0x1ef0 bit 1 is set and the argument is false it writes a derived value at +0x1418, obtains a world iterator through 0x14468fb70 and processes its entries before further state changes. The named aircraft flag and complete setter/dependency behavior remain unresolved. No drop-zone body is proposed.
