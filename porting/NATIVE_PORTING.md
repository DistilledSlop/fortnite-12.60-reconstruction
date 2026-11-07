# Native function body porting: state, method, remaining work

Single source of truth for the "Native function bodies ported" block. Supersedes the old `HANDOFF.md` and `NATIVE_RECOVERY_HANDOFF.md` (history in Git). Figures below were recomputed from the repo on 2026-11-07; recompute with the snippet in section 8 before trusting them later.

## 1. Scope and numbers

| Quantity | Value | Source |
|---|---|---|
| Reflected native functions with an exec address | 20,574 | `generated/reflection/native_funcs.json` |
| Covered by stock UE 4.25 declarations (not ours to port) | 5,931 | `progress/engine_provided_natives.json` `verified` (declaration + param-count match only; Epic fork body differences untested) |
| **In-scope denominator** | **14,643** | dashboard `native_functions_with_exec` |
| Manifest entries `status=ported`, kind != `member_field` | 3,886 | `behavior/MANIFEST.json` |
| ...of which match a reflected exec function | 3,236 | helpers, `_Implementation`, validate etc. make up the other 650 |
| Dashboard "done" | 3,895 (26.6%) | counts all ported entries plus 5 handwritten Kismet bodies, so it slightly overstates reflected coverage |
| **Strictly unported reflected functions** | **11,407** | in-scope minus matching ported keys |

11,407 is the strict reflected-name count. **228 of those are already covered** by a ported `<fn>_Implementation` entry and 71 more have only `<fn>_Validate` ported (ported RPC and native-event bodies are keyed under those names, so they never match the reflected name; `mark_covered.py` flags them in `final2.json`). Working remaining count (after the two stub batches, 2026-11-07): **11,032 unported by reflected name (incl. 61 `_Validate`-only), of which 252 are interface functions that have no body at the interface level (section 9)**. The dashboard's implied figure is lower because it also counts helpers. The same name mismatch means the dashboard does not credit those 228.

Largest remaining classes: UFortKismetLibrary 298, AFortPlayerController 260, AFortPlayerControllerAthena 186, UFortGlobalUIContext 164, AFortPlayerPawn 163, UFortSettingsContext 161, AFortMinigame 126, AFortCreativeMoveTool 115, UFortMcpContext 97, UFortInventoryContext 95.

Manifest breakdown (ported): exec 3,055, member_field 383, helper 292, getter 140, native_event_impl 123, validate 116, impl 105, onrep 39, static_helper 11. Skipped: 99 impl, 20 exec, 17 callee.

## 2. What has been done

- Committed through `a037f865` (pushed). Last successful generation: `generated/gameplay-bulk-integration-455/reports/behavior_splice.json`, 3,684 accepted (3,341 bodies + 343 fields), zero rejected.
- **Last build/test-green checkpoint: 3,058 bodies.** Everything since is source recovery only, with no build or runtime check. The standing user rule was no builds or tests until 4,058 bodies, then one broad check. Confirm that rule is still wanted before relying on it.
- Proven bulk patterns: one-string callbacks sharing wrapper `0x14286b840` (34 bodies); empty-implementation natives (whole-RET body plus exact vtable proof); constant/zero field defaults from complete constructors; native virtual wrappers resolved by vtable slot (section 5); `native_event_impl` and RPC `impl` kinds.
- Tooling added (all read-only analysis, outputs git-ignored under `generated/`): `group_direct_thunks.py` and `generated/direct-thunk-groups/`; `build_thunk_table.py`, `resolve_class_vtables.py`, `resolve_virtual_targets.py`, `finalize_thunk_table.py`, `build_work_packets.py`, `resolve_wrappers.py`, `mark_covered.py`, `check_thunk_calls.py`, `extract_ground_truth.py` (section 3). Two earlier byte-scan scripts (`resolve_virtual_thunks.py`, `group_direct_thunks.py`) were removed as superseded.

## 3. Where every remaining body is (final table, 2026-11-07)

`exec_addr` is normally the UHT-generated exec thunk. We never port thunks (our UHT regenerates them); we read them only to find the function they call. Sometimes the thunk contains the body (inlined getter) or `exec_addr` is not a thunk at all (e.g. `AAIHotSpot::GetSlotCount`).

