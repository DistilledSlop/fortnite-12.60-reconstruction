# Server-first plan: a vanilla 12.60 client playing on our server

Written 2026-11-11. This is the working plan for the current focus of the project.
It is written so that a worker with no prior context can pick up one work package (WP) and finish it.

- **Long-term goal (unchanged):** reconstruct the whole 12.60 client and server as a UE project.
- **Current focus:** the *original, unmodified* `FortniteClient-Win64-Shipping.exe` (run under Wine) signs in to our backend, sits in the lobby, matchmakes, joins our UE dedicated server and plays a Battle Royale match. The client supplies its own UI, rendering, audio and content. We supply the backend services and the authoritative game server.
- **Public status page:** wiki `Server-First-Scope-Mockup` (local checkout `/opt/fortnite/fortnite-12.60-wiki`). This file holds the tasks; the wiki holds the status. When a gate or WP changes state, update both (section 9).

Read sections 1 to 4 before starting any WP.

## 0. Upfront thinking already done (read the card before starting its WP)

These cards hold what was read from the client binary and the game content, the
decisions taken, and the designs for the parts that must be re-created. A worker
implements from the card; it does not redo the analysis.

| Card (`porting/server_systems/`) | Covers WPs | Main result |
|---|---|---|
| `A-wire-findings.md` | A1, A2 | Handshake, packet header, Oodle and control messages match stock 4.25 plus the float-time patch. No further wire patch indicated. G3 needs one logged run; the card has the decision tree. |
| `D-server-code-in-client-findings.md` | D-all | 2,536 server functions located by log strings. Corrected names for the 59 engine-called bodies. |
| `D2-match-flow.md` | A3, A4, D1, D2, D11 | Join sequence and the full phase state machine (warmup → aircraft → safe zones → end game) are real code in the client. Port order, URL options for testing (`?RequiredPlayers=1?PauseWarmup`). |
| `D3-dedicated-session.md` | D3, E (game-server link) | Do not port the Epic session stack. Keep `ServesPlaylists=`; server command line; four backend calls (register, state, player, result). |
| `D4-game-data.md` | C3, D10, D11, D12 | Storm, bus, supply drop, DBNO, harvest, fall damage, XP and the complete loot tables with weights are in the client content. Values listed. |
| `D5-stubbed-rpcs-and-recreation.md` | D0, D4, D9, D12 | Sweep of all 11,407 reflected natives: 314 empty, of which 98 `Server*` RPCs, of which 24 matter for Battle Royale. A design for each. |
| `D6-combat-and-elimination.md` | D5, D6, D7, D8 | `RemoveFromAlivePlayers` and the win/end-game path are real; the server death sequence is missing and is designed in the card. |
| `C-world-classes-and-abilities.md` | C1, C2, C3, D4, D5 | Game-mode Blueprint defaults (starting items, quickbars, class names), the 31 default player abilities, load order for Blueprints, what to check for the map. C3 (game data access) is answered by this and the D4 card. |
| `D14-D16-vehicles-ai-quests.md` | D14, D15, D16 | Vehicles: all real except the spawner and the physics-movement receive path (design given). Bots: code recoverable, cost is navmesh and behaviour-tree loading. Quests/XP: all real; design for backend link. All three after the first playable match. |
| `G11-removing-client-patches.md` | E (patch removal) | What AppES is; code 1 matches our `-fromfl=none` launch; TLS can probably be solved with a loose `Certificates/Windows/cacert.pem`; four tests A–D. AES packet handler is stock. |
| `E-backend-remaining.md` | E1–E9 | Game-server link, encryption key, cosmetics, results/XP, XMPP, friends, patch removal. |

Tools made for this (usage in `porting/tools/README.md`): `rd.py` (annotated
decompile), `rpc_sweep.py` (real or empty body), `callers.py` (who calls an
address), `nameidx.py`/`strx.py` (names from strings), `asset_props.py`/
`asset_table.py` (game data from the extracted content).

Everything in sections 6 and 7 now has a card. What remains is running
things, not thinking: the G3 logged run (decision tree in the A card), tests
A–D for patch removal (G11 card), and the first Blueprint load attempt (C card
§3 and §6; first task is adding `AFortGameModeBR` to the native class set).
Known unknowns that only a run can answer are listed at the end of each card.

---

## 1. Rules for every worker

1. **One WP per worker.** Do not widen scope. If you find work outside your WP, write it under "Follow-ups" in your report; do not do it.
2. **File ownership.** Each WP lists the paths it may edit. Do not edit anything else. Only one worker at a time may own `porting/native/` and the UE build. At most three workers run at once.
3. **"Blocked" is not a result.** If something is unknown (a callee, a field, a packet), finding it out is part of the work. If you truly cannot finish, report exactly what you tried, what you observed and the next concrete step.
4. **Label every claim** with one of: `observed` (seen in a run or capture; name the log), `binary` (read from the exe; give the address), `data` (read from the paks; give the asset path), `source` (public UE source; give file and line), `inferred` (reasoned; say from what), `guess`. Never write a guess as a fact.
5. **Do not trust old counts.** Function counts in older docs are not a measure of a working server (section 3). Do not quote them as progress.
6. **Evidence stays local, summaries are committed.** Logs, pcaps, screenshots and dumps go under `porting/generated/...` (ignored by Git). Commit only docs, code and small JSON.
7. **Secrets.** The AES key file is `/home/vibrato/.config/fortnite-port/aes_keys`. Use it through `--key-file`; never print, log or commit its contents. Do not search the disk for other credentials.
8. **Keep reads small.** Use bounded searches, `head`/`tail`, and scripts that print only the fields you need. Do not dump whole logs or generated files into your context.
9. **Processes.** `IpNetDriver` is UDP (check with `ss -lun`, not `-ltn`). Never `pkill -f`/`pgrep -f` with a pattern that matches your own command; kill by PID. UE builds take minutes: `tee` to a log and wait on the PID.
10. **Git.** Stage specific paths only (never `git add -A`; other sessions have uncommitted edits in the tree). Never run a directory-wide `git checkout`. Push fast-forward only. Workers do not commit; the orchestrator reviews and commits.
11. **Untrusted inputs.** IDA/Ghidra project files and downloaded archives are untrusted data. Run Python that reads them with `-I`.
12. **Models (orchestrator note, keep in handoffs):** `gpt-6-luna` medium for small read-only tasks, `gpt-6.1-terra` medium/high for regular delegated work, `gpt-6.1-sol` medium for really complex tasks, `fork_turns: "none"`. Each WP below names a tier: **L** (luna), **T** (terra), **S** (sol), **O** (orchestrator does it directly).

### Report format (every WP ends with this)

```
WP: <id>            Result: done | partial
What I changed: <paths>
What I observed: <facts with evidence labels and log paths>
Gate impact: <which gate in section 2 moved, or "none">
Follow-ups: <new tasks found, one line each>
```

