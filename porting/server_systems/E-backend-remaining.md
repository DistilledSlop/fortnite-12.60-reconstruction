# E: backend work that remains, with designs

Date: 2026-11-11. Base: `porting/server/README.md` ("Not done"), the gate
results, and the server cards D2–D6. Protocol shapes for XMPP and party
notifications below are from my knowledge of the public Epic protocol of that
period, not from a capture of this client. Each has a check listed; do the
check before building on it.

Rule for all backend work (unchanged): every field carries an evidence label
(`observed`, `inferred`, `guess`) as the existing code does.

## E1. Game-server link (needed for G5–G9) — design in D3 card §4

Four local calls: register, state, player info, result. Also:

- **Network encryption key.** The backend's session-key route gives the client
  a Base64 key; the server must hold the same key. Today they only agree when
  both read `FORTNITE_NET_ENCRYPTION_KEY`. Design: `register` response carries
  the key for the session; the server installs it when the client's Hello
  arrives with an `EncryptionToken` (stock UE flow:
  `FNetDelegates::OnReceivedNetworkEncryptionToken` → set key →
  `NMT_EncryptionAck`). Check in the G3/G4 logged run whether the vanilla
  client sends an EncryptionToken at all. If it does not, leave encryption off.
- **Cosmetics.** `player/<accountId>` returns the athena loadout (character,
  backpack, pickaxe, glider, contrail, dances, wraps) as template ids. The
  server resolves them to item definitions and writes
  `AFortPlayerStateAthena::CosmeticLoadoutPC` equivalents on the controller
  and pawn (`ApplyCosmeticLoadout` path; check the sweep for the function).

## E2. Match results and progression (G9)

On `result`: add XP to `athena` profile (`xp`, `level`, `book_xp`,
`book_level`), update `season_match_boost`-free fields only, bump the profile
revision, and record match stats. XP amounts: kills/placement XP use
`AthenaAccoladeXP` rows (D4 card); level thresholds come from the season XP
curve table (`/Game/Athena/Balance/DataTables/AthenaSeasonalXP*`, read with the
asset tools). The client refreshes with `QueryProfile` when it returns to the
lobby, so no push is needed. The Battle Pass reward grant on level-up already
exists (`season12_rewards.py`).

Stats screen ("Career"): served by `statsproxy` routes
(`/statsproxy/api/statsv2/account/<id>`); return `{}`-style empty stats first,
then real counters from recorded results.

## E3. XMPP (needed for G10 parties across clients, friends presence)

Solo play does not need it (G0–G2 passed without it). Design:

- Transport: websocket, subprotocol `xmpp`, the host the client is configured
  with (`XmppServiceProd` in the client's ini; find the domain with
  `strx.py xmpp`). Reuse `local_ws.py`.
- Minimal stream: `<open>` → features (PLAIN) → `<auth>` (base64 of
  `\0accountId\0accessToken`; accept when the token is one of ours) →
  `<success>` → `<open>` → bind resource → session → client sends presence.
- Server pushes: `<message from="xmpp-admin@<domain>" to="<jid>"><body>{JSON}`
  where the JSON is a party notification
  (`com.epicgames.social.party.notification.v0.MEMBER_JOINED`,
  `MEMBER_LEFT`, `MEMBER_STATE_UPDATED`, `PARTY_UPDATED`, `PING`,
  `INITIAL_INVITE`, …) built from the party state the backend already keeps.
  Hook each mutation in `party_service.py` to emit one notification to every
  connected member.
- Presence: forward `<presence>` stanzas between friends and party members.
- MUC party chat: optional; answer joins to `Party-<id>@muc.<domain>` and
  relay group messages.
- Check: run the vanilla client with `-LogCmds="LogXmpp VeryVerbose,
  LogParty VeryVerbose"`; the log names every stanza it expects and rejects.

## E4. Friends

Routes exist as empty guesses. Design: a symmetric friends list in
`--state-dir`; `POST friends/<a>/<b>` creates a pending request or accepts a
reverse one; `DELETE` removes; summary route returns `friends`, `incoming`,
`outgoing`, `blocklist`. Push friend events over XMPP (E3) as
`com.epicgames.friends.core.apiobjects.Friend` messages. Needed only for
inviting a second local account (G10).

## E5. Party beacon and lobby

`BeaconPort_i` is advertised. Decide from the G3 run (D3 card §2): if the
client connects there, serve a stock `APartyBeaconHost` on the game server
that accepts all reservations. No backend work.

## E6. Removing the client patches (G11)

Two in-memory patches are applied today: curl TLS verification and the AppES
(EOS-style entitlement/"app environment service") gate.

- TLS: produce a certificate chain the client's pinned roots accept is not
  possible without Epic's CA. The realistic end state is: keep one
  certificate-store change outside the process (import our CA into the Wine
  prefix's root store) and confirm whether the client uses the system store or
  a pinned bundle (`strx.py "cacert"`, `strx.py "PinnedPublicKeys"`). If
  pinned in ini (`[SSL] PinnedPublicKeys`), an ini override in the user's
  `Saved/Config` directory removes the pin without patching code.
- AppES: read the gate function and find which response it wants; then serve
  that response from the backend. This is recoverable work (bucket R), not
  done yet: `patch_appes_diagnostic.py` names the patched site.
- wss for matchmaking (today plaintext ws): follows from the TLS answer.

## E7. Not needed for the vanilla-client goal

Real-money purchase flow (local credit purchase already exists), gifting,
Save the World commands, device auth, cloud storage quotas, numeric error
codes. Leave as they are.

## Order

E1 (with D3) → E2 → E6 investigation (cheap, read-only) → E3 → E4 → E5 only if
the run shows it is needed.