Pipeline (all read-only, run from the repo root, outputs git-ignored under `generated/`): `build_thunk_table.py` (classify each thunk from its Ghidra decompile) -> `resolve_class_vtables.py` (class vtables from constructors) -> `resolve_virtual_targets.py` (vtable slot -> target) -> `finalize_thunk_table.py` -> `generated/thunk-table/final.json`, `final_summary.json`. All 11,407 thunks have a decompile and every address in the table is captured.

| Group | Functions | Meaning |
|---|---|---|
| A inline | 804 | thunk decompile is the body (may call other objects virtually) |
| B direct | 8,121 | body is the callee(s) listed; the thunk's frame helpers are filtered out by frequency (callees in 200+ thunks) |
| C virtual | 2,465 | body is the vtable target(s) of a call on `this`; targets come from the class's constructor-derived vtable |
| U | 17 | no single body: 12 interface dispatch (`UFortVehicleInterface` etc., implemented by implementing classes), 4 pure-virtual slots on a base-only vtable (`UFortPlayerSurveyQuestion`, `UFortCreativeItemListPanelData`; bodies in subclasses), 1 embedded-subobject call (`UFortSpline::ClearSpline`) |

Earlier estimates from raw byte scans (5,369, 4,899 and 907 "virtual") and the class-index vtable candidates (only 408 of 6,957 verified) were wrong or unreliable; do not use them.

**Accuracy (checked against `extract_ground_truth.py`, 1,334 pairs from ported functions, mostly the manifest `native_body_address` field; some `.inc`-comment pairs are medium confidence and a few are wrong):**
- Group B: where the true body is a direct callee, the single non-helper callee was it 667 times, among 2-3 candidates 14 times, and missed 49 (about 93% precise on a sample biased toward easy functions).
- Group C: 345 of 383 testable pairs (90%) hit the true body. Misses include thunks with several virtual call sites (the first is not always the body) and noisy ground truth.
- Class vtables reproduce the two known cases (`AFortAthenaVehicle` primary `0x145debe20`, secondary `+0x3b0 = 0x145decf00`; `ABuildingTrap` `0x145f46db0`). 916 of 924 classes in group C resolved; 8 have no vptr store found and rely on the old candidate.
- "Captured" means a Ghidra capture exists, not that a body is ported or that the chosen callee is right. Group B `multi` rows (935) list several callees: pick the one that receives the unpacked parameters.

### Refinements after the first table (2026-11-07)

- `resolve_wrappers.py` follows pure tail-jump wrappers (`[ADD|SUB RCX,N;] JMP target`) to the real body: 169 functions changed, 9,953 -> 9,886 distinct bodies. Output `final2.json` (`real_bodies`, `resolved[].chain/this_delta/forward_kind`; 52 are `MOV RAX,[RCX]; JMP [RAX+N]` virtual forwards). Other tiny forwarders that preset args or load members are real small bodies and are kept.
- `check_thunk_calls.py` compares each call's argument count with the reflected signature. **It is weak: among 681 true ground-truth bodies only 54% match the expected count exactly (34% one lower, 9% one higher), because of hidden return pointers, dropped args and by-ref structs.** Do not treat a mismatch as an error. It is only a tie-break for multi-callee rows.
- Distinct real bodies: about 9.9k for 11.4k functions, so sharing saves only about 13%. By body size (group A sizes inflated by frame code): about 2,900 bodies of 48 bytes or less (about 3,600 functions), 6,241 functions have bodies over 16 instructions. A template generator for leaf bodies could cover at most about 950 functions (237 straight-line bodies serving 619 functions, mostly a few shared stubs, plus 266 branching leaves serving 326), so it is a modest lever, not a bulk solution.
- Stub family (`generated/thunk-table/stub_family.json`): 273 uncovered functions whose real bodies are all pure return stubs (`RET` at `0x1406801c0` serves about 250; `mov al,1; ret` at `0x1406801d0`; `xor eax,eax; ret` at `0x1406801e0`): 53 plain void empty, 51 native-event void empty, 43 net-rpc (`empty+rettrue` = `_Implementation` plus `_Validate`), 22 plain "returns a value but empty body" (suspicious: do not port blindly), the rest return true/false/0. This is the first safe batch. Entries follow the existing conventions: kind `impl` for RPC `_Implementation`, `validate` for `_Validate` (shared `_shared/validate_return_true.inc`), `native_event_impl` with pinned `reflection_flags` for BlueprintNativeEvents, `virtual:true` for ordinary virtuals.

