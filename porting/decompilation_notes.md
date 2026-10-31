# Decompilation notes: quality of the Athena target set

Scope: the 908 ranked targets in `porting/generated/reflection/decompile_targets.json` (section 10 of
`binary_analysis.md`), decompiled by `porting/ghidra/decompile_targets.py` from the slim project
`fortnite-slim`. Output is git-ignored under `porting/generated/decomp/` (853 `.c` files and `index.json`).
The raw code is derived from the user's supplied client and is not reproduced here beyond short excerpts.

## Run results [observed]

| Tier | Meaning | Targets | ok | timeout | failed | skipped |
|---|---|---|---|---|---|---|
| 1 | FUNC_Net (RPC / replicated) | 182 | 182 | 0 | 0 | 0 |
| 2 | FUNC_Native + BlueprintEvent | 7 | 7 | 0 | 0 | 0 |
| 3 | BlueprintCallable with exec | 436 | 436 | 0 | 0 | 0 |
| 4 | other native with exec | 228 | 228 | 0 | 0 | 0 |
| 5 | no exec address | 55 | 0 | 0 | 0 | 55 |
| **Total** | | **908** | **853** | **0** | **0** | **55** |

- The wall time for all 908 was about 32 s (sequential, JVM start included). No function hit the 30 s timeout.
- 266 functions had no Ghidra function at their exec address (not covered by `.pdata`). They were created in
  memory with `--create-missing`. The creation and every rename ran inside one transaction that was rolled back.
- Opening the project rewrote five bookkeeping files (`idata/~index.bak`, `idata/~index.dat`,
  `versioned/~index.bak`, `versioned/~index.dat`, `project.prp`). The program database was not written.
- Nothing was saved. The probe run and every later run reported the same five files, and no others.
- 39 targets share 10 code addresses (the linker most likely folded identical bodies, e.g. empty stubs). The index is keyed by class and function for this reason.

## Sample review: 15 functions [observed; judgment is the author's]

| # | Function | Kind | Lines | Verdict |
|---|---|---|---|---|
| 1 | `AFortPawn.GetLocalSpin` | getter (T3) | 10 | **Yes.** Needs the `+0x638` field name and the return type. |
| 2 | `AFortPawn.ServerTeleportNearLocation` | Server RPC with `_Validate` | 32 | **Yes.** Validate and implementation slots are clear (see excerpt B). |
| 3 | `AFortPlayerControllerAthena.ServerSetTeam` | Server RPC with `_Validate` | 26 | **Yes.** Same shape as #2. |
| 4 | `AFortPlayerControllerAthena.ServerSetOnCreativePortalLoadedDelegate` | Server RPC, one vtable call | 20 | **Yes**, once slot `0x25e8` is named. |
| 5 | `AFortPlayerStateAthena.ClientNotifyMatchEntered` | Client RPC, two FFrame reads | 36 | Partial: the reads and the slot call (`0x868`) need names. |
| 6 | `AFortPlayerControllerAthena.ClientAutoEquipFirstItem` | Client RPC, struct + bool | 39 | Partial: struct parameter type unknown. |
| 7 | `AFortPlayerStateAthena.ClientAddKillFeedErrorMessage` | Client RPC, TArray of ref-counted items | 59 | Partial: the array-release loop is hard to read by hand. |
| 8 | `AFortPlayerControllerAthena.ClientBroadcastOnUpdateCreativePlotName` | Client RPC, FString + bool | 54 | Partial: FString and ref-count release are unnamed. |
| 9 | `AFortPlayerPawn.ClientNotifyAbilityFailed` | Client RPC, GAS-related | 38 | Partial: the target slot `0xf98` is unnamed. |
| 10 | `AFortPlayerPawn.ClientResetAbilitySystemComponent` | thunk, tail jump through vtable | 12 | Partial: "Treating indirect jump as call". The target is unknown. |
| 11 | `AFortPlayerControllerAthena.ServerAddSimulatedXp` | Server RPC with FString | 68 | Partial (skimmed, top half read). |
| 12 | `AFortPawn.NetMulticast_InvokeGameplayCueAdded` | multicast, GAS cue parameters | 84 | **No.** Stack struct built from a static pointer; needs the cue-parameter layout. |
| 13 | `AFortPlayerPawn.HandleInStasis` | the only `switch` | 105 | **No.** Jump table not recovered; an `IMAGE_DOS_HEADER_140000000` expression is junk. |
| 14 | `AFortPlayerControllerAthena.PublishCreativePlot` | long Server RPC | 244 | **No.** Stack-struct copy noise. |
| 15 | `AFortPlayerStateAthena.OnRep_DeathInfo` | OnRep, large (tier 4) | 1717 | **No.** Needs struct layouts and helper names throughout. |

Verdict: **4 of 15 are portable by hand now** (getter, two validated RPCs, and the single-call RPC once its slot is named). Seven need a
few names (vtable slots, FFrame helpers, a struct type). Four are not usable as a port guide. Across
all 853 files: 273 of 436 tier-3 functions and 70 of 182 tier-1 functions are 20 lines or fewer, so
the short wrapper-type functions are where this pays off.

## Recurring noise [observed counts across the 853 files]

- **FFrame argument fetch.** `FUN_142ed9cf0` and `FUN_142ed9d20`, 667 call sites each. Each RPC and
  BlueprintCallable wrapper uses them to read every parameter from the script frame, with the
  `*(param_2 + 0x20) == 0` test repeated per parameter. Naming these two would clear a large share of every RPC wrapper.
- **Vtable calls.** `(**(code **)(*param_1 + 0x...))`: 997 call sites in 391 files, 454 distinct offsets.
  The most common are 0x150 (57), 0x38 (25) and 0x10 (24), which are base-class virtuals; the rest are spread out, including the high RPC slots.
- **`FUN_142bee710`**, 410 call sites in 123 files, always after a null test on a pointer. `FMemory::Free` (identified in the second pass, see Helper identities).
- **`FUN_142d6e0f0(L"<Function>_Validate")`**, 96 sites. `RPC_ValidateFailed` (identified in the second pass; the checkf/ensure guess was wrong: it only stores the name in a global).
- **Ref-count release loops** (`LOCK()`/`UNLOCK()`), 161 sites in 28 files: TSharedRef/TRefCountPtr releases, inlined.
- **Untyped locals and parameters.** `param_N` 11,724 uses, `undefined*` 6,607, `longlong *` casts 4,309,
  stack locals (`uStack_`, `alStack_`, `plStack_`) 10,194. Most RPC prototypes are `(longlong *, longlong)`.
- **Switch and jump tables.** One `switch` recovered in the whole set. The rest appear as `goto` / `LAB_` labels (235 in 50 files).
  Warnings "Could not recover jumptable" appear where the decompiler gives up.
- **Thunk tail calls** shown as calls, and **identical code under different names** (39 targets share 10 bodies).
- **Strings.** Literals resolve. Defined string data is tried first (`StringDataInstance`). Undefined ASCII and UTF-16
  literals are read from memory, because the slim project has the ASCII strings analyzer off. Only 96 distinct literals
  appear, all `*_Validate` names. No log or UFunction strings are referenced directly by these 853 bodies.

## Symbol pass [observed]

`--rename` names each native exec function `Class_Function` from `native_funcs.json` (16,389 renamed,
4,185 not applied because no function object exists there). Before/after on two functions:

- `AFortPlayerPawn.ClientNotifyAbilityFailed`: the function name changes from `FUN_142a7b220` to `AFortPlayerPawn_ClientNotifyAbilityFailed`. The body is identical.
- `AFortPlayerControllerAthena.ClientAutoEquipFirstItem`: the name changes from `FUN_142a56690` to `AFortPlayerControllerAthena_ClientAutoEquipFirstItem`. The body is identical.

Zero of the 353 distinct direct callees of the 853 functions are UFUNCTION exec addresses, so renaming
resolves no callees. The gain is the function's own name, nothing more.

## Excerpts (raw output, then the C++ I would write)

**A. Getter.** Raw (`AFortPawn::GetLocalSpin`):
```c
*(ulonglong *)(param_2 + 0x20) = (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
*param_3 = *(undefined4 *)(param_1 + 0x638);
```
C++ (the first line is `P_FINISH`, which I read as `Stack.Code += !!Stack.Code`; unverified against the 4.25 macros):
```cpp
void AFortPawn::execGetLocalSpin(UObject* Context, FFrame& Stack, void* const Result) {
    P_FINISH;
    *(float*)Result = LocalSpin;   // LocalSpin at +0x638 (name and type need the layout)
}
```

**B. Validated Server RPC.** Raw (`AFortPawn::ServerTeleportNearLocation`):
```c
cVar1 = (**(code **)(*param_1 + 0x8d0))(param_1,&uStack_18);
if (cVar1 == '\0') {
  FUN_142d6e0f0(L"ServerTeleportNearLocation_Validate");
  return;
}
(**(code **)(*param_1 + 0x8d8))(param_1,&uStack_18);
```
C++:
```cpp
// params read from Stack by the FFrame helpers, then:
if (!ServerTeleportNearLocation_Validate(Params)) {   // vtable slot 0x8d0 / 8 = 282
    RPC_ValidateFailed(TEXT("ServerTeleportNearLocation_Validate"));   // FUN_142d6e0f0 (second pass: identified)
    return;
}
ServerTeleportNearLocation_Implementation(Params);    // vtable slot 0x8d8 / 8 = 283
```

**C. Client RPC with a call through the vtable.** Renamed output (`AFortPlayerPawn_ClientNotifyAbilityFailed`):
```c
(**(code **)(*param_1 + 0xf98))(param_1,uStackX_8,alStack_28);
```
C++ (types unknown: `uStackX_8` is an 8-byte value, `alStack_28` a 32-byte struct):
```cpp
ClientNotifyAbilityFailed_Implementation(AbilityHandle, Reason);   // vtable slot 0xf98 / 8 = 499
```

## Vtable slot numbers the thunks give [observed]

