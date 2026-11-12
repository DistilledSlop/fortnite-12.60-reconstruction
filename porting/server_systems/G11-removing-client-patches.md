# G11: running the vanilla client with no in-memory patches

Updated: 2026-11-12. **G11 remains OPEN:** secure local TLS trust and removal
of the client memory diagnostics have not been achieved. Sections 2–3 retain
the November 11 hypotheses; section 4 records the actual November 12 probes.

The original diagnostic setup applies two patches to the running client
(file on disk untouched):
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

## 4. TLS trust: November 12 findings and remaining work

The original 12.60 client was tested in a private network namespace with an
isolated Wine prefix and a private Content overlay. Its certificate platform
is **Windows**, and its project directory is `../../../FortniteGame/`.

**Trust probe:** the local self-signed CA/server certificate with the required
host names was supplied as a private loose
`FortniteGame/Content/Certificates/Windows/cacert.pem`. With the Curl
verification diagnostic off, requests still failed with OpenSSL
`SSL certificate problem: certificate not trusted`. This did not solve trust.
A live read showed **122 root certificates and six pinned domains**. The
root count does not prove adoption of the loose CA. Stock SSL verification
also sets `X509_V_ERR_CERT_UNTRUSTED` on pin or verification-delegate rejection,
so this error alone does not establish whether root loading failed.

**Pins are compiled into game initialization.** The previous claim that pins
must come only from backend hotfix INIs is superseded. All 15 shipped Engine
Config and 29 FortniteGame Config INIs in the full extraction lack `[SSL]`,
`PinnedPublicKeys`, and `DebuggingCertificatePath`, but the actual initializer
at `0x1422ec980` obtains the SSL certificate manager, clears its pins at
`0x1422eca87`, and sets these domains through virtual slot `+0x30`:

| Domain | SetPinnedPublicKeys call |
|---|---|
| `.ol.epicgames.net` | `0x1422ecad9` |
| `.ol.epicgames.com` | `0x1422ecb3b` |
| `.on.epicgames.net` | `0x1422ecb9d` |
| `.on.epicgames.com` | `0x1422ecbff` |
| `.ak.epicgames.com` | `0x1422ecc77` |
| `.fortnite.qq.com` | `0x1422eccf7` |

These match the six live entries. The initializer reads compiled digest
constants and has no configuration reads or conditionals. Its direct caller
is `0x1422b8cec` in protected function `0x1422b8b10`; an outer configuration
gate has not been established. The SSL module's initial INI reader at
`0x1438bd850` therefore does not establish a lasting pin override. The manager's
digest verifier is `0x1438bda50` (slot `+0x38`), and its X509 verifier is
`0x1438bdc10` (slot `+0x40`); their protected bodies and any bound verification
delegate remain unresolved.

**Configuration and packaging limits:** `Saved/Config/.../Engine.ini` is
ignored by this cooked shipping build. Actual `LoadExternalIniFile` at
`0x142c77618` forces generated overrides off for non-`GameUserSettings` files
at `0x142c77650` / `0x142c776dc`. Thus the proposed Saved Engine INI
`DebuggingCertificatePath` or pin-array override is superseded. A supported
source-INI trust override remains unproven. Shipping also excludes the stock
`-ini:` overrides, `DisableSSLCertificatePinning`, and
`OverrideCertificateBundlePath` facilities. A private unsigned certificate
pak was rejected for its missing Epic signature; the proposed extra-pak
fallback is superseded.

The stock root search remains useful evidence, not a successful fix: it uses
the first existing project platform PEM, project generic PEM, or Engine
`Certificates/ThirdParty/cacert.pem`, then an optional
`[SSL] DebuggingCertificatePath` certificate. Actual loose-file visibility
and CA adoption must be established separately from pin acceptance. Repeating
the historical Test D with only a Saved INI change or an unsigned pak is not
an actionable next step.

### Opt-in WSS diagnostic, not a trust solution

`server/patch_wss_diagnostic.py` is an explicitly opt-in, runtime-only
certificate-validation bypass for the private network namespace. It checks
both the branch context and existing target context, then changes opcode
`75` to `EB` at `0x144866e35`, selecting the existing validation-disabled
branch. It requires root, rejects the host network namespace, and only
considers client processes in its own namespace; it leaves the EXE on disk
untouched.

In the latest November 12 run, it was applied after the AppES diagnostic and
Frontend readiness. Together with the Curl and AppES diagnostics, this let
the original client reach WSS open, SASL authentication, stream reopen, and
bind IQ. After accepting its unqualified stanza fragments, the next probe
completed binding, session setup and presence. The client logged XMPP
`LoggedIn`. Its initial party-success callback was misleading: HTTP capture
showed create-party returning 400 for the client's joinability value, followed
by retries in a disconnected party state. Subsequent backend fixes accepted the
observed config and nested root PATCH: later captures show create=200 and both
root/member-meta PATCH=204. A later client also accepted and removed a friend
and displayed a live incoming-request toast. Party notification fields still
need parity fixes, and two-client social/chat flows remain unverified.
An earlier run that applied the WSS
diagnostic before login exited with process status 5; causation is unproven.
This progress validates more of the backend protocol path while **secure
trust and G11 remain OPEN**. The bypass is not the final solution.

