# Binary analysis of the shipping client exe

Scope: read-only static analysis of `Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe` (162.6 MB, PE32+, x64). Nothing was unpacked, patched or executed, and no protection, anti-tamper or entitlement logic was analysed or bypassed. Tags: **[observed]** = read directly from the file; **[inferred]** = reasoning from observed data; **[project note]** = taken from existing repo docs and not re-checked here.

## 1. Section table [observed]

Parsed with a stdlib-only reader (pefile is not installed). Entropy is Shannon bits/byte over the raw section bytes.

| Section | Virtual size | Raw size | VA | Flags | Entropy |
|---|---|---|---|---|---|
| `.text` | 93,301,760 | 93,301,760 | 0x1000 | RX | 7.99 |
| `.rdata` | 32,594,944 | 32,594,944 | 0x58FC000 | R | 5.37 |
| `.data` | 12,512,208 | 9,158,656 | 0x7812000 | RW | 3.24 |
| `.pdata` | 5,744,128 | 5,744,128 | 0x8401000 | R | 7.14 |
| `_RDATA` | 160,768 | 160,768 | 0x897C000 | R | 5.51 |
| `.rsrc` | 45,056 | 45,056 | 0x89A4000 | R | 7.47 |
| `.reloc` | 3,656,704 | 3,656,704 | 0x89AF000 | R | 5.47 |
| `.text` (second) | 17,962,496 | 17,962,496 | 0x8D2C000 | RX | 7.85 |

- Linker 14.24 (MSVC), compile timestamp 2020-05-15, image size 0x9E4D600, entry RVA 0x580476C (in the first `.text`), Authenticode certificate of 13,056 bytes at end of file. [observed]
- Two `.text` sections with the same name. A normal MSVC/UE link produces one. [observed]
- Import table: 50 DLLs, 1,067 named imports (KERNEL32 243, MSVCP140 160, USER32 120, WS2_32 45, and others), a normal-looking table. Delay-import directory 576 bytes, IAT 8,936 bytes. Imports include `oo2core_5_win64.dll` (Oodle), the XAudio family (X3DAudio, XAPOFX), D3D11/DXGI, OpenGL and WinHTTP. [observed]

## 2. Packed or encrypted on disk? [observed + inferred]

**Verdict: code is encrypted or obfuscated on disk. It is not a conventional packer. The reflection tables in `.rdata` are plaintext.**

Evidence:

1. **Code entropy.** Every 1 MiB window of the first `.text` is between 7.52 and 8.00 bits/byte (89 of 89 windows at or above 7.5). Compiled x86-64 code is typically around 6 bits/byte [inferred, general knowledge]. Data sections behave normally: `.rdata` 4.0–5.6 bits per window (two windows near 7.0–7.2, not investigated), `.data` about 2.6. [observed]
2. **Byte histogram is neither uniform nor normal code.** 0x00 is 0.44% of `.text` (uniform random expectation 0.39%). 0xCC is 1.26%, about three times uniform, which fits int3 padding but is far too few bytes for code. [observed]
3. **Unwind table is plaintext, function bodies are not.** `.pdata` has 478,677 RUNTIME_FUNCTION entries. 478,638 begin inside `.text`, and 18 are out of order and 40 have implausible lengths. The first three bytes at each function start show no common prologue (`sub rsp`, `push rbx`, `mov [rsp+..]`). The most frequent 3-byte prefix in the first 200,000 functions occurs 3 times. [observed]
4. **Entry stub.** The entry point at file offset 0x5803D6C is `48 83 EC 28` (`sub rsp,0x28`), then `E8` (`call`) to 0x8D2F877 in the second `.text`. That target starts with a register-save sequence (`push rax; push rdx; push rdi; push rbx; push r12 ...`). The bytes after the call (file 0x5804775) decode to `fcom`, `sahf`, `lods` and similar nonsense. [observed]
5. **Decode failure rate.** `objdump -b binary -m i386:x86-64` gives 10.8% `(bad)` lines for 64 KiB at +1 MiB of the first `.text`, 10.9% at +4 MiB of the second, and 3.8% in the last 2 MiB of the second. [observed] Real code is far lower [inferred]. The last ~4 MiB of the second `.text` has entropy 5.46–6.05, which is more typical of plaintext code or tables. It was not investigated further.
6. **[project note]** `porting/README.md` says the original client is used under Wine as a dynamic oracle for "decrypted code". This is consistent with code that is decrypted at run time. It was not re-verified here.