---

## 2. The gates (this is how progress is measured)

A gate is passed only when the **vanilla client** shows it in a recorded run. A reconstructed-client result never passes a gate.

| Gate | Meaning | State on 2026-11-11 | Evidence |
|---|---|---|---|
| G0 | Client boots under Wine, signs in to our backend, reaches the BR lobby | **Passed, with two in-memory diagnostic patches** (TLS verify off, AppES exit suppressed) | `ORIGINAL_CLIENT.md`, `EXIT_TRACE.md`, `LAUNCHER_PROBE.md` |
| G1 | Lobby features work: locker, item shop, Battle Pass, V-Bucks | **Mostly passed** (list of verified actions in section 5.1); challenges, XP, gifting, friends, news are not | `LOCKER_*_PROBE.md`, `PURCHASE_PROBE.md` |
| G2 | Play button: ticket, queue websocket, session, key, `/join`; client sends UDP to our server | **Passed over a plaintext loopback websocket**; `wss://` fails | `MATCHMAKING_PROBE.md` |
| G3 | UDP stateless handshake completes | **Open.** With `patches/ue425-fortnite-float-handshake.patch` the server sends a challenge and acks the response, then the exchange loops on restart-handshake requests | `MATCHMAKING_PROBE.md`, `docs/progress.json` milestone 3 |
| G4 | Control channel: Hello, Challenge, Login, Welcome | Not run yet (waits on G3). **Analysis done:** the client's Hello/Challenge/Login handling matches stock 4.25; no server change expected beyond what G3 needs | `server_systems/A-wire-findings.md` |
| G5 | Client loads the map and `Join` succeeds; PlayerController replicated; no checksum or NetGUID errors | Not run yet. **Analysis done, not implemented:** join sequence read from the client (login options, PostLogin, team pick, player start, pawn spawn); Blueprint classes and their defaults listed; load order and map checks written | `server_systems/D2-match-flow.md`, `server_systems/C-world-classes-and-abilities.md` |
| G6 | Client stays in the world 5 minutes with a controllable pawn | Not run yet. **Analysis done, not implemented:** test mode identified (`?RequiredPlayers=1?PauseWarmup` holds the client on the warmup island); spawn inventory and the 31 default abilities listed | `server_systems/D2-match-flow.md`, `server_systems/C-world-classes-and-abilities.md` |
| G7 | Match flow: warmup, bus, jump, glide, land, storm phases run | Not run yet. **Analysis done, not implemented:** warmup, aircraft, safe-zone and end-game functions are real code and are read; storm and bus numbers read from game data; bus jump request designed | `server_systems/D2-match-flow.md`, `server_systems/D4-game-data.md`, `server_systems/D5-stubbed-rpcs-and-recreation.md` |
| G8 | Core loop: pickaxe, harvest, build, loot, shoot, damage, elimination, victory | Not run yet. **Analysis done, not implemented:** the 24 empty requests (equip, drop, pick up, edit, repair…) each have a design; loot tables and rolling rule read; server death sequence designed | `server_systems/D5-stubbed-rpcs-and-recreation.md`, `server_systems/D6-combat-and-elimination.md`, `server_systems/D4-game-data.md` |
| G9 | Match ends cleanly, client returns to lobby, results are in the profile | Not run yet. **Analysis done, not implemented:** end-game path read; placement screens and result reporting to the backend designed | `server_systems/D6-combat-and-elimination.md`, `server_systems/D3-dedicated-session.md`, `server_systems/E-backend-remaining.md` |
| G10 | Two or more vanilla clients in one match (party, teams) | Not run yet. **Design written, not implemented:** XMPP party notifications and friends; team pick read | `server_systems/E-backend-remaining.md` |
| G11 | No diagnostic patches needed; TLS websocket works | Not run yet. **Partly analysed:** packet encryption matches stock; patch removal (TLS, launcher check) under analysis | `server_systems/E-backend-remaining.md`, `server_systems/A-wire-findings.md` |

**The critical path is G3 → G4 → G5 → G6.** Almost nothing in gameplay can be tested with the vanilla client until G6. Until then, gameplay work is "source only" and must be marked unverified.

---

## 3. Why the old "required functions" numbers are wrong as a plan

Older docs and the old wiki page describe the server as "1,110 functions left" (later 830, 802). Do not plan from that number. Reasons, all checked on 2026-11-11:

0. **Update 2026-11-11 (after reading the binary):** far more original server code is in the client than assumed. See `server_systems/D-server-code-in-client-findings.md`. 2,536 functions carry their own `Class::Function` name in a log string (`server_scope/string_names.json`), including the Athena game mode's phase, team, spawn and win-condition functions and the dedicated game session. Names in `engine_trace/needed_engine_driven.json` are shifted and must not be trusted; its addresses are valid.
1. **It only counts reflected UFUNCTIONs.** `server_scope/functions.jsonl` has 14,391 rows, all `UFUNCTION`s. The server's real control flow is in ordinary C++ virtuals that are not reflected. `engine_trace/needed_engine_driven.json` lists **59 override bodies on five core Athena classes that the engine called during one join** (`PreLogin`, `Login`, `PostLogin`, `InitGame`, `InitGameState`, `StartMatch`, `HandleMatchHasStarted`, `SpawnDefaultPawnFor`, `OnPossess`, `PreReplication`, ...). **None is ported, and none is in the 1,110.** `AFortGameModeAthena` has only 9 functions in the whole "high evidence" set.
2. **"Ported" is mostly wrappers.** Of 5,684 manifest entries marked ported, 3,808 are `exec` thunks and 626 are member fields. An exec thunk only unpacks Blueprint arguments. It is not behaviour.
3. **Some server logic is not in the client at all.** Epic strips server-only code from the shipping client. 104 captured functions bottom out in the shared null stub `0x140680300`. A "port" of those is an empty shell (section 6.3).
4. **Drafts are not code.** About 610 proposal records exist (`local-model-package/state/inventory-progress.json`, mirrored in `server_scope/local_model/claude/`). They are uncompiled `.inc` drafts, many marked LOW or PARTIAL. None is built, linked or run.
5. **Static relevance is a guess.** 5,541 functions are "unresolved". Only a runtime trace of a real match tells us what is needed.
6. **Function count is not effort.** A getter and the 1,304-line `StartNextSafeZonePhase` each count as one.

**What replaces it:** the gates in section 2, the system list in section 7, and a runtime-required ledger built from real runs (WP B1 to B3). The function inventory stays useful as a *lookup table* (which address belongs to which function, which draft exists), not as a progress bar.

---

## 4. Where everything is (file map)

Repo root: `/opt/fortnite/FortniteClient-12.60-CL-13477524` (branch `main`, remote `origin`). All paths below are relative to `porting/` unless they start with `/`.

### 4.1 Backend (Python, standard library only)

