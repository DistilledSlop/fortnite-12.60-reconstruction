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
| `frontend_stubs.py` | Social ban, receipts, billing and other menu answers. Labelled guess. |
| `matchmaking.py` | Ticket, session and session-key routes. Websocket state machine (`local_ws.py` is the RFC 6455 side). |
| `fortnite_stub.py` | Compatibility facade for `probe_server.py --stub`. Keeps the earlier `respond()` and `websocket_route()` names. |
| `probe_server.py` | HTTP server. `--stub` serves the backend. Without it, it only records requests and serves fixtures. |
| `tests/` | HTTP regressions, including `SchemaTests` (reflection checks); additional `tests/test_*.py` modules cover persistence, transactions, social transport, quests and the game-server bridge. |
| `friends_service.py`, `xmpp_service.py` | Durable friends/requests/blocks and authenticated WebSocket XMPP social transport. |
| `gifting_service.py` | Atomic cross-account gifting and gift-box dismissal. |
| `gameserver_service.py`, `gameplay_service.py` | Trusted local game-server registration, loadouts and results; StatsV2 reads. |
| `progression_service.py`, `quest_service.py` | Durable match rewards, recovered XP tables and initial Season 12 quest/style activation. |
| `lobby_content.py` | Local lobby notices or an explicitly supplied CMS snapshot and images. |
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
python3 -m unittest discover -s porting/server/tests -p 'test_*.py'  # combined backend tests
python3 -I porting/server/tests/test_services.py                  # isolated HTTP tests
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

The original client's `OTHERS_ACCOUNT_CLIENT_SERVICE` session cleanup preserves its
current token and revokes other sessions for that local account. A Wine probe with
this behavior reached the `Completed` sign-in step without the previous invalid-token
responses. The original client still needs its launcher/security path resolved.

For isolated investigation only, `FN_PATCH_APPES_DIAGNOSTIC=1` on
`run_original_client.sh` changes the running 12.60 process after login begins,
redirecting an unsafe-environment notification handler to its cleanup path and
suppressing a separate timer callback observed requesting exit. Both changes are
signature checked, confined to the runner's network namespace and never written to
the executable. The callback's full purpose is unknown, and the patch suppresses
all its calls. It does not fix the underlying checks and is off by default. A
diagnostic probe completed local sign-in and rendered the Battle Royale locker
using DXVK and software Vulkan. On 2026-11-09 the client renamed a preset and
cleared an owned back bling; both changes reappeared after restarting the client
and backend. A subsequent equip request exposed a template-ID mismatch; after
the backend fix, the client equipped that back bling and the profile stored it.
A separately seeded Drift style was saved and restored after a client restart;
see [the style probe](../LOCKER_STYLE_PROBE.md). The full range of locker
operations remains unverified. Do not treat this diagnostic as an entitlement
pass or a working launcher.

For a client that supports the local grant, POST `/account/api/oauth/token` with form
`grant_type=local_username` and `username=<chosen name>`. The existing client exchange-code
transport can instead use `exchange_code=local-username:<chosen name>`; this is an
identity selector, not a password. For the original-client analysis harness, run
`porting/run_original_client.sh --username "Name"`; the original binary may still stop at
its own launcher entitlement gate. The rebuilt client has not yet been verified through
a full login-to-lobby flow.

