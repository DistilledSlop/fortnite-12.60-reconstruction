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
