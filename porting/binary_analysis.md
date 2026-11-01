# Binary analysis of the shipping client exe

This pass records read-only static analysis of `Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe` (162.6 MB, PE32+, x64). The observations below concern PE structure and reflection; launcher, protection and entitlement compatibility are also required by the full reconstruction goal and are tracked separately. Tags: **[observed]** = read directly from the file; **[inferred]** = reasoning from observed data; **[project note]** = taken from existing repo docs and not re-checked here.

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

The decryption or unpacking routine, the protection layer that owns the second `.text`, and the EasyAntiCheat and BattlEye components beside the exe require separate investigation for the full compatibility goal.

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

## 8. Parent chain resolution

Scope: the super class (parent) of each named FClassParams block, for `porting/gen_native_classes.py`. Read-only over the runtime dump (section 6), the reflection JSON, the Ghidra evidence (section 7) and the UE 4.25.0 headers at `/opt/fortnite/UnrealEngine` (`Build.version` is 4.25.0). Tool: `porting/resolve_class_parents.py` (usage in its docstring). Output: `porting/generated/reflection/class_parent_map.json`, `{block_va: {name, parent_name, parent_va, chain_ok, parent_source, chain, package}}`. Git-ignored with the rest of `porting/generated/`. No Ghidra run was needed: the evidence file already gives each block's construct function, and every parent is an exact address match.

### Method [observed + inferred]

1. **Which entry is the parent.** UHT 4.25 (`Programs/UnrealHeaderTool/Private/CodeGenerator.cpp`, around line 2200) builds `DependentSingletons` as the super class's `Z_Construct_UClass_*` first, when there is a super class, then the module's `Z_Construct_UPackage__Script_*` last. A class with no super class (UObject) has only the package. The data agree: 6,753 blocks have two entries, the first matching a named block's construct function. The 203 with an unmatched first entry are described in step 4. `base_fn` in `extract_reflection.py` is `dependency_fns[1]`, which is the **package**, not the parent. `gen_native_classes.py` used it as the parent before this section.
2. **Construct function per block.** Taken from `class_name_map.evidence.json` (`construct_fns`): one function per block, no duplicates across 6,957 blocks.
3. **Thunks.** A first entry that is not a construct function is followed through single jmps (`E9 rel32`, `FF 25`) when an image is given. Result: 0 thunks needed. The unmatched first entries are not thunks: their bytes are ordinary `sub rsp,N` prologues.
4. **Orphan parents.** An unmatched first entry is a class with no FClassParams block. Five such constructs exist in this build and are CoreUObject intrinsics: `0x142e73ed0` (UInterface, 199 children), `0x142e497d0` (UClass, 1 child), `0x142e49a40` (UEnum), `0x142e49c00` (UPackageMap), `0x142e49c80` (UScriptStruct). Byte scans of their bodies show no RIP-relative reference to a block. They are named only from child headers: every child with a UCLASS/UINTERFACE header must name the same base. Children: 73 header-known children, 73 agree. This is a consistency check, not an independent validation.
5. **Packages.** For each second entry, a LEA to an `FPackageParams` record whose name starts with `/Script/` was found for all 6,957 named blocks (for example `/Script/FortniteGame` 2,842, `/Script/FortniteUI` 1,681, `/Script/Engine` 967). The package is therefore always last, which confirms step 1.
6. **Chain.** Follow parents through blocks. `chain_ok` is True only when the chain reaches a root named UObject with every link a block.

### Results [observed]

| Measure | Value |
|---|---|
| Blocks in map | 6,967 (6,957 named, 10 unnamed) |
| Named blocks with a parent that is a block (or the root UObject) | 6,754 of 6,957 (6,753 block, 1 root) |
| Named blocks whose parent is an orphan CoreUObject intrinsic | 203 |
| Named blocks with `chain_ok` (through blocks to UObject) | 6,747 of 6,957 |
| Named blocks whose parent is a block but whose chain ends in an orphan | 7 (for example UFortAbilitySystemInterface -> UAbilitySystemInterface -> UInterface) |
| Unnamed blocks | 10, parent_source `no_dependencies`, see below |

