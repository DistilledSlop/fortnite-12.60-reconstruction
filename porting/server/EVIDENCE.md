# Backend evidence table

This file says where each backend route comes from. It replaces the observed, inferred and guess
labels in `server_surface.md` where local evidence exists. The data is in `endpoint_evidence.json`
(same directory). The table at the end is generated from that JSON by `build_evidence.py`.

The generated route table is primarily static evidence. Later sections record
driven original-client probes; their diagnostic bypasses and verification limits
are stated with each result.

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

The captured defaults led to recovery of PARTY_UPDATED reader `0x140c4a3f0`
and MEMBER_STATE_UPDATED reader `0x140c49b30`. Both read flat `party_id`, `ns`
and `sent`. Party updates additionally read `captain_id`, `party_privacy_type`,
`max_number_of_members`, `party_type`, `party_sub_type`, `revision`,
`party_state_removed`, `party_state_updated` and `party_state_overrode`.
Member updates read `account_id`, `account_dn`, `member_state_removed`,
`member_state_updated`, optional `connection`, `revision` and `reason`.
The updated/overrode fields require JSON objects, not strings containing JSON.
The backend now emits submitted metadata deltas under those names and uses
the member revision for member events. Snapshot copies prevent later mutations
from changing queued messages. A subsequent original-client diagnostic decoded
the correct captain, max members=16 and nonempty party metadata; its member-state
event decoded nonempty member metadata and revision=1. This validates decoding
in one client, not two-client party behavior. Other event types remain inferred.

Privacy decoder `0x140a46060` recognizes OPEN, INVITE_ONLY and INVITE_AND_FORMER.
Handler `0x140c253b0` maps OPEN to permission enum 3 and the other two to 0.
The local legacy CLOSED value is emitted as INVITE_ONLY; that compatibility
mapping is local policy. Capture groups in the private package are
`418135cc66a849369c6056ab92b2f540`, `5378d6ff36694fa880acabbc8c81404b`,
`1bad8abea9dd4de09a86cdd3972bfaeb` and `6c65e9fa026444bd9b390177b5cee47d`.

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
one-registration-per-match requirement are explicit local policies. A subsequent
original-client diagnostic displayed one recent player with the expected display
name. That check used two explicitly seeded, zero-XP result fixtures in the private
probe state; it does not establish real match-result/native integration.

## Friend nicknames

The original alias setter `0x140cd7af0` sends PUT to
`/friends/api/v1/{owner}/friends/{peer}/alias` with `Content-Type: text/plain`
and the raw string as its body. Delete builder `0x140c7eac0` uses the same path
with DELETE and no serialized body. The shared response validator explicitly
accepts empty 204; completion callbacks update/clear the local cache and notify
friends changes. Summary entry reader `0x140c67670` and single-friend handler
`0x140cb91d0` read top-level string `alias`. Private capture index:
`out/friend-alias-contract-279670ff16ea46d9ba5162ccaf5aed8b.json`.

The backend stores aliases in the social SQLite database, keyed by owner and
peer. Summary and legacy-list readback expose only the requesting owner's alias.
Requiring an accepted friendship, rejecting blank/control-containing names,
strict UTF-8 decoding and a 16 UTF-16-unit limit are local policies; the length
matches the original UI's displayed limit. Unfriend/block clears both private
aliases in the same transaction as the relationship change. Backend dispatcher
tests exercise authorization, privacy, restart, updates/deletion, validation,
cleanup and transaction rollback. The original UI subsequently submitted
`ProbeAlias` through PUT, received 204 and displayed it immediately. A fresh
client/backend displayed that saved nickname. Clearing the nickname in the
original UI sent DELETE, received 204 and restored SocialLiveFriend immediately. In that isolated run,
a scripted local friend completed XMPP authentication and presence. The original
client displayed it online as In the Lobby, showed its incoming diagnostic whisper,
and sent an exact UI reply that the peer received. This is one original client
plus a scripted peer, not a two-original-client acceptance test. Sanitized requests,
logs and screenshots are under ignored
`generated/g1-isolated-probe/attempt-alias-online-whispers/`; nickname restart
and deletion evidence is in `attempt-alias-restart-delete/`.

