# Backend evidence table

This file says where each backend route comes from. It replaces the observed, inferred and guess
labels in `server_surface.md` where local evidence exists. The data is in `endpoint_evidence.json`
(same directory). The table at the end is generated from that JSON by `build_evidence.py`.

Nothing here is a capture of a driven client. The 12.60 client still stops at the frontend login
gate (`direct_connect_results.md`), so every row is static evidence or an earlier recorded run.

## Method

1. `evidence_scan.py` reads the 12.60 runtime image (`porting/generated/dump/...runtime.exe`, whose file
   offsets equal RVAs and whose `.text` is decrypted). It finds every exact NUL-terminated ASCII and UTF-16
   string that is a service path, plus the key and verb literals passed to it. For each one it finds the
   RIP-relative `lea` instructions in the code sections that load it, and the function that contains each
   `lea` (from `.pdata`). Command (about 15 s on an idle machine; the flag searches every identifier-like
   string, about 24,000 names, and the verbs `GET`, `POST`, `PUT`, `DELETE`, `PATCH` and the words `key`,
   `token`, `url`):

   ```sh
   python3 -I porting/server/evidence_scan.py --identifier-keys --out SCAN.json
   python3 -I porting/server/build_evidence.py --scan SCAN.json
   ```

   `SCAN.json` and the generated files are the outputs. `build_evidence.py` rewrites `endpoint_evidence.json`,
   `reflection_schemas.json` and the table below. It reads `porting/generated/reflection/struct_blocks.json`
   (git-ignored) only for the non-MCP structs.
2. `build_evidence.py` matches each backend route to a client format string. Matching is on path segments:
   a literal must equal a literal, and a capture pairs only with a capture. The one exception is the party
   namespace (`{namespace}` in the binary, `Fortnite` in our routes). Alternations are expanded. The
   service host prefix is not compared, because the client builds it from config.
3. Schemas come from the reflection data. MCP structs are read from the tracked headers under
   `porting/native/Source/McpProfileSys/Generated`, which are generated from `struct_blocks.json`. The chat,
   catalog and calendar structs are read from `struct_blocks.json` and copied, with member names and kinds, to
   `reflection_schemas.json`. Field JSON names follow the engine's `FJsonObjectConverter::StandardizeCase`
   (`Engine/Source/Runtime/JsonUtilities/Private/JsonObjectConverter.cpp`, line 15): lower-case the first
   character, then `ID` becomes `Id`.

## Evidence classes

| Class | Meaning | Confidence |
| --- | --- | --- |
| `wine_log` | The request was observed in an earlier Wine run of the 12.60 client (`server_surface.md`, "Requests observed, in order"). Gives the verb and the status seen. | Highest for the verb and status; the run was against our stub, not Epic. |
| `reflection` | The response or request shape is a reflected USTRUCT (member names and kinds). Linking a route to a struct is by name for the non-MCP ones (inferred). | High for the member list; medium for the route-to-struct link. |
| `binary` | The route format string is in the exe, and at least one function loads it (`lea`). The loading function and the verb and key literals in it are listed. | Medium: the route is built there; the verb and the response parser are not proven. |
| `asset` | The format is in the cooked config (`porting/generated/DefaultEngine.ini`, from the pak). | High for the template; the use is in config. |
| `binary_string_only` | The literal is in the exe, but no code reference was found (`lea` count 0). | Low: the string is present; its use is unknown. |
| `convention` | Public Epic API naming or our own choice. Not seen locally. | Lowest. |

A row's primary class is the strongest of its classes, in the order `wine_log`, `reflection`, `binary`,
`asset`, `binary_string_only`, `convention`. The JSON lists every class a row has.

Verbs are not in the strings. A verb literal (`GET`, `POST`, ...) found in the function that loads a route
is a hint only: 29 of 198 route builders have one. Verbs for the other routes are the backend's own
choice and are marked by the absence of a `wine_log` verb.

## Coverage