A tier-1 thunk calls `(*this + off)`. For a validated RPC the call sequence is validate, then
implementation at the next slot. For a Client or Multicast RPC there is one call, to the implementation.
Examples (slots in the class's own table, from the constructor vtables in `binary_analysis.md` section 10):

- `AFortPlayerStateAthena` (table `0x145e35fa8`): ClientNotifyMatchEntered 269, ClientReportDBNO 270,
  ClientReportTeamKill 271, ClientReportKill 272, Client_OnNewLevel 273, ClientAddKillFeedErrorMessage 276.
  Server_TogglePreserveSquad 267/268 (validate/impl). Server_SetCanEditCreativeIsland 274/275.
- `AFortPlayerPawnAthena` (table `0x145e2cfd0`): NetMulticast_SuccessfulBuildingEdit 639, ServerSuicide 640/641,
  ServerCreativeToggleGhost 636/637, FastSharedReplication 642, ServerSetInteractingItem 643/644.

These are inferred from call sites. Check each against the implementation target before use.

## What is missing to make this portable

1. **Vtable slot names.** The first 30 slots of both Athena tables are base-class virtuals with no UFUNCTION match
   (20 distinct targets; 13 of the 30 slots point to three shared stubs). The RPC slots above are the only names available. The
   base slots need UObject, AActor and APlayerState declaration order from matching engine headers.
2. **Identities of the helpers.** Done in the second pass (Helper identities below), except `FUN_142c0c240` (unidentified). A UE 4.25 build would
   let these be matched by their string literals and call shapes.
3. **Struct layouts.** `FVector`, `FItem`, `FGameplayCueParameters`, `FDeathInfo`, and the creative structs. These are in
   the recovered struct tables (section 9); the decompiler does not use them yet.
4. **Prototypes.** The decompiler has no UFunction parameter types. Setting prototypes from `function_blocks.json`
   before decompiling would replace `longlong *`/`undefined*` with real types.
5. **Switch recovery.** Decompiler switch analysis is off in the slim project. Re-enabling it only for the few
   functions that need it, or decoding jump tables by hand, is needed for `HandleInStasis`-type code.

## Method notes

- `porting/ghidra/decompile_targets.py` opens the project without saving. Created functions and renames are
  rolled back with `endTransaction(tx, False)`. The run reports any project file that changed on disk.
- Output: `porting/generated/decomp/<Class>/<Function>.c` with a header (class, function, tier, address,
  flags, net roles) and `porting/generated/decomp/index.json` (status, line count, direct and indirect calls,
  string literals).


## Second pass: improved inputs, measured [observed unless marked]

Status: items 1 to 6 of the task are done for the 853 targets. The script is `porting/ghidra/improve_decompile.py`
(tracked). Its tables are in `porting/ghidra/name_tables/` (tracked): `helpers.json` (hand-written, with evidence),
`struct_layouts.json` (generated by `--build-tables`), and `vtable_slots_<Class>.json` (written by each run). Output:
`porting/generated/decomp_improved/` (923 files plus `index.json`; git-ignored), `porting/generated/decomp/portability.json`
(all 923 records, plus `top60`) and `before_after.json` (git-ignored).

Method: the project is opened as in `decompile_targets.py` (no nested location, no save). All edits run in one
transaction that is rolled back with `endTransaction(tx, False)`. Each run changed only the same five bookkeeping
files as before (`idata/~index`, `versioned/~index`, `project.prp`), and the program database did not change. 266
exec addresses have no `.pdata` function and were created in memory (rolled back). No `fortnite-decomp` copy was
needed. Run time is about 70 s with a 48G heap.

### Helper identities

Each row has the disassembly or source evidence used. Call-site counts: "853" are call lines in the 853 raw files
(`porting/generated/decomp/`), "whole exe" are Ghidra call references in the program (includes non-Athena code).

| Name (applied) | Address | Identity | Evidence | Call sites: 853 / whole exe |
|---|---|---|---|---|
| `FFrame_Step(Stack, Context, Result)` | `0x142ed9cf0` | `FFrame::Step(UObject*, RESULT_DECL)` | Reads `Code` at `+0x20`, `B = *Code++`, then `GNatives[B](Context, *this, Result)` through the table at `0x148262b60`. Source: `CoreUObject/Private/UObject/ScriptCore.cpp:335` | 667 in 389 files / 22,444 |
| `FFrame_StepExplicitProperty(Stack, Result, Property)` | `0x142ed9d20` | `FFrame::StepExplicitProperty` | Tests `Property->PropertyFlags` (`+0x40`) for `CPF_OutParm` (0x100); walks `OutParms` (`+0x78`) or uses `Locals` (`+0x28`) plus `Offset_Internal` (`+0x4c`); tail-calls a virtual on the property (`+0xc8`); the source's else branch calls `CopyCompleteValueToScriptVM`. `ScriptCore.cpp:341` | (paired with the row above) |
| `FMemory_Free(Ptr)` | `0x142bee710` | `FMemory::Free(void*)` | `if (Ptr) GMalloc->Free(Ptr)`; `GMalloc` is the global at `0x14823d250`, set in `FUN_142bee820`. | 410 in 123 files / 162,938 (plus 5,779 jumps) |
| `FMallocThreadSafeProxy_Free(this, Ptr)` | `0x142bee6c0` | GMalloc's slot `+0x30` | Body: if `Ptr`, lock `this+0x10`, `UsedMalloc->Free(Ptr)` (`UsedMalloc` at `+0x8`), unlock. `GMalloc` is a `FMallocThreadSafeProxy` (vtable `0x1465c5df0`, object built in `FUN_142bee820`). | 0 direct (virtual) |
| `FMallocThreadSafeProxy_Malloc(this, Count, Alignment)` | `0x142bfb090` | GMalloc's slot `+0x10` | Body: lock, `UsedMalloc->Malloc(Count, Alignment)`. No direct call to this address exists in the 853 raw files; callers reach it through the vtable. | 0 direct |
| `FMallocThreadSafeProxy_Realloc(this, Original, Count, Alignment)` | `0x142bfd190` | GMalloc's slot `+0x20` | Body: lock, `UsedMalloc->Realloc(Original, Count, Alignment)`. | 0 direct |
| `RPC_ValidateFailed(Reason)` | `0x142d6e0f0` | `RPC_ValidateFailed(const TCHAR*)` | Body is `MOV [0x148260c58],RCX; RET`, i.e. `GLastRPCFailedReason = Reason`. Source: `CoreUObject/Private/UObject/CoreNet.cpp:521`. UHT emits `if (!P_THIS->X_Validate(...)) { RPC_ValidateFailed(TEXT("X_Validate")); return; }` (`UnrealHeaderTool/Private/CodeGenerator.cpp:5061`). The first-pass guess (checkf/ensure) was wrong. | 96 in 96 files / 399 |

Data labels applied: `GMalloc` (`0x14823d250`), `GNatives` (`0x148262b60`), `GLastRPCFailedReason` (`0x148260c58`).

Not identified: `FUN_142c0c240` (called with a stack local in several RPC bodies, e.g. `ServerAddSimulatedXp` and
`ClientBroadcastOnUpdateCreativePlotName`; no evidence yet).
Inline idioms, not functions: FString free (`if (Data) FMemory_Free(Data)`), the ref-count release (`LOCK()` / decrement /
`UNLOCK()`, then a vtable destructor call; 322 `LOCK()` lines in the 853 raw files), and the `StepCompiledIn` branch
(`if (Code == 0) StepExplicitProperty else Step`) that the compiler inlined into every parameter read.

### What the edits do

1. **Exec prototypes (853).** `void Class_Fn(UObject* Context, FFrame* Stack, void* Result)`, with `Context` typed as
   the class struct. The 667 fetch blocks in the 853 bodies match 667 UFunction input parameters in order
   (`fetch_count_mismatch` is 0). Each block is rewritten to `Stack->StepCompiledIn(&Name);  /* P<i> Name: type */`
   and the local is renamed to the parameter name (665 of 667; the other two were skipped by the name-clash or the
   identifier test in `normalize_code`).
2. **FFrame struct.** 0x98 bytes, UE 4.25 `Stack.h` (FOutputDevice base, `Node` `+0x10`, `Code` `+0x20`, `OutParms`
   `+0x78`). The helper bodies use the same offsets.
3. **Class structs** from reflection property offsets, including Fortnite ancestors (`struct_layouts.json`, `size`
   from the bitfield `size_of_outer`):

| Class | Size | Fields | Chain length (incl. self) |
|---|---|---|---|
| AFortPlayerStateAthena | 0x1400 | 134 | 3 |
| AFortPlayerPawnAthena | 0x30A0 | 492 | 3 |
| AFortPlayerControllerAthena | 0x3720 | 417 | 5 |
| AFortGameStateAthena | 0x2948 | 332 | 6 |
| AFortPlayerPawn | 0x28B0 | 411 | 2 |
| AFortPawn | 0x1040 | 164 | 1 |
| AFortWorldSettings | 0x830 | 40 | 1 |

   Engine classes above the first Fortnite ancestor are not recovered (gap fields `_gap_0x...`). Bitfield bools are not
   placed (listed per class in `bitfields_not_placed`). The decompiler output reads `Context->LocalSpin`, not
   `*(float*)(param_1 + 0x638)`, for example in `AFortPawn.GetLocalSpin`.
4. **Vtable slot names** from the tier-1 RPC thunks: a validated RPC calls slot (validate) and slot+1
   (implementation); an unvalidated one calls the implementation. Conflicts are 0 for all three tables.

| Table | Address | Named slots | RPC prototypes applied | Evidence for the table |
|---|---|---|---|---|
| AFortPlayerStateAthena | `0x145e35fa8` | 10 (slots 267 to 276) | 7 | Constructor (binary_analysis section 10) |
| AFortPlayerPawnAthena | `0x145e2cfd0` | 78 | 28 | Constructor (binary_analysis section 10) |
| AFortPlayerControllerAthena | `0x145e1aac0` | 189 | 35 | New in this pass: same constructor rule. The thunk `0x142a527d0` (argument 10 of `GetPrivateStaticClass` `0x142a53450`) tail-jumps to the constructor `0x1419b7d70`, whose only vptr store is `MOV [RDI],RAX` after `LEA RAX,[0x145e1aac0]`. Checked by slot consistency only (no conflicts, and the names match the RPC pairs seen in the raw code, e.g. `ServerAddSimulatedXp` at 0x2308/0x2310). |

   The 10 PlayerState names match the first-pass RPC slot list (267/268 TogglePreserveSquad, 269
   ClientNotifyMatchEntered, 276 ClientAddKillFeedErrorMessage, 274/275 SetCanEditCreativeIsland, and so on). Other
   virtual slots stay `vfn_<offset>`: no base-class virtual order was applied.
5. **RPC prototypes.** `_Validate` (bool) and `_Implementation` at the table entries, with `this` typed as the class
   struct and the C parameters from `native_funcs.json`. Output: 35 (controller), 28 (pawn), 7 (state). Bodies are
   decompiled as separate records (`kind` implementation or validate).
6. **Text stage after Ghidra** (`normalize_code` in the script, counted per record in the index). Ghidra kept the
   `StepCompiledIn` branches, the `P_FINISH` expression and the vtable calls as raw expressions. These rewrites give
   the same meaning in C++-like form: `P_FINISH;` (850), field casts `*(T*)Context->F` to `Context->F` (3,392 field
   derefs), vtable calls `(**(code**)(Context->vftable + 0xNNN))(Context, ...)` to `Context->SlotName(...)` (504),
   and `*(T*)Result` to the return type. Ghidra did not fold the vtable call through the struct on its own, so this
   stage does it with the same slot names.

### Before and after, 853 exec targets [observed; the verdict is a proxy]

Verdict rule (`verdict_of`): portable needs score >= 75, <= 80 lines, no unnamed `FUN_` call and no unnamed vtable
call. Needs names is score >= 45. The score is in `score_text` (penalties for lines, unnamed calls, vtable calls,
gotos, loops, switch, and noise tokens such as `param_N`, `undefinedN`, `uStack_`).

| Verdict | Before (raw, 853) | After (improved, 853) |
|---|---|---|
| portable | 0 | 136 |
| needs names | 296 | 586 |
| not usable | 557 | 131 |

The 853 counts are the automatic proxy. The 15-function sample is judged by hand below.

### Sample of 15: re-judged with the same criteria

Notes first pass: Yes 4, Partial 7, No 4. Second pass: Yes 6, Partial 5, No 4. Two functions move from partial to
portable (`ClientNotifyMatchEntered`, `ClientResetAbilitySystemComponent`), and `ServerSetOnCreativePortalLoadedDelegate`
becomes unconditional (its slot is now named).

| # | Function | First pass | Now | What decides it now |
|---|---|---|---|---|
| 1 | `AFortPawn.GetLocalSpin` | Yes | Yes | `Context->LocalSpin` from reflection, `float` result cast |
| 2 | `AFortPawn.ServerTeleportNearLocation` | Yes | Yes | Validate and implementation named (282/283). The FVector is split into three locals (decompiler stack analysis is off) |
| 3 | `AFortPlayerControllerAthena.ServerSetTeam` | Yes | Yes | Validate and implementation named from the controller table (slots 1167/1168, 0x2478/0x2480) |
| 4 | `AFortPlayerControllerAthena.ServerSetOnCreativePortalLoadedDelegate` | Yes, once 0x25e8 named | Yes | Slot 0x25e8 (1213) named `ServerSetOnCreativePortalLoadedDelegate_Implementation` |
| 5 | `AFortPlayerStateAthena.ClientNotifyMatchEntered` | Partial | **Yes** | Slot 269 named (`_Implementation`); FString parameters named |
| 6 | `AFortPlayerControllerAthena.ClientAutoEquipFirstItem` | Partial | Partial | Implementation named; a 16-byte struct is packed from a bool and an FGuid by scalar stores |
| 7 | `AFortPlayerStateAthena.ClientAddKillFeedErrorMessage` | Partial | Partial | The TArray element release is an inline loop (LOCK/UNLOCK, vtable calls); element type not recovered |
| 8 | `AFortPlayerControllerAthena.ClientBroadcastOnUpdateCreativePlotName` | Partial | Partial | `FUN_142c0c240` unnamed (FText init); ref-count release |
| 9 | `AFortPlayerPawn.ClientNotifyAbilityFailed` | Partial | Partial | Implementation named; the 32-byte GameplayTagContainer is passed as an 8-byte local (locals not typed) |
| 10 | `AFortPlayerPawn.ClientResetAbilitySystemComponent` | Partial | **Yes** | Tail call now named (`ClientResetAbilitySystemComponent_Implementation`); the jump-table warning is the tail call |
| 11 | `AFortPlayerControllerAthena.ServerAddSimulatedXp` | Partial | Partial | Validate and implementation named; `FUN_142c0c240` unnamed; ref-count release |
| 12 | `AFortPawn.NetMulticast_InvokeGameplayCueAdded` | No | No | Cue-parameter structs built on the stack from a static pointer (63 lines of stack copies) |
| 13 | `AFortPlayerPawn.HandleInStasis` | No | No | Jump table not recovered (switch analysis is off); unnamed calls |
| 14 | `AFortPlayerControllerAthena.PublishCreativePlot` | No | No | 229 lines; 2 unnamed calls; stack-struct copies |
| 15 | `AFortPlayerStateAthena.OnRep_DeathInfo` | No | No | 1730 lines; 84 unnamed callees; 43 labels or warnings |

### Top 60 candidates (smallest, cleanest, highest value)

Selection: 25 getters, setters and predicates (Get/Set/Is/Has/On/Can/Should, exec), 20 RPC functions with the
implementation or validate pair, and 15 other small bodies. Only portable or needs-names records with at most 40 lines
are eligible. Full ranking: `porting/generated/decomp/portability.json`. Score and verdict are the proxy above.

| Rank | Function | Kind | Lines | Score | Verdict | Category |
|---|---|---|---|---|---|---|
| 1 | AFortPawn.IsSpinning | exec | 13 | 92.9 | portable | getter/setter/predicate |
| 2 | AFortPawn.IsStaggered | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 3 | AFortPlayerPawn.HasRespawnedInAir | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 4 | AFortPlayerPawn.OnRep_ReplicatedIsInSlipperyMovement | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 5 | AFortGameStateAthena.HasValidFocalPointActor | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 6 | AFortPlayerControllerAthena.IsClientNotifiedOfLoss | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 7 | AFortPlayerControllerAthena.IsClientNotifiedOfTeamWin | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 8 | AFortPlayerControllerAthena.IsClientNotifiedOfWin | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 9 | AFortPlayerControllerAthena.IsLoadingScreenActive | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 10 | AFortPlayerPawn.IsInCharacterCustomization | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 11 | AFortPlayerPawn.OnRep_ParachuteLockedOpen | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 12 | AFortPawn.GetControlRecoveryBehavior | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 13 | AFortPawn.GetLocalSpin | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 14 | AFortPawn.IsAllowChainStun | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 15 | AFortPawn.IsDBNO | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 16 | AFortPawn.OnlySpinWhenFalling | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 17 | AFortPlayerPawn.GetIsSlopeSliding | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 18 | AFortPlayerPawn.GetStasisMode | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 19 | AFortPlayerPawn.IsInFrontEndHologram | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 20 | AFortPlayerPawn.IsInSkyTube | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 21 | AFortPlayerPawn.IsInSlipperyMovement | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 22 | AFortPlayerPawn.IsInVortex | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 23 | AFortPlayerPawn.IsParachuteForcedOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 24 | AFortPlayerPawn.IsParachuteLockedOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 25 | AFortPlayerPawn.IsParachuteOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 26 | AFortPlayerControllerAthena.ServerSetMinigameClassSlot | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 27 | AFortPlayerPawn.ServerInterrogateDBNOPlayer | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 28 | AFortPlayerStateAthena.ClientReportDBNO | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 29 | AFortPlayerStateAthena.ClientReportKill | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 30 | AFortPlayerStateAthena.ClientReportTeamKill | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 31 | AFortPlayerStateAthena.Client_OnNewLevel | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 32 | AFortPlayerControllerAthena.ServerDBNOReviveInterrupted | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 33 | AFortPlayerControllerAthena.ServerDBNOReviveStarted | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 34 | AFortPlayerControllerAthena.ServerEndLoadingVolume | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 35 | AFortPlayerControllerAthena.ServerEndUnloadingVolume | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 36 | AFortPlayerControllerAthena.ServerSetTeam | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 37 | AFortPlayerControllerAthena.ServerSimulateLevelUp | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 38 | AFortPlayerControllerAthena.ServerStartLoadingVolume | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 39 | AFortPlayerControllerAthena.ServerStartUnloadingVolume | exec | 21 | 80.5 | portable | RPC (validate pair where present) |
| 40 | AFortPlayerPawn.MulticastUpdateVehicleInputStateReliable | exec | 15 | 80.2 | needs names | RPC (validate pair where present) |
| 41 | AFortPlayerPawnAthena.FastSharedReplication | exec | 15 | 80.2 | needs names | RPC (validate pair where present) |
| 42 | AFortPawn.PlaySound | exec | 20 | 80.0 | portable | RPC (validate pair where present) |
| 43 | AFortPlayerControllerAthena.ClientPreviewScreenshotModeActivated | exec | 20 | 80.0 | portable | RPC (validate pair where present) |
| 44 | AFortPlayerPawn.ServerChooseGender | exec | 20 | 80.0 | portable | RPC (validate pair where present) |
| 45 | AFortPlayerControllerAthena.ServerAddToCachedPurchased | exec | 24 | 77.3 | portable | RPC (validate pair where present) |
| 46 | AFortPlayerPawn.IsSkydiving | exec | 9 | 92.2 | portable | other small body |
| 47 | AFortPlayerPawn.IsSkydivingFromBus | exec | 9 | 92.2 | portable | other small body |
| 48 | AFortPlayerPawn.IsWaterSprintBoosting | exec | 9 | 92.2 | portable | other small body |
| 49 | AFortGameStateAthena.OnRep_CurrentPlaylistId | exec | 8 | 91.8 | portable | other small body |
| 50 | AFortGameStateAthena.OnRep_DamageForStormCapMarking | exec | 8 | 91.8 | portable | other small body |
| 51 | AFortPlayerStateAthena.OnRep_Downs | exec | 8 | 91.8 | portable | other small body |
| 52 | AFortPlayerControllerAthena.ClientAutoEquipFirstItem_Implementation | implementation | 9 | 91.8 | needs names | other small body |
| 53 | AFortPlayerPawn.EndGhostModeExit | exec | 13 | 92.9 | portable | other small body |
| 54 | AFortPawn.StopSpinning | exec | 12 | 92.8 | portable | other small body |
| 55 | AFortPlayerPawn.SetIsInFrontEndHologram | exec | 16 | 89.8 | portable | other small body |
| 56 | AFortPlayerPawnAthena.SetShouldDropItemsOnDeath | exec | 15 | 89.6 | portable | other small body |
| 57 | AFortPlayerPawnAthena.SetShouldSkipMovementFullSimulation | exec | 15 | 89.6 | portable | other small body |
| 58 | AFortPlayerControllerAthena.UpdateRichPresenceTimer | exec | 10 | 92.5 | portable | other small body |
| 59 | AFortPlayerPawn.DriverToPassengerAnimationStopped | exec | 10 | 92.5 | portable | other small body |
| 60 | AFortPlayerPawn.PassengerToDriverAnimationStopped | exec | 10 | 92.5 | portable | other small body |

### Limits and what is left

- **Struct-valued parameters are split.** The slim project has decompiler stack analysis off, so Ghidra does not
  type stack locals. `FVector`, `FGuid`, `FGameplayTagContainer` and `FItem`-like values appear as 2 to 4 scalar
  locals. Fixing this needs stack variables created from the `LEA reg,[RSP+off]` operand before each fetch call.
- **Enum width** is assumed to be 1 byte (no UENUM underlying type data used).
- **Bitfield bools** are not placed in struct layouts (per class in `bitfields_not_placed`).
- **Vtable slots** other than the RPC pairs stay `vfn_<offset>`. All 189 controller names, and all names in the other
  tables, come from RPC thunks; no base-class virtual order was applied. The `vfn_` calls count as not portable.
- **Verdicts** are an automatic proxy over the text (score in `score_text`). The 15-function re-judgement is manual.
- **Text stage.** Ghidra cannot yet print the StepCompiledIn idiom or the vtable call in C++ form, so
  `normalize_code` does the rewrite on the text. This is reproducible, but it is not a Ghidra type.
- **Not done:** the `--improved` flag on `decompile_targets.py`. The improved run is a separate script over the same
  853 targets, and the before counts come from the existing raw outputs.
- **Reproduce:** `porting/.venv/bin/python -I porting/ghidra/improve_decompile.py [--build-tables]` (about 70 s);
  `--rank-only` re-ranks the existing outputs.


## Second pass 2: callee identities, RPC bodies, portability [observed unless marked]

Scope: the 365 callees in `behavior/BLOCKING_CALLEES.md`, 67 extra helper candidates, the shared stub addresses, and
every named RPC slot of the three verified vtables (277 slots). Tools (tracked): `porting/ghidra/probe_callees.py`
(new: decompile, callers, callees, strings and disassembly per address, rolled back), `porting/ghidra/classify_callees.py`
(new: offline evidence tags), `porting/ghidra/improve_decompile.py` (extended), `porting/ghidra/name_tables/helpers.json`
(30 entries, 23 new, each with `evidence`, `confidence` and a C `signature`). Probe output is git-ignored under
`porting/generated/callee_probe/`. Every run changed only the same five bookkeeping files of the project; the program
database was not saved.

### Callees identified

- Probed: 365 ranked callees (one run, 45 s) and 67 extra candidates (24 s). The top 60 by blocked count were read
  by hand; the rest are tagged by `classify_callees.py` (431 rows, 27 with a name, including helpers that are not ranked).
- Of the 365 ranked callees, **10 are identified** (9 high, 1 medium). **9 of them are in the top 60** (ranks 6, 11,
  17, 18, 19, 21, 22, 26, 27). Together they cover 20 of the 433 blocked-count total.
- The other top-60 callees are described, not named. The ones that are game logic and too big to name are queued in
  `porting/behavior/CALLEE_PORT_QUEUE.md` (17 entries, with decisions: port-fn, accessor, struct, verify).
- Confidence: **high** = the body or a string literal matches a UE 4.25 source line (cited in the table); **medium** =
  shape match without a source line. Call counts are whole-program references (`getReferencesTo`, call type).

| Name (applied) | Address | Identity | Confidence | Calls (whole program) |
|---|---|---|---|---|
| `FMemory_QuantizeSize` | `0x142bfce10` | FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment) (Core/Public/HAL/MemoryBase.h:113) | high | 1773 |
| `FMemory_Realloc` | `0x142bfd200` | FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment) (MemoryBase.h:93) | high | 23884 |
| `TArray8_ResizeGrow` | `0x140690bb0` | TArray<T>::ResizeGrow(int32 OldNum) for 8-byte elements (6638 call sites) | high | 6638 |
| `TArray16_ShrinkIfSparse` | `0x140690e50` | TArray<T>::ResizeTo/Shrink for 16-byte elements (258 call sites) | medium | 258 |
| `TInlineAllocator4_ResizeAllocation16` | `0x1406d94c0` | TInlineAllocator<4>::ResizeAllocation for 16-byte elements (UE 4.25 Containers/ContainerAllocationPolicies.h) | medium | 1 |
| `FWeakObjectPtr_Get` | `0x142ee5dd0` | FWeakObjectPtr::Get() (CoreUObject/Public/UObject/WeakObjectPtrTemplates.h) | high | 13086 |
| `FWeakObjectPtr_GetEvenIfPendingKill` | `0x142ee5e30` | FWeakObjectPtr::Get(bool bEvenIfPendingKill) | high | 45 |
| `FWeakObjectPtr_IsValid` | `0x142ee7880` | FWeakObjectPtr::IsValid(bool bEvenIfPendingKill=false, ...) | high | 3597 |
| `FName_EqualsEName` | `0x142cd8ed0` | FName::operator==(ENames) against a predefined name index | medium | 4200 |
| `UObject_FindFunction` | `0x142ec7720` | UObject::FindFunction(FName) = GetClass()->FindFunctionByName(InName) (CoreUObject/Private/UObject/ScriptCore.cpp:1228) | high | 1292 |
| `UObject_FindFunctionChecked` | `0x142ec7730` | UObject::FindFunctionChecked(FName) (CoreUObject/Private/UObject/ScriptCore.cpp:1233-1238) | high | 3801 |
| `GetPrivateStaticClassBody` | `0x142d698b0` | UObject GetPrivateStaticClassBody (UE 4.25 CoreUObject/Private/UObject/Class.cpp, generated per class by UHT) | high | 7004 |
| `AFortGameStateAthena_StaticClass` | `0x14290f690` | AFortGameStateAthena::StaticClass() (UHT GPSC body) | high | 1116 |
| `AFortPlayerControllerAthena_StaticClass` | `0x142a53450` | AFortPlayerControllerAthena::StaticClass() (UHT GPSC body) | high | 901 |
| `FortMovementComp_CharacterAthena_StaticClass` | `0x1429f6440` | UFortMovementComp_CharacterAthena::StaticClass() (UHT GPSC body) | high | 23 |
| `FortRuntimeOptions_StaticClass` | `0x142ac77b0` | UFortRuntimeOptions::StaticClass() (UHT GPSC body) | high | 8 |
| `FMsg_Logf_Internal` | `0x142c54b70` | FMsg::Logf_Internal(const ANSICHAR* File, int32 Line, const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...) | high | 23784 |
| `FName_FromEName` | `0x142cdbd60` | FName::FName(EName) (predefined name constructor) | medium | 578 |
| `FTimerManager_InternalClearTimer` | `0x144627760` | FTimerManager::InternalClearTimer(FTimerHandle InHandle) (Engine/Private/TimerManager.cpp:539-541) | high | 810 |
| `UFortRuntimeOptions_GetDefault` | `0x1426fdde0` | UFortRuntimeOptions::GetDefault<T>() (class default object) | medium | 1218 |
| `Stub_Empty` | `0x1406801c0` | empty function (no-op), folded | high | 2165 |
| `Stub_ReturnTrue` | `0x1406801d0` | returns true (validate default), folded | high | 67 |
| `Stub_ReturnFalse` | `0x1406801e0` | returns false, folded | high | 2129 |

Naming the helpers changed the exec verdicts only a little: 136 portable (exec) after the first pass, 149 now. The proxy
score penalises struct noise (`param_N`, `undefined`), unnamed vtable slots and gotos more than `FUN_` names.

### RPC bodies (task 2)

- Before this pass, 197 named RPC targets (the list in `BLOCKING_CALLEES.md`) had no decompile record. The causes, all in
  `apply_rpc_prototypes`: an entry shared with another slot was skipped; an entry with no function was skipped; and the
  controller table was read only to 1200 entries (`MAX_SLOTS`), so 95 named slots (up to slot 1294) were never reached.
- Now every named slot of the three tables has a record: **277 records** (70 from the first pass, 207 new). The 207
  are the 197 targets plus 10 named slots that the dump list did not include. Output:
  `porting/generated/decomp_improved/<Class>/<Function>.c`, with `index.json` (kind `implementation` or `validate`).
- The controller table now reads to 1379 entries (the cap is 4096). Prototypes applied to it: 35 to 75.
- **Folded bodies.** 141 records share their entry address with other slots. These are identical bodies that the
  linker folded (identical-code folding), not missing bodies. Verified by disassembly: `0x1406801d0` is `MOV AL,1; RET`
  (returns true, the validate default, 72 slots), `0x1406801c0` is `RET` (empty, 53 slots), `0x140f6b550` is
  `CMP [RDX+8],1; SETG AL; RET` (an FString argument is non-empty, 6 slots), and `0x1406801e0` is `XOR AL,AL; RET`
  (returns false). The first-pass note that these entries are "padding, the entry is elsewhere" was wrong. An
  unlisted `ret` is the body.
- The RPC header is rewritten in the text (`FUN_xxx` becomes `Class_Function`, validate returns `bool`), so a folded
  body is scored under its own RPC name.
- **Tail jumps.** Seven records are a single tail jump through a vtable slot (`ClientInternalEquipWeapon_Implementation`
  at `0x14230e590` jumps through `+0xc58`). They are noted as thunks, not bodies.
- The 197 targets: **125 portable, 37 needs names, 35 not usable** (proxy). 139 of the 197 are folded bodies.

### Portability before and after (proxy verdict)

| Set | Raw (first decompile) | After pass 1 | After pass 2 (final) |
|---|---|---|---|
| 853 exec targets (portable / needs names / not usable) | 0 / 296 / 557 | 136 / 586 / 131 | **149 / 575 / 129** |
| RPC records of pass 1 (70) | not decompiled | 1 / 27 / 42 | 5 / 23 / 42 |
| RPC records new in pass 2 (207) | not decompiled | none | 128 / 40 / 39 |
| All records (923 in pass 1, 1130 final) | | 137 / 613 / 173 | **282 / 638 / 210** |

### Top candidates (second pass)

The ranking is the proxy score (`porting/generated/decomp/portability.json`, `top60`). The top 60 has 25 getters and
predicates, 20 RPC exec wrappers with a validate pair, 8 validate bodies and 7 implementation bodies. Most of the
bodies are the folded trivial bodies above (`return true`, empty), so the list rewards short bodies, not value.
The game-logic candidates that matter most are in `CALLEE_PORT_QUEUE.md`.

