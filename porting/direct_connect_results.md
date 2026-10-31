# Direct connect results: original Win64 client to native Linux server

Stage 1 of `original_client_net_protocol.md` (section 4). Runner:
`porting/run_original_direct.sh [seconds] [client args...]`. Both processes run in one private
network namespace (`unshare -n -m`). Logs are under `porting/generated/direct-connect/`
(ignored by Git; per-run copies under `runs/`). This doc records what was observed, not what was expected.

**Correction.** An earlier version of this doc said the native server stalls between Browse and
listen. That was wrong. The stall was a harness bug: `IpNetDriver` is UDP, and the runner checked
for a TCP listener (`ss -ltn`), so it never saw port 7777. The server listens within milliseconds
of its Browse (see "Retracted: server stall" below). The runner is fixed (UDP check, line-buffered
server stdout).

## Setup

- Client: original `FortniteClient-Win64-Shipping.exe` under Wine 10.0 in the
  existing prefix `porting/generated/wine-prefix`. Flags as in
  `run_original_client.sh`, minus every `-AUTH_*` flag: `-nosound -NoSplash -log
  -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none -epicapp=Fortnite
  -epicenv=Prod -NOEPICPORTAL`. No `patch_curl_verify.py` memory write.
- Server: staged `FortniteLinuxServer` (built 21:09, after `FortniteGame.cpp` 21:07),
  map `/Game/Maps/FortniteEmptyDedicated`, `-nullrhi -nosound -unattended -port=7777
  -log -FortniteOriginalClient`, run as `stdbuf -oL -eL` (line-buffered stdout). No encryption key was set.
- Client loose config: a marked block in `Saved/Config/WindowsClient/Engine.ini` (written
  by the runner, replaced on each run): `[SystemSettings] net.MagicHeader=1110`,
  `[PacketHandlerComponents] EncryptionComponent=AESHandlerComponent`,
  `[OodleHandlerComponent] bEnableOodle=False`. The `GameDefaultMap` override tried in
  attempt 2 was removed after it had no effect. A backup of the pre-change file was
  deleted during cleanup.
- The probe server (`server/probe_server.py --stub` on 127.0.0.1:443) answers the
  HTTP/TLS endpoints the client calls at startup. No login is attempted.

## Attempt 1: address as the first command-line token

Runs: the first two runs failed in the runner's own readiness check (`rg` was not
on root's PATH inside `sudo`). The server log shows the server did listen. The third
run (with a 180 s readiness window and `grep`) ran to completion. Client result:

- Client command line, from `LogInit`: `Filtered Command Line: -127.0.0.1:7777 -nosound
  -NoSplash -log -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none`.
- The client then went to its default map: `LogNet: Browse: /Game/Maps/Frontend?Name=Player`,
  `LogLoad: LoadMap: /Game/Maps/Frontend?Name=Player`, then the frontend login
  (`LogFortLogin: Could not auto-login. Starting manual login process`).
- The server saw no connection (`IpNetDriver listening on port 7777` and no NMT_Hello).

Cause: `UGameInstance::StartGameInstance` (`Engine/Source/Runtime/Engine/Private/GameInstance.cpp`)
takes the first token as the package name, and falls back to the default map when
the token starts with `-` (`**PackageName == '-'`). `FCommandLine::BuildWhitelistCommandLine`
(`Core/Private/Misc/CommandLine.cpp:208-225`) rebuilds the command line and prefixes
every argument with `-`, so `127.0.0.1:7777` becomes `-127.0.0.1:7777`. The
`Filtered Command Line` line in the client log is that rebuilt string. A bare
address therefore cannot be the first token in this build. The plan's `ip:port`
option is blocked by this, not by the server.

## Attempt 2: GameDefaultMap=127.0.0.1:7777 in loose Engine.ini

The same `StartGameInstance` fallback path is reached when the first token is absent,
and it uses `GameMapsSettings.GameDefaultMap` + `LocalMapOptions`. The default-map browse
was observed in attempt 1 (`Browse: /Game/Maps/Frontend?Name=Player`).

Result: the client still browsed to `/Game/Maps/Frontend?Name=Player` and showed the
frontend login (`LogNet: Browse: /Game/Maps/Frontend?Name=Player`, 21:45:16 in the run
after the readiness fix). The loose `GameDefaultMap=127.0.0.1:7777` line had no effect.