## Party approval and member event contracts

Join response reader `0x140a0af40` requires `party_id` and string `status`;
decoder `0x140a0e8b0` recognizes JOINED, PENDING_CONFIRMATION and REJECTED.
Confirm builder `0x140a4f440` sends POST without a serialized body. Reject builder
`0x140a51840` sends POST with `{}` or optional string `reason` (writer `0x140a715e0`).
Exact original success codes are unproven; the local service chooses 200 for join
and 204 for decisions. Applicants remain separate from admitted members, with
captain-only decisions and eligibility rechecks. Using intention TTL for request
expiry, silent expiration and invite retention until admission are local policies.

MEMBER_JOINED (`0x140c22590`), MEMBER_REJECTED (`0x140c235e0`) and
MEMBER_REQUIRE_CONFIRMATION (`0x140c23a80`) share flat reader `0x140c49b30`.
Joined dispatch additionally requires nonempty `ns` and `connection.id`.
Connection reader `0x140c48df0` reads `id`, `disconnected_at` and object `meta`.
Rejection handler `0x140c4fd90` passes a decimal reason string to the native
handler, which clears pending state and completes the local failed join.
The service now emits these flat fields with member revisions and copied metadata;
reject goes to the applicant with the supplied string reason. Their complete UI
flows remain unverified. Private index:
`work/party_approval_contract/evidence.json`.

Require-confirmation metadata `urn:epic:member:joinrequestusers` is a JSON string
containing an object with `users:[{id,dn,plat,data}]`; all four fields are strings
(reader `0x140bc8dd0`). Dispatch requires valid IDs and nonempty display name and
platform. The backend preserves the client's submitted metadata, without
fabricating joining users.

## Deadpool quest activation and trusted events

`season12_quest_data.py` records cooked definitions and uasset hashes for 21 quests,
12 bundles and 11 schedule entries. `QuestService` now follows those quest/bundle
reward links and initializes each recovered objective. Dated grants require the
operator's explicit EventFlag.LobbySeason12 start. W7 is independently scheduled
at day 42, its first quest at day 46; W6's separate dummy bundle is preserved.
Positive ObjectiveCompletionCount is honored; absent values use all objectives
as local policy, consistent with the native getter's nonpositive fallback but
without proof of every cooked class default.

The authenticated local game-server quest-event route requires admission for new
events. Receipts include the registration session and occurrence ID, commit in
the same profile transaction as progress/rewards, reject changed retries and
allow exact replays after restart without creating profiles. Events cannot advance
quests unlocked by that same occurrence. Tests cover date boundaries, multiple
objectives, W7/dummy separation, rollback, restart and bridge authorization.
Actual native gameplay emission remains unwired.

Weekly bundle completion rewards are not yet awarded. Client getters distinguish
`num_progress_quests_completed` from `num_quests_completed`; treating suppressed
quests as excluded from completion is unproven and would leave W2 with zero
eligible quests. Historical epoch, suppression/reward accounting and daily
rotation remain open.

## Recipient gift eligibility

Builder `0x144df16e0` issues GET to the configured service base plus
`/api/storefront/v2/gift/check_eligibility/recipient/{recipientId}/offer/{offerId}`;
no query/body is added. The base is dynamic, so accepting both bare and
`/fortnite`-prefixed paths is a local compatibility policy. Authentication setup
was outside this recovery; the backend requires a local OAuth account session.
Reader `0x140c2def0` explicitly accepts HTTP 204 without a response body.
It also accepts 200 through 206, requiring valid JSON when Content-Type is JSON.
Success callback `0x144df13e0` reads no eligibility-specific response fields.

