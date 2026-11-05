# Local backend stand-in and server probes

This directory holds the local stand-ins for the Epic backend services that the
12.60 client calls, plus the probes used to observe the client. Everything here is
Python standard library only. Nothing here is a real account, credential or key. Tokens
and session keys are generated per process and kept in memory.

It does not implement the reconstructed UE game server. That lives in `porting/native/`.
Evidence for each route is in `porting/server_surface.md`.

## Layout

| File | Role |
| --- | --- |
| `backend.py` | Single dispatcher. Method and path resolution, error bodies, in-memory request log. `python3 -I backend.py --routes` and `--commands` print the tables below. |
| `api_common.py` | Error codes (`Err`), ids (`Context`: seeded, deterministic), request and response types, the router, evidence labels. |
| `oauth_service.py` | Token (`client_credentials`, `exchange_code`, `refresh_token`), exchange code, verify, session kill. |
| `account_service.py` | Account lookup, `tryPlayOnPlatform`, profile token verify, user setting. |
| `profile_service.py` | MCP profile store, revisions, atomic command runner, command set. |
| `profile_defaults.py` | Default athena, common_core, common_public, campaign and creative profiles. |
| `party_service.py` | Party v1 (create, join, invite, decline, member and party meta, promote, leave/kick, disconnect, ping) and presence. |
| `content_service.py` | Lightswitch, version check, enabled features, calendar, storefront, keychain, datarouter. |
| `cloudstorage_service.py` | System files from `--cloudstorage-dir` (default: none); local user files can persist. |
| `frontend_stubs.py` | Friends, social ban, receipts, billing and other menu answers. Labelled guess. |
| `matchmaking.py` | Ticket, session and session-key routes. Websocket state machine (`local_ws.py` is the RFC 6455 side). |
| `fortnite_stub.py` | Compatibility facade for `probe_server.py --stub`. Keeps the earlier `respond()` and `websocket_route()` names. |
| `probe_server.py` | HTTP server. `--stub` serves the backend. Without it, it only records requests and serves fixtures. |
| `test_services.py` | Self-contained tests. 72 tests over real HTTP on a random localhost port, including `SchemaTests` (reflection checks). |
| `chat_service.py` | Game chat room reservation (`reserveGeneralChatRooms`), answered with `GeneralChatReturn` members and no rooms. |
| `schema_check.py` | Reflection-derived struct schemas and response completeness checks; requiring each member is a policy, not proof the client requires it. `python3 -I schema_check.py` prints the members. |
| `reflection_schemas.json` | Member names and kinds of the non-MCP reflected structs used by the checks (generated). |
| `evidence_scan.py` | Scans the runtime image for route and key strings and their code references. Writes JSON; no output is checked in. |
| `build_evidence.py` | Builds `endpoint_evidence.json`, `reflection_schemas.json` and the table in `EVIDENCE.md` from the scan. |
| `EVIDENCE.md`, `endpoint_evidence.json` | Where each route comes from, with its evidence class. Read `EVIDENCE.md` first. |
| `smoke_matchmaking.py` | End-to-end check of the lobby-to-match routes, through `probe_server.py --stub` in a subprocess. |
| `smoke_join.sh` | Native UE dedicated server and client join. Unrelated to the Python stand-in. |

## Running

```sh
python3 -I porting/server/test_services.py          # backend HTTP service tests
python3 -I porting/server/smoke_matchmaking.py      # lobby-to-match smoke test
python3 -I porting/server/probe_server.py --stub --port 8080 [--state-dir DIR] [--cloudstorage-dir DIR]
python3 -I porting/server/backend.py --routes       # route table with evidence labels
python3 -I porting/server/backend.py --commands     # MCP command table
```

### Username-only local accounts

The local service selects an account by username, without a password, entitlement check,
or anti-cheat. The same username (case-insensitive, ignoring surrounding whitespace)
reopens the same local account and locker. Different usernames have separate profiles
and cloud files. This is local identity selection, not identity verification.

For a client that supports the local grant, POST `/account/api/oauth/token` with form
`grant_type=local_username` and `username=<chosen name>`. The existing client exchange-code
transport can instead use `exchange_code=local-username:<chosen name>`; this is an
identity selector, not a password. For the original-client analysis harness, run
`porting/run_original_client.sh --username "Name"`; the original binary may still stop at
its own launcher entitlement gate. The rebuilt client has not yet been verified through
a full login-to-lobby flow.

The handwritten `ULocalAccountSubsystem` is an unverified client integration
point: `SelectUsername` requests this grant and exposes the selected account and
token. It is not yet connected to the game's online login or profile systems;
the native generator must be promoted before building it with its HTTP dependency.