**Interfaces.** One UINTERFACE gives one U-prefixed UClass block with CLASS_Interface (`0x4000`, `ObjectMacros.h` line 201) whose parent is UInterface. No I-prefixed block exists in the set. The brief's "two classes" premise is not borne out in this build: 199 interface UClasses have the UInterface orphan as parent, plus 3 interfaces that derive from other interfaces.

**Target chains** (`[observed]`, from the map):

- AFortPlayerStateAthena -> AFortPlayerStateZone -> AFortPlayerState -> APlayerState -> AInfo -> AActor -> UObject
- AFortGameStateAthena -> AFortGameStateZone -> AFortGameState_InGame -> AFortGameState -> AFortGameStateBase -> APlayspaceGameState -> AGameState -> AGameStateBase -> AInfo -> AActor -> UObject
- AFortPlayerControllerAthena -> AFortPlayerControllerZone -> AFortPlayerControllerGameplay -> AFortPlayerController -> APlayspacePlayerController -> APlayerController -> AController -> AActor -> UObject
- AFortPlayerPawnAthena -> AFortPlayerPawn -> AFortPawn -> ACharacter -> APawn -> AActor -> UObject
- AFortWorldSettings -> AWorldSettings -> AInfo -> AActor -> UObject
- AFortAthenaMapInfo -> AActor -> UObject
- AFortPoiManager -> AActor -> UObject
- AFortWorldManager -> AInfo -> AActor -> UObject

The first four match the pattern in the brief. The Fort classes have no header in the tree, so they are unchecked against headers. Their packages are all `/Script/FortniteGame`.

### Accuracy against the UE 4.25.0 headers [observed]

Every named class with a `UCLASS`/`UINTERFACE` declaration in `Engine/Source` or `Engine/Plugins` was compared with the first base class in its header, when that base is itself a UCLASS. The UObject root is compared as "no UCLASS parent" (its header base is `UObjectBaseUtility`).

- **Accuracy: 2,053 of 2,058 = 0.9976** (binary parent matches header base).
- **Engine list, 12 of 12:** AActor<-UObject, APawn<-AActor, ACharacter<-APawn, APlayerController<-AController, APlayerState<-AInfo, AGameStateBase<-AInfo, AGameModeBase<-AInfo, AWorldSettings<-AInfo, UActorComponent<-UObject, USceneComponent<-UActorComponent, AInfo<-AActor, AController<-AActor.
- **Header coverage.** 4,786 UCLASS/UINTERFACE names are declared in the tree; 2,132 of them are named in the binary. 2,654 declared names have no block. This includes editor and tool classes that are not shipped, which were not analysed.

**Five mismatches, all engine skew, not extraction errors:**

| Class | Binary parent | 4.25.0 header |
|---|---|---|
| UAnimSequencerInstance | UAnimCustomInstance | UAnimInstance |
| UGameplayTagsDeveloperSettings | UDeveloperSettings | UObject |
| UAnimCompress | UObject | UAnimBoneCompressionCodec |
| UAnimCompress_LeastDestructive | UAnimCompress | UAnimCompress_BitwiseCompressOnly |
| UOnlineBlueprintCallProxyBase | UObject | UBlueprintAsyncActionBase |

Evidence: the first one is observed. UAnimCustomInstance is named in the binary and is absent from every 4.25.0 header, so the shipped engine is not the 4.25.0 tree. The others are inferred. In each, the binary is self-consistent under the UHT rule, and the header differs in a way that fits a different engine revision. UAnimBoneCompressionCodec is also not a named block in the binary.

### Failures and limits

- Orphan parents (the 5 CoreUObject intrinsics) are named from child headers, not from the binary. Their own parents (UClass -> UStruct -> UField -> UObject) are not in the map.
- 7 blocks have a block parent whose chain ends at an orphan, so `chain_ok` is False for them.
- The Fort classes have no header, so the 8 target chains are checked for shape only (against the pattern in the brief), not against source.
- `base_fn` is still `dependency_fns[1]` (the package) in the reflection JSON. Use `class_parent_map.json` for parents.

