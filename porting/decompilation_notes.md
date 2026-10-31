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