With `--stub`, `probe_server.py` persists account mappings, profile revisions and user
cloud files under ignored `porting/generated/local-backend-state` by default. Set
`--state-dir DIR` to choose a different local directory, or `--ephemeral` for fresh
in-memory state. Tokens, parties and matchmaking sessions remain process-local; obtain
a new token after restarting the service. Only one probe server can use a state
directory at a time; a second server exits instead of risking conflicting writes.
Code embedding `Backend` directly must likewise use a single owner per state directory.

`probe_server.py` binds `127.0.0.1` by default and records method, path, route
name, query keys, body length and status to ignored `porting/generated/server-requests.jsonl`.
Headers, bodies and token values are not recorded. The tests write to
`porting/generated/test-services-*`, which is also ignored.

## Evidence labels

`EVIDENCE.md` is the authority. The labels below are the older route labels, which the dispatcher
still checks (`observed`, `inferred`, `guess`). Each route also has an evidence class there
(`wine_log`, `reflection`, `binary`, `asset`, `binary_string_only`, `convention`).

Each route and command carries one label. The tables below use these labels.

- **observed**: the name or path appears in the 12.60 Win64 binary strings, in the
  cooked `DefaultEngine.ini`, or in the earlier login capture in `server_surface.md`.
- **inferred**: the path is observed (or the namespace is), but the method, body or
  response shape is reasoned from public Epic API documentation or nearby evidence.
  It has not been checked against a client capture.
- **guess**: invented here, or kept from the earlier stub. Unverified.

Field names that appear in the binary are noted in the module docstrings. Where a
field name is observed but its nesting or value set is not, that is inferred.

## What is implemented

### Routes

The table is generated by `backend.py --routes` and reflects the code as it is.

