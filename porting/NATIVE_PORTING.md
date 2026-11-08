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

### 11.1 Tail-jump thunks: the body is the jump target (found after the first leaf batches)

About 1,700 exec thunks (B ~1,050, C ~650) are `frame advance; JMP target` (the asm ends in a far direct `JMP`). The decompile
of such a thunk shows the *target's* code, so `build_thunk_table.py` picked "callees" and virtual-call targets from inside the
real function instead of the target itself; only 40 of the 1,703 previously matched. `porting/resolve_tailjumps.py` finds them
from the captured thunk asm, captures the targets, follows pure adjustor wrappers and writes `generated/thunk-table/tailjmp.json`;
`final2.json` was re-pointed (original kept as `final2_pre_tailjmp.json`; rows carry `body_source: thunk_tail_jump`). Group B/C counts
and the "stub body" detections for these rows changed accordingly: 23 already-ported entries (12 stub, 11 leaf) used the wrong body
and were removed (blocker `real_body_is_tail_jump_target`, real body recorded). Rerun the script after any new thunk capture.
- `porting/thunk_receiver.py` gives per-thunk receiver delta, extra direct calls and indirect calls on non-`this` receivers; the
  work packets print `receiver:` and `CHECK THUNK LOGIC:` lines from it (rebuild packets with `build_work_packets.py`).
- Before accepting any "stub"/"empty" classification, confirm the thunk does not tail-jump or call anything else.
- **Body-location counts after re-pointing** (supersede the section 3 table): A inline 788, B single body 8,242, B multi-candidate 527,
  C virtual target 1,594, I interface 252, U 4 (of 11,407). The multi-candidate backlog shrank from 1,312 to 527.

### 11.2 Shared "free the parameter" bodies and generator gaps (small-leaf pass)

- **By-value parameter destructors are bodies too.** Bodies such as `0x14072e780` (`MOV RCX,[RDX]; TEST RCX,RCX; JNZ 0x142bee710; RET`, 52 functions),
  `0x141ccaf30` (`[R8]`), `0x1406963a0` (`[RDX+0x18]`), `0x140692260` (`[RCX]`) are not logic: `0x142bee710` is the free routine and the body only
  releases the heap buffer of a by-value FString/TArray/struct parameter (C++ generates that destructor). The native body is EMPTY. Check the learned signature has the
  by-value parameter at the position the register implies (RDX = first, R8 = second after `this`; floats take XMM slots but do not shift GPR positions), then port with `_shared/return.inc`.
  52 functions were ported this way.
- **Generator gaps found (functions blocked until fixed):** classes outside the native class set (UAbilitySystemGlobals, others); policy-owned classes (collisions); delegate/enum
  declarations the generator cannot emit; hand-written UFortKismetLibrary.
- **RPC events need no generator change:** the earlier `native_event_mismatch` came from filing an RPC under kind `native_event_impl`. Use kind `impl`
  with function `<Rpc>_Implementation` (and `validate` with `<Rpc>_Validate` for Server RPCs): the generator already emits both. The 504 Net+event functions
  are ordinary jobs; `build_job_batches.py` derives the kind (net -> impl, plain event -> native_event_impl, else exec).
- **Wrapper tier:** `build_small_pool.py 10 calls` writes `generated/small-pool/pool_calls.json` (<=10 instructions that CALL/JMP out);
  `build_job_batches.py POOL PREFIX N` probes exact signatures (restoring only the files it changed) and writes worker batches. A wrapper can only be ported
  when every call target is a named native function (final2 real body or ported `native_body_address`); targets that are bare `FUN_` addresses stay deferred.
  Policy-owned classes (the POLICY set in `build_job_batches.py`) are excluded because their policy emits colliding methods.
- **Older-port audit (entries with no recorded `native_body_address`, 2,136):** `audit_unrecorded_bodies.py` plus a thunk-asm check found one real defect class: 31 ports
  whose snippet transcribed the exec thunk (`param_1`/`param_2`, `Stack->Code`) instead of the body. They did not compile and were counted; they are removed and listed in
  `behavior/stub_bulk_runtime/decompile_thunk_ports_removed.json` (20 re-entered the table as group B/C rows in `final2.json`). All 420 tail-jump thunks of the rest were checked
  (380 target return stubs; the other 40 match their bodies). The 5 "stub-exec with extra virtual calls" entries are false positives: the extra virtuals are the by-value `FText` shared-ref
  destroy (`[RAX]`, `[RAX+8]` on a non-this receiver), so the stub body is correct. CommonUI engine calls used by wrapper ports are declared in the Fortnite 14 CommonUI source only; the project has no 12.60 CommonUI headers.
- **Pool filter:** `build_small_pool.py` selects clean-thunk, call-free bodies of <=10 instructions, but a conditional jump out of the body (`JNZ far`) still escapes the
  filter and yields a body that is a prefix of a larger function; worker batches classify those as deferred (`behavior/stub_bulk_runtime/deferred_tiers.json`).
- **Signature probe recipe:** add dummy entries with `@SIG@` and `_shared/return.inc`, run `run_native_batch.py`, read the learned signatures from the manifest, then restore the
  manifest copy (never `git checkout` a directory: other sessions leave uncommitted edits in the tree). The real promotion regenerates the touched files.