| Rank | Function | Kind | Lines | Score | Verdict | Category |
|---:|---|---|---:|---:|---|---|
| 1 | AFortPawn.IsSpinning | exec | 13 | 92.9 | portable | getter/setter/predicate |
| 2 | AFortPawn.IsStaggered | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 3 | AFortPlayerPawn.HasRespawnedInAir | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 4 | AFortPlayerPawn.OnRep_ReplicatedIsInSlipperyMovement | exec | 14 | 92.9 | portable | getter/setter/predicate |
| 5 | AFortGameStateAthena.HasValidFocalPointActor | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 6 | AFortPlayerControllerAthena.IsClientNotifiedOfLoss | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 7 | AFortPlayerControllerAthena.IsClientNotifiedOfTeamWin | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 8 | AFortPlayerControllerAthena.IsClientNotifiedOfWin | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 9 | AFortPlayerControllerAthena.IsLoadingScreenActive | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 10 | AFortPlayerPawn.IsInCharacterCustomization | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 11 | AFortPlayerPawn.OnRep_ParachuteLockedOpen | exec | 10 | 92.5 | portable | getter/setter/predicate |
| 12 | AFortPawn.GetControlRecoveryBehavior | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 13 | AFortPawn.GetLocalSpin | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 14 | AFortPawn.IsAllowChainStun | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 15 | AFortPawn.IsDBNO | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 16 | AFortPawn.OnlySpinWhenFalling | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 17 | AFortPlayerPawn.GetIsSlopeSliding | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 18 | AFortPlayerPawn.GetStasisMode | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 19 | AFortPlayerPawn.IsInFrontEndHologram | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 20 | AFortPlayerPawn.IsInSkyTube | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 21 | AFortPlayerPawn.IsInSlipperyMovement | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 22 | AFortPlayerPawn.IsInVortex | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 23 | AFortPlayerPawn.IsParachuteForcedOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 24 | AFortPlayerPawn.IsParachuteLockedOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 25 | AFortPlayerPawn.IsParachuteOpen | exec | 9 | 92.2 | portable | getter/setter/predicate |
| 26 | AFortPlayerControllerAthena.ClientSetStreamingTestStreamingLevels | exec | 15 | 89.6 | portable | RPC (validate pair where present) |
| 27 | AFortPlayerControllerAthena.ServerSetStreamingTestStreamingLevels | exec | 15 | 89.6 | portable | RPC (validate pair where present) |
| 28 | AFortPlayerControllerAthena.ClientSendTeamStatsForPlayer | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 29 | AFortPlayerControllerAthena.ServerSetMinigameClassSlot | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 30 | AFortPlayerControllerAthena.ServerSetOnCreativePortalLoadedDelegate | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 31 | AFortPlayerControllerAthena.ServerStartInteractWithIslandPortal | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 32 | AFortPlayerControllerAthena.ServerTeleportViaCreativePortal | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 33 | AFortPlayerControllerAthena.ServerTickForDoneTeleporting | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 34 | AFortPlayerControllerAthena.ServerTryFindPlotItemForDefinition | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 35 | AFortPlayerPawn.ClientAcknowledgeVehicleInputState | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 36 | AFortPlayerPawn.ServerInterrogateDBNOPlayer | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 37 | AFortPlayerStateAthena.ClientReportDBNO | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 38 | AFortPlayerStateAthena.ClientReportKill | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 39 | AFortPlayerStateAthena.ClientReportTeamKill | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 40 | AFortPlayerStateAthena.Client_OnNewLevel | exec | 15 | 86.2 | portable | RPC (validate pair where present) |
| 41 | AFortPawn.ClientInternalEquipWeapon | exec | 14 | 85.8 | portable | RPC (validate pair where present) |
| 42 | AFortPlayerPawn.ServerHoistDBNOPlayer | exec | 14 | 85.8 | portable | RPC (validate pair where present) |
| 43 | AFortPlayerControllerAthena.ServerRequestObjectTrackingDetails | exec | 15 | 82.2 | needs names | RPC (validate pair where present) |
| 44 | AFortPlayerControllerAthena.ClientNotifyLost | exec | 18 | 81.6 | portable | RPC (validate pair where present) |
| 45 | AFortPlayerControllerAthena.ClientReceiveKillNotification | exec | 18 | 81.6 | portable | RPC (validate pair where present) |
| 46 | AFortPlayerControllerAthena.OnPostGameForced_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 47 | AFortPlayerControllerAthena.OnPostGameForced_Validate | validate | 7 | 98.2 | portable | other small body |
| 48 | AFortPlayerStateAthena.Server_SetCanEditCreativeIsland_Validate | validate | 7 | 98.2 | portable | other small body |
| 49 | AFortPawn.ServerInternalEquipWeapon_Validate | validate | 7 | 98.2 | portable | other small body |
| 50 | AFortPawn.ServerTeleportNearLocation_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 51 | AFortPawn.ServerTeleportNearLocation_Validate | validate | 7 | 98.2 | portable | other small body |
| 52 | AFortPlayerControllerAthena.ClientAddProfileGoCollection_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 53 | AFortPlayerControllerAthena.ClientReceiveSquadQuickChatMessage_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 54 | AFortPlayerControllerAthena.ResetMyCurrentCreativePlot_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 55 | AFortPlayerControllerAthena.ResetMyCurrentCreativePlot_Validate | validate | 7 | 98.2 | portable | other small body |
| 56 | AFortPlayerControllerAthena.SendClientPhysicsBallStateToServer_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 57 | AFortPlayerControllerAthena.SendClientPhysicsBallStateToServer_Validate | validate | 7 | 98.2 | portable | other small body |
| 58 | AFortPlayerControllerAthena.SendPhysicsBallHitToServer_Implementation | implementation | 7 | 98.2 | portable | other small body |
| 59 | AFortPlayerControllerAthena.SendPhysicsBallHitToServer_Validate | validate | 7 | 98.2 | portable | other small body |
| 60 | AFortPlayerControllerAthena.ServerAddSimulatedXp_Validate | validate | 7 | 98.2 | portable | other small body |

### Limits and what is left

- **Proxy only.** The verdict is the score in `score_text`. It is not a correctness check. The 15-function sample in
  "Sample of 15" was not re-judged in this pass.
- **Identities are per address** in the slim program (the runtime exe). Re-verify them if the binary changes.
- **Medium identities** (`TArray16_ShrinkIfSparse`, `TInlineAllocator4_ResizeAllocation16`, `FName_EqualsEName`,
  `FName_FromEName`, `UFortRuntimeOptions_GetDefault`) are shape matches. The Stub and `StaticClass` names are exact
  (folded bodies or the class-name literals).
- **Another process rewrote `porting/generated/reflection/native_classes_index.json` during one improve run** (03:53 to
  03:54). The run failed to parse it once and passed on the retry. The file is not ours and was not touched.
- **Remaining unidentified:** the 365 ranked callees minus 10, of which 17 are queued. The classifier tags the rest.
- **Reproduce:**
  `porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input <list.json> --out porting/generated/callee_probe/<dir> --mem 32G`,
  `porting/.venv/bin/python -I porting/ghidra/classify_callees.py --probe <dir> --out <dir>`,
  `porting/.venv/bin/python -I porting/ghidra/improve_decompile.py --mem 32G` (about 90 to 140 s).


## Third pass: the top blocker, empty exec thunks, delegate parameter order [observed unless marked]

Scope: the top of `behavior/BLOCKING_CALLEES.md` (`0x1406a9dc0`) and its callers, the folded exec and validate bodies,
and the 223 unprobed callees that block two or more records. The 223 were probed with `ghidra/probe_callees.py` into a
scratch directory (outside the repo; one Ghidra process; the project was rolled back and the same five bookkeeping
files changed as before). The probe inputs are the callees with `blocked >= 2` that had no probe output yet.

### Identities

| Name (proposed) | Address | Identity | Confidence | Evidence |
|---|---|---|---|---|
| `TMulticastScriptDelegate_ProcessMulticastDelegate` (was `FUN_1406a9dc0`, rank 1) | `0x1406a9dc0` | `TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>(void*) const`, UE 4.25 `Core/Public/UObject/ScriptDelegates.h` (around line 479) | high | Copies the invocation list into `TArray<TScriptDelegate, TInlineAllocator<4>>` (the 64-byte stack buffer is 4 x 16 bytes; the copy is `FUN_1406d94c0`). Per entry: bound = name not None and `Get()` non-null and `FindFunction` non-null, then `ProcessDelegate`: `Object->ProcessEvent(FindFunctionChecked(name), Parms)` through vtable `+0x210` (the slot index was not counted against the 4.25 header); else compactable (name None or `!Get(true)`), then `RemoveInternal`, which searches the live list and `RemoveAtSwap`s the match (copy last over it, then shrink, `FUN_140690e50`). The loop continues after a skipped entry, as in the source. **Correction:** the earlier note "stop at the first entry whose name is None or whose function is missing" is wrong: nothing stops the loop. |
| `FUN_145803c70` | `0x145803c70` | Stack cookie check (`XOR reg,RSP` then `CALL`, MSVC pattern) | high | Compiler artifact in every function with a stack buffer. Ignored in the unresolved-call counts. |
| `FUN_142c0c240` | `0x142c0c240` | Copy of an `FText` from a static text returned by `FUN_1407c2c80`, with a shared-reference increment and a zeroed flags word | medium | 53 bytes; `LOCK` increment on the reference count at +8. Not used by any port. |
| `FUN_140f6b550` (folded) | `0x140f6b550` | Validate body shared by 6 slots: `return 1 < ArrayNum(first argument)` (an `FString` or `TArray`, ArrayNum at +8) | high for the body; medium for the mapping | Each exec thunk passes its first container argument first. The six mappings rest on vtable slot names, so they are held back. |
| `FUN_1406801c0` (folded) | `0x1406801c0` | Empty body (`ret 0`) | high | Already known (`_shared/impl_empty.inc`). |
| exec entry `0x14075b6b0` (folded, 17 exec records) | `0x14075b6b0` | `P_FINISH` then `JMP 0x1406801c0`, so the native body is the shared empty stub | high | Asm of the entry. 15 are ported as empty bodies (the other two belong to `AFortGameModeAthena`, which has no generated header here). |
| `FUN_140685370` | `0x140685370` | TArray compaction: removes entries whose pointer is null or whose virtual `+0x20` returns true, then resizes | low-medium | Not ported. Needs the element type and the slot name. |

### Parameter block order of multi-parameter delegates (finding for the generator)

The asm of three delegate call sites shows the parameter block in the reverse of the order in the generated
`DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` macro:

- `FOnMutatorGameplayEvent` (macro: EventParam3, EventParam2, EventParam1, EventId). The struct copy writes
  EventId, EventParam1, EventParam2, EventParam3 at offsets 0, 4, 8, 12 (body `0x1419a6bb0`).
- `FOnMutatorGenericIntegerUpdated` (macro: NewIntegerValue, GenericIntegerIndex). The block holds the index k at
  offset 0 and `MutatorGenericInt_k` at offset 4 (bodies `0x1419a6c10`, `0x1419a6c40`, `0x1419a6c70`).
- `FOnUsingFocalPointChanged` (macro: bUsingFocalPoint, FocalPointActor, FocalPointUser). The block holds `this` at 0,
  `FocalPoint` at 8 and `true` at 0x10 (body `0x1419f1f70`).

All three agree with "block order = reverse of macro order", and the reversed names are semantically sensible (the
index is the constant, the value is the field). The snippets pass the values by name, which reproduces the bytes. They
are MEDIUM for this reason. The generator owner should confirm the order with one more delegate before relying on it.

### Snippets added (35; 202 ported in total)

- Empty bodies (15): `AFortPawn` LogChugSplashEvent, MovingEmoteStopped; `AFortPlayerPawn` DebugDestroyVehicle,
  ToggleBodyType, ToggleGender; `AFortPlayerPawnAthena` ForceReviveFromDBNO; `AFortGameStateAthena`
  ConditionalStopAutoPerfCapture, StopAutoPerfCaptureTimer, OnRep_CurrentPlaylistId, OnRep_DamageForStormCapMarking;
  `AFortPlayerStateAthena` OnRep_Downs; `AFortPlayerControllerAthena` ClearDroppableItems,
  EnableCreativeContentManagerBasedOnPlaylist, PreLobbyIslandTeleport, RemoveDroppableItems.
- Delegate broadcasts (14, through `0x1406a9dc0`): `AFortGameStateAthena` HandleGameModeMessageDefaultPosition,
  OnRep_WinningScore, OnRep_StormCapState, OnRep_bPlaylistStoppedSafeZonePhases, OnRep_MutatorEventData (MEDIUM),
  OnRep_MutatorGenericInt_0/1/2 (MEDIUM); `AFortPlayerStateAthena` OnRep_TeamScore, OnRep_TeamScorePlacement,
  OnRep_TeamAverageDamageDealt; `AFortPlayerPawn` AttemptInstantInteractSearch; `AFortPlayerControllerAthena`
  OnRep_FocalPoint, ServerLookedAtFocalPoint_Implementation (MEDIUM).
- FVector and float (5): `AFortPlayerPawn` GetPreviousPosition, GetPreviousVelocity, SetPreviousPosition,
  SetPreviousVelocity, EndGhostModeExit.
- Validate (1, MEDIUM): `AFortPlayerPawn` ServerChooseGender_Validate (`return Gender < 4;`, unique body `0x14234a140`).

### Not ported (uncertain or inconsistent)

- Six creative-plot validates on the folded body `0x140f6b550`: the body is consistent with every call, but the
  mapping to the names depends on vtable slot names.
- `ServerChoosePart_Validate`, `ServerCycleAccessoryColorSwatch_Validate`, `ServerCycleColorSwatch_Validate`: their
  shared folded bodies compare the second argument register, which is not the enum argument the exec passes
  (`bNext` or `Chosen`). Inconsistent, so not ported.
- `ClientBroadcastOn*_Implementation` (the `FText` family): calls `FUN_14079d060` (unidentified). The text is built
  and then not passed to the delegate, which passes only the bool. Intent unclear; not ported.
- `OnRep_StormShieldComponent` (reads a gap field at `+0xf0`), `ClientAlertLeaveIsland_Implementation` (virtual `+0x14d8`
  unnamed), `OnlySpinWhenFalling` (byte `+0x63c`, a gap), the `UAnimMontage` accessors (`GetPawnLOD`,
  `GetPawnSignificance`), `GetDBNOHoister`, `IsStaggered` (gap bytes).
- `AFortGameModeAthena` functions (`GetServerBotManager`, `GetBattleLabTeleporterManager`, `IsMapInfoInitialized`,
  `AddContentKeysToGameState`, `FireServerPerformanceEvent`): bodies are simple, but no generated header exists for the
  class in `native/`.
- The `ServerChoosePart`, `ServerCycleAccessoryColorSwatch` and `ServerCycleColorSwatch` validates: see the bullet above.

### Next blockers

1. `0x1419d3ff0` (17 records; accessor over vtable `+0x150`, then `IsA AFortGameStateAthena`). Needs the UObject or
   AActor virtual order from the 4.25 headers to name `+0x150`.
2. `0x140685370` (11 records alone). Needs the element type and the predicate slot `+0x20`.
3. `FUN_142c0c240` (10 records alone) and `FUN_14079d060`: the `FText` family. Name `FUN_14079d060` first.
4. `0x1406aaec0` (6 records alone): copy constructor of the GameplayCue struct; needs the struct name.
5. Generator: the 130 `proposed` declarations (exec, validate, impl, onrep) and the delegate parameter order above.

## Fourth pass: blocker identities, delegate order, generator gaps [observed unless marked]

Scope: the four blockers of the third pass (`0x1419d3ff0`, `0x140685370`, `FUN_142c0c240` and `FUN_14079d060`, `0x1406aaec0`),
the delegate parameter-order check with more delegates, and the manifest hygiene (`behavior/MANIFEST.json`). Probe: `ghidra/probe_callees.py`
on five addresses (one Ghidra process, about 13 s; outputs in the session scratchpad, not in the repo; the project was rolled back and
the same bookkeeping files under `fortnite-slim.rep` changed as before).

### vtable slot order (Win64 shipping layout, from the engine headers)

Counted from `Engine/Source/Runtime/CoreUObject/Public/UObject/{UObjectBase,UObjectBaseUtility,Object}.h` and `Engine/Classes/GameFramework/Actor.h`.
Rules used: a virtual destructor takes one slot (MSVC scalar deleting destructor); virtuals inside `#if WITH_EDITOR` /
`WITH_EDITORONLY_DATA` are dropped (both are 0 in the client shipping build); `#if WITH_ENGINE` is kept (1).

| Slot | Offset | Function | Evidence |
|---:|---:|---|---|
| 0 to 2 | 0x00 to 0x10 | `UObjectBase`: `~UObjectBase`, `RegisterDependencies`, `DeferredRegister` | `UObjectBase.h` lines 60, 77, 87 |
| 3 to 6 | 0x18 to 0x30 | `UObjectBaseUtility`: `CanBeClusterRoot`, `CanBeInCluster`, `CreateCluster`, `OnClusterMarkedAsPendingKill` | `UObjectBaseUtility.h` 337 to 357 |
| 7 to 59 | | `UObject` virtuals (filtered), starting `GetDetailedInfoInternal` | `Object.h` |
| 42 | 0x150 | `UObject::GetWorld() const` | `Object.h` line 557, `[if WITH_ENGINE]`; the 36th kept `UObject` virtual |
| 66 | 0x210 | `UObject::ProcessEvent(UFunction*, void*)` | `Object.h` line 1164; 66 * 8 = 0x210. Confirmed: the multicast dispatcher `0x1406a9dc0` calls `+0x210` with two arguments |

So `+0x150` is `GetWorld()` and `+0x210` is `ProcessEvent`, both HIGH (the count gives the same slot for both; a non-MSVC layout would move `ProcessEvent` to `+0x218`, which the binary does not use).
Slot 4 (`+0x20`) is `UObjectBaseUtility::CanBeInCluster() const` on a UObject; the predicate of `0x140685370` uses it on an element pointer whose type is not established (see below).

### Identities

| Name (proposed) | Address | Identity | Confidence | Evidence |
|---|---|---|---|---|
| accessor `GetFortGameStateAthena` (was `FUN_1419d3ff0`) | `0x1419d3ff0` | `Cast<AFortGameStateAthena>(GetWorld()->GameState)`: calls `GetWorld()` (slot `+0x150`) on its argument, reads `UWorld::GameState` at `+0x130`, and returns it if `IsA(AFortGameStateAthena::StaticClass())` | high (slots, IsA) / high (`+0x130`) | The `+0x130` offset is `GameState` in the reflection group of `UWorld` (group 4926, property `GameState`, offset 0x130; the same group has `AuthorityGameMode` at 0x128 and `AISystem` at 0x138). The IsA test is `UStruct::IsChildOfUsingStructArray` (`Class.h` 262): the decompile compares the parent's chain index with the child's chain count and the chain entry with the parent's address. `FUN_14290f690` is `AFortGameStateAthena::StaticClass`. |
| `FUN_142c0c240` | `0x142c0c240` | `FText::FText()` default constructor | high | `Core/Private/Internationalization/Text.cpp` line 221: `TextData(FText::GetEmpty().TextData), Flags(0)`. The decompile calls `FUN_1407c2c80` (`FText::GetEmpty`), copies the two-word `TSharedRef`, increments the reference count at +8 of the controller and writes 0 to the flags dword. |
| `FUN_14079d060` | `0x14079d060` | `FText::operator=(const FText&)` (defaulted copy assignment) | high (behaviour), medium (name) | Copy-and-swap of the `TSharedRef` (temporary copy with +1, swap, release of the old value), then the flags dword is copied. This is the `TSharedRef` assignment; the name is by behaviour, the header not checked. |
| `FUN_140685370` | `0x140685370` | Compaction of a `TArray` of 16-byte elements `{ T* Ptr @+0, int32 Flag @+8 }` with a predicate (`TArray::RemoveAll`-like) | low-medium | Removes element `i` when `Flag == 0`, or `Ptr == null`, or `Ptr->vfn[+0x20]()` is true (`FUN_140690780(this, i, 1, 1)`, probably `RemoveAt`); then resizes through `FUN_1406911b0` when the count changes. The element type `T` is not known; 11 callers and 10 data references would name it. Not ported. |
| `0x1406aaec0` | `0x1406aaec0` | Copy constructor of a 0xB8-byte struct: fields at +0 and +4 (4-byte), a pointer-and-count pair at +0x10 (refcount +1, non-atomic), 8-byte fields, two `TArray` members at +0x30 and +0x50 (copy assignment `FUN_143e8a690`) | low-medium (type) | Shape matches `FGameplayCueParameters` (floats, handle, two tags, two tag containers, vectors, weak pointers, levels) in name order, but the decompile writes a constant address (`DAT_1459aede0`) at +0x08 that no 4.25 member explains, and the increment is non-atomic. Not named, not ported. |

### Delegate parameter order: confirmed on two more delegates

The binary's UFunction parameter table (`generated/reflection/fortnite_game_reflection.json`, `params[].offset`) and the asm agree on this:
the parameter block lays out the parameters in the reverse of the macro order. Three data points now:

| Delegate | Macro order (generated header) | Binary offsets | Reading |
|---|---|---|---|
| `FOnMutatorGameplayEvent` (earlier pass) | EventParam3, EventParam2, EventParam1, EventId | EventId 0, EventParam1 4, EventParam2 8, EventParam3 12 | reversed |
| `OnInGameLoadScreenChanged` (new) | HUDReasonText, bEnableLoadScreen, PlayerController | PlayerController 0, HUDReasonText 16, `bEnableLoadScreen` (bit, no offset in the table) | reversed |
| `IslandCreationCompleted` (new, reflection name) | Reason, bSuccess | Reason 8, `bSuccess` (bit, no offset in the table) | reversed (the bool is at 0 in the asm of the `ClientBroadcastOn*_Implementation` bodies, which broadcast a bool and an FText at offsets 0 and 8) |

The two new rows match the rule. The bool offsets are not in the table; the asm gives 0. This is the finding for the generator owner: the generated `Broadcast()` and the Parms struct must follow the binary offsets, not the macro order.

### Corrections to the third pass

- `ClientBroadcastOn*_Implementation` (the FText family): the FText is not dropped. The body builds a parameter block `{ bool bSuccess @0; FText Reason @8 }` (`FUN_142c0c240` constructs the FText in the block, `FUN_14079d060` assigns `Reason` to it, `FUN_1406a9dc0` broadcasts). Its only unnamed call was `FUN_14079d060`, now named, so the body is fully named.
- Blocked by the generator, not by a callee: the broadcast target is the member at `+0x2xxx` of `AFortPlayerControllerAthena`, an `InlineMulticastDelegate` that the generated header does not declare. Offsets from the reflection table: `OnMakeNewCreativePlotFinished` 0x2DD0, `OnDestroyCreativePlotFinished` 0x2DE0, `OnRestoreCreativePlotFinished` 0x2DF0, `OnDuplicateCreativePlotFinished` 0x2E00, `OnUpdateCreativePlotName` 0x2E30, `OnUpdateCreativeDescriptionTags` 0x2E40. Eight `InlineMulticastDelegate` properties of the class are missing from `native/Source/FortniteGame/Generated/AFortPlayerControllerAthena.h` (these six, plus `InGameMatchmakingQueuedStatusUpdated` and `InGameMatchmakingStateChanged`). The six bodies stay skipped until the generator emits the members; a snippet must not name a member that the generated class does not have.

### Blockers that did not unblock any function

- `0x1419d3ff0` (17 records): every caller also calls `FUN_141991bf0` (queue row 5), `FUN_141a3c2b0` (row 2) or `FUN_141a3c390` (row 3), which are not ported. Checked for the `OnRep_Num*`, `OnRep_Kills`, `OnRep_AIKills`, `OnRep_DeathInfo` and `OnClientVolumeManagerReplicated` callers. The accessor's identity is recorded in `behavior/CALLEE_PORT_QUEUE.md`; no snippet was added.
- `FUN_142c0c240` and `FUN_14079d060`: the callers that use only them are the six `ClientBroadcastOn*` bodies above; the others (`GetPlaylistGameTypeText`, `OnServerMarkerAdded`, `ClientStartRespawnPreparation`, `ServerAddSimulatedXp`) also call unnamed helpers.

### Manifest hygiene (task 1)