### The 10 unnamed blocks (bounded attempt, not resolved) [observed]

Addresses `0x1470b7990`, `0x1470b8278`, `0x1470b8aa8`, `0x1470b8bd8`, `0x1470d4428`, `0x1470d4da8`, `0x1470d4e88`, `0x1470d4ef8`, `0x1470d5118`, `0x1470d5608`. None of them gets a name. Checks performed:

- **Layout.** Word 0 is a code pointer, words 1 to 6 are zero, and the flags word (`0x4542xxxx`) is not a valid EClassFlags value. Against the UE 4.25 FEnumParams and FStructParams, the name fields are zero, so they do not match either. They are probably not FClassParams.
- **Code references.** A RIP-relative LEA or MOV to any of the 10 is absent from both executable sections of the dump. The second `.text` is still encrypted in the dump in parts, so this is not conclusive.
- **Data references.** No data section contains a pointer to any of the 10.
- **Pointed-to functions.** Six of the first-pointer functions begin with `lea rax,[rip+X]` and return it, and the rest start with a register save or `mov eax,imm`. None has the prologue of a `Z_Construct_UClass_*` function.

Conclusion: no names, no parents. Confidence that these are real classes: low. The 10 stay in the map with `parent_source: no_dependencies`.

### Rerun

```
python3 -I porting/resolve_class_parents.py [--no-validate]   # map + summary (about 2 s with the dump and headers)
python3 -I porting/resolve_class_parents.py --self-test
python3 -I porting/gen_native_classes.py --parent-map porting/generated/reflection/class_parent_map.json ...
```


## 9. Structs, enums and delegates

Scope: the struct, enum and delegate-signature tables that the class tables reference, for `porting/gen_native_classes.py --promote`. Read-only over the runtime dump (section 6), Ghidra reference sites (`porting/ghidra/export_type_xrefs.py`, read-only, no analysis run), and the UE 4.25.0 headers. Scripts: `porting/extract_types.py` (scan, join, self-test), `porting/validate_types.py` (header comparison), `porting/gen_native_classes.py` (`TypeGate`, emitters, self-test). Outputs are git-ignored under `porting/generated/reflection/`: `type_block_candidates.json`, `type_block_xrefs.json`, `struct_blocks.json`, `enum_blocks.json`, `delegate_blocks.json`, `function_blocks.json`, `construct_fn_to_block.json`, `type_closure.json`, `type_validation.json`.

### Method [observed + inferred]