### 11.3 Resolved-call tier, the virtual-slot attempt, and what actually blocks the rest (2026-11-07)

Measured with `build_call_pool.py` (bodies of at most 60 instructions, clean thunk, one captured body; counts are bodies):

- **Resolved-call tier** (`build_call_pool.py`, `build_job_batches.py`): every call target must be a reflected function found by address, a `GetPrivateStaticClass` getter, or a known engine helper (`FMemory::Free` `0x142bee710`, multicast broadcast `0x1406a9dc0`, verbosity-gated log `0x142c54b70`, weak-pointer get/valid/assign `0x142ee5dd0`/`0x142ee7880`/`0x142ee3b30`). Bodies with any other target are skipped.
- **Unresolved callees are the largest blocker: about 2,060 bodies** (2,552 at a 120-instruction limit). Their callees are internal C++ functions with no reflection entry, so generated code has nothing to call. 917 bodies have exactly one such callee, spread over 574 distinct functions. Only 730 of 2,256 distinct callee addresses have captured asm. Yield if the top N callees were named or ported: 25 -> 211 bodies, 50 -> 287, 100 -> 390, 200 -> 529. The top callees are a mix of engine API (`0x142cd4ab0` builds an `FString` from a `TCHAR*`, 108 callers) and unreflected game logic (`0x144d03d50`, a weak-object chain walk, 114 callers). Unblocking them needs a helper registry (address -> engine expression or a generated `NativeHelper_<addr>` free function); that does not exist yet.
- **Indirect calls, about 2,700 bodies at a 120-instruction limit, are not a single recoverable group.** Of 1,484 indirect-call bodies examined at the 60-instruction limit: about 405 call virtuals on `this` (45 resolved to a named slot, 360 could not be named) and 1,079 call through some other object: about 630 unknown provenance (call results, stack spills), about 280 member pointers, about 65 parameters, about 35 register calls (function pointers/delegates). Counts come from an approximate register tracker.
- **The `this`-virtual resolver was tried and not committed.** It named a slot from the reflected function whose exec thunk makes one virtual call at that slot, searched along the class chain. The result was wrong: 45 candidate bodies became 35 jobs and only 1 ported (`AFortAIPawn::SetCanInteract`); 14 jobs were `virtual_slot_mismatch` (the named function is void or takes different arguments than the asm uses, or is the function itself) and most of the rest were `callee_not_declared`. Thunk-based slot names are not reliable. A usable resolver must name a slot by address (read the slot from the object's class vtable and match it to a reflected function body) and leave the slot unresolved otherwise. The ceiling for `this` virtuals is about 400 bodies, not the full indirect group.
- **A larger size limit does not help.** Raising the limit from 60 to 120 instructions adds about 20 pool jobs, because unresolved callees and indirect calls dominate.
- Batch records: `behavior/vs1_runtime`, `behavior/vs2_runtime`.

### 11.4 Build checkpoint and smoke test (2026-11-07)

First compile of the roughly 700 ports added since the last build (UE 4.25, `FortniteLinux` and `FortniteLinuxServer`, Development):

- **UnrealHeaderTool:** 16 ports rejected, because a parameter was a struct the tree does not declare as Blueprint-supported (`FCard`, `FAshtonStoneState` containers and others). Removed and recorded in `behavior/stub_bulk_runtime/build_removed_unsupported_struct.json` (unblock: declare the struct with its recovered layout).
- **clang:** about 55 more ports failed (undeclared recovered members, thunk-transcribed code mentioning `Stack`/`ulonglong`, wrong return types, shadowed locals, incomplete types). Removed with their first error recorded in `behavior/stub_bulk_runtime/build_removed_compile_errors.json`. `porting/build_fix_round.py` does the attribution. 20 bodies that read private `AActor` members (`Role`, `Owner`, `PawnPrivate`, `CharacterMovement`) were fixed in place to use the public accessors.
- **Incomplete types:** snippet `includes` accept engine headers only, but the generator derives includes from type names in the body, so a ported body that only calls through a forward-declared pointer must name the type (`static_assert(sizeof(T) > 0, ...)`).
- **Interfaces:** the `Native*163` helpers are called through `IFortVehicleInterface*` and `IFortTeamActorInterface*`. The new policy `generator_policies/native_virtual_interfaces_runtime/` declares those virtuals on the interfaces (non-pure, neutral defaults), generates `UFortTeamActorInterface` (derives from `UGenericTeamAgentInterface`; removed from `native_class_blocklist.txt`, added to `native_class_set.txt`) and makes the topmost implementers derive from the interfaces (13 team classes, 2 vehicle classes).
- **Result:** `FortniteLinux` and `FortniteLinuxServer` build with exit 0. Replacing the staged executables (old ones kept as `*.prev`, git-ignored) and running `server/smoke_join.sh` gives `IpNetDriver listening on port 7777`, network checksum 683914758, `Join succeeded`, `Welcomed by server (Level: /Game/Maps/FortniteEmptyDedicated ...)`, and no fatal errors in either log. (`smoke_join.sh` needs `rg`; this shell had none, so a `grep -E` shim was put on `PATH`.)
- **Not covered:** gameplay behaviour of the new ports is untested; the smoke test exercises startup, map load and join only. The removed ports return to the pool.
