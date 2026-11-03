# Native function port handoff — 2026-11-03

## Stop state

User explicitly requested: “write the handoff and stop.” All three Terra workers are terminal. No Ghidra, generator, build, runtime test, or external-model process was started after that request. Goal is being paused at the user's request; do not resume without user instruction.

Repo: `/opt/fortnite/FortniteClient-12.60-CL-13477524`; parent `/opt/fortnite` is not the Git repo. Source is committed and pushed through **a037f865** (`Port trap geometry, vehicle state, driver detection and death report reset`). Latest successful source generation: `porting/generated/gameplay-bulk-integration-455/reports/behavior_splice.json`: **3,684 accepted = 3,341 bodies + 343 fields; zero rejected**. No accepted source changes remain uncommitted. Pending proposals below have NOT been added to the canonical manifest.

Dashboard files are dirty and require refresh/review/commit on resume; last explicitly committed native dashboard was 31b5f5ba at 3,298 bodies. Several refreshes since then are uncommitted, and another asset-work session also updates progress files. Do not stage that session's work accidentally.

This replaces a 504 KB concatenation of stale checkpoints. Older checkpoint history remains accessible through Git history.

## Non-negotiable continuation instructions

Read the active goal attachment before resuming:
`/home/vibrato/.codex/attachments/b7d98663-f76c-4409-8f6a-b5264a90db67/pasted-text-1.txt`.
Goal: finish the entire native function body block faithfully, using efficient bulk work, frequent meaningful commits/pushes and dashboard updates. Do not mark complete while bodies remain.

- **No builds, tests, or runtime checks until 4,058 bodies. Last green tested checkpoint: 3,058.** Static evidence captures and zero-rejection source generation are permitted. None of our work in this checkpoint ran builds/tests/runtime; the independent asset session may run its own probes.
- Primary is planner/orchestrator/reviewer and also implements difficult integration work. User explicitly requests continued rotating **gpt-6.1-terra, low reasoning, fork_turns:none** bulk workers. Four concurrency slots total: root + three workers. Bounded related batches; rotate at useful stopping points. User's explicit Terra-low request supersedes medium/high defaults for this bulk work. Small read-only tasks may use Luna medium per AGENTS.
- Claude, Haiku, agy and external model sessions remain paused/freed. Do not start them. Pause marker: `/opt/fortnite/local-model-package/runtime/haiku262/claude-paused.json`.
- Economical reads: bounded rg/script summaries, small matched windows and log tails. Avoid printing minified reflection JSON with rg: one line can be megabytes. Parse JSON and return selected fields.
- Root alone owns canonical `porting/behavior/MANIFEST.json`, generator, generated Native files, source generation, commits/pushes, and Ghidra. Workers own separate proposal directories. **Ghidra captures are serialized**; exact `porting/generated/native*/0xADDRESS.c` cache lookup before every requested capture.
- No routine Markdown handoffs; JSON native evidence/dispositions preferred. This handoff was explicitly requested by the user.
- Preserve unrelated/concurrent changes, especially `large_bulk150c_runtime/native-evidence.json`, asset pipeline/CUE4/progress edits, old untracked proposals and graphics. Never reset/clean the shared tree.

## Ready or conditional proposals left unintegrated

### vehicle_shared453_runtime — four ready bodies

`GetDriver`, `ContainsPlayers`, `GetAllPassengers`, `GetPawnAtSeat` for AFortAthenaVehicle. Reuse now-canonical virtual `NativeGetSeatComponent163` and existing UFortVehicleSeatComponent helpers `NativeGetDriverPawn163`, `NativeContainsPlayers163`, `NativeGetAllPassengers163`, `NativeGetPawnAtSeat163`.

Read final .inc/manifest/evidence before promotion. Worker was told to name a typed `UFortVehicleSeatComponent*` local so generator emits the complete native class include. GetAllPassengers converts the canonical helper's `TArray<APawn*>` to reflected `TArray<AFortPlayerPawn*>`; verify exact filtering/order against the captured native chain. Original component dereferences have **no null guard**. Ordinary virtual reflected methods require `virtual:true` to preserve dispatch.

Five target captures are already in **native459-callees**: FindSeatIndex 14084ba30, CanContainPlayers 14190df80, ClientIsDriver 14084b700, GetSeatWeaponComponent 1419178d0, SeatIsTurret wrapper 14283cd20. Follow-on requests were NOT captured before stop:
`14084b6a0`, `1419177a0`, `141d06fd0`, `141d0a110`, `14084f960`.

### trap_data454_runtime — conditional three-entry proposal

Two bodies: GetDamageDelay and GetFireDelay; one virtual helper NativeHasTrapData. Uses `TrapData->WeaponStatHandle.GetRow<FFortBaseWeaponStats>(TEXT("UFortWeaponItemDefinition::GetWeaponStats"))`, downcasts the returned base row to FFortTrapStats and reads DamageDelay/FireDelay. Native has no row-null guard after the data-validity virtual.

Required row UScriptStruct helper **142b23580 is now captured in native459-callees**, but final verification has not been done. Worker finished before root delivered it. Confirm it is FortBaseWeaponStats. `FFortTrapStats` is not yet generated; generator TypeGate.body_refs (~5127) may auto-promote it when body references it. Inspect resulting header/struct closure and required modules.

Worker manifests incorrectly list native headers (`UFortTrapItemDefinition.h`, `FFortTrapStats.h`) as snippet includes; these must be removed because explicit includes are engine-index-only. Put native type names into body typed locals/casts so generator infers native includes. NativeHasTrapData only checks TrapData!=nullptr and needs no complete native header. Do not count/promote until zero source-generation rejections and dependencies are closed.