Error callback `0x144df1b80` recognizes 404 as not a Fortnite player and the
subcatalog errors `receiver_will_not_accept_gifts` and `purchase_not_allowed`
as declined and already owned/ineligible. The generic reader extracts top-level
`errorCode`, `errorMessage` and optional numeric `numericErrorCode`. The backend
maps recipient absence, receive-gifts preference and existing ownership to
these client-recognized cases; the mapping and friend/block policy are local.
It shares those recipient predicates with the final gift transaction, which
rechecks them before debit/grant. The endpoint itself has no profile, currency
or graph writes. Private capture groups are `720719df4ed241929b5891d4209a15e0`,
`31eae107de844affbabf48d2f2c21297` and `23d529ad9cab41e78ef072eef5e4f628`.

This does not resolve sender MFA/eligibility or validate the original gifting
UI: the local accounts still report `tfaEnabled: false` without an enrollment
mechanism, and the client gift button remains gated.

## Maya schedule and lifecycle corrections

The cooked PaperDoll bundle lists nine quests, with day offsets 0, 5, 12, 19,
26, 33, 40, 47 and 54. Their names are not chronological: the top-20 quest
is second, and outlive is ninth. The local engine now uses each cooked stat
row, objective target and individual HeroSelection_01–09 reward. EventFlag.LobbySeason12
requires an explicit start, shared with Deadpool. With no start only the
GrantWithBundle quest appears. Receipt replay and transaction rollback apply
to these quests. Normalized definitions and source hashes are in
`season12_maya_data.py`; full property reports remain ignored. The bundle’s
HeroSelection_10 reward, CompletionCount=-1, gift-box and cleanup behavior
remain unresolved; passing nine quests does not currently award that token.

Party leave/kick now clears partyId without changing HTTP presence status or
lastOnline. Automatic captain succession emits the explicit promotion event
shape after MEMBER_LEFT to remaining members. This sequence is a local
inference, covered with three-member lifecycle tests; original two-client
acceptance and the disconnected-member reconnect contract remain open.

Maya permanent customization remains a separate missing profile path. Reflected
service requests are UnlockRewardNode(nodeId, rewardGraphId, rewardCfg) and
SetRewardGraphConfig(state:string[], rewardGraphId); their wire route and JSON
serialization are not captured. The cooked S12_HeroSelections graph contains
58 nodes across ten key tiers; each node maps to a cosmetic variant token.
Tier10.1 has RewardOperation RichColor with Maya as its context item. Node
requirements, one-key-per-tier limits and mappings are preserved in ignored
`generated/maya-graph-reports/normalized.json`. Ownership validation, graph creation,
purchase-token consumption and RichColor configuration need recovery before
this can safely become a permanent customization API.

## Remaining party notification readers

Reader `0x140c49680` consumes INITIAL_INVITE as flat party_id, ns, sent,
inviter_id, invitee_id, expires_at and object-valued meta. Its parser requires
valid party/inviter IDs, namespace and a nonempty `urn:epic:conn:platform`
metadata value. The backend now emits those fields from the stored invitation,
including the actual recipient and client-supplied metadata. It does not invent
a platform when the caller omits it.

MEMBER_LEFT and MEMBER_NEW_CAPTAIN use shared member reader `0x140c49b30`,
including flat account_id; neither requires a connection ID. Departures preserve
a pre-removal member snapshot for the notification. Captain removal now emits
MEMBER_KICKED; the original removal parser distinguishes envelope reasons
LEFT=1, EXPIRED=2 and KICKED=3. PARTY_UPDATED uses reader `0x140c4a3e0`; its flat
fields match the previously implemented party update. Private evidence index:
`/opt/fortnite/local-model-package/work/party_remaining_contract/evidence.json`.
Dispatcher tests cover recipient selection, state snapshots, deep copying,
explicit promotion, voluntary departure, captain removal and disband. Original
two-client acceptance remains pending.


## Daily lifecycle and original party chat naming