1. **Layouts** (`UObjectGlobals.h`, `UECodeGen_Private`, WITH_METADATA off). `FStructParams` is 72 bytes: OuterFunc, SuperFunc, StructOpsFunc, NameUTF8, SizeOf, AlignOf, PropertyArray, NumProperties (i32 @56), ObjectFlags (@60), StructFlags (@64). `FEnumParams` is 56 bytes: OuterFunc, DisplayNameFunc, NameUTF8, CppTypeUTF8, EnumeratorParams, NumEnumerators (i32 @40), ObjectFlags (@44), DynamicType (int32 @48, an enum class with no underlying type), CppForm (u8 @52). `FEnumeratorParam` is 16 bytes (name, i64 value). `FFunctionParams` is 72 bytes, as in section 3. The delegate property kinds keep their signature function at offset 40 (`FDelegatePropertyParams`, `FMulticastDelegatePropertyParams`), so `extract_reflection.py` now records it as `type_refs.signature`.
2. **Candidates** (`extract_types.py scan`). Every 8-byte-aligned position in the data sections is tested against the layout: name and string pointers, code or null for the function pointers, a small SizeOf and a power-of-two AlignOf, and a PropertyArray whose every entry is a data pointer to a record with a readable name. The three kinds are disjoint on these tests except for 21 function records that match a struct or enum at the same address; those are dropped.
3. **Construct functions** (`extract_types.py join`). The construct function of a block (`Z_Construct_UScriptStruct_*`, `Z_Construct_UEnum_*`, `Z_Construct_UDelegateFunction_*`, `Z_Construct_UFunction_*`) references its block with a RIP-relative LEA. Ghidra lists every reference site of each candidate block (31,962 blocks, about 10 s). The owning function of a site is the `.pdata` entry that contains it. The result is `construct_fn_to_block.json`.
4. **Property tables** are decoded with the class decoder (`decode_props`). Struct, enum and delegate references in class tables resolve through the construct map.
5. **Packages.** A struct or enum takes its package from the FPackageParams that its OuterFunc names (`resolve_class_parents.Probe`). A delegate declared in a class takes the class's package, because its OuterFunc is the class's construct function.
6. **Struct ops** (`StructOpsFunc`). For a struct with custom ops, the function does `new TCppStructOps<T>`: it stores a vtable pointer, the size and the alignment. The trait values are read from the vtable slots (the `ICppStructOps` declaration order in `Class.h`, destructor first), and each slot is classified by its code: a constant `true` or `false` return, or unclassified. This is the only place the custom-serializer traits are visible, because `STRUCT_NetSerializeNative` and the other computed bits are set at load time and are absent from StructFlags (`FVector_NetQuantize` has StructFlags `Native|Atomic` only).

### Results [observed]

| Kind | Recovered | With construct function | Notes |
|---|---|---|---|
| Struct | 4,512 (Fort 2,059, Engine 655, other plugins the rest) | 4,512 | 4,438 with struct ops; 2 structs have an unreadable vtable (`SoundCueCrossfadeInfo`, `SoundCueTemplateQualitySettings`) |
| Enum | 2,056 (EnumClass 1,457, Namespaced 346, Regular 253) | 2,056 | enumerators of EnumClass enums are stored as `EnumName::Value` |
| Delegate signature | 1,515 (1,400 with the MulticastDelegate flag) | 1,514 | one has no construct function (`OnChallengeSetEstablished`, which has no `__DelegateSignature` suffix) |
| UFunction parameter block | 23,879 | 23,510 | used for the RPC join, not yet used by the generator (see limits) |

- **Construct map**: 31,592 functions mapped to one block each, 0 ambiguous.
- **Class-table references** (2,622 distinct class refs are unchanged, see below) for struct, enum and delegate: 7,647 struct, 1,446 enum and 1,811 delegate references, all resolved, 0 unresolved.
- **Struct ops checks**: SizeOf equals the size written into the TCppStructOps object in 4,438 of 4,438 structs, and AlignOf equals it in 4,438 of 4,438. The size and alignment fields are independent of the trait vtable, so this is a cross-check of both.
- **Custom serializers** (from the ops vtable, 4,438 structs with ops): `HasNetSerializer` 41, `HasNetDeltaSerializer` 50, `HasNetSharedSerialization` 13. Checked against the engine: `FVector_NetQuantize`, `FRepMovement`, `FGameplayTagContainer`, `FUniqueNetIdRepl` and `FGameplayTag` report `HasNetSerializer`; `Vector`, `Quat`, `Transform` and `FVector` (no ops) and `RepAttachment` do not. Nine Fort-package structs have a custom NetSerialize (`AthenaPawnReplayData`, `CreateBuildingActorData`, `FortAIAttributeReplicationProxy`, `FortAISharedRepMovement`, `FortGameplayAbilityTargetData_SingleTargetHit`, `FortGameplayEffectContext`, `FortRootMotionSource_FallingBoost`, `ReplicatedPhysicsPawnState`, `SharedRepMovement`).

### Validation against the UE 4.25.0 headers [observed]

`porting/validate_types.py` compares each recovered type with a header declaration of the same name, using only names declared once in the engine tree, and drops members under `#if` blocks. The shipped engine is not the 4.25.0 tree (section 8), so a mismatch can be engine skew as well as a decode error.