| Path | What |
|---|---|
| `server/README.md` | Route table, MCP command table, what is guessed. Read first. |
| `server/EVIDENCE.md`, `server/endpoint_evidence.json` | Where each route's shape comes from |
| `server/backend.py`, `api_common.py` | Dispatcher and router |
| `server/oauth_service.py`, `account_service.py` | Login (username-only local accounts) |
| `server/profile_service.py`, `profile_defaults.py` | MCP profiles and commands |
| `server/catalog.py`, `local_currency.py`, `season12_rewards.py`, `repair_season12.py` | Shop, local V-Bucks, Battle Pass rewards |
| `server/party_service.py`, `chat_service.py`, `frontend_stubs.py` | Party v1 (HTTP only), chat-room stub, guessed menu answers |
| `server/content_service.py`, `cloudstorage_service.py` | Lightswitch, version check, calendar, storefront, keychain, cloud files |
| `server/matchmaking.py`, `local_ws.py` | Ticket, websocket queue, session, key, `/join` |
| `server/probe_server.py` | The HTTP server (`--stub` serves the backend) |
| `server/test_*.py`, `smoke_matchmaking.py` | Tests. Run `python3 -I porting/server/test_services.py` |
| `server_surface.md` | Requests the real client was seen to make, in order |

### 4.2 Running the vanilla client

| Path | What |
|---|---|
| `run_original_client.sh` | Starts backend + original client in a private network namespace. See `ORIGINAL_CLIENT.md` for env vars |
| `run_original_direct.sh` | Older direct-connect runner (no login). Mostly superseded |
| `server/patch_curl_verify.py`, `server/patch_appes_diagnostic.py` | The two in-memory patches |
| `generated/wine-prefix/` | Wine prefix; client loose config at `.../Saved/Config/WindowsClient/Engine.ini` |
| `generated/wine-investigation/<date>-<topic>/` | Per-run logs and screenshots (ignored) |
| `MATCHMAKING_PROBE.md`, `direct_connect_results.md`, `original_client_net_protocol.md` | Network findings so far |

### 4.3 Game server (UE 4.25 project)

| Path | What |
|---|---|
| `native/FortniteLinux.uproject`, `native/README.md` | The project and how to build it |
| `native/Source/FortniteGame/FortniteGame.cpp` | Module startup: network version override, MagicHeader, AES key, encryption-token delegate |
| `native/Source/FortniteGame/Private/FortniteAthenaGameplay.cpp` | The only hand-written game mode code (`StartPlay`, `RestartPlayer`, invented Apollo actors). Not original behaviour |
| `native/Source/FortniteGame/Generated/` | ~8,500 generated class files (do not hand-edit; regenerate) |
| `native/Config/DefaultEngine.ini` | Server net config |
| `gen_native_classes.py`, `promote_native_classes.sh`, `generator_policies/` | The generator and its inputs |
| `behavior/<Class>/<Fn>.inc`, `behavior/MANIFEST.json` | Recovered function bodies spliced by the generator |
| `patches/ue425-*.patch` | Engine patches (engine checkout `/opt/fortnite/UnrealEngine` is not pushed) |
| `server/smoke_join.sh` | Reconstructed client ↔ server join smoke test |
| `/opt/fortnite/UnrealEngine` | UE 4.25.0 source (what we build) |
| `/opt/fortnite/UnrealEngine4260` | UE 4.26.0 source (reference only; the client reports `EngineNetVer 15`, which is a 4.26-era value) |

### 4.4 Reverse-engineering inputs

| Path | What |
|---|---|
| `generated/dump/FortniteClient-Win64-Shipping.runtime.exe` | Decrypted runtime image of the client. The thing to disassemble |
| `generated/protected-analysis-20261110/` | Later full image where the second `.text` section is readable, plus notes |
| `generated/reflection/` | `fortnite_game_reflection.json` (classes, properties, offsets, RPC roles), `native_funcs.json` (exec addresses), `class_checksums.json` |
| `replication_conditions.json` | Replication condition per property |
| `generated/thunk-table/final3.json` | exec thunk → body candidates |
| `generated/decomp*`, `generated/class-vtables/` | Earlier decompiles and vtable maps |
| `virtual_slots.py`, `behavior/vtable_lookup.py` | Resolve a vtable slot to a function |
| `engine_trace/` | Which engine virtuals a join calls on the Athena classes (the 59) |
| `tools/` | `strx.py` (string → code references), `nameidx.py` (name index), `vt.py` (vtable slots), `strs_near.py`. See `tools/README.md` |
| `server_scope/string_names.json` | 2,536 functions named by their own log strings. Look here first |
| `server_systems/` | Findings and system cards (`A-wire-findings.md`, `D-server-code-in-client-findings.md`) |
| `server_scope/` | Static "does a server need it" classification of UFUNCTIONs |
| `server_scope/local_model/` | Draft bodies (`claude/*.inc`), per-chunk notes (README), cut list |
| `/opt/fortnite/local-model-package/` | Capture tool and state: `scripts/evidence.py capture`, `captures/<hash>/<addr>.{c,asm,json}`, `scripts/inventory_progress.py` |
| `/opt/fortnite/ghidra-projects`, `/opt/fortnite/ghidra-install` | Ghidra |
| `server_authority_inventory.md` | Partial scan of who writes replicated properties (has known defects) |

### 4.5 Game data

| Path | What |
|---|---|
| `/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content` | Full extraction of the paks (963,427 files) |
| `.../Content/Athena/Playlists/` | Playlists, `AthenaCompositeGameData*`, loot override tables |
| `.../Content/Athena/Athena_GameMode`, `Athena_GameState`, `Athena_PlayerController`, `PlayerPawn_Athena*` | The Blueprint classes the client expects |
| `.../Content/Athena/SafeZone`, `Aircraft`, `SupplyDrops`, `Items`, `Balance` | Storm, bus, drops, items, balance tables |
| `cue4inspect/` | Our pak/asset inspector (`--function-report` dumps Blueprint bytecode; `--key-file` for encrypted paks) |
| `generated/DefaultEngine.ini` | The client's cooked engine config (net settings, MCP URLs) |

### 4.6 Reference only (other game versions; never copy offsets from these)

| Path | What |
|---|---|
| `/opt/fortnite/Fortnite-23.20-CL-23659353-SDK-Dump` | Header dump of a much later build. Useful for names and comments of fields, not layout |
| `/opt/fortnite/Fortnite14Research` | Community 14.30 project with recreated class headers. Same caveat |

### 4.7 Stale or superseded docs (do not plan from these)

`master_plan.md` (full-client plan, dated 2026-11-01), `REMAINING_WORK.md` (function-body classification for the full client), `server_surface.md` section "Current blocker: entitlement gate" (solved by the diagnostic patch), `direct_connect_results.md` "login gate" blocker (superseded by the matchmaking route), the "Not checked against a client" paragraph at the end of `server/README.md` (the client has since driven login, locker, shop and matchmaking).

