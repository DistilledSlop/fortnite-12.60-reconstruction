# Direct connect results: original Win64 client to native Linux server

Stage 1 of `original_client_net_protocol.md` (section 4). Runner:
`porting/run_original_direct.sh [seconds]`. Both processes run in one private
network namespace (`unshare -n -m`). Logs are under `porting/generated/direct-connect/`
(ignored by Git). This doc records what was observed, not what was expected.

## Setup

- Client: original `FortniteClient-Win64-Shipping.exe` under Wine 10.0 in the
  existing prefix `porting/generated/wine-prefix`. Flags as in
  `run_original_client.sh`, minus every `-AUTH_*` flag: `-nosound -NoSplash -log
  -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none -epicapp=Fortnite
  -epicenv=Prod -NOEPICPORTAL`. No `patch_curl_verify.py` memory write.
- Server: staged `FortniteLinuxServer` (built 21:09, after `FortniteGame.cpp` 21:07),
  map `/Game/Maps/FortniteEmptyDedicated`, `-nullrhi -nosound -unattended -port=7777
  -log -FortniteOriginalClient`. No encryption key was set.
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
and it uses `GameMapsSettings.GameDefaultMap` + `LocalMapOptions`. Setting
`GameDefaultMap` in the loose `Engine.ini` therefore makes the client browse straight
to the server address. The default-map browse was already observed in attempt 1
(`Browse: /Game/Maps/Frontend?Name=Player`), which confirms this path runs.

Result: the client still browsed to `/Game/Maps/Frontend?Name=Player` and showed the
frontend login (`LogNet: Browse: /Game/Maps/Frontend?Name=Player`, 21:45:16 in the run
after the readiness fix). The loose `GameDefaultMap=127.0.0.1:7777` line had no effect:
the value in `Saved/Config/WindowsClient/Engine.ini` was present in the file and the
client's `StartGameInstance` default-map browse still used the cooked
`/Game/Maps/Frontend`. The cause is not yet identified; likely the property is not read
from the loose config for this class, or Fortnite's game instance overrides the path.

Server side in attempt 2, from `porting/generated/direct-connect/server.log`:

- `LogNetVersion: FortniteLinux 1.0.0, NetCL: 0, EngineNetVer: 14, GameNetVer: 0 (Checksum: 326778485)`
- `LogFortniteNet: net.MagicHeader = 1110`, `net.MaxChannelSize = 65535`, `net.AllowEncryption = 1`
- `LogFortniteNet: Network encryption key not configured.`
- `LogNet: Browse: /Game/Maps/FortniteEmptyDedicated?Name=Player`
- `LogNet: GameNetDriver IpNetDriver_2147482579 IpNetDriver listening on port 7777`

No NMT_Hello, no Remote network version line and no client connection was logged.

Runner notes from this attempt:

- The server stdout is block-buffered into its log file. The readiness check on
  the log could not see the `listening` line until the process exited. The runner
  now checks the socket with `ss -ltn`.
- The first attempt-2 run was interrupted (the server was stopped by hand) and the
  runner read a half-rewritten copy of itself; those runs are not evidence about the
  client. Attempt 2 is the run that followed.

## Attempt 3: `-nomcp` (documented in the exe as "never expected except in -nomcp")

Command: `porting/run_original_direct.sh 90 -nomcp`, with the same loose `Engine.ini`
block as attempt 2. The exe contains a `-nomcp` branch; the string is the only evidence
for it so far.

Result: not tested. The server did not reach its listen call within the runner's
150 s server lifetime (`timeout` wrapper: seconds + 60), so the runner printed
`Server did not listen on port 7777` and never started the client. The server log
(block-buffered) ends at `[2XYXYXY] Clearnupworld ... -> (nil)` followed by
`LogNet: Browse: ...`-less silence, and the process sleeps at 0% CPU.

## Solo server check (no client, no netns, no probe)