| Kind | Compared | Exact | Accuracy |
|---|---|---|---|
| Struct (UPROPERTY member names, declaration order) | 1,409 | 1,307 | 92.8% (101 differ, 1 order only) |
| Enum (enumerator names and values) | 562 | 546 | 97.2% (16 names differ) |
| Delegate (parameter count of the DECLARE macro) | 270 | 269 | 99.6% (1 differs) |

Structs with known shapes: `Vector` (12 bytes, 4-byte alignment, X Y Z), `Quat` (16, 16, X Y Z W), `Transform` (48, 16, three 16-byte vector registers), `FVector_NetQuantize` (12, inherits `FVector`, custom NetSerialize). Enums: `ENetRole` (ROLE_None to ROLE_MAX, values 0 to 4) and `EMovementMode` match. Shipped-engine skew is visible in names such as `ECameraFocusMethod` (`None` here, `DoNotOverride` in the 4.25 header).

### Delegates and enumerators

- A delegate type name is the signature name without `__DelegateSignature`, prefixed with F (`ActorEndPlaySignature__DelegateSignature` gives `FActorEndPlaySignature`). `delegate_name` is the owning property's name (`OnEndPlay`), so it is not the type name.
- Macro choice: a signature without FUNC_MulticastDelegate is `DECLARE_DYNAMIC_DELEGATE`. With the flag, a property of kind `InlineMulticastDelegate` gives `DECLARE_DYNAMIC_MULTICAST_DELEGATE`, and `SparseMulticastDelegate` gives `..._SPARSE_...`. Arity 0 to 9 maps to `OneParam` through `NineParams`.
- A namespaced enum (`EFortBuildingState::Type`) is emitted as `namespace EFortBuildingState { enum Type ... }` in its own header.

### Limits

- **C++ names and spellings are reconstructed, not read.** The enum underlying type is `uint8` (the UHT rule, not recorded in the exe). A struct's own C++ name is `F` plus the UObject name, which UHT uses, and the delegate type name comes from the signature name. Namespaced enums use the UHT pattern and have not been compiled.
- **Bitfield bools have no offset** in the table, so a struct that has one cannot be ordered by offset exactly; the declaration order is the reverse of the table.
- **Not recovered:** metadata, `UMETA` specifiers, `Replicated` specifiers inside structs (not emitted), `RepSkip` flags on struct fields (the checksum skips them), and the bodies of custom NetSerialize functions. A replicated use of a struct with a custom NetSerialize is therefore skipped, not emitted.
- **Engine types** (not in the promoted packages) are referenced by name and included from the engine tree (`EngineTypeIndex`). The engine version is assumed to match the 4.25 code for the NetSerialize of engine structs, which is not verified.
- **Unavailable types:** three plugin modules that are not in the engine tree (`McpProfileSys`, `MeshNetwork`, `CommonInput`) are skipped with categories `*_not_available`.
- **Out, reference and return parameters** of delegates, and delegates with more than nine parameters, are not emitted (category `delegate_member_unresolved`, reason recorded).
- **Class references are unchanged.** `resolve_class_refs.py` resolves 2,326 of 2,622 class refs, and the 296 unresolved targets are not class-block functions. Each target is a singleton builder that calls a helper with stack-built parameters, with no LEA to any class block in its body and none one call deeper. They are not identified. The 148 unnamed and 232 outside-set class refs therefore still block the classes that use them, and so do the structs and delegates that use them.
- **RPCs** use the name join, not the construct-function join. The 23,510 UFunction parameter blocks with construct functions could resolve the 4,764 ambiguous name joins (`native_classes_status.md`); the generator has not been changed to use them.

## 10. Native function tables

Scope: exec addresses of the native UFUNCTIONs (`FNameNativePtrPair` tables), the entry points for decompiling gameplay code, and the vtable search. Read-only over the runtime dump (section 6) and the reflection outputs of sections 7 to 9. Script: `porting/extract_native_funcs.py` (stdlib, `--self-test`). Outputs (git-ignored, `porting/generated/reflection/`): `native_funcs.json`, `native_class_index.json`, `decompile_targets.json`. Nothing was built, and `porting/native/` and `Fortnite/` were not touched.