| Method | Path pattern | Route | Evidence |
| --- | --- | --- | --- |
| POST | `/account/api/oauth/token` | oauth.token | observed |
| GET | `/account/api/oauth/exchange` | oauth.exchange | inferred |
| GET | `/account/api/oauth/verify` | oauth.verify | inferred |
| DELETE | `/account/api/oauth/sessions/kill` | oauth.sessions.kill | observed |
| DELETE | `/account/api/oauth/sessions/kill/{token}` | oauth.sessions.kill_token | observed |
| GET | `/account/api/public/account/{id}` | account.by_id | observed |
| GET | `/account/api/public/account?accountId=` | account.by_ids | observed |
| GET | `/account/api/public/account/displayName/{name}` | account.by_display_name | inferred |
| GET | `/account/api/public/account/{id}/externalAuths` | account.external_auths | observed |
| POST | `/api/v1/user/setting` | account.user_setting | observed |
| POST | `/fortnite/api/game/v2/tryPlayOnPlatform/account/{id}` | account.try_play_on_platform | observed |
| POST | `/fortnite/api/game/v2/profileToken/verify/{id}` | account.profile_token_verify | observed |
| POST | `/fortnite/api/game/v2/profile/{id}/(client\|dedicated_server\|public)/{command}` | profile.command | observed |
| POST | `/party/api/v1/Fortnite/parties` | party.create | inferred |
| GET | `/party/api/v1/Fortnite/parties/{partyId}` | party.get | inferred |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}/join` | party.member.join | inferred |
| DELETE | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}` | party.member.leave_or_kick | inferred |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/invites/{id}` | party.invite | inferred |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/invites/{id}/{from}/decline` | party.invite.decline | inferred |
| PATCH | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}/meta` | party.member.meta | inferred |
| PATCH | `/party/api/v1/Fortnite/parties/{partyId}/meta` | party.meta | guess |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}/promote` | party.member.promote | inferred |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}/(confirm\|reject)` | party.member.applicant | inferred |
| POST | `/party/api/v1/Fortnite/parties/{partyId}/members/{id}/disconnect` | party.member.disconnect | inferred |
| GET | `/party/api/v1/Fortnite/user/{id}` | party.user | inferred |
| POST | `/party/api/v1/Fortnite/user/{id}/pings/{from}/parties` | party.ping | inferred |
| GET | `/presence/api/v1/_/{id}/last-online` | presence.last_online | inferred |
| GET | `/presence/api/v1/_/{id}/subscriptions` | presence.subscriptions.list | inferred |
| POST | `/presence/api/v1/_/{id}/subscriptions/{other}` | presence.subscriptions.add | inferred |
| DELETE | `/presence/api/v1/_/{id}/subscriptions/{other}` | presence.subscriptions.remove | inferred |
| GET | `/lightswitch/api/service/bulk/status?serviceId=` | lightswitch.bulk | observed |
| GET | `/lightswitch/api/service/{id}/status` | lightswitch.single | inferred |
| GET | `/fortnite/api/v2/versioncheck/{platform}` | versioncheck | observed |
| GET | `/fortnite/api/game/v2/enabled_features` | enabled_features | observed |
| GET | `/fortnite/api/calendar/v1/timeline` | calendar.timeline | guess |
| GET | `/fortnite/api/storefront/v2/catalog` | storefront.catalog | guess |
| GET | `/fortnite/api/storefront/v2/keychain` | storefront.keychain | guess |
| GET | `/content/api/pages/fortnite-game` | content.page | guess |
| POST | `/datarouter/api/v1/public/data` | datarouter | observed |
| GET | `/fortnite/api/cloudstorage/system` | cloudstorage.system.list | observed |
| GET | `/fortnite/api/cloudstorage/system/{file}` | cloudstorage.system.file | observed |
| GET | `/fortnite/api/cloudstorage/user/{id}` | cloudstorage.user.list | observed |
| GET | `/fortnite/api/cloudstorage/user/{id}/{file}` | cloudstorage.user.file | observed |
| PUT | `/fortnite/api/cloudstorage/user/{id}/{file}` | cloudstorage.user.put | observed |
| GET | `/friends/api/v1/{id}/summary` | friends.summary | guess |
| GET | `/friends/api/v1/{id}/(blocklist\|recent/{id})` | friends.lists | guess |
| GET | `/friends/api/v1/{id}/settings` | friends.settings | guess |
| GET | `/socialban/api/public/v1/{id}` | socialban | guess |
| GET | `/party/api/v1/Fortnite/user/{id}/notifications/undelivered/count` | party.notifications_count | guess |
| GET | `/fortnite/api/game/v2/privacy/account/{id}` | privacy.account | guess |
| GET | `/fortnite/api/game/v2/clientfeaturekeys/{id}` | clientfeaturekeys | guess |
| GET | `/fortnite/api/receipts/v1/account/{id}/receipts` | receipts | guess |
| GET | `/api/public/payment/accounts/{id}/billingaccounts` | billing.accounts | guess |
| POST | `/fortnite/api/storeaccess/v1/request_access/{id}` | storeaccess | guess |
| POST | `/api/game/v2/chat/{accountId}/reserveGeneralChatRooms/{subgame}/{platform}` | chat.reserve_general_rooms | inferred |
| GET | `/presence/api/v1/_/{id}/settings/subscriptions` | presence.settings | guess |
| GET | `/presence/api/v1/Fortnite/{id}/subscriptions/nudged` | presence.nudged | guess |
| GET | `/api/v1/Fortnite/get` | legacy.get | guess |
| GET | `[/fortnite]/api/game/v2/matchmakingservice/ticket/player/{id}` | matchmaking.ticket | observed |
| GET | `[/fortnite]/api/matchmaking/session/{id}` | matchmaking.session | observed |
| GET | `[/fortnite]/api/game/v2/matchmaking/account/{id}/session/{sid}` | matchmaking.session_key | observed |

Unknown paths answer 404 `errors.com.epicgames.not_found`. A known path with the wrong
method answers 405. Party and presence routes that are not in the table, such as
`/conferences/connection`, answer 404. The matchmaking websocket is served at
`/fortnite/api/game/v2/matchmakingservice/ws/{ticketId}`, the path in `serviceUrl`.

### Authentication

- Profile, party, presence, cloud user storage and the account-scoped OAuth routes
  require `Authorization: bearer <token>`. A missing token answers 401
  `errors.com.epicgames.common.oauth_authentication_required`. An unknown or expired
  token answers 401 `errors.com.epicgames.account.auth_token.invalid_access_token`.
  A token for another account answers 403 `errors.com.epicgames.common.missing_permission`.
- Public account lookups, the lightswitch and version routes, cloud system files and
  matchmaking take no token. That matches the earlier smoke test and the observed
  pre-login requests. A client capture with headers would confirm or change this.
- Access tokens last 8 hours and refresh tokens 32 hours, which are the values the
  earlier stub returned. Refresh rotates both tokens.

### Exchange code policy

The `local-username:` exchange-code prefix selects a named local account. Other
non-empty `exchange_code` values retain the legacy LocalPlayer fallback unless they use the
reserved `local-exchange:` prefix. A code issued by `GET /account/api/oauth/exchange`
uses that prefix, is single use, and maps to its own account. Unknown, expired, and
used codes with the reserved prefix are rejected. This is a local policy, not Epic behaviour.

### MCP profile commands

`profile_service.py` implements the command set below. `QueryProfile` is handled
separately: it always returns a `fullProfileUpdate`, which matches the observed
login capture. The table is generated by `backend.py --commands`.

| Command | Profiles | Name evidence | Body |
| --- | --- | --- | --- |
| QueryProfile | any | observed | none |
| ClientQuestLogin | athena | observed | no body. Once per UTC day: advances the daily login counter. |
| CopyCosmeticLoadout | athena | observed | sourceIndex, targetIndex, optNewNameForTarget (inferred). |
| EquipBattleRoyaleCustomization | athena | guess | slotName, itemToSlot, indexWithinSlot (inferred). Targets the last applied loadout. |
| MarkItemSeen | athena, common_core | observed | itemIds: list of item ids (inferred). |
| SetAffiliateName | common_core | observed | affiliateName (inferred). No affiliate check. |
| SetBattleRoyaleBanner | athena | guess | bannerIconTemplate and bannerColorTemplate (inferred). Mirrors to common_public (multiUpdate). |
| SetCosmeticLockerBanner | athena | observed | lockerItem, bannerIconTemplateName, bannerColorTemplateName (reflected; JSON casing inferred). |
| SetCosmeticLockerName | athena | observed | lockerItem and name (inferred). |
| SetCosmeticLockerSlot | athena | observed | lockerItem, category, itemToSlot, slotIndex (inferred). variantUpdates ignored. |
| SetCosmeticLockerSlots | athena | observed | lockerItem and loadoutData (reflected names; JSON casing inferred). |
| SetItemFavoriteStatus | athena | observed | targetItemId and bFavorite (inferred). |
| SetItemFavoriteStatusBatch | athena | observed | itemIds and itemFavStatus, equal-length lists (inferred). |
| SetMtxPlatform | common_core | observed | newPlatform: one of the platform names (inferred). |
| SetReceiveGiftsEnabled | common_core | observed | bReceiveGifts: boolean (inferred). |
| PurchaseCatalogEntry | - | observed | not implemented (operation_not_found) |
| GiftCatalogEntry | - | observed | not implemented (operation_not_found) |
| RemoveGiftBox | - | observed | not implemented (operation_not_found) |
| RefreshExpeditions | - | observed | not implemented (operation_not_found) |
| SetActiveHeroLoadout | - | observed | not implemented (operation_not_found) |
| ClearHeroLoadout | - | observed | not implemented (operation_not_found) |

The default athena profile has a locker, `CosmeticLocker:cosmeticlocker_athena`, that
holds the default character, backpack, pickaxe, glider and dance. The template names
exist in the cooked pak index (`generated/assets.json`):
`CID_001_Athena_Commando_F_Default`, `BID_001_BlueSquire`, `DefaultPickaxe`,
`DefaultGlider`, `EID_Celebration`, `CosmeticLocker_Athena`. The `AthenaX:` prefix and
the slot mapping are inferred.

### Revision and envelope model

Every command response has `profileRevision`, `profileId`,
`profileChangesBaseRevision`, `profileChanges`, `profileCommandRevision`,
`serverTime`, `responseVersion` (1), `notifications` and `multiUpdate`. The field names
are observed in the binary. The `multiUpdate` entry shape is inferred.

- `profileRevision` rises by 1 for each committed command that changed the profile. A
  command that changes nothing leaves it alone.
- `profileCommandRevision` rises by 1 for every executed command, changed or not.
- `profileChangesBaseRevision` is the revision before the command. The deltas apply on
  top of it. The change types are `itemAdded`, `itemAttrChanged`, `statModified`
  (observed names), and `fullProfileUpdate` for QueryProfile.
- A command is atomic. If any step fails, the stored profile does not change.
- `multiUpdate` lists the other profiles a command changed, each with its own revisions.
- `notifications` is always `[]`. The notification payloads are not known.

### Party and presence

- Party state is in memory. A party has `config` (type, joinability, discoverability,
  maxSize, invite and intention TTLs), `meta`, `members` and `invites`, and a `revision`.
  Each member has `role` (CAPTAIN or MEMBER), `connections`, `joined_at`, `updated_at`,
  `revision` and `meta`.
- Joining is refused when the party is full (409), when the caller is already in a
  party (409), when the build id in `meta` (`urn:epic:cfg:build-id_s`, inferred) does not
  match (403), or when the party is CLOSED and no invite exists (403).
- The captain can kick members, promote a member and change party meta. When the
  captain leaves, the earliest-joined remaining member becomes captain. An empty party is
  deleted.
- `sendPing` on an invite records a ping. Pings and invites appear in `GET .../user/{id}`.
- Presence records the status (Online/Offline) and the party id, updated by join,
  leave, disconnect and create. `last-online` returns it. Subscriptions can be added and
  removed.

### Matchmaking and websocket

The session id and ticket id come from the deterministic id source. The signing secret and
session keys come from `secrets`. Websocket messages use the binary-observed `name` and
object `payload` envelope; Play includes match id, session id and join delay. The session-key
answer contains account id, session id and a Base64 key. See `EVIDENCE.md` for the client
reader traces and the unverified state values and end-to-end flow.
The local ticket's comma-separated `partyPlayerIds` become the session's public roster,
with duplicates removed and the ticket player included. Available public slots reflect
that roster. Only listed players can retrieve the session key; they receive the same key.
The party delimiter, roster policy and denial status are local choices, not confirmed
against the original client or Epic service.
The state step is 0.5 s by default, and `Backend(state_step_sec=0)` removes it for tests.

## Provenance

- **Observed in the 12.60 Win64 binary** (UTF-16 or ASCII strings): the command names
  above, the change-type names, the envelope and attribute names, the party and
  presence template paths, the party and presence field names, the error codes listed in
  `api_common.Err` (comment marks observed), the OAuth response field names, and the
  cloud storage field names. `porting/generated/DefaultEngine.ini` has the MCP URL
  templates and the `ServiceName="party"` setting. The earlier login capture in
  `porting/server_surface.md` has the pre-login route list.
- **Given in the task:** `buildUniqueId` = `683914758`, and the session record field names.
- **Guessed, unverified:**
  - The HTTP methods of the party and presence routes, the request body shapes, the
    response shapes beyond the observed field names, and the nesting of party objects.
  - The matchmaking ticket payload, status values and sequencing, session record values,
    and server key lifecycle (see the `[guess]` comments in `matchmaking.py`).
  - The profile command body keys and notification contents; the multiUpdate element type is reflected.
  - The lightswitch status values UP, DOWN, MAINTENANCE. The binary has none of them exactly.
  - Client id `3446cd72694c4a4485d81b77adbb2141` and device id (32 zeros). Carried from the earlier stub.
  - The keychain entry, which is 32 zero bytes as a placeholder and not a key.

## Not done

- **Not checked against a client.** The 12.60 client still stops at the frontend login
  gate (`direct_connect_results.md`), so none of these routes has been driven by the real
  client. A capture with headers and bodies is the next evidence.
- **XMPP and push.** Party and profile notifications, the presence stream, and the party
  chat channel are not served. `notifications` is always empty.
- **Beacons and the game session.** `BeaconPort` is advertised but not served. Party and
  lobby beacons, the game-session protocol and the UE netcode are not here.
- **Join requests and expiry.** The applicant flow answers 404 `applicant_not_found`
  because no request can exist. Invite and ping `expires_at` is informational, and
  nothing expires them.
- **Commerce and campaign.** Catalog purchases, gifting, hero and campaign loadouts,
  homebase, receipts and offers are not implemented. The command names are listed as
  not implemented.
- **Friends and social.** The friends, social ban and privacy routes are guesses with
  empty state. Friend requests and blocks have no backend.
- **Device and external auth.** Only client credentials, exchange code and refresh token
  are accepted. Device auth, external auth and `deviceAuthorization` answer 400
  unsupported grant or 404.
- **Error numeric codes.** Only the `numericErrorCode` 1004 from the earlier not-found
  body is kept. Other error bodies carry no numeric code because none is verified.
- **Cloud storage.** System files only come from `--cloudstorage-dir`. Storage quotas
  and retention are not modelled.
- **TLS.** The Python probe serves TLS only with `--tls-cert` and `--tls-key`. Whether
  the client accepts the self-signed chain is unverified (see `server_surface.md`).

## Probe usage (unchanged)

`probe_server.py` takes `--fixtures PATH` for JSON fixtures. A fixture can match a path
segment with `{name}` and substitute `${name}` in its response. Fixtures are checked
after the stub, so they can override a stub answer during investigation.

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

`/__health` answers readiness. Unknown requests answer 501 without `--stub`.

## Native dedicated server milestone (unchanged)

`FortniteLinuxServer.Target.cs` builds a Linux UE dedicated server from the new
`FortniteGame` module. With the recovered `FortniteEmptyDedicated` map staged, the server
listened on UDP 7777, and a local instance of the Linux client completed the UE join.
Run `bash porting/server/smoke_join.sh` to repeat the probe. Pass a staged map path as its
first argument. Logs go only under ignored `porting/generated/`.

The Apollo maps and the `FortGameModeAthena` probes are described in `porting/README.md`.
They do not depend on the Python services.
