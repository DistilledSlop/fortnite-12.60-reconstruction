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