- `ServerSendZiplineState_Implementation`, `ServerSendZiplineState_Validate` (AFortPlayerPawn) and `ClientReceiveSquadQuickChatMessage_Implementation` (AFortPlayerControllerAthena): now `skipped`, `signature_status: not_declared`, no `inc_path`, reason in `notes`. Their parameter structs (`FZiplinePawnState`, `FAthenaQuickChatActiveEntry`) are not generated (no match in `native/Source`), so the RPC declarations cannot be emitted.
- `SetDamageCameraShakeClass` (AFortPlayerPawnAthena): signature is now `void SetDamageCameraShakeClass(TSubclassOf<UCameraShake> NewValue)` (the reflection type; `native/Source/.../AFortPlayerPawnAthena.h` declares `DamageCameraShakeClass` as `TSubclassOf<UCameraShake>`). Body re-checked: the decompile stores the 8-byte argument unchanged at +0x3070 and makes no class check; `TSubclassOf` is one `UClass` pointer, so the body is `DamageCameraShakeClass = NewValue;` (medium, unchanged).
- The stale "Declared in generated header" lines were refreshed in 57 snippets (no: the generator emits the declaration when the splice is on; yes: the header has it), and in `_shared/validate_return_true.inc`.
- `python3 -I porting/behavior/check_behavior.py`: 199 ported, 196 skipped, FAIL 0, pending declarations 3 (`SetDamageCameraShakeClass`, `GetPreviousPosition`, `GetPreviousVelocity`). Counts in `behavior/COUNTS.txt`.
- Note for the build owner: the promote run at 16:05 used the manifest before these edits. Re-run the promote step after this change, because the splice report now sees the new signature and the three demoted entries.

### Snippets (task 3)

No new snippets this pass. Ported total: 199 (202 before the three demotions). Every function these identities unblock is still blocked by another unnamed callee or by the generator's missing delegate members (listed above).

### Next blockers

1. Generator: emit the eight missing `InlineMulticastDelegate` members of `AFortPlayerControllerAthena`; then the six `ClientBroadcastOn*_Implementation` bodies can be ported (their body is fully named). Also make the generated delegate Parms follow the binary offsets (reverse of the macro order, see above).
2. `FUN_141991bf0` (queue row 5, pair at `+0x1df8`/`+0x1df0` of `AFortGameStateAthena`), then `FUN_141a3c2b0` (row 2) and `FUN_141a3c390` (row 3). Together they block the 17 callers of `0x1419d3ff0`, which is now named.
3. `0x140685370`: the element type of the 16-byte `{ptr, flag}` array. Look at its data references (`0x148463250`, `0x14734ef38`, `0x14734ef6c`) for the owning type.
4. `0x1406aaec0`: confirm the layout of `FGameplayCueParameters` in this build (the constant at +0x08).
5. Generator: the 129 `proposed` declarations, as before.

## Fifth pass: parameter order confirmed on the thunks (generator) [observed]

The generator owner's check of the parameter-order finding (fourth pass) is done, and it covers functions as well as delegates.
The reflection lists parameters in reverse declaration order, and the recorded offsets ascend in declaration order (all 331
delegates and 4,492 functions with two or more offsets). The binary agrees on the wire side:

- `AFortPlayerPawn::ServerCyclePart` (`generated/decomp/AFortPlayerPawn/ServerCyclePart.c`): the thunk reads the `Part` byte
  (offset 0) before the bool, and calls `ServerCyclePart_Validate` with `(Part, bNextPart)`. Declaration order is `(Part, bNextPart)`.
- `AFortPlayerPawn::ServerHandlePickup` (`ServerHandlePickup.c`): reads `Pickup` (offset 0), `InFlyTime` (8), `InStartDirection`
  (12), then the bool, and calls `_Validate(Pickup, InFlyTime, InStartDirection, bPlayPickupSound)`.

The generator now spells parameters in declaration order (`--param-order offset`, the default). The 27 manifest signatures that
were in table order were corrected in `behavior/MANIFEST.json`, and `AFortPlayerPawn::GetPreviousPosition` and
`GetPreviousVelocity` are non-const (no `FUNC_Const` in the reflection). `IslandCreationCompleted` (`UFortClientPilot_GameplayCreative`)
and `OnInGameLoadScreenChanged` (`AFortMinigame`) are ordinary UFunctions, not delegates: their parameter blocks follow the same rule.

## Sixth pass: creative and zipline bodies, queue rows 2, 3 and 5 [observed unless marked]

Scope: the nine bodies the fifth pass re-enabled (task a), queue rows 5, 2 and 3 with their callees, `0x140685370` and `0x1406aaec0`.
No Ghidra run: the probe outputs under `porting/generated/callee_probe/` and the improved decompile were enough. The generator
(`gen_native_classes.py`) was read only, and its `--promote` run went into a scratch copy of `native/Source` (see the end).

### Bodies (task a): 9 ported (208 in total)

- Six `AFortPlayerControllerAthena::ClientBroadcastOn*Finished_Implementation` (MakeNew, Destroy, Duplicate, Restore,
  UpdateCreativeDescriptionTags, UpdateCreativePlotName). Each body builds `{ bool bSuccess @0; FText Reason @8 }`
  (`FUN_142c0c240` FText constructor, `FUN_14079d060` FText assignment), then `FUN_1406a9dc0` (the multicast dispatcher) on
  the member at `+0x2dd0`, `+0x2de0`, `+0x2df0`, `+0x2e00`, `+0x2e30` or `+0x2e40`. The member offsets match the reflection
  table (11728, 11744, 11760, 11776, 11824, 11840) and the eight `InlineMulticastDelegate` members are emitted now. The
  snippet is `Member.Broadcast(bSuccess, Reason);`. Parameters follow the generated declaration order, `(bool bSuccess, const FText& Reason)`:
  the manifest had the older reversed order, which the splice rejects, so the six signatures were corrected.
- `AFortPlayerPawn::ServerSendZiplineState_Implementation` (`0x1406801c0`, empty) and `ServerSendZiplineState_Validate`
  (`0x1406801d0`, returns true). The exec thunk `ServerSendZiplineState` (`0x142a7f7a0`) calls the validate through vtable `+4000`
  (slot 500) and then the implementation. The RPC is `UFUNCTION(Server, Reliable)` in the generated header, so the splice
  adds `WithValidation` to it. HIGH for the bodies; MEDIUM for the slot naming, as for every folded validate.
- `AFortPlayerControllerAthena::ClientReceiveSquadQuickChatMessage_Implementation` (`0x1406801c0`, empty). Signature
  corrected to `(const FAthenaQuickChatActiveEntry& ChatEntry, AFortPlayerController* SenderPC)`.

RPC declarations: no generator change is needed. The header already declares the three RPCs (`ServerSendZiplineState`,
`ServerSendZiplineState_Implementation`, `ClientReceiveSquadQuickChatMessage_Implementation`), and the splice adds
`_Validate` with `WithValidation` for the zipline validate.

### Queue rows (task b)

| Row | Address | Identity | Confidence | Evidence |
|---|---|---|---|---|
| 5 | `0x141991bf0` | `return CurrentPlaylistInfo.OverridePlaylist ? OverridePlaylist : BasePlaylist` on `AFortGameStateAthena` (accessor) | HIGH (body, offsets) | The struct with construct function `0x142a9daf0` is `PlaylistPropertyArray` (size 488, `BasePlaylist` at +0x120, `OverridePlaylist` at +0x128, both UFortPlaylist-class objects). `CurrentPlaylistInfo` is at `+0x1cd0` of `AFortGameStateAthena` (block `0x1463ea150`), so the pair is `+0x1df0` and `+0x1df8`. Not a UFunction, so the splice cannot emit it. |
| 2 | `0x141a3c2b0` | Scoring loop over the playlist's array at `+0x4e8` (count `+0x4f0`) of 0x68-byte rules; for each rule with `Type == param_2` and `Divisor > 0`: points = `(int)(param_3 / Divisor)` capped at the rule's cap (`+0x44`) if the cap is positive, times the float value of the rule's scalable float (`FUN_14070fe00`); if the new total differs from `param_4`, broadcast `PointsAddedToScore` (`this+0xf08`) with `{ (int)value @0, Type @4 }`. | MEDIUM-HIGH | The broadcast parameter order matches `FOnPointsAddedToScore(int32 NewPoints, EAthenaScoringEvent ScoreType)` in the generated header. `FUN_14070fe00` has the probe string `FScalableFloat::GetValueAtLevel` (MEDIUM; not found by grep in `Runtime/Engine`). |
| 3 | `0x141a3c390` | Stat threshold and award on `AFortPlayerStateAthena`: sums nine int32 values at `+0xebc` .. `+0xefc` (stride 8) and the int at `+0xe88`, compares with the last total at `+0xf00`; if the byte at `+0xf0` is 3, the difference is positive and vfn `+0x770` returns an object whose class is `FortTeamInfoAthena` (`FUN_142af1340`, the static-class getter, HIGH), it adds the difference to that object's `+0x370`. It stores the new total at `+0xf00`, and when the byte is not 3 it calls the listener dispatch `FUN_1407ac640`. | LOW-MEDIUM | Unnamed: vfn slot `0x770` (slot 238 of `AFortPlayerStateAthena`, not in the name tables), `FUN_1407ac640` (the dispatcher below). The reflection gives the stat properties (`AIKillCount` `+0xeb0`, `NumChestsOpened` `+0xeb8`, ... `NumGoldCoinsCollected` `+0xef8`) as int32 at the offsets 4 bytes below these reads. The OnRep callers show the same 4-byte shift in the decompile, so the offset relation is unresolved: check it against the asm before porting row 3. |
| - | `FUN_1407ac640` | Dispatch over a lock-counted list: lock counter `+0x14`, count `+0x8`, data `+0x0`, elements 16 bytes `{ Ptr @0, int32 Flag @8 }`; calls vfn `+0x50` on each element with two arguments; compacts through `0x140685370` when an element fails. | LOW-MEDIUM | Same container as `0x140685370`. The element type is not known. |
| - | `0x140685370` | Compaction of that list (remove where `Flag == 0`, `Ptr == null`, or the vfn `+0x50` predicate is false), then resize. | LOW-MEDIUM | The three data references the third pass named (`0x148463250`, `0x14734ef38`, `0x14734ef6c`) are 32-bit RVA tables (the values are functions in `0x1406853xx` to `0x14073xxxx`). They are not a type descriptor and do not name the element. |
| 7 | `0x1406aaec0` | Unchanged: copy constructor of an 0xB8-byte struct (the `DAT_1459aede0` constant at +0x08 still has no 4.25 member). | LOW-MEDIUM | Not named. |

Earlier identities stand: `0x1419d3ff0` (row 4, accessor, HIGH), `FUN_142c0c240` (FText constructor), `FUN_14079d060` (FText
assignment). New: `FUN_142af1340` = `FortTeamInfoAthena::StaticClass` (HIGH; the registration arguments name the class and its size
0x398 in `/Script/FortniteGame`).

### Port status (task c): nothing new ported, and why

- Rows 2 and 3 are non-reflected member helpers. The generator splices only `getter` and `exec` (`porting/gen_native_classes.py`,
  the `splice_names` set and the kind dispatch near lines 1248 and 1409 to 1461), plus the RPC kinds. A `callee` entry has no splice
  path, so a snippet cannot call a helper it does not own. Row 5 is in the same position.
- Every caller of `0x1419d3ff0` (the `OnRep_Num*`, `OnRep_Kills`, `OnRep_AIKills`, `OnRep_DeathInfo` and `OnClientVolumeManagerReplicated`
  family) also calls row 3 (and row 2), so the 17 callers stay blocked. Inlining the rows into each caller would copy about 60 lines
  into 13 snippets, so it was not done.
- Rows 2 and 3 also need the listener element type (`0x1407ac640` and `0x140685370`), which the decompile does not give.

### Splice check (no change to `native/`)

`gen_native_classes.py --promote` was run with `--source-root` and `--report-dir` pointed at a scratch copy of `native/Source`
(the scratchpad, outside the repo; the reflection inputs were read only). The checker against that output reports 208 ported,
208 spliced and verified, 0 rejected, FAIL 0. The live `native/` tree still shows the nine new entries as FAIL until the build owner
re-runs `porting/promote_native_classes.sh`. Snippets are not compile-checked: the six broadcast bodies and the zipline bodies
use only generated members and the engine names allowed by the contract, so the build owner should watch `AFortPlayerControllerAthena`
(`ClientBroadcastOn*`) and `AFortPlayerPawn` (`ServerSendZiplineState_Validate`) after the next build.

### Next blockers

1. Generator: a splice kind for non-reflected member helpers (declaration plus `.inc` body), so rows 2, 3 and 5 can be called.
   Without it the 17 callers of `0x1419d3ff0` cannot be ported.
2. The lock-counted 16-byte list family (`0x1407ac640`, `0x140685370`): name the owner container and the element type. It blocks row 3 and the
   OnRep callers by itself.
3. vfn slot `0x770` of `AFortPlayerStateAthena` (slot 238): name it, and check the `FortTeamInfoAthena` `+0x370` field from the reflection table.
4. `FScalableFloat::GetValueAtLevel` (`FUN_14070fe00`): confirm the 4.25 signature in the engine source (not in `Runtime/Engine/Public`).
5. The playlist scoring rule struct (0x68 bytes, array at `+0x4e8`, count at `+0x4f0`) and the playlist byte at `+0x467`: name both from the reflection table.

## Seventh pass: custom net serializers and replication conditions [observed unless marked]

Scope: the `struct_custom_netserialize` gaps of `class_checksum.py` (50 replicated properties and 8 RPC parameters in
the Fort package, which made 109 classes non-parity through their ancestors) and the missing `COND_*` of
`GetLifetimeReplicatedProps`. Done read-only on the slim project (`fortnite-slim`, opened without analysis; functions the
database does not have were created in memory and the transaction rolled back, nothing saved; one Ghidra process at a
time). Tool: `porting/ghidra/net_lifetime_serializers.py` (`inputs`, `probe`, `build`). Outputs are scratch-only.

### Which structs have a custom serializer

The ops vtable of each struct (`struct_blocks.json`, `struct_ops.vtable_va`) is a `TCppStructOps<T>`. The slots follow the
`ICppStructOps` declaration order in UE 4.25 `UObject/Class.h`: 12 `HasNetSerializer`, 13 `HasNetSharedSerialization`,
14 `NetSerialize`, 15 `HasNetDeltaSerializer`, 16 `NetDeltaSerialize`. The trait slots agree with `binary_analysis.md`
section 9. 92 structs have one of the three traits: 53 in FortniteGame, 1 in PlayspaceFramework, 1 in Lobby, 2 in
MantisRuntime (not promoted), and 35 engine-package structs (16 Engine, 17 GameplayAbilities, 2 GameplayTags; not generated,
their serializers are engine code). `STRUCT_NetSerializeNative` is not in `StructFlags` (set at load time), so the trait is
the identification.

### Family 1: FFastArraySerializer structs (HasNetDeltaSerializer, 45 Fort/Playspace/Lobby/Mantis)

- Identity, high: the slot 16 target of each struct decompiles to a function of 1,220 to 1,300 lines that contains the
  `FastArrayDeltaSerialize for %s. %s. Reading / Writing / GatherGuidReferences / MovedGuidToUnmapped ...` log strings and
  the item type name (for example `FortSpectatorZoneItem` in `FFortSpectatorZoneArray`; the `probe` step of
  `porting/ghidra/net_lifetime_serializers.py` writes `serializers/FFortSpectatorZoneArray_slot16.c`). That is `FFastArraySerializer::FastArrayDeltaSerialize<Item, Self>`
  inlined by the compiler (declared in `Engine/NetSerialization.h`).