Not analysed, per the rules: the decryption or unpacking routine, the protection layer that owns the second `.text`, and the EasyAntiCheat and BattlEye components beside the exe.

## 3. Reflection data recovery [observed; script: `porting/extract_reflection.py`]

Layouts come from the UE 4.25.0 engine headers (`UObjectGlobals.h`, `Class.h`, `ObjectMacros.h`, `Script.h`) at `/opt/fortnite/UnrealEngine`, assuming `WITH_METADATA` off (shipping). Header-level layout reference only; no game data was read outside the exe.

- **Recovered:** 6,967 `FClassParams`-shaped blocks across all native modules (80-byte layout; validated by `CLASS_Native` set, consistent counts and pointer types). Each decodes to config name, class flags, dependency pointers, function table (`FClassFunctionLinkInfo`, names as ASCII pointers) and property table (`FPropertyParamsBase` with per-kind offsets). Property names, `EPropertyFlags`, `EPropertyGenFlags` kinds and `CPF_Net` replication flags decode.
- **Validation:** the engine `AActor` block (config `Engine`) decodes to the expected replicated set: Role, RemoteRole, Owner, AttachmentReplication, ReplicatedMovement, bReplicateMovement, bTearOff, bCanBeDamaged, bHidden, Instigator. [observed]
- **Function flags:** 22,730 distinct `NameUTF8` pointers from `FFunctionParams`-shaped records. FUNC_Net flags join by pointer equality, with an "ambiguous" marker where more than one record shares a pointer.
- **Package:** `/Script/FortniteGame` FPackageParams is at 0x146475C78, declaring 775 singletons. [observed]
- **Groups:** class blocks are grouped by dependency-0 (the package function, inferred). 22 groups with at least 20 blocks contain Fort or Athena names: 2,740 blocks, 863 replicated properties, in the output.

### What is not recovered: the 8 target classes

**None of the eight target classes is named.** The script writes `name: null` for every block and records each target as `unresolved`.

- The 756-block dependency-0 group (`0x142EEA530`) contains engine `AActor`, so it is the Engine module, not FortniteGame. [inferred, from the Actor replicated set]
- The `/Script/FortniteGame` package declares 775 singletons. No dependency group clearly matches that count. Dependency-1 values do not fall in the `/Script/Engine`, CoreUObject, GameplayAbilities or UMG singleton sets as extracted from their package records, so dependency-1 could not be confirmed as the parent. [observed]
- Plaintext `Class::Member` strings in `.rdata` exist for FortGameStateAthena, FortPlayerControllerAthena, FortPlayerPawnAthena and FortPlayerStateAthena. None of their members matched a function or property name in any dependency group with at least 20 blocks, so they cannot tie a name to a block. FortWorldSettings, FortAthenaMapInfo, FortPoiManager and FortWorldManager have no such strings. [observed]
- MSVC RTTI type names are present (611 `.?AV` entries), but none is an `AFort*` class. [observed]
- The code that names each class (`Z_Construct_UClass_*`, `Z_CompiledInDefer`, the `TEXT("...")` arguments) is in the encrypted `.text`. That is the blocker. [inferred]

The UTF-16 class names in `.rdata` are unordered literals with no pointer back to a class block. [observed]

### Fallback

1. Use cooked data for names where no pak key is needed. `porting/generated/extracted/` already holds loose cooked packages, for example `BP_SafeZoneStorm` (super `FortSafeZoneStorm`) and `DefaultMapInfo` (super `FortAthenaMapInfo`), confirmed in the existing `*-functions.json` reports. These give native parent names for Blueprints but not the native property tables. The README documents `--loose` for extracted packages; combining it with `--function-report` was not tested here.
2. Pass `--name-map` (JSON `{block_va: class_name}`) to the script once a name is established. The output carries the names.
3. Companion IDA files `FortniteClient-Win64-Shipping.exe.id0` (179 MB), `.id1` (636 MB) and `.nam` sit beside the exe in the supplied tree. They may carry symbol names, but the task limited this analysis to the exe, so they were not opened. Whether to use them is for the orchestrator to decide.

## 4. Replication implications (for `original_client_net_protocol.md` section 2)

- The reflection layout from this exe is reproducible with `porting/extract_reflection.py`. Output goes to `porting/generated/reflection/fortnite_game_reflection.json` (ignored by git).
- Blockers for step 3 of the staged plan: class names and module membership (section 3), and the parent chain, because dependency-1 could not be confirmed as the parent.
- Replicated property counts are small: 863 `CPF_Net` properties across the 2,740 blocks in the Fort/Athena groups. This decode is validated on `AActor` (section 3), but the low count for Fortnite classes has not been checked against live behaviour. Verify before using it for wire order.