Other trap queries remain blocked: GetTrapRating definition virtual +2d0 unidentified; GetMaxDurability/percent need the full lazy attribute-set creation helper 141b94070 (cached native303-callees) through 141b9b750.

### building_max452_runtime — evidence only, empty manifest

GetHealth/GetHealthPercent depend on GetMaxHealth fallback. The custom Fortnite attribute initializer is now identified, but not implemented/wired. No faithful source body is ready.

- UAbilitySystemGlobals base ctor 1406ed860: native456-callees.
- Base globals vtable 1459c1c38; Fortnite globals vtable145d1ac28, ctor1415e60e0.
- Native457-callees contains seven allocator candidate bodies.
- Fortnite allocator 1415e96c0 (slot+2f0) allocates 0x58, stores vptr145d1a570, installs shared pointer at globals+220.
- Initter slot+30 target141b9b170, +28 target141badad0, +08 target141bc1730; these plus core lookup141baa550 and helpers14070fd10/142d67db0 are captured in native459-callees.
- Custom lookup is a map hierarchy keyed by attribute-set class, property name, init category/subcategory. Missing subcategory falls back to None only if requested category is non-None. Found curve evaluates at float(level) via curve virtual+88, writes output only on success. Preload141bc1730 walks CurveTables and parses 3/4 row-name components.
- Stock FAttributeSetInitter lacks this Fortnite map/API. Faithful completion needs custom initializer allocation/preload integration into UFortAbilitySystemGlobals, plus proven policy bytes/level selection. Replacing it with a direct curve lookup would omit behavior.

Other useful uncommitted evidence: material_names421_runtime recovered eleven material parameter FNames and owner fields; no functions ported because virtual identities/helpers remain unresolved. Its initializer captures are native425-callees. Broad short-wrapper candidate list `/tmp/native-short-candidates418.json` and worker dispositions 428/429/430 exist; most remaining wrappers need actual vtable target resolution, not another superficial scan.

## Key integration lessons and reusable proof

- Native virtual wrappers can be resolved in bulk: derive primary/secondary vptr from native constructor; read slot qword from original flat image; capture actual target. Secondary this offsets matter. AFortAthenaVehicle primary145debe20, secondary145decf00 at actor+3b0. ABuildingTrap primary145f46db0. Original flat image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base140000000, file offset=VA-base.
- Generated native method bodies must preserve original virtual dispatch. Manifest `virtual:true` supported for ordinary reflected methods and helpers. Native Blueprint events use **kind native_event_impl**, pinned reflection_flags, function `_Implementation`; RPCs use kind impl. Do not flatten a BlueprintNativeEvent into ordinary exec. See committed native_event448_runtime for null default GetTeammateReviveGameplayEffectOverride.
- Original native empty implementations are valid faithful ports, not guessed stubs: require whole RET body and exact vtable proof. constant_close418 and virtual_bulk434 are examples.
- `/tmp/native-empty-rpcs458.json` is an UNVALIDATED research list. It scans three old vtable tables, but the names are inherited and not own reflected functions on the table class. **Do not promote those blindly**; validate class/constructor/slot/owner/override semantics first.
- Field defaults may be proven by engine zero allocation plus complete constructor and parent/derived extent: UObjectGlobals.cpp2452 Memzero(TotalSize). Dual_fields404 and zero_fields419 show exact evidence. Explicit constructor writes are preferable. Do not guess offsets/types/defaults; constructors often already cached in native129/native126/native111 streams.
- Engine header index now scans Plugins/FX/Niagara; plugin and module were already enabled. Change committed in c37e3728. Native enum headers should not be explicit snippet includes; return/body type inference promotes them.
- Struct/enum reflection signatures are authoritative: byte enum parameter often bare `E::Type` rather than TEnumAsByte; regular enums lack `::Type`. Correct proposal and canonical signature together after generator rejection.
- Root centroid implementation in centroid_close440 uses explicit scalar native order, including TX/TY/TZ doubling and cross + (T*W+offset), then translation. This avoids arithmetic reordering from calling the generic engine RotateVector.

## Source generation, commits, dashboard

Latest runner `/tmp/promote-native-batch455.py`, report `porting/generated/gameplay-bulk-integration-455/`. Copy runner and change integration number for next batch. It reuses command JSON from integration-3, preserves identical Native file mtimes, and asserts zero rejections and no removals. Generator itself can exit0 with rejected snippets: always inspect `behavior_splice.json`.

Check duplicates by `(class,function)` across kinds and check actual generated C++ too: hardcoded implementations may not be in canonical MANIFEST. Confidence tags required in every .inc. Stage only reviewed proposals, accepted canonical/native changes and required new generated headers. If referencing an old untracked helper include, stage that exact include too. `NativeGetSeatComponent163` now canonically references the staged target_filter163 include; do not stage the rest of that large draft directory indiscriminately.

Meaningful source commits pushed during this checkpoint: c37e3728,23dc1490,dfea5748,da0f4835,8792eca3,5a03a35e,478a2977,32722758,471c794e,**a037f865**. Independent asset-session commits also exist; preserve them.

Dashboard refresh commands (on resume):
`python3 porting/progress/refresh_totals.py`
`python3 porting/progress/progress.py --quiet`
`python3 porting/progress/publish.py`
Review dirty/concurrent progress changes before staging README/docs. Do not run tests/builds until 4,058 bodies. Resume only when the user asks.