- All 45 (43 FortniteGame, 1 PlayspaceFramework, 1 Lobby) have the same shape: the parent is `FFastArraySerializer`, and one
  TArray member has `FFastArraySerializerItem`-derived structs (checked from the reflection's `super_fn` and array inner records). The generator emits `NetDeltaSerialize`
  as `FFastArraySerializer::FastArrayDeltaSerialize<Item, Self>(Items, DeltaParms, *this)` and
  `TStructOpsTypeTraits<F...>` with `WithNetDeltaSerializer = true`. 43 are emitted; `FLobbyPlayerStateInfoArray` (Lobby is not
  a promoted package) and `FPickupInstigatorArray` (not emittable for another reason) are not.
- Bit layout: the engine template (item ReplicationID and ReplicationKey per element, the array's own key). Not recovered by
  hand; it is the engine's, so the confidence is in the identity, not in a re-derived layout. Medium-high.
- Engine FastArray structs (GameplayAbilities `ActiveGameplayEffectsContainer`, `ActiveGameplayCueContainer`, ...) are engine
  code and are not emitted by the generator. Unchanged.

### Family 2: other custom NetSerialize structs (HasNetSerializer / HasNetSharedSerialization, Fort)

Identity of the slot 14 target (decompiled by the `probe` step, `serializers/<Struct>_slot14.c`): the function body was read, not re-derived. None is
emitted with a native body. Each is emitted as a declared fallback: no trait, and a `TODO` comment in the header. The
replicated uses serialize with the default property serialization, which is not the original wire format.

| Struct | Slot 14 target | Lines | What the decompile shows | Confidence |
|---|---|---|---|---|
| `FAthenaPawnReplayData` | `0x142a82160` | 354 | long field-by-field body (not read through) | low |
| `FCreateBuildingActorData` | `0x142a3dc40` | 100 | not read through | low |
| `FFortAIAttributeReplicationProxy` | `0x1427a19e0` | 11 | two calls through FArchive slot `0x168`, at `param+0` and `param+4` (two 4-byte fields) | medium (field type not known) |
| `FFortAISharedRepMovement` | `0x1427a19f0` | 104 | not read through | low |
| `FortGameplayAbilityTargetData_SingleTargetHit` | `0x142788290` | 33 | two uint32 at `+0x98` and `+0x9c` through the bit reader (`0x29 & 0x20` checks) | medium |
| `FortGameplayEffectContext` | `0x14077d350` | 12 | tail jump through vtable slot `0xc0` (not resolved) | low |
| `FortRootMotionSource_FallingBoost` | `0x1409dc190` | 12 | tail jump through a vtable slot the decompiler cannot resolve | low |
| `ReplicatedPhysicsPawnState` | `0x142a25f80` | 80 | not read through | low |
| `SharedRepMovement` | `0x142a82170` | 267 | bit-packed movement state, not read through | low |

Emitted fallbacks: `FAthenaPawnReplayData`, `FCreateBuildingActorData`, `FFortAIAttributeReplicationProxy`, `FFortAISharedRepMovement`,
`FReplicatedPhysicsPawnState`, `FSharedRepMovement`, `FAthenaBatchedDamageGameplayCues_Shared` (shared serialization only). The
other three are not used by any emitted member. The ones in this family are the work item for a later pass: the native bodies
are the next step, after the field types of `SharedRepMovement` and `ReplicatedPhysicsPawnState` are checked.

Three shared bodies appear in the slots and were not treated as structs: `0x1406801e0` (a two-instruction stub that the
slots of the structs without a custom serializer point to; not decoded further here), `0x1406801c0` and `0x1406801d0`
(see `porting/behavior/README.md`).

### Replication conditions (GetLifetimeReplicatedProps)

Method (`porting/ghidra/net_lifetime_serializers.py`): for each class with owned replicated properties, the VA of each
property name's UTF-16 literal is found in the dump. The registration function is the caller of `RegisterReplicatedLifetimeProperty`
that references the most of the class's literals (the two overloads of this exe are `0x144678530` for a descriptor and
`0x144678610` for an `FProperty*`, identified by the call shape of `UFortHomebaseSet` and `AFortPlayerControllerAthena`).
Each registration's `FDoRepLifetimeParams` is read from the stores to its block. Inferred from the decompile and checked on
AActor (below): Condition at the block address, RepNotifyCondition at block-4, bIsPushBased at block-8 (the decompiler's stack
names run opposite to the addresses). A block written once and passed to several registrations keeps its last value. A
2-argument registration is the default `DOREPLIFETIME`. The layout is the 4.25 struct (`UnrealNetwork.h`), and the
`COND_*` values are the 4.25 `CoreNetTypes.h` values.

Validation, against the UE 4.25 source: `ACharacter` (8 properties) 8 of 8; `APawn` (3) 3 of 3 including `RemoteViewPitch`
`COND_SkipOwner`; `AActor` (10) 10 of 10 including `AttachmentReplication` (`COND_Custom`, `REPNOTIFY_Always`, push) and
`ReplicatedMovement` (`COND_SimulatedOrPhysics`, `REPNOTIFY_Always`, push), and the `SharedParams` push flags. Push flags are
recorded but not emitted: a push-model registration needs the native bodies that mark the property dirty, which are not
recovered yet.

Coverage: the 1,715 owned replicated properties of the 325 classes that have any. Per property: 1,659 high, 5 medium
(registered in two branches, the first is used: `AFortPlayerState`, `ABuildingActor`, `ABuildingSMActor`), 51 unknown. The
conditions are: COND_None 1,468, COND_OwnerOnly 99, COND_InitialOnly 48, COND_SkipOwner 40, COND_SimulatedOnly 22,
COND_Custom 27, COND_ReplayOnly 9, and one each of COND_InitialOrOwner and COND_SkipReplay; 109 use REPNOTIFY_Always. Unknown
means the property is not in the chosen function (for example `AFortGameStateAthena` 7 properties, `AFortMinigame` 3) or the
class has no registration function found (`UFortControlResistanceSet`, 9 properties). Those are emitted as `COND_None` with a
TODO. The data file also holds 45 engine and validation classes (AActor, ACharacter, APawn, ...) that the generator does not use. `REPNOTIFY_Always` on properties without a
RepNotify in the reflection (68 of the 109, on 20 classes: 19 attribute-set-like `*Set` classes, which use the GAS
`GAMEPLAYATTRIBUTE_REPNOTIFY` pattern, and `AFortRemoteControlledPawnAthena`, not checked further). The reflection does not
record `ReplicatedUsing` for these, so they are kept as decoded.

Data: `porting/replication_conditions.json` (generated, tracked). The generator emits `DOREPLIFETIME_CONDITION` /
`DOREPLIFETIME_CONDITION_NOTIFY` in `--promote`; the lines of a low-confidence property carry the confidence in a comment.

### Checksum effect

`class_checksum.py` parity (native set, 4,572 classes): 4,445 to 4,535 (non-parity 127 to 37). Remaining causes:
`struct_other_module` 16 (`FProfileUpdateSingle`, `FMeshNetworkStatus`, `FAthenaRewardResult`, ...), RPC flags without a role 13,
`struct_member_unresolved` 5, enums 3, `static_or_const_rpc` 1. No class that was parity-yes became non-parity.

## Eighth pass: lock-counted delegate list, vfn 0x770, scoring rules, FScalableFloat, FGameplayCueParameters [observed unless marked]

Scope: queue rows 2, 3 and 5 (bodies and their callees), the lock-counted 16-byte list family (`0x1407ac640`, `0x140685370`), the
virtual slot `+0x770` of `AFortPlayerStateAthena`, `FUN_14070fe00`, the playlist scoring structs, and `0x1406aaec0`.
Research only: no snippet, MANIFEST, native/, generator or git change. No Ghidra process was started this pass; every
decompile and disassembly below is an existing probe output or was taken from the read-only runtime dump bytes with
`objdump -D -b binary -mi386:x86-64` (not in the repo). Reflection: `generated/reflection/*.json`. Engine: `/opt/fortnite/UnrealEngine/Engine/Source`
and `Engine/Plugins/Runtime/GameplayAbilities/Source` (the 4.25 tree; `FScalableFloat` and `FGameplayCueParameters` live in the plugin).

### Identities

| Name (proposed) | Address | Identity | Confidence | Evidence |
|---|---|---|---|---|
| `Broadcast_PlayerStateInt32` (was `FUN_1407ac640`) | `0x1407ac640` | `TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>::Broadcast`, UE 4.25 `Core/Public/Delegates/DelegateSignatureImpl.inl:998` | HIGH (structure), MEDIUM-HIGH (argument types) | Lock++ at `+0x14`; reverse loop over the list; `vfn +0x50` on each element (`ExecuteIfSafe`); a failed call sets the compaction flag; lock--; then `CompactInvocationList` (`0x140685370`, flag 0). Callers pass the list, the player state and an int32 (17 `call` references in the binary's xref data). |
| `FMulticastDelegateBase::CompactInvocationList` (was `0x140685370`) | `0x140685370` | UE 4.25 `Core/Public/Delegates/MulticastDelegateBase.h` | HIGH | `InvocationList` (`TArray<FDelegateBase>`) at `+0`, `CompactionThreshold` at `+0x10`, `InvocationListLockCount` at `+0x14` (the 4.25 member order, 0x18 bytes). Returns when locked; with the flag set returns when `--threshold > Num`; removes null entries and entries whose `vfn +0x20` (`IsCompactable`) is true with `RemoveAtSwap` (`0x140690780`); `threshold = max(2, 2*Num)`; `Shrink` (`0x1406911b0`) when the old count exceeds the threshold. Replaces the "element type unknown" of the third and sixth passes. |
| element of the list | 16 bytes at `list+0` | `FDelegateBase`: `DelegateAllocator` (heap pointer to an `IDelegateInstance`) at `+0`, `DelegateSize` (int32) at `+8`; 0 means empty | HIGH | `Core/Public/Delegates/DelegateBase.h` (allocator alias `void*`, `DelegateSize`). The asm null-tests both the size and the pointer. |
| vfn slot numbers of the instance | | Shipping build (`UE_BUILD_SHIPPING`): `TryGetBoundFunctionName` is compiled out (`Delegates/DelegateSettings.h`). Slots: `GetUObject` 0, `GetObjectForTimerManager` 1, `GetBoundProgramCounterForTimerManager` 2, `HasSameObject` 3, `IsCompactable` 4 (`+0x20`), `IsSafeToExecute` 5, `GetHandle` 6, `~IDelegateInstance` 7, `CreateCopy` 8, `Execute` 9, `ExecuteIfSafe` 10 (`+0x50`) | HIGH | Two independent offsets agree with this count: `+0x20` is the no-argument bool (`IsCompactable`, used by `0x140685370`) and `+0x50` is the bool call with two arguments (`ExecuteIfSafe(PlayerState, value)`, used by `0x1407ac640`). `IBaseDelegateInstance<void(...)>` in `Delegates/DelegateInstanceInterface.h:34-47`. |
| four native stat-change delegates of `AFortPlayerStateAthena` | `+0x1120`, `+0x1150`, `+0x1180`, `+0x11b0` | `TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>`, 0x18 bytes each (list, threshold, lock at `+0x14` of each). `+0x1120` = KillScore (OnRep_Kills, the `FUN_1407ac640(this+0x1120, this, KillScore)` tail call); `+0x1150` = TeamKillScore (OnRep_TeamKillScore; the decompile's `PlayerNameCustomOverride + 0x170` is the same address); `+0x1180` = TotalPlayerScore (UpdateTotalPlayerScore; OnRep_TotalPlayerScore inlines the same broadcast, lock at `+0x1194`); `+0x11b0` = AIKillCount (OnRep_AIKills) | MEDIUM-HIGH | Not in the reflection (native delegates are not reflected). The names are placeholders: `FOnKillScoreChangedNative` and so on. The generated header declares none of them (no gap members), so snippets cannot name them yet. |
| `Broadcast_Global` (`0x140684de0`, not identified) | `0x140684de0` | The same `Broadcast` template on a static list (the list pointer is loaded from `0x148351eb8`). Callers: OnRep_Kills, OnRep_AIKills, OnRep_DeathInfo, ClientNotifyTeamLost. The argument list is not established. | LOW-MEDIUM | Asm: `inc [rcx+0x14]`, reverse loop, `call [rax+0x50]`, unlock, compaction. Next blocker. |
| vfn `+0x770` of `AFortPlayerStateAthena` (slot 238, vtable `0x145e35fa8`) | `0x1424192a0` | `GetPlayerTeam()`: returns `PlayerTeam` (`AFortTeamInfo*`, reflected Object at `+0x640` of `AFortPlayerState`, replicated, `OnRep_PlayerTeam`) | HIGH (body), MEDIUM (declared name) | The asm is two instructions: `mov rax,[rcx+0x640]; ret`. The vtable read is from the dump (`vtable[238]`). |
| `FortTeamInfoAthena +0x370` | `AFortTeamInfo` `+0x370` | A non-reflected int32 in `AFortTeamInfo`, directly after the reflected `PrivateInfo` (Object at `+0x368`). `AFortTeamInfoAthena::BotInfo` is at `+0x388` (class size `0x398`, registration in the sixth pass). The Authority branch of the score update adds the player's score delta to it. | MEDIUM (class and offset), LOW (name) | `0x141a3c390` asm: `add [rbp+0x370], esi` after the `IsA` test. No other function in the decompiles touches it. |
| `FScalableFloat::GetValueAtLevel` (was `FUN_14070fe00`) | `0x14070fe00` | `float FScalableFloat::GetValueAtLevel(float Level, const FString* ContextString) const`, GameplayAbilities `Private/AttributeSet.cpp:421` | HIGH | Field-by-field against the 4.25 source: `LocalCachedCurveID` at `+4` against `UCurveTable::GetGlobalCachedCurveID()` (global `0x147cdd300`); `Curve.CurveTable` at `+8`; `FinalCurve` at `+0x18`; `Curve.GetCurve(ContextString)` at `0x144166df0`; `FRealCurve::Eval` at `vfn +0x88`; default context literal `"FScalableFloat::GetValueAtLevel"` at `0x1459c5a60`. FScalableFloat is 0x20 bytes: `Value` at `+0`, the curve handle at `+8`, the cache at `+0x18`. The reflection's `ScalableFloat` record (32 bytes, `Value` at 0, `Curve` at 8) agrees. |
| playlist `+0x467` | `UFortPlaylistAthena::WinConditionType` | `EAthenaWinCondition` (3 = `FirstToGoalScore`, `EAthenaWinCondition.h:15`) | HIGH | Reflection offset 0x467; OnRep callers and row 2 gate on `== 3`. |
| playlist `+0x4a8` / `+0x4e8` / `+0x4f0` | `ScoringData` (`FWinConditionScoreData`, 80 bytes) | `GoalScore` (FScalableFloat) at `+0`, `BigScoreThreshold` (FScalableFloat) at `+0x20`, `ScoreDataList` (`TArray<FAthenaScoreData>`) at `+0x40` | HIGH | Reflection: `ScoringData` Struct at 0x4a8 (struct `0x142a97fe0`, size 80); `ScoreDataList` at 80-byte offset 64. Row 2 reads the data pointer at `playlist+0x4e8` and the count at `+0x4f0`. |
| scoring rule (0x68 = 104 bytes) | `FAthenaScoreData` | `ScoringEvent` (EAthenaScoringEvent, uint8) at `+0`; `ScoreNameText` (FText) at `+8`; `EventInclusionTags` (FGameplayTagContainer) at `+0x20`; `NumOccurrencesForScore` (int32, divisor) at `+0x40`; `NumOccurrencesPermitted` (int32, cap) at `+0x44`; `ScoreAwarded` (FScalableFloat) at `+0x48` | HIGH | Reflection record `AthenaScoreData` (block `0x147b75d70`, size 104) and the generated `FAthenaScoreData.h` agree; row 2 asm uses `+0`, `+0x40`, `+0x44` and `+0x48` (the FScalableFloat call). |
| `GetCurrentPlaylistAthena` (was `0x141991bf0`) | `0x141991bf0` | `return CurrentPlaylistInfo.OverridePlaylist ? OverridePlaylist : BasePlaylist` on `AFortGameStateAthena` (`CurrentPlaylistInfo` at `+0x1cd0`, `FPlaylistPropertyArray`: `BasePlaylist` at `+0x120`, `OverridePlaylist` at `+0x128`, both `UFortPlaylistAthena*`) | HIGH | Asm: `mov rax,[rcx+0x1df8]; test; jne; mov rax,[rcx+0x1df0]; ret`. Reflection `CurrentPlaylistInfo` at 0x1cd0 and the struct offsets 288 and 296. |
| `GetFortGameStateAthena` (was `0x1419d3ff0`, fourth pass) | `0x1419d3ff0` | `Cast<AFortGameStateAthena>(Object->GetWorld()->GameState)` | HIGH | Re-read in the asm: vfn `+0x150` with a null test, `UWorld::GameState` at `+0x130` with a null test, `IsA` through the class depth and the SuperStructs array against `StaticClass` (`0x14290f690`); returns the game state or null. |
| `UpdateScoreForEvent` (row 2, `0x141a3c2b0`) | `0x141a3c2b0` | `int32 AFortPlayerStateAthena::UpdateScoreForEvent(EAthenaScoringEvent Event, int32 Count, int32 PreviousPoints, UFortPlaylistAthena* Playlist)` | HIGH (body), MEDIUM (names) | Pseudocode below. Every callee is now named: `0x14070fe00` (FScalableFloat), `0x1406a9dc0` (the dynamic broadcast of `PointsAddedToScore` at `this+0xf08`, third pass). Returns the points of the last matching rule. |
| `UpdateTotalPlayerScore` (row 3, `0x141a3c390`) | `0x141a3c390` | `void AFortPlayerStateAthena::UpdateTotalPlayerScore()` | HIGH (body), MEDIUM (names) | Pseudocode below. Callees: `0x1424192a0` (PlayerTeam), `0x142af1340` (FortTeamInfoAthena::StaticClass, sixth pass), `0x1407ac640` (Broadcast). |
| `FGameplayCueParameters` copy constructor (row 7, `0x1406aaec0`) | `0x1406aaec0` | Implicit copy constructor of `FGameplayCueParameters` (`GameplayAbilities/Public/GameplayEffectTypes.h:922`, 0xB8 bytes; reflection block `0x14782dd20`, size 184) | HIGH | Asm layout check (below). The constant at `+8` is the vtable `0x1459aede0` of `FGameplayEffectContextHandle`: the handle has a virtual destructor (`GameplayEffectTypes.h:596`), so it is 24 bytes (vptr `+0`, `TSharedPtr` `+8`); the dtor `0x1406afe20` stores that vtable at offset 0. The previous "+0x08 constant mismatch" is this vptr. |
| `FGameplayCueParameters` destructor (row 6, `0x1406ae340`) | `0x1406ae340` | Implicit destructor: frees the four TArray buffers at `+0x30`, `+0x40`, `+0x50`, `+0x60` (two `FGameplayTagContainer` members), then releases the handle's controller | MEDIUM-HIGH | Compiler-generated; port as `= default` (no body needed). |
| `FGameplayTagContainer::operator=` (row 9, corrected) | `0x143e8a690` | Copy-assign of two `TArray<FGameplayTag>` (8-byte elements, FName): `GameplayTags` at `+0`, `ParentTags` at `+0x10`, memcpy of count*8 after a resize (`0x1406d9a30`) | HIGH (shape), MEDIUM (name; engine-wide, 1252 call sites) | Third pass called it a generic TArray copy; the second array at `+0x10` shows it is the 2-array container. |
| `FUN_142a8ae70` | `0x142a8ae70` | `AFortPlayerStateAthena::GetPrivateStaticClass` (the UClass registration with `/Script/FortniteGame`, `FortPlayerStateAthena`, size 0x1400, calls `FUN_142d698b0`) | HIGH | `generated/decomp-vtable/AFortPlayerStateAthena/GetPrivateStaticClass__142a8ae70.c`. It was the unnamed callee of two PC bodies. |
| `FUN_142cd3770` | `0x142cd3770` | Ends a stat scope for `STAT_InvNPerfHook_PSA_RCO`, gated by the byte at `0x1482472e4` (profiling only) | LOW | Not identified beyond the gate. Port as a no-op (`SCOPE_CYCLE_COUNTER`-style). Two bodies call it (OnRep_NumChestsOpened, OnRep_NumMinutesAlive). |

### The 10 reads of the total (queue row 3): offset relation resolved

Row 3 reads ten int32 values (asm of `0x141a3c390`: `+0xefc, +0xef4, +0xeec, +0xee4, +0xedc, +0xed4, +0xecc, +0xec4, +0xebc, +0xe88`).
Nine of them are the reflected `Int` counters at `+4`. Each OnRep caller writes the points of its stat into that `+4` slot:

| Stat (reflected offset) | Cache read by row 3 | Scoring event (written by) |
|---|---|---|
| `KillScore` (0xe74) | `0xe88` (not `+4`) | 1 Elimination (OnRep_Kills) |
| `AIKillCount` (0xeb0) | `0xeb4` (**not summed**) | 11 AIKilled (OnRep_AIKills) |
| `NumChestsOpened` (0xeb8) | `0xebc` | 2 ChestOpened |
| `NumAmmoCansOpened` (0xec0) | `0xec4` | 3 AmmoCanOpened |
| `NumSupplyDropsOpened` (0xec8) | `0xecc` | 4 SupplyDropOpened |
| `NumLlamasOpened` (0xed0) | `0xed4` | 5 SupplyLlamaOpened |
| `NumForagedItemsConsumed` (0xed8) | `0xedc` | 6 ForagedItemConsumed |
| `NumMinutesAlive` (0xee0) | `0xee4` | 7 SurvivalInMinutes |
| `NumBronzeCoinsCollected` (0xee8) | `0xeec` | 8 CollectedCoinBronze |
| `NumSilverCoinsCollected` (0xef0) | `0xef4` | 9 CollectedCoinSilver |
| `NumGoldCoinsCollected` (0xef8) | `0xefc` | 10 CollectedCoinGold |

So the "+4" shift of the sixth pass is the per-stat points cache (the value `UpdateScoreForEvent` returns), not a counter offset.
The AI-kill cache is written by OnRep_AIKills and never read into the total (observed; a runtime check would confirm whether that is intended).

### Callers of `0x1419d3ff0` (the 17 OnRep and RPC bodies) after this pass

The table gives the unnamed callees each body still had after the queue's named helpers, then after this pass (`decomp_improved` files).

| Body | Other unnamed callees before | After this pass |
|---|---|---|
| OnRep_NumAmmoCansOpened, OnRep_NumBronzeCoinsCollected, OnRep_NumForagedItemsConsumed, OnRep_NumGoldCoinsCollected, OnRep_NumLlamasOpened, OnRep_NumSilverCoinsCollected, OnRep_NumSupplyDropsOpened | rows 2, 3, 5 | none: **ready to port** (needs a helper splice and the cache members) |
| OnRep_NumChestsOpened, OnRep_NumMinutesAlive | rows 2, 3, 5; `FUN_142cd3770` | `FUN_142cd3770` only (profiling no-op): **ready** |
| OnRep_Kills | rows 2, 3, 5; `FUN_1407ac640` | `FUN_140684de0` (global Broadcast), `FUN_1423def80`, `FUN_142a6ee20` |
| OnRep_AIKills | rows 2, 3, 5; `FUN_1407ac640` | `FUN_140684de0`, `FUN_1423def70`, `FUN_142a6ee20` |
| OnRep_DeathInfo | row 5 only (no rows 2, 3) | about 80 unnamed (`FUN_1406800e0`, `FUN_140684de0`, `FUN_142a6ee20`, `FUN_141a2d0b0`, ...): low priority |
| OnClientVolumeManagerReplicated (PC) | none of rows 2, 3, 5 | `FUN_140694550`, `FUN_1419c1a40`, `FUN_1419e0370`, `FUN_141c74820`, `FUN_141c7d420` |
| ClientNotifyTeamLost_Implementation (PC) | none | `FUN_140684de0`, `FUN_140f95ae0` (row 11), `FUN_1419922b0`, `FUN_1419967f0`, `FUN_142bad740`, `FUN_142ee3b30`, `FUN_144621fb0` (row 16), `FUN_1446278d0` |
| ClientSendMatchStatsForPlayer_Implementation (PC) | none | `FUN_141b5d450`, `FUN_141b6bb40`, `FUN_141e8db50`, `FUN_141ed1800`, `FUN_141ed2350`, `FUN_14463f970` |
| ServerClearSkydiveLeader_Implementation (PC) | none | `FUN_1419982c0`, `FUN_1419e89a0` (`FUN_142a8ae70` is now named) |
| Server_SetCanEditCreativeIsland_Implementation (PS) | none | `FUN_14199e3c0`, `FUN_1419c3880` (row 14), `FUN_141a2c5b0`, `FUN_141f0a3c0`, `FUN_1423a0aa0`, `FUN_142414c60`, `FUN_14297b320` |

The fourth pass said that every caller of the accessor also calls row 2, row 3 or row 5. That is true for the twelve stat and death
bodies but not for the five PC and PS bodies in the last three rows, which have their own blockers. The accessor alone unblocks none of them.

### Ready-to-port pseudocode (UE 4.25 API)

These are the bodies for the next round. Names are proposed; the member names are the generated header's where they exist. The
non-reflected members (the caches and the four delegates) need the generator (see "Generator requirements" below). The asm checks are
in the identity table above.

```cpp
// 1. GetFortGameStateAthena  (0x1419d3ff0)  HIGH
AFortGameStateAthena* GetFortGameStateAthena(const UObject* Object)
{
    UWorld* World = Object->GetWorld();                      // vfn +0x150
    if (!World) return nullptr;
    AGameStateBase* GS = World->GameState;                   // UWorld +0x130
    return (GS && GS->IsA(AFortGameStateAthena::StaticClass()))
        ? static_cast<AFortGameStateAthena*>(GS) : nullptr;
}

// 2. GetCurrentPlaylistAthena  (0x141991bf0)  HIGH
UFortPlaylistAthena* GetCurrentPlaylistAthena(const AFortGameStateAthena* GameState)
{
    const FPlaylistPropertyArray& Info = GameState->CurrentPlaylistInfo;
    return Info.OverridePlaylist ? Info.OverridePlaylist : Info.BasePlaylist;
}

// 3. UpdateScoreForEvent  (0x141a3c2b0, queue row 2)  HIGH body
int32 AFortPlayerStateAthena::UpdateScoreForEvent(EAthenaScoringEvent Event, int32 Count,
                                                  int32 PreviousPoints, UFortPlaylistAthena* Playlist)
{
    int32 Result = 0;
    const TArray<FAthenaScoreData>& Rules = Playlist->ScoringData.ScoreDataList;
    for (int32 i = 0; i < Rules.Num(); ++i)
    {
        const FAthenaScoreData& Rule = Rules[i];
        if (Rule.ScoringEvent != Event || Rule.NumOccurrencesForScore <= 0)
            continue;
        int32 Points = Count / Rule.NumOccurrencesForScore;             // idiv: truncates toward zero
        if (Rule.NumOccurrencesPermitted > 0 && Points > Rule.NumOccurrencesPermitted)
            Points = Rule.NumOccurrencesPermitted;
        const int32 Unit = (int32)Rule.ScoreAwarded.GetValueAtLevel(0.f);   // level 0, null context
        Result = Unit * Points;
        if (Result != PreviousPoints)
            PointsAddedToScore.Broadcast(Unit, Event);                  // FOnPointsAddedToScore(NewPoints, ScoreType)
    }
    return Result;                                                      // last matching rule; 0 if none
}

// 4. UpdateTotalPlayerScore  (0x141a3c390, queue row 3)  HIGH body
void AFortPlayerStateAthena::UpdateTotalPlayerScore()
{
    // Ten point caches. The AI-kill cache (+0xeb4) is not in the sum.
    const int32 NewTotal = KillPointsCache        // +0xe88
        + ChestPointsCache     + AmmoCanPointsCache   + SupplyDropPointsCache + LlamaPointsCache
        + ForagedPointsCache   + MinutesPointsCache   + BronzePointsCache     + SilverPointsCache
        + GoldPointsCache;                            // +0xebc, +0xec4, ..., +0xefc

    if (Role == ROLE_Authority)                       // AActor::Role at +0xf0
    {
        const int32 Delta = NewTotal - TotalPlayerScore;               // +0xf00
        if (Delta > 0)
        {
            AFortTeamInfo* Team = GetPlayerTeam();                     // vfn +0x770, PlayerTeam at +0x640
            if (Team && Team->IsA(AFortTeamInfoAthena::StaticClass()))
            {
                Team->TeamPointsAccumulator += Delta;                  // AFortTeamInfo +0x370 (non-reflected)
            }
        }
    }
    TotalPlayerScore = NewTotal;
    if (Role != ROLE_Authority)
    {
        OnTotalPlayerScoreChangedNative.Broadcast(this, NewTotal);     // native list at +0x1180
    }
}

// 5. TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>::Broadcast  (0x1407ac640)  HIGH structure
void Broadcast(AFortPlayerStateAthena* PlayerState, int32 Value) const
{
    ++InvocationListLockCount;                                          // +0x14
    bool bNeedsCompaction = false;
    for (int32 i = InvocationList.Num() - 1; i >= 0; --i)               // reverse order
    {
        IDelegateInstance* Instance = InvocationList[i].GetDelegateInstanceProtected();   // null if DelegateSize == 0
        if (!Instance || !static_cast<IBaseDelegateInstance<void(AFortPlayerStateAthena*, int32)>*>(Instance)
                              ->ExecuteIfSafe(PlayerState, Value))           // vfn +0x50
        {
            bNeedsCompaction = true;
        }
    }
    --InvocationListLockCount;
    if (bNeedsCompaction)
        CompactInvocationList(false);                                   // 0x140685370
}

// 6. FMulticastDelegateBase::CompactInvocationList  (0x140685370)  HIGH
void CompactInvocationList(bool bCheckThreshold)
{
    if (InvocationListLockCount > 0) return;
    if (bCheckThreshold && --CompactionThreshold > InvocationList.Num()) return;
    const int32 OldNum = InvocationList.Num();
    for (int32 i = 0; i < InvocationList.Num(); )
    {
        IDelegateInstance* Instance = InvocationList[i].GetDelegateInstanceProtected();
        if (!Instance || Instance->IsCompactable())                     // vfn +0x20
            InvocationList.RemoveAtSwap(i);                             // 0x140690780
        else
            ++i;
    }
    CompactionThreshold = FMath::Max(2, 2 * InvocationList.Num());
    if (OldNum > CompactionThreshold)
        InvocationList.Shrink();                                        // 0x1406911b0
}

// 7. FScalableFloat::GetValueAtLevel (0x14070fe00) and the implicit FGameplayCueParameters copy/dtor
//    (0x1406aaec0, 0x1406ae340) and FGameplayTagContainer::operator= (0x143e8a690): engine code. Use the 4.25 types;
//    no port body is needed.
```

### Generator requirements found by this pass

1. A splice kind for non-reflected member functions (already the top blocker): the five bodies above are named now and cannot be spliced without it.
2. Non-reflected data members used by the bodies. Either declare them as gap members with their offsets, or give the helper kind raw-offset accessors:
   - int32 caches: `+0xe88`, `+0xeb4`, `+0xebc`, `+0xec4`, `+0xecc`, `+0xed4`, `+0xedc`, `+0xee4`, `+0xeec`, `+0xef4`, `+0xefc` (on `AFortPlayerStateAthena`).
   - four `TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>` members, 0x18 bytes each, at `+0x1120`, `+0x1150`, `+0x1180`, `+0x11b0`.
   - int32 `TeamPointsAccumulator` at `AFortTeamInfo +0x370` (name is a placeholder).
3. Virtual slot 238 (`+0x770`) of `AFortPlayerStateAthena` is the `PlayerTeam` getter. A body can call `PlayerTeam` directly; the slot is not needed for the port.

### Corrections to earlier passes

- Third pass: `FUN_140685370` is `CompactInvocationList`, and `FUN_1407ac640` is the `Broadcast` of a native multicast delegate. The element is `FDelegateBase`, not an unknown type.
- Third pass: `FUN_143e8a690` is `FGameplayTagContainer::operator=` (two arrays), not a generic TArray copy.
- Sixth pass row 3: the offsets are ten reads, and the `+4` shift is the per-stat points cache, not a counter offset.
- Fourth pass: "every caller also calls row 2, 3 or 5" holds for the stat and death bodies only (see the caller table).
- Row 7 (`0x1406aaec0`): the "constant at +0x08" is the vtable of `FGameplayEffectContextHandle`; the struct is `FGameplayCueParameters`. The earlier 0xB8 versus 4.25 concern came from counting the handle as 16 bytes.

### Next blockers (ordered)

1. Generator: the helper splice kind and the non-reflected members listed above. Unblocks nine stat bodies now (rows 2, 3, 5 are bodies, not blockers) and OnRep_Kills and OnRep_AIKills after item 2.
2. `0x140684de0` (global Broadcast; four callers) and `0x142a6ee20`, `0x1423def80`, `0x1423def70` (OnRep_Kills, OnRep_AIKills, OnRep_DeathInfo). Find the global delegate's signature and the three helpers.
3. `0x142cd3770`: decide to port the profiling scope as a no-op (recommended) or identify it. Two bodies.
4. The PC and PS bodies of the last three caller rows: about 25 unnamed callees (listed in the caller table).
5. OnRep_DeathInfo: about 80 unnamed callees; not started.
6. Queue rows 8 and 10 (`0x142c64fe0`, `0x143e8a780`) were not examined this pass.

### Files touched

- `porting/decompilation_notes.md` (this section)
- `porting/behavior/CALLEE_PORT_QUEUE.md` (eighth-pass ranking)
- `porting/behavior/BLOCKING_CALLEES.md` (eighth-pass note)
- `porting/ghidra/name_tables/helpers.json` (12 new function identities, applied by `improve_decompile.py` on its next run)

Not touched: `porting/native/`, `gen_*.py`, snippets, `behavior/MANIFEST.json`, git state, `/opt/fortnite/gen-worktree`.

## Ninth pass: helpers, member fields and the stat bodies [observed unless marked]

Scope: the manifest kinds `helper`, `static_helper` and `member_field` (generator contract: `behavior/README.md`, "Helpers" and
"Member fields"). Queue rows 2, 3, 4 and 5 become ported entries, the nine `OnRep_Num*` stat bodies are ported, and the
Eighth pass non-reflected members are declared as `member_field` entries. `porting/native/` is not touched and
`gen_native_classes.py` is not edited: the splice was checked in a scratch promote only. Each body was checked against its
decompile (`porting/generated/callee_probe/blocking/0x141a3c2b0.c`, `0x141a3c390.c`, `0x1419d3ff0.c`, `0x141991bf0.c`) and against
the generated headers. No Ghidra run.

### Entries (manifest 424 entries: 237 ported, 187 skipped; 28 added, 1 renamed)

| Kind | Entry | Address | Notes |
|---|---|---|---|
| helper | `AFortPlayerStateAthena::UpdateScoreForEvent(EAthenaScoringEvent, int32, int32, const UFortPlaylistAthena*)` | `0x141a3c2b0` | queue row 2, public |
| helper | `AFortPlayerStateAthena::UpdateTotalPlayerScore()` | `0x141a3c390` | queue row 3, public |
| static_helper | `GetFortGameStateAthena(const UObject*)` in `AFortPlayerStateAthena.cpp` | `0x1419d3ff0` | queue row 4; includes `UObject/Object.h`, `Engine/World.h`, `GameFramework/GameStateBase.h` |
| helper (renamed) | `AFortGameStateAthena::GetCurrentPlaylistAthena() const`, public | `0x141991bf0` | queue row 5; was `GetActivePlaylist` (protected) |
| member_field | 11 `int32` point caches (below) | offsets | `AFortPlayerStateAthena` |
| member_field | 4 `TMulticastDelegate<void, AFortPlayerStateAthena*, int32>` | `+0x1120`, `+0x1150`, `+0x1180`, `+0x11b0` | `Delegates/Delegate.h` |
| member_field | `int32 AccumulatedPlayerScore` | `AFortTeamInfo +0x370` | the team's score accumulator |
| onrep | `OnRep_NumAmmoCansOpened`, `OnRep_NumBronzeCoinsCollected`, `OnRep_NumChestsOpened`, `OnRep_NumForagedItemsConsumed`, `OnRep_NumGoldCoinsCollected`, `OnRep_NumLlamasOpened`, `OnRep_NumMinutesAlive`, `OnRep_NumSilverCoinsCollected`, `OnRep_NumSupplyDropsOpened` | `0x142a8dad0` ... `0x142a8dbd0` | exec record addresses (decompile_improved) |

The stat bodies (decompile): the game state from `GetFortGameStateAthena`; when `GetCurrentPlaylistAthena()->WinConditionType`
is `FirstToGoalScore` (3, `EAthenaWinCondition`), the cache of the stat becomes `UpdateScoreForEvent(event, stat, cache, playlist)`;
then `UpdateTotalPlayerScore()`. `P_FINISH` (script epilogue) is not ported. `FUN_142cd3770` (profiling scope) is a no-op in
`OnRep_NumChestsOpened` and `OnRep_NumMinutesAlive`, as the Eighth pass recommended.

### Naming of the non-reflected members (the Eighth pass offsets)

Each stat's points cache is the reflected counter plus 4 bytes (the decompile writes `Context->Num* + 4`). The names are ours
(MEDIUM): the stat and the event are exact.

| Field | Offset | Stat (reflected offset) | Scoring event (value) | Summed by UpdateTotalPlayerScore |
|---|---|---|---|---|
| `KillScorePoints` | `0xe88` | `KillScore` (0xe74), OnRep_Kills (not ported) | Elimination (1) | yes |
| `AIKillCountPoints` | `0xeb4` | `AIKillCount` (0xeb0), OnRep_AIKills (not ported) | AIKilled (11) | **no** (decompile skips it) |
| `NumChestsOpenedPoints` | `0xebc` | `NumChestsOpened` (0xeb8) | ChestOpened (2) | yes |
| `NumAmmoCansOpenedPoints` | `0xec4` | `NumAmmoCansOpened` (0xec0) | AmmoCanOpened (3) | yes |
| `NumSupplyDropsOpenedPoints` | `0xecc` | `NumSupplyDropsOpened` (0xec8) | SupplyDropOpened (4) | yes |
| `NumLlamasOpenedPoints` | `0xed4` | `NumLlamasOpened` (0xed0) | SupplyLlamaOpened (5) | yes |
| `NumForagedItemsConsumedPoints` | `0xedc` | `NumForagedItemsConsumed` (0xed8) | ForagedItemConsumed (6) | yes |
| `NumMinutesAlivePoints` | `0xee4` | `NumMinutesAlive` (0xee0) | SurvivalInMinutes (7) | yes |
| `NumBronzeCoinsCollectedPoints` | `0xeec` | `NumBronzeCoinsCollected` (0xee8) | CollectedCoinBronze (8) | yes |
| `NumSilverCoinsCollectedPoints` | `0xef4` | `NumSilverCoinsCollected` (0xef0) | CollectedCoinSilver (9) | yes |
| `NumGoldCoinsCollectedPoints` | `0xefc` | `NumGoldCoinsCollected` (0xef8) | CollectedCoinGold (10) | yes |

The four delegates, all `TMulticastDelegate<void, AFortPlayerStateAthena*, int32>` (0x18 bytes each, MEDIUM-HIGH identity from
the Eighth pass; `Broadcast(this, value)` in the decompile): `KillScoreNative` (`+0x1120`, OnRep_Kills), `TeamKillScoreNative`
(`+0x1150`, OnRep_TeamKillScore; the decompile reaches it as `PlayerNameCustomOverride + 0x170`, not re-verified here),
`TotalPlayerScoreNative` (`+0x1180`, the non-authority broadcast of UpdateTotalPlayerScore), `AIKillCountNative` (`+0x11b0`).
`AccumulatedPlayerScore` (`AFortTeamInfo +0x370`) is the team's sum of the authority's positive score deltas (LOW name).

### Quirks ported as decompiled

- `UpdateScoreForEvent` broadcasts `PointsAddedToScore` with the **per-unit** value (the `(int)` of `ScoreAwarded.GetValueAtLevel(0)`)
  and the event, not the new total `unit * points`. The decompile writes the unit at the first parameter slot and the event byte
  at the second, and `FOnPointsAddedToScore` is `(int32 NewPoints, EAthenaScoringEvent ScoreType)` in the generated header.
- Points: `Count / NumOccurrencesForScore` (truncated); when `NumOccurrencesPermitted` is positive the points are capped at it; the
  broadcast happens only when `unit * points != PreviousPoints`; the return value is the last matching rule's result, or 0.
  The rule array and its count are read on each pass, as the decompile does.
- `UpdateTotalPlayerScore`: sums ten caches (not `AIKillCountPoints`); on the authority, a positive delta goes to the team when
  `PlayerTeam` `IsA AFortTeamInfoAthena`; `TotalPlayerScore` is stored in every case; the native broadcast happens **only when the
  role is not ROLE_Authority**. (The Eighth pass wording "otherwise broadcasts" meant this branch; the pseudocode there was right.)
- The stat bodies call `UpdateTotalPlayerScore()` even when the playlist gate fails, and write their cache only inside the gate.

### Corrections to earlier passes (found while porting or by the syntax check)

- **Multicast spelling.** The Eighth pass wrote `TMulticastDelegate<void(AFortPlayerStateAthena*, int32)>`. UE 4.25 spells it
  `TMulticastDelegate<void, AFortPlayerStateAthena*, int32>` (`Core/Public/Delegates/DelegateSignatureImpl.inl:1064`); the function-type
  form is undefined and the compiler rejected it. The README example was corrected too.
- **`UWorld::GameState` is private** in 4.25. The accessor reads it through `UWorld::GetGameState()` (inline, returns the same field at
  `+0x130`).
- **`AActor::Role` is private** in 4.25. The body reads it through `GetLocalRole()` (inline, `return Role`).
- **Placeholder names** of the Eighth pass (`TeamPointsAccumulator`, `OnTotalPlayerScoreChangedNative`, `FOnKillScoreChangedNative`) are
  replaced by `AccumulatedPlayerScore` and the `*Native` names above.
- **`GetPlayerTeam`.** The vtable slot `+0x770` is `AFortPlayerState::GetPlayerTeam` (`0x1424192a0`, `mov rax,[rcx+0x640]; ret`, in
  `helpers.json`). The body reads `PlayerTeam` directly. Same value unless a C++ subclass overrides it; none is in the generated
  tree, and the declaration is not in our generated headers.
- **Row 5 name.** `GetActivePlaylist` (kind helper, protected) is renamed `GetCurrentPlaylistAthena` (Eighth pass name) and made public:
  `AFortGameModeAthena::SafeZoneInsideChecks` (`decomp/AFortGameModeAthena/SafeZoneInsideChecks.c`) also calls it.
- **Argument of the accessor.** Eight of the nine stat decompiles show `FUN_1419d3ff0()` with no argument (the Chests and Minutes
  bodies pass `Context`). The port passes `this` everywhere: the accessor calls `GetWorld()` on its argument, and `this` is the only
  object in the body. Confirm in the disassembly before relying on it.

### Checks

- `python3 -I porting/behavior/check_behavior.py`: 424 entries, ported 237 (getter 32, onrep 26, impl 61, exec 25, validate 73,
  helper 3, static_helper 1, member_field 16), skipped 187; `declared_in_header` 217, `pending_declaration` 19 (16 fields, 2 helpers,
  `GetCurrentPlaylistAthena`), FAIL 0. `COUNTS.txt` rewritten by the checker.
- Scratch promote (the `promote_native_classes.sh` command with `--source-root` and `--report-dir` in the scratchpad; the
  resolved class-ref map also written there): exit 0, `behavior_splice.json` spliced **237**, rejected **0**.
  `check_behavior.py --gen-root <scratch>/FortniteGame/Generated --splice-report <scratch>/behavior_splice.json`: 237 spliced and
  verified, not spliced 0, **FAIL 0**.
- Review of the scratch output by eye: the caches and delegates sit in offset order among the reflected members; the helpers are
  declared after `GetLifetimeReplicatedProps` outside any UFUNCTION; `GetFortGameStateAthena` is in the anonymous namespace before
  the member definitions; the two new forward declarations are `class AFortGameStateAthena;` and `class UFortPlaylistAthena;`;
  `AFortTeamInfo` gains the field after `PrivateInfo`.
- Syntax check (not a full build): `clang++ -fsyntax-only` from the UE 4.25 toolchain (`Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/
  Linux_x64/v16_clang-9.0.1-centos7`, clang 9) with the `FortniteGame` compile flags of the last build (`AFortPlayerStateAthena.cpp.o.rsp`,
  `AFortGameStateAthena.cpp.o.rsp`, same `-Werror`), the scratch `Generated` directory first. The last build's UHT output
  (`*.generated.h`) was made for the old header lines, so two patched copies were used (only the `UCLASS` and `GENERATED_BODY`
  line numbers in the macro names changed; the function and property lists are unchanged). Result: `AFortPlayerStateAthena.cpp` and
  `AFortGameStateAthena.cpp`: 0 diagnostics. The first run found the three errors above; the static-helper "no matching function"
  errors of that run did not come back once they were fixed. `AFortTeamInfo` was compiled only through `AFortPlayerStateAthena.cpp`.
  Not checked: the UHT run itself, the `.gen.cpp` files, and the rest of the module.

### Decisions

- Helper access is `public` for all three new helpers and all fields: the decompile shows callers only in
  `AFortPlayerStateAthena` and one caller of `GetCurrentPlaylistAthena` in `AFortGameModeAthena`, but the whole binary is not
  decompiled, so no narrower access is claimed.
- The static helper lives in `AFortPlayerStateAthena.cpp` (its nine callers are here). Its other callers (PlayerController, PlayerState
  RPCs, GameMode, the `OnRep_Kills`/`OnRep_AIKills`/`OnRep_DeathInfo` bodies outside the stat set) need their own static_helper entry in
  their class's .cpp, or a shared home, when they are ported.
- The `Role` and `UWorld` names were not added to the checker as they are. `GetLocalRole`, `ROLE_Authority`, `UWorld` and
  `AGameStateBase` were added to `ENGINE_NAMES` (`check_behavior.py`), and the local-declaration regex now accepts references and
  `for` heads. Both are checker-only changes; they do not change what the generator accepts.

### Watch-list for the build owner

1. Promote into `native/`, then the full UBT and UHT build and both smoke variants. The checks above did not run UHT. Confirm that
   UHT accepts the forward declarations and the non-UPROPERTY `TMulticastDelegate` members inside `UCLASS`, and that `-Werror` is clean.
2. Layout: the new fields are in offset order among the reflected members, but nothing checks the C++ layout against the binary
   (no `static_assert` on offsets or the class size). If parity matters, add gap members or assert the offsets.
3. Line-keyed UHT macros: the next promote regenerates them; no manual patch is needed.
4. The `*Points` names and `AccumulatedPlayerScore` are ours (MEDIUM / LOW). Rename in the manifest and in the `.inc` files together.
5. Not ported in this pass: `OnRep_Kills` and `OnRep_AIKills` (global `Broadcast` `0x140684de0` unidentified), `OnRep_DeathInfo`,
   `OnRep_TotalPlayerScore` (inlines the same broadcast with its lock at `+0x1194`; not examined in this pass),
   `OnRep_TeamKillScore`, and the PC and PS callers.

### Next blockers (ordered)

1. Build owner: promote, build, smoke (above).
2. `0x140684de0` (global Broadcast) and `0x142a6ee20`, `0x1423def80`, `0x1423def70`: `OnRep_Kills`, `OnRep_AIKills`, `OnRep_DeathInfo`.
3. `OnRep_TotalPlayerScore`: examine, port if its inlined broadcast matches `TotalPlayerScoreNative`.
4. The PC and PS bodies (`OnClientVolumeManagerReplicated`, `ClientNotifyTeamLost_Implementation`, `ClientSendMatchStatsForPlayer_Implementation`,
   `ServerClearSkydiveLeader_Implementation`, `Server_SetCanEditCreativeIsland_Implementation`): their callees are in the caller table of the
   Eighth pass; the static helper needs an entry in each of their classes.

### Files touched

- `porting/behavior/MANIFEST.json` (28 entries added; `GetActivePlaylist` renamed; four `callee` notes updated)
- `porting/behavior/README.md` (status, ninth pass, helper/field/checker sections)
- `porting/behavior/CALLEE_PORT_QUEUE.md` (ninth pass paragraph, rows 2 to 5, ranking item 1)
- `porting/behavior/COUNTS.txt` (rewritten by `check_behavior.py`)
- `porting/behavior/check_behavior.py` (`ENGINE_NAMES`, `local_names`)
- `porting/behavior/AFortGameStateAthena/GetCurrentPlaylistAthena.inc` (renamed from `GetActivePlaylist.inc`)
- `porting/behavior/AFortPlayerStateAthena/`: `UpdateScoreForEvent.inc`, `UpdateTotalPlayerScore.inc`, `GetFortGameStateAthena.inc`, and nine `OnRep_Num*.inc`
- `porting/decompilation_notes.md` (this section)

Scratch only (not in the repository): the promote output and reports, the syntax logs, and the patched UHT headers under the scratchpad.
Not touched: `porting/native/`, `gen_native_classes.py` and its `.md`, the git index, `/opt/fortnite/gen-worktree`, `porting/ghidra/`.

## Tenth pass: rank-2 callees, the kill and team-kill bodies, rank 3 [observed unless marked]

Scope: queue rows 2 and 3 (`OnRep_Kills`, `OnRep_AIKills` and their callees), rank 3 (`0x142cd3770`), and the other OnRep bodies
the hand-off names (`OnRep_TotalPlayerScore`, `OnRep_TeamKillScore`, `OnRep_DeathInfo`, the PlayerController callers).

Method. One Ghidra run: `porting/ghidra/probe_callees.py` on the four rank-2 addresses (12.9 s; outputs in
`porting/generated/callee_probe/tenth_rank2/`, git-ignored). The lock file `fortnite-slim.lock` was stale (written 17:15, no Ghidra
process running), so the project opened. The probe reports that it changed files under `fortnite-slim.rep` (`idata/~index.*`,
`project.prp`, `versioned/~index.*`) although the transaction was rolled back; check that before the next probe. No full analysis.
Every other check was read-only: the runtime dump bytes (`objdump -D -b binary -m i386:x86-64 --adjust-vma=0x140000000`, file
offset = VA - 0x140000000; the bytes match Ghidra's listing at the four callees), the reflection JSON and the generated headers.

### Identities

| Address | Identity | Confidence | Evidence |
|---|---|---|---|
| `0x140684de0` | `TMulticastDelegate<void>::Broadcast()` (no arguments; the delegate's list is the argument) | HIGH (structure) | Probe asm: lock `inc [rcx+0x14]`, reverse loop over 16-byte entries, `call [rax+0x50]` (`ExecuteIfSafe()`, no arguments), unlock, `FMulticastDelegateBase::CompactInvocationList` (`0x140685370`) when a call failed. The Eighth pass said the list pointer is loaded from `0x148351eb8`: the asm has no such reference (corrected). |
| `0x142a6ee20` | `AFortPlayerControllerZone::StaticClass()` | HIGH | `GetPrivateStaticClassBody` with `/Script/FortniteGame`, `FortPlayerControllerZone`, size `0x2ae0`. |
| `0x1423def80` | zone controller member `+0x28f0`: `TMulticastDelegate<void>` (`ZoneKillScoreChangedNative`, name LOW) | HIGH (accessor `this + 0x28f0`), MEDIUM (type) | Probe body is `return param_1 + 0x28f0`. Its call in OnRep_Kills is followed by `0x140684de0`. No reflected property at `+0x28f0` in any class. |
| `0x1423def70` | zone controller member `+0x2908`: `TMulticastDelegate<void>` (`ZoneAIKillCountChangedNative`, name LOW) | HIGH (accessor), MEDIUM (type) | Probe body is `return param_1 + 0x2908`. It is `0x18` bytes after the kill delegate (one list type). The reflected property at `+0x2908` belongs to `AFortPlayerPawnAthena` (`ReviveFromDBNOTime`). |
| `0x1419e4f10` (not in the queue; callee of `OnRep_TeamKillScore`) | Athena controller member `+0x3210`: `TMulticastDelegate<void>` (`TeamKillScoreChangedNative`, name LOW) | HIGH (accessor: `lea rax,[rcx+0x3210]; ret`, read from the dump), MEDIUM (type) | Its result goes to `0x140684de0`. The only reflected property at `+0x3210` belongs to `AFortCharacterVehicle` (`DashMovementLockDuration`), not to this class. |
| `0x142cd3770` (rank 3) | profiling stat-scope end | HIGH that it writes no game state | Decompile: reads the thread id and cycle counters, may append the stat name, and logs "Leaving stat scope on hitch" (`0x142c54b70`) when a threshold is exceeded. Callers gate it with the byte at `0x1482472e4` (profiling). Only profiler state is written. |

### Ported (manifest 432 entries: 244 ported, 188 skipped)

| Entry | Kind | Confidence | Basis |
|---|---|---|---|
| `AFortPlayerStateAthena::OnRep_Kills` | onrep | MEDIUM | Asm of the body at `0x141a51910`: GetFortGameStateAthena, GetCurrentPlaylistAthena, UpdateScoreForEvent, UpdateTotalPlayerScore, zone delegate, then `KillScoreNative` (tail call). |
| `AFortPlayerStateAthena::OnRep_AIKills` | helper, protected | MEDIUM | Asm of `0x141a56ed0`. Not a RepNotify (`AIKillCount` is not `ReplicatedUsing`, flags `0x80401` = Final, Native, Protected), so the generated header has no UFUNCTION and the generator writes the declaration (the helper kind). Nothing in the module calls it yet. |
| `AFortPlayerStateAthena::OnRep_TotalPlayerScore` | onrep | HIGH | Asm of `0x141a53d60` is one tail call: `TotalPlayerScoreNative` (`+0x1180`) with `(this, TotalPlayerScore)`. |
| `AFortPlayerStateAthena::OnRep_TeamKillScore` | onrep | MEDIUM | Asm of `0x141a531d0`: the controller's delegate (`0x1419e4f10`), then `TeamKillScoreNative` (`+0x1150`) with `(this, TeamKillScore)`. |
| `AFortPlayerControllerZone::ZoneKillScoreChangedNative` | member_field, `+0x28f0` | MEDIUM (type), LOW (name) | `TMulticastDelegate<void>`, public. |
| `AFortPlayerControllerZone::ZoneAIKillCountChangedNative` | member_field, `+0x2908` | MEDIUM (type), LOW (name) | `TMulticastDelegate<void>`, public. |
| `AFortPlayerControllerAthena::TeamKillScoreChangedNative` | member_field, `+0x3210` | MEDIUM (type), LOW (name) | `TMulticastDelegate<void>`, public. |

Skipped: `AFortPlayerStateAthena::OnRep_DeathInfo` (see below).

### Findings

- Exec records and bodies. The `Address` of each `decomp_improved` OnRep file is the exec record. Its first instructions add
  one to a counter at `[rdx+0x20]` (`x = x + (x != 0)`; the decompile shows this as `P_FINISH`) and then tail-jump to the native
  body: `0x141a51910` (Kills), `0x141a56ed0` (AIKills), `0x141a53d60` (TotalPlayerScore), `0x141a531d0` (TeamKillScore). The
  decompile merged the two parts. The bodies are the jump targets. `P_FINISH` is not ported, as in the ninth pass.
- The `this` argument. In OnRep_Kills (and OnRep_AIKills) `rcx` still holds `this` at the call to `0x1419d3ff0`, so
  `GetFortGameStateAthena(this)` is the right reading. The ninth-pass open question is answered for these bodies (the stat bodies
  pass `this` as well).
- The `PreviousPoints` quirk of OnRep_Kills. The asm loads `KillScore` (`+0xe74`) into both the Count and the PreviousPoints
  argument (`r8d` and `r9d`), so `UpdateScoreForEvent(Elimination, KillScore, KillScore, Playlist)` and its broadcast test compares
  with the kill count, not the cache. It is ported as the shipping code does it. The other stat bodies and OnRep_AIKills pass the
  cache.
- The result goes to `KillScorePoints` (`+0xe88`), which is what the decompile's `HumanKillScore + 0xc` means (`HumanKillScore` is
  the reflected int32 at `+0xe7c`).
- Owner. `Owner` (`AActor`, reflected, `OnRep_Owner`) is at `+0xe0` (reflection offset 224). The bodies read it through the public
  accessor `GetOwner()` (checker `ENGINE_NAMES` gained `GetOwner`; checker-only).

### Blocked and why

- `OnRep_DeathInfo` (`0x142a8d9b0`): 75 distinct unnamed callees after the identities above (`FUN_140683490` 32 calls, `FUN_141ed1800` 20,
  `FUN_140680160` and `FUN_1406800e0` 16 each). Not probed: the one Ghidra run was limited to the four rank-2 callees.
- PlayerController and PlayerState callers of `GetFortGameStateAthena` (the eighth-pass caller table). Each still has unnamed callees, so
  none needs only a `static_helper` and none was ported. No new `static_helper` entry was added. (The only copy is in
  `AFortPlayerStateAthena.cpp`.) The unnamed callees: `ClientNotifyTeamLost_Implementation` (`FUN_1419922b0`, `FUN_1419967f0`,
  `FUN_142a53870`, `FUN_142bad740`, `FUN_142ee3b30`, `FUN_144621fb0`, `FUN_1446278d0`), `ClientSendMatchStatsForPlayer_Implementation`
  (`FUN_141b5d450`, `FUN_141b6bb40`, `FUN_141e8db50`, `FUN_141ed1800`, `FUN_141ed2350`, `FUN_14463f970`),
  `OnClientVolumeManagerReplicated` (`FUN_140694550`, `FUN_1419c1a40`, `FUN_1419e0370`, `FUN_141c74820`, `FUN_141c7d420`),
  `ServerClearSkydiveLeader_Implementation` (`FUN_1419982c0`, `FUN_1419e89a0`), and `Server_SetCanEditCreativeIsland_Implementation` (seven).
- The controller delegates' names (`ZoneKillScoreChangedNative`, `ZoneAIKillCountChangedNative`, `TeamKillScoreChangedNative`) are ours.
  The binary gives the offsets and the no-argument type. Name them from the code that binds them, or leave them.

### Rank 3 decision: `0x142cd3770` is a no-op

It is the end of a profiling stat scope. It reads the thread id and cycle counters and logs a hitch warning; it writes no UObject state.
Porting it would need the stats framework. Leaving it out does not change game state. The ninth pass already applies this to
`OnRep_NumChestsOpened` and `OnRep_NumMinutesAlive` (the bodies skip the call). Nothing else changes.

### Corrections to earlier passes

- Eighth pass: `0x140684de0` does not read a global list (`0x148351eb8` is not in its asm). Its list is the argument.
- Eighth pass: `0x140684de0` was called "global Broadcast". It is the no-argument `TMulticastDelegate<void>::Broadcast()`.
- The ninth-pass `GetFortGameStateAthena(this)` argument question is answered for OnRep_Kills and OnRep_AIKills (see Findings).

### Checks

- `python3 -I porting/behavior/check_behavior.py --verbose`: 432 entries, ported 244, skipped 188, 239 signatures declared, 4 pending
  declarations (`OnRep_AIKills` and the three controller delegates), FAIL 0. `COUNTS.txt` was rewritten by the checker.
- Scratch promote (not `promote_native_classes.sh`, which deletes `porting/native/Source/*/Generated`): I copied `porting/native/Source`
  to the scratchpad and ran the same `resolve_class_refs.py` and `gen_native_classes.py --promote` steps with `--source-root`,
  `--report-dir` and `--out` in the scratchpad. `behavior_splice.json`: spliced 244, rejected 0. The scratch header declares
  `protected: void OnRep_AIKills();` and the three delegates at their offsets. `check_behavior.py --gen-root <scratch>/FortniteGame/Generated
  --splice-report <scratch>/behavior_splice.json`: 244 spliced and verified, FAIL 0.
- The exact native-tree check (`check_behavior.py --gen-root porting/native/Source/FortniteGame/Generated --splice-report
  porting/generated/reflection/behavior_splice.json`) was refused by the permission check of the tool host, so it was not retried.
  The native tree predates these entries, so it cannot verify them until the tree is promoted.
- Not compiled. The entries use `Cast<T>` and `GetOwner()` (engine), `TMulticastDelegate<void>` (`Delegates/Delegate.h`) and the zone
  and Athena controller headers, which the generator includes in the .cpp (the scratch `AFortPlayerStateAthena.cpp` includes both).
  The build owner must compile them.

### Next blockers (ordered)

1. Build owner: promote into `native/`, build, run both smoke variants (now covers the seven new entries).
2. Probe the 75 callees of `OnRep_DeathInfo`, then port it.
3. Probe the PlayerController and PlayerState callers in the list above (each needs its own callees named; then the static helper).
4. Optional: name the three controller delegates from the code that binds them (the port does not need the names).

### Files touched

- `porting/behavior/MANIFEST.json` (eight entries: four bodies, three member fields, one skipped `OnRep_DeathInfo`)
- `porting/behavior/AFortPlayerStateAthena/OnRep_Kills.inc`, `OnRep_AIKills.inc`, `OnRep_TotalPlayerScore.inc`, `OnRep_TeamKillScore.inc` (new)
- `porting/behavior/check_behavior.py` (`ENGINE_NAMES`: `GetOwner`)
- `porting/behavior/README.md` (status, tenth-pass paragraph, `ENGINE_NAMES` note)
- `porting/behavior/CALLEE_PORT_QUEUE.md` (tenth-pass paragraph, ranking rows 2, 3, 8, 9 and a new row 10)
- `porting/behavior/COUNTS.txt` (rewritten by `check_behavior.py`)
- `porting/decompilation_notes.md` (this section)
- Git-ignored evidence: `porting/generated/callee_probe/tenth_rank2/`

Not touched: `porting/native/`, `gen_native_classes.py`, the git index, `/opt/fortnite/gen-worktree`, `porting/ghidra/`,
`porting/generated/reflection/behavior_splice.json`, `BLOCKING_CALLEES.md`.

## Eleventh pass: the six fallback NetSerialize bodies [observed unless marked]

Scope: the six structs of the handoff's "Known wire gaps" (`SharedRepMovement`, `ReplicatedPhysicsPawnState`, `FortAISharedRepMovement`,
`FortAIAttributeReplicationProxy`, `AthenaPawnReplayData`, `CreateBuildingActorData`). Result: five bodies recovered (drafts below, not yet
spliced), one blocked (`AthenaPawnReplayData`, fallback kept). No generator, `porting/native/`, manifest or `porting/behavior/` change in this
pass. The mechanism is decided below. Repo HEAD when this was written: `d29db8d` (the census commit on top of `9f771cc`).

### Method

- The slot-14 NetSerialize of each struct is a two-instruction thunk: `MOV RCX,qword ptr [RSP+0x28]` then `JMP <body>` (objdump and the probe
  listing agree). The seventh-pass files `serializers/<Struct>_slot14.c` are Ghidra decompiles of the thunk, which show the target body. The
  real bodies are the jump targets, probed here: the decompile and asm are of the body, the argument order is the Windows x64 one (the body
  takes the archive in RDX, the bool* in R9 and the struct pointer as the fifth argument, which the thunk moves into RCX).
- Two `probe_callees.py` runs on the slim project (13.4 s and 14.2 s, no analysis, nothing saved): the six slot-14 addresses, the six bodies, and
  the unnamed callees. Lock file absent and no Ghidra or Java process before the first run. The probe reported project metadata written
  again (`fortnite-slim.rep/idata/~index.*`, `project.prp`, `versioned/~index.*`), the same as the tenth pass; the project still opened. `.gpr`
  is 0 bytes (pre-existing). Outputs: `/tmp/claude-1000/-opt-fortnite/cb6b2fd6-af2b-4311-9c18-8992e33f0be4/scratchpad/nser/probe` and `.../nser/probe2`.
- The asm call sequences were extracted from the probe listings with a script (`.../nser/calls.py`, `.../nser/dest.py`): for every archive call
  the vtable slot, the size argument and the destination operand. Literal constants were read from the dump bytes (file offset = VA -
  `0x140000000`). Engine semantics were checked against UE 4.25 source under `/opt/fortnite/UnrealEngine`.
- Vtable slots: `FArchive` declares `Serialize`, `SerializeBits`, `SerializeInt`, `SerializeIntPacked` in that order
  (`Core/Public/Serialization/Archive.h` 1397, 1399, 1409, 1415), so slots `0x150`, `0x158`, `0x160`, `0x168` are those four. Every call in the
  six bodies matches the argument shape of its slot (for example `SerializeInt(ptr, 0x1ff)` at `0x160`).

### Identities

| Struct | ops vtable | slot 14 thunk | body (jump target) | bytes / insns | binary traits | Result |
|---|---|---|---|---|---|---|
| `FSharedRepMovement` | `0x1464cf188` | `0x142a82170` | `0x141a1b9d0` | 1893 / 532 | HasNetSerializer, HasNetSharedSerialization | recovered, HIGH |
| `FFortAISharedRepMovement` | `0x1462b4fc0` | `0x1427a19f0` | `0x1422df100` | 720 / 204 | HasNetSerializer, HasNetSharedSerialization | recovered, HIGH |
| `FReplicatedPhysicsPawnState` | `0x146492160` | `0x142a25f80` | `0x1423350b0` | 541 / 146 | HasNetSerializer, HasNetSharedSerialization | recovered, HIGH (raw floats) |
| `FFortAIAttributeReplicationProxy` | `0x1462b5220` | `0x1427a19e0` | `0x1422df0c0` | 60 / 18 | HasNetSerializer | recovered, HIGH |
| `FAthenaPawnReplayData` | `0x1464cf2b8` | `0x142a82160` | `0x141a1afd0` | 2557 / 603 | HasNetSerializer | blocked (see below) |
| `FCreateBuildingActorData` | `0x1464a87e8` | `0x142a3dc40` | `0x1423953d0` | 604 / 159 | HasNetSerializer | recovered, HIGH layout, MEDIUM on the saving yaw wrap |

`HasNetSharedSerialization` is ICppStructOps slot 13, a constant trait function (`Class.h` 1026), not a second body. UE 4.25 documents the
trait as "a NetSerialize that does not require the package map" (`Class.h` 684). None of the three traited bodies touches an object
reference, which is consistent with the trait; the blocked struct has no shared trait.

### Helpers identified (asm)

| Address | Identity | Evidence |
|---|---|---|
| `0x141a22ec0` | `FRepMovement::SerializeQuantizedVector` | Switch on the level: 0 and other values `SerializePackedVector<1,24>` (`0x141a080f0`), 1 `<10,27>` (`0x141a083c0`, `*10.0`), 2 `<100,30>` (`0x141a086b0`, `*100.0`); non-finite on save writes a zero vector. Matches `FRepMovement` in `Engine/Classes/Engine/EngineTypes.h` (about line 3115). |
| `0x142c5e060` / `0x142c5e260` | `FRotator::SerializeCompressed` / `SerializeCompressedShort` | Per component: `SerializeBits(1)` (nonzero flag), then 1 byte (`0x060`) or 2 bytes (`0x260`). Matches `UnrealMath.cpp` 79. |
| `0x14072de10` | archive `Serialize(ptr, 4)` (raw 4-byte value) | Buffer fast path, else vtable `0x150` with length 4. |
| `0x1406dfb40` | archive `FVector` raw (3 floats) | Calls `0x14072de10` at +0, +4, +8 of the vector. |
| `0x142ccc610`, `0x142ccee40`, `0x142ccc920` | temporary wrapper around the archive (allocate, pointer, free) | No wire effect. |
| `0x142ccd220` | in-place byte reversal of `n` bytes | Runs only when the archive state bit `0x20` at `+0x29` is set, which is `ArForceByteSwapping` (bit 13 of the state bitfield, `Archive.h` 641). Clear on the network path. |
| `0x1423642d0` | sync-key writer | `MULSS 13.0`, `ADDSS` itself, `ADDSS 0.5`, `CVTSS2SI` (nearest-even), `SAR 1`, `+0x800000`, clamp to 24 bits, `SerializeInt(v, 0x1000000)`. |
| `0x142bc0d80` | remainder helper for the yaw | `(yaw, 360.0f)`: `Yaw - trunc(Yaw/360)*360` with the guard, clamped to `[-360, 360]`. |
| `0x1409de870`, `0x141a221a0` | encrypt / decrypt used by the replay body | Not identified; see the blocked section. |

Constants checked in the dump: `180.0`, `360.0`, `90.0`, `-90.0`, `13.0`, `0.5`, `0.07692307978868484` (the `1/13` float). Field
offsets come from the generated headers (`porting/native/Source/FortniteGame/Generated/`) and agree with the asm destinations.

### Recovered bodies

Fields are listed in wire order. Confidence: HIGH means the asm call and destination were seen and the width is fixed by the slot argument;
MEDIUM means the value of a name or a rule was inferred and is labelled.

#### FSharedRepMovement (`0x141a1b9d0`) [HIGH]

| Order | Offset | Field (header) | Wire | Quantization / note | Confidence |
|---|---|---|---|---|---|
| 1 | `+0x30` bits 0-1 | `RepMovement` flags (bSimulatedPhysicSleep bit 0, bRepPhysics bit 1) | 2 bits | `SerializeBits(2)` | HIGH |
| 2 | `+0x18` | `RepMovement.Location` | packed vector | `SerializePackedVector` by `+0x31`: 0 `<1,24>`, 1 `<10,27>`, 2 `<100,30>` | HIGH |
| 3 | `+0x24` | `RepMovement.Rotation` | rotator | by `+0x33`: 0 byte components, 1 short components, other none. Each component is a 1-bit flag then 8 or 16 bits | HIGH |
| 4 | `+0x00` | `RepMovement.LinearVelocity` | packed vector | by `+0x32` | HIGH |
| 5 | `+0x0c` | `RepMovement.AngularVelocity` | packed vector | by `+0x32`, only when flag bit 1 is set | HIGH |
| 6 | `+0x47` | `RepMovementMode` | 8 bits | `Serialize(1)` | HIGH |
| 7 | `+0x48` | `JumpFlashCountPacked` | 8 bits | | HIGH |
| 8 | `+0x49` | `LandingFlashCountPacked` | 8 bits | | HIGH |
| 9 | `+0x4a` | `CurrentMovementStyle` (`EFortMovementStyle`) | 8 bits | | HIGH |
| 10-19 | `+0x4b`..`+0x54` | ten bools | 32 bits each | `SerializeBool` (UBOOL). Order is the ascending offset; the names follow the header declaration order, which is the layout order (not read from a setter) | HIGH (width), MEDIUM (name to byte) |
| 20 | `+0x34` | `RepTimeStamp` | 1 bit, then 32 bits when nonzero | otherwise `+0.0` | HIGH |
| 21 | `+0x40` | `RemoteViewData32` | 32 bits | always present | HIGH |
| 22 | `+0x44` | `AccelerationPack` (uint16) | 1 bit, then 16 bits when nonzero | | HIGH |
| 23 | `+0x46` | `AccelerationZPack` (int8) | 1 bit, then 8 bits when nonzero | | HIGH |
| 24 | `+0x38`, `+0x3c` | `TurretYaw`, `TurretPitch` | 1 bit (either nonzero), then two 32-bit floats | yaw first; both `+0.0` when clear | HIGH |

The FRepMovement block is the engine's `FRepMovement::NetSerialize` (flag bits, Location, rotator, linear velocity, angular velocity when
`bRepPhysics`), so the draft calls it. Its `bOutSuccess` accumulation is the decompile's `*param_4 &= ...`.

#### FFortAISharedRepMovement (`0x1422df100`) [HIGH]

| Order | Offset | Field (header) | Wire | Note | Confidence |
|---|---|---|---|---|---|
| 1 | `+0x30`, `+0x18`, `+0x24`, `+0x00`, `+0x0c` | `RepMovement` | as above | same FRepMovement block | HIGH |
| 2 | `+0x38` | `RepMovementMode` (uint8) | 8 bits | | HIGH |
| 3 | `+0x39` | `RepCurrentFortAILODLevel` (`EFortAILODLevel : uint8`) | 8 bits | | HIGH |
| 4 | `+0x34` | `RepTimeStamp` | 1 bit, then 32 bits when nonzero | | HIGH |
| 5 | `+0x3c` | `RepSharedAnimInfo.AnimState` | 1 bit (`MontageIndex != -1`), then 8 bits, then 32 bits, then 8 bits | wire order is `+0x3c`, `+0x40`, `+0x3d` (checked by destination) | HIGH |
| 6 | `+0x40` | `RepSharedAnimInfo.MontageIndex` (int32) | 32 bits | | HIGH |
| 7 | `+0x3d` | `RepSharedAnimInfo.MontageSectionToPlay` | 8 bits | | HIGH |
| 8 | load only | default when the bit is clear | | `AnimState = 11` (`Anim_MAX`), `MontageSectionToPlay = 0`, `MontageIndex = -1` (asm stores `0xffffffff0000000b` over `+0x3c..+0x43`) | HIGH |

#### FReplicatedPhysicsPawnState (`0x1423350b0`) [HIGH]

Raw layout, no flags and no quantization: `Translation` 3 floats (`+0x00`), `Rotation` as `FQuat` 4 floats (`+0x10`, `+0x14`,
`+0x18`, `+0x1c`), `LinearVelocity` 3 floats (`+0x20`), `AngularVelocity` 3 floats (`+0x2c`), `SyncKey` uint16 (`+0x38`). Total 54 bytes.
The member types in the generated header (`FVector_NetQuantize100`, `FVector_NetQuantize10`) are not used by the wire: the body writes
the raw floats. This is the main finding for this struct. Confidence HIGH for the bytes.

#### FFortAIAttributeReplicationProxy (`0x1422df0c0`) [HIGH]

`SerializeIntPacked(Health)` (`+0x00`, uint32) then `SerializeIntPacked(MaxHealth)` (`+0x04`, uint32). The body never writes
`bOutSuccess` (the asm does not use R9) and returns true. Slot `0x168` is `SerializeIntPacked` (MEDIUM on the name, HIGH on the call).

#### FCreateBuildingActorData (`0x1423953d0`) [HIGH layout, MEDIUM yaw wrap]

| Order | Offset | Field | Wire | Note | Confidence |
|---|---|---|---|---|---|
| 1 | `+0x00` | `BuildingClassHandle` (uint32) | `SerializeInt(v, 0x1ff)`, 9 bits | | HIGH |
| 2 | `+0x1c` | `bMirrored` | 32 bits (UBOOL) | | HIGH |
| 3 | `+0x04..+0x0f` | `BuildLoc` | 3 raw floats | member type `FVector_NetQuantize10` not used | HIGH |
| 4 | `+0x34` | `BuildingClassData.UpgradeLevel` | 8 bits | `FBuildingClassData` at `+0x28`, `UpgradeLevel` at its `+0x0c` | HIGH |
| 5 | `+0x20` | `SyncKey` (float) | `SerializeInt(v, 0x1000000)`, 24 bits | `v = ((round_even(26x + 0.5)) >> 1) + 0x800000`, clamped; load `(v - 0x800000) * 0.07692308` | HIGH (encoding), MEDIUM (rounding mode of the rounding instruction, default nearest-even) |
| 6 | `+0x14` (`BuildRot.Yaw`) | yaw code | 8 bits (values 0..3) | save: bins (90,180]=0, (0,90]=1, (-90,0]=2, [-180,-90]=3 after the wrap; load: 0=180, 1=90, 2=0, else -90 | HIGH (table), MEDIUM (wrap helper re-expressed) |

`BuildRot.Pitch` (`+0x10`) and `BuildRot.Roll` (`+0x18`) are never written. The order inside the body is 1, 2, 3, 4, then 5 and 6 (sync
first, then the yaw code), on both paths.

#### FAthenaPawnReplayData (`0x141a1afd0`) [BLOCKED]

Not recovered. Why:
- The wire depends on runtime state, not only on the archive direction: the body branches on `IsSaving` / `IsLoading` and on the owner chain
  (`FUN_14479fb90`, `FUN_14447f220`, the `AFortPlayerControllerAthena` static class check), then on a per-connection key state
  (`*(int*)(lVar4+0x30a8)`) and a global flag (`_DAT_1478c2940`). The log strings name Case 1 (server encrypts and sends), Case 2 (receives
  ciphertext), Case 3 (saves ciphertext), Case 4 (decrypts with a key) and the no-key and no-valid-data paths, each with its own framing.
- The encrypted paths call `FUN_1409de870` (encrypt) and `FUN_141a221a0` (decrypt), neither identified. They handle replay key material,
  which this pass does not touch (no keys, and anticheat and entitlement stay out of scope).
- Partial, not enough for a body: the plaintext framing of the no-key branches is a 32-bit count followed by the CipherText bytes
  (`Serialize(4)`, then `Serialize(count)`), and `World` (`UWorld*`, `+0x18`) is never serialized: the asm only reads it to choose a branch (MEDIUM).
- Keep the fallback marked. The next step needs the crypto identity and an explicit decision on key handling.

### Draft bodies (verbatim copies of the scratchpad `.inc` files)

Each body is the text between the braces of `bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)`. Not compiled.

#### FSharedRepMovement (`0x141a1b9d0`)

```cpp
// NetSerialize of FSharedRepMovement. Draft body for the struct_net kind (not spliced yet).
// Original 0x141a1b9d0, reached from the ops thunk 0x142a82170 (MOV RCX,[RSP+0x28]; JMP). Source: Ghidra slot-14 decompile and objdump of the dump.
// Verified from the asm: the archive call sequence and each destination, the quantizer arguments, and the rotator choice.
// CONFIDENCE: HIGH
bOutSuccess = true;

// FRepMovement block: engine FRepMovement::NetSerialize (2 flag bits, Location +0x18 by +0x31, rotator +0x24 by +0x33,
// LinearVelocity +0x00 by +0x32, AngularVelocity +0x0c by +0x32 only when bRepPhysics).
RepMovement.NetSerialize(Ar, Map, bOutSuccess);

Ar.Serialize(&RepMovementMode, 1);            // +0x47 uint8, 8 bits
Ar.Serialize(&JumpFlashCountPacked, 1);       // +0x48 uint8, 8 bits
Ar.Serialize(&LandingFlashCountPacked, 1);    // +0x49 uint8, 8 bits
Ar.Serialize(&CurrentMovementStyle, 1);       // +0x4a enum byte, 8 bits

// Ten bools in ascending offset order, each as a 32-bit UBOOL (FArchive::SerializeBool).
Ar.SerializeBool(bProxyIsJumpForceApplied);   // +0x4b
Ar.SerializeBool(bIsCrouched);                // +0x4c
Ar.SerializeBool(bIsSkydiving);               // +0x4d
Ar.SerializeBool(bIsParachuteOpen);           // +0x4e
Ar.SerializeBool(bIsSlopeSliding);            // +0x4f
Ar.SerializeBool(bIsProxySimulationTimedOut); // +0x50
Ar.SerializeBool(bIsTargeting);               // +0x51
Ar.SerializeBool(bIsWaterJump);               // +0x52
Ar.SerializeBool(bIsWaterSprintBoost);        // +0x53
Ar.SerializeBool(bIsWaterSprintBoostPending); // +0x54

{
	// RepTimeStamp (+0x34): 1 bit (nonzero), then the float when it is set; otherwise it becomes +0.0.
	uint8 bHas = RepTimeStamp != 0.0f ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&RepTimeStamp, 4);
	}
	else
	{
		RepTimeStamp = 0.0f;
	}
}

Ar.Serialize(&RemoteViewData32, 4);          // +0x40 uint32, always present, 32 bits

{
	// AccelerationPack (+0x44): 1 bit, then uint16 when set.
	uint8 bHas = AccelerationPack != 0 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&AccelerationPack, 2);
	}
	else
	{
		AccelerationPack = 0;
	}
}

{
	// AccelerationZPack (+0x46, int8): 1 bit, then 8 bits when set.
	uint8 bHas = AccelerationZPack != 0 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&AccelerationZPack, 1);
	}
	else
	{
		AccelerationZPack = 0;
	}
}

{
	// TurretYaw (+0x38) and TurretPitch (+0x3c): 1 bit (either nonzero), then both floats when set.
	uint8 bHas = (TurretYaw == 0.0f && TurretPitch == 0.0f) ? 0 : 1;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&TurretYaw, 4);
		Ar.Serialize(&TurretPitch, 4);
	}
	else
	{
		TurretYaw = 0.0f;
		TurretPitch = 0.0f;
	}
}

return true;
```

#### FFortAISharedRepMovement (`0x1422df100`)

```cpp
// NetSerialize of FFortAISharedRepMovement. Draft body for the struct_net kind (not spliced yet).
// Original 0x1422df100, reached from the ops thunk 0x1427a19f0 (MOV RCX,[RSP+0x28]; JMP).
// Verified from the asm: archive call sequence and destinations (+0x3c, +0x40, +0x3d order), default store on load.
// CONFIDENCE: HIGH
bOutSuccess = true;

// FRepMovement block: same pattern as FSharedRepMovement (2 flag bits, Location, rotator, linear and angular velocity).
RepMovement.NetSerialize(Ar, Map, bOutSuccess);

Ar.Serialize(&RepMovementMode, 1);            // +0x38 uint8
Ar.Serialize(&RepCurrentFortAILODLevel, 1);   // +0x39 EFortAILODLevel (uint8)

{
	// RepTimeStamp (+0x34): 1 bit (nonzero), then the float when set; otherwise +0.0.
	uint8 bHas = RepTimeStamp != 0.0f ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&RepTimeStamp, 4);
	}
	else
	{
		RepTimeStamp = 0.0f;
	}
}

{
	// RepSharedAnimInfo (8 bytes at +0x3c): 1 bit (MontageIndex != -1). When set the wire order is
	// AnimState (+0x3c, 8 bits), MontageIndex (+0x40, 32 bits), MontageSectionToPlay (+0x3d, 8 bits).
	// When the bit is clear and the archive is loading, the body stores AnimState = Anim_MAX (0x0b),
	// MontageSectionToPlay = 0 and MontageIndex = -1 (the asm stores 0xffffffff0000000b over +0x3c..+0x43).
	uint8 bHas = RepSharedAnimInfo.MontageIndex != -1 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&RepSharedAnimInfo.AnimState, 1);
		Ar.Serialize(&RepSharedAnimInfo.MontageIndex, 4);
		Ar.Serialize(&RepSharedAnimInfo.MontageSectionToPlay, 1);
	}
	else if (Ar.IsLoading())
	{
		RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_MAX;
		RepSharedAnimInfo.MontageSectionToPlay = 0;
		RepSharedAnimInfo.MontageIndex = -1;
	}
}

return true;
```

#### FReplicatedPhysicsPawnState (`0x1423350b0`)

```cpp
// NetSerialize of FReplicatedPhysicsPawnState. Draft body for the struct_net kind (not spliced yet).
// Original 0x1423350b0, reached from the ops thunk 0x142a25f80. Raw layout: no flags, no quantization.
// Verified from the asm: destinations +0x00..+0x0b (helper, 3 floats), +0x10..+0x1c (4 dwords), +0x20 and +0x2c
// (helper, 3 floats each) and +0x38 (2 bytes). The member types (FVector_NetQuantize100/10) are not used for the wire.
// CONFIDENCE: HIGH
Ar.Serialize(&Translation.X, 4);
Ar.Serialize(&Translation.Y, 4);
Ar.Serialize(&Translation.Z, 4);
Ar.Serialize(&Rotation.X, 4);
Ar.Serialize(&Rotation.Y, 4);
Ar.Serialize(&Rotation.Z, 4);
Ar.Serialize(&Rotation.W, 4);
Ar.Serialize(&LinearVelocity.X, 4);
Ar.Serialize(&LinearVelocity.Y, 4);
Ar.Serialize(&LinearVelocity.Z, 4);
Ar.Serialize(&AngularVelocity.X, 4);
Ar.Serialize(&AngularVelocity.Y, 4);
Ar.Serialize(&AngularVelocity.Z, 4);
Ar.Serialize(&SyncKey, 2);
bOutSuccess = true;
return true;
```

#### FFortAIAttributeReplicationProxy (`0x1422df0c0`)

```cpp
// NetSerialize of FFortAIAttributeReplicationProxy. Draft body for the struct_net kind (not spliced yet).
// Original 0x1422df0c0, reached from the ops thunk 0x1427a19e0. Two SerializeIntPacked calls (FArchive slot +0x168).
// The body never writes bOutSuccess (the R9 argument is not used by the asm); it returns true.
// CONFIDENCE: HIGH (call order and width); MEDIUM on the slot name (vtable slot 0x168 is SerializeIntPacked by FArchive order).
Ar.SerializeIntPacked(Health);
Ar.SerializeIntPacked(MaxHealth);
return true;
```

#### FCreateBuildingActorData (`0x1423953d0`)

```cpp
// NetSerialize of FCreateBuildingActorData. Draft body for the struct_net kind (not spliced yet).
// Original 0x1423953d0, reached from the ops thunk 0x142a3dc40. Layout from the asm: SerializeInt(handle, 511),
// bool as UBOOL, BuildLoc as raw floats, BuildingClassData.UpgradeLevel (+0x34), then saving or loading SyncKey
// (SerializeInt over 2^24 with the 13x quantizer) and a 2-bit yaw code in one byte. BuildRot Pitch and Roll are never serialized.
// CONFIDENCE: HIGH for the layout and constants (180, 90, -90, 360, 13, 2^23); MEDIUM for the wrap of the saving yaw
// (helper 0x142bc0d80 re-expressed here as clamp(Yaw - trunc(Yaw/360)*360, -360, 360)).
Ar.SerializeInt(BuildingClassHandle, 0x1ff);
Ar.SerializeBool(bMirrored);
Ar.Serialize(&BuildLoc.X, 4);
Ar.Serialize(&BuildLoc.Y, 4);
Ar.Serialize(&BuildLoc.Z, 4);
Ar.Serialize(&BuildingClassData.UpgradeLevel, 1);

if (Ar.IsSaving())
{
	// SyncKey (0x1423642d0): ((round_half_even(2 * (13 * x) + 0.5)) >> 1) + 0x800000, clamped to [0, 0xffffff].
	const float Scaled = SyncKey * 13.0f;
	const float Doubled = Scaled + Scaled + 0.5f;
	int32 Biased = (static_cast<int32>(std::nearbyint(Doubled)) >> 1) + 0x800000;
	if (static_cast<uint32>(Biased) > 0xffffff)
	{
		Biased = (Biased > 0) ? 0xffffff : 0;
	}
	uint32 SyncCode = static_cast<uint32>(Biased);
	Ar.SerializeInt(SyncCode, 0x1000000);

	// Yaw code (0x142bc0d80 with 360.0f, then the bins of 0x1423953d0): 0 = (90, 180], 1 = (0, 90], 2 = (-90, 0], 3 = [-180, -90].
	float Quotient = BuildRot.Yaw / 360.0f;
	if (FMath::Abs(Quotient) < 8388608.0f)
	{
		Quotient = static_cast<float>(static_cast<int32>(Quotient));
	}
	float Mid = Quotient * 360.0f;
	if (FMath::Abs(BuildRot.Yaw) < FMath::Abs(Mid))
	{
		Mid = BuildRot.Yaw;
	}
	float Wrapped = BuildRot.Yaw - Mid;
	if (Wrapped < -360.0f)
	{
		Wrapped = -360.0f;
	}
	else if (Wrapped > 360.0f)
	{
		Wrapped = 360.0f;
	}
	if (Wrapped < 0.0f)
	{
		Wrapped = Wrapped + 360.0f;
	}
	if (Wrapped > 180.0f)
	{
		Wrapped -= 360.0f;
	}
	uint8 YawCode = 0;
	if (Wrapped <= 90.0f)
	{
		if (Wrapped <= 0.0f)
		{
			YawCode = (Wrapped <= -90.0f) ? 3 : 2;
		}
		else
		{
			YawCode = 1;
		}
	}
	Ar.Serialize(&YawCode, 1);
}
else
{
	uint32 SyncCode = 0;
	Ar.SerializeInt(SyncCode, 0x1000000);
	SyncKey = static_cast<float>(static_cast<int32>(SyncCode) - 0x800000) * 0.07692308f;

	uint8 YawCode = 0;
	Ar.Serialize(&YawCode, 1);
	BuildRot.Yaw = (YawCode == 0) ? 180.0f : ((YawCode == 1) ? 90.0f : ((YawCode == 2) ? 0.0f : -90.0f));
}

bOutSuccess = true;
return true;
```

### Mechanism decision

The existing kinds cannot express these bodies. `getter`, `exec`, `onrep`, `impl` and `validate` are UFUNCTIONs of a UClass, `helper` and
`static_helper` are members of a class or static functions of a class .cpp, and `member_field` has no body. The generator emits a USTRUCT
through `render_struct_header` (`gen_native_classes.py` 3334), which writes only the declaration plus, for the delta case,
`TStructOpsTypeTraits`. No path adds a member function to a struct.

Decision: add a struct-function kind `struct_net`:
- `class` = the struct's C++ name (`FSharedRepMovement`), `function` = `NetSerialize`, `kind` = `struct_net`, `status` = `ported`,
  `signature` = `bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)`, `signature_status` = `proposed`,
  `inc_path` = `FSharedRepMovement/NetSerialize.inc`, `confidence` as above, `address` = the body (`0x141a1b9d0`) and the thunk in `notes`,
  `traits` = list of the native traits to keep (`WithNetSharedSerialization` only for `FSharedRepMovement`, `FFortAISharedRepMovement`,
  `FReplicatedPhysicsPawnState`), `includes` = `EFortSharedAnimationState.h` for `FFortAISharedRepMovement`.
- The five drafts: `.../nser/inc/<Struct>/NetSerialize.inc` (scratchpad, not in the repo; copy them to `porting/behavior/<Struct>/NetSerialize.inc`
  with the manifest entries when the kind lands). `FAthenaPawnReplayData` gets a `skipped` entry with the reason above.

Exact generator change (not made here; the build owner or a worktree does it):
1. `gen_native_classes.py`, `net_serializer` (about line 3190): before the fallback return, look up the `struct_net` manifest entry for
   `info.cpp_name` (only when the behaviour splice is on). Return `{"kind": "native", "body": <text of the .inc>, "traits": [...], "vtable": vtable}`.
2. `render_struct_header` (about line 3334): for `kind == "native"`, drop the TODO comment, add `class UPackageMap;` to the forward declarations,
   write `bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)` with the body indented one tab before `};`, and after
   the struct emit `template<> struct TStructOpsTypeTraits<X> : public TStructOpsTypeTraitsBase2<X> { enum { WithNetSerializer = true, WithNetSharedSerialization = <traits> }; };`
   (same form as the delta block). Add the `includes` headers.
3. `verify_spliced`: check the declaration once inside the struct, its body equal to the `.inc`, and the trait block.
4. `check_behavior.py`: accept `struct_net` (class is a generated struct, signature match, the token check must know the struct's member
   names as well as the FRepMovement and enum names of the body). `behavior/README.md`: document the kind.
5. `class_checksum.py`: confirm that a struct with a spliced native NetSerialize leaves the `struct_custom_netserialize` gap (not checked here).

Checks: none of the bodies has been compiled (the build owner compiles). `std::nearbyint`, `FMath::Abs`, `FRepMovement::NetSerialize`,
`FArchive::SerializeBool`, `SerializeIntPacked`, `SerializeInt`, `SerializeBits` and `Serialize` are the names the bodies use; all exist in UE 4.25.

### Corrections to earlier passes

- Seventh pass: the `*_slot14.c` decompiles are of the thunk targets (the body is at the jump target), not of the thunk itself.
- Seventh pass: `FFortAIAttributeReplicationProxy` "field type not known" is resolved: two uint32 fields, `SerializeIntPacked`.
- Seventh pass: `FFortAISharedRepMovement`, `SharedRepMovement`, `ReplicatedPhysicsPawnState` and `FCreateBuildingActorData` were "low";
  they are now recovered (see the tables). `FAthenaPawnReplayData` stays blocked.
- Generated headers: the member types `FVector_NetQuantize100/10` of `ReplicatedPhysicsPawnState` and `FCreateBuildingActorData` do not
  describe the wire. The native bodies write raw floats.
- Generated headers: the fallback comment says "The custom net serializer of this struct is declared below", which is false for the fallback.

### Checks run

- Call sequences and destinations of the five bodies (script, all archive calls listed with vtable slot, size and destination).
- Helper arguments in the asm before each call (for example the quantization level bytes `+0x31`, `+0x32`, `+0x33`).
- Constants from the dump (see above) and the UE sources listed above.
- objdump of the `SharedRepMovement` thunk and the first body instructions, which match the Ghidra listing.
- Not run: any UE build, `promote_native_classes.sh`, `check_behavior.py` on the new kind (the kind does not exist).

### Next blockers (ordered)

1. Implement the `struct_net` kind in the generator and checker (in a worktree, `gen/netserialize`, per the hand-off rules), then splice the five
   drafts and compile them (build owner).
2. Identify `FUN_1409de870` and `FUN_141a221a0` and the mode selection of `AthenaPawnReplayData` (needs an explicit key-handling decision;
   out of scope for this pass).
3. Optional: confirm the ten bool names to byte offsets with a runtime setter in the dump (the bools are not in `bool_bits.json`, which is per class).

### Files touched

- `porting/decompilation_notes.md` (this section)
- Scratchpad only: `.../scratchpad/nser/probe`, `.../nser/probe2` (Ghidra outputs), `.../nser/calls.py`, `.../nser/dest.py`, `.../nser/inc/<Struct>/NetSerialize.inc` (five drafts)
- Not touched: `porting/native/`, `porting/behavior/`, `gen_native_classes.py`, `porting/ghidra/`, git index, worktrees. Ghidra project metadata
  (`/opt/fortnite/ghidra-projects/fortnite-slim.rep/...`) was rewritten by the two probes, as in the tenth pass.
