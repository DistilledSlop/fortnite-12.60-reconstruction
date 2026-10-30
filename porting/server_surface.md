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

`server/probe_server.py` is the first local server-side tool. It records request
metadata and can return observed response fixtures, so endpoint behavior can
be developed against a running client. It currently provides no login,
matchmaking, XMPP, or game-session implementation. See its README for usage.

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
pure HTTP mock backend cannot satisfy. This was not resolved; doing so would
require reverse-engineering and altering the shipping binary's own
entitlement/anti-tamper logic, which is out of scope for this effort.