---

## 5. What is already built

### 5.1 Backend

Status words: **verified** = the vanilla client did it in a recorded run; **implemented** = code and tests exist, the client has not exercised it; **stub** = fixed or guessed answer; **missing** = nothing.

| Area | Status | Detail |
|---|---|---|
| OAuth token, exchange code, verify, session kill | verified | Username-only local accounts. No passwords, no entitlement |
| Account lookup, `tryPlayOnPlatform`, externalAuths | verified | |
| Lightswitch, version check, enabled features, datarouter sink | verified (startup sequence) | Status values are guessed |
| MCP `QueryProfile` (athena, common_core, common_public, ...) | verified | Always full profile update |
| Locker: equip, slots, presets (copy, delete, rename, active index), banner, favourites, variants/styles, wraps incl. "apply to all" | verified for the cases in the `LOCKER_*_PROBE.md` files | Update policy is inferred; not every cosmetic type tried |
| Item shop: catalog, `PurchaseCatalogEntry`, purchase notification | verified | Local sample catalog, not the real 12.60 rotation |
| V-Bucks | verified | Local virtual currency, free refill pack. **No payment provider. There is no real-money path and none is planned**; receipts/billing/storeaccess are stubs |
| Battle Pass: purchase, level purchase, reward grants from the cooked season schedule | verified | |
| Battle Pass progression: XP, level-up from play, challenges/quests, style tokens | missing | Needs match results from the game server (WP E6) |
| Gifting (`GiftCatalogEntry`, `RemoveGiftBox`) | missing | Returns `operation_not_found` |
| Cloud storage (system and user files) | implemented | No hotfix files are served by default |
| Content pages (news, lobby), calendar/timeline, keychain | stub | Guessed shapes; keychain is a zero placeholder |
| Friends, blocklist, presence, social ban | stub | |
| Party v1 over HTTP | implemented | Not verified with the client. Real parties need XMPP |
| XMPP (presence, party messages, friend notifications, MCP push) | missing | TLS to our XMPP endpoint failed on self-signed cert in an early probe |
| Matchmaking: ticket, queue websocket, session record, session key, `/join` | verified (plaintext loopback websocket) | `wss://` fails; cause not proven |
| Game-server-facing API: server registers its session, heartbeats, reports results, runs `dedicated_server` profile commands | missing (the `dedicated_server` route prefix exists in the router; nothing calls it) | |
| Stats, leaderboards, events/tournaments, creative discovery, Save the World | missing | Out of scope until G9 except stats |
| Voice (Vivox), QoS ping | missing | Client tolerates absence so far (observed: reaches lobby and queue) |
| Party/lobby beacons | missing | **Not needed for a solo join so far**: in the 2026-11-09 probe the client went from `/join` straight to UDP on the game port (observed) |

### 5.2 Game server

| Item | Status |
|---|---|
| UE 4.25 Linux dedicated server target builds | yes (last recorded full green checkpoint: 3,058 bodies, `progress/native-checkpoint-3058.json`; later source is not covered by that checkpoint) |
| Generated native classes with offline class-checksum parity | 4,617 of 4,617 (offline calculation only; never compared on the wire with the vanilla client) |
| Network startup: `net.MagicHeader=1110`, AES handler component, Oodle off, version override 683914758, key from `FORTNITE_NET_ENCRYPTION_KEY[_FILE]`, token delegate | implemented; only MagicHeader and the first handshake step have been seen working against the vanilla client |
| Reconstructed Linux client joins reconstructed server (empty map, Apollo terrain, Athena pawn) | yes, headless |
| Original game mode behaviour (login, spawn rules, phases, bus, storm, inventory, weapons, building, loot, elimination) | **none running.** `FortniteAthenaGameplay.cpp` is 371 hand-written lines that spawn a pawn and some invented Apollo actors |
| Server loading the real `Apollo_Terrain` with its streamed levels and cooked Blueprints | no (cooked-Blueprint load problems; see `map_compatibility.md`, `streamed_levels_inventory.md`) |
| Missing native functions at runtime | silently skipped with one warning (`patches/ue425-null-native-function-call-skip.patch`). Good for booting, hides gaps: WP B1 turns this into a list |

---

## 6. Three kinds of remaining work

Every piece of server behaviour falls in exactly one bucket. Decide the bucket **before** starting work on it; the method differs.

### 6.1 Bucket R: recoverable from the client executable

The code is in the client and decompiles normally. This includes everything that runs on both sides, and a large part of the game mode that Epic did not strip.

Known to be present (binary, addresses in `engine_trace/needed_engine_driven.json`, `server_authority_inventory.md`, `server_scope/local_model/README.md`):

- The 59 engine-called overrides on `AFortGameModeAthena`, `AFortGameStateAthena`, `AFortPlayerControllerAthena`, `AFortPlayerPawnAthena`, `AFortPlayerStateAthena` (login, spawn, possess, match start).
- Storm: `JumpToSafeZonePhase` (0x141950090), `SafeZoneInsideChecks` (0x1428f08d0), `StartNextSafeZonePhase` (1,304 decompiled lines), storm tick 0x14194cd50.
- Aircraft: `OnAircraftEnteredDropZone`, `OnAircraftExitedDropZone`, `OnAircraftFlightEnded`.
- Bots: `UFortServerBotManagerAthena::SpawnBot` (0x141a5ca90, 2,270 bytes).
- All replication layouts, `OnRep_*`, Client and Multicast RPC bodies, movement, ability prediction, weapon client code.
- Custom `NetSerialize` bodies (five already recovered and built).

Method: capture → decompile → name callees and fields from reflection → write body → compile → run. Tools in 4.4. Caveat: a body that exists can still be hollow inside (an inner call goes to a null stub). Check every callee against the stub table in 6.3.

### 6.2 Bucket D: recoverable from the paks (data and Blueprints)

Nothing to reverse; it must be *loaded and executed* by our server.

- Blueprint bytecode of `Athena_GameMode`, `Athena_GameState`, `Athena_PlayerController`, `PlayerPawn_Athena`, `BP_SafeZoneStorm`, supply drops, chests, pickups, weapons, abilities (`GA_*`), gameplay effects (`GE_*`), gameplay cues.
- Tables: playlists, `AthenaCompositeGameData*` (safe-zone timings and sizes, game rules), loot tier and loot package tables, weapon stats, building costs and health, resource rates, XP tables.
- The map: actor placements for chests, floor loot spawners, ammo boxes, vehicles, POIs, player starts, the warmup island.

Method: make the server load the cooked asset, then find which native functions the Blueprint calls (`cue4inspect --function-report`, plus the "undeclared native function" warnings at runtime). The blocker is cooked-asset loading on the server (WP C1 to C3).

### 6.3 Bucket X: not in the client, must be re-created