### Method [observed + inferred]

1. **Table layout.** `FNameNativePtrPair` is `{const char* NameUTF8; FNativeFuncPtr Pointer}` (`CoreNative.h`, 16 bytes). `FClassFunctionLinkInfo` is `{UFunction* (*CreateFuncPtr)(); const char* FuncNameUTF8}` (`Class.h`). Every native-table name pointer is pointer-equal to a name pointer in its class's function link list (35,535 of 35,535 checked in the first scan).
2. **Table to class.** `StaticRegisterNatives<C>` calls `C::StaticClass()` (`GetPrivateStaticClass`), then `FNativeFunctionRegistrar::RegisterFunctions(Class, Array, N)` with `mov r8d, N; lea rdx, Array`. The table is therefore a RIP-relative LEA target from that function. Its class is the `GetPrivateStaticClass` it calls. `GetPrivateStaticClass` is found from the compiled-in-defer initializer, which also references the construct function `Z_Construct_UClass_C` (block link from section 7). Thunks (`jmp rel32`, often outside `.pdata`) are followed.
3. **Flags and params.** A function's flags and parameters come from `function_blocks.json` by the `Z_Construct_UFunction_*` address, which is the `CreateFuncPtr` of the class's link list. This join is exact: 23,509 of 23,758 links join, with the name equal in every case.
4. **Fallback (name-only).** A table can be found without a code reference. A run of `(name, code)` pairs whose names all belong to one class, and which contains every native-flag name of that class, is assigned to it when the class is unique. It is used only for classes that the LEA route does not cover. Its results are tagged `source: name_segment`.
5. **Vtables.** `GetPrivateStaticClass` passes `InternalConstructor<C>` and `InternalVTableHelperCtorCaller<C>` as stack arguments 10 and 11 (`Class.h` `InternalConstructor`, `ObjectMacros.h` `GetPrivateStaticClassBody`). They were read from the stores through `r11`. The constructor they reach stores the vptr at offset 0 of `this` (`mov [reg], reg` with no displacement). The last such store is the candidate vtable. Slot counts run from the table start to the next class's table start.

### Coverage [observed]

| Measure | Value |
|---|---|
| Named classes (`class_name_map.json`) | 6,957 |
| Classes with at least one function link | 2,849 |
| Classes with a native table | 2,541 (2,532 by LEA, 9 by name fallback) |
| Function links (`FClassFunctionLinkInfo`) | 23,758, joined to flags 23,509 |
| Distinct function name strings in links | 20,881 |
| `FUNC_Native` function links (`0x400`) | 20,581 |
| Native function links with an address | 20,581 of 20,581: 20,573 code addresses, 8 writable-data stubs (`stub_addr`, not code) |
| `FUNC_Native` `UFunction` blocks (`function_blocks.json`) | 20,582, one of them spurious (`0x1471cff20`: flags `0x52415453`, no construct function, no owner) |
| Distinct native names with a code address in some class | 18,173 of 18,180 real names (7 have only a stub) |
| Exec address from LEA route / name fallback | about 20,443 / 139 link-level (the fallback counts include the stubs) |
| Tables rejected (names not in class; UMaterial single-letter names) | 10 |
| Conflicts (same class and name, different code; LEA kept) | 15 (13 in `UFortMcpProfileAthena`, 2 in `UFortMcpProfileCampaign`) |

The `UFunction` block count is 23,879 with 20,697 distinct name strings. The 22,730 in the brief is the number of distinct `NameUTF8` pointers from the parameter blocks (pointers, not strings).

### Validation [observed]