## 5. Rerun

```
python3 -I porting/extract_reflection.py [--exe PATH] [--out PATH] [--name-map JSON]
```

Runtime is about 25 seconds, stdlib only, read-only on the exe.

## 6. Runtime dump

Scope: a read-only copy of the shipping exe image from our own Wine run. The process was not patched, and the exe on disk and the EAC/BattlEye/AppES components were not touched. Nothing in this section relies on the supplied `.id0`, `.id1` or `.nam` files, which were not opened.

### Method [observed]

- **Launch.** `sudo -n python3 -I porting/dump_runtime_image.py --timeout 200 --interval 0.05` was started first. Then `FN_PATCH_VERIFY=0 porting/run_original_client.sh 220`. The repo's `bVerifyPeer` write (`patch_curl_verify.py`) was disabled, so the run wrote nothing to the process. The dumper attached to the Wine process within about 3 s of launch. The dump run never reached login: libcurl error 60 is logged, as expected without that patch. No "not started correctly" dialog appeared in this run.
- **Reads only.** The dumper reads `/proc/<pid>/maps` and `/proc/<pid>/mem` with `pread`. It never attaches with ptrace and never writes. It needs root because `ptrace_scope` is 1. Unreadable pages are zero-filled and counted; the accepted dump has 0.
- **Polling.** Every 50 ms it reads eight 64 KiB slices spread over each executable section and computes their entropy. A full read and verification run only when the mean is near code levels.
- **Verification.** (a) The median 1 MiB-window entropy of each executable section must be at or below 6.8 bits/byte. (b) objdump (`-D -b binary -m i386:x86-64`) decodes 400 functions evenly sampled from `.pdata`, each from its own start, and the share of `(bad)` lines among all decoded instructions must be at most 5%. Plain sample slices from arbitrary offsets were used first and were replaced: they start mid-function, and they reported about 6.7% `(bad)` lines.
- **Conversion.** `porting/dump_to_pe.py` writes a PE whose section raw pointers equal the virtual addresses, with the original headers kept. The output has the same bytes as the dump and is 165,991,936 bytes long, equal to SizeOfImage.

### Timing [observed]

| Process age | Sample entropy | Result |
|---|---|---|
| 24.65 s | 7.12 | full read: `.text` (first) median ~6.6, second `.text` median 7.997; decode 2.9% `(bad)` lines, 305/400 functions clean |
| 32.51 s | 6.77 | **accepted**: all verified checks pass |

The decryption finishes between about 25 and 32 s after process start. The first full attempt at 25 s was still partly encrypted. An earlier run, launched before the `.pdata` check existed, retried every 8 s from 40 to 104 s. Its first `.text` median stayed at 6.697, and its arbitrary-offset slice check reported about 6.7% `(bad)` lines throughout. Those numbers are from that earlier run and were not measured in the accepted one.

### Verification numbers (accepted dump) [observed]

- **Dump file.** `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin`, 165,991,936 bytes, sha256 `604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`. Metadata is in `porting/generated/dump/dump.json`. Both are git-ignored.
- **`.text` (RVA 0x1000, 93,301,760 bytes).** 88 windows of 1 MiB. Entropy min 5.958, median 6.515, max 6.72. Zero windows at or above 7.5. Share of sampled bytes equal to the disk file: 0.5%.
- **`.text` (RVA 0x8D2C000, 17,962,496 bytes).** 17 windows. Entropy min 5.457, median 6.734, max 7.998. Five windows at or above 7.5, covering RVA 0x942C000 to 0x9A2C000 (about 5 MiB). Those bytes are identical to the disk file, and the disk file's entropy there is also 7.997. The process never changed them, and no `.pdata` entry covers them, so they are not decoded function code. Share equal to disk for the whole section: 60.0%.
- **Decode.** `.pdata` has 432,866 functions starting in code. 400 sampled: 400 of 400 decode with no `(bad)` line, 22,120 instructions, 0 `(bad)`. The first attempt at 25 s decoded 305 of 400 clean (2.9% `(bad)` lines), which is the partial-decryption state.
- **Other sections, share equal to disk (64-byte stride).** `.rdata` 0.9997, `.data` 1.0, `.pdata` 0.9999, `_RDATA` 0.9972, `.rsrc` 0.9957, `.reloc` 0.9999. Read-only and data sections are unchanged, as expected.
- **Unreadable pages.** 0.

