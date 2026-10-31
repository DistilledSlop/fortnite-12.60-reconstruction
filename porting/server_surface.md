# Client service surface observed so far

This is an inventory of the 12.60 client, not a claim that a local server is
implemented or that every endpoint is known.

## Sources

- The cooked `DefaultEngine.ini` from `pakChunkEarly-WindowsClient.pak` is
  extracted to `porting/generated/DefaultEngine.ini` (ignored by Git).
- `inspect_client_routes.py` found 59 literal API path templates in the Win64
  client: 19 under `/api/game/v2/`, 34 under `/api/public/`, and 6 under
  `/api/oauth/`. Runtime-generated routes are not represented.

## Named subsystems

The config identifies MCP account, identity, party, profile, matchmaking,
session, XMPP, and game-service settings. It includes a `GameServiceMcp
Localhost` profile. The client binary also contains matchmaking ticket and
session API templates. These are leads for protocol reconstruction; the config
does not define response schemas or the game-session simulation.

## Server implementation order

1. Document the exact local startup sequence and each network request made by
   a runnable client. Static strings alone cannot establish ordering.
2. Implement local identity and profile responses needed to reach the lobby.
3. Implement party, matchmaking ticket, and session discovery responses needed
   to join a locally hosted session.
4. Implement the actual UE game-session protocol and authoritative gameplay
   state. HTTP route emulation alone will not put a player into a game.

The new native client project should use local service endpoints and omit EAC
integration. The supplied Windows client has not been modified.

`server/probe_server.py` is the local HTTP server. It records request metadata and
can return observed response fixtures. With `--stub` it serves the local service layer
described in "Local service layer" below. It provides no XMPP or game-session
implementation. See `server/README.md` for usage.

The install has a main `FortniteClient-Win64-Shipping.exe` and separate EAC and
BattlEye launcher executables. A bounded Wine probe of the main executable
with `-nullrhi -nosound -NoSplash -unattended -log` and an isolated ignored
Wine prefix exited with code 1. Its console output showed an engine exit and
shutdown of `LauncherPlatform`, `LauncherCheck`, and `Solaris`, without a
request trace. This does not establish a working client startup sequence or
anti-cheat removal from the original binary. The new native project remains
the path where EAC is absent by construction.

The new `FortniteLinuxServer` target now provides a native UE dedicated server
for the reconstructed project. A local new-client/new-server probe completed
UE login and join on `FortniteEmptyDedicated`; this does not supply Fortnite's
service or gameplay protocols. See `server/README.md`.

## Original client against the local mock backend

`run_original_client.sh [seconds] [client args]` runs the unmodified Win64
shipping exe under Wine (EAC and BattlEye off: `-nobe -noeac -fromfl=none`) in
a private network namespace (`sudo -n unshare -n -m`). Inside it `generated/hosts`
is bind-mounted over `/etc/hosts` so every Epic hostname resolves to loopback,
and `server/probe_server.py --stub` serves TLS on 127.0.0.1:443 with
`server/fortnite_stub.py` answering endpoints. Nothing leaves the namespace and
the host's port 443 is untouched. Requests are logged (method, path, query keys,
form keys, body hash; no values) to `generated/server-requests.jsonl`.

TLS: libcurl failed with error 60 because the shipping build compiles out the
`n.VerifyPeer` override (it is ignored in config, `[SystemSettings]`,
`[ConsoleVariables]`, and with a loose `Content/Certificates/cacert.pem`).
`server/patch_curl_verify.py` clears `FCurlHttpManager::CurlRequestOptions.bVerifyPeer`
in the running process through `/proc/<pid>/mem` once the log shows the options were
initialised. No file of the install is changed.

Login: the client does not log in by itself. Passing
`-AUTH_LOGIN=unused -AUTH_PASSWORD=<any> -AUTH_TYPE=exchangecode` starts auto-login,
which posts `grant_type=exchange_code` to the mock OAuth endpoint.

### Requests observed, in order

1. `POST /datarouter/api/v1/public/data` (telemetry, repeated; 204)
2. `POST /account/api/oauth/token` grant `client_credentials` (200, client token)
3. `GET /fortnite/api/v2/versioncheck/Windows` (200 `{"type":"NO_UPDATE"}`)
4. `GET /fortnite/api/cloudstorage/system` (200 `[]`)
5. `POST /account/api/oauth/token` grant `exchange_code` (200, user token with `account_id`, `displayName`)
6. `DELETE /account/api/oauth/sessions/kill` (204)
7. `GET /account/api/public/account/{id}` and `.../externalAuths`
8. `GET /account/api/public/account?accountId=` (200 array), `POST /api/v1/user/setting`
9. `POST /fortnite/api/game/v2/tryPlayOnPlatform/account/{id}` (200 text `true`)
10. `GET /lightswitch/api/service/bulk/status` (200 array, status UP)
11. `GET /fortnite/api/game/v2/enabled_features`, `POST .../profileToken/verify/{id}` (204)
12. `GET /fortnite/api/cloudstorage/user/{id}`, `GET /content-controls/{id}` (404 accepted)
13. `POST /fortnite/api/game/v2/profile/{id}/client/QueryProfile?profileId=&rvn=` (twice; `fullProfileUpdate`)
14. `GET /fortnite/api/storefront/v2/keychain?numKeysDownloaded=` (must be a JSON array of
    `guid:base64key:path` strings; an empty array fails with UnableToParseJson)