- Backend routes served: 61. Primary class: `wine_log` 28, `binary` 29, `reflection` 1 (chat),
  `convention` 3 (`lightswitch.single`, `socialban`, `storeaccess`).
- Client route format strings in the exe that the backend does not serve: 151 (`binary_unserved` in the JSON).
  Every one has at least one code reference. They include the `/api/matchmaking/session/...` routes,
  `/api/cloudstorage/...`, `/api/calendar/...`, party, presence and friends templates, `/api/inventory/v1`,
  `/api/messaging`, `/api/statsv2`, `/api/shared/...`, and the `/api/public/...` templates in the
  `backend.public_catalog` leaf.
- The keychain (`/api/storefront/v2/keychain`) and the datarouter (`datarouter/api/v1/public/data?SessionID=`)
  are strings with no code reference (`binary_string_only`). Their shapes come from the Wine capture only.
- The content page `/content/api/pages/fortnite-game` has no string in the exe (no match). It is seen only
  in the refresh pass of the Wine run.

## Findings from the schema check

The check is `schema_check.py` with the tests in `tests/test_services.py` (`SchemaTests`).

1. **Profile responses lacked `lockExpiration` and `notifications` in `multiUpdate` entries, and the root
   lacked `lockExpiration`.** `FProfileUpdateSingle` has seven members (reflection). The backend sent six
   at the root and five per entry. Fixed in `profile_service.py` (`_entry`). The value
   `0001-01-01T00:00:00.000Z` is `FDateTime::MinValue`, the value of an unlocked profile. The member is
   reflection; the value is a convention.
2. **The calendar timeline sent `eventsTimeOffsetHrs`, which `CalendarDownload` does not declare.** Removed
   from `content_service.py`. The timeline's `channels` entries match `EventsTimeline`.
3. **The catalog answer matches `CatalogDownload` exactly** (`storefronts`, `expiration`, `dailyPurchaseHrs`,
   `refreshIntervalHrs`). No change needed.
4. **`GeneralChatReturn` has five bool bitfields** (`bIsSubGameGlobalChatDisabled` and four more) that have no
   memory offset. A reader that skipped offset-less members would miss them. The chat route
   (`chat_service.py`) sends all five, as `false`.
5. **The MCP envelope is reflected.** `ProfileUpdate` inherits `ProfileUpdateSingle` and declares
   `ResponseVersion` (Int), `ServerTime` (DateTime) and `MultiUpdate` (array of ProfileUpdateSingle).
   The enclosing header is not generated; `ENVELOPE_FIELDS` records the recovered types. The
   checker requires a complete response even though the shipped converter skips absent properties.

## Open conflicts and risks

- **Key case.** Stock UE 4.25 `JsonAttributesToUStruct` looks up the property name in a
  `TMap<FString, ...>` (`JsonObjectConverter.cpp:799`). `FString::operator==` uses `Stricmp`
  (`Core/Public/Containers/UnrealString.h:1008-1010`), and its hash is case-insensitive
  (`UnrealString.h:1936-1940`), so this stock lookup can match `ProfileRevision` to
  `profileRevision`. The earlier assertion that the stock reader requires exact case was wrong.
  The shipped MCP path uses a reflected-struct converter, but its comparison helper has not been
  independently verified; maintain the existing camelCase wire keys pending that evidence.
- **Session key response.** Static analysis supports the names and the three nonempty
  string checks described below. The exact server key lifecycle and a live client join
  have not been verified; the backend's random per-session key is a local implementation.
- **Route inventory.** `porting/generated/routes.json` (the dashboard's route list, from
  `inspect_client_routes.py`) matches only `/api/(game/v2|public|oauth)/`. It misses the ~150 other route
  format strings above, so route-based dashboard leaves understate the surface. Not changed here, because
  changing a denominator changes what the leaf measures.
- **Stub credit counts tests.** The route measure counts a literal path that appears anywhere under
  `porting/server/**`, including the tests and this file. It is not a count of served routes.
