# Local server protocol probe

`probe_server.py` is a local HTTP recorder and fixture responder. It provides
the first server-side tool for reconstructing the client protocol. It does not
implement login, profiles, matchmaking, XMPP, or a UE game session yet.

Run `python3 porting/server/probe_server.py --port 8080`. It binds to
`127.0.0.1` by default and records method, path, query parameter names, body
length and hash, and response status to ignored
`porting/generated/server-requests.jsonl`. It does not store authorization
headers or request bodies. `/__health` returns a readiness response; unknown
requests return HTTP 501.

The optional `--fixtures path.json` accepts a JSON array. A fixture can match
a path segment with `{name}` and substitute `${name}` in its response:

```json
[
  {
    "method": "GET",
    "path": "/example/{name}",
    "status": 200,
    "body": { "name": "${name}" }
  }
]
```

Fixtures should be based on observed client requests and responses. This
tool's HTTP route coverage does not replace the authoritative game server.

## Native dedicated server milestone

`FortniteLinuxServer.Target.cs` builds a Linux UE dedicated server from the
new `FortniteGame` module:

```sh
/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FortniteLinuxServer Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
```

The built binary can be copied from `porting/native/Binaries/Linux/` into the
ignored staged build's `FortniteLinux/Binaries/Linux/` directory. With the
recovered `FortniteEmptyDedicated` map already staged, the server entered play
and listened on UDP 7777. A local instance of the new Linux client completed
UE challenge, login, and join, then loaded that map. Run
`bash porting/server/smoke_join.sh` to repeat the bounded probe, or pass a
staged map path as its first argument. It writes logs only under ignored
`porting/generated/`.

The same probe with
`/Game/Maps/Skybox/Skybox_Outpost_HomeBase` also succeeded: the server entered
the recovered skybox map and logged `Join succeeded`; the client received its
welcome and brought the skybox world up for play. The probe used `-nullrhi`
for both processes and ran for a bounded ten seconds.

The probe also succeeded with the recovered
`/Game/Athena/Apollo/Maps/Apollo_Terrain` persistent map after the four direct
Blueprint classes were rebuilt and cooked for Linux. The server brought that
map up and logged `Join succeeded`; the client was welcomed by the server and
brought the same map up. The streamed island levels and full Fortnite gameplay
are still absent.

The new native `AFortGameModeAthena` can be selected for this map with an
explicit UE map option:

```sh
bash porting/server/smoke_join.sh \
  '/Game/Athena/Apollo/Maps/Apollo_Terrain?game=/Script/FortniteGame.FortGameModeAthena'
```

This probe checks that the server spawns `AFortPlayerPawnAthena` and the
client receives its replicated pawn. It also checks that all 61 reconstructed
Apollo terrain meshes load on both peers and that the server pawn settles on
D4 terrain with Walking movement. It also checks that the temporary sun light
replicates to the client. The pawn currently has a first person camera,
basic walking, mouse look, and jumping input; it does not implement Fortnite abilities,
inventory, combat, cosmetics, or the original client protocol. Apollo still
lacks its streamed gameplay levels, props, and player start, so this is not a
playable island.

The earlier join probes use UE's `GameModeBase` and the newly authored client module. The
original Fortnite client protocol, account/profile services, game rules,
replication classes, and gameplay state are still unimplemented. The probe
does not establish compatibility with the supplied Windows executable.

## Lobby to match routes (`matchmaking.py`, `local_ws.py`)

`fortnite_stub.py` answers the matchmaking ticket, websocket, session and
session-key routes from section 3 of `original_client_net_protocol.md`. The
websocket is served by `probe_server.py` through `local_ws.py`, a stdlib-only
RFC 6455 server (no websocket library is installed, and none was added).

Run the end-to-end check, which starts the stub itself and prints PASS/FAIL per
step:

```sh
python3 porting/server/smoke_matchmaking.py
python3 porting/server/smoke_matchmaking.py --tls-cert CERT --tls-key KEY   # https and wss
```

Its request log goes to ignored `porting/generated/smoke-matchmaking-requests.jsonl`.

