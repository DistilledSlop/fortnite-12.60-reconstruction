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

The check is `schema_check.py` with the tests in `test_services.py` (`SchemaTests`).

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
5. **The MCP envelope names `serverTime`, `responseVersion` and `multiUpdate` are in no McpProfileSys header.**
   Their PascalCase names are in the binary. Their types are a convention (`ENVELOPE_FIELDS` in
   `schema_check.py`). The element type of `multiUpdate` is taken to be `FProfileUpdateSingle`, which is an
   inference.

## Open conflicts and risks

- **Key case.** The engine writes `ProfileRevision` as `profileRevision`, but its reader
  (`JsonObjectToUStruct`, `JsonObjectConverter.cpp` around line 799) looks up the exact PascalCase
  property name. So if the client parsed MCP responses with the stock reader, our camelCase keys would be
  ignored. The binary has the camelCase log text (`Missing new 'profileChangesBaseRevision' key`) and the
  standalone literal `profileId`, which point to camelCase on the wire. The parser was not located, so this
  is not proven.
- **Session key field.** The backend's `key` (`matchmaking.py`, `SESSION_KEY_FIELD`) is a guess. A search for
  it found no literal of four or more characters, and the scan's 3-character search found it only as a
  generic identifier in 8 functions. The exe does have the literal `EncryptionKey` (UTF-16), but it sits in a
  run of stat names. Not changed.
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
| `profile.command` | POST | `(config) McpClientCommandUrl=/api/game/v2/profile/`accountId/client/`fname?profileId=`profileId` | - | 0 | FProfileUpdateSingle (+ serverTime, responseVersion, multiUpdate) | wine_log, reflection, asset | yes |
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
| `matchmaking.session_key` | GET | `/api/game/v2/matchmaking/account/`accountId/session/`sessionId` | 0x1460dfbe0 | 1 | - | binary | yes |
<!-- END GENERATED TABLE (build_evidence.py) -->

## Open question: how the client reads response keys

Stock UE 4.25 `FJsonObjectConverter` writes keys with `StandardizeCase` (camelCase, JsonObjectConverter.cpp:265) but
`JsonObjectToUStruct` looks them up by the exact PascalCase property name (JsonObjectConverter.cpp:799), so the stock reader
cannot parse its own output. The shipped client therefore must read MCP responses with its own code. The exe carries
camelCase key literals and the backend serves camelCase (the publicly documented wire form), but the parsing function has not
been located. Next step: find it from the key-string load sites (`evidence_scan.py` already lists them) and confirm which
keys it requires and whether lookups are case-insensitive. Until then the response key spelling is `binary`/`convention`,
not confirmed by a parser.