### Work packets and confidence tiers

`build_work_packets.py` writes one Markdown packet per distinct body to `generated/work-packets/<addr>.md` (+ `index.json`; 10,595 bodies, 44 MB, 7 s to rebuild): served functions with reflected signatures, body decompile with callee names substituted (62,487 named, 19,717 left as `FUN_`), `this`-offset field annotations (11,296 resolved, 20,413 not; off for bodies shared across classes or static functions), thunk excerpt, strings and hints. Spot-check findings:
- The reflected signature line decodes some parameter kinds badly (e.g. `HandleInventoryUpdated(ProfileRevision: Int64, out ItemChangeFlags: Set, ItemChangeFlags: Str)`); take parameter types from the generated headers or reflection, not the packet line.
- A packet can list a frame helper as a body. `0x1406800e0` (117 bytes, 35 classes, 44 "served" functions) is in the packets of functions whose thunk was ambiguous; it is a shared helper, not their body.

**Confidence tiers for group B (8,121 rows):** 6,806 have a single candidate callee (6,322 direct plus 484 reclassified from virtual; best-supported, about 93% on ground truth), 796 have 2-3 candidates and **519 have 4+ candidates (low confidence; the thunk may be the body, with the callees being helpers)**. Treat the 1,315 multi-candidate rows as needing per-function review before transcription. Group C 2,465 virtual rows and group A 804 inline rows are separate tiers.

## 4. Capture tooling

`porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input LIST.json --out DIR --timeout 120 --mem 24G` captures a list of addresses (about 0.05-0.16 s each). Output `DIR/<addr>.c/.asm/.json`. Ghidra runs are serialized; look up `generated/native*/0xADDR.c` first. This session added `native-cap-direct`, `native-cap-direct2`, `native-cap-virtual*`, `native-cap-gpsc/ictor/ctor` (about 5,500 captures, about 70 MB). Keep 12 GB free on `/`.

## 5. Class vtables and the virtual recipe

`resolve_class_vtables.py` chain: `native_class_index.gpsc_targets[0]` (GetPrivateStaticClass) -> stack arg 10 of `GetPrivateStaticClassBody` (address = call RSP + 0x48; the frame size N comes from `SUB RSP,N`, so it is `[R11-(N-0x48)]` or `[RSP+0x48]`) = `InternalConstructor<C>`, often a 4-instruction stub that tail-jumps to the real ctor (follow `JNZ`/`JMP` targets) -> in the ctor, `LEA reg,[vtbl]; MOV [this-alias],reg` at offset 0 is the primary vptr (last store wins), stores at other offsets are secondaries. Only stores through registers aliasing `this` count; stores through other registers are member subobjects (this bug mis-resolved `AAIHotSpot` until fixed).