### Controlled Windows ROOT-store probe

A subsequent private capture of Windows loader `0x1438bc8d0` established that it
calls the base PEM loader, defaults `bUsePlatformProvidedCertificates` to true,
opens Windows `ROOT` with `CertOpenSystemStoreW`, enumerates its certificates,
decodes their DER and inserts them into the SSL manager. Private evidence:
`local-model-package/captures/3e035b4d80ed4a7ba21e5a5146c1bdbd/`.

A fresh valid self-signed CA was generated only for the ignored
`generated/g11-root-store-probe/` experiment. A small Crypt32 tool checked the
exact DER certificate in the isolated Wine prefix: ROOT contained 125 certificates
with zero matches before enrollment and 126 with one exact match afterward.
The host trust store was not modified. The older probe certificate was already
enrolled, so repeating that import would not have tested a new trust input.

The client was restarted with Curl verification bypass **off**, WSS bypass **off**,
AppES callback diagnostic **on**, and the Oodle diagnostic off. Wine's built-in
renderer was sufficient for this service-only check. Curl still failed with
error 60 / `certificate not trusted`, and the HTTPS listener reported
`SSLV3_ALERT_BAD_CERTIFICATE`; login did not reach a WSS trust test. A read-only
inspection found one SSL manager with 123 roots and six pinned domains, compared
with 122 roots in the earlier probe. This supports platform-store adoption but
does not identify the new CA inside that array or independently prove that pins
caused the rejection. The client remained alive until the 180-second deadline.

ROOT enrollment alone therefore does not solve G11. The next investigation is
the actual X509/digest verifier and its verification delegate or a supported
trust configuration. Do not repeat an unchanged certificate import as a fix.

## 5. Packet encryption (related to G11, already answered)

The client's AES packet handler (`Incoming` 0x1409dd1c0) is the stock
`FAESHandlerComponent`: same control flow and messages, decrypt through
`STAT_OpenSSL_AES256_ECB_Decrypt` (stock PlatformCrypto, AES-256-ECB). The
stock server-side component is compatible. No work.

## 6. Order

Resolve root-CA adoption and compiled pin acceptance first, using the actual
shipping behavior above. The historical Test D assumptions are superseded;
the WSS diagnostic only enables isolated protocol investigation. Once secure
TLS works without the diagnostics, revisit C, A, and B as needed. Record each
run with arguments, diagnostics enabled, closing code, and exit time in
`EXIT_TRACE.md`.

### Original verification callback recovered

Private machine-code recovery locates Curl SslCertVerify at `0x1438cc1e0`.
Entry ECX is the OpenSSL PreverifyOk input; RDX is X509_STORE_CTX. Only an
input of 1 reaches the manager verifier call at `0x1438cc253` (slot +0x40).
AL at `0x1438cc257` is its boolean result; EBX at `0x148f0461f` is the final
callback result. A false manager result changes it to zero. These boundaries
permit a diagnostic observation that distinguishes chain preverification from
a later manager rejection. A false manager result alone still does not prove
which pin or delegate rejected the chain. Calls must be paired per thread and
invocation because callbacks may run for multiple chain certificates.

The private evidence index is
`/opt/fortnite/local-model-package/work/g11-callback-boundary-evidence.json`, with principal
capture `4aa6dc35167d4aee920f8ff34ba640a0`. The ignored diagnostic observer uses
hardware breakpoints and logs only thread IDs and verifier booleans. Debugger
observation is separate from G11 acceptance; no patch-free success is claimed.

### Live callback observation with enrolled ROOT certificate

A subsequent isolated run kept Curl and WSS bypasses off and AppES diagnostics
on, with the same fresh CA enrolled in the private Windows ROOT store. After
the protected callback code became available, hardware-breakpoint observation
repeatedly recorded PreverifyOk=1, manager=false, final=false on the request
thread. Thus OpenSSL preverification passed for these observed invocations,
and the additional manager check rejected the connection. This narrows the
remaining Curl failure to that manager check rather than base CA trust. The
manager’s digest/pin check versus its optional verification delegate is not
yet isolated. The client still failed to sign in without a TLS bypass.

Only verifier booleans and thread IDs were recorded in ignored
`generated/g11-root-store-probe/verify-observation.out`; no certificate data
or request credentials were collected by the observer. This was an instrumented
diagnostic, not a successful G11 acceptance run.

### Diagnostic Curl option address confined

The recovered sslctx_function tests the bVerifyPeer byte at `0x147c69478`
(at original `0x140c593b3`). The diagnostic formerly wrote every aligned match
of its generic option signature; recent runs matched another address too. A
matching byte pattern does not establish that other object as the intended
option. The diagnostic now validates initialization at the observed address
and changes only that one byte, with write readback and a private-namespace
requirement. This remains a verification bypass. It has not been established
that the former extra write caused any client exit.