### Caveats [inferred unless marked]

1. The dump is one snapshot at 32.5 s, before login. Code that the client decrypts later (login, lobby, gameplay) may be absent or still encrypted. Check each function you need with the `.pdata` decode check before relying on it. The five 1 MiB windows noted above are a known unknown.
2. The entropy and decode checks sample; they do not cover every function. A clean sample is good evidence, not proof for every function.
3. `.data` is runtime state from this run. It is not the disk file, and it does not carry the original initial values.
4. The per-section share in `dump.json` used to collide on the name `.text`. It is keyed by name and address from this point on. The numbers above were recomputed from the dump with the same 64-byte stride.
5. Rerun: `sudo -n python3 -I porting/dump_runtime_image.py` (start it before the client), then `porting/dump_to_pe.py porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin -o porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe`.

## 7. Class name recovery

Scope: names for the FClassParams blocks in the runtime dump (section 6). Read-only Ghidra analysis of `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe`. The original exe and `Fortnite/` were not touched. Section 3 above is superseded for names: the eight targets now resolve.

### Analysis runs

- **Full run, project `fortnite` (default analyzers, 16 GB heap).** It died with `java.lang.OutOfMemoryError: Java heap space` after about 70 min, inside `AddressTableAnalyzer` (Create Address Tables), stack `AddressSet.subtract`. Nothing was saved. The project directory is empty and was not reused.
- **Slim run, project `fortnite-slim`.** Import with `-noanalysis` (log `porting/generated/ghidra/slim-import.log`). Then `analyzeHeadless ... -process <exe> -preScript SlimAnalysisOptions.java -max-cpu 8 -analysisTimeoutPerFile 3000`, with `GHIDRA_HEADLESS_MAXMEM=48G` and a 90-minute timeout. Result: "Analysis succeeded", "Save succeeded", 1442 s (about 24 min). The project is about 1.5 GB.
- **Options used** (`porting/ghidra/SlimAnalysisOptions.java`, names from `ListAnalyzers.java`):
  - Off: Create Address Tables and its Data Reference / Reference sub-options, Decompiler Parameter ID, Decompiler Switch Analysis, Apply Data Archives, Function ID, Stack, Windows x86 PE RTTI, Non-Returning Functions - Discovered, Function Start Search, x86 Constant Reference Analyzer, Call Convention ID, ASCII Strings, Embedded Media, TEB, Aggressive Instruction Finder.
  - On: Windows x86 PE Exception Handling, Disassemble Entry Points, Reference, Data Reference, Subroutine References, Non-Returning Functions - Known, Scalar Operand References, External Entry References, Call-Fixup Installer.

### Method

UHT (`Engine/Source/Programs/UnrealHeaderTool/Private/CodeGenerator.cpp`, around line 2511) emits, for each native class:

`static FCompiledInDefer Z_CompiledInDefer_UClass_X(Z_Construct_UClass_X, &X::StaticClass, TEXT("/Script/FortniteGame"), TEXT("AFortX"), false, ...)`

The class-name literal is `#TClass`, so it keeps the C++ prefix: `"AFortGameStateAthena"`, not `"FortGameStateAthena"`. The chain is:

ClassParams block (.rdata) <- `Z_Construct_UClass_X` (lea of the block) <- registration initializer (lea of the construct function and of the name literal).

`porting/ghidra/export_class_names.py` walks it from the block. It takes the functions that reference the block (the construct functions), then the UTF-16 literals in each one, then the same for their callers, up to depth 2. A literal matching `^[AUFIETS][A-Z][A-Za-z0-9_]{2,80}$` names the block if it is the only candidate. More than one candidate gives `ambiguous`. Nothing gives `unresolved` or `no_code_reference`.

Not tried: the literal-order alternative (the FPackageParams singleton order). Not tried either: walking from the Class::Member strings (see limits).

Dry run: a small UHT-shaped test binary compiled with gcc (scratch, not in the repo) gave named, depth-2 named, ambiguous, unresolved and no-code results as designed. It gave the same results with the slim option set.

### Results