Two sources of holes:

**Stripped server code.** Known shared stubs (binary):

| Stub | Bytes | Meaning |
|---|---|---|
| `0x140680300` | `xor eax,eax; ret` | returns null/0/false. 104 captured functions call it |
| `0x1406801c0` | `ret 0` | does nothing |
| `0x1406801e0` | `xor al,al; ret` | returns false |
| `0x1406801d0` | `mov al,1; ret` | returns true (shared `_Validate` for most Server RPCs) |

**Correction:** `ServerCreateBuildingActor_Implementation` (0x14236d1a0) and `ABuildingContainer::SpawnLoot` (0x141bec150) are real bodies that reach `UWorld::SpawnActor`; the earlier "stripped" note for building placement was wrong. Every other "stripped" label below is unverified until re-checked with the rule in the findings file (look up the name index, resolve the Athena override, capture the `_Implementation`). Suspected stripped so far: pickup spawn in `AFortAthenaSupplyDrop::SpawnPickupFromItemEntry`, `ServerCreateBuildingAndSpawnDeco` spawn, `TossSpecificItem`, `ServerCraftSchematic`, `ServerAddSimulatedXp`, `ServerSubmitGameplayVote`, `ServerPlaySquadQuickChatMessage`, `ServerRespawnFromDBNO` on Athena. The full list has **not** been swept (WP D0).

**Protected code.** Second `.text` at `0x148d2c000`–`0x149e4d600`. Readable in the later image; uses scattered blocks and stack-based jumps. Seven entries partly recovered (skydiving dispatch, supply-drop registry cleanup, weapon traces, vehicle collision, two player-state/controller paths). Details: wiki page, `generated/protected-analysis-20261110/`.