15. `DELETE /account/api/oauth/sessions/kill/{token}` on a failed login

### Further requests observed once the frontend stub is fuller

With friends/presence/party/calendar/catalog/content endpoints stubbed (see
`server/fortnite_stub.py`), login proceeds through `CheckServiceAvailability`,
`CheckEntitledToPlay`, `VerifyAgeGateRequirements`, `DownloadingClientSettings`,
`QueryProfile`, `QueryKeychain`, `RedeemOfflinePurchases`, `CheckingRejoin`,
`DoQosPingTests`, `StartVoiceChat`, `CreatingParty` (which needs XMPP; see
below), and `Completed`. The client then reaches the Frontend map and starts
its own CMS/friends/presence/catalog refresh pass (`content/api/pages/fortnite-game`,
`fortnite/api/calendar/v1/timeline`, `fortnite/api/storefront/v2/catalog`,
`friends/api/v1/*`, `presence/api/v1/*`, `fortnite/api/game/v2/privacy/account/*`).

### XMPP / WebSocket TLS

The client's websocket stack (libwebsockets via `FLwsWebSocket`) verifies TLS
independently of libcurl and independently of `n.VerifyPeer`: it reads
`[WebSockets.LibWebSockets] bDisableCertValidation` (manager-level extra-certs
callback) and, more specifically, `[LwsWebSocket] bDisableCertValidation` (the
per-connection `LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION`
handler in `Private/Lws/LwsWebSocket.cpp`). Setting both `true` in the loose
`Saved/Config/WindowsClient/Engine.ini` did not stop the handshake failure in
this build (`Lws(Error): SSL error: self signed certificate`), so XMPP login
still fails locally; `CreatingParty` eventually times out and treats party
creation as succeeded anyway (the login flow does not hard-block on it).

### Current blocker: AppES / entitlement gate after `CheckEntitledToPlay`