- **Blocks:** 6,967 (all groups: `extract_reflection.py --all-groups`). Named 6,957. Status: named 6,957, no_code_reference 10, ambiguous 0. Duplicate names: 0.
- **Fort/Athena-group subset:** the 2,740 blocks from the default extraction are all named. The earlier export on that subset resolved only 3 of the 8 targets. The other five sit outside that group filter, so the reflection was regenerated with `--all-groups`. The previous file is kept as `porting/generated/reflection/fortnite_game_reflection.fort-groups-2740.bak.json`.
- **The 8 targets, all resolved** (block VA: name):
  - `0x1462f0c20`: AFortAthenaMapInfo
  - `0x1463ea150`: AFortGameStateAthena
  - `0x1464ad0a0`: AFortPlayerControllerAthena
  - `0x1464cd530`: AFortPlayerPawnAthena
  - `0x1464d3e50`: AFortPlayerStateAthena
  - `0x1464e7140`: AFortPoiManager
  - `0x14657b630`: AFortWorldManager
  - `0x14657f0c0`: AFortWorldSettings
- **Unnamed (10):** 0x1470b7990, 0x1470b8278, 0x1470b8aa8, 0x1470b8bd8, 0x1470d4428, 0x1470d4da8, 0x1470d4e88, 0x1470d4ef8, 0x1470d5118, 0x1470d5608. None is a target. Not investigated; the dump is a single snapshot (see limits).

### Evidence and sanity checks

1. **AActor (`0x146b59ff0`) passes.** It is named AActor. Its replicated set is exactly the 10 expected: AttachmentReplication, Instigator, Owner, RemoteRole, ReplicatedMovement, Role, bCanBeDamaged, bHidden, bReplicateMovement, bTearOff. This matches the validation in section 3.
2. **Cooked cross-check is weak.** `DefaultMapInfo-functions.json` says DefaultMapInfo_C has super FortAthenaMapInfo. The name AFortAthenaMapInfo exists under that name, so this checks existence only.
3. **Class::Member names are inconclusive.** The strings `FortPlayerStateAthena::SetDisconnected`, `FortGameStateAthena::NotifyGameMemberAdded`, `FortPlayerControllerAthena::HandleGameMemberAdded`, `FortPlayerPawnAthena::InitAbilitySystemComponentFromPlayerState` and similar are in .rdata. Their member names do not appear in any UFUNCTION table of the 6,967 blocks. Most likely these are C++ methods with no UFUNCTION, which the reflection does not record. Ghidra finds no code references to these strings, so they cannot be used to walk back to a block.
4. **Parent chain not validated.** `base_fn` of every Fort class is 0x142a02c30, and AActor's is 0x1446fdba0. Neither equals the construct function of any block in the set, so parents cannot be named from this data.

### Limits

- The method depends on UHT's output shape (prefixed literal in the compiled-in initializer). The dry run confirms the mechanics. The 6,957 unique names are consistent with that shape, but they are not checked against a second source beyond the AActor replicated set.
- The dump is a snapshot at about 32.5 s. Code decrypted later (login, lobby, gameplay) may be missing; the 10 unnamed blocks may be such cases.
- The second `.text` section (RVA 0x8D2C000) is not decoded. Functions there are not in the analysis.
- Within the slim option set, the reference-producing analyzers are on. The x86 Constant Reference Analyzer is off; the dry run showed the LEA references still exist without it.

### Tool fixes made here

- `porting/ghidra/run_export_class_names.py`: PyGhidra defaults to `nested_project_location=True`, so it looked for `<dir>/<name>/<name>.gpr`. It found nothing and imported a fresh, unanalyzed copy. The runner now passes `nested_project_location=False`. It defaults to project `fortnite-slim`, with overrides `FN_GHIDRA_PROJECT_DIR`, `FN_GHIDRA_PROJECT` and `FN_GHIDRA_PROGRAM`. The original runner had the same bug against project `fortnite`.
- `porting/ghidra/export_class_names.py`: target matching compared unprefixed names to prefixed literals and never matched. It now compares the name without its prefix letter.

### Rerun

- Reflection (all groups): `python3 -I porting/extract_reflection.py --all-groups` (25 s). The regenerated file is the default output.
- Names: `porting/.venv/bin/python -I porting/ghidra/run_export_class_names.py` (about 20 s). It writes `porting/generated/reflection/class_name_map.json` and `class_name_map.evidence.json`. The project must not be locked by an analysis run.
- Slim analysis: `GHIDRA_HEADLESS_MAXMEM=48G analyzeHeadless /opt/fortnite/ghidra-projects fortnite-slim -process FortniteClient-Win64-Shipping.runtime.exe -preScript SlimAnalysisOptions.java -max-cpu 8 -analysisTimeoutPerFile 3000` (run from `porting/ghidra` with `-scriptPath`). The import is `-import ... -loader PeLoader -noanalysis` into the same project first.
