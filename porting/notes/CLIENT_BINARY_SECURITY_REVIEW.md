# Downloaded client security review

Reviewed 2026-11-13. No malware detections, signature/content mismatches,
tampered `.pak` chunks, malicious blueprint API calls, or unexpected external
URLs/domains/IPs were found across the supplied installation's 151 PE binaries,
31 `.pak`/`.sig` archives, loose files, or 909,364 extracted assets/configs.
This substantially increases confidence in the distribution set, but does not
certify the entire project as safe.

## Scope and results

The review recursively inventoried `/mnt/vibrato/Fortnite` (Engine and
FortniteGame binaries, `.pak`/`.sig` archives, and loose files), the 32.5 s
decrypted shipping runtime dump, and the 909,364 non-`.ubulk` extracted files in
`/mnt/vibrato/fortnite-extracted-full`. All checks were local and read-only on
the reviewed artifacts; no files were uploaded.

| Check | Result |
|---|---|
| ClamAV scan | 151 files scanned; 0 infected; exit code 0 |
| Embedded PKCS7 signature cryptography | Passed for all 151 files |
| Authenticode file digest against signed digest | Matched for all 151 files |
| Certificate padding, table placement and PE section geometry | No anomalies in the checks performed |
| Duplicate filenames | All 55 duplicate-name groups had identical signed contents |
| `.pak`/`.sig` RSA-4096 master signatures | 31 / 31 matched `SHA-1(ChunkHashes)` against shipping `.exe` key |
| `.pak`/`.sig` 64 KiB chunk CRC32 tables | 31 / 31 `.pak` files passed with 0 chunk mismatches |
| Binaries, runtime dump & loose files URL/domain/IP scan | 294 files + runtime dump; no injected/unexpected endpoints |
| Extracted assets, configs & blueprints scan | 909,364 files (`0` missing); no malicious blueprint APIs or endpoints |

Coverage included NVIDIA Aftermath, GfeSDK and PhysX; Intel Embree, TBB and
TBBMalloc; Chromium/CEF, Discord, Vivox, Oodle, Microsoft runtime and audio
libraries; the launchers; and BattlEye/EasyAntiCheat components. No standalone
EOS SDK DLL was present in the installation.

The verified signature objects identified Epic Games Inc. as signer for 146
files, BattlEye Innovations e.K. for 2, and EasyAntiCheat Oy for 3. Vendor
libraries in this distribution were signed by Epic rather than necessarily
carrying their original vendor's signature.

ClamAV 1.5.4 reported 3,628,269 known signatures. The expanded scan ran from
16:11:43 to 16:14:51 EST (UTC-05:00) on 2026-11-13, reading 356.24 MiB and reporting
269.51 MiB scanned. Configured limits were 30 seconds per file, 300 MiB maximum
file size and 600 MiB maximum scan size. No scan errors or limit warnings were
reported; these totals do not establish that every byte was semantically
inspected.

For the signature checks, OpenSSL verified PKCS7 cryptography with certificate
trust checking disabled (`smime -verify -noverify -binary`). A read-only Python
PE parser independently computed the Authenticode digest, excluding the checksum,
security directory entry and certificate table, and compared it with the signed
digest. Every comparison matched. Certificate-table tails and padding were also
checked for unexpected nonzero bytes.

Main shipping executable SHA-256, identifying the reviewed copy:

```text
f7ab5f95e666f6dc066699896126a50d267328b9e923e1973cee29b34c08010c
```

## Interpretation and remaining limits