`FortniteLinuxServer /Game/Maps/FortniteEmptyDedicated -nullrhi -nosound -unattended
-port=7777 -log -FortniteOriginalClient -stdout` on the host (run for 120 s, then
stopped by hand):

- Reached `LogNetVersion: FortniteLinux 1.0.0, NetCL: 0, EngineNetVer: 14 ... (Checksum: 326778485)`,
  `LogNet: Browse: /Game/Maps/FortniteEmptyDedicated?Name=Player`.
- No `IpNetDriver listening on port 7777` and no socket on 7777 after 20 s. The stdout
  log stopped growing (492 lines) with the process at 0% CPU.

The same server listened within a second of its browse in attempt 2
(`LogNet: GameNetDriver IpNetDriver_2147482579 IpNetDriver listening on port 7777`,
at 21:41:56.827). So the stall is intermittent. Attempt 2 is the only passing
readiness check on the socket, and it shows the listen happened. Host load (Ghidra
`analyzeHeadless`, about 130% CPU) is one candidate. The map load after Browse is
another. Neither has been confirmed.

## Blockers and next steps (ranked)

1. **Server stall between Browse and listen** (blocks every run). Reproduce it solo
   with `-stdout` and `-ini`/`-log` as before. Compare the working run's timings.
   Check whether the server's async loading waits on a package or on the host IO
   (disk is 93% full, 20 GB free). Confirm whether it repeats when Ghidra is idle.
   Owner: porting/native (only the server's map load; no client changes).
2. **Client connect path.** The first-token approach is blocked by UE's whitelist
   rewrite. A loose `GameDefaultMap` did not redirect. Next: test `-nomcp` (documented
   in the exe) with a live server; if the frontend still gates the connect, look for
   a Fortnite UI path that calls `Engine->Browse` with a typed address (e.g. the
   `UFortUIStateWidget_JoinServer` strings in the exe) and record each string
   before using it.
3. **Version override.** Verify that `GetFortniteNetworkVersion` is reached on NMT_Hello
   (`Local network version checksum: 683914758` in the server log). The current
   log shows only the stock value 326778485 at startup.
4. **Encryption.** Not tested. The client's `net.AllowEncryption` is unknown; the
   server reports `net.AllowEncryption = 1` with no key configured. Test with a random
   scratch key only after an unencrypted Hello is seen.
5. **Class checksums and NetGUID errors.** Not reached. Nothing can be logged until a
   connection exists.

## Result summary

- NMT_Hello accepted: no. No Hello reached the server.
- Remote network version seen by the server: none (no `Remote network version` line).
- Challenge / Login / Welcome / Join: none observed.
- Network checksum mismatch, NetGUID, class path errors: none observed.
- Client crash: none observed. The client was stopped by its 90/120 s timeout or
  stayed in frontend login.
- EAC, BattlEye, AppES: not patched or bypassed. The client's `LogAppES: postponed 2`
  appears in the frontend login, which is consistent with the earlier note that the
  AppES gate follows CheckEntitledToPlay. Not investigated further.

## Files changed

- `porting/run_original_direct.sh` (new): sibling runner, same netns and client flags
  as `run_original_client.sh` minus AUTH flags, no curl-verify memory patch.
- `porting/direct_connect_results.md` (new): this report.
- `porting/original_client_net_protocol.md`: added the observed step 0/1 status
  block only.
- Ignored, not tracked: `porting/generated/wine-prefix/.../Saved/Config/WindowsClient/Engine.ini`
  (loose net block, see the runner), `porting/generated/direct-connect/` logs.

No native code was changed and no UE build was run in this stage. The staged server
binary (built 21:09) was used as is.

## Process hygiene

- No leftover Wine, Xvfb or server processes from this stage. Ghidra was not touched.
- Temporary files under the session tmp folder were removed. The `porting/generated/direct-connect-Engine.ini.bak`
  copy was removed.
