# Downloaded client binary security review

Reviewed 2026-11-13. No malware detections or signature/content mismatches were
found across the supplied installation's 143 DLLs and 8 EXEs. This increases
confidence in the binary set, but does not certify the entire project as safe.

## Scope and results

The review recursively inventoried `/mnt/vibrato/Fortnite`, including both Engine
and FortniteGame binaries. All checks were local; no files were uploaded and no
client binaries were modified or executed for these checks.

| Check | Result |
|---|---|
| ClamAV scan | 151 files scanned; 0 infected; exit code 0 |
| Embedded PKCS7 signature cryptography | Passed for all 151 files |
| Authenticode file digest against signed digest | Matched for all 151 files |
| Certificate padding, table placement and PE section geometry | No anomalies in the checks performed |
| Duplicate filenames | All 55 duplicate-name groups had identical signed contents |

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