| Route | Answer |
| --- | --- |
| `GET [/fortnite]/api/game/v2/matchmakingservice/ticket/player/{id}` | 200 `{serviceUrl, ticketType, payload, signature}` |
| `WSS` `serviceUrl` | `wss://fortnite-public-service-prod11.ol.epicgames.com/fortnite/api/game/v2/matchmakingservice/ws/{ticketId}`; the host is mapped to loopback by `gen_hosts.py` and the cert wildcard covers it |
| `GET /fortnite/api/matchmaking/session/{id}` | 200 OnlineSessionMcp record, or 404 for an unknown session |
| `GET [/fortnite]/api/game/v2/matchmaking/account/{acct}/session/{sid}` | 200 `{"key": <base64 of 32 random bytes>}`, or 404 for an unknown session |
| `GET /fortnite/api/game/v2/clientfeaturekeys/{id}` | 200 `{}` (guess) |
| `GET /api/public/payment/accounts/{id}/billingaccounts` | 200 `[]` (guess) |

Websocket flow: `StatusUpdate` for `Connecting`, `Waiting`, `Queued`, `ReadyCheck`
and `SessionAssignment`, 0.5 s apart, then `Play` with `sessionId` and
`joinDelaySec`. The socket stays open. Client frames are read and discarded, pings
are answered, and the socket closes after 60 s idle. An unknown ticket is closed
with code 1008. Each ticket creates a session in memory, so the `Play` session id
is the one the session and key routes accept.

### Provenance

- **Observed in the 12.60 Win64 binary** (UTF-16 strings): `StatusUpdate`, the
  five state names, `HandlePlayMessage`, `joinDelaySec`, `estimatedWaitSec`,
  `matchId`, `totalPlayers`, `connectedPlayers`, `queuedPlayers`, `serviceUrl`,
  `ticketType`, the `Epic-Signed` header format, `BeaconPort`, `buildUniqueId`.
- **Observed in `original_client_net_protocol.md` section 3:** the route paths and
  the session record field names.
- **Given in the task:** `buildUniqueId` = `683914758`, and the session record fields.
- **Guessed, unverified:**
  - Message envelope. Each message is a flat JSON object with its name in a
    top-level `name` field (`MESSAGE_NAME_FIELD`). The binary shows field names
    but not the envelope. The message name `Play` is also a guess, since the
    binary only shows `HandlePlayMessage`.
  - Key response field `key` (`SESSION_KEY_FIELD` in `matchmaking.py`). The binary
    has no `encryptionKey` string, so the name is unverified. Change that one
    constant if the client disagrees.
  - Ticket `ticketType` (`mms-player`), the ticket `payload` contents, and the
    `signature`. The signature is an HMAC with a per-process secret. No server
    check of it exists, and the client's verification is unknown.
  - Session values: `serverAddress` 127.0.0.1, `serverPort` 7777, `BeaconPort` 7778,
    `serverName`, `maxPublicPlayers` 100, `openPublicPlayers` 99, `publicPlayers`
    [owner], `privatePlayers` [], `started` false, `ownerName` from the display name.
  - Message values: `totalPlayers` 1, `connectedPlayers` 1, `queuedPlayers` 0 or 1,
    `estimatedWaitSec` 0, `joinDelaySec` 0, and the 0.5 s pacing.
  - `clientfeaturekeys` and `billingaccounts`: the method and body are guesses,
    and neither route is in the observed login or lobby requests.

### Keys and secrets

- The session key and the ticket signing secret come from `secrets.token_bytes`
  and live only in process memory. Restarting the stub drops every ticket,
  session and key. Nothing is written to disk, and the request log records only
  paths, status codes and the `bucketId`/`profileId`/`rvn` selectors.
- Section 3 step 4 places the key request after the session lookup, but the
  client's use of the returned field is unverified. Treat the field name and key
  format as unverified until a client request or the binary confirms them.

### Not done

- The remaining `routes.json` templates are unanswered and return 404. They are
  chat, voice, twitch, neowiz, wallet, `world/info`, `deviceAuth`, agreements,
  assets, lookup, sources and affiliates. None of them appears in the observed
  login sequence. Their shapes are unknown, and a wrong 200 is worse than a 404.
- The ticket and session routes have not been exercised by the real client.
  Their verification is limited to the smoke test and curl. Whether the client
  accepts the self-signed wss certificate is also unknown. See "XMPP / WebSocket
  TLS" in `porting/server_surface.md` for the related `bDisableCertValidation`
  settings.
- Beacon, party, and the game-session protocol are not implemented.
- `probe_server.py` `/__health` used to fail with `UnboundLocalError`, because
  `form` was only assigned on the other branch. This was fixed while adding the
  websocket branch.
