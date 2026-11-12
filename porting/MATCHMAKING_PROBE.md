# Original-client matchmaking probe

On 2026-11-09 the original client was run in the private network namespace with
local login, TLS and AppES diagnostic runtime patches, and a native dedicated
server listening on the empty test map. Pressing Play returned a matchmaking
ticket, but the client's secure WebSocket failed to connect to the local
self-signed service. The exact cause of that connection failure is not proven.

A second run used the runner's loopback-only plaintext WebSocket listener; the
client connected, processed the queue and ready-check messages, and received a
session assignment. Its subsequent session lookup returned a response, but the
client reported `Unable to read session settings!` and a missing
`openPrivatePlayers` field, then `FindSessionFailure`. It did not join the
server. The dedicated server and backend shared a temporary test key, but the
client did not reach the encryption handshake. This is a diagnostic test, not a
verified match or a playable Battle Royale server.

A later run supplied the assigned session ID and the client's compatible
changelist (12896485) as the session build ID. The client then accepted the
session, fetched its encryption key, and posted to the session's `/join`
endpoint. That route returned 404, so no game-server connection was attempted.
The network checksum 683914758 is a different value from the matchmaking build ID.

After the local join endpoint was added, the original client reported session
join success and sent UDP packets to the dedicated server. The server received
them but repeatedly reported `IncomingConnectionless: Error reading handshake
packet`; no game-world join occurred. A loopback packet capture showed a
25-byte initial client handshake. Its exact format and the compatibility fix
remain under investigation.

## 2026-11-11: handshake, control channel and Join reached (G3, G4; G5 rejection list)

Runs use `tools/vanilla_play_run.sh` (server and original client in one namespace, scripted clicks at 125 s and
235 s, packet capture). Evidence is under the ignored `generated/wine-investigation/20261111-*` directories.

- **G3 passed.** With `patches/ue425-fortnite-float-handshake.patch` the server sends the challenge, accepts the
  response, sends the ack and creates the `UNetConnection` ("Server accepting post-challenge connection").
  The handshake was never the loop cause. The loop came from the first data packets (observed, `server.log` and
  `udp.pcap` of run 1 and run 4).
- **Cause of the restart loop (observed + binary).** The client's packet handler chain is Oodle, AES, StatelessConnect
  (client log, `PacketHandlerLog: Loaded PacketHandler component`). The Oodle component puts one "compressed packet"
  bit after the AES bit on every packet, even though the loose `Engine.ini` sets `bEnableOodle=False`. In the capture every
  normal packet starts `1110` (magic), handshake bit 0, AES bit 0, then this bit (0 on 11-byte packets, 1 on the 93-byte
  packets). The server did not read that bit, so it parsed the packet header one bit early (sequence read as 2n+1, acked
  sequence as 2m mod 2^14) and then closed the connection on the first large packet with
  `Malformed_Packet: Received packet with 0's in last byte`; a later packet got `SendRestartHandshakeRequest`.
- **Fix (works, run 4 onward).** (1) Engine patch `patches/ue425-fortnite-original-client-net.patch`: with
  `-FortniteOriginalClient`, the engine's Oodle plugin (no SDK) creates a bit-only stand-in that reads/writes that one
  bit (a set bit is dropped, we cannot decompress). `FortniteGame.cpp` sets `PacketHandlerComponents.Components` to
  `OodleHandlerComponent` in that mode so the server chain is Oodle, AES, StatelessConnect, as on the client.
  (2) The client must never compress: `net.OodleMinSizeForCompression` is 19 from shipped config and the loose-ini
  override does not win (same reason `n.VerifyPeer` needs a memory patch), so `server/patch_oodle_min_size.py`
  (started by `run_original_client.sh`, `FN_PATCH_OODLE=0` to disable) writes 100000 into the cvar data in the running
  client every 0.25 s. Diagnostic in-memory patch like the TLS and AppES ones; G11 will need a different answer.
  Supporting compression instead needs the Oodle Network SDK, which we do not have.