- **Party, presence and friends are not reflected.** Their response types are not in the headers, and the
  reflected structs for them are not promoted. Their shapes stay `convention` or `guess`.

## Generated table

Each row is one served backend route. `Route format` is the client's string (or the config template). `exe VA`
is the address of the string in the runtime image (image base 0x140000000). `refs` is the number of `lea`
instructions that load it. Served rows only; the unserved route strings are in `endpoint_evidence.json`.

<!-- BEGIN GENERATED TABLE (build_evidence.py) -->
| Endpoint | Verb | Route format (client) | exe VA | refs | Response schema | Evidence | Served |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `oauth.token` | POST | `/api/oauth/token` | 0x145a9ea00 | 2 | - | wine_log, binary | yes |
| `oauth.exchange` | GET | `/api/oauth/exchange` | 0x145b0cf90 | 1 | - | binary | yes |
| `oauth.verify` | GET | `/api/oauth/verify` | 0x145a9ea28 | 1 | - | binary | yes |
| `oauth.sessions.kill` | DELETE | `/api/oauth/sessions/kill` | 0x145a9e998 | 1 | - | wine_log, binary | yes |
| `oauth.sessions.kill_token` | DELETE | `/api/oauth/sessions/kill` | 0x145a9e998 | 1 | - | wine_log, binary | yes |
| `account.by_id` | GET | `/api/public/account` | 0x145aa0490 | 2 | - | wine_log, binary | yes |
| `account.by_ids` | GET | `/api/public/account` | 0x145aa0490 | 2 | - | wine_log, binary | yes |
| `account.by_display_name` | GET | `/api/public/account/displayName/`name` | 0x145afdd60 | 1 | - | binary | yes |
| `account.external_auths` | GET | `/api/public/account/`id/externalAuths` | 0x145afdcb0 | 4 | - | wine_log, binary | yes |
| `account.user_setting` | POST | `/api/v1/user/setting` | 0x145aacdc8 | 1 | - | wine_log, binary | yes |
| `account.try_play_on_platform` | POST | `%s/api/game/v2/tryPlayOnPlatform/account/%s?platform=%s` | 0x1460e5940 | 1 | - | wine_log, binary | yes |
| `account.profile_token_verify` | POST | `/api/game/v2/profileToken/verify/`accountId` | 0x1460dd5c0 | 2 | - | wine_log, binary | yes |
| `profile.command` | POST | `(config) McpClientCommandUrl=/api/game/v2/profile/`accountId/client/`fname?profileId=`profileId` | - | 0 | FProfileUpdate (inherits FProfileUpdateSingle) | wine_log, reflection, asset | yes |
| `party.create` | POST | `/api/v1/{namespace}/parties` | 0x145a9fd28 | 1 | - | binary | yes |
| `party.get` | GET | `/api/v1/{namespace}/parties/{partyId}` | 0x145a9fd60 | 2 | - | binary | yes |
| `party.member.join` | POST | `/api/v1/{namespace}/parties/{partyId}/members/{accountId}/join` | 0x145a9f5b0 | 1 | - | binary | yes |
| `party.member.leave_or_kick` | DELETE | `/api/v1/`namespace/parties/`partyId/members/`accountId` | 0x145adc2d0 | 1 | - | binary | yes |
| `party.invite` | POST | `/api/v1/{namespace}/parties/{partyId}/invites/{accountId}` | 0x145a9ebd0 | 1 | - | binary | yes |
| `party.invite.decline` | POST | `/api/v1/{namespace}/parties/{partyId}/invites/{accountId}/{fromAccountId}/decline` | 0x145a9ed00 | 1 | - | binary | yes |
| `party.member.meta` | PATCH | `/api/v1/{namespace}/parties/{partyId}/members/{accountId}/meta` | 0x145a9f660 | 1 | - | binary | yes |
| `party.meta` | PATCH | `/api/v1/{namespace}/parties/{partyId}` | 0x145a9fd60 | 2 | - | binary | yes |
| `party.member.promote` | POST | `/api/v1/{namespace}/parties/{partyId}/members/{accountId}/promote` | 0x145a9f720 | 1 | - | binary | yes |
| `party.member.applicant` | POST | `/api/v1/{namespace}/parties/{partyId}/members/{accountId}/confirm` | 0x145a9f450 | 1 | - | binary | yes |
| `party.member.disconnect` | POST | `/api/v1/{namespace}/parties/{partyId}/members/{accountId}/disconnect` | 0x145a9f4f0 | 1 | - | binary | yes |
| `party.user` | GET | `/api/v1/{namespace}/user/{accountId}` | 0x145aa0120 | 1 | - | binary | yes |
| `party.ping` | POST | `/api/v1/{namespace}/user/{accountId}/pings/{fromAccountId}/parties` | 0x145a9ffd0 | 1 | - | binary | yes |
| `presence.last_online` | GET | `/presence/api/v1/_/`id/last-online` | 0x145b13e40 | 1 | - | wine_log, binary | yes |
| `presence.subscriptions.list` | GET | `/presence/api/v1/_/`id/subscriptions` | 0x145b13f50 | 1 | - | wine_log, binary | yes |
| `presence.subscriptions.add` | POST | `/presence/api/v1/_/`id/subscriptions/`otherId` | 0x145b13ef0 | 2 | - | wine_log, binary | yes |
| `presence.subscriptions.remove` | DELETE | `/presence/api/v1/_/`id/subscriptions/`otherId` | 0x145b13ef0 | 2 | - | wine_log, binary | yes |
| `lightswitch.bulk` | GET | `/api/service/bulk/status` | 0x145aaa6f8 | 2 | - | wine_log, binary | yes |
| `lightswitch.single` | GET | `not in binary` | - | 0 | - | convention | yes |
| `versioncheck` | GET | `/api/v2/versioncheck/`platform?version=`version` | 0x145b0d050 | 1 | - | wine_log, binary | yes |
| `enabled_features` | GET | `/api/game/v2/enabled_features` | 0x145ab8520 | 1 | - | wine_log, binary | yes |
| `calendar.timeline` | GET | `/api/calendar/v1/timeline` | 0x145c0c918 | 1 | CatalogDownload / CalendarDownload (reflection, not checked yet) | wine_log, reflection, binary | yes |
| `storefront.catalog` | GET | `/api/storefront/v2/catalog` | 0x145c08358 | 1 | CatalogDownload / CalendarDownload (reflection, not checked yet) | wine_log, reflection, binary, asset | yes |
| `storefront.keychain` | GET | `/api/storefront/v2/keychain` | 0x145c09448 | 0 | - | wine_log, binary_string_only | yes |
| `content.page` | GET | `not in binary` | - | 0 | - | wine_log | yes |
| `datarouter` | POST | `datarouter/api/v1/public/data?SessionID=` | 0x1467674c0 | 0 | - | wine_log, binary_string_only | yes |
| `cloudstorage.system.list` | GET | `/api/cloudstorage/system` | 0x145ad7e00 | 1 | - | wine_log, binary | yes |
| `cloudstorage.system.file` | GET | `/api/cloudstorage/system` | 0x145ad7e00 | 1 | - | binary | yes |
| `cloudstorage.user.list` | GET | `/api/cloudstorage/user/` | 0x145b1eb70 | 1 | - | wine_log, binary | yes |
| `cloudstorage.user.file` | GET | `/api/cloudstorage/user/`id/`filename` | 0x145b21090 | 2 | - | binary | yes |
| `cloudstorage.user.put` | PUT | `/api/cloudstorage/user/`id/`filename` | 0x145b21090 | 2 | - | binary | yes |
| `friends.summary` | GET | `/api/v1/`id/summary` | 0x145b08e28 | 1 | - | wine_log, binary | yes |
| `friends.lists` | GET | `/api/v1/`id/blocklist` | 0x145b08f78 | 1 | - | wine_log, binary | yes |
| `friends.settings` | GET | `/api/v1/`id/settings` | 0x145b08fe0 | 1 | - | wine_log, binary | yes |
| `socialban` | GET | `not in binary` | - | 0 | - | convention | yes |
| `party.notifications_count` | GET | `/api/v1/{namespace}/user/{accountId}/notifications/undelivered/count` | 0x145aa01d0 | 1 | - | binary | yes |
| `privacy.account` | GET | `not in binary` | - | 0 | - | wine_log | yes |
| `clientfeaturekeys` | GET | `/api/game/v2/clientfeaturekeys/`accountId` | 0x1460dd620 | 1 | - | binary | yes |
| `receipts` | GET | `/api/receipts/v1/account/`accountId/receipts` | 0x145b17220 | 1 | - | binary | yes |
| `billing.accounts` | GET | `/api/public/payment/accounts/`id/billingaccounts` | 0x145ad3cf0 | 1 | - | binary | yes |
| `storeaccess` | POST | `not in binary` | - | 0 | - | convention | yes |
| `presence.settings` | GET | `/presence/api/v1/_/`id/settings/subscriptions` | 0x145b13e90 | 2 | - | binary | yes |
| `presence.nudged` | GET | `/presence/api/v1/`namespace/`id/subscriptions/nudged` | 0x145b14090 | 1 | - | binary | yes |
| `legacy.get` | GET | `/api/v1/`namespace/get` | 0x145b10358 | 1 | - | binary | yes |
| `chat.reserve_general_rooms` | POST | `/api/game/v2/chat/`accountId/reserveGeneralChatRooms/`subgame/`platform` | 0x14635c190 | 1 | GeneralChatReturn | reflection, binary | yes |
| `matchmaking.ticket` | GET | `/api/game/v2/matchmakingservice/ticket/player/`id` | 0x1460d3c20 | 1 | - | binary | yes |
| `matchmaking.session` | GET | `/api/matchmaking/session` | 0x145b1ac08 | 1 | - | binary | yes |
| `matchmaking.session_key` | GET | `/api/game/v2/matchmaking/account/`accountId/session/`sessionId` | 0x1460dfbe0 | 1 | accountId, sessionId, key (base64) | binary | yes |
<!-- END GENERATED TABLE (build_evidence.py) -->

## MCP response conversion trace

The runtime image contains the UTF-16 diagnostic `MCP-Profile: Missing new
'profileChangesBaseRevision' key. Assuming base of %lld` at `0x145bfd4d0`. The LEA scan places
its code reference at RVA `0x11029f1` in `0x1411028a0`, which applies an already-decoded profile
update. Its caller `0x141102520` identifies itself as `UMcpProfile::HandleProfileUpdate`.

The response path `0x1411036a0` accepts an object or array. For an object, it obtains the
`/Script/McpProfileSys.ProfileUpdate` struct through `0x141113e20` and calls `0x143eed4a0`,
which forwards to `0x143eecfd0`. The latter iterates reflected properties, looks up names in the
JSON map, skips missing fields, converts values, and contains the log string
`JsonObjectToUStruct - Unable to parse %s.%s from JSON`: strong evidence identifying the shipped
converter path. An array element instead uses `AccountIdAndProfileResponse` from `0x1411136a0`,
whose reflected `Response` is `ProfileUpdate`. The reflection blocks identify the `ProfileUpdate`
superclass and all three envelope types; the test checks these relationships directly.

UE 4.25 source uses case-insensitive `FString` equality and hashing for the JSON attribute map
(see “Key case” above). The shipped binary's comparison helper is not independently verified, so
its exact case behavior remains open. Existing camelCase responses remain unchanged. The backend
schema checker demands every field as a completeness policy; a missing property alone does not
mean the converter rejects the response.

The standalone `profileId` literal at `0x145bff818` is referenced at RVA `0x10f00fc` in
`0x1410effd0`, which builds a request with `profileId` and `clientCommandRevision`; it is not
response-parser evidence. Ghidra slim decompiles, disassembly and caller metadata are in the
ignored `generated/mcp-parser-probe/`, `mcp-caller-probe/`, `mcp-caller2-probe/`,
`mcp-parser3-probe/` and `mcp-parser4-probe/` directories. Probe transactions were rolled back.

## Matchmaking session-key response trace

The route literal at `0x1460dfbe0` is loaded by `0x14222ab40`, which constructs the
account/session URL, supplies `GET`, and binds `0x142227630`. The callback's diagnostic
strings name `OnQueryEncryptionKeyForUniqueIdComplete`; it accepts a JSON object, invokes
the response object's parser, rejects three empty strings, then Base64-decodes the third.
The response object's vtable at `0x1460ddf98` has `0x142230050` at offset `+0x38`; that
reader passes `accountId`, `sessionId` and `key` to string-field handling at offsets `+0x08`,
`+0x40` and `+0x50`, respectively. These are binary observations, not a live capture.
`matchmaking.py` returns the requested identifiers and a Base64-encoded per-session key;
server-side encryption and a client round trip remain unverified. The ignored decompiles,
disassembly and caller metadata are under `generated/sessionkey-probe/`,
`sessionkey-callback/`, `sessionkey-parser/` and `sessionkey-fields/`.

The ticket request's `partyPlayerIds` parameter and the OnlineSessionMcp
`publicPlayers`, `openPublicPlayers` and `maxPublicPlayers` fields appear in the client
route/field inventory (`original_client_net_protocol.md`, section 3). The backend now
keeps those local values consistent for a comma-separated party roster: each listed
player can request the same session key, and an unlisted account gets a 404. The
delimiter, admission rule, 100-player capacity and error status are local policies;
binary evidence does not establish Epic's exact behavior. The HTTP/websocket test
`MatchmakingTests.test_party_ticket_session_roster_and_keys` verifies this flow without
a reconstructed-client join.

## Matchmaking websocket message trace

The client handler containing `0x14106b115` loads top-level `name` and `payload`, reads
`payload` as an object, and dispatches `StatusUpdate` to `0x14106a6a0` and `Play` to
`0x1410697d0` (dispatch references in `generated/ws-initial-probe/0x14106b115.asm`).
The status handler reads `state` from that object. The Play payload field reader
`0x14106dbf0` names `matchId`, `sessionId` and `joinDelaySec`. The backend sends the
nested envelope and those three Play fields. The matchmaking tests exercise the server's
websocket frames, not the original client.

The state dispatcher `0x14106a6a0` routes `Connecting` directly without a payload
reader, and routes the other states through these handler/vtable/reader pairs:

| State | Handler | Vtable | Reader | Payload fields read |
| --- | --- | --- | --- | --- |
| Waiting | `0x14106a9c0` | `0x145bd40c8` | `0x14106def0` | `totalPlayers`, `connectedPlayers` (integers) |
| Queued | `0x141069ac0` | `0x145bd4150` | `0x14106dc80` | `ticketId` (string), `queuedPlayers`, `estimatedWaitSec` (integers), `status` (type unresolved) |
| ReadyCheck | `0x141069f50` | `0x145bd41d8` | `0x14106dd20` | `matchId`, `status` (strings), `durationSec`, `totalPlayers`, `readyPlayers`, `unreadyPlayers` (integers), `ready`, `responded` (booleans) |
| SessionAssignment | `0x14106a470` | `0x145bd42a8` | `0x14106de90` | `matchId` (string) |

Each vtable's slot `+0x38` points to its listed reader, invoked through `0x14069c3d0`.
ReadyCheck explicitly branches on `ACTIVE`, `PASSED`, `FAILED` and `TIMED_OUT` and reports
other status strings as invalid. The backend supplies `ACTIVE` and all the named ReadyCheck
fields with locally selected values. It does not supply Queued `status`: its type and value
are unresolved. Field-reader calls alone do not prove that absent fields are rejected, and
no driven client has verified this complete sequence. Decompiles are in ignored
`generated/ws-initial-probe/`, `generated/ws-parsers/` and `generated/ws-state-handlers/`.

## November 12 social compatibility follow-up

The original friend-message dispatcher `0x140ca91f0` accepts messages from
`xmpp-admin` on the logged-in JID domain. The Friend reader `0x140cd6a80`
reads `accountId`, `status`, `direction`, `created` and optional `connections`.
Decoder `0x140cddf70` recognizes ACCEPTED, PENDING+INBOUND,
PENDING+OUTBOUND and SUGGESTED; a pending notification without direction maps
to the ignored state. Removal uses the distinct
`com.epicgames.friends.core.apiobjects.FriendRemoval` type, whose reader
`0x140cd6ef0` reads `accountId` and `reason`. Recognized reasons are DELETED,
REJECTED and ABORTED; their mapping to local delete/reject/cancel operations
is inferred. Envelope parser `0x144873a60` accepts a nested object/string
payload or the whole body when payload is absent; a timestamp is optional.
The backend emits recipient-relative account IDs after the graph transaction
commits. Capture index: ignored private-package
`out/friend-notification-contract-bfbe53233d7f44d6937b7d46bf5e30d5.json`.

An original-client Accept action called POST
`/friends/api/v1/{accountId}/friends/{otherAccountId}`. The backend now exposes
that observed alias alongside the existing public route. A follow-up client run
returned 204, removed the pending entry and displayed the accepted account in
the offline friends list. Remove Friend then called the matching v1 DELETE,
returned 204 and cleared that entry. During the same run a newly issued friend
request displayed a live notification toast and incoming badge without restart.
This validates one client with a local API peer; two online client accounts,
presence and whisper delivery still need verification.

The party root PATCH builder `0x140a53ad0` and serializers `0x140a72850`,
`0x140a720c0` and `0x140a72b60` establish optional `config` and `meta` objects
and a signed 64-bit `revision`. Config uses `max_size`, `invite_ttl`,
`join_confirmation`, `discoverability` and `joinability`. Meta supports
string-array `delete` and object-valued `update` and `override`.
The response checker `0x140a57390` accepts HTTP 200 through 206 and does not
require a success JSON body; 204 is a compatible local choice. Local override
handling deletes keys, merges override, then merges update; this is an explicit
local policy, not a recovered original-server rule. Private capture index:
`out/party_patch_contract_20261112/contract.json`.

The next isolated original-client diagnostic observed create=200,
root PATCH=204 (both config/meta and meta-only requests) and member-meta
PATCH=204. It used the Curl, AppES and WSS diagnostic bypasses. XMPP logs
recognize PARTY_UPDATED and MEMBER_STATE_UPDATED but show default/empty values
for several configuration/metadata fields. Notification reader parity and
two-client party behavior remain open.

The recent-player callback `0x140cc1460` wraps the raw response array under
`recentplayers` internally, preserving accepted entry order. Reader `0x140cd7250`
reads `accountId` and `lastPlayed`, with `0x142ca3070` parsing the date string.
The generic reader requires a nonempty account ID; a missing/invalid date does
not itself reject the entry. The request namespace partitions the client cache.
The live client requested `/friends/api/v1/{accountId}/recent/fortnite`.
Private capture index: `out/recent-players-contract-0dc3119a69c44e8ebb70a5319ac43e82.json`.
The backend now derives entries from committed match-result pairs sharing a
session, with latest shared time and deterministic account-ID tie-breaking.
Blocking filters either direction. These ordering/filtering rules and the
one-registration-per-match requirement are explicit local policies. Nonempty
client recent-player rendering remains unverified.
