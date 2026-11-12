# G11: running the vanilla client with no in-memory patches

Date: 2026-11-11. Nothing here has been run. Each section is a hypothesis with
the evidence for it and a test that confirms or kills it.

Today two patches are applied to the running client (file on disk untouched):
`server/patch_curl_verify.py` (TLS verification off) and
`server/patch_appes_diagnostic.py` (unsafe-environment handler and its 20 s
exit timer suppressed).

## 1. What "AppES" is

`LogAppES` is a log category in the client. It belongs to the startup
environment check that ends in the handler at 0x142121930. The handler takes a
reason code and shows a localized message, then the game closes:

| Code | Message key | Meaning |
|---|---|---|
| 1, 15 | `UnsafeEnvironment_BodyMissingLauncher` | "Fortnite was not started correctly … launch the game using the Epic Games Launcher" |
| 2, 3, 4, 5 | `UnsafeEnvironment_Body` | cheat or reverse-engineering tool detected |
| 6–9 | `UnsafeEnvironment_BodyCorruptedContent` | corrupted game files |
| 14, 16, 19, 20 | other notification / network error texts | not read |

The code that produces the reason is obfuscated (the functions around
0x14211d960 do not decompile to meaningful code, and the handler has no direct
caller, `lea` reference or vtable entry). Reading it statically is expensive.
The plan below does not need it.

Observed (existing notes `EXIT_TRACE.md`, `LAUNCHER_PROBE.md`):

- Direct start of the shipping exe: `closing code 1`, exit 20 s later.
- Start through the supplied `FortniteLauncher.exe` with `-nobe -noeac`, with
  the TLS memory patch active: `closing code 3`.

## 2. Launcher check (code 1): hypothesis and test

**Hypothesis:** code 1 is the direct result of our command line. Both run
scripts pass `-fromfl=none` and no `-fltoken` (see `run_original_client.sh`
line 117, `run_original_direct.sh` line 157). A launcher start passes
`-fromfl=<be|eac>`, `-fltoken=<token>`, `-epicportal` and related arguments.
The client contains the strings `fromfl`, `fltoken`, `skippatchcheck`, `nobe`.

**Test A (cheap):**
1. Run the supplied launcher once more and record the exact command line it
   gives the shipping client: the client log line `LogInit: Command Line:` or
   `wine` process arguments. The earlier run did not keep it.
2. Start the shipping exe directly with that same command line (same
   `-fltoken`, `-fromfl`, `-epicportal`), our auth arguments replacing the
   launcher's, AppES patch **off**, TLS patch on.
3. Result table: no `closing code` → launcher arguments were the whole check;
   `closing code 1` → the client also checks the parent process (go to test B);
   `closing code 3` → see section 3.

**Test B (if A still gives code 1):** the client checks that it was started by
the launcher / anti-cheat wrapper process. Start it as a child of a small
stand-in process named like the original parent (the launcher run shows which
executable is the direct parent: launcher or the BattlEye wrapper), with the
same command line. This is a process arrangement, not a patch.

## 3. Tool detection (code 3): hypothesis and test

In the launcher run the reason was 3 ("cheat or reverse-engineering tool").
Candidates, in order of likelihood:

1. **Our own TLS patcher**: it opens the client's memory for writing
   (`/proc/<pid>/mem`) while the check is running. Under Wine the client may
   not see that directly, but the modified `bVerifyPeer` bytes or a debugger
   flag could be what is detected.
2. The anti-cheat wrapper not running correctly under Wine (`-nobe -noeac`
   given together with a launcher token that says anti-cheat should be on).
3. Wine itself.

**Test C:** repeat the launcher run with the TLS patch **off** and TLS solved
by section 4 instead. If code 3 disappears, candidate 1 was the cause, and
both patches go away together. If it stays, vary `-fromfl=` / `-nobe -noeac`
against what the launcher passes.

## 4. TLS without a patch: hypothesis and test

**Evidence:** the client contains the stock UE strings
`Certificates/%s/cacert.pem`, `Certificates/cacert.pem`,
`Certificates/ThirdParty/cacert.pem`, `DebuggingCertificatePath` and
`PinnedPublicKeys` (all UTF-16, as in stock `SslCertificateManager.cpp`).
Stock 4.25 builds the trusted root list from the **first existing** of:

1. `<Project>/Content/Certificates/<Platform>/cacert.pem`
2. `<Project>/Content/Certificates/cacert.pem`
3. `<Engine>/Content/Certificates/ThirdParty/cacert.pem` (this is the one
   shipped: 138 certificates, inside the paks)

and then adds one extra certificate from `[SSL] DebuggingCertificatePath` in
the engine ini if set. Pins come from `[SSL] PinnedPublicKeys` in config; the
client's reader is at 0x1438bd850. No shipped ini under the extracted
`Config/` directories contains `[SSL]` or `PinnedPublicKeys`, so pins, if any,
arrive by hotfix ini from cloud storage, which is **our backend**.

**Hypothesis:** a loose file
`FortniteGame/Content/Certificates/Windows/cacert.pem` containing the 138
shipped certificates plus our local CA makes the client trust the backend with
`bVerifyPeer` left on. No code is touched; the client reads a file from its
own install directory.

**Test D:**
1. Make a local CA and a server certificate signed by it with the right
   subject alternative names for every host in `server/gen_hosts.py`
   (today `gen_tls_cert.sh` makes a bare self-signed certificate with
   `CN=local-fortnite-mock`; a verifying client needs matching names).
2. Write `Certificates/Windows/cacert.pem` = shipped bundle + our CA.
3. Run with the TLS patch off. Watch `LogHttp`/`LogSsl` in the client log.
4. Outcomes: requests succeed → done; "certificate verify failed" → the loose
   file was not used (try path 2, then `DebuggingCertificatePath` through a
   `Saved/Config/WindowsClient/Engine.ini` override); "pinned key" failure →
   make sure the hotfix `DefaultEngine.ini` our backend serves has no
   `PinnedPublicKeys`, or serve pins computed from our own certificate.
5. If the client only reads paks for that path (loose files ignored in
   shipping), fall back to adding a small extra pak; that modifies the install
   directory but not the executable. Record which.

The matchmaking websocket (`wss://`, today plaintext) uses the same trust
store through libwebsockets in stock UE; re-test it once D passes.

## 5. Packet encryption (related to G11, already answered)

The client's AES packet handler (`Incoming` 0x1409dd1c0) is the stock
`FAESHandlerComponent`: same control flow and messages, decrypt through
`STAT_OpenSSL_AES256_ECB_Decrypt` (stock PlatformCrypto, AES-256-ECB). The
stock server-side component is compatible. No work.

## 6. Order

D (TLS) first: it is independent and removes the memory writer. Then C, then A,
then B only if needed. Record every run in a table (arguments, patches on/off,
closing code, time of exit) in `EXIT_TRACE.md`.