- **G4 passed.** Server log: remote network version 683914758 matches, encryption token (65 characters) answered with
  the 32-byte key, packets decrypt, `Level server received: Login`, then (after the next item) `Welcomed`. Client log:
  `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated, Game: /Script/FortniteGame.FortGameModeAthena)`.
  Login URL options the client sends: `EncryptionToken`, `Name`, `AuthTicket`, `ASID`, `Platform`, `AnalyticsPlat`,
  `bIsFirstServerJoin`; unique id `MCP:<32 hex>`, platform `MCP`.
- **Two stock-engine checks stopped the original client** and are bypassed only under `-FortniteOriginalClient`
  (same engine patch): `AGameModeBase::PreLogin` rejected the `MCP` unique id (`incompatible_unique_net_id`), and
  `UOnlineEngineInterfaceImpl::RegisterPlayer` crashed the server on `FUniqueNetIdNull` (assertion in the Null
  subsystem types). The original `AFortGameModeAthena::PreLogin` (0x141950bd0) and `Login` (0x141959680) are not ported;
  these bypasses stand in for them.
- **G5 first result.** `Join succeeded: LocalPlayer`, the server spawned `FortPlayerPawnAthena` and opened actor channels
  2 to 5. The client then reported `NetChecksumMismatch` for the archetype (class default object) of the first four
  actors and closed the connection (server: `failed to initialize the PlayerController channel`). The first number is
  the checksum the server sent and the second the client's own (client log, run 6):

| Archetype (path under `/Script/FortniteGame`) | Server sent | Client computed |
|---|---|---|
| `Default__FortPlayerControllerAthena` | 1624899644 | 944441294 |
| `Default__FortPlayerPawnAthena` | 2152353856 | 2681584166 |
| `Default__FortGameStateAthena` | 1490346551 | 2463825438 |
| `Default__FortPlayerStateAthena` | 2713638545 | 3459852286 |

  The offline checksum parity (4,617 of 4,617) therefore does not hold against the real client at runtime for these
  four classes. The next step (plan WP A3 step 4) is to find which replicated properties or functions differ,
  starting with the shared engine base classes (the client reports `EngineNetVer 15`).


### 2026-11-12: class checksums now match the client (G5)

Method: the server dump (`-FortniteDumpClassChecksums`, plus `-FortniteDumpChecksumProps=<names>` for a recursive
child listing) prints each property's own and struct-inclusive checksum, and `class_checksum.py` was compared per property.
Engine ancestors (Actor, Controller, PlayerController) matched once the model was fixed (super struct fields included,
struct bitfield offsets from `generated/reflection/struct_bool_setters.json`, `mov byte [rcx]` setters decoded).
The remaining differences were all in the server's generated headers (fixed in `gen_native_classes.py`, 59+ headers regenerated):

| Cause | Fix in the generator |
| --- | --- |
| Struct members with `CPF_RepSkip` (0x80000000) were hashed on the server | emit `UPROPERTY(NotReplicated)` |
| Bitfields from different original bytes were packed into one byte, so the (offset, name) sort order differed | `uint8 : 0;` between original byte groups (`bool_bits.json`) |
| `TArray<UClass*>` inner (no `CPF_UObjectWrapper`) was emitted as `TSubclassOf<UObject>` | plain `UClass*` for replicated properties and struct members |

Result: server ClassChecksum equals the client's for FortPlayerControllerAthena 944441294, FortPlayerPawnAthena 2681584166,
FortGameStateAthena 2463825438, FortPlayerStateAthena 3459852286. Vanilla run `20261112-play-run-1`: 0 `NetChecksumMismatch`,
connection held until the run ended. Model gap still open: `class_checksum.py` gives 2004319565 for FortGameStateAthena
(the client value is authoritative), so its "expected" is not trustworthy for FortGameStateAthena or its struct children.
Known limits: `promote_native_classes.sh` currently fails on 3 `USceneComponent` behavior splices (present at HEAD, unrelated),
so the regenerated headers were copied from a scratch generation; struct bitfields have no byte-group separators yet.

### 2026-11-12: first G6 findings (pawn spawns, client stays on LOADING)

Runs `20261112-play-run-2` to `-5` (vanilla client, server build with the changes below). The client stays connected,
the server spawns `FortPlayerPawnAthena`, but the client's `ClientRestart_Implementation` never completes, so it stays
on the Battle Royale loading screen.