The handwritten `ULocalAccountSubsystem` requests this grant, then loads the
authenticated full Athena profile. It publishes the account, token and validated
profile snapshot together; a failed selection preserves the previous account.
The original `UFortSignInWidget` now has a source adaptation that labels its first
input USERNAME, hides password and forgotten-password controls, and submits through
this subsystem on click or Enter. It uses `http://127.0.0.1:8080` by default;
`-LocalBackend=http://127.0.0.1:PORT` selects another service origin.
The original login-interactions panel selects its existing credentials child on
construction and activation, hides Back, and deactivates the child with its parent.
This bypasses the account-provider selection screens for the local username flow.
These native changes are build pending under the checkpoint cadence. The selected
snapshot is not yet connected to the game's online login or MCP profile manager,
and no lobby transition is implemented. The rebuilt launcher still joins directly;
the original frontend loading path remains blocked by cooked-material compatibility.
Original-client decompilation shows `UFortSignInWidget::StartSignIn` dispatching
email and password through a native delegate at widget offset `0x410`. The parent
login-interactions panel binds it to its credentials handler, which initiates
`UOnlineAccountCommon::AutoLogin`. The Fortnite override reaches a common
next-tick state machine and conditional waiting-room/identity-login steps. These
static traces do not establish successful local account initialization. See
`porting/behavior/account_state_callback_runtime/evidence.json` and
`porting/behavior/waiting_room_completion_runtime/evidence.json`. The local widget
adaptation still bypasses the original credential dispatch.

`CreateSelectedOnlineUserAccount` now materializes the selected data as an owned
`FUserOnlineAccount` snapshot for a future identity provider. It requires a valid
provider-supplied ID matching the selected account and exposes the original
consumer's `id`, display name, empty email and selected access token. The snapshot
does not change login status or broadcast completion; provider wiring, token
renewal, native build and runtime verification remain pending.

Typed login-step append and anchor-relative insertion now have native source
implementations, preserving duplicate entries and delegate cloning. Their
33 original state names now have direct literal-initializer receipts, including
`Started`, `LoginWaitingRoom`, `LoggingIn` and `Completed`. Account registration
wiring, build and runtime verification remain pending. See
`porting/behavior/login_step_registry_runtime/evidence.json`.

With `--stub`, `probe_server.py` persists account mappings, profile revisions and user
cloud files under ignored `porting/generated/local-backend-state` by default. Set
`--state-dir DIR` to choose a different local directory, or `--ephemeral` for fresh
in-memory state. Tokens, parties and matchmaking sessions remain process-local; obtain
a new token after restarting the service. Only one probe server can use a state
directory at a time; a second server exits instead of risking conflicting writes.
Code embedding `Backend` directly must likewise use a single owner per state directory.