The normalized daily module records 62 cooked quest definitions, 70 objectives
and source hashes. AthenaChallengeRewardS12Daily supplies 31,000 seasonal XP;
each quest also grants one hidden athena_dailychallenge_token. Conditions and
tag/template filters are preserved as data. Trusted stat-row events must already
satisfy those filters; a general raw-event condition evaluator is not implemented.

Daily grants, rerolls and repeat instances now share the profile transaction.
The rotation is an explicit local policy, not recovered historical scheduling:
one grant per UTC day, at most three active quests, one reroll per day, no
catch-up, deterministic selection, missing Weight treated as one and explicit
zero excluded. Reward reconciliation during purchases or repair does not rotate
dailies. Seasonal quest XP advances season/book XP under local policy; crossed
rewards and durable event receipts commit atomically. Original daily UI and
claim UI remain unverified.

Original room construction at 0x140b89610 prefixes Party- to the party ID;
Strophe login at 0x14487ace3 constructs muc.%s from the configured domain.
JoinPublicRoom at 0x140c26bb0 constructs the occupant nick as
UrlEncode(displayName):accountId:xmppResource. URL encoding uses UTF8 and uppercase
hex. The backend preserves that nick and accepts the original room, retaining
bare party IDs only for local compatibility. The 1,023-character nick limit is
a local bound, replacing the insufficient 128-character limit. Backend XMPP
tests cover original-shaped long nicks, membership, blocks, chat and departure;
two original clients remain an outstanding check. Private captures:
71d2ca28a5994929a843d5eb8e2e9026, 2fb414ca95f04e7e8b3556afd53bb89d,
2abbfd02428343cdb94e6301a07dca1d, 132d31a2339f41eb8c849c7a1ce6e3be
and 9e69629bdfc94b15b177e833c6d75598.


## Fixed quest reward claims

Original UFortQuestManager::ClaimQuestReward at 0x142438a50 calls
ClaimSelectedQuestReward with index -1 when no selectable rewards exist. The
selection path at 0x142438be0 validates an index for selectable rewards, then
passes quest instance ID and index through 0x1429a7180 to the reflected profile
request. HTTP JSON casing and an omitted-index default were not observed.

The local ClaimQuestReward implementation requires an explicit index -1 for
supported fixed-reward quests. It validates stored Completed state and objective
counts, shares the atomic reward path with trusted progression, and accepts
already Claimed retries without another award. These server policies are local;
selectable quest rewards remain unsupported. Current trusted events still
automatically claim completed quests, also a local policy. Tests cover restart
retries, rejected premature/invalid claims and award-failure rollback.


An original-client run after daily quest grants showed the previously unsupported
MarkNewQuestNotificationSent operation as a modal error. The reflected profile
request takes ItemIds; UFortQuestItem reflects bool sent_new_notification. The
new handler validates the complete owned-quest batch, persists that field and
leaves objective/reward state unchanged. Body casing and field serialization
remain inferred; a new original-client run is required to confirm the modal is
resolved. Focused tests cover atomic rejection, idempotence and restart.


## First weekly mission family

The cooked Brutus family contains 22 quests and 36 objectives across weeks 1–2
and two separate faction choices. The implementation includes the 20
weekly missions, their standard 40,000 XP rewards, weekly/agent token grants and
bundle completion token thresholds. The normalized source and hashes are in
season12_weekly_quest_data.py; the larger source report remains ignored. Week 1
grants with the owned Season12_Mission_Schedule, and week 2 has a ten-day offset.
Using the explicitly configured LobbySeason12 timestamp as their calendar anchor
is local policy; no historical epoch or faction activation gate is fabricated.

The serialized once-per-match and single-match flags are now enforced using
trusted registration IDs supplied by the authenticated quest-event endpoint.
Per-objective deduplication and ignoring previously seen superseded matches are
explicit local policies. Context-free direct events cannot advance flagged
quests. Match-aware receipts preserve exact legacy retries and reject changed
new contexts; XP, tokens, completion bookkeeping and receipts commit together.
The native gameplay emitter still must qualify the source stat filters and send
events. Faction choice, its gate and variant consumption remain incomplete.