**Backend behaviour** was never in the client at all (Epic's services). Everything in `server/` beyond request shapes is re-created.

#### How to re-create a Bucket X behaviour (the question list)

For each stripped behaviour, answer these in a short "system card" (template in section 8) **before** writing code:

1. **What does the client send?** Which Server RPC, with which arguments? (reflection: `net_roles`; runtime: the RPC log from WP B1.)
2. **What must the client then see?** Which replicated properties change, which actor classes get spawned, which Client/Multicast RPCs arrive? Read the client-side `OnRep_*` and Client RPC bodies (Bucket R): they show exactly which fields the client reads and in what order it expects them.
3. **What does the client predict locally?** (e.g. building previews, weapon fire, ability activation.) The server result must match the prediction or the client rubber-bands.
4. **What data drives it?** Table rows, curve tables, item definitions, Blueprint defaults (Bucket D).
5. **What fragments survive in the client?** The `_Validate`, parameter structs, helper functions called from non-stripped code, log strings and `ensure` messages near the stub (search the runtime image for the function and log-category names), shared code used by Save the World paths.
6. **What does the Blueprint layer expect?** Events and delegates the server-side function is supposed to fire (`BlueprintImplementableEvent`, multicast delegates on the class).
7. **What are the invariants?** Resource counts, ammo, max stack, build limits, cooldowns.
8. **How will we test it with the vanilla client?** A specific in-game action and the expected on-screen result.

#### Where to look, in order of trust

1. The 12.60 runtime image (client side of the same feature).
2. The 12.60 paks (tables and Blueprint bytecode).
3. A live run with the vanilla client: what it sends, what it logs (`LogNet`, `LogFort*` warnings in the Wine client's `FortniteGame.log`), what it shows.
4. Public UE 4.24–4.26 source for engine-level systems (GameplayAbilities, CharacterMovement, replication, online beacons).
5. Other-version references in `/opt/fortnite` (23.20 SDK dump, 14.30 project): names and field meanings only.
6. Public community documentation of the Chapter 2 era backend API and game-server behaviour, and recorded 12.x gameplay. Use these as *leads* for ordering and semantics; verify each against 1 to 3 and label `inferred` until then. Do not copy code.

---

## 7. Work packages

Order of work: **A (wire) first and alone on the critical path.** B can start as soon as A1 is done. C and D0 can run in parallel with A because they need no vanilla client. D1+ can be written before G6 but cannot be verified until then. E is independent.

### Phase A: get the vanilla client into the world

> **A1/A2 status 2026-11-11:** the static half is done. `server_systems/A-wire-findings.md` shows the client's handshake, packet header, sequence init, Oodle-off behaviour and control messages (Hello, Login, Join) all match stock 4.25 apart from the float timestamp, which the patch covers. No further wire patch is indicated. What remains of A1 is one logged run and the decision tree in section 6 of that file. Steps 3 to 5 below are already answered there.

#### WP A1: finish the UDP stateless handshake (G3) [S, owns: `patches/ue425-fortnite-float-handshake.patch`, engine `StatelessConnectHandlerComponent.*`, `MATCHMAKING_PROBE.md`]

Known (observed): client's first packet is 25 bytes. Stock 4.25 expects 227 handshake bits; the client uses 195 (timestamp is a 32-bit float, not a double) and a 355-bit restart response. With the patch, the server sends the challenge, the client answers, the server acks, then the server keeps answering client packets with restart-handshake requests.

Steps:
1. Reproduce: build the server with the patch, run `run_original_client.sh` with the dedicated server in the same namespace, pass `-FortniteOriginalClient` to the server, press Play. Capture loopback UDP to `generated/wine-investigation/<date>-handshake/udp.pcap` and run the server with `-LogCmds="LogHandshake VeryVerbose, LogNet VeryVerbose, LogNetTraffic Verbose"`.
2. From the server log decide which case it is: (a) no `UNetConnection` was created after the ack (search for `NotifyAcceptingConnection`, `Server accepting post-challenge connection`); (b) a connection was created and then closed (search `UNetConnection::Close`, `Malformed_Packet`, `PacketHandler Error`); (c) client packets still arrive on the connectionless path (`IncomingConnectionless`).
3. Find the client's handshake code in the runtime image: search for the UTF-16 strings used by `LogHandshake` (for example `Restart handshake`, `Challenge`, `Incoming: Error reading handshake packet`), follow the references to the `StatelessConnectHandlerComponent` functions (`Incoming`, `Outgoing`, `SendInitialPacket`, `SendChallengeResponse`, `ParseHandshakePacket`), capture them with `evidence.py capture`.
4. Compare them field by field with `/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Private/PacketHandlers/StatelessConnectHandlerComponent.cpp` and the same file in `/opt/fortnite/UnrealEngine4260`. Check every item: bit order and count of each packet type, the cookie input (does the HMAC cover the float or the double timestamp bytes), `SecretId`, how the initial packet sequence numbers are taken from the cookie, the restart-response layout, the magic-header bits, the per-packet handshake bit on normal packets, and which handler components reserve bits before/after it (AES, Oodle).
5. Decode the pcap by hand for the first six packets with a small script (`porting/tools/` is fine) and print each field. Compare what the client sent after the ack with what the server's connection expects.
6. Fix the patch. Keep all changes behind `UseFortniteHandshake()` so the reconstructed client still joins (`server/smoke_join.sh` must pass).

Done when: the server log shows an accepted connection and the first non-handshake packet from the vanilla client is parsed without error (it will be the start of G4). Record the packet layouts in `original_client_net_protocol.md` section 1.

#### WP A2: control channel to Welcome (G4) [S, owns: `native/Source/FortniteGame/FortniteGame.cpp`, `native/Config/DefaultEngine.ini`, `original_client_net_protocol.md`]

Steps:
1. Log on the server, for every control message, its type and size, and for `NMT_Hello` the remote `NetworkVersion`, `IsLittleEndian` and `EncryptionToken` length. Confirm the remote version is 683914758 (inferred today, never seen on the wire).
2. Packet header: the client reports `EngineNetVer 15`; we build 14. In `/opt/fortnite/UnrealEngine4260` find every use of `HISTORY_CLASSNETCACHE_FULLNAME` and every 4.25→4.26 change in `NetConnection.cpp` (`ReceivedPacket`, `WritePacketHeader`), `DataChannel.cpp`, `DataBunch.cpp`, `NetPacketNotify`, `PackageMapClient.cpp`. Write a table "format difference → does 12.60 have it (check the runtime image) → patch needed". The client is a Fortnite-main build, so it can have a mix of 4.24, 4.25 and 4.26 traits (the float handshake is a 4.24 trait). Do not assume; check each in the binary.
3. Encryption: the client asks the backend for the session key and sends `?EncryptionToken=` in Hello. Start the server and backend with the same `FORTNITE_NET_ENCRYPTION_KEY_FILE`. Confirm `NMT_EncryptionAck` and that packets after it decrypt (AES-256 ECB handler per `original_client_net_protocol.md`). If the client's AES handler differs from stock 4.25 (bit layout, padding), capture the client's `AESHandlerComponent::Incoming/Outgoing` and match it.
4. Oodle: the runner writes `[OodleHandlerComponent] bEnableOodle=False` into the client's loose `Engine.ini`. Confirm in the client log that it is applied. (That is a config file, not an exe change. Supporting Oodle on the server is a later, optional task and needs the Oodle Network SDK, which we do not have.)
5. Challenge/Login: log the login URL options the client sends (strip tokens before saving). `AFortGameModeAthena::PreLogin`/`Login` originals are at 0x141950bd0 / 0x141959680 (WP D1); for now accept the login in our stand-in and record which options exist.

Done when: server log shows `NMT_Welcome` sent and the client log shows it received `Welcome` (client `LogNet: Welcomed by server`).

#### WP A3: map load and Join (G5) [S, owns: `native/Source/FortniteGame/Private/FortniteAthenaGameplay.cpp`, `map_compatibility.md`]

Steps:
1. Decide what `Welcome` must carry: map package `/Game/Athena/Apollo/Maps/Apollo_Terrain` and the game mode class path. Read the client log to see what it tries to load and whether it needs the playlist first (search the client log for `LogPlaylist`, `LogLoad`, `LogStreaming` errors).
2. The server must not replicate any class the client does not have. Put all invented actors (`AApolloLandscapeTile`, `AApolloBackgroundMesh`, `AApolloSkyAtmosphere`, `AApolloSunLight`) behind the existing original-client flag so they are never spawned or never replicated in that mode.
3. The first replicated actors must be the classes the client expects: `Athena_GameState_C`, `Athena_PlayerController_C`, `FortPlayerStateAthena`, `PlayerPawn_Athena_C` (data: `Content/Athena/*.uasset`). If our server cannot load those Blueprints yet (WP C1), replicate the native parent as a first experiment and record what the client does.
4. For every `Network checksum mismatch`, `NetGUID`, `Failed to resolve` or `class not found` line in the **client** log, write a row in a table (class, property or path, client value, server value) and fix the server class. The offline checksum parity says this should be rare; this is the first real test of that claim.
5. Static actors: the client resolves level actors by path. Until the server loads the same levels, anything the server replicates by static path will fail. List what failed.

Done when: client log shows `Join succeeded` and the loading screen drops, or a precise list of what the client rejected.

#### WP A4: stay connected and controllable (G6) [S then T, owns: same as A3]

Steps:
1. Keep the client connected for 5 minutes. For each disconnect, record the `ENetworkFailure`/close reason from both logs and fix in order.
2. Using the RPC log (WP B1), list every Server RPC the client sends in the first minute (`ServerAcknowledgePossession`, `ServerReadyToStartMatch`, `ServerLoadingScreenDropped`, `ServerSetClientHasFinishedLoading`, ...; the real list comes from the run, these names are examples from reflection) and make sure each has at least a safe handler.
3. List every Client RPC and replicated property the client appears to wait for (HUD does not appear, loading screen does not drop, pawn cannot move). Use the client-side code of the loading-screen and HUD state checks to find which replicated flags gate them.

Done when: the player can walk around on the server's world for 5 minutes with HUD visible, recorded as a screenshot and both logs.

### Phase B: replace estimates with runtime evidence

#### WP B1: server-side call and RPC logging [T, owns: a new file `native/Source/FortniteGame/Private/FortServerTrace.cpp`, one small engine patch under `patches/`]

Build a trace that can be switched on with `-FortServerTrace=<file>` and writes JSON lines:
- every RPC received from a client: class, function, payload bit size, connection;
- every native `UFunction` executed on the server through `ProcessEvent`/`CallFunction` (name and class, counted, not one line per call);
- every "undeclared native function" skip (extend `ue425-null-native-function-call-skip.patch` to log the *callee import name*, not only the caller);
- every `UFunction` whose native pointer is the generator's unimplemented stub (the generator emits a recognisable stub; log class and function on first call).

Done when: a reconstructed-client join produces a trace file and `python3 -I porting/server_scope/runtime_report.py <trace>` (write it) prints counts per category.

#### WP B2: client-side expectation scan [L, read-only]

Write `porting/tools/client_log_scan.py`: given a Wine client `FortniteGame.log`, print grouped counts of warnings and errors for the categories `LogNet`, `LogNetPackageMap`, `LogRep`, `LogNetTraffic`, `LogFort*`, `LogAbilitySystem`, `LogStreaming`, `LogLoad`, with first and last timestamp and one example line each. No tokens or account ids in the output.

#### WP B3: the runtime-required ledger [T, owns: `server_scope/runtime_required.jsonl`, `server_scope/README.md`]

Merge every trace from B1 into one ledger: function → first gate at which it was called → state (`missing`, `stub`, `draft exists`, `built`, `verified`). Join it with `functions.jsonl` (static tier), `behavior/MANIFEST.json` (built bodies) and `server_scope/local_model/claude/` (drafts). From G6 on, **this ledger is the list of required functions**, and the wiki numbers come from it.

### Phase C: world and data on the server

#### WP C1: load the Athena Blueprint classes on the server [S, owns: `gen_native_classes.py` policies, `native_class_set.txt`]

Goal: the dedicated server loads `Athena_GameMode`, `Athena_GameState`, `Athena_PlayerController`, `PlayerPawn_Athena` and `Playlist_DefaultSolo` from the cooked paks without crashing.
Steps: stage the packages and their hard dependencies (use the staging helper described in memory/`frontend` notes: `generated/frontend-probes/stage_missing.py LOG`); load each with a commandlet-style probe; from the log collect `Skipping import Function`, `Could not find class`, `Missing Dependency`, `still RF_NeedLoad`; add the missing classes and declare all reflected UFunctions for them (the known root cause from the frontend work: Blueprint overrides of native events need the native super function declared). Repeat until clean.
Done when: all five load on the server and their class default objects construct.

#### WP C2: load `Apollo_Terrain` and the streamed levels the server needs [S]

Start from `streamed_levels_inventory.md` and `map_compatibility.md`. A dedicated server needs collision, player starts, loot and chest spawners, vehicles spawners, POI volumes; it does not need render data. Produce a list of the 87 streamed levels with: loads / fails (first error) / needed by server (yes, no, unknown). Decide and document whether level streaming on the client is driven by replicated state (`FortWorldManager`, `WorldTileManager`) or purely client-side.

#### WP C3: game data access [T]

Write a headless check (commandlet or `cue4inspect` script) that prints, for `Playlist_DefaultSolo`: game mode class, game data tables, safe-zone definition rows (count, wait and shrink times, radii, damage), loot tier table and loot package table names with row counts, max players, team size. Output to `porting/generated/gamedata/` and a short committed summary `porting/gamedata_summary.md`. This is the Bucket D source for D2, D10, D11.

### Phase D: gameplay systems

#### WP D0: sweep for stripped and protected functions [T, read-only on code; owns `server_scope/stripped.json`]

**Status 2026-11-11: done for reflected functions.** Method, numbers and the list are in `server_systems/D5-stubbed-rpcs-and-recreation.md`; data in `server_scope/server_rpc_sweep.json` and `server_scope/native_stub_sweep.json`. Still open from the text below: C++-only functions (use `callers.py` and the name index per system) and the protected-range check.

For every captured function in `/opt/fortnite/local-model-package/captures/` and every body listed in `engine_trace/needed_engine_driven.json`: record whether it calls one of the four stubs (table in 6.3) or jumps into `0x148d2c000`–`0x149e4d600`, and whether the stub call is the *main effect* (the function does nothing else that matters) or incidental. Output `server_scope/stripped.json`: function, address, stub, role (`main`/`incidental`), notes. Then extend to all Server RPC `_Implementation` bodies of the classes in section 7's system list.
Done when: every function used by systems D1 to D11 has a bucket (R, D or X).

#### WP D-triage: sort the ~610 drafts [T, owns: `server_scope/local_model/review.json`]

For each draft `.inc` in `server_scope/local_model/claude/`: classify `complete` (all callees named and present), `partial` (helpers summarised, list them), `stripped-shell` (from D0), `out-of-scope` (creative, STW, cosmetic). Do not fix bodies. Output counts per system in section 7. This tells us what the drafts are actually worth.

#### System work packages D1 to D17

Each system is one WP family. For each, the first task is always the **system card** (section 8). Tier is S for the card of D1, D2, D4, D5, D9, D10; T otherwise. Owner of `native/` rule applies when writing code.

| WP | System | Server must do | Main bucket | Known leads |
|---|---|---|---|---|
| D1 | Login and spawn pipeline | `PreLogin`, `Login`, `PostLogin`, player state setup, team assignment, default pawn class, spawn on warmup island, possess, cosmetic loadout from profile | R | The 59 bodies in `engine_trace/needed_engine_driven.json`; `ServerSetTeam` draft |
| D2 | Match state and game phases | `InitGame` 0x14194cd90, `HandleMatchHasStarted` 0x14194bb50, `StartWarmupPhase` 0x141972c30, `UpdateWarmupPhase` 0x14197a400, `StartAircraftPhase` 0x14196b7ca, `StartSafeZonesPhase` 0x141971aa0, `StartEndGamePhase` 0x14196cb95, `HandleMatchHasEnded` 0x14194b750; replicated `GamePhase` and timers on `AFortGameStateAthena` | R + D | Findings file; draft bodies for chunks 10, 11; playlist data (C3) |
| D3 | Battle bus, jump, skydive, glider | Spawn aircraft, fly path, drop zone events, `ServerAttemptAircraftJump`-type RPCs, skydiving state, auto-deploy, landing | R + X (protected skydiving path) | `OnAircraft*` bodies; protected `0x1495a4a24`; chunk 22, 24, 25 drafts |
| D4 | Inventory and quickbar | World inventory actor, item list fast-array replication, quickbar slots, grant pickaxe and building pieces at spawn, equip (`ServerExecuteInventoryItem`), drop, stack, ammo | R for containers, X for grant/drop | Chunk 15 drafts (pickup, inventory interface slots 0xa0/0xa8); `TossSpecificItem` is stripped |
| D5 | Ability system | Grant default ability sets, attribute sets (health, shield, movement), activate abilities from client, apply gameplay effects, gameplay cues | D (Blueprint GAs/GEs) + R + public GAS source | Chunks 33, 34 drafts (ability library); `UFortAbilitySystemComponent` |
| D6 | Weapons and damage | Fire validation, hit processing, projectiles, reload, ammo, damage calc, falloff, headshots | R + X (two protected weapon traces) | Chunks 30 to 32 drafts; protected `0x14941fa9a`, `0x1496f7856` |
| D7 | Health, DBNO, elimination, spectating, victory | Apply damage to health/shield, down state, death, kill feed (`ClientReportKill`), placement, spectate, win condition, `WinningPlayerState` | R | `CheckWinConditionLastManStanding` 0x141937590, `GrantVictory*`, `GrantLoss*`, `OnFortPlayerPawnAthenaDied` binding at 0x1419e4f50 |
| D8 | Harvesting and resources | Pickaxe hits on building actors, damage, resource grant, weak points | R + X | `ABuildingSMActor` drafts (chunk 04) |
| D9 | Building and editing | Place piece (`ServerCreateBuildingActor_Implementation` 0x14236d1a0, real body), costs (`UFortGameData::GetBuildingCost`), structural support, edit, repair, destroy | R | Findings file section 2; chunk 32, 33 notes |
| D10 | Loot | Floor loot, chests (`ABuildingContainer::SpawnLoot` 0x141bec150, real body), ammo boxes, supply drops, llamas, pickup spawn (supply-drop path unverified), loot table rolls, `GetLootChanceMultiplier` / `GetLootCountMultiplier` | R + D, X only where re-check confirms | Loot tables (C3); supply-drop drafts chunk 09 |
| D11 | Storm | Phases from data, shrink, damage ticks, storm-camping | R + D | Hand decode `work/dependencies/claude-br/storm-and-phase-findings.json` in the package |
| D12 | Vehicles, water, ziplines | Boats, helicopter, swimming, seats | R + X (protected vehicle collision) | Chunk 09, 23, 26 drafts |
| D13 | Season 12 AI | Henchmen, bosses, vaults, keycards, ID scanners, shakedown, phone booths | R + D | `UFortServerBotManagerAthena`, behaviour trees in paks |
| D14 | Consumables, traps, throwables | Use, effect, placement | D + R | |
| D15 | Teams and squad features | Squad ids, revive, reboot vans, pings and markers, team chat | R | Chunk 22 notes |
| D16 | Match results, XP, quests | Per-player stats, XP, quest progress, reporting to backend | R + X | `ServerAddSimulatedXp` has a real body (sweep: mixed). Result reporting and XP: `E-backend-remaining.md` E2, `D6` card §2 steps 7–9 |
| D17 | Replication graph, relevancy, dormancy | Performance for 100 players; not needed for a first match | public UE source + R | `UFortReplicationGraph` is server-only and absent |

Recommended order after G6: D1 → D2 → D5 → D4 → D11 → D3 → D10 → D6 → D7 → D8 → D9 → D15 → D16 → the rest.

### Phase E: backend completeness

| WP | Task | Tier | Notes |
|---|---|---|---|
| E1 | Make the matchmaking websocket work over `wss://` | T | Find why the client's websocket TLS rejects our cert (the curl patch does not cover libwebsockets). Either a trusted local CA in the Wine prefix or a second in-memory patch. Then drop the plaintext special case |
| E2 | Game-server-facing API | T | Server registers its session (address, port, playlist, build id), heartbeats, marks started/ended; backend hands the session key to the server instead of both reading one env var. Design it ourselves (Bucket X); keep it small |
| E3 | XMPP service | S | Needed for parties, presence, friend lists that update, and MCP push. Start with TLS + auth + presence + party MUC messages. Leads: client config `generated/DefaultEngine.ini` (XMPP section), strings and JSON field names in the runtime image, the party HTTP service already written |
| E4 | Party verified with two vanilla clients | T | After E3. Then decide if party/lobby beacons are needed for squads (inferred not needed for solo) |
| E5 | Real 12.60 catalog and content | T | Build storefronts from pak data (item definitions, prices are not in the paks: author a price table and label it `guess`), news and lobby content pages, calendar/timeline for Season 12 events, keychain entries for dynamically keyed paks if any |
| E6 | Progression | T | XP, account and Battle Pass level from match results (needs E2, D16), challenges/quests (`ClientQuestLogin`, quest items in the athena profile), style tokens |
| E7 | Gifting, receipts, friends, stats, leaderboards | T | Low priority; implement when a UI path is shown to need it |
| E8 | Launcher and AppES gate without diagnostics (G11) | S | Understand what triggers `closing code 3` and the 20-second exit timer (`EXIT_TRACE.md`, `LAUNCHER_PROBE.md`) and satisfy it, or keep the narrow `timer` patch and document it as the supported mode |
| E9 | Cloud storage hotfix set | L/T | Supply a minimal `DefaultGame.ini`/`DefaultRuntimeOptions.ini` hotfix set; this is also the supported way to change client config (for example Oodle) without touching the exe |

### Phase F: keep the project findable

| WP | Task | Tier |
|---|---|---|
| F1 | Add a "superseded by SERVER_FIRST_PLAN.md" banner to each stale doc in 4.7; fix the stale "Not done" section in `server/README.md` | L |
| F2 | Move runtime findings scattered in `docs/progress.json` evidence strings into `MATCHMAKING_PROBE.md` / `original_client_net_protocol.md` | L |
| F3 | Change the server-first treemap input so its native tile reads from the runtime ledger (B3) instead of the 1,681 static set; until B3 exists, label the tile "static estimate" | T |

---

## 8. System card template (first deliverable of every D-system)

Save as `porting/server_systems/<Dn>-<name>.md`. Keep it under two pages.

```
# <system>
Gate it unlocks: G7 | G8 | ...
## Client → server (what the client sends)
| RPC (class::function) | args | when | exec addr | impl body addr | bucket R/X | draft .inc |
## Server → client (what the client must observe)
| replicated property or Client/Multicast RPC | class | type | condition | OnRep/handler addr | what the client does with it |
## Server-only functions (not RPCs)
| function | addr | called from | bucket | status (missing/draft/built/verified) |
## Data
| asset or table | what it provides |
## Blueprint events/delegates the server must fire
## Stripped or protected parts and the re-creation plan (answers to the 8 questions in 6.3)
## Test with the vanilla client
| action in game | expected on screen | expected in server log |
## Open questions
```

Fill the tables from: `generated/reflection/fortnite_game_reflection.json` (properties, RPC roles), `replication_conditions.json`, `generated/reflection/native_funcs.json` (exec addresses), `/opt/fortnite/local-model-package/work/claude/addrs.json`, the captures, `server_scope/local_model/README.md` chunk notes, `server_scope/stripped.json` (after D0), and `cue4inspect --function-report` on the relevant Blueprints.

---

## 9. Keeping status honest

After any run with the vanilla client:

1. Add a dated paragraph to the matching probe doc (`MATCHMAKING_PROBE.md` for G2 to G6; a new `GAMEPLAY_PROBE.md` for G7+).
2. Update the gate table in section 2 of this file and the same table on the wiki page.
3. If a number on the wiki changes, change it from the source file, not from memory. Sources: `server_scope/functions.jsonl`, `behavior/MANIFEST.json`, `inventory_progress.py status`, `server_scope/runtime_required.jsonl`, `server_scope/stripped.json`.
4. Treemap only: `python3 -I porting/progress/server_first/refresh_wiki.py /opt/fortnite/fortnite-12.60-wiki`. It does not rewrite the page text.
5. Commit specific paths; push fast-forward only; wiki is its own repo (`master`).

## 10. Open decisions for the project owner

These do not stop any WP above; defaults are stated.

1. **Client config changes.** Default: allowed (loose `Engine.ini` and cloud-storage hotfix files; exe on disk untouched). This is how Oodle is switched off. Alternative: obtain an Oodle Network SDK and support the stock client config.
2. **In-memory patches.** Default: keep the TLS and AppES/timer diagnostics until G9; G11 removes them later.
3. **Scope of "all gameplay".** Default: Battle Royale solo first, then squads; Creative, Save the World, tournaments stay out until G10.
4. **Draft chunk campaign (chunks 39+).** Default: pause it until WP D0, D-triage and B3 show which functions matter; more LOW-confidence drafts do not move a gate.