The default loopback HTTP server advertises its actual local WebSocket address
and port in matchmaking tickets. For other bind addresses or a different public
address, set `--matchmaking-ws-origin` to the reachable `ws://` or `wss://` origin.
TLS probes without an explicit origin retain the original-client hostname and
require its host mapping and TLS setup.

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
| DELETE | `/account/api/oauth/sessions/kill/([^/]+)` | oauth.sessions.kill_token | observed |
| GET | `/account/api/public/account/([^/]+)` | account.by_id | observed |
| GET | `/account/api/public/account` | account.by_ids | observed |
| GET | `/account/api/public/account/displayName/([^/]+)` | account.by_display_name | inferred |
| GET | `/account/api/public/account/([^/]+)/externalAuths` | account.external_auths | observed |
| POST | `/api/v1/user/setting` | account.user_setting | observed |
| POST | `/fortnite/api/game/v2/tryPlayOnPlatform/account/[^/]+` | account.try_play_on_platform | observed |
| POST | `/fortnite/api/game/v2/profileToken/verify/([^/]+)` | account.profile_token_verify | observed |
| POST | `/fortnite/api/game/v2/profile/([^/]+)/(client\|dedicated_server\|public)/([A-Za-z]+)` | profile.command | observed |
| POST | `/party/api/v1/Fortnite/parties` | party.create | inferred |
| GET | `/party/api/v1/Fortnite/parties/([^/]+)` | party.get | inferred |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)/join` | party.member.join | inferred |
| DELETE | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)` | party.member.leave_or_kick | inferred |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/invites/([^/]+)` | party.invite | inferred |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/invites/([^/]+)/([^/]+)/decline` | party.invite.decline | inferred |
| PATCH | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)/meta` | party.member.meta | inferred |
| PATCH | `/party/api/v1/Fortnite/parties/([^/]+)/meta` | party.meta | guess |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)/promote` | party.member.promote | inferred |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)/(confirm\|reject)` | party.member.applicant | inferred |
| POST | `/party/api/v1/Fortnite/parties/([^/]+)/members/([^/]+)/disconnect` | party.member.disconnect | inferred |
| GET | `/party/api/v1/Fortnite/user/([^/]+)` | party.user | inferred |
| POST | `/party/api/v1/Fortnite/user/([^/]+)/pings/([^/]+)/parties` | party.ping | inferred |
| GET | `/presence/api/v1/_/([^/]+)/last-online` | presence.last_online | inferred |
| GET | `/presence/api/v1/_/([^/]+)/subscriptions` | presence.subscriptions.list | inferred |
| POST | `/presence/api/v1/_/([^/]+)/subscriptions/([^/]+)` | presence.subscriptions.add | inferred |
| DELETE | `/presence/api/v1/_/([^/]+)/subscriptions/([^/]+)` | presence.subscriptions.remove | inferred |
| GET | `/friends/api/v1/([^/]+)/summary` | friends.summary | inferred |
| GET | `/friends/api/v1/([^/]+)/settings` | friends.settings | inferred |
| GET | `/friends/api/v1/([^/]+)/recent/[^/]+` | friends.recent | inferred |
| GET | `/friends/api/v1/([^/]+)/blocklist` | friends.blocklist | inferred |
| GET | `/friends/api/public/friends/([^/]+)` | friends.legacy | inferred |
| GET | `/friends/api/public/blocklist/([^/]+)` | friends.legacy_blocklist | inferred |
| POST | `/friends/api/public/friends/([^/]+)/([^/]+)` | friends.request | inferred |
| DELETE | `/friends/api/public/friends/([^/]+)/([^/]+)` | friends.delete | inferred |
| POST | `/friends/api/public/blocklist/([^/]+)/([^/]+)` | friends.block | inferred |
| DELETE | `/friends/api/public/blocklist/([^/]+)/([^/]+)` | friends.unblock | inferred |
| POST | `/friends/api/v1/([^/]+)/blocklist/([^/]+)` | friends.block_v1 | inferred |
| DELETE | `/friends/api/v1/([^/]+)/blocklist/([^/]+)` | friends.unblock_v1 | inferred |
| GET | `/lightswitch/api/service/bulk/status` | lightswitch.bulk | observed |
| GET | `/lightswitch/api/service/([^/]+)/status` | lightswitch.single | inferred |
| GET | `/fortnite/api/v2/versioncheck/[^/]+` | versioncheck | observed |
| GET | `/fortnite/api/game/v2/enabled_features` | enabled_features | observed |
| GET | `/fortnite/api/calendar/v1/timeline` | calendar.timeline | guess |
| GET | `/fortnite/api/storefront/v2/catalog` | storefront.catalog | guess |
| GET | `/fortnite/api/storefront/v2/keychain` | storefront.keychain | guess |
| GET | `/content/api/pages/fortnite-game` | content.page | guess |
| GET | `/content/api/assets/(.+)` | content.asset | guess |
| POST | `/datarouter/api/v1/public/data` | datarouter | observed |
| GET | `/fortnite/api/cloudstorage/system` | cloudstorage.system.list | observed |
| GET | `/fortnite/api/cloudstorage/system/([^/]+)` | cloudstorage.system.file | observed |
| GET | `/fortnite/api/cloudstorage/user/([^/]+)` | cloudstorage.user.list | observed |
| GET | `/fortnite/api/cloudstorage/user/([^/]+)/([^/]+)` | cloudstorage.user.file | observed |
| PUT | `/fortnite/api/cloudstorage/user/([^/]+)/([^/]+)` | cloudstorage.user.put | observed |
| GET | `/socialban/api/public/v1/[^/]+` | socialban | guess |
| GET | `/party/api/v1/Fortnite/user/[^/]+/notifications/undelivered/count` | party.notifications_count | guess |
| GET | `/fortnite/api/game/v2/privacy/account/[^/]+` | privacy.account | guess |
| GET | `/fortnite/api/game/v2/clientfeaturekeys/[^/]+` | clientfeaturekeys | guess |
| GET | `/fortnite/api/receipts/v1/account/[^/]+/receipts` | receipts | guess |
| GET | `/api/public/payment/accounts/[^/]+/billingaccounts` | billing.accounts | guess |
| POST | `/fortnite/api/storeaccess/v1/request_access/[^/]+` | storeaccess | guess |
| GET | `/presence/api/v1/_/[^/]+/settings/subscriptions` | presence.settings | guess |
| GET | `/presence/api/v1/Fortnite/[^/]+/subscriptions/nudged` | presence.nudged | guess |
| GET | `/api/v1/Fortnite/get` | legacy.get | guess |
| POST | `(?:/fortnite)?/api/game/v2/chat/([^/]+)/reserveGeneralChatRooms/([^/]+)/([^/]+)` | chat.reserve_general_rooms | inferred |
| GET | `(?:/fortnite)?/api/game/v2/matchmakingservice/ticket/player/([^/]+)` | matchmaking.ticket | observed |
| GET | `(?:/fortnite)?/api/matchmaking/session/([^/]+)` | matchmaking.session | observed |
| POST | `(?:/fortnite)?/api/matchmaking/session/([^/]+)/join` | matchmaking.join | observed |
| GET | `(?:/fortnite)?/api/game/v2/matchmaking/account/([^/]+)/session/([^/]+)` | matchmaking.session_key | observed |
| POST | `/local/gameserver/register` | gameserver.register | guess |
| POST | `/local/gameserver/([^/]+)/state` | gameserver.state | guess |
| GET | `/local/gameserver/([^/]+)/player/([^/]+)` | gameserver.player | guess |
| POST | `/local/gameserver/([^/]+)/result` | gameserver.result | guess |
| GET | `/statsproxy/api/statsv2/account/([^/]+)` | stats.account | inferred |

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

The `local-username:` exchange-code prefix selects a named local account. A code
issued by `GET /account/api/oauth/exchange` uses the `local-exchange:` prefix, is
single use, and maps to its own account. Unknown, expired and used codes are
rejected rather than selecting a different account. This is a local policy, not Epic behaviour.

### Local item shop and currency

The local catalog reconstructs the May 20, 2020 UTC release-day rotation from
[the dated FNBR archive](https://fnbr.co/shop/may-20-2020): nine Featured and six
Daily offers, with archived V-Bucks prices and included outfit back blings.
Offer IDs and catalog layout fields are local reconstructions. Template and
display asset names exist in the 12.60 asset index. Lace, Echo and Echo’s back
bling receive their two initially unlocked Material styles from the cooked
ItemVariants definitions. Purchase-to-equip and gifting persistence have backend
tests; their new shop display and style flows still require client verification. This fixed rotation
does not advance with the current date. PurchaseCatalogEntry
checks the catalog price and owned items, deducts the local balance, and grants the
cosmetic to the Athena profile in one transaction. Catalog entries, purchase payload,
and response semantics are partly inferred. Original-client cosmetic and Battle
Pass purchases have been observed. Season 12 pass purchases and level skips grant
reached rewards from the cooked season schedule. The two immediate challenge
bundles and their initial quests now activate; earned Agent Peely style tokens
are consumed into owned variants. Later timed quests and daily rotation remain
incomplete, and these new quest shapes require original-client verification. A level skip has also been verified in the
original client. The V-Bucks storefront offers a repeatable free pack of 1,000
local test V-Bucks. No payment provider is connected and real-money purchases
are not implemented.

For a profile that purchased the pass before reward grants were implemented, stop
the backend and run `python3 porting/server/repair_season12.py /path/to/state ACCOUNT_ID`.
The repair adds missing level-one rewards without replacing owned items.

With the server stopped, credit an account in its persistent state directory:

```sh
python3 porting/server/local_currency.py --state-dir /path/to/state --username Player --amount 2000
```

The tool refuses to change state while the backend holds its state-directory lock.

### MCP profile commands

`profile_service.py` implements the command set below. `QueryProfile` is handled
separately: it always returns a `fullProfileUpdate`, which matches the observed
login capture. The table is generated by `backend.py --commands`.

| Command | Profiles | Name evidence | Body |
| --- | --- | --- | --- |
| QueryProfile | any | observed | none |
| ClientQuestLogin | athena | observed | no body. Once per UTC day: advances the daily login counter. |
| CopyCosmeticLoadout | athena | observed | sourceIndex, targetIndex, optNewNameForTarget (inferred). |
| DeleteCosmeticLoadout | athena | observed | index and fallbackLoadoutIndex (reflected names; JSON casing inferred). Deletion policy is local. |
| EquipBattleRoyaleCustomization | athena | guess | slotName, itemToSlot, indexWithinSlot (inferred). Targets the last applied loadout. |
| MarkItemSeen | athena, common_core | observed | itemIds: list of item ids (inferred). |
| SetActiveLoadoutIndex | athena | observed | index (reflected name; JSON casing inferred). Selection policy is local. |
| SetAffiliateName | common_core | observed | affiliateName (inferred). No affiliate check. |
| SetBattleRoyaleBanner | athena | guess | bannerIconTemplate and bannerColorTemplate (inferred). Mirrors to common_public (multiUpdate). |
| SetCosmeticLockerBanner | athena | observed | lockerItem, bannerIconTemplateName, bannerColorTemplateName (reflected; JSON casing inferred). |
| SetCosmeticLockerName | athena | observed | lockerItem and name (inferred). |
| SetCosmeticLockerSlot | athena | observed | lockerItem, category, itemToSlot, slotIndex and variantUpdates. ItemWrap uses seven slots (0–6); the observed index -1 applies the item or clear to all seven. Styles are validated against the owned cosmetic's stored variants; client-supplied owned options are ignored. Locker and item update semantics are a local policy. |
| SetCosmeticLockerSlots | athena | observed | lockerItem and loadoutData (reflected names; JSON casing inferred). |
| SetItemFavoriteStatus | athena | observed | targetItemId and bFavorite (inferred). |
| SetItemFavoriteStatusBatch | athena | observed | itemIds and itemFavStatus, equal-length lists (inferred). |
| SetMtxPlatform | common_core | observed | newPlatform: one of the platform names (inferred). |
| SetReceiveGiftsEnabled | common_core | observed | bReceiveGifts: boolean (inferred). |
| PurchaseCatalogEntry | common_core | observed | offerId, purchaseQuantity, currency, currencySubType and expectedTotalPrice (inferred). |
| GiftCatalogEntry | common_core | observed | Validates catalog price, accepted friendship, receive-gifts setting and ownership; atomic giver debit and recipient grants. Request and gift-box JSON are inferred. |
| RemoveGiftBox | common_core | observed | giftBoxItemIds; removes owned wrappers without granting the contents twice. |
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
- `notifications` includes inferred `CatalogPurchase` reward presentation after purchases. Other commands generally return an empty list; live social updates use XMPP.

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
When the native server uses `FORTNITE_NET_ENCRYPTION_KEY` or
`FORTNITE_NET_ENCRYPTION_KEY_FILE`, give the backend the same environment setting:
it then returns that key to session players. Without either setting the backend
generates a per-session key, which is not automatically shared with the native server.
An encrypted end-to-end join through matchmaking remains unverified.
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
- **Observed in the client:** the default `buildUniqueId` is the compatible changelist `12896485`; the network checksum `683914758` is a separate value. Session record fields are partly inferred.
- **Guessed, unverified:**
  - The HTTP methods of the party and presence routes, the request body shapes, the
    response shapes beyond the observed field names, and the nesting of party objects.
  - The matchmaking ticket payload, status values and sequencing, session record values,
    and server key lifecycle (see the `[guess]` comments in `matchmaking.py`).
  - The profile command body keys and notification contents; the multiUpdate element type is reflected.
  - The lightswitch status values UP, DOWN, MAINTENANCE. The binary has none of them exactly.
  - Client id `3446cd72694c4a4485d81b77adbb2141` and device id (32 zeros). Carried from the earlier stub.
  - The keychain entry, which is 32 zero bytes as a placeholder and not a key.

## Remaining acceptance work

- **Original-client G1 verification.** Login, locker edits, favorites, wraps, seeded
  styles, shop purchases, Battle Pass rewards and local V-Bucks have probe evidence
  in `porting/LOCKER_*_PROBE.md` and `porting/PURCHASE_PROBE.md`. The newly added
  friends, gifting, XMPP, content, quests and progression paths have backend tests;
  they have not yet passed their affected original-client UI flows.
- **Social compatibility.** XMPP supports SASL PLAIN using local OAuth sessions,
  resource binding, roster reads, presence, ping and queued JSON notifications.
  Notification payloads remain inferred. Party-only MUC chat uses `<party-id>@muc.<stream-domain>` as a local convention.
  Its room naming and reconnect behavior require client validation. Friends settings are still fixed to public; recent
  players and suggestions remain empty. Parties are process-local.
- **Quest coverage.** Initial Maya and Deadpool quests and Agent Peely style tokens
  are implemented from cooked definitions. Timed unlocks, later quest chains,
  daily rotation and bundle completion rules remain incomplete. Trusted quest
  progress is an internal API; native gameplay event integration remains pending.
- **Native integration.** The local registration/loadout/result protocol is tested
  over HTTP and WebSocket, including durable result retries. The native server
  must call it and supply validated gameplay results. Beacon and UE netcode work
  belongs to the concurrent native-server reconstruction.
- **Content parity.** The default lobby notice is locally authored. A supplied
  CMS snapshot and images can be served; historical news/calendar rotations and
  all affected client screens remain unverified.
- **Commerce and campaign.** Local cosmetic gifting is implemented. Real payments,
  receipts, campaign loadouts and homebase remain unimplemented.
- **Device and external auth.** Local username, client credentials, exchange code
  and refresh token flows exist; external providers and device auth do not.
- **TLS.** HTTP client probes currently use the documented diagnostic TLS patch.
  Original-client WSS certificate trust remains a blocker for live XMPP validation.
- **Error codes and storage policy.** Most numeric error codes, storage quotas and
  retention semantics have not been recovered.

### New local service configuration

`FORTNITE_GAMESERVER_SECRET` enables authenticated `/local/gameserver/*` routes
and registered-server matchmaking. Keep its value out of logs and command lines.
`--registered-matchmaking` also selects that matchmaking mode explicitly; it waits
for a live, compatible server rather than using the legacy diagnostic session.
Server registration requires `address`, `port`, `playlists`, `maxPlayers` and
`buildUniqueId`. The returned session ID and encryption key are process-local.
Send a state heartbeat at least every 30 seconds with `phase`, `numPlayers` and
`joinable`; optionally supply authoritative `connectedPlayerIds`. A newly
registered server is not joinable until its state enables it.

A trusted server can read `GET /local/gameserver/{session}/player/{account}` and
submit `POST /local/gameserver/{session}/result`. Results require `accountId`,
`placement`, `kills`, `endedAt` and either an explicit `xp` total or recovered
`accolades` row counts; optional `bookXp` defaults to the XP total. Exact retries
return the durable receipt without awarding twice; changed retries conflict.
Clients may read authenticated `/statsproxy/api/statsv2/account/{account}` but
cannot award XP. These `/local` contracts are ours, rather than recovered Epic APIs.

`--content-dir DIR` loads `DIR/content.json`; `--content-file FILE` instead selects
a snapshot directly. Images referenced as `local-asset:filename.png` or `.jpg`
resolve beside that file. Set `--content-public-origin https://HOST` for absolute
image URLs. Invalid configuration fails at startup. No supplied snapshot means
a clearly local notice, rather than a fabricated historical news rotation.

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