The coherent Fortnite/Unreal decompilation and runtime research provide further
evidence that the core client is genuine. They address compatibility and game
behavior rather than exhaustive malicious-behavior analysis. The existing
[binary analysis](../binary_analysis.md#2-packed-or-encrypted-on-disk-observed--inferred)
explicitly leaves startup protection/decryption and anti-cheat investigation
incomplete.

Signature cryptography and matching signed contents do not by themselves establish
publisher trust. An additional chain check against the Linux system roots, with
date checks disabled, succeeded for the sampled EasyAntiCheat certificate. It
could not complete the sampled Epic and BattlEye chains because local issuer
certificates were unavailable. Complete Windows trust-chain, timestamp and
revocation validation remains outstanding, as does comparison with an independently
trusted official distribution manifest.

No exhaustive security audit of all project source, tools, dependencies, assets,
Wine components or runtime behavior was performed. Antivirus results and valid
signatures cannot guarantee malware absence. The runner's private network
namespace reduces network exposure but does not provide full host-filesystem
isolation. These findings apply to the reviewed files at this point in time.

## Continuation: `.pak` signatures, blueprints, and URL/domain/IP scans

Extended read-only checks on 2026-11-13 covered `.pak`/`.sig` archive integrity,
embedded URLs/domains/IPs across binaries and loose files, and the extracted
assets and blueprints in `/mnt/vibrato/fortnite-extracted-full`.

### 1. `.pak` and `.sig` cryptographic verification

All 31 `.pak` and 31 `.sig` pairs in `/mnt/vibrato/Fortnite/FortniteGame/Content/Paks`
were verified directly against Unreal Engine's `FPakSignatureFile` format
(`0x73832DAA`, version `1`, 64 KiB chunk CRC32 table, RSA-4096 PKCS#1 v1.5
signature over `SHA-1(ChunkHashes)`):

- **Signing key:** Located the 4096-bit RSA public modulus at `.rdata+0x7327c0`
  in `FortniteClient-Win64-Shipping.exe` (little-endian modulus SHA-256
  `d36fb723ac0fe70f780a38e153f64cd066eaddf1546cd7bc58da8fb0ab29aa0b`, `e = 65537`).
- **Master signature verification:** **31 / 31** `.sig` files decrypted cleanly
  and matched `SHA-1(ChunkHashes)`.
- **Chunk CRC32 verification:** **31 / 31** `.pak` files matched 100% of their
  64 KiB chunk CRC32 entries in `.sig` with **0 mismatches**.

### 2. Binaries, runtime dump, and loose files (URLs / domains / IPs)

- **294 non-pak files** in `/mnt/vibrato/Fortnite` (all 151 PE binaries, CEF3
  `.pak`/`.dat` resources, BattlEye/EAC `.ini`/`.bat`/`.json`/`.cer`/`.txt`,
  `.srt` subtitles; skipping `Content/Paks/`, `.sig`, `.mp4`, `.bmp`, `.png`,
  and IDA sidecar `.id0`/`.id1`/`.id2`/`.nam`) plus the decrypted runtime image
  `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` were
  scanned for ASCII and UTF-16LE URLs, IPv4 strings, and domains (`1,644` URLs,
  `88` IP matches, `1,945` raw domain matches).
- A string-bounded domain pass across the core client, launcher, EAC, BattlEye,
  `CrashReportClient.exe`, and `UnrealCEFSubProcess.exe` confirmed all external
  domains were expected Epic Games / Tencent (`*.fortnite.qq.com`), OAuth,
  Vivox, CA/CRL/OCSP, or third-party library strings (plus UE4 gameplay tags
  and HLSL/C++ symbols).
- Non-loopback/non-version IPs in the client binaries reduced to Vivox SIP SDP
  example strings (`69.90.155.77`, `69.90.168.6` in `vivoxsdk_x64.dll`), while
  `1.2.0.4` (zlib version) and `2.2.2.2`/`6.6.6.6`/`9.9.9.9` were zlib version
  and ASCII lookup-table bytes.
- Loose BattlEye/EAC scripts and configs (`BELauncher.ini`,
  `Install_BattlEye.bat`, `Uninstall_BattlEye.bat`,
  `EasyAntiCheat/Launcher/Settings.json`) were inspected directly; `game.cer`
  is issued to `CN=Fortnite, O=Epic Games` by `EasyAntiCheat Oy`.

### 3. Extracted assets, configs, and blueprints (`/mnt/vibrato/fortnite-extracted-full`)

Using the 963,427-entry index in `porting/generated/assets.json`, all **909,364**
`.uasset`, `.uexp`, `.umap`, `.ini`, `.uproject`, `.uplugin`, `.upluginmanifest`,
`.json`, `.locres`, `.po`, and `.archive` files were scanned (0 missing; `.ubulk`
bulk blobs skipped):

- **Blueprint / native API scan:** Checked `.uasset`, `.uexp`, and `.umap`
  files for 29 dangerous native API/execution keywords (`LaunchURL`,
  `ExecuteConsoleCommand`, `BlueprintFileUtils`, `FFileHelper`,
  `FPlatformProcess`, `CreateProc`, `ShellExecute`, `LoadLibrary`,
  `WebBrowserWidget`, `UWebBrowser`, `SWebBrowser`, `IHttpRequest`,
  `FHttpModule`, `HttpRequest`, `VaRest`, `TcpSocket`, `UdpSocket`,
  `PythonScript`, `ScriptPlugin`, `ClipboardCopy`, `ClipboardPaste`,
  `GetEnvironmentVariable`, `powershell`, `cmd.exe`, `wscript`, `mshta`,
  `rundll32`, `regsvr32`, `certutil`). Only two matched:
  - `LaunchURL`: 11 matches across 9 UI assets (`LlamaStoreRoot`,
    `PrerolledLlamaStoreRoot`, `GiftingScreen`, `BladeMenu_MainMenu`,
    `MainMenu`, `PendingAccountMergeNotification`, `AffiliateKeyModal`,
    `AthenaNewsModal`, `CreativeIslandToolsPanel`), all tied to standard
    Fortnite URL properties or `https://www.epicgames.com/fortnite/faq#creative`.
  - `ExecuteConsoleCommand`: 14 assets, all invoking benign rendering/debug or
    gameplay commands (`r.DistanceFieldAO`, `r.HDR.*`, `r.ShadowQuality`,
    `r.screenpercentage 100`, `highresshot filename=`, `pause`,
    `PauseMissionTimer`).
- **URLs, IPs, and domains:** Scanned all 909,364 files (`85` URLs, `157` IP
  matches, `1,940` domain matches). All URLs were Epic/UE4, OAuth (Google,
  Facebook, Twitch), AWS S3, Vivox, NVIDIA, third-party legal credits,
  `www.ustream.tv` (`FMS_OutpostVictory_Streamed.uexp`), `www.youtube.com`
  (`PublishModal.uexp`), or `www.surveymonkey.com/s/93YGFVZ` (`DefaultGame.ini`).
  All real IPs were in `.ini` configs (`DatacenterDefinitions` AWS/GCP ping
  targets, `127.0.0.1`, `0.0.0.0`, `239.0.0.1`, `1.0.0.0`); `.uexp` IP-like
  matches (such as `8.8.8.8` in `Apollo_Papaya_LS_B3.uexp`) were verified to be
  raw binary asset bytes. Non-allowlisted domain matches were UE4 gameplay tags,
  Google OAuth client IDs, `www.surveymonkey.com`, or random binary byte matches
  in `.uexp` textures/meshes/audio.

### Remaining scope limits

- `.pak`/`.sig` verification proves the archives were not modified relative to
  the RSA key embedded in `FortniteClient-Win64-Shipping.exe`, which still
  inherits the Windows certificate-chain trust caveat above.
- Bulk `.ubulk` blobs (70.0 GB), `.mp4` videos, IDA sidecar files
  (`.id0`/`.id1`/`.id2`/`.nam`), and any `FortniteClient-Win64-Shipping.exe`
  code pages decrypted after the 32.5 s runtime dump snapshot were not scanned
  for strings.
- Blueprint checks used keyword/API scanning rather than full Kismet bytecode
  decompilation, mitigated by 100% `.pak`/`.sig` cryptographic verification.