Fixed (server, C++ in the dedicated-server build):
- `SerializeNewActor failed to find/spawn actor` (channel 5): the actor was `GameplayDebuggerCategoryReplicator`
  (`/Script/GameplayDebugger`), which the Development server spawns per player controller and the shipping client does
  not have. The engine patch now skips those replicators under `-FortniteOriginalClient`
  (`GameplayDebuggerPlayerManager.cpp`, in `patches/ue425-fortnite-original-client-net.patch`).
- `ClientRestart_Implementation failed because WorldInventory is invalid`: the controller needs a replicated
  `AFortInventory` owned by it. `EnsureWorldInventory` in `FortniteAthenaGameplay.cpp` spawns an empty one before the
  first restart (stand-in until the original login body is ported).

Open:
- The client now logs `ClientRestart_Implementation failed because Quickbars are invalid` thousands of times.
  `ClientQuickBars` (`AFortPlayerController` +0x14d0, client-only, not replicated) is created by a controller virtual
  (`0x1423f20a0`) that `OnReadyToStartMatch` (`0x1423df310`) calls. `OnReadyToStartMatch` runs from
  `SetHasFinishedLoading` (`0x1423af810`) once the client's own `bHasClientFinishedLoading` and the replicated
  `bHasServerFinishedLoading` are both true. The client received `bHasServerFinishedLoading = true`
  (`OnRep_bHasServerFinishedLoading 1`, set by a stand-in in `EnsureWorldInventory`, because the generated
  `ServerSetClientHasFinishedLoading_Implementation` is a stub), but never called `SetHasFinishedLoading`.
- On the client that call comes from a per-tick controller check (`0x1423cc220`): server flag true, then
  `0x14236a740` (building foundations replicated, only logs at VeryVerbose), a loading-counter test
  (`0x143a9f860 == 0`) and a virtual at +0x2170. Which gate fails is not yet known; `LogFort VeryVerbose` printed none
  of the "Waiting for building foundations" messages.
- Related evidence that the player state is incomplete: the pawn logs `Customization ... still hasn't completed
  Hero:None DoCharacterPartsNeedReplication:1` and `SetPlayerTeam to No TeamInfo`. Team assignment, hero and character
  parts are the login/spawn pipeline (plan D1: `PostLogin`, player-state setup, team, loadout) and are the likely next
  fix.

Correction (later 2026-11-12, see `G6_CONTROLLER_PROBE_FIX.md`): the memory probe `tools/pc_probe.py` found no
controller in runs 6 to 9 because its vtable addresses were not table starts. The quickbar creator is slot `+0x1a98`
(not `+0x1d00`), called from the base `OnReadyToStartMatch` (`0x14239ccf0`). The per-tick driver `0x1423bc750` has
two gates (`[+0x220]`, bit 0x20 of `[+0x658]`) before the check `0x1423cc220` runs. Static analysis only.

Tooling: `tools/vanilla_play_run.sh <seconds> [client args...]` now forwards extra client arguments (for example
`"-LogCmds=LogFort VeryVerbose"`). Use at least 420 s; extra logging slows the client's start so 300 s ends right after
the join. Decompiles used `/opt/fortnite/local-model-package/scripts/evidence.py capture` on the addresses above.

### 2026-11-12 (later): G6 root cause of the quickbar wait found and fixed (run 12)

The client's `OnReadyToStartMatch`/quickbar creation runs from a 0.5 s timer that the client arms only on receiving the
`ClientOnGenericPlayerInitialization` RPC, which our server never sent. `EnsureWorldInventory` now sends it. Run 12
(`generated/wine-investigation/20261112-play-run-12`): `SetHasFinishedLoading` runs, `OnReadyToStartMatch` marks the
player ready, `ClientQuickBars` exists, `Quickbars are invalid` fell from 11,114 to 8. The client is still on the
loading screen at 300 s; next blocker is the pawn customization (`Hero:None`, `No TeamInfo`), see
`G6_CONTROLLER_PROBE_FIX.md` (live result section). The earlier note about `0x1423cc220` being a per-tick check is
corrected there: it runs from the timer.
