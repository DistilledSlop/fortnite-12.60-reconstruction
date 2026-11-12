# D3: dedicated session, playlist command line, backend ↔ game server contract

Date: 2026-11-11. Source: client binary via `porting/tools/rd.py`, and
`porting/server/matchmaking.py`.

## 1. What is in the client binary (bucket R)

| Function | Address (chunk start) | Read result |
|---|---|---|
| AFortGameSessionDedicated::CreateDedicatedSession | 0x14214fcd0 | Builds the session settings: `MaxPlayers` (+0x21c) from the playlist (0x141f30010 fills MaxPartySize +0x220 and MaxPlayers), then `MaxPlayers += MaxSpectators` (+0x218). Sets session settings **`GAMEMODE`** and **`SESSIONKEY`**. Logs "COPY DATATABLES: Copying Data Table and Data Curves" (applies playlist data tables here). Uses ReservationBeaconHost (+0x288). Calls the online session interface CreateSession (slot +0x48) with the game session name (EName 0x11f). State set through vfn +0x868 (value 5 at the end). |
| AFortGameSessionDedicatedAthena::ParsePlaylistCommandLine | inside 0x142159d10 | Reads **`ServesPlaylists=`** and **`ExcludesPlaylists=`** from the command line (comma lists). |
| …::ProcessPlaylistCommandLine | 0x142159d10 | Validates each name with UFortPlaylistManager::GetAthenaPlaylist (0x141f2b110); a `*` wildcard skips validation. |
| …::GetServerMatchmakingBucketIdsV2 | same body | Bucket id format **`%u:%d:%s:%s:**`** = `<build CL>:<int>:<region>:<playlist>` plus `:**`; without playlists `%u:%d:%s:**`. Reads **`MMSPool=`**; pool patterns `*:*:*:*:*:private.*:**` / `*:*:*:*:*:public.*:**`. Shipping servers must have at least one included or excluded playlist (fatal log otherwise). |
| AFortGameSessionDedicated::OnAllPlayersUnregistered | 0x142164690 | restart trigger when everyone left |
| …::QueueEndSessionRestart / Restart | 0x14216ffa0 / 0x1421742e0 | end-of-match restart path |
| …::OnConnectionStatusChanged | 0x14216571e | backend connection loss handling |
| …Athena::OnMatchmakingSessionBackfillMatchRequestV2 | 0x14216977f | backfill (not needed) |
| …Athena::RemovePlayerFromSquadAndTeamMap | 0x1421728c9 | squad bookkeeping on leave |

## 2. Decision: do not port the Epic session stack for local play

The original dedicated server talks to Epic's matchmaking service and the
online-subsystem session interface. We have neither on Linux, and the vanilla
client never sees that traffic. It only sees:

1. the matchmaking websocket messages from **our backend**
   (`StatusUpdate` … `Play`, already working, G2 passed);
2. the session record from `GET /fortnite/api/matchmaking/session/<id>`
   (`serverAddress`, `serverPort`, `attributes.BeaconPort_i`, already served);
3. the UDP connection to that address.

So the plan is a small replacement, not a port:

- `AFortGameSessionDedicatedAthena` on our server keeps only: playlist choice,
  MaxPlayers from the playlist, RegisterPlayer/UnregisterPlayer bookkeeping,
  restart when all players left, and a report call to the backend.
- Skip: reservation beacon, backfill, MMS pools, bucket registration, mesh
  network. The client's `BeaconPort_i` is only used for party reservation on
  real servers. **Check in the G3 logged run** whether the vanilla client opens
  a beacon connection to `BeaconPort_i` before the game connection. If it does,
  the server needs a `APartyBeaconHost` that accepts every reservation (stock
  4.25 class, about 30 lines of setup). If it does not, nothing is needed.

## 3. Server command line (design)

```
FortniteLinuxServer Athena_Terrain?game=/Script/FortniteGame.FortGameModeAthena?RequiredPlayers=1
    -ServesPlaylists=Playlist_DefaultSolo -port=7777 -log
    -BackendUrl=http://127.0.0.1:<port> -SessionId=<id from backend>
```

- `ServesPlaylists=` is the original switch; keep the name so the ported
  ProcessPlaylistCommandLine works unchanged. The server picks the first
  listed playlist as the current playlist (design choice, mine; the original
  takes the playlist from the matchmaking assignment).
- URL options are the ones InitGame reads (see D2 card §2).
- `-BackendUrl`, `-SessionId` are ours (not original).

## 4. Backend game-server API (to add to `porting/server/matchmaking.py`)

All local, no auth beyond a shared secret from the environment. Names are ours.

| Call | Direction | Purpose |
|---|---|---|
| `POST /local/gameserver/register` `{address, port, playlists[], maxPlayers}` | server → backend | backend learns a server is up; returns `{sessionId}`. Replaces the hard-coded `SESSION_SERVER_ADDRESS/PORT`. |
| `POST /local/gameserver/<sessionId>/state` `{phase, numPlayers, joinable}` | server → backend | lets the backend stop sending new tickets to a match past warmup. |
| `GET /local/gameserver/<sessionId>/player/<accountId>` | server → backend | returns the profile data the server needs at PostLogin: display name, team/party, equipped cosmetics (athena profile loadout). This is how skins show in game. |
| `POST /local/gameserver/<sessionId>/result` `{accountId, placement, kills, xp…}` | server → backend | match result; backend applies XP/stat changes to the profile (G9). |

Backend ticket logic change: on a ticket, pick a registered joinable server
whose playlists include the ticket's bucket playlist; if none, either wait
(keep sending `Queued`) or launch one (later, F-package).

The bucket id the client sends in its ticket request has the same shape as
above (`<CL>:<int>:<region>:<playlist>`); parse the playlist from field 4.

## 5. Work items

1. Port ProcessPlaylistCommandLine (only the `ServesPlaylists=` part) and the
   playlist → MaxPlayers part of CreateDedicatedSession.
2. Apply playlist data tables where CreateDedicatedSession logs
   "COPY DATATABLES" (calls into ApplyPlaylistDataTables 0x141934980). This is
   needed before loot and storm work (D9, D10).
3. Add the four backend calls and a tiny HTTP client on the server
   (UE HTTP module, already in the project).
4. RegisterPlayer / UnregisterPlayer (GameSession vfn +0x840 / +0x848 as called
   from PostLogin / Logout) → report player count.
5. In the G3 run, answer the beacon question in §2.