Manual recipe still applies for edge cases: read the slot qword from `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (base `0x140000000`, offset = VA - base), capture the target, and use manifest `virtual:true` for reflected virtuals. Common virtual targets `0x1406801c0`, `0x1406801d0`, `0x1406801e0` are trivial return stubs. `/tmp/native-empty-rpcs458.json` is unvalidated; do not promote from it.

## 6. Rules and pitfalls (learned, keep)

- Root alone owns `behavior/MANIFEST.json`, the generator, generated Native files, source generation, commits and Ghidra. Workers write separate proposal directories (`behavior/<name>_runtime/`). Ghidra captures are serialized; look up `generated/native*/0xADDR.c` before every capture.
- Never hand-edit generated native files. Generation can exit 0 with rejected snippets: always inspect `behavior_splice.json` and require zero rejected and no removals. Check duplicates by `(class,function)` across kinds; hardcoded implementations may exist outside the manifest.
- Every `.inc` needs a confidence tag. Do not guess offsets, types or defaults. Explicit constructor writes beat inferred zero-init.
- Snippet manifests must not list native headers as explicit includes (explicit includes are engine-index only). Put native type names in typed locals or casts so the generator infers includes and promotes structs and enums.
- Blueprint native events use kind `native_event_impl` with function `_Implementation` and pinned reflection flags; RPCs use kind `impl`. Do not flatten events into ordinary exec.
- Signatures: struct and enum reflection signatures are authoritative; byte enum parameters are often bare `E::Type`, regular enums have no `::Type`. Fix proposal and canonical signature together.
- **Known generator bug:** `gen_native_classes.py` `decode_params` tests `CPF_OutParm` before `CPF_ConstParm|CPF_ReferenceParm`, emitting mutable `T&` for reflected const references. 2,416 reflected functions have an affected parameter; 46 ported manifest keys are an upper bound on affected entries (e.g. `ABuildingActor::AddEditorOnlyInstanceGameplayTags`). Fix the rule order, migrate affected signatures coherently, regenerate, and resolve gate rejections before promoting more entries with const-reference params.
- Preserve the dirty shared tree: asset/CUE4 edits, `behavior/large_bulk150c_runtime/native-evidence.json`, old untracked proposals. Stage specific paths only. Never reset or clean. Dashboard files are rewritten by the post-commit hook; stage `README.md` and `docs/{history.html,index.html,progress.json,progress.svg}` as a separate commit.
- Do not print minified reflection JSON (one line can be megabytes); parse and select fields. Builds exceeding ~180 s background in the Bash tool: tee to a log and wait on the PID.
- Commit as Codex `<codex@openai.com>` with the Claude co-author line; ordinary push, never force.

## 7. Unintegrated proposals and blocked items

Untracked `behavior/` directories and `.inc` files currently exist in the working tree (for example `ABuildingActor/Native_141ba76d0.inc`, `AFortHoagieVehicle/Native_14084*.inc`). They are not canonical until added to the manifest and generated with zero rejections. Review each before promotion.

- **vehicle_shared453_runtime, four ready bodies** for `AFortAthenaVehicle`: `GetDriver`, `ContainsPlayers`, `GetAllPassengers`, `GetPawnAtSeat`, reusing canonical virtual `NativeGetSeatComponent163` and `UFortVehicleSeatComponent` helpers. Original has no null guard on the component. Verify `GetAllPassengers` filtering and order against the capture. Name a typed `UFortVehicleSeatComponent*` local. Follow-on captures not done: `14084b6a0`, `1419177a0`, `141d06fd0`, `141d0a110`, `14084f960`.
- **trap_data454_runtime, conditional:** `GetDamageDelay`, `GetFireDelay` plus virtual helper `NativeHasTrapData`, reading `WeaponStatHandle.GetRow<FFortBaseWeaponStats>` then downcasting to `FFortTrapStats` (not yet generated). Row struct helper `142b23580` is captured but unverified. Remove native headers from snippet includes. Blocked: `GetTrapRating` (virtual +0x2d0 unidentified), `GetMaxDurability` (needs `141b94070` to `141b9b750`).
- **building_max452_runtime, evidence only:** `GetHealth`/`GetHealthPercent` need `GetMaxHealth`, which depends on the Fortnite custom attribute initializer (map keyed by attribute-set class, property, category and subcategory; allocator `1415e96c0`, preload `141bc1730`, lookup `141baa550`, installed at globals+0x220). Stock `FAttributeSetInitter` lacks this API, so faithful completion means wiring a custom initializer into `UFortAbilitySystemGlobals`. A direct curve lookup would drop behavior.
- **material_names421_runtime:** eleven material parameter FNames recovered, nothing ported (virtual identities unresolved).
- `triage061_new`, `062`, `058`, `060`: drafts missing storage identity, writers or initialization; EventMode belongs to an unemitted module; some call uncaptured `0x1406947d0`. Not countable.
- Seven functions share wrapper `0x142766b10` (steps two 32-bit values, calls bare return): `UCaptureComponent::UpdateCaptureSpeed`, `AFortAthenaMutator_Heist::OnMutatorGenericIntegerUpdated`, and `UFortCheatManager::{AthenaGiveScoreTo,SetViewportResolution,TestPartyMemberRange,TestSafeZoneLocations,TriggerDelayReturnToLobby}`. Confirm class emission, parameters and manifest first.
- `triage067_new`: `UFortGameplayAbility::EquipVehicleWeapon` and `AFortPlayerController::SetClientPawnNewRotation`; confirm `FRotator` mapping and signatures first.

## 9. Blocked and non-portable functions (nothing is silently dropped)

Everything the generator or analysis cannot take is recorded, with a reason and the condition that unblocks it:

- **Interface functions: 252 in-scope functions on 129 `UINTERFACE` classes, none ported.** The exec thunk dispatches through the implementing object's interface sub-vtable (the secondary vptrs seen on `AFortAthenaVehicle` etc.), so there is no body at the interface level; the bodies are the implementers' own virtuals. Reading the interface class's own vtable gave a meaningless stub, so these are group I in the table, never proposed as stubs, and the generator's splice does not support interface classes. **Open decision:** whether to remove these 252 from the dashboard denominator (14,643 -> 14,391) or keep them as a separate "interface dispatch" line. They are unported and cannot be ported as bodies.
- **Classes in packages with no module** (native_class_coverage.md cause C1): `UEpicCMSTileCarousel` (`/Script/EpicCMSUIFramework`), `UUACNetworkComponent` (`/Script/UACBase`). Unblock: add a module declaring the package and add the class to `native_class_set.txt`.
- **UE 4.26 engine functions** (`engine_provided_natives.json` `gap_declared_in_reference`, 31 of them): e.g. `UFXSystemComponent::SetIntParameter`. Unblock: backport the declaration into the engine checkout.

The stub-batch record is `behavior/stub_bulk_runtime/`: `entries.json` (batch 1, 51 parameterless), `entries_batch2.json` (137 with parameters), `skipped_batch2.json` (23 left for review: 18 non-bool returns, 4 net-RPC stub shapes that are not empty plus true-validate, 1 void with a non-empty stub) and `blocked.json` (14 entries with reasons and unblock conditions). Regenerate proposals with `make_stub_proposals.py`; fill parameter signatures from the generator's own reflection-derived signature with `fill_signatures_from_generator.py REPORT_DIR` (propose with the `@PARAMS@` placeholder, run the promotion, read the signatures from the rejection text, patch, rerun until zero rejections). Run the promotion with a copy of the earlier runner (`/tmp/promote-native-batch455.py` pattern: back up source mtimes, run `gen_native_classes.py --promote` with the saved command JSON, assert zero rejections and no removals).

## 10. Short-body batches: what the workers found (2026-11-07)

Pool: 2,213 short non-stub bodies (<= 16 instructions): 1,063 with calls, 576 tail-jump forwarders, 574 leaf bodies (branches, getters, setters, other). First batch: 119 plain leaf getter/setter jobs (`generated/work-batches/leaf{1,2}.json`) to two Haiku workers (`behavior/leaf{1,2}_runtime/`, each with `entries.json` and `dispositions.json`); merged as `behavior/leaf_batch1/entries.json`.

- **Yield: 24 proposed, 21 ported (18% of the jobs).** All 24 were verified against reflection with `verify_leaf_entries.py` (members and offsets; it does not prove semantics). The literal mappings that look semantically odd (`AFortOctopusVehicle::CacheParticleFX` assigns `InDust` to `CacheCoilIdleTopR`) are what the asm does.
- **Why 95 were skipped: 57 have no declared member at the asm offset** (the generated class layout lacks the field), 9 unclear param/member pairing, 9 struct/array/delegate copies, 8 void functions that load but do not store, 12 other. The layout gap is the main blocker for short bodies. Existing precedent: non-reflected fields declared as `member_field` manifest entries with auto-names such as `NativeRecoveredBool_0x430` (383 ported). A getter's return type (or a setter's parameter type) gives the member type, so these can be unblocked by declaring a typed `member_field` per offset.
- **Reflected member offsets are not in the generated headers** (only `table index N, flags`); they live in `fortnite_game_reflection.json` (`classes_in_groups_with_fort_names[].properties[].offset`, decimal) and bool bits in `bool_bits.json`. Class names map through `class_name_map.json` (block VA -> name) and `class_parent_map.json`.
- **Manifest is not an inventory of ported bodies.** Generator policies provide bodies without manifest entries (`porting/frontend/*`, `porting/generator_policies`); adding a manifest entry for such a function fails with e.g. "GetCurrentInputType declaration collision" (`UCommonInputSubsystem::GetCurrentInputType`, now recorded in `blocked.json`). The dashboard counts manifest entries only, so policy-provided functions are an undercount, and some of the remaining 11,407 may already be done by policy. Quantifying that is open work.
- Rejected by the generator and recorded in `blocked.json`: `UFortChatContainer` and `UFortMediaSubtitlesPlayer` (packages with no module).
- Automation: `run_native_batch.py ENTRIES.json TAG` (append, promote, learn signatures from the generator, rerun, report; never drops entries itself); `verify_leaf_entries.py ENTRIES.json`. Bodies that dereference another native class use a typed local (`UFoo* Local = Member; return Local->Field;`) so the generator infers the include; compile is unchecked until the build checkpoint.

## 8. Recommended next steps

1. Review the 23 skipped stub-family functions (`skipped_batch2.json`), especially the value-returning ones with an empty body, which look suspicious. The rest of the stub family (188 functions) is ported.
2. Recover missing class layout for short bodies: declare typed `member_field` entries for the offsets that getters/setters touch (type from the signature), then re-run those 57 skipped jobs. Then the other leaf shapes (branching leaves 323, forwarders and short calls) via Haiku batches with `generated/work-packets/`.
3. Fix the const-reference generator bug before porting functions with const-reference params.
4. For multi-callee and multi-site virtual rows, verify the chosen body per function before transcription (argument count is not a reliable check).
5. Leave large bodies (Kismet library, player controller, minigame, creative tools) for individual transcription.
6. Run the deferred broad build and runtime check at the agreed body-count checkpoint.

Recount snippet:
```python
import json
nf=json.load(open('porting/generated/reflection/native_funcs.json'))
m=json.load(open('porting/behavior/MANIFEST.json'))
ep=json.load(open('porting/progress/engine_provided_natives.json'))['verified']
epset={(c,f) for c,v in ep.items() for f in v}
ex={(c,f) for c,fs in nf.items() for f,d in fs.items() if d.get('exec_addr')}
done={(e['class'],e['function']) for e in m if e.get('status')=='ported' and e.get('kind')!='member_field'}
print(len(ex), len(ex-epset), len(ex-epset-done))   # 20574 14643 11407
```

Related docs kept: `master_plan.md`, `native_classes_status.md` and `native_class_coverage.md` (class and type promotion, not function bodies), `decompilation_notes.md` (decompile quality; long, historical), `gen_native_classes.md`, `binary_analysis.md`.

## 11. Receiver adjustment and thunk-logic checks (learned from the layout-recovery pass)

- **Body offsets are relative to the body's `this`, not the object.** An exec thunk may `ADD RCX,N` before the call (secondary-vptr or
  interface dispatch: `MOV RAX,[RCX]; CALL [RAX+slot]` after `ADD RCX,N`), and the vtable target may itself be an adjustor
  (`ADD RCX,M; JMP body`, which `resolve_wrappers.py` records as `resolved[].this_delta`). Object offset = thunk delta + wrapper
  `this_delta` + the asm offset. Example: Octopus `IsBoosting` reads `[RCX+0xc8]` in body `0x141b720c0`; the real byte is at
  `0x3b0 + 0xc40 + 0xc8 = 0x10b8`. `porting/audit_receiver_adjust.py` lists adjusted thunks; `porting/lookup_offset.py CLASS 0xOFF`
  shows the reflected member (own or generated ancestor) covering an object offset. `work-packets` do not yet carry the delta: compute it.
- **Thunks can contain the function.** Before trusting a body, read the thunk asm: extra compares, casts, or interface calls on
  arguments mean the logic is inlined in the thunk and the called body is only a helper (`AFortAthenaMutator_Arsenic::IsPawnAnInfected`,
  `UFortWorker::GetWorkerTypeBP`). Struct parameter decoding adds benign virtual calls; judge by whether the result depends on them.
- **Undeclarable layout.** A field in inherited engine/generated parent storage, a bit owned by a declared bitfield, or a pointee class
  that is not recovered is accessed with explicit raw offsets from `this` (byte-exact to the asm, commented). Unreflected gaps inside a
  class's own region become typed `member_field` entries named `NativeRecovered<Kind>_0x<objectoffset>`. Never invent a member inside
  inherited storage.
- **Generator limits found:** classes owned by a policy (`UCommonButton`) reject manifest members/bodies; delegate types and enums with no
  emitted declaration make a function "not emitted"; snippet comments must not contain `not recovered`, `stub body`, `TODO`, `placeholder`.
- **Denominator:** interface functions (group I, 252) have no body and are excluded from the native denominator
  (`porting/progress/native_interface_excluded.json`, applied by `native_status.py` and `refresh_totals.py`; 14,643 -> 14,391).
  The dashboard reads inputs from the committed tree, so commit `totals.json` and the in-scope files before regenerating.
- **Blocked register:** `behavior/stub_bulk_runtime/blocked.json` lists only genuinely blocked functions, each with `blocker`, `reason`, `unblock`.