After `Checking entitlements... (CheckEntitledToPlay)` the client always logs
`LogAppES: Display: closing code 1` and shortly after shows a fatal UI dialog
("Fortnite was not started correctly and needs to be closed. We recommend
trying to launch the game using the Epic Games Launcher or contact player
support.") regardless of the mock backend's responses at that stage, of
`-epicenv`, and of `-EpicPortal`/`-NOEPICPORTAL`. `LogAppES` and the dialog
string are both literal strings in the shipping exe; the client binary
contains `[UOnlineAccountCommon::LoginStep_CheckEntitledToPlay] IsDevelopment(),
skipping mcp entitlement check`, implying there is a non-HTTP, binary-level
entitlement/launcher-origin check (likely tied to the Epic Games Launcher
provenance / "AppES" app-entitlement-service client component, possibly
checking how the process was launched rather than calling out to a server) that a
pure HTTP mock backend cannot satisfy. This was not resolved; the gate is not
yet bypassed. Passing it would require reverse-engineering and altering the
shipping binary's own entitlement/anti-tamper logic.

## Local service layer (implemented in `porting/server/`)

The stand-in covers the account, OAuth, MCP profile, party, presence, lightswitch,
cloud storage, storefront, calendar, chat and matchmaking routes. Each route now has an evidence
class from `porting/server/EVIDENCE.md` (generated table, `endpoint_evidence.json`). The classes are
`wine_log` (an earlier Wine run), `reflection` (a reflected struct), `binary` (route format in the exe
and code that loads it), `asset` (cooked config), `binary_string_only` (in the exe, no code reference)
and `convention` (public naming, not seen locally). `server/README.md` has the route table. The tests are
`python3 -I server/test_services.py` (70 pass, including the schema checks).

### Evidence-based (observed in the 12.60 binary, the cooked config, reflection, or the earlier login capture)

- OAuth: `POST /account/api/oauth/token` (client_credentials and exchange_code), session kill (both forms).
  Class `wine_log` (verbs and order) and `binary` (`/api/oauth/token`, `/api/oauth/sessions/kill`).
- Accounts: `GET /account/api/public/account/{id}`, the `accountId=` list, `externalAuths`,
  `tryPlayOnPlatform`, `profileToken/verify`, `/api/v1/user/setting`. Class `wine_log`, with `binary`
  for each template.
- MCP: `POST /fortnite/api/game/v2/profile/{id}/{client|dedicated_server|public}/{command}`. The
  template is in `DefaultEngine.ini` (class `asset`). The response is checked against
  `FProfileUpdateSingle` (class `reflection`, the tracked headers). The command names are in the
  binary. The envelope names are in the binary in PascalCase only (`ProfileRevision`, `MultiUpdate`,
  `LockExpiration`, ...); the camelCase wire form follows the engine's `StandardizeCase` rule.
- Party v1: the route formats in the binary (`/api/v1/{namespace}/parties...`, members, invites,
  pings, user, notifications count). Class `binary`; the `{namespace}` is `Fortnite` from config. Verbs
  are not confirmed. The party error codes are in the binary.
- Presence: `/presence/api/v1/_/{id}/last-online`, `subscriptions`, `settings/subscriptions`, `nudged`.
  Class `binary`.
- Lightswitch: `/api/service/bulk/status` (class `wine_log` and `binary`), with the field names
  `serviceInstanceId`, `status`, `allowedActions`, `banned`. The single-service route is `convention`.
- Version check `/api/v2/versioncheck/{platform}` (`wine_log`, `binary`) with answer `NO_UPDATE`.
- `GET /fortnite/api/game/v2/enabled_features` (`wine_log`, `binary`).
- Calendar `GET /api/calendar/v1/timeline` (`binary`, `reflection`: CalendarDownload and EventsTimeline).
- Storefront `GET /api/storefront/v2/catalog` (`binary`, `asset` for `ListCatalogEntriesUrl`, `reflection`:
  CatalogDownload).
- Keychain `GET /api/storefront/v2/keychain` (`wine_log`: an array of `guid:base64key:path` strings).
  The literal has no code reference, so `binary_string_only`.
- Chat `POST /api/game/v2/chat/{accountId}/reserveGeneralChatRooms/{subgame}/{platform}` (`binary` route,
  `reflection` GeneralChatReturn; the verb is `convention`).
- Cloud storage: system and user paths (`binary`), and the field names `uniqueFilename`, `filename`,
  `hash256`, `length`, `contentType`. The user list is `wine_log`.
- Datarouter `POST /datarouter/api/v1/public/data` (`wine_log` 204; the literal has `?SessionID=` and no code
  reference, so `binary_string_only`).
- Matchmaking: the ticket, session and key paths (`binary`, one loading function each), and the websocket
  states (`convention` and the earlier implementation).

### Inferred or convention (path or name in the binary, shape or verb not confirmed)

- HTTP verbs for the party and presence routes not in the login capture, and the request bodies. These
  stay `convention`.
- Invite and ping shapes, `expires_at` semantics (informational only), the build-id key
  `urn:epic:cfg:build-id_s`, and the CLOSED joinability value.
- Profile command body keys (for example `newPlatform`, `itemIds`, `lockerItem`, `slotIndex`). The
  literals are not in the binary as standalone strings, so these are `convention`.
- `GET /account/api/oauth/exchange` and `verify` (`binary`), and the `expiresInSeconds` and
  `creatingClientId` names (`creatingClientId` is a literal in the exe; the rest of the shape is not).
- The lightswitch status values UP, DOWN and MAINTENANCE (`convention`).
- Token lifetimes (8 h access, 32 h refresh), which are the earlier stub's values (`convention`).

### Guess (no evidence beyond the earlier stub)

- `EquipBattleRoyaleCustomization` and `SetBattleRoyaleBanner`. These names are not in the 12.60 binary
  strings, so the client may never send them.
- `PATCH .../parties/{id}/meta`: the `/parties/{partyId}/meta` literal is not in the binary (the member
  meta route is).
- The keychain placeholder (32 zero bytes, not a key) and the session key field `key` (see EVIDENCE.md).
- The content page `/content/api/pages/fortnite-game`: no string in the exe. Seen only in the refresh pass
  of the earlier Wine run (`wine_log`, verb not recorded). Its body is a guess.
- Client id `3446cd72694c4a4485d81b77adbb2141`, which the earlier stub used and the binary does not show.
- The friends, social ban, privacy, receipts, billing, storeaccess and presence-nudged answers in
  `frontend_stubs.py`. Their route formats are in the binary or the refresh pass, but the bodies are not.

### Remaining gaps

- No capture of a driven client has checked any of these routes. The client still stops
  at the frontend login gate (`direct_connect_results.md`). Next step: a capture that
  records headers and bodies once the gate is passed through an approved route.
- XMPP, the notification stream and party chat. `notifications` is always empty.
- Party and lobby beacons (`BeaconPort`) and the game-session protocol.
- Join requests (the applicant flow), invite and ping expiry, and voice conferences.
- Catalog purchase, gifting, campaign and hero loadouts, homebase, receipts, and offers.
- Device and external auth, and the numeric error codes beyond the earlier 1004.
- Cloud storage quotas, and a real system file set (none is served by default).