Cause (from source; not tested with a different value): `GameDefaultMap` is declared as
`FSoftObjectPath` in a `config=Engine` class (`Engine/Source/Runtime/EngineSettings/Classes/GameMapsSettings.h:169`,
`UCLASS(config=Engine, defaultconfig)` at line 69). A string like `127.0.0.1:7777` is not an
object path, so the config value cannot be imported. The earlier guess (the property is not read
from loose config) is not supported by this: the loose `Engine.ini` is applied (`LogConfig: Applying CVar
settings from Section [/Script/Engine.NetworkSettings] File [...Saved/Config/WindowsClient/Engine.ini]`).

Other command-line route checked: `-ini:Engine:[section]:Key=Value` overrides are compiled only when
`ALLOW_INI_OVERRIDE_FROM_COMMANDLINE` (`Core/Public/Misc/ConfigCacheIni.h:27`) is set, that is
`UE_SERVER || !UE_BUILD_SHIPPING`. The client is Shipping, so this route is closed.

Server side in attempt 2, from `porting/generated/direct-connect/server.log`:

- `LogNetVersion: FortniteLinux 1.0.0, NetCL: 0, EngineNetVer: 14, GameNetVer: 0 (Checksum: 326778485)`
- `LogFortniteNet: net.MagicHeader = 1110`, `net.MaxChannelSize = 65535`, `net.AllowEncryption = 1`
- `LogFortniteNet: Network encryption key not configured.`
- `LogNet: Browse: /Game/Maps/FortniteEmptyDedicated?Name=Player`
- `LogNet: GameNetDriver IpNetDriver_2147482579 IpNetDriver listening on port 7777`

No NMT_Hello, no Remote network version line and no client connection was logged.

Runner notes from this attempt (corrected): the stdout capture was line-buffered in the
runner's view, and the runner's readiness check used TCP (`ss -ltn`). The TCP check was
the bug; see "Retracted: server stall". The runner now checks UDP port 7777 (`ss -lunH`,
with a `/proc/net/udp` fallback) and runs the server under `stdbuf -oL -eL`.

## Attempt 3: `-nomcp` (run 01) and `-nomcp -MeshParent` (run 02)

Command: `porting/run_original_direct.sh 90 -nomcp`, then
`porting/run_original_direct.sh 90 -nomcp -MeshParent=127.0.0.1 -MeshParentPort=7777`, with the
same loose `Engine.ini` block as attempt 2. No `-AUTH_*` flags. Logs in `runs/01-nomcp/` and
`runs/02-nomcp-meshparent/`.

Server, both runs: the runner printed `Server listening on UDP 7777.` On the engine log
(`Saved/Logs/FortniteLinux.log`, copied to `server-UE.log`):

- Run 01: `LogNet: Browse: /Game/Maps/FortniteEmptyDedicated?Name=Player` at 22:00:57.779,
  `LogNet: GameNetDriver IpNetDriver_2147482579 IpNetDriver listening on port 7777` at 22:00:57.786.
- Run 02: Browse at 22:03:47.788, listening at 22:03:47.795.

Client, run 01, from `client-FortniteGame.log`:

- `LogInit: Filtered Command Line: -nosound -NoSplash -log -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none -nomcp`
- `LogNetVersion: FortniteGame 1.0.0, NetCL: 12896485, EngineNetVer: 15, GameNetVer: 0 (Checksum: 683914758)` (the client's own value; the override was loaded)
- `LogNet: Browse: /Game/Maps/Frontend?Name=Player`, `LogLoad: Took 7.449360 seconds to LoadMap(/Game/Maps/Frontend)`
- `LogFortLogin: Could not auto-login. Starting manual login process`
- `LogFortLogin: Activating widget SignIn for login stage SignIn.`
- `LogAppES: Display: postponed 2` (last line before the 90 s timeout)
- MCP calls fail: `LogOnline: Warning: MCP: Http Unexpected Result: HttpResult: 0 ... Raw=connection_failure`,
  `LogOnlineIdentity: Warning: MCP: Client auth request failed`. The probe recorded no requests
  (no `server-requests.jsonl`).

Run 02 gave the same result. The client accepted `-MeshParent=127.0.0.1 -MeshParentPort=7777`
(in its `Filtered Command Line`), stayed on `Frontend` and the SignIn screen, and never logged
`GetMeshConnectInfo`. That mesh path is not reached from the frontend.

Server, both runs: no NMT_Hello, no `Remote network version`, no Challenge, Login, Welcome or Join.

Exe strings that mention `-nomcp` (recorded before use): `Seasonal Event manager doesn't exist ...
(this is never expected except in -nomcp)!`, `Hotfixing skipped due to -nomcp`, `GetMeshConnectInfo:
Running with -nomcp requires -MeshParent`, `MeshParent=`, `MeshParentPort=`, `NoMCPAccessToken`
(in the `?AuthTicket=...&GameAccountId=` login URL, which exists only after a login). None of them
skips the manual sign-in.

## Retracted: server stall (harness bug)

The stall reported in the earlier version of this doc is not real. Evidence:

- The orchestrator's 12 of 12 solo runs listened about 780 ms after Browse.
- Runs 01 and 02 above listened 7 to 8 ms after their Browse on the same log.
- The runner waited on `ss -ltn`, a TCP listener, for an IpNetDriver that is UDP. Every "did not
  listen" result came from that check.
- Stdout was not line-buffered, so the captured log lagged. The 0% CPU idle was the frame limiter.

The runner is now fixed (`porting/run_original_direct.sh`): UDP readiness check, `stdbuf -oL -eL`
for the server, a bounded cleanup that kills only its own server and probe, and a copy of the engine
log (`server-UE.log`) at exit.

## Blockers and next steps (ranked)

1. **Login gate (blocks every path found so far).** Without `-AUTH_*` flags the frontend
   shows the manual sign-in (`Could not auto-login`, `Activating widget SignIn`). The join path is
   driven by matchmaking and MCP (`UFortUIStateWidget_JoinServer` handlers, `ServerIP` session
   attribute). `-nomcp` and `-MeshParent` do not skip it. `LogAppES: postponed 2` stayed unresolved
   for the whole 90 s window, and no "not started correctly" popup was seen. A working login route
   remains an unresolved prerequisite for original-client network observations.
2. **Typed-address routes checked and ruled out:** first token (rewritten to `-address`), loose
   `GameDefaultMap` (`FSoftObjectPath`, see attempt 2), `-ini:` overrides (compiled out of Shipping),
   `-MeshParent` (not reached from the frontend). No "join by address" UI string was found in the exe.
   The only `{host:port}` string is a credential-name template.
3. **MCP stub.** The client's MCP calls fail with `connection_failure` and the probe records no
   requests. This matters only after blocker 1 is solved. Likely the TLS trust or hosts path; not investigated.
4. **Version override.** Not verified on the wire. The server log shows only the stock value
   326778485. `Local network version checksum: 683914758` has not appeared because no NMT_Hello was received.
5. **Encryption.** Not tested. Test with a random scratch key only after an unencrypted Hello is seen.
6. **Class checksums and NetGUID errors.** Not reached.

## Result summary

- NMT_Hello accepted: no. No Hello reached the server (runs 01 and 02).
- Remote network version seen by the server: none (no `Remote network version` line).
- Challenge / Login / Welcome / Join: none observed.
- Network checksum mismatch, NetGUID, class path errors: none observed.
- Client crash: none observed. Both runs stopped at the 90 s timeout on the SignIn screen.
- The run ended with `LogAppES: postponed 2` unresolved.

## Files changed

- `porting/run_original_direct.sh` (new, then fixed): sibling runner, same netns and client flags
  as `run_original_client.sh` minus AUTH flags, no curl-verify memory patch. Fixes: UDP readiness check
  (`ss -lunH` with a `/proc/net/udp` fallback, no `rg` needed); server run under `stdbuf -oL -eL`;
  bounded cleanup that kills only its own server (`pkill` on this stage's binary) and probe; copies the
  engine log to `server-UE.log`.
- `porting/direct_connect_results.md` (new, then corrected): this report.
- `porting/original_client_net_protocol.md`: status block (Step 0/1) corrected.
- Ignored, not tracked: `porting/generated/wine-prefix/.../Saved/Config/WindowsClient/Engine.ini`
  (loose net block, see the runner), `porting/generated/direct-connect/` logs and `runs/`.

No native code was changed and no UE build was run in this stage. The staged server
binary (built 21:09) was used as is.

## Process hygiene

- No leftover Wine, Xvfb or server processes from this stage. Ghidra was not touched.
- `/tmp/server-stall` (a subagent's stall experiments, kept outside the repo) was not touched.
- Temporary exe string dumps under `/tmp` were removed.