- **Address range.** All 20,573 code addresses (and the 20,574 exec addresses counted at class level) are inside an executable section (`.text`, both sections). 8 entries are stubs in writable `.data`, reported only as `stub_addr`.
- **Decode.** 200 random exec addresses, objdump of each function body up to its first `ret` or `jmp`: 188 clean (94%), 13 `(bad)` lines in 2,727 instructions (0.5%). Six sampled functions have a `(bad)` line, and they were not investigated individually. 168 of the 200 are `.pdata` function starts. The remaining 32 are small leaf functions with no unwind entry; their extent is estimated from int3 padding. The earlier 48-byte window overcounted, because it runs into the next function's data.
- **Engine headers** (UE 4.25 `Engine/Source/Runtime`, `UFUNCTION` declarations, approximate regex). The shipped engine is not the 4.25 tree (section 8), so differences are expected.

| Class | Header UFUNCTIONs | In recovered class | With exec | Recovered functions (exec) |
|---|---|---|---|---|
| AActor | 135 | 124 | 102 | 127 (105) |
| APawn | 29 | 29 | 27 | 29 (27) |
| ACharacter | 46 | 46 | 40 | 46 (40) |
| APlayerController | 135 | 135 | 135 | 135 (135) |
| AGameModeBase | 28 | 28 | 23 | 28 (23) |
| UActorComponent | 25 | 25 | 22 | 25 (22) |
| USceneComponent | 59 | 59 | 59 | 61 (61) |
| UKismetMathLibrary | 435 | 420 | 420 | 650 (650) |
| UGameplayStatics | 118 | 118 | 118 | 118 (118) |
| UKismetSystemLibrary | 230 | 226 | 226 | 243 (241) |

`AActor::K2_GetActorLocation` resolves to `0x1446b0aa0` (flags `0x54820401`). UKismetMathLibrary has 650 native functions, not thousands. The header declares 435 UFUNCTIONs in that file.

### Vtables [observed; not reliable]

- A candidate vptr store was found for 6,751 of 6,957 named classes. Only 844 distinct addresses result, and 4,249 candidates (on the earlier rule) were shared with classes outside the candidate's ancestry. Example: `AFortWorldSettings` gets the table of unrelated animation-instance classes.
- Published as `vtable_addr` only when the class hierarchy supports it: 365 unique tables and 43 inherited (a class that adds no virtuals reuses its parent's table). That is 408 classes. Everything else keeps `vtable_candidate` with `vtable_status: unverified`, and is not used.
- Verified-looking Athena tables: `AFortGameStateAthena` `0x145a359b8` (unique, slots 787 is an upper bound) and `AFortPlayerControllerAthena` `0x1459ff650` (unique, 34). Neither is checked against anything beyond the hierarchy rule.
- AActor's constructor store was read by hand: `lea rax,[0x1469fec60]; mov [rdi],rax` (the own vptr at offset 0). The published table is `inherited`.
- **Limit.** The `ClassConstructor` and `VTableHelperCtorCaller` helpers are reached by thunks, and the constructor chain can share a tail-jumped base constructor. Without symbols or RTTI (only 611 `.?AV` names, none `AFort*`) no slot-to-function mapping is possible.
- **Ghidra work needed later:** decompile the `InternalConstructor<C>` thunk, then the constructor it tail-jumps to, read the vptr store with the decompiler, and check the store against the slot list. The slot-to-name map needs the base class declaration order from the headers, which the shipped engine does not match exactly.

### Decompile targets [observed]

`porting/generated/reflection/decompile_targets.json` lists 908 functions for the eight Athena framework classes, ranked by tier, each with exec address, flags and net roles: tier 1 (`FUNC_Net`, RPC or replicated) 182; tier 2 (`FUNC_Native` and `BlueprintEvent`) 7; tier 3 (`BlueprintCallable`, exec) 436; tier 4 (other native, exec) 228; tier 5 (no exec address) 55. `AFortWorldSettings` has no UFUNCTION entries in the reflection data (`function_count` 0), so only its class record is written. Parent chains come from `class_parent_map.json`. Tier 5 entries have `exec_addr: null` and need a vtable or `ProcessEvent` path.

### Decompile run [observed; script: `porting/ghidra/decompile_targets.py`]

PyGhidra opens `fortnite-slim` read-only (no save). Each target is decompiled with `DecompInterface` (30 s limit), writing `porting/generated/decomp/<Class>/<Function>.c` and `decomp/index.json`. Tiers 1 to 4 gave 853 ok, 0 timeout, 0 failed. The 55 tier-5 entries have no exec address and are recorded as skipped. The run took about 32 s. 266 functions had no `.pdata` function and were created in memory (rolled back). Opening the project rewrote five bookkeeping files (`~index` and `project.prp`); the program database was unchanged. Quality review, noise list and what is still missing are in `porting/decompilation_notes.md`.

### Vtables from the constructors [observed; replaces the candidate for these two classes]

`GetPrivateStaticClassBody` takes `InternalConstructor<C>` as argument 10 (the `GetPrivateStaticClass` addresses in `native_class_index.json`, `gpsc_targets`). That thunk is `if (*this) Ctor(*this, this)`. The constructor's only primary store of the vptr is `*param_1 = &DAT_...`, and it comes after the base-class constructor call. The decompiler reads it directly. The `InternalVTableHelperCtorCaller` argument is a shared stub that returns 0.

| Class | GetPrivateStaticClass | InternalConstructor | Constructor | Primary vtable | `vtable_candidate` (unverified) |
|---|---|---|---|---|---|
| `AFortPlayerStateAthena` | `0x142a8ae70` | `0x142a8af30` | `0x141a2e060` | `0x145e35fa8` | `0x1459e9d30` (does not match) |
| `AFortPlayerPawnAthena` | `0x142a82010` | `0x142a81d70` | `0x141a08ef0` | `0x145e2cfd0` | `0x14605c260` (does not match) |

- The first 30 slots (read from the PE image) contain 20 distinct targets. None is a UFUNCTION exec address, so no `Z_Construct`/UFunction names apply. 13 of the 30 slots point to three shared stubs (`0x1406801c0`, `0x1406801d0`, `0x1406801e0`).
- RPC slots come from the thunk call sites. A validated RPC calls `slot` (the `_Validate` returns a bool) and then `slot + 1` (the implementation). A single-call RPC calls the implementation only. Examples: `AFortPlayerStateAthena` 267 to 277 (`ClientNotifyMatchEntered` 269, `ClientReportKill` 272, `ClientAddKillFeedErrorMessage` 276); `AFortPlayerPawnAthena` 636 to 644 (`ServerCreativeToggleGhost` 636/637, `ServerSuicide` 640/641). These are inferred, not verified against the names in the table.
- The vtable rule in `extract_native_funcs.py` picks the wrong store for these two classes. The index is not changed here. Use the table above until that rule is fixed.

### Limits

- **No symbols.** Names come from the reflection tables and the table association. The association is verified by the class's function list, not by the code bodies.
- **Name-only fallback.** About 139 function links use it, mostly in `UFortMcpProfile*`. In `UFortMcpProfileAthena` the LEA table (13 functions) and a 24-entry `.rdata` table give different code for the same names (15 conflicts over the two classes), and the LEA table was kept. Either could be wrong. The fallback entries are the lower-confidence set and are tagged `source: name_segment`.
- **Unlinked tables.** The `UMcpProfile` family has no LEA-referenced table. Its registration code is outside the decoded `.text` (`porting/binary_analysis.md` section 6 caveat 1 and section 7 limit 3). Coverage there is only the `.rdata` table.
- **Dump snapshot.** Code decrypted after 32.5 s (login, lobby, gameplay) may be absent. Each exec address should be checked with the decode step before use.
- **Params** are the decoded `FFunctionParams` property tables from `function_blocks.json` (section 9 limits apply, including the `RepSkip` and metadata gaps).
- **Rerun:** `python3 -I porting/extract_native_funcs.py` (about 35 s). Self-test: `python3 -I porting/extract_native_funcs.py --self-test`. Options: `--no-decode-check`, `--no-vtables`, `--no-name-fallback`, `--no-header-check`.
