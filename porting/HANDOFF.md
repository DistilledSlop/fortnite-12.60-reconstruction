# Handoff: state of the Fortnite 12.60 Linux reconstruction

## Current resume point (2026-11-02)

- Current data/storage candidate: frontend/common_action_binding_runtime/ implements actual
  UE-owned binding records, FText/key/delegate copies, process TMap and modulo32 ID allocation.
  Original signatures and ctor/storage/counter contracts are closed under work/action-binding-*
  and common-action-binding-record-types-1. NaN classification preserves distinct tag/legacy
  branches and ordered comparisons respect DAZ; dead weak-bound delegate instances are cloned.
  Normal generation adds only CommonUI/Generated/CommonActionBindingData.h/.cpp and preserves
  12710other mtimes. New ActionBindingDataProbe exercises26 engine cases (UE validation pending).
  This is DATA/storage only: explicit resolver interface, no fabricated default provider,
  no original UFUNCTION registration or completed production router/lifecycle/UI claim.
  Portable fast-math/DAZ/ownership tests2 and runner/verifier41tests pass; generator141checks pass.
  Root owns Native/generation/build/stage; no other worker may mutate them during validation.

- Claude's effort review is implemented at dashboard commit3d62edb: 132leaves,
  2019relative points, UI199/CommonUI44. Original +78 UI proposal was not applied.
  Duplicate Wine/join, mesh/general-package and voice-route credit is separated; seven
  capped census tasks now expose their declaration and unfinished integration shares.
  Three one-point foundations are carved from residual budgets and have four source-checked
  passing ledger entries. Missing full-game rendering/animation/physics/scale/replay scopes
  are explicit, low-confidence estimates at0%; all original modes remain in scope.
  Applied decisions/status: progress/effort_review/implementation_status.json. Reports and
  original snapshot remain unchanged; USER starts Claude, do not invoke it.
- Latest Native dcc14c3 restores original CommonInputSubsystem.GetCurrentInputType.
  Both builds PASS132.953/138.969s with source/config stable. Real LocalPlayerSubsystem
  metadata/direct/ProcessEvent/restoration diagnostic13rows PASS; eight retained diagnostics,
  table421rows/30checks and both joins21.599/21.595s PASS. Original Root creation remains
  seven widgets/all4startup classes; skipped imports fall22->20 (17distinct in actuallog).
  Input producers/routing and interactive UI are not established. Last viewport attachment
  remains the failed a93a1c0 attempt; GetCurrentTextStyleClass and queue renderer are still missing.
- Current-type checkpoint: generated/frontend-native-evidence/common-input-current-type-runtime-1/checkpoint.json
  SHA256 bad3def28dc6c5a3f72e39f9f1ab720c39675b78c718df5e6b39f5d387772192;
  archive /mnt/vibrato/fortnite-evidence/frozen-runtime/common-input-current-type-runtime-1
  binds14source pairs,86artifact pairs,18binary rows. Release SHA256
  86103596459b8329ea54f862979f7e4412aa17b5c67c44723429251b81245b2b.
  TERMINAL promotion96569/build40525/stage71343/runtime49407; no Native owner is active.
  Scripts /mnt/vibrato/fortnite-work/*common_input_current_type*1.py. NativeSource12708files;
  regeneration changed only UCommonInputSubsystem.h/.cpp, preserving12706other mtimes.
  Independent Luna audit rehashes100source/artifact pairs and checks13rowtranscript/provenance;
  /mnt/vibrato/fortnite-work/common-input-current-type-receipt-audit-1/HANDOFF.md.
  Giant debug contents not independently rehashed; source/archive/stage producer hashes retained.
- Foundation recorder: progress/record_frontend_foundation_runtime.py validates historical
  releases, critical source unchanged across phases, actual pinned diagnostic verifiers,
  process/build/stage/join receipts and frozen artifacts. Six rejection/tamper tests pass.
  UIcredit is limited to button defaults/query, input row+accessor and startup selections.
- History tab implements HIST01–10: deltas/replay+known model markers, scale, filters/paging,
  detail selection, UTC presets/date axis/category trace, touch/focus/accessibility. Generated
  Progress61tests plus JS syntax and17interactions pass in functions.exec V8 with minimal DOM double; browser/visual
  behavior remains unverified. Smoke artifact /mnt/vibrato/fortnite-work/history-v8-smoke-1.json.
- New bounded proofs under /mnt/vibrato/fortnite-work/: common-button-action-registration-1
  corrects140800e60 as binding creation, actual cleanup1408018c0. Routing/dispatch still open.
  common-action-widget-slate-mapping-1 closes SBox->SOverlay->three SImage graph, exact brush/
  progress-visibility/layout+volatility calls and release ownership. These are source proofs,
  not integrated UI. Both pinned-image replays pass.
  common-action-fallback-brush-gate-1 additionally closes static FSlateNoResource fallback
  and live CommonInput.ShowKeys console variable (default1, nonzero gate). Do not substitute
  default FSlateBrush + NoDraw (wrong image size) or a constant gate. Replay passes.
- Previous button baseline04822b2 restores six explicit CommonButton reflected flag defaults
  plus inherited focus. Both builds PASS188.045/147.589s with source/config stable;
  actual MainButtonStateProbe44rows checks all7defaults on CDO and fresh widget.
  All8retained diagnostics, original table421rows/30checks and both joins21.565/21.569s
  PASS. Root creation remains7widgets, all4startup classes,22unresolved imports.
  No new viewport attachment was attempted; previous a93a1c0 failure below remains
  unresolved. Full unreflected constructor state/lifecycle and all4milestones remain open.
- Defaults checkpoint: generated/frontend-native-evidence/main-button-defaults-runtime-1/checkpoint.json
  SHA256 fc8a03edc6e938e455229bbf7ab552a58899bede6ffbe9b5021389f2f178a8ca;
  archive /mnt/vibrato/fortnite-evidence/frozen-runtime/main-button-defaults-runtime-1
  binds10source files,82artifact pairs,18binary rows. Release SHA256
  f2ed9e2c6a82fdc887bc5dfa095e4d952e9588c6a75f0095d59d03fcc23facd1.
  TERMINAL promotion45360/build8269/stage25815/runtime14378. Scripts in
  /mnt/vibrato/fortnite-work/*main_button_defaults*1.py. Normal regeneration changed
  only generated UCommonButton.h/.cpp, preserving12705other mtimes. Tests: defaults/query5,
  new portable cores6, dashboard47; source evidence11+6spans verified. Engine edits preserved.
  Policy/evidence: frontend/common_panel_runtime/main-button-defaults-*.
  Independent Luna audit PASS rehashes all82artifact pairs and10source pairs and inspects
  the CDO/instance checks: /mnt/vibrato/fortnite-work/main-button-defaults-receipt-audit-1/HANDOFF.md.
  Giant debug binaries were not independently rehashed; the stage producer recorded their hashes.
- New source work: common_panel_runtime/{CommonButtonSelectionCore.h,
  CommonButtonTextStyleCore.h,CommonActionKeyLookupCore.h} implements portable recovered
  selection/style/notification order, text-style fallback and ordered controller/key lookup.
  Companion evidence/tests pin original spans/signatures; these are NOT registered or wired
  into Native yet. Do not treat portable tests as UI/runtime parity or add completion credit.
  Actual Root runtime below is unchanged. Main state/lifecycle and action-widget adapters remain.
- Lifecycle proofs now close Initialize containment and binding, direct-parent virtual slots,
  rebuild->NativeConstruct dispatch and FCommonButtonMetaData weak-this ownership. Scratch:
  common-button-{initialize-closure,lifecycle-slots}-1 under /mnt/vibrato/fortnite-work/.
  Action identity/brush proofs are common-action-widget-{refresh-identity,brush-contract}-1.
  InputActionWidget is +b68; its refresh uses real multi-row brush lookup and visibility,
  including repeated Collapsed calls. Global gate14081d130 ignores its subsystem argument.
  Construct binding proof common-button-construct-bindings-1 closes +470 input-subsystem
  listener/+478 removal; +488 nested action registration and cleanup remain unclosed.
- Next source integration: CommonButton selected/style/lifecycle and shared action-binding
  registry remain unfinished. frontend/common_action_display_text/ recovers the original
  first-row/input-type/hold-text getter but has no registered body: widget+240 is an int32
  global binding-registry key, not a weak pointer field. Registry creation/routing and
  ownership must be represented before wiring the helper. FCommonInputTypeInfo holdflag
  setter is independently proved. Do not replace registry lookup with a permanent miss.
- common-action-binding-record-types-1 proof closes142c0c240 as FText default ctor;
  CommonButton+328/+340 and descriptor+28 are FText, not FKey. Record copy ownership includes
  shared text, weak routing control, cloned delegates and owned FKey/float arrays. Original
  type/API names and delegate signatures remain open; no registry integration is claimed.
- Dashboard effort audit: Luna medium screened all117leaves/1855weight units. User requires
  total effort only, excluding importance; persisted in AGENTS.md and progress/README.md.
  No numeric changes justified by metadata alone. Review overlapping package/static-mesh,
  native-body/feature, and shared Wine startup allocations before reweighting. Scratch audit:
  /mnt/vibrato/fortnite-work/dashboard-effort-weight-audit-1/{audit.json,HANDOFF.md}.
- Previous viewport probe sourcea93a1c0 adds an opt-in original Root viewport lifecycle probe. Both builds
  PASS101.407/93.457s with Native/config inputs stable. Creation control still returns7widgets,
  all4original startup classes and22unresolved imports. Actual table421rows/30checks,8retained
  diagnostics and default/Athena joins21.762/21.637s pass. All4product milestones remain open.
- Original attachment fails with processSIGSEGV(-11) AFTER before_attach and before after_attach.
  No attachment/frame/removal success is claimed. Core proves fatal UObject::CallFunction(Function=null)
  during OpenButton (IconTextButton_C) PreConstruct -> UpdateTextStyle. Bytecode offsets10/11..18
  are EX_FinalFunction/nullpointer. Serialized original call is CommonUI.CommonButton.GetCurrentTextStyleClass,
  import-49, no params, .uexp11379 bytes1c cf ff ff ff16. The call identity is proved, not inferred.
- A separate earlier ensure is missing UWidget::RebuildWidget for WidgetQueue_ModalContent,
  classCommonActivatableWidgetQueue, parentCommonActivatableWidgetContainerBase. Corrected livegdb
  decodesexactobject/class via GNameBlocksDebug; hiddenreturn means self is RSI, not RDI. Thisensure
  is distinct from the later fatal null-call. Do not add a spacer/emptyhandler to hide either problem.
- Runtime checkpoint generated/frontend-native-evidence/root-viewport-attach-runtime-1/checkpoint.json
  SHA256 ffe3158d83f58b950eec05b654a8861cb34c7c4cc013e45f0d9d3dc009c1eca9;
  archive /mnt/vibrato/fortnite-evidence/frozen-runtime/root-viewport-attach-runtime-1 binds5sources,
  82artifact pairs,18binary rows. Release SHA256
  fc5a86d46996feb6617638de6a69e456b0d0d737b11e82994ca7a03f4433c6ba.
  TERMINAL handles: build42905/stage33892/attach12848/create82360/controls55752/joins67203/row81786/
  freeze78195. All debugger agents also finished. No live Native/build/stage/probe owner remains.
- Reproduction scripts /mnt/vibrato/fortnite-work/{build_,stage_,run_,freeze_}root_viewport_attach*1.py;
  nativepreflight root-viewport-attach-ue-preflight-1. Tracked runner --attach preserves independent
  viewport_verification.json, lastphase and attachment facts even if Root dependency checks fail.
  Originalmode stillusescreationonly. NativeSource/config/stagedRootclasses unchanged;14probe tests pass.
- Next implementation dependency: source-faithful GetCurrentTextStyleClass plus real CommonButton
  selected-state/style/lifecycle representation. Getterproof common-button-text-style-getter-1 has
  classCDO fallback logic for disabled, selected-hovered, selected, hovered and normal styles.
  Constructor clears selected bit, but public SetIsSelected, SetSelectedInternal, ClearSelection,
  click/group paths also produce changes; do not substitute permanentfalse. Transition proof is
  common-button-selected-transition-contract-1. Style refresh proof common-button-style-refresh-contract-1
  closes events OnCurrentTextStyleChanged/BP_OnSelected/BP_OnDeselected and UButton::SetStyle order.
  Embedded FButtonStyle metadata and Initialize containment/binding are now proved above;
  next close native construct registration, action-widget/brush adapters and remaining
  reachable transition/input/delegate operations before registering the getter.
- Parallel queue renderer proof activatable-queue-rebuild-contract-1 identifies original SOverlay,
  SCommonAnimatedSwitcher and SSpacer, shared ownership and boundcallbacks. SCommonAnimatedSwitcher
  behavior, callback signatures, initialslotidentity and releasepolicy remain unported/unproved.
  RebuildWidget alone cannot be replaced by an invented empty Slate container.
- All above scratch evidence is under /mnt/vibrato/fortnite-work/. Exact raw-call proof is
  icon-text-style-raw-call-decode-1/HANDOFF.md; core/debug proof root-rebuild-widget-failure-identity-1
  and base-rebuild-widget-instance-identity-2. Original CUE pseudocode parser skipped this function
  because it wrongly reads an owner after legacy FFieldPath; raw decoder resolves the call anyway.
  No sharedCUE source was modified. Posthoc investigation snapshots are archived separately.
- Client-event argument contract is now closed at client-event-dispatch-argument-contract-1:
  ProcessEvent buffer48bytes holds source/focus pointers plus32-byte event copy; object-pointer
  dedup gives at mostonecallback perlistener acrossspecific/Undefined collections. Lifecycle/registry
  integration remains open. This is source evidence, not a reason to add a placeholder dispatcher.

## Previous startup-class checkpoint (2026-11-02)

- Current source edits extend the explicit Root probe with --attach / -RootWidgetInstanceAttach:
  actual AddToViewport, three subsequent frame observations with GFrameCounter, then actual removal.
  Existing creation-only mode is preserved. Actual C++ syntax passes in root-viewport-attach-ue-preflight-1;
  14focused runner/verifier tests pass. Full build/attachment execution remain pending; root owns Native.
- Latest validated Native is78c5b61. Original config restores FortGameInstance,
  FortLocalPlayer and FortGameViewportClient; the maintained Athena constructor restores
  PlayerControllerClass=FortPlayerControllerAthena. All4 class paths are now measured in startup.
  The original full game mode hierarchy/constructor and later original CDO overrides are unproved.
- Both builds PASS138.078/112.174s with Native/config inputs stable. Root CreateWidget returns with
  a populated7-widget tree and correct ownership. Its overall check remains failed because22 Native
  imports are unresolved. Actual table421rows/30checks,8retained diagnostics, default/Athena joins
  21.629/21.611s all pass. No Slate attachment, interactive menu, full Blueprint VM or pixel parity claim.
- Controller checkpoint generated/frontend-native-evidence/athena-controller-default-runtime-1/
  checkpoint.json SHA256 a86f92e378125af3271f11a0e613a50242125a784d41a0309c0bb8e015474064.
  Archive /mnt/vibrato/fortnite-evidence/frozen-runtime/athena-controller-default-runtime-1 binds
  3committed sources,81artifact pairs,18binary rows. Release SHA256
  481dcdb0210adef749d7d0344bd91d65b075dfd03c7bce6684c4830ab59ef2a8. All handles TERMINAL:
  builds61218/stage42135/Root26217/controls17992/joins68381/row81034/freeze18988.
  Native/build/stage ownership is released; no live process awaits observation.
- Configuration checkpoint Nativeea68ee4/tools926330e proves generic Engine classes become the3
  original Fort classes with the SAME binary after source/staged config synchronization. It retains
  old config and both runs. Checkpoint SHA2569feb58b5f000ca54012d493e13b94743a66e7ddde95033f456179db24b1b5bf1;
  release0fad26fbbed3af2056a0930a1452277dbd1a3cca42d794c7c0ac42f0bdccef99. Source and stage
  DefaultEngine.ini now match, SHA256e31d783a09241b3da294ecd7d9a7829bc030d7392b713a90fecb2b00571b26c4.
- Reproduce from /mnt/vibrato/fortnite-work/{build_,stage_,run_,freeze_}athena_controller_default*1.py;
  build receipts athena-controller-default-native-build-1, preflight athena-controller-default-ue-preflight-1.
  Config staging tool porting/frontend/stage_native_config.py checks committed bytes and original3
  class settings, preserves prior staged bytes and writes a fresh receipt. Root runner --config binds
  each launch to unchanged ini bytes. Root v2 reports classes and widget count; its parser accepts
  spaces in original widget names (Master Overlay). The first failed parser receipt is preserved.
- Original source proof: athena-controller-default-proof-1/HANDOFF.md and tracked
  frontend/athena_controller_defaults/original-runtime.json. Source image and3hash-only store/literal
  spans verify;4scratch candidate checks, actual C++ syntax,9Root tests,3config tests and47progress
  tests pass. No generated Native source was hand-edited; controller fix is in maintained gameplay cpp.
- Next runtime work: observe actual Slate/viewport construction using the original Root and resolve
  the resulting dependencies. Root is currently CREATED, not attached. The22missing imports include
  authentic confirmation factories and UI/native helpers. Preserve the full4milestone/parity goal;
  all4 remain open. Original config and controller defaults are partial startup progress, not full startup.
- Independent audit athena-controller-runtime-audit-1 verifies3 pinned Git/archive sources and81
  artifact pairs, while explicitly excluding independent18binary-content rehashing. Audit SHA256
  58da5b444409d0196576c8c02d2cad393dc91f24ace79d31219c8f9b9d115df1; copied into archive's
  independent-audit-1. The root freezer itself rehashed all18binary rows before release.

## Previous configuration and row checkpoints (2026-11-02)

- Config/runtime checkpoint is frozen at Nativeea68ee4 / tools926330e. Both builds PASS95.270/91.291s,
  Native and config trees stable. Paired runs prove generic Engine GI/player/viewport become the
  original FortGameInstance/FortLocalPlayer/FortGameViewportClient after byte-exact config staging.
  Root has7widgets;22 Native imports remain. Both joins21.602/21.565s, input table421rows/30checks
  and8retained diagnostics pass. Controller is measured as Engine.PlayerController in both runs.
  Checkpoint SHA256 9feb58b5f000ca54012d493e13b94743a66e7ddde95033f456179db24b1b5bf1;
  release SHA256 0fad26fbbed3af2056a0930a1452277dbd1a3cca42d794c7c0ac42f0bdccef99 at
  /mnt/vibrato/fortnite-evidence/frozen-runtime/startup-owner-config-runtime-1. All handles TERMINAL:
  build29189/stage15447/before57766+53458/after27243/controls81089/joins25191/row4938.
- Current unvalidated controller edit restores one original Athena constructor assignment:
  PlayerControllerClass=AFortPlayerControllerAthena::StaticClass() in maintained FortniteAthenaGameplay.cpp.
  Original proof athena-controller-default-proof-1/HANDOFF.md; scratch candidate tests4 and actual
  C++ syntax pass (athena-controller-default-ue-preflight-1). It preserves the manual game mode's
  parent and behavior; the original full hierarchy/constructor and later CDO changes remain unproved.
  Full builds, actual spawned-class observation and both joins remain pending; root owns Native.
- Startup-owner config source now restores original FortGameViewportClient and FortLocalPlayer,
  alongside the existing FortGameInstance. Byte-exact committed-config staging utility and3 tests
  pass. Root v2 records actual class paths, root-widget identity and widget count;8 verifier/runner
  tests and actual C++ syntax pass. Full builds and paired old/new config runs are pending; root
  owns Native/build/stage. No controller-class default is asserted or changed.
- Validated Native source29a2c24 restores the original CommonUI input-table row and defaults.
  Both builds PASS125.058/119.205s;12707 Native sources stable. The actual UE row probe passes30
  diagnostic rows and loads all421 original table rows. Original row/info sizes1064/168, defaults,
  initialization, type identity and copy-capability flags pass; row-content parity and routing are unproven.
- Root CreateWidget still returns with startup ownership and a non-null tree. Missing-row errors
  fall3->0 and skipped native imports23->22. Its initialization verifier correctly remains failed.
  The frozen v2 counter reports21 because it omitted an import category; the new broad counter
  reports22. Original receipts stay immutable. Eight retained diagnostics, Root class-load control
  and joins21.629/21.588s pass. No interactive menu, Slate/pixel parity or milestone is established.
- Checkpoint generated/frontend-native-evidence/common-input-action-row-runtime-1/checkpoint.json
  SHA256 f4fc348961f36825b3cd059240e634b6ea8914fc8c122443199f63d2866abb56; archive
  /mnt/vibrato/fortnite-evidence/frozen-runtime/common-input-action-row-runtime-1 contains19source
  files,192artifact pairs and18binary rows. Release SHA256
  45875fffea484bd3c73521d67e8859361144a82c1bd26a58a16c5baebdd74010. All jobs are TERMINAL:
  builds71483/stage53521/row68788/Root63756/controls24053/joins73642/class66777/freeze53888.
  Native/build/stage ownership is released; there is no live job awaiting observation.
- Reproduce with /mnt/vibrato/fortnite-work/{promote_,build_,stage_,run_,freeze_}common_input_action_row*.py.
  Build receipts common-input-action-row-native-build-1; UHT/write audit and7 C++ checks in
  common-input-action-row-ue-preflight-1. Three policy,141 generator,32 probe-directory,5 Root-verifier
  and47 progress tests pass. Original image and6 constructor/ops spans are independently hashed.
- Posthoc comparison is /mnt/vibrato/fortnite-work/common-input-action-row-comparison-1.json,
  archived under posthoc-analysis-1. The stricter current interpretation is
  /mnt/vibrato/fortnite-work/common-input-action-row-root-interpretation-v3.json. It is an analysis
  of the retained log, not a rerun or replacement receipt.
- Independent bounded audit common-input-row-runtime-audit-1 verifies19 pinned Git/archive sources
  and384 nonbinary artifact copies. Audit SHA256
  954b9b9cc3e9b88ec8564efb1322d376af73b3e07114c67862c84a4d73ec178b; copies are in posthoc-analysis-1.
  It confirms the421 rows and remaining22 imports; it does not independently rehash18 binary fields.
- Next: fix configuration staging before further UI integration. Stage DefaultEngine.ini has16lines
  and omits the GameInstanceClass already in the17-line project source. Restore original runtime
  Engine class settings FortGameViewportClient and FortLocalPlayer from the extracted original
  DefaultEngine.ini lines416/420, and synchronize/record source and staged config. Original GI class
  is confirmed at line2. Existing logs show object names, not GetClass identities; measure the effective
  classes. Athena PlayerControllerClass remains unproved and must not be guessed. Evidence:
  /mnt/vibrato/fortnite-work/original-startup-owner-class-inventory-1/HANDOFF.md.
- Preserve the full project objective and4 acceptance gates. The dashboard currently estimates
  about21.46%; this table fix does not add a completed input-routing or menu gate.

## Previous checkpoint details (2026-11-01)

- Current row source29a2c24: both builds PASS125.058/119.205s,12707 Native sources stable;
  stage completed. Live handles are row68788, Root63756, retained24053, joins73642, class66777.
  Root owns Native/build/stage until these and freeze_common_input_action_row_runtime_1.py finish.
  Receipts: common-input-action-row-native-build-1; runtime phase generated/frontend-native-evidence/
  common-input-action-row-runtime-1. Do not restart a live handle on an observation timeout.
- Next configuration fix is evidence-backed: the stage's DefaultEngine.ini differs from source
  only by missing GameInstanceClass. Original extracted config at
  /mnt/vibrato/fortnite-extracted-full/FortniteGame/Config/DefaultEngine.ini also specifies
  FortGameViewportClient(line416) and FortLocalPlayer(line420); source currently omits those.
  Owner-class inventory /mnt/vibrato/fortnite-work/original-startup-owner-class-inventory-1/HANDOFF.md.
  Current probe logs object paths, not class paths, so their engine-style names are not type proof.
  Correct and record staging/config separately after the row comparison; no Athena PC class was
  established by that inventory and no controller-class value should be guessed.
- Native6a12acc is built and observed: client/server PASS116.842/111.969s,12703 sources stable.
  The original Root CreateWidget returns with actual startup player/controller, correct ownership
  and a non-null WidgetTree. This does not prove a populated tree, Slate or Blueprint correctness.
  Its load logs23 skipped native imports and3 missing-row errors. The original v1 receipt passes
  creation observations only; the strengthened verifier reports creation_completed=true and
  passed=false with26 dependency messages. No original receipt was overwritten. The separate
  interpretation is /mnt/vibrato/fortnite-work/root-widget-instance-interpretation-v2.json.
  Eight retained diagnostics, Root class-load control and joins21.678/21.631s pass.
- Root instance checkpoint generated/frontend-native-evidence/root-widget-instance-runtime-1/
  checkpoint.json SHA256 8113e837d27e83e6f3761e311b8b25937024b08c8ab954577a715ae773c4cb75;
  archive /mnt/vibrato/fortnite-evidence/frozen-runtime/root-widget-instance-runtime-1 binds9source
  files,161artifact pairs,18binary rows. Release SHA256
  c1bb45ef6b680b2f9e481daaf50e92530a3a119847109d1d52d62ea0914ad278.
  TERMINAL handles: build26720/stage97270/instance30902/controls62843/joins86091/class71002.
  Freeze completed synchronously. Reproduction scripts /mnt/vibrato/fortnite-work/{build_,stage_,
  run_,freeze_}root_widget_instance*; tracked instance runner is under porting/frontend/.
- Current unvalidated edits add the original CommonUI.CommonInputActionData row through a guarded
  standalone type seed, nested CommonInputTypeInfo and recovered constructor defaults. Normal
  generation adds2headers, changes no existing generated files, restores12703 mtimes. The info
  constructor uses Enabled,false,0.5seconds and OverrideBrush.DrawAs=NoDrawType, as proved against
  original constructor stores and brush reflection. No padding is zeroed. Three policy tests and
  generator141 pass; verify_original.py --record common_input_action_row/original-runtime.json
  verifies6 original spans. UHT and all7 C++ checks pass in common-input-action-row-ue-preflight-1,
  with zero writes outside scratch. An initial probe include used its own-module Generated/ prefix;
  the cross-module include is corrected and its failed log is retained beside the passing rerun.
  Actual CommonInputActionRowProbe and its strict runner are integrated but unbuilt/unrun.
  Promotion common-input-action-row-integration-1/result.json; source/constructor proof and candidate
  handoffs under /mnt/vibrato/fortnite-work/common-input-*; root retains current Native ownership.
- Historical confirmation receipt checking now verifies pinned Git source plus frozen copies,
  instead of comparing the old receipt with current working files. The added committed-source
  tamper check passes; all47 progress tests pass. Previous Native runtime baseline remains below.
- Dashboard history now opens on the latest 50 commits, retaining all history, range controls and
  adaptive labels/marker density. A keyboard/touch slider and pointer inspect individual commits;
  the range reports fitted versus zero-baseline scale. Evidence values participate in both scale
  calculations. Root reviewed the bounded Terra candidate and restored an accidental unrelated
  treemap edit. `python3 -m unittest discover -s porting/progress -p test_history.py` passes all 9 tests;
  `python3 porting/progress/progress.py --no-record` renders all 391 existing points; `git diff --check`
  passes. Browser interaction remains untested because no browser runtime is installed. No progress
  weights or milestone evidence changed.
- Previous confirmation baseline is00e127967abad8711a089052ae6eac71c9ff08a3 / Engine d7e04d4b6.
  Confirmation-description construction and two typed native single-cast callbacks are integrated
  through guarded generator policy. Normal promotion changed only FFortDialogDescription_NUI.h,
  restoring12698 unchanged mtimes. Both builds PASS126.507/125.866s with12701 Native sources stable.
  The actual UE probe passes13rows: original size280/align8, native callback offsets248/264,
  recovered defaults, C++ copy/assignment, and real UScriptStruct copy/destruction ownership.
  Root class load exits0 with zero unresolved-bytecode-field warnings. Seven retained diagnostics
  pass; default/Athena joins PASS21.605/21.559s. This does not prove confirmation action/factory,
  queue presentation, original Root construction, Blueprint execution, interactive UI or pixel parity.
- Checkpoint generated/frontend-native-evidence/confirmation-description-runtime-1/checkpoint.json
  SHA256 6be18260996d52ffd1f2fcf43884048a5934dc74d3392d5a195028aa48df5594 binds15committed sources,
  185artifact pairs and18binary rows. Archive /mnt/vibrato/fortnite-evidence/frozen-runtime/
  confirmation-description-runtime-1; source-release.json SHA256
  c7a9541c6ff5b9c7bb313f55d2813349ca1bc8cba0d3ff7c8fac72a20f86ed74 releases ownership.
  TERMINAL handles: builds10226/stage39214/diagnostics67003/Root5278/joins59111/freeze70962.
  Native/build/stage ownership is released; no process is awaiting observation.
- Reproduce using /mnt/vibrato/fortnite-work/{promote_confirmation_description_1.py,
  build_confirmation_description_native_1.py,stage_confirmation_description_native_1.py,
  run_confirmation_description_regressions_1.py,run_confirmation_description_probe_1.py,
  run_confirmation_description_joins_1.py,freeze_confirmation_description_runtime_1.py}.
  Runtime phase needs its parent directory and sources/results/stage receipts copied from build
  before running the retained script (an initial missing-parent launcher failed before any UE run).
  Root command uses env CONFIRMATION_DESCRIPTION_STAGE_RECEIPT pointing to the build's stage.json,
  the tracking-llama-bundle-2/combined-manifest.json and fresh epoch. Build receipts live in
  confirmation-description-native-build-1; promotion in confirmation-description-integration-1.
- Policy/lifetime3, Blueprint-struct4, cross-module3, generator141, verifier18 and runner10 checks pass.
  verify_original.py verifies the immutable image and12 hash-only constructor/lifetime/signature
  spans. Isolated UHT and13 C++ units pass with zero writes outside scratch:
  confirmation-description-ue-preflight-1/{run_uht.py,audit_uht_writes.py,syntax.py}.
  Independent bounded source audit confirmation-description-source-audit-1 found no issues; it
  explicitly excludes live execution and debug-binary rehashing.
- Next recovered contracts: confirmation-queue-presentation-1/HANDOFF.md and
  confirmation-factory-manager-resolution-1/HANDOFF.md under /mnt/vibrato/fortnite-work/.
  The queue paths have different readiness/activation/empty-queue effects; do not stub dependencies.
  Factory suspension reads external FE alone (IsInputSuspended combines FD||FE). GetUIManagerWidget
  casts the director pair object itself to the manager widget; it does not search PlayerWidgets.
- Simple factories/default Confirm/Decline names and action value constructor now have a replayable
  root proof at confirmation-simple-factory-proof-1/HANDOFF.md under /mnt/vibrato/fortnite-work/.
  Original Root creation is now observed above; a populated tree, Slate and initialization remain
  unverified. The original scratch probe candidate is retained at root-widget-instance-probe-candidate-1.
  Inventory
  root-widget-instantiation-probe-inventory-1 has historic load-boundary notes; those old intro/outro
  failures are resolved by the current baseline and must not be treated as current failures.
- Dashboard now exposes four confirmation-family checks within one point split from the existing
  30-point FortniteUI behavior budget (29 remain elsewhere). Only description construction/copy
  is credited, so this family is25%, not complete. Total weight1855,117 leaves, all4 gates open;
  expected overall21.457% after the receipt commit. The pinned importer
  porting/progress/record_frontend_confirmation_description_runtime.py verifies15 source pairs,
  185 archived artifacts, process/log identity and the strict13-row transcript; it cross-checks18
  binary fields without independently rehashing binary contents. Six importer tests and all46
  progress tests pass. Existing source/stage/freezer performed the actual binary hashes.
- Latest confirmation type closure is source aa91a8efab55281d348c07c995bcded98fb414d9, now runtime-validated.
  The existing FortniteUI->FortniteGame dependency is admitted by type resolution; reverse direction remains
  refused. Cross-module struct exports are emitted without relocating ownership. This restores39enums,
  56structs,36delegates and264class properties:131added/188changed generated files, none removed;
  12380unchanged mtimes restored.197properties remain skipped and32types remain unemittable.
  Both full builds PASS218.780/162.101s, all12699Native sources stable. Isolated full UHT and12targeted
  C++ checks pass with zero writes outside scratch. Generator141self-checks and3new policy tests pass.
- Root class load still exits0, now with ZERO unresolved-bytecode-field warnings (all four former dialog
  field paths resolve). Both joins PASS21.619/21.572s. Fresh7retained diagnostics all pass: Panel outro60rows,
  intro49, main-button44, internal-button34, activation40, context registry and lifecycle. These controlled
  diagnostics do not establish Blueprint VM execution, original constructor semantics, async confirmation
  action behavior, interactive UI or pixel parity. No extra behavior/milestone credit was assigned.
- Checkpoint generated/frontend-native-evidence/confirmation-closure-runtime-1/checkpoint.json SHA256
  ea72ddcf52338cb18fe5dde10e7d7d0dc85cf20d1189f75f6dfe5647708f2590 binds321critical committed sources,
  136archive pairs,18binarypaths,bothbuilds,Root and joins; Native tree12699files rehashed unchanged.
  Independent bounded audit /mnt/vibrato/fortnite-work/confirmation-closure-audit-1 verifies source hashes,
  receipts and critical types (does not rehash large debug binaries). Archive source-release.json SHA256
  c5babdd46ec0580ca4f365bb2797938fe263390ece42421cba0063bd9928ced3 at
  /mnt/vibrato/fortnite-evidence/frozen-runtime/confirmation-closure-runtime-1 releases ownership. All handles
  TERMINAL: build40159/stage31498/Root28458/joins24531/retained40593/freeze93788. All workers finished.
- Reproduction scripts in /mnt/vibrato/fortnite-work: build_confirmation_closure_native_1.py,
  stage_confirmation_closure_native_1.py, run_confirmation_closure_probe_1.py, run_confirmation_closure_joins_1.py and
  run_confirmation_closure_regressions_1.py. Build receipts in confirmation-closure-native-build-1;
  promotion confirmation-closure-integration-1/result.json; isolated confirmation-closure-ue-preflight-1.
  /tmp reached its inode limit during an isolated copy; no shared Native mutation occurred. Large scratch
  trees moved to /mnt/vibrato/fortnite-work, with old-path symlinks retained. Use that drive for new large
  scratch trees. Generated headers retain the generator's pre-existing extra blank line at EOF.
- NEXT behavior closure: original native action implementation is NOT emitted yet. Read source proofs:
  /tmp/confirmation-factory-body-1/HANDOFF.md (factory144b1a660/core144b1a460),
  /mnt/vibrato/fortnite-work/confirmation-description-ctor-1/HANDOFF.md (ctor142b1bea0),
  /mnt/vibrato/fortnite-work/confirmation-action-ctor-1/HANDOFF.md (ctor144ae1d30), and
  /mnt/vibrato/fortnite-work/confirmation-activate-slot-1/HANDOFF.md (Activate144ae8b90, callback144afc830).
  Factory transiently constructs the action, casts explicit FortLocalPlayer, initializes Description and
  sets Cancelable=false. Helper14514f020 is GetUIManagerWidget;1407c1790 resolves CommonUISubsystem;
  1407faa40 is the FE input-suspension setter, matching existing bridge NativeSetExternalInputSuspended.
  Action flag+168 records suspension cleared by the factory. Constructor uses real Description ctor and
  leaves both native delegates unbound; Description defaults include Dismissable=true, Cancelable=false.
  Its32unreflected tail bytes are two16byte delegate-like values (+f8/+108), not padding. Activate binds
  the first to144afc830, then calls UI-manager interface vslot+40; interface contract and second delegate
  signature still need recovery before faithful implementation. Callback carries EFortDialogResult byte
  and FName BY VALUE: reflected offsets0/4 match its native parameter buffer; failure is Unknown/NAME_None.
  The earlier guessed constructor1406801c0 was wrong (shared no-op/AddReferencedObjects); correct class
  constructor thunk1451f52f0 calls144ae1d30. Do not add empty handlers or force startup phases.
- Goal unchanged: full reconstructed client/server with feature, visual and wire parity. All four product
  milestones remain open. Dashboard approximately21.44%,116leaves,totalweight1855; materials129/142,
  bounded non-Native signatures41/41. Only the TrackingLlama pending check becamepass this phase.
- Latest validated material/build source3e97743 / Engine d7e04d4b65fb8ee464082849414cf5f7ae4d7bca.
  Both builds PASS151.094/120.596s with all12568Native sources stable. TrackingLlama passes actual indexed
  GT/RT texture lookup, asset sampler/RHI slots, all7shaders and8graph/owner uniform cases each, plus
  4reader negatives. Independent verifier checks14shader payload files and source mip/binding receipts.
  Existing ComicBubble parent also passes the new generic2texture diagnostic, reversed indices and
  asset/shared-clamp samplers. Default/Athena joins PASS21.582/21.572s. No raw compressed GPU readback,
  rendered quad, original pixels or authentic frontend behavior is established by these probes.
- Original Root class load NOW PASSES: generated/frontend-probes/tracking-llama-root-1 exits0 and reports
  /Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C -> WidgetBlueprintGeneratedClass. It still warns
  about unresolved Blueprint fields; loading a class is not executing its VM or showing interactive UI.
  The four paths in Root and its confirmation window are FortDialogDescription_NUI:{Cancelable,ShowSound,DeclineButtonInputAction}
  under /Script/FortniteGame and FortAsyncAction_ShowConfirmation_NUI:DialogResult under /Script/FortniteUI.
  Current action header emits only WorldContextObject/TargetLocalPlayer; standalone dialog struct/enum
  headers were not found. Scout /tmp/root-confirmation-delegate-frontier-1 used older typed-events reports:
  DialogResult delegate FFortConfirmationResultDelegate_NUI was blocked by EFortDialogResult cross-module
  closure; class block146edd640, delegate1451f5b70, enum142b242c0, struct142b25370. Recheck CURRENT generator
  metadata and module dependencies before implementing. Recover exact types/signatures and real action
  behavior; do not add fake handlers or force startup phases. Broader pending native work: main-button
  constructor/Initialize/state/callback graph, Panel constructor/public phase APIs, interface routing,
  queue/global/held-action/input producers, complete original GameInstance startup and interactive UI.
- New ordinary base-UMaterial manifest path validates original proof/reference order/decoded expressions
  and resource SHA, restores exact Texture2D refs, and has an opt-in diagnostic for real RHI bindings.
  Existing fixed owners/Comic handling remain intact. TrackingLlama original Icon-Llama-NoEyes is256square
  PF_DXT5 with9mips (2external/7inline); all3stage files matchoriginal. Initial GPU residency is7mips/64square,
  explicitly observed rather than full residency. Source defaults remain qualified. All7original programs
  translate/reflection-pass without View: MaterialCBV1/span112, texture0/sampler1; fonts add loose t1/s1.
- Checkpoint generated/material_pipeline/tracking-llama-credit-1/checkpoint.json SHA256
  dbc24e995d3a70ae5546d79ef20b45a82b3c7b99d8867231b08fd2e3da1b2df4 freezes61committed source snapshots,
  97artifacts,18binaryrows,bothbuilds,one credited probe and joins. Independent report SHA256
  7ceae38a50ecc724e49316a8957bca14311b7cef1802a9d8fb35047f46bfba97. Root rehashes12568Native files,
  61critical sources and143archivepairs; phase root-integrity.json SHA256
  f192c9d982d5b22841a5a1ff91a6cf44158234c0bb4737c31d48ca09223b3c97. Archive
  /mnt/vibrato/fortnite-evidence/frozen-runtime/tracking-llama-runtime-1/source-release.json SHA256
  021502abf1c5fcda3a66f0a887a746a23bf6c0588e87cffff537a64fdd617382 releases Native/build/stage ownership.
  All handles TERMINAL: build55175/stage44840/material80698/regression98945/root35746/joins18043.
  All workers finished. Four exporter/source rejection tests and57verifier tests pass. Full older
  CommonUI diagnostics and exhaustive original metadata/relocation proof were not repeated this phase.
- Reproduction: /tmp/build_tracking_llama_native_1.py, /tmp/stage_tracking_llama_native_1.py,
  /tmp/run_tracking_llama_probe_1.py material|root <manifest> <epoch> with
  TRACKING_LLAMA_STAGE_RECEIPT=/tmp/tracking-llama-native-build-1/stage.json; /tmp/run_tracking_llama_joins_1.py.
  Use fresh output epochs. Final manifest tracking-llama-bundle-2/combined-manifest.json SHA256
  321c84c9d1fc72c8d65569b931a1cd2a228f90efd8ea7eb805a57805b2962e9b (old122+new1 owners).
  Credit adapter/commands /tmp/tracking-llama-material-credit-1/HANDOFF.md; source context replay and texture
  frame dependencies /tmp/tracking-llama-{texture-context,texture-frame}-1 are copied into the archive.
- Persistent collaboration: root plans/orchestrates/reviews and also implements difficult work. Luna-medium
  small read-only; Terra-medium/high regular; Sol6.1-medium complex; always fork_turns:none. Bounded tasks,
  rotate at useful checkpoints. Economical file/log reads. One Native/build/stage owner; preserve the five
  pre-existing Engine edits and external CUE edit. No remote/push. Keep the full goal active.

## Superseded resume snapshot (2026-11-01, historical)

- TrackingLlama work IN PROGRESS at source3e97743 (not runtime-validated yet). Root owns Native/build/stage;
  full client/server build session55175 RUNNING via /tmp/build_tracking_llama_native_1.py, outputs
  /tmp/tracking-llama-native-build-1. Do not mutate Native or launch another build while live. Code adds
  source-authenticated TrackingLlama exporter/context replay and a general plain Texture2D BASE UMaterial
  manifest PrepareOwner/opt-in GT/RT/RHI diagnostic, preserving fixed/Comic behavior. Source refs/proof/
  decoded fields must match; resource SHA checked before restore. Generic receipt OriginalTextureBindings.json
  requires -FortniteLegacyMaterialGenericTextureProbe; verifier checks exact source frames/mip bytes and
  typed live bindings. Both-target standalone UE syntax checks PASS /tmp/ordinary-texture-candidate-checks-1.json;
  57verifier tests plus4exporter/source rejection tests PASS. Original seven selected shaders convert/reflection
  PASS generated/material_pipeline/tracking-llama-shaders-1; no View, MaterialCBV1/span112, texture0/sampler1,
  font loose ElementTexture t1/s1. Full19source wrappers,5vector/6scalar roots and8numeric cases retained.
  Final exporter bundle generated/material_pipeline/tracking-llama-bundle-2/manifest.json
  SHA32fb2c5334f8758486c018ef7766bb7c13178fa8f86be7c971fa71022a54165b; combined old122+new1
  combined-manifest.json SHA321c84c9d1fc72c8d65569b931a1cd2a228f90efd8ea7eb805a57805b2962e9b.
  Original Icon-Llama-NoEyes256squarePF_DXT5/9mips,2external+7inline; all source mip bytes preserved.
  /tmp/tracking-llama-texture-frame-1/receipt.json SHA0a04773f887a655bff04e6ebf78a6ed1f13e21da309848af20400c37f313d25f,
  8frame rejection controls; all3stagedpackagefiles byte-identical, /tmp/tracking-llama-stage-parity-1.json.
  Inspector sampling/sRGB defaults are qualified, actual sRGB/RHI flags must match at runtime; no pixels
  claimed. Stage when builds pass with /tmp/stage_tracking_llama_native_1.py -> NEW archive
  frozen-runtime/tracking-llama-runtime-1. Runner /tmp/run_tracking_llama_probe_1.py material|root
  <combinedmanifest> <epoch> requires TRACKING_LLAMA_STAGE_RECEIPT=/tmp/tracking-llama-native-build-1/stage.json.
  Then independent verifier/importer and fresh /tmp/run_tracking_llama_joins_1.py; no credit until actual
  runtime succeeds. Prior Comic de38015 checkpoint/source release below remains the validated baseline.
- ComicBubble phase2 source de38015 is independently verified and credited (128/142 material owners). Root reviewed/applied the168line
  maintained Native candidate restoring the proven full2reference context and adding actual GT/RT/RHI
  texture/sampler checks, with real ComicBubbleTextureBindings.json receipts; verifier gains exact
  2texture layout/receipt support. Both actual UE-target standalone syntax checks pass
  /tmp/comicbubble-candidate-checks-1.json; all54verifier tests pass. Both builds PASS141.576/122.258s,
  all12568Native source files stable. Parent/instance probes at comicbubble-{parent,instance}-4 PASS
  33/19graph+owner uniform cases, seven shaders and4reader negatives each,6instance overrides and real
  indexed GT/RT/RHI texture+sampler checks; zero invalidTextureIndex warnings. Source9/11mips remain,
  actual initial GPU residency7mips/64x64 each is measured, not falsely called full residency. Independent
  report_for passes actual14payloads+binding receipt perowner. Default/Athena joins PASS21.566/21.585s.
  Root attempt5 stillfails-11 at TrackingLlama/MemoryImage1064 afterComic passes. No GPU BC1 readback,
  pixel or authenticUI acceptance. All jobs build28302/stage77438/parent58350/instance42983/root45226/
  joins27359 are TERMINAL. Root sole Native/build/stage owner; stage stable. Actual build/stage receipts
  /tmp/comicbubble-native-build-2 and generated/frontend-native-evidence/comicbubble-runtime-2;
  archive frozen-runtime/comicbubble-runtime-2. Same immutable ingress3/combined manifest. New bounded
  phase2 credit audit passed the unchanged strict importer for exactly2owners. Checkpoint
  generated/material_pipeline/comicbubble-credit-2/checkpoint.json SHA256
  68a27bb37520a33719d3704e72cb8aa17be10c5b9726cb73d3f1808f2e5efd70 freezes56committed
  source snapshots,113artifacts,bothbuilds,18binarypaths and joins. Root rehashes12568Native source
  files,56critical sources and167source/archive pairs; root-integrity.json SHA256
  93310cac2ecbe03a267360b5c408b4a7279c46d15d192ffd43ad5e555fe67c08. Archive source-release.json
  SHA2568de360162bbbb72736fcf1ba874611e2569dd123deed1ab6ac3fd22442605fab releases Native/build/stage
  ownership; all workers/jobs terminal. Only2pending owner checks becamepass, components/history/weights
  unchanged; all36progress tests pass. Root did not rerun the full older CommonUI diagnostic suite or
  exhaustive original metadata/relocation proof in this material phase. Source proof /tmp/tracking-llama-texture-context-1/HANDOFF.md now
  establishes original index0 Icon-Llama-NoEyes, all5calledfunction roots onlynull slots/no dependencies;
  includes fresh source inspector replay and source/tool hashes. Its bounded worker completed.
- ComicBubble phase1 PARTIAL: source commit2298bed adds a bounded parent/instance exporter,
  12passing rejection tests and removes the obsolete manifest-wide texture rejection from the maintained
  Native provider. Existing decoder/source digest/full graph/typed binding checks remain. Both builds
  PASS140.718/117.273s with12568sources stable. Parent/instance actual probes PASS33/19graph and owner
  uniform cases, four source-reader negatives each, seven shaders each and both-thread attachment;
  six instance override identities verified. Default/Athena joins PASS21.572/21.570s. No GPU draw/pixel
  acceptance is established. Build77193/stage22138/parent91244/instance17291/joins63801 are TERMINAL.
  Root owns Native/build/stage, currently stable; phase /tmp/comicbubble-native-build-1 and
  generated/frontend-native-evidence/comicbubble-runtime-1. Archive frozen-runtime/comicbubble-runtime-1
  has all six binaries and18source/archive/stage hash rows; formal progress checkpoint audit is pending.
  Final offline candidate generated/material_pipeline/comicbubble-ingress-3/manifest.json
  SHAb80be54d783161148144db694651eab14ef5db7c4734d2a895a0136b541ec963; combined with old120manifest
  entries in combined-manifest.json SHAc0657d1b75e6f363b1280f10d89885b557d187b3e2ea9400bc7564c70810865d.
  Exact14mapped shader chains and10GameTime-only View proofs pass in comicbubble-shaders-1. Every parent
  PS has material texture resources0/2,samplers3/5; fonts additionally have loose ElementTexture t2/s2.
  Instance everyPS has texture0/sampler1; fonts add loose t1/s1. An earlier scout misread loose parameter
  maps as full UB tables; corrected /tmp/comicbubble-binding-audit-1/HANDOFF.md is authoritative.
  Parent has38vector/59scalar/twoTexture roots; instance16/20/oneClouds root and6numeric overrides.
  Both inherit reference0Clouds/1Tail; parent compiled orderTail/source2 thenCloud/source0. All20original
  BC1 mip payloads framed with15reject controls at /tmp/comicbubble-texture-framing-1/receipt.json
  SHA6d58f503f6ff875ae5920025ad81806224c8bb0364f0ff7e11f38d97b56cfdcc. Constructor proves TF_Default
  and default sRGBtrue, tail serializes false; wrap/tailWorld/effective sampler remain inferred; evidence
  /tmp/comicbubble-sampler-defaults-1. Baseline live parent probe failed1 at the exact blanket rejection,
  retained generated/frontend-probes/comicbubble-parent-1 with live inode/argv/cwd+binary hash binding.
  Scratch /tmp/run_comicbubble_probe_1.py supports parent/instance/root, optional epoch and environment
  COMICBUBBLE_STAGE_RECEIPT; /tmp/stage_comicbubble_native_1.py stages only after both successful builds.
  New parent/instance successful runs generated/frontend-probes/comicbubble-{parent,instance}-2. Root
  attempts2/3 pass ComicBubble then fail at TrackingLlama: /Game/UI/Foundation/Modals/ErrorWindow/
  Materials/M_UI_TrackingLlama.M_UI_TrackingLlama entry/notapplicable -> MemoryImage1064, exit-11.
  Both raw observations are retained; attempt3 enables the actual boolean trace flag. All six staged
  cloud/tail package files are byte-identical to original, independently checked in phase/stage-texture-parity.json.
  Independent credit audit refuses both owners: actual logs have invalid TextureIndex(1/0,parent) and
  (0/0,instance), because PrepareOwner hardcodes prior owners and skipsComicBubble; actual references
  remain empty despite valid map/math. No progress/gate credit. Verifier additionally lacks2texture
  layout support and requires a real texture-binding/layout receipt, absent in phase1. Failed adapter
  checkpoint generated/material_pipeline/comicbubble-credit-1/checkpoint.json SHAebe0f9def271decc84831f95a96791acf4c2e50bafe49193a1de98d775939d9c
  is NOT importer-validated. Preserve it and /tmp/comicbubble-material-credit-1/HANDOFF.md. New bounded
  candidate /root/comicbubble_runtime_texture_context owns scratch /tmp/comicbubble-runtime-texture-context-1
  for faithful full2reference PrepareOwner restore and real GT/RT/RHI sampler diagnostic; root alone may
  integrate/build/stage. /root/comicbubble_material_credit now owns only verifier capability+tests for the
  actual new receipt, with no ledger mutation until fresh successful runtime observations. New source
  changes need a separately committed phase2/new builds and receipts; never overwrite phase1 artifacts.
- Goal remains the full reconstructed client and multiplayer server with feature/visual/wire parity.
  All four product milestones are false. Dashboard approximately21.44%,116leaves,totalweight1855;
  an existing CommonUI behavior point now credits independently audited outro/held cleanup, leaving18
  unfinished behavior points. Four bounded behavior diagnostics are separately credited; none is a
  product gate. Source/header allocations remain separate. Material128/142; bounded signatures41/41.
  All36progress tests pass; no historical points/denominator changes.
- Latest full CommonUI diagnostic Nativea99655f487efb11a49fa94822206ac1401fa6785 / Engined7e04d4b6. Client/server
  builds PASS184.454/153.841s with all12568Native source files unchanged. Actual outro60rows, intro49,
  main query44, internal button34, activation40 and registry/lifecycle diagnostics pass in seven fresh
  processes with executable inode/argv/cwd/rawlog/before-after hash binding. Default/Athena joins
  PASS21.576/21.623s. All runs/build session15773/stage15784/diagnostics56843/join-root9974 are terminal.
- Original Root import now passes both OnBeginIntro and OnBeginOutro missing dependencies. Actual next
  boundary: M_UI_ComicBubble_Inst entry -> not applicable -> MemoryImage.cpp1064 DerivedType, exit139.
  Raw logs generated/frontend-probes/panel-outro-root-load-1. No interactive UI/pixel acceptance.
- Interface resolver restores22previously skipped properties in22classes (17Interface scalars,1array,
  4delegate properties) plus2delegate headers, with actual UINTERFACE aliases/header/module guards.
  Manager.CurrentlyHeldActionInputHandler is the real TScriptInterface<ICommonActionHandlerInterface>
  property (originala8/index5/kind13/flags44000000000000/typeref140806da0->block145a00be0).
  Structural U/I declarations exist; original native interface functions/routing remain unported.
- Real native outro flow is now emitted: guarded NativeDeactivateRequest clears held input, invokes real
  BlueprintNativeEvent OnBeginOutro/default EndOutro, THEN CSV Slate Deactivate: %s. EndOutro clears
  introed, calls proven RET completion, clears active and invokes complete Panel deactivation handler:
  remove real input listener, clear held, pre/base/Panel/post notifications. Handle ID remains valid after
  removal as original. Full canonical action record/map and handler data/delegates are emitted, including
  the original unreflected weak popup. No progress-only substitute, fake event body or forced phase.
- Held clear faithfully handles invalid manager/empty row/nonpositive/missing record and NaN duration:
  original COMISS/JAE means !(duration<=0) broadcasts zero even forNaN. After callback it re-resolves the
  weak manager, resets both actual interface words on replacement; second resolve is intentionally
  unchecked as original. Real typed structural-interface fixture verifies both words and swap ordering.
  Controlled C++ ProcessEvent interception is not Blueprint VM/authentic UI, full interface routing,
  native input/manager queue producers, full Panel constructor/public phase APIs or main-button parity.
- Checkpoint generated/material_pipeline/frontend-panel-outro-runtime-checkpoint-results-1.json
  SHA256fe8c7bffce934727e3d1ec3af154839f9cd3361e056613538528257852e52219.
  Evidence generated/frontend-native-evidence/panel-outro-runtime-1; root-report.json
  SHA2562ec8ffb191e6bf18eca7efc868133756213c239e779379de27272844984d96f4.
  Independent integrity-audit-1.json SHA2560d4e3ed4418b60a535f6823935a237b44e239e4650b5e7a7c770397f66daaa0c
  verifies88artifact pairs,53critical committed sources,18binarypaths,12568Native files, seven terminal
  processes and two joins; repeats60/49/44/34/40verifiers. Raw Root stdout+archive independently searched
  for resolved dependencies and actual material/assertion boundary. Context semantic checks were root-run,
  not independently repeated. No new exhaustive original metadata/relocation or material/pixel audit.
  Archive /mnt/vibrato/fortnite-evidence/frozen-runtime/panel-outro-runtime-1; source-release.json
  SHAf383625757cc7f4c740a2223b4074c4df07e8f0f709c0f443142fbbaeec99607 confirms ownership released.
- Source validation: alias3focused tests/141generator, outro34policy/core tests(CSV on/off),24diagnostic
  tooling. Fresh interface UHT/48actual UE source/glue checks pass (interface-alias-ue-preflight-1);
  fresh outro UHT/11targeted checks plus weak Get/delegate/map instantiations pass (outro-map-ue-preflight-1).
  Both write audits show zero outside-scratch mutations. Final shared builds above supersede preflight.
  Normal promotion /tmp/panel-outro-root-integration-1 changes24files/adds7headers/restores12535unchanged mtimes.
- User history graph request DONE, commit6b7c278 in progress.py. Responsive ticks/dots, All/Recent50,
  older/newer/zoom and pointer detail; subset Yscale fits visible overall/measured values with nice ticks,
  All keeps zero baseline. Original380existing values/order/table preserved. Python compile/6history
  tests pass; actual emittedJS V8/DOMfixture checks 1280/320px, varied/flat, allvertices/ranges/zoom/details.
  Served history.html verified with controls and fitted-scale explanation; no actual browser rendering claim.
- NEXT material frontier, read-only scout findings: numeric batch12 report rejects ComicBubble parent+
  instance for Texture expressions (original offsets7837/2294). Instance parent is M_UI_ComicBubble;
  instance has zero texture overrides,2scalar/4vector params. Existing original_texture_context.md proof4
  establishes parent reference order0Clouds/1BubbleTail and shader binding Tail index1/sampler2 then
  Clouds index0/sampler0. Preserve instance layer-before-parent mapping; MaterialLayersParameters tag
  absent in this instance. Frozen context evidence v4 SHA95b4887076fbda3e278d53e8de93a0fc6063932e77e862928de22cd941166c5b.
  Exact artifact paths/receipts are in material_pipeline/original_texture_context.md and
  generated/material_pipeline/frontend-numeric-batch-12/report.json. Next bounded work: needed original
  Texture expression/evaluation/resource binding, concrete parent texture/uniform inputs then instance
  scalar/vector inheritance. No substituted textures/recooks/guessed defaults or claims of known dimensions.
- Other required native work remains: main CommonButton constructor/state/Initialize/callback graph
  (query integrated; RebuildWidget inherited), full Panel public BeginIntro/BeginOutro/IsIntroed and
  constructor, queue/global/held dispatch and input producers. Canonical stage19 and cursor/held candidates
  remain in previous details. Earlier immutable baseline checkpoints are retained: intro2 6ee95ee5...,
  main-query1 2856a0b3..., internal1 c9f4c509..., broader strict material/activation Nativec06a5cb/build2/
  run-only attempt5/tools973d38e cb324a7258.... Their strict reports remain historical, not current-tree claims.
- Persistent collaboration: root plans/orchestrates/reviews and also implements/debugs difficult work.
  Luna-medium small read-only; Terra-medium/high regular; Sol6.1-medium complex; always fork_turns:none.
  Bounded tasks, concise checkpoint then rotate when old context stops helping. Economical file/log
  windows and output. One Native/build/stage owner. Preserve five existing Engine dirty files/external
  CUE edit. No remote/push configured. Goal stays active; no milestone owned by a long-lived worker.

## Previous integration details (2026-11-01, retained history)

- **Latest green runtime: Native `5d45ea2` / Engine `d7e04d4b6`.** Google phase2 passes both builds
  (156.851/130.768s), defaults and combined strict material verification, full context metadata/loaded
  reflection/registry/lifecycle checks, retained material/texture/Splash regressions, and fresh default/Athena
  joins (21.557/21.551s). Checkpoint `generated/material_pipeline/frontend-google-checkpoint-results-2.json`
  SHA-256 `aacafe94827d5d717f5f73650d245e06904b877c7e4f6738c701e281925a4fd8` freezes 202 committed sources, 18
  binary rows and 1,693 artifacts. All jobs ended and ownership is released in
  `/mnt/vibrato/fortnite-evidence/frozen-runtime/google-frozen-runtime-2/source-release.json` SHA-256
  `142e9fee0ede0eb3b900dc04d71fa6afc17ab01bec11079397a7b024682ab374`.
- **Independent review and progress:** root reran both strict Google verifiers and the complete context
  verifier; a separate checksum audit passed. Aggregate
  `generated/material_pipeline/google-runtime-root-audit-2.json` SHA-256
  `0768befa2f3e39cf1b60546152edd4343ae6e92ee3df41522fa6a168e0d4b5d7` binds those reports. Integrity audit
  `generated/frontend-native-evidence/google-runtime-2/integrity-audit-1.json` actual SHA-256 is
  `424df964f0dec72e4966c02b5e8caa4435f4763f929337c95b636095e978c6ec` (use the file hash, not the worker's
  mistyped final message). The existing Google ledger key is now pass: **126/142 owners, 113 leaves, total
  weight 1,855; all four milestone gates remain false**. The narrow importer preserves denominators and
  historical points; all 19 progress tests pass.
- **Actual remaining load boundaries:** the latest run first encounters Root `M_UI_ComicBubble` and Login
  `M_TempIcon_Login_Facebook`, then MemoryImage.cpp:1064. Native `OnBeginIntro` remains unimplemented;
  asynchronous load order changed the first observed boundary. Material diagnostics establish neither
  interactive UI nor original-client pixel parity.
- **Failure preserved:** Google phase1 (`eefffa9`) failed before provider registration because the bundle
  omitted mandatory `parent_path`. Its checkpoint and independent audit remain immutable. Producer correction
  `5281c38` and bundle pin `5d45ea2` add the explicit empty parent without relaxing the loader. Corrected bundle
  SHA-1 `a519120049649f3c0835b5e341e27778aa4ee781`, SHA-256
  `b4f58ba6519eec121e32757c60bf9611c1aca78cbcec0c861af2155351f9e327`; fixture/raw texture/19 records/7
  programs/10 mips are unchanged. Details: `material_pipeline/GOOGLE_TEXTURE.md`.
- **Activation source integrated; runtime unverified:** `/tmp/activation-base-rebase-1/HANDOFF.md`, receipt
  SHA-256 `e6ea0cff9759b6b5724225d1babd60b1d326122ea2b7ce5d44c904a2c37ab699`, patch
  `10da4d072ac6b4d013381621a7719ad27539f71544a3cba1859b9043e5d5bdf0`. Generator `9cee697c…` → candidate
  `1a1b529e…` preserves GetContext. Full generation changes only the activation widget CPP/header and two helper
  headers; portable, policy, isolated UHT and actual CPP syntax checks pass. Root reviewed
  constructor/guards/reentry notification order, applied the exact source patch and regenerated through the
  normal promoter. Root generation changed only the expected two widget files plus two helpers and restored
  12,568 unchanged timestamps; 141 generator tests, seven policy tests and ASan/UBSan core controls pass.
  `/tmp/activation-root-integration-1/source-review.json` records the bounded review. The exact namespace-fixed
  opt-in probe is also applied; all six expected context source hashes match. Superseding probe preflight
  `/tmp/activation-probe-preflight-2/HANDOFF.md` fixes unity namespace collisions and passes full current
  FortniteGame unity syntax. Fresh bounded `/root/activation_link_proof` finished the superseding verifier in
  `/tmp/activation-link-proof-3`, receipt SHA-256
  `15b4928cc2363bf64845b36a09c28182f17ab111793461e55593e97703ae4e4d`. Root reviewed the exact relocated
  implementation/thunk-byte and link-recipe checks and independently reran all 18 verifier tests. Exhaustive
  hermetic loader closure is an explicit limitation, consistent with existing context evidence. Source
  integration is committed as `c06a5cb`; it is not a new verified runtime. Activation phase1 stopped before any
  build because the tool normalized the header comment but missed the CPP comment's fixed second sentence.
  Failure checkpoint `generated/material_pipeline/frontend-activation-runtime-failure-checkpoint-results-1.json`
  SHA-256 `71c3a3f15a69e9311107f09d9b6c5585205045c00396430da393027463522b9b`; explicit release SHA-256
  `e1989f629093fd543783cbd3b02d263e349f7470aed7eb8d72cae00f3eb005e2`. Source snapshots and prior intermediates
  are immutable under `/mnt/vibrato/fortnite-evidence/frozen-runtime/activation-frozen-runtime-1`; no
  compilation/runtime/credit. Root corrected the exact comment matcher and tested both actual files plus
  body/filename/fixed-sentence mutations; all 19 tests pass, including actual full source preflight. Candidate
  `/tmp/activation-source-label-fix-4/receipt.json` SHA-256
  `7e5552ca311fa9b2df0c7526174d3226a5ff2578f40d1652ca797246d0631827`; maintained tools are now
  `frontend/activation_runtime_probe/`. Phase2 reused prior archives and preflight passed; both builds pass
  (196.381/192.277s), binder finish collected both targets, separate context processes each exit0. Exact
  activation argv omitted the known working project/map/Vulkan/unattended/compatibility launch baseline and
  timed out after240s without a transcript. Stall cause is not proven; no strict/retained-regression acceptance.
  Failure checkpoint `generated/material_pipeline/frontend-activation-runtime-failure-checkpoint-results-2.json`
  SHA-256 `2f6ae1bee29f1579b1dc90af3f599f1f70881cc1390dd19b02e2af89a856b4a4`; explicit release SHA-256
  `2a457d888601347836aca5dbe23361987fb31c887b0a9a291fbd209596766c2e`. Both builds, six immutable
  binaries,42relevant metadata/object snapshots and27runtime artifacts are preserved under the phase2 archive.
  Fresh bounded `/root/activation_resume_launcher` finished the fixed baseline argv and validated run-only
  resume, receipt `/tmp/activation-resume-launcher-5/receipt.json` SHA-256
  `61e35c10ec3f0da897c3311806f6d1f105504f3204b6979bf48776d3c6b3fb9f`. Root reviewed and promoted the tools; 25
  tests and a real-input resume preflight pass. Retry mode validates exact parent/checkpoint/build/tool/source
  references, retains the original compile epoch and requires new context/activation processes after its retry
  epoch. Build/restamping is prohibited in that mode. Large binary/debug hashes are checked once at resume and
  in the unchanged final strict proof, with executed binary hashes checked before/after each process. No fresh
  compilation or runtime pass follows from these tool checks. Attempt3 ran corrected baseline argv: all three
  processes exit0, activation emits40passingrows (root independently verified exact transcript), and full
  context strict report passes SHA-256 `eecc1e7b4fdfdf9674511cc2ee34c128a81d6e7bd0d80a7f8093b8ddf1a02778`. Its
  activation metadata observer then incorrectly matched both the IsActivated UFunction constructor and bool
  setter endingEPv. No full activation/regression acceptance. Failure checkpoint SHA-256
  `1f4d0d041d7c7404d007348d508adb974bee2422e9efa47698d87bc5d7a40488`, release SHA-256
  `4c3941ef0563d25606f24a04fbbf31497129975da4240839176f5bd2e9bd043f`; allprocessesended. Root corrected the
  complete constructor-name selector, explicitly retains the original historical metadata observer hash for
  inherited build receipts, and separately binds the current corrected observer. No metadata/object/ELF checks
  are dropped. New tests use the actual ambiguous symbol pair and reject missing/duplicate constructors.
  Attempt4 preflight then exposed a second broad observer selector matching the base registration plus
  Queue/Stack/Switcher/ContainerBase. Failure checkpoint SHA-256
  `5f1b15ee79e5dcf977d17a8241ebc451b548d34882e5687dbf133061538cf84c`, release SHA-256
  `ecdab7bacc3acbd960c53329fbe99149ef1989d3319d4c37771ab82853a5919a`; no runtime launched. Root narrowed the
  complete registration member name and added the actual five-class regression control (28testsPASS). Before
  another run, root directly audited BOTH real inherited targets with the corrected metadata and unchanged
  implementation/link tools: client PASS `/tmp/activation-actual-metadata-preflight-2/FortniteLinux.json`
  SHA-256 `424d1d2a022c67bf24e36dbb6815e3e1fd47a2cbe70e24201427faa003b27dc4`, server PASS
  `FortniteLinuxServer.json` SHA-256 `dc60aec52744c22350757809d768dbf273633d7539a0409f8f34f1e673e93466`. These
  are actual artifact observer preflights, not full runtime/regression acceptance. No Native/generator/Engine
  source change for either launcher correction.
- **One held-input method:** earlier `/tmp/panel-held-input-method-1` stopped at its first partial checkpoint;
  its follow-up errored without further results. Fresh `/root/panel_held_method_finish` finished and released
  its one-method checkpoint under `/tmp/panel-held-input-finish-2`; receipt SHA-256
  `2266523bb7e568a39a249d91397694241b2b6e5ed303a7c525b0ce7b29c6c882`. Real typed held wrapper and Panel/Global
  cancellation adapters remain scratch-only. ASan/UBSan controls and UE clang syntax pass using declaration
  fixtures/prior compatible UHT, not fresh UHT or the actual Panel hierarchy; manager implementation/composition
  remains pending. Earlier failed syntax logs were overwritten by that worker, explicitly disclosed in its
  handoff; those failures are not available for independent replay. Resolved cancellation proof
  `/tmp/held-cancel-contract-1/receipt.json` SHA-256
  `283f58b64bf97f693006cd7eee93f2f787d9a62ba593ab23662d2d5aa11af711` permits project-local
  `NativeCancelHeldInput(int32, const FCommonInputActionData&, uint64 Unused)` passing zero. Both known
  implementations ignore the fourth register; the carrier is equivalent, not uniquely recovered original C++
  spelling. Global +30 is `140800b70`, distinct from cleanup `1407ae750`; Panel +30 is `140800860`. No whole
  Panel/input/runtime claim.
- **Cursor remains a source checkpoint:** `/tmp/native-cursor-warp-1` is not promotable while its clamp helper
  is undefined. `/tmp/cursor-window-layout-control-1` connects original paired weak setter `14422eaf0`, sole
  direct caller `1441e70a4` and subsequent layer-manager call `14422d120` with Engine CreateGameViewportWidget.
  Compile-control offsets corroborate Window d0/d8, but other layout offsets differ. Freeze the
  structural/dataflow proof before using the guard; original cursor producers remain unwired.
- **Persistent collaboration:** root plans, integrates and reviews, and performs difficult implementation
  directly. Small read-only tasks use Luna medium; regular bounded work uses Terra medium/high; complex bounded
  work uses Sol 6.1 medium, always `fork_turns: "none"`. Keep reads and output economical. Give agents one
  function/task/investigation, checkpoint and finish when context stops being useful, then rotate; never assign
  a whole milestone or tile group. One owner controls Native generation/build/staging. Preferences are saved in
  global, workspace and project AGENTS.md and must survive compaction.

- **Pending CommonUI behavior accounting:** one existing point of the 22-point CommonUI behavior budget now has
  the objective `frontend_commonui_base_activation_runtime` ledger check; 21 points remain for
  Panel/queue/input/buttons/other widgets/integration/parity. Source-stage points remain separate. The check is
  absent/pending until the complete fresh runtime phase plus independent audit passes; no new credit, weight
  inflation or gate change.
- **Additional bounded source checkpoints:** `/tmp/cursor-viewport-clamp-2/HANDOFF.md` closes the previously
  undefined clamp helper off workspace, receipt SHA-256
  `c7c0d3c71cc21010f38c0cd26a68d20a5a5be74145b053bb3c52cbbeef71d649`. Root verified all 13 artifact bindings and
  reviewed actual Geometry AbsoluteToLocal/LocalToAbsolute render-transform semantics and original ordered
  comparisons. Actual UE syntax and mock guard/affine/NaN/small-size controls pass; four relevant original
  windows match across old/late captures. Cursor producer wiring and runtime remain unfinished. Narrow
  `/tmp/button-internal-interaction-map-1` binds the internal interaction bool to +461; its caller weak-resolves
  then calls unconditionally, so do not invent a null guard. Fresh `/root/button_slate_interactable_target`
  finished the exact construction/vtable/getter binding: actual SCommonButton vtable1459f15d0 +1c8
  targets1407cf560, reading interaction5d9 and28bit0, not5d8. Root independently replayed its13windows then
  traced the actual base constructor1430824d0 ->basevtable14665bcf8 ->samepredicateSlot1407cf580, a7-byte28bit0
  getter. Maintained `frontend/inspect_common_button_slate_interaction.py` now freshly passes21original
  windows/2,060bytes. Root followed base mouse-enter/leave through calls that set/clear the queried28bit0; it is
  hover state. The typed adapter is `bInteractionEnabled && SButton::IsHovered()`. The earlier
  IsInteractable/IsEnabled label was an incorrect source-correlation hypothesis and has been corrected; no
  Native implementation used it. `common_button_slate_interaction_evidence.md` records limits; no widget
  implementation/runtime credit. Root bounded vtable comparison finds14distinct slotaddresses
  includingdestructor across77inspected slots; full SCommonButton overrides still need source work.

## Earlier snapshot (2026-11-01, retained history; latest state above)

- **Google Native phase1 failed and explicitly released:** exact reviewed manual Probe applied/committed as **eefffa9**; both fresh builds pass153.04/130.36s and unchanged provenance passes fresh both-target UHT/genCPP/object/ELF/debug. Google defaults exits1 before provider registration: pinned v4 bundle lacks mandatory explicit empty `parent_path`, rejected by existing general `LoadBundle`. No texture/shader/frame acceptance; combined and all dependent regressions stopped, no correction or rerun. Failure checkpoint `generated/material_pipeline/frontend-google-runtime-failure-checkpoint-results-1.json` SHA-256 `e99774d30ab116dae528b2993bfb4134f5b84a57616dd7d8b93956a121b96f1f` binds202 committed sources,18binaryrows/sixMNTbinaries,1,126inputs/1,099artifacts. Explicit release `/mnt/vibrato/fortnite-evidence/frozen-runtime/google-frozen-runtime-1/source-release.json` SHA-256 `86c56c27e8923109978d4bb6525516e15a228cc3b6a73d1ff7299895e47b37c0`; all jobsended and source/build/stage ownership released. Exact five original/staged asset hashes and diagnosis are in `material_pipeline/GOOGLE_TEXTURE.md`. All18Google/23context tool tests pass, but actual context/regression/joins were unattempted after failure. Prior95ff94f phase3 remains fullygreen; noGoogleowner/milestonecredit. Next retry requires a separately reviewed producer-contract correction/newbundlepins/newphase; preserve frozenphase1.

- **User dashboard request completed:** audit found completed source stages hidden in broad manual-zero UI leaves. Three one-point source allocations plus a one-point generic runtime share now sit beside the unfinished lifecycle/CommonUI behavior shares; total weight stays **1,855**, leaves become **112**, bounded material owners **125/142**, event signatures **41/41**. At identical HEAD/local inputs the revision moves effort **21.010% →21.172%** for source stages, then **21.172% →21.226%** for independently verified generic context runtime (+0.054 points), explicitly model corrections with historical points preserved. All four acceptance gates remain false. Material inventory keeps its two-point share within the separately allocated 20-point material pipeline budget; no hours-based inflation. Combined blue fill plus square markers remains implemented.
- **Objective import:** `python3 -I porting/progress/record_frontend_source_stages.py` pins three receipts and rehashes **46,166 frozen bindings** (31,105 generic context,14,995 activation base,66 consolidated input). Context is root-reviewed generic source/generator; activation/input are bounded isolated candidates. No shared integration, concrete context, Panel/global/held completion, runtime or playable UI credit. `python3 -m unittest discover -s porting/progress -p 'test_*.py' -v` passes **13 tests**, including tamper/missing-byte rejection, preserved weights and historical points. External evidence availability remains required; weights remain estimates.
- **NEW fully verified runtime is phase3 (95ff94f), aggregate integrity audit passed:** source **95ff94f / Engine d7e04d4b6**, full strict context report `generated/frontend-native-evidence/context-runtime-3/report.json` SHA-256 `4b6760fca3ef743caaae3c081d870864500d6b3c7c2ea3a33db8c435c074f5b7`. Root independently reruns the unchanged promoted verifier and gets PASS in `root-report-1.json` SHA-256 `ab5e924073e9559675093f9f8aabeb6f1cb5346caaed267f51713deda24713cb`: both-target raw UHT/object/linked ELF, exact three measured loaded-property rows and real registry/GC/cleanup/travel/copied-map observations. Both builds158.267/134.597s,12 retained cases/11fixedowners/10textureprofiles/8strict texture reports/Splash3keys/fresh joins21.573/21.604s pass. Root still stops at OnBeginIntro; Login at Google then MemoryImage1064. Full checkpoint `generated/material_pipeline/frontend-context-checkpoint-results-3.json` SHA-256 `22b6b87c942044d7f46067a660e1f1e25e289262810fc52cf878d868f88ca8e6`:122 committed sources,18binaryrows/siximmutablebinaries,872inputs/1,416artifacts. Separate explicit release `/mnt/vibrato/fortnite-evidence/frozen-runtime/context-frozen-runtime-3/source-release.json` SHA-256 `6ee7c1c6dbf95cb60057fa7b9ab6ac14e775d8bec922605c899254caa9d79a48`; all jobsended. Fresh bounded Luna-medium `/root/context_phase3_integrity` finished: all122 committed source blobs/1,416 artifacts/18 binary rows match, release flags and root strict report pass. Its unchanged report is preserved at `generated/frontend-native-evidence/context-runtime-3/integrity-audit-1.json`, SHA-256 `d09c52d9d05333a680c40c6e000c9fa1b641f49333d7a4c66a1fe535a8a43828`; original /tmp report remains. No further runtimes/builds. All4milestones remain false; noauthenticstartup/concretederivedcontext/UIparity claim.
- **Generic runtime progress importer reviewed:** `record_frontend_context_runtime.py` validates pinned phase3 checkpoint/full strict report/independent integrity audit and binds source, binaries, actual metadata, loaded properties and runtime cases. Five ledger checks share one existing lifecycle point; lifecycle manual share12 +generic source1 +generic runtime1 preserves14. Current112leaves/1,855weight/**21.226%**; all4gatesfalse. Root moved the audit pin from ephemeral /tmp to the identical ignored generated copy and fixed the older source-budget test. All16 accounting/history/material tests pass. No source candidate or observer-only evidence substitutes for the complete phase.
- **NEW persistent bounded-agent rule:** use fresh agents for bounded functions/tasks/investigations, not long-lived whole milestones or tile groups. Follow-ups are allowed within a relevant bounded task; checkpoint and rotate once old context is no longer useful. Saved alongside models/inputefficiency in global `/home/vibrato/.codex/AGENTS.md`, workspace `/opt/AGENTS.md`, project `AGENTS.md`; preserve allthree preferences in compactions/handoffs. Long-lived build owner has finished/released at phase3; CommonInput owner has checkpointed/finished at stage19; Panel owner is finishing a safe checkpoint. Root retains overall integration/review. Fresh bounded Terra-high `/root/cursor_warp_functions` has finished at `/tmp/native-cursor-warp-1/HANDOFF.md`, receipt SHA-256 `87e96478bbf4df875b0c2466241ac7787a7e88707d15febd4c03a10dc4d10f35`: writer/outer recovered and syntax checked, but viewport weak field+d0 type is unresolved and clamp helper deliberately undefined; producer callers unwired. NOT promotable, no source/runtime claims. Next bounded investigation is that exact field identity. The fresh medium-Terra runtime-ledger agent also finished after its one import/model task.
- **Source handoff for fresh bounded tasks:** CommonInput/global canonical unpromoted source `/tmp/common-input-native-dependencies-1/frozen-global-owner-stage-19/source/porting`, receipt SHA-256 `2d7fc7e78e9a4fce46b139556f1c9895a6255f4d02ea673d1dd12e47f1ff5c6b`. Real globalowner bodies/record/interface were emitted; fresh UHT15 and actual owner CPP/genCPP syntax pass with declaration-only Button getter and manager route fixtures. Panel held/immediate slot+10 contract, real Button/manager composition and cursor functions remain. SharedGhidralease released; processcheck before reuse. Oldworker is completed; do not extend its broad responsibility. Root button state proof `/tmp/common-button-native-state-root-1/receipt.json` SHA-256 `a86f4f4ce81cb165911c3633006364055cab9cde08a3aefdaecc012faef95510` pins77windows/default06/typedSetIsEnabled override and full537-byte ctor/230-byte setter. Source notes/replay tracked under frontend/common_button_interaction_state_evidence.md and inspect_common_button_state.py; full internalSlate/visual/setter behavior still requires porting.
- **Google offline candidate independently reviewed, unapplied:** root verifies89frozenbindings,18producer/verifier tests, and a fresh byte-identical originalcollector replay. Audit `generated/material_pipeline/google-root-review-1/root-audit-1.json` SHA-256 `c57958fae642a4d8d729ccafdbeebd2c84a6947330ebc137efd6ab30accb85b6`. Source remains `/tmp/google-icon-runtime-candidate-1/manifest.json` SHA-256 `5fd3499066be1283948cff7a70f851bb82ab1bd8e74d063335f7448c776488b7`. Before Native promotion, copy reviewed tools into material_pipeline with final maintained paths, fresh replay/fixture+bundle hashes and matching verifier/Probe pins; relocating source provenance can change hashes. Nextphase needs a fresh bounded owner, not the retired long-lived agent. NoGoogle runtime acceptance or new material count yet.
- **Latest context phase2 is failed and explicitly released:** source8eb2037/Engined7e, both builds pass158.171/202.011s and both-target raw UHT/object/linked ELF metadata passes. All observer-only registry/GC/deinit/travel/copied-map checks pass, but **reflection_return/class/object FAIL**, so full phase remains false; dependent material regressions/joins were stopped. Original pair also duplicated Unix console/stdout logs; a source-proven NEW pair using only added `-NOCONSOLE` preserves single-stream output and exposes the same three failures. All processes ended. Checkpoint `generated/material_pipeline/frontend-context-runtime-failure-checkpoint-results-2.json` SHA-256 `25da1a1ef8f6e79e3c87f19d3584f29d3f1bf98f651c4481d4ae4a0633c089f9`:118 committed sources,18 binary rows/six immutable binaries,908 artifacts. Root independently hashes all and validates committed blobs in `generated/frontend-native-evidence/context-runtime-2-root-failure-audit-1.json` SHA-256 `11dd65817525ea3db55471b6a9aea819bbd7d16d4dcf549d570232179c267aca`. Separate explicit release `/mnt/vibrato/fortnite-evidence/frozen-runtime/context-frozen-runtime-2/source-release.json` SHA-256 `1ae431e2d8db468437336cbd020fb414576c806a5d102336440c3a09b8173451` supersedes pre-release flag.
- **Correct diagnostic expectation found in original code:** constructor dataflow adds exact computed type flags **0008001040000200** to raw FPropertyParams flags, independently matching Engine TProperty behavior. `frontend/inspect_context_property_computed_flags.py` replays25 exact original windows and all3descriptors from the late image; no heap/original process access. Frozen proof `/tmp/native-context-computed-flags-proof-1/receipt.json` SHA-256 `5205b8f3cb5b600ad5ed1039ca5019127a4de6dd375d35a6bb6243cafaf70ef1`; source notes in frontend/context_property_computed_flags_evidence.md. V6 harness candidate under `/tmp/native-context-runtime-probe-candidate-6` adds exact typed computed-bit expectation and measured-field rows plus strict row verification;23 tests and bounded fresh-phase2-UHT syntax pass. Root preliminary review agrees; final manifest/patch freeze and authorization still pending. **No V6 source/tool application yet**, no new runtime acceptance, raw original metadata gate unchanged.
- **Applied context source and preserved failure:** **32e9dab** applied generic GetContext and v4 diagnostics. Fresh client UHT passed, but the client unity build exits6 after121.977s with15 `-Wshadow` errors caused by diagnostic file-scope objects `Start`/`Stop`. No new linked binary, server build or runtime accepted. Failure checkpoint `generated/material_pipeline/frontend-context-build-failure-checkpoint-results-1.json` SHA-256 `fadc287e50f996ede3bfcd70e70996964d10b19feb7b0d97d029cff671a128db` preserves103 committed sources,122 artifacts,five compile snapshots. Root independently rehashed all and checked committed blobs. Separate explicit source release receipt `/mnt/vibrato/fortnite-evidence/frozen-runtime/context-frozen-runtime-1/tool-copy-and-source-release.json` SHA-256 `099ecdbb75e0944f69a6dc299443528a5dd2505183da637856161df050516540` supersedes the checkpoint's pre-release flag. Previous fully green runtime before context work was **8f87ade / Engine d7e04d4b6**; phase3 supersedes it above.
- **Reviewed v5 retry:** `/tmp/native-context-runtime-probe-candidate-5/receipt.json` SHA-256 `26f561e7ec578d844a6636cb6fd6ad68b7a44846706834d61fb2e82be118c175`; `unity-command-names.patch` SHA-256 `0b7d7dd7a61f0c55574e6458758a3a1bf2ab9b8f33b51188f21bbfe66e8c6f96` changes only those two C++ object identifiers. Probe after-hash `65db3b9df0d5aa71b50580d45fee4d4a354ff5a895a2855bcf12b802405ece0d`. No handler/string/other-source/whitespace change. Sole Native owner `/root/donut_texture_engine` applied the tiny correction as **8eb2037**, froze118 committed build/tool sources, and is running the NEW context-runtime-2 phase. Client build passed158.171s after fresh UHT; server build is live (PTY5560/provenancePID2432628→UBTPID2433043). Acceptance still requires both fresh targets/UHT/object/ELF, actual registry/lifecycle, retained regressions/Splash/joins. Never overwrite failed phase1 or weaken gates. Inspect owner/job state before doing shared work.
- **Versioned tools c66e0ca:** `porting/frontend/context_runtime_probe/` now tracks strict provenance/event/object/ELF verifiers, six source hash expectations and tests. All **16 tests** pass. Initial copied test run failed because the package lacked its archived manifest helper and expected an off-workspace after/ layout; root included the helper and mapped source contracts to repository paths, then reran successfully. Exact local original candidate/root-audit inputs remain pinned outside Git. V5 tools are frozen separately; no runtime success from synthetic tests.
- **Full original GI Init recovery reviewed and saved:** `generated/frontend-native-evidence/late-gi-init-recovery-1/receipt-2.json` SHA-256 `975df3b2295593d607f0bd799c2f18c897fa0dfc18cca10e60d809ab59b1d821`;1,225 windows/5,702 reachable instructions/149 proven conditional dispatches/334 calls/zero unknown CFG frontiers. New late image proves real Init→gated InitializeUI plus paired PreLoadMap/PostLoadMap/Shutdown registrations; full source has more account/player/service work. Applied the actual `generated/frontend-native-evidence/late-gi-init-recovery-1/candidate-integration-2.patch` (the earlier /tmp patch path was stale). Root verified153 frozen inputs/125 outputs/51 helper windows and fresh full replay plus8 tests; independent `root-audit-1.json` SHA-256 `650d8722bd7c58a5a7ad26d402a503b57b82c129b2cb51a758844d17ca8a028a`. Inspector/test/source notes are now tracked under frontend/. Transitive implementation is not ready; no product credit from the proof alone.
- **Root closed another authentic Init source dependency:** original UpdateManager+40 / callback141e70c30 is **OnUpdateCheckComplete(EUpdateCompletionStatus)**, not the other EUpdateState event. Original singleton/FEnumParams independently binds all9 scoped names/ordinals; original LexToString switch matches all9cases. Actual isolated UEClang9/API control gives status getter this+40(decimal), completion this+64(decimal=hex40),24-byte multicast, exact enum/signature static asserts. Receipt `generated/frontend-native-evidence/gi-update-completion-1/receipt.json` SHA-256 `986000b09b9f28163f793f6786d0502ebd01ae6f199dd2d91ae2cc2e72f42d25` freezes1,483 bindings (1,468 successful readonly compiler input files) and7 outputs; root rehashesall and3negative tests pass. New tracked inspector/test/source notes in frontend/. No original process access/shared Native or Engine mutation/UHT generation/link/runtime; full GI callback service/player body still needs porting.
- **Ongoing source workers:** `/root/authentic_gi_init_control_flow` owns off-workspace Panel/base/queue; `/root/common_input_native_dependencies` owns input/global/held dependencies. Input latest bounded stage7 receipt `/tmp/common-input-native-dependencies-1/frozen-global-weak-stage-7/receipt.json` SHA-256 `138e3dd5ffb4b7bb131cb30799d5b8c89bffa342090810dc7cd5c10a78e9c882` includes fresh isolated UHT10/syntax. Held records/listener adapters and portable global registration/held-dispatch behavior have advanced through bounded stages8–13; exact public registration type UUserWidget and public Priority sort field are proven. Full combined UE global/interface/action integration and cursor warp remain unfinished; ask the dependency owner for the latest consolidated receipt. No shared source application from these candidates. Coordinate Ghidra lease directly.
- **Persistent input efficiency (user instruction):** use bounded head/tail/matched excerpts, capped searches/log output, and scripts that summarize large generated files. Expand only when necessary; full reasonably sized files are allowed when needed. Saved alongside delegation in global `/home/vibrato/.codex/AGENTS.md`, workspace `/opt/AGENTS.md` and project `AGENTS.md`; preserve both preferences in all handoffs/compactions.
- **Persistent delegation:** root plans, orchestrates, reviews and directly implements difficult work when useful. Small read-only **gpt-6-luna medium**; regular **gpt-6.1-terra medium/high**; really complex **gpt-6.1-sol medium**; always **fork_turns:none**, max3 active workers. The requested dashboard Terra-high audit is finished. Keep Native/build ownership singular. Preserve prior Engine/CUE edits, original extraction, keys and all historical failure receipts. Launcher/EAC/entitlement compatibility stays in scope; no remote/push. Goal active, all gates unfinished.

## Earlier snapshot (historical, superseded above)


- **Goal/dashboard:** all four milestones remain false. The bounded ledger now credits **125/142 material owners** and **41/41 non-Native frontend signatures**; total weight **1,855**, **108 leaves**, lifecycle/widget/visual/gameplay measures remain unfinished. Combined treemap fill/squares remains implemented and verified. Import changes have passed the five history/model and four material-accounting tests; no denominator or milestone change.
- **Latest fully verified runtime:** Native **`8f87ade`**, Engine **`d7e04d4b6`**. `generated/material_pipeline/frontend-media-social-checkpoint-results-2.json` SHA-256 `d59f65a8e332eae196159281495c245e4cfefea6943482d86416a08ddcea39be`: 95 committed source snapshots, 18 binary rows/six immutable binaries, **1,388 artifacts**. Both builds pass **137.689/117.810 s**, fresh default/Athena joins **21.629/21.601 s**. Six numeric regression owners, all 11 fixed owners, all ten texture profile runs and actual Splash keys pass; opt-out failure retained. Root independently verifies every committed source/binary/artifact, all eight strict/root reports and actual runtime markers in `frontend-media-social-root-checkpoint-audit-1.json` SHA-256 `01f995348590e1bf33230a3c0dad741521818ef0204949b4134c150bfb7ff9fe`. Phase 1 failure remains immutable. All phase-2 jobs ended and **sources explicitly released**; no later shared Native implementation is applied yet.
- **New owner evidence and actual blockers:** fresh root reports `generated/frontend-probes/media-social-owner-probes-2/{social-defaults,social-combined,media-defaults,media-combined}-root-validation-1.json` verify original GPU bytes, all seven shader payloads each, source/copy equality, Social direct-root bits plus five VM controls/15 rejected forms, correct Media world-clamp binding/no named proxy. Default checker delta0 is observed; combined deltas are **4,096 Social / 3,892 Media**. Original atlas World LODGroup remains an explicit inference. Actual Root now stops at missing **CommonActivatablePanel.OnBeginIntro**; Login advances to **M_TempIcon_Login_Google**. No interactive UI, original pixel or gameplay parity claim.
- **Important NEW original source input:** root executed the original client in a NEW copied Wine prefix and private local-only namespace, with **FN_PATCH_VERIFY=0**, no process patcher, read-only `/proc` memory capture, no ptrace/stop/write. Both launch/capture finished. Artifacts: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/`, five full images and 12 target versions; old dump/receipts untouched. Use **`image-125.runtime.bin` SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`**, 165,991,936 bytes, MEMORY VA=140000000+offset. Previously protected `14987c5b1` and `1495a0c8c` are now readable; old thunks/primary entry bytes stay unchanged. New proofs MUST bind this new image identity. Capture tool/launch/source receipts are in that directory, never commit image/prefix/TLS material. Later images differ in runtime data; do not silently replace the canonical old source proof input.
- **Lifecycle source advances:** committed **`36b4ddc`** adds `frontend/gi_init_control_flow_evidence.md` and replay inspector for the OLD-image prefix/module proof; root verifies 214 inputs/eight outputs/1,506 windows/20 typed edges in `gi-init-control-flow-root-audit-1.json`, SHA-256 `b6882af07004a8bb7df547975bda43e0cabfa60b0527a3ad3b3286fa3b2a18c6`. FortniteUI's third caller is module **PostLoadCallback**, not ordinary StartupModule. NEW-image worker `/root/late_gi_init_recovery` has recovered the complete original Init span through 149 conditional RET dispatches into a separate isolated Ghidra project. It now finds **Init directly calls gated InitializeUI at141e6934b in !dedicated branch**, PreLoadMap141e728e0 registration/global147c4e8b0, OnPostLoadMap141e72150/global147c4e8c8, with paired Shutdown RemoveAll. Old-image negative/prefix statements remain historical; new full evidence/receipt is still pending. No forced phase/input mode or reduced Init implementation is authorized.
- **Generic GetContext candidate:** canonical `/tmp/native-context-runtime-candidate-1/audited-final/Source`, frozen receipt `d58a3485a207d96f8957214321faeda92c4b8e3216159c6c10e6a4ca93e87a77`, integration patch SHA-256 `b3d4db68521cd6946ccb96076ba2078be6c13669584ae9daa62b7b88e55d5d04`. Root independently verifies all 30,984 bindings/18,434 snapshots/61 outputs/60 original windows, exact original GetContext flags/props and four changed generated files in `generated/frontend-native-evidence/native-context-runtime-root-audit-1.json` SHA-256 `d52405265c36dab4812a583edf171792bf5b4b12e5fb5919223a64adf65eb147`. Core and eight policy tests pass. Complete generic lookup/registry/base lifecycle is reviewed; actual derived contexts/authentic startup remain separate. No shared application or fresh UHT/build/runtime yet.
- **Context harness review:** v3 `/tmp/native-context-runtime-probe-candidate-3/manifest.json` SHA-256 `f7eb8ec11529b65161d177ec576e055b213f8e7a71c56e3c92d837d679acfce8`, instrumentation patch `f6119b74899e1717bdf5b56a71d0600487454244523dae87df70ad16474587aa`, 3,358 frozen files/15 tests, measures real GC bypassing pending-kill rejection, deinit, complete copied-map dispatch, automatic normal travel and actual UHT/linked ELF. Root found two additional pre-build issues: driver global constructor reads FCommandLine before initialization, and UBT legitimately preserves unchanged response/unity recipe mtimes. Root is preparing **v4** in `/tmp/native-context-runtime-probe-candidate-4`: move driver registration to manual FortniteGame StartupModule, accept old exact recipes with fresh generated code/object/binaries and compile-log binding. v4 bootstrap.patch and tool changes are not yet validated/frozen/applied. Review/freeze v4, then one owner applies generic patch + v3 instrumentation + v4 bootstrap, regenerates normally, and performs paired fresh UHT/metadata/live registry+travel and all retained regressions/joins. No fake Native events or direct source phase forcing.
- **Panel/input implementation owners:** Sol-medium `/root/authentic_gi_init_control_flow` owns `/tmp/common-panel-native-runtime-candidate-1` for base/Panel/queue. Completed base piece has 14,979-input receipt `c727a782c26059a767f8703085d63f54fd6d9f4bda8865bf62ae0225c2ff6255`, supplemental `6a8ab900289535d8d61217479e8faa3584dabf9e6894bc9a1220111d953918a8`, preferred base-stage-integration-2.patch, fresh redirected UHT and real source/glue syntax. UHT trace truthfully discloses ignored global Saved/config/mutex writes (no source/generated/stage writes); no blanket isolation claim. Complete portable queue/enqueue/reentry/three-tick core and typed adapter draft remain unintegrated; weak UE delegate lifetime and complete real dependencies still required. **Profiling prerequisite now closed** using the late image: empty TStatId return proved by 19 instructions/zero calls/no result writes plus exact Windows ABI control, receipt `source-evidence/stat-id-late-1/frozen-1/receipt.json` SHA-256 `876a847d98f5697e5b0f98825755bedfb5f28941ab784c012ad09b4927ff2f27`. No guessed stat implementation is needed.
- **Input dependency worker:** Terra-high `/root/common_input_native_dependencies` owns `/tmp/common-input-native-dependencies-1`; implements source CommonInput native producer/settings/global/held-action adapters. Interim 91-file research receipt `55b701588e3c0d1011cc544dfed729cc9f897eced13cef9beb9e44d572d574ab` is evidence only. Portable producer core, exact rooted per-module NewObject settings identity, public World.bIsTearingDown guard, row/interface type and strict generator drafts are progressing. IMPORTANT correction: lock priority uses **numerically greatest input enum**, not the earlier minimum claim. Frozen old research is preserved; new corrected implementation/tests required. Interface alias candidate receipt `ba1f1b5e8c10ad64005a48d9e895e95e8b0c151afac8e4868450e69491c15b34` is not fresh runtime evidence. Coordinate original shared Ghidra lease with this worker; the new GI worker uses an entirely separate project.
- **Next Login material candidate:** sole build owner `/root/donut_texture_engine` completed unapplied Google source/probe/verifier candidate `/tmp/google-icon-runtime-candidate-1/manifest.json` SHA-256 `5fd3499066be1283948cff7a70f851bb82ab1bd8e74d063335f7448c776488b7`, Probe SHA-256 `fa106b853e6ad71dc4212ad1bd305f6b781c727cce94c7ba8e491b17d6222c46`, patch `91f054c9eb84a4deb105f48956524c7e5a2fb7a8bb9966676a11a17b9ecde615`. Source bundles3/4 replay identically: SHA-1 `d0c13355d8b8ce26e47acdb0587b7f67fd6f2bfb`, fixture SHA-256 `21a60f0dc74358cc8801bd16875af8a4b1c2e47ee7355e7533476c1e6fc87325`. Eight producer +ten verifier tests and bounded syntax pass; root review pending. Exact name is **Texture**, not NameTexture. All19records/seven programs, 512² BC1 texture, first3 external plus7 inline mips, signed stored offsets and physical4×4 blocks for final logical2×2/1×1 preserved. Asset sampler is **resource1**, not Media clamp3. Residency must be requested/measured via ordinary diagnostic streaming APIs; no raw-BC1 GPU or original MIC-override claim. No shared application/staging/build/runtime yet.
- **Preservation/delegation:** root orchestrates/reviews and does difficult implementation directly. Small read-only `gpt-6-luna` medium; regular `gpt-6.1-terra` medium/high; really complex `gpt-6.1-sol` medium; always `fork_turns:none`, maximum three active workers. Preserve five prior Engine source edits and external CUE static-mesh edit; no shared CUE rebuild, no original extraction mutation, no remote/push or keys in Git. One coordinated owner for shared Native generation/build/stage; current previous owner is idle after release. Launcher/EAC/entitlement compatibility remains in scope. Goal stays active; this turn is progress, not a blocking wait.

## Earlier atlas retry snapshot (historical; superseded above)

- **Goal and dashboard:** all four product milestones remain false. The fixed non-Native frontend registration inventory now has **41/41** independently verified signatures; the existing material closure stays **123/142**. Total effort weight stays **1,855 across 108 leaves**. Native lifecycle, actual widgets, visual parity and gameplay remain unfinished. The requested combined treemap is already implemented by `350c651` and documented by `0333320`: blue fill rises behind gray/blue unit squares, with desktop/mobile, both sort orders and static SVG checked. No new visual edit was needed.
- **Last fully verified material/runtime baseline:** native **`1f0340e`**, Engine **`66105c47c`**, FadedFlogo checkpoint `frontend-flogo-checkpoint-results-1.json`, SHA-256 `fcf857b47374458acf23b8aa1dae3b506064851f1626c0ac9c9c6319bd722ac5`. Its complete regressions, profiles, Splash key checks and both joins passed. Its actual RootLayout blocker was SocialUserItemBorder; Login's was MediaPlayerButton. Preserve this baseline separately from the newer failed intermediate phase and pending retry; an interactive frontend remains unverified.
- **New paired build, explicitly failed material phase:** native **`2f40d73`**, Engine **`d7e04d4b6`**, portable patch **`2ceeda4`**. `generated/material_pipeline/frontend-media-social-failure-checkpoint-results-1.json`, SHA-256 `147b551097d98e73380586ebd660a51ac054e3df5b6a97cb06a8bffa58842536`, freezes 92 committed sources, 18 binary records/six immutable binaries, 16 registration snapshots and 689 artifacts. Builds pass in 311.941/361.349 s; fresh default/Athena joins pass in 21.577/21.689 s. Social defaults exits 1 at its atlas metadata expectation after seven required shaders/missing zero. VM/scratch/rejection/GPU/draw checks were not reached; remaining profiles, regressions and Root/Login were held. Root independently verifies all frozen bytes, source blobs, build logs, actual join markers and the failure in `frontend-media-social-failure-root-checkpoint-audit-1.json`, SHA-256 `868a0e2b5ce6db12135bc4b2b44e3bb422fecefa20acbf576922484521757ed3`; `passed` concerns integrity and **`phase_passed` remains false**. Phase 1 ended and sources were explicitly released.
- **Atlas failure explained and corrected source committed:** CUE's generic defaults were incorrectly used for this subclass. Original getter `1446ef940`→`1446f0b70`, callback `1446f0e00`, constructor `1441589a0` and original reflection/enum bytes prove **sRGB false, bilinear, HDR compression, clamp X/Y**. An independent package-byte parser confirms no serialized overrides for those fields. Original LODGroup initialization remains unresolved; the retained World value is explicitly an inference. Constructor receipt `social-atlas-defaults-proof-1/receipt.json` SHA-256 `787cd43f8289a5ddad33905c4ef9e8c22bad628f6d1a0ee690a45d26fd0b6815`; independent tag receipt `social-atlas-independent-tags-1/receipt.json` SHA-256 `c33b34e17d6fea309b682e4c15ad6cf57999fdb561ac9e1cb1f3d878b48edd94`. Seven producer/verifier/inspector/test/doc paths are committed **`3e8d6b1`**; the sole Engine owner applied the reviewed Probe gate/JSON/bundle-pin correction as **`8f87ade`**. No Engine correction was needed for these defaults.
- **Reviewed retry fixture and authorization:** corrected bundle `generated/material_pipeline/social-atlas-corrected-ingress-3/bundle.json` SHA-256 `bfa6dba715fd7f72ce4084eba67cb29b237bb6c5df9da9a1dbb115ccc3819a65`, SHA-1 `d5c2198bee653b99a95b219f517870d82979dd5f`; fixture SHA-256 `db3c8a1bf00347d86e9d47b9ea1aff778a21cf9d5dff0e0f6601eaec61e45cc6`. Candidate patch `/tmp/social-atlas-candidate/candidate-3.patch` SHA-256 `6f9614c93313ce684bcb8882ac7ecef33c1500d4a3d55441443404200ea62a33`; root audit `social-atlas-correction-root-audit-1.json` SHA-256 `a2153fc88e4110e133054a514fbb6c617798bd4f74db2c67566991571bdfc14f` verifies all eight paths, 220 frozen fixture inputs/four outputs, 39 original windows, fresh byte-identical root/shared replays and 21 passing tests. Only five metadata fields and proof provenance change in the fixture; shaders, source pixels, expressions, numeric controls and Media remain unchanged. The sole Engine owner is authorized for a NEW frozen paired-build/stage/runtime phase, all Social/Media profiles and strict verifiers, old regressions/fixed owners/texture profiles, actual Splash keys/opt-out, fresh Root/Login and both joins. Do not overwrite phase 1 or weaken contracts on failure. New immutable binaries go directly to MNT. Runtime retry is not yet accepted.
- **All 41 bounded signatures now credited:** root's 13-event/26-target ELF audit checks original function-constructor LEA bindings, exact metadata/name literals, flags, struct size, ordered properties, offsets/bool setters and referenced struct constructors against actual frozen client/server objects. A Luna read-only review agrees with signature-only credit and caught a commit-provenance gap; root closed it by binding the exact signature snapshots to the independently audited failed-phase source/binary checkpoint. Authoritative `generated/frontend-framework-signatures-2/root-framework-elf-signature-audit-3.json` SHA-256 `6fb0f2fbba83e05a9b3933ff8aabc898eeab673e06bcd2fb7ca762c538d821fe`; prior audits remain intact. New documentation: `frontend/framework_event_signatures_evidence.md`. Only the 13 pending non-Native registration checks were imported. Native panel intro/outro, material acceptance and widget behavior receive no credit. Progress history/model tests pass 5+4.
- **Social/Media implementation before retry:** new legacy preshader opcode 42 dynamically evaluates direct original TextureProperty vector roots through indexed textures, with width/height/+0/+0 and missing-resource zeros; unsupported nested/named/scalar/property1 forms remain rejected. Source/fixture producers `06957bf`, `a4855cb`, `284a9af`, and strict verifiers `7795188` were independently reviewed. The original Social atlas half mip is 8,192 bytes; Media is the original 128×128 BGRA8 asset with a World/clamp sampler selection distinct from asset addressing. No new owner acceptance follows from source fixtures or the failed metadata gate.
- **Authentic startup/context remains separate:** source checkpoints 5 (`9264eec`, receipt SHA-256 `40cc5232ca791d8d635e6e0d11a35d7deb1eefb86f31627d2c8714f34f8d8736`) and 6 (`dec068a`, `33135dc68bec913acc8e96076576ca693c0f6b838eee64d524aeae840c4856bc`) are root-audited original evidence, not runtime implementation. Actual FrontEnd/GlobalUI contexts are LocalPlayerSubsystems with their own concrete tickable bases and nontrivial init/tick/deinit; do not route them through the legacy registry or invent tick/StatID bodies. FortAssetManager phase progression, UI director/root/layer creation and real player/account hooks still need integration; do not force phases or legacy input.
- **Off-workspace GetContext candidate:** `/root/original_texture_context` owns `/tmp/native-context-runtime-candidate-1`; no shared application is authorized in the atlas retry. Candidate covers all three lookup branches, process-global FGCObject registry/weak player keys, world/deferred cleanup and base PostInitProperties/BeginDestroy/GetWorld phases. Review discovered original repeated creation clears SELF before NewObject while not-yet-replaced PEERS remain old. Helper `14081f800`/resolver `1408212f0` prove this; candidate now uses `Map.Add(C,nullptr)` with original pre-captured-slot order and tests those observations. Initial FindOrAdd draft is archived. Portable tests, eight generator policy tests and three real UE syntax units pass, with disclosed stale-UHT line aliases/textual PCH; no fresh UHT/link/runtime claim. Corrected manifest is now frozen at `/tmp/native-context-runtime-candidate-1/frozen-final-1/receipt.json`, SHA-256 `d58a3485a207d96f8957214321faeda92c4b8e3216159c6c10e6a4ca93e87a77`: 30,984 read/source bindings (18,434 unique snapshots), 61 outputs/60 original VA windows and both 12,512-file generation trees. Integration patch is `candidate-integration.patch` in that candidate directory. Root review is pending; review it before the sole build owner applies/generates in a later separate phase.
- **Ownership/preservation:** root plans/orchestrates/reviews and also performs difficult implementation directly. Small read-only tasks use **`gpt-6-luna` medium**; regular work **`gpt-6.1-terra` medium/high**; really complex work **`gpt-6.1-sol` medium**; always **`fork_turns: "none"`**, maximum three active workers. Sole Native generation/build/stage/promotion owner is `/root/donut_texture_engine`. Preserve five existing Engine edits and external CUE static-mesh edit; do not rebuild shared CUE tools or write extraction inputs. No push/keys in Git. Launcher/EAC/entitlement compatibility remains in scope. Keep the unbounded goal active; all four product gates remain false.

## Earlier FadedFlogo snapshot (historical; superseded above)

- **Goal and dashboard:** all four product milestones remain false. The dashboard now credits **28/41** original non-Native frontend event registrations and **123/142** owners in the existing bounded material closure. Keybind is also recorded among four named material cases. Total estimated effort stays **1,855 weight across 108 leaves**; widget/lifecycle/visual work retains its own unfinished measures. The combined bottom-up blue fill behind gray/blue unit squares is implemented by `350c651` and documented by `0333320`; a fresh Luna read-only source review confirms both sort orders and the mobile layout preserve both indicators. Earlier desktop/mobile and standalone SVG verification remains recorded. No new visual changes were needed.
- **Latest verified runtime:** native **`1f0340e`**, Engine **`66105c47c`**. `generated/material_pipeline/frontend-flogo-checkpoint-results-1.json`, SHA-256 `fcf857b47374458acf23b8aa1dae3b506064851f1626c0ac9c9c6319bd722ac5`, freezes 75 committed build-source blobs/snapshots, two separately committed verifier-tool sources (`047e97b`), 18 binary metadata records/six immutable binaries and 856 artifacts. Client/server builds pass (149.800/128.249 s); both fresh default/Athena joins pass (21.597/23.065 s). Twelve prior material cases, nine fixed-owner cases, Keybind/Hollow/Flogo defaults and combined profiles, and actual Splash-key readback all pass. Root independently verifies every frozen source/binary/artifact and successful logs/joins in `frontend-flogo-root-checkpoint-audit-1.json`. Flogo strict root reports verify the real parent cache, actual named child override, all 14 numeric overrides, nine graph/nine owner cases and all seven shader payloads. Actual source/copy BC3 draws match; separate checker controls change 2,897/2,923 pixels. Reports: `generated/frontend-probes/flogo-owner-probes-1/flogo-{defaults,combined}-root-validation-1.json`. No raw BC3 GPU-byte or original-client pixel parity claim. All build/runtime jobs ended and sources are explicitly released.
- **Actual blockers:** RootLayout stops at `/Game/UI/SocialPanel/Art/Materials/M_UI_SocialUserItemBorder_Master.M_UI_SocialUserItemBorder_Master`; Login now stops at `/Game/UI/Materials/M_MediaPlayerButton_Base.M_MediaPlayerButton_Base` (both MemoryImage.cpp1064, exit139). Interactive root/menu and widget construction remain unverified. The next Social path needs actual dynamic TextureProperty evaluation and its original half-float atlas. Original direct-vector zero initialization and atlas reference getter are now proven (below); native implementation remains pending. Do not flatten/drop expressions or substitute assets.
- **Hollow and Keybind evidence:** Hollow preserves all 19 source records/seven Slate shaders, original nonparameterized PolarCoords texture, all eight mips, clamp/no-sRGB metadata and exact GPU top-mip source bytes. Default and separate combined draws are byte-identical to exact source copies; checker controls change 759/1472 pixels. Root independently verifies both texture/control reports and seven shader payloads: `generated/frontend-probes/hollow-owner-probes-1/hollow-{defaults,combined}-root-validation-1.json`; original-source replay/audit `generated/material_pipeline/hollow-ingress-root-audit-1.json` verifies 50 frozen inputs. Keybind checkpoint `frontend-keybind-current-checkpoint-results.json` SHA-256 `b671d326cb8caae576ef774b12de0f87430822c829596c8f0cb0dbcb063806ec` independently passes49 source / 18 binaries/536 artifacts and both strict runtime reports. Use `keybind-defaults-root-validation-3.json` and `keybind-combined-root-validation-1.json`; prior failed direct/common verifier attempts are retained. Dedicated verifier committed `800ac65` preserves full texture verification and separately checks every shader. Neither result proves original-client pixel parity.
- **Animation version fix verified:** Engine `66105c47c` adds scoped `-FortniteLegacySequencer` handling only for unversioned source loads, selecting original Dev-Sequencer version 11 instead of current 12. Original float-channel Serialize returns false and uses tagged-property fallback. Actual loaded Splash class now retains all three original times `[0,30000,60000]`, values `[1,0,1]` and tangent/interpolation bits; no animation playback claim. Root checkpoint audit verifies 55 source / 18 binary / 557 artifact records in `frontend-movie-float-checkpoint-results.json`, SHA-256 `1d12cfaf1077abdb8d562949a0ff4fe6759df4254a9e3e29f5469dda885ce02d`; old opt-out overread exit139 is preserved. Runtime-key JSON SHA-256 `66db4cd8ed44b665b40754f93e329fd7fb953ff3c5f3bdd9b6a9c5e351eddfd7`. Source proof `generated/frontend-native-evidence/movie-scene-float-channel-proof-2/receipt.json`, SHA-256 `fb652f574d2aa1335a9db7dcd5c33af2f2fd3b270498acafa1268ec732593bec`; portable patch and eight portable/source tests are committed `696903b`.
- **Typed events verified and credited:** successful source `e06c28b` fixes the narrowly evidenced confirmation struct after the first UHT failure. `frontend-typed-events-checkpoint-results-2.json`, SHA-256 `39f79c6bc70501aac7c4ec102a228b231c1293f602eee9d3996529398d87ade6`, freezes 69 source / 18 binary / 656 artifact records; both builds, regressions and joins pass. Root checkpoint audit passes, and `generated/frontend-typed-events-2/root-typed-elf-symbol-audit-1.json` independently compares all 22 client/server event records with original memory bytes, actual ELF metadata, property flags/types/offsets, bool setters and constructor relocations. Both actual C++/UHT confirmation structs are 176 bytes/alignment8, with field offsets 0/24/32/168 and original BlueprintVisible flags. The previous 17 registered events plus 11 new ones make 28/41; the two Native intro/outro events remain behavior work, not fake stubs. Failed attempt1 is preserved uncredited. Original parameterless 14 checkpoint is independently audited in `frontend-super-events-root-checkpoint-audit-2.json` (use audit 2; audit 1 has a recorder hash-field bug).
- **Authentic UI startup source checkpoint:** committed `01ea647` adds four source files, including an isolated CUE probe; the later phase/context source checkpoint is committed `679ad3f`. `generated/frontend-native-evidence/native-ui-startup-checkpoint-3/receipt.json`, SHA-256 `dbedd66b0726d7dfe9917a049be2adfd31cc01de7faf6c3bc70a543d6b2eb663`, freezes 250 inputs/40 outputs; earlier 152/17 checkpoint retained. Root independently verifies all 250 frozen inputs, 40 outputs, 36 original raw VA windows and the three committed probe sources in `generated/frontend-native-evidence/native-ui-startup-root-audit-1.json`. Source startup routes GI→FortUIDirector→configured UIManager_Startup→RootLayout_Athena, then hierarchical UI.State.Startup.Login→UIData.LoginScreen→root-owned CreateWidget/SBox layer. Native legacy-input default is false and the cooked startup manager does not override it. FortAssetManager phase must reach original GameStartupLoading(3); current native config lacks that asset-manager class. Recover actual startup progression and shared process-global context registry before implementation; do not force phase 3 or legacy input. Native panel/queue mechanics are separately committed `ef87b26`, `e8a3c97`, `e36290b`; behavior remains unported/runtime-unverified. Latest checkpoint4 `native-ui-startup-checkpoint-4/receipt.json`, SHA-256 `3459ba0ebe7777b8d60c6015a290c8c4d820c5b63afe1a7d849a681cdaee9834`, adds real phase transitions and source player-context creation/destruction/deferred-world/GC ownership evidence. Root verifies 374 frozen inputs, 72 outputs and 68 original VA windows in `native-ui-startup-root-audit-2.json`. Further existing-player creation and destroy-caller findings are still generated-only and must be frozen separately; do not treat new analysis as an implemented lifecycle.

- **Original TextureProperty and atlas proofs:** root committed `8fafc55` after independently recovering the original per-vector zero/call/copy loop. `generated/material_pipeline/texture-property-caller-proof-2/receipt.json`, SHA-256 `03a19ce546da78b2a0229d909d107c5e8f76af51fc05d78a440c839101535c13`, binds the original image, six raw code/table/unwind windows and frozen helper. Actual FillUniformBuffer entry `144353900` zeroes all four lanes before each direct vector call at virtual `+18`; property0 consequently gives width/height/+0/+0, and missing texture/resource gives four zeros. Nested callers remain unproven. Three tests, including mutation rejection and actual-image replay, pass. Atlas proof committed `1252d27`: `generated/frontend-native-evidence/original-curve-atlas-reference-proof-1/receipt.json`, SHA-256 `f007855d75984f806382e05b2b79a1cc2899b28edc28ee032fc1372569e2a202`, proves CurveAtlasRowParameter's installed vtable returns Atlas+68 at +268 and true at +270. Root freshly replays and verifies all 23 inputs/20 outputs/19 raw windows in `original-curve-atlas-reference-root-audit-1.json`. Together with the complete source expression/function order, these enable a bounded Social fixture; no runtime credit yet.
- **Next coordinated implementation:** Terra owns NEW Social source/fixture files and its isolated atlas inspector. First source-only candidate `social_border-source-1/source.json` SHA-256 `765b8321e271cfe3e11b195b6ad4ba94ba9c61d7f652f0ba728a5fea8b3500ec` and 46-input receipt `social_border-frozen-inputs-1/receipt.json` SHA-256 `037123d240c58df507d01142339eafd5369056b2e648731ddeec052ad4ced9e3` deliberately leave lanes/reference order unresolved; preserve them and produce NEW outputs using the newly proven caller/getter. The original atlas is 32×32 PF_FloatRGBA, one 8,192-byte half-float mip, source sRGB true/asset wrap, while expression sampler2 uses world-clamp resource index3. Engine owner prepares an OFF-WORKSPACE direct-root legacy TextureSize2D design; ordinary current TextureSize also writes SizeZ and mishandles the original missing-resource stack contract, so it cannot be blindly reused. No Engine/Native application is authorized yet. Sol owns source startup-phase/context recovery and exclusive Ghidra. Native generation, promotion, staging and UE builds retain one owner. Root owns review, integration, ledger and handoff. The numeric manifest remains 120 owners; prior full118 evidence is retained.
- **Commands and checks:** `/tmp/fortnite_root_checkpoint_audit.py new-checkpoints` passes Movie/typed/Hollow integrity and runtime-log checks; its separate `flogo` mode also passes all 75 build sources, two tool sources, 18 binary rows and 856 artifacts; `/tmp/fortnite_root_typed_elf_audit.py` passes 22 actual compiled events plus both layouts; `/tmp/record_verified_frontend_checkpoint.py` imports only those 11 event passes and Hollow into existing denominators. `python3 -I porting/progress/test_history.py` (5 pass) and `python3 -I porting/progress/test_record_frontend_materials.py` (4 pass) verify budget, target scope, grid reconciliation, unchanged 142-owner closure and all four open gates. Flogo strict verifier has 10 passing tests; its producer has seven tests and a fresh byte-identical original-source replay (`flogo-ingress-root-audit-1.json`, all 58 frozen inputs). Earlier green checkpoints/audits remain immutable historical evidence; never overwrite them or rewrite recorded progress history.
- **Preservation and delegation:** no remote/push. Preserve the five preexisting Engine edits and external CUE static-mesh edit. Put all new immutable binary snapshots directly under `/mnt/vibrato/fortnite-evidence/frozen-runtime/`; seven old directories were relocated byte-for-byte with symlink paths preserved (journal `generated/material_pipeline/evidence-snapshot-relocation-1.jsonl`). Root plans/orchestrates/reviews and does hard implementation directly when useful. Small read-only **`gpt-6-luna` medium**; regular work **`gpt-6.1-terra` medium/high**; complex **`gpt-6.1-sol` medium**; always **`fork_turns: "none"`**, at most three active workers. One owner for Native generation, promotion, staging and UE builds. Launcher/EAC/entitlement compatibility stays in scope.

## Earlier Splash snapshot (historical; superseded above)

- **Root audit follow-up:** independent source/binary/artifact audits now pass for ContinueText (22 source blobs, 439 artifacts), StartMusic (26/442) and Splash (31/447), each with 18 binary metadata rows/six immutable binaries, both actual joins and 12 successful regression logs. Reports are `generated/material_pipeline/frontend-{login-event,start-music,splash-event}-root-checkpoint-audit-1.json`; `ui-events-root-elf-symbol-audit-1.json` independently compares exact metadata symbol bytes in all six compiled objects. `keybind-and-texture-source-root-audit-1.json` verifies 72 final Keybind inputs, exact fresh replay, all 18 producer tests, 64 v4/38 v5 artifacts, eight original texture vtable slots and the original opacity store/getter. `generated/paper2d-isolated-cook-3/experiment-root-audit-1.json` verifies all 289 experiment artifacts, original/editor hashes, four overlaid material files, unchanged Dummy/global archives and the two successful material loads without logged fallback. These audits supersede the pending-audit notes below; none establishes interactive UI or original pixel parity. Exact audit commands/scripts: `/tmp/fortnite_root_checkpoint_audit.py`, `/tmp/fortnite_keybind_source_audit.py`; fresh producer command `python3 -I porting/material_pipeline/export_keybind_ingress.py --output porting/generated/material_pipeline/keybind-ingress-root-replay-1/bundle.json`, then `python3 -I porting/material_pipeline/test_export_keybind_ingress.py` (18 pass). Dashboard M1 evidence now reflects these current boundaries instead of the obsolete detached-Locker state.
- **Current build in progress:** generated event headers committed as `7c0aca3`; isolated/official generation changes only the seven approved headers (+70 lines), preserving all other generated bytes and restoring 12,504 unchanged mtimes. Client build passes (148.91 s); paired server/UHT/runtime checkpoint is still pending. Do not replace the last frozen green source with this partial build.
- **Goal and dashboard:** all four product milestones remain false. The ledger credits **121/142** owners in the bounded frontend material closure (prior MainL plus 120 numeric owners), approximately **20.953%** estimated effort across 107 leaves/1,855 weight. This closure does not include every runtime dependency. Interactive widgets, original visual parity and full gameplay remain unfinished. The combined bottom-up blue fill and gray/blue unit squares are already implemented by `350c651`; the dashboard subagent rechecked the live desktop/mobile, both sorts and standalone SVG without further changes.
- **Latest built baseline:** native `e4e66b1` / Engine `8fba83b44`. Splash checkpoint `generated/material_pipeline/frontend-splash-event-checkpoint-results.json`, SHA-256 `2fff9ba9e0883f3cffc162b8d688f3e08434708f1ff21f93c7814057b4df19cc`: both builds, 12 material regressions and fresh default/Athena joins pass. Client/server builds 143.047/143.578 s; joins 21.683/21.784 s. Compiled UHT/ELF proofs preserve the original protected, non-Native Blueprint event with one uint8 enum argument and two serialized property records. Root's independent full checkpoint audit is still pending; do not call the worker receipt a root audit.
- **Actual UI blockers:** RootLayout still fails on original regular-texture material `/Game/UI/InGame/KeybindWidget/Materials/M_Buttonborder_DF.M_Buttonborder_DF` (MemoryImage.cpp1064). Login progresses past recovered ContinueText, StartMusic and Splash input events, then fails on missing `/Script/CommonUI.CommonActivatableWidget:BP_OnActivated` (AsyncLoading.cpp3126). Both class probes exit139; widget construction and an interactive menu remain unverified.
- **Next controlled build:** `d241921` adds 14 original parameterless non-Native Blueprint super events to the generator, not generated output. Root reviewed the code, ran portable/actual-reflection tests, and independently verified all 14 raw 72-byte FFunctionParams, owner/flag fields and constructor RIP references. Sole build owner `donut_texture_engine` is authorized to isolate generation, accept only seven intended owner headers, verify all 14 compiled declarations in both targets, then run paired builds, regressions, joins and Root/Login probes. Do not conflate this source approval with a built checkpoint. The two Native CommonActivatablePanel intro/outro events have real side effects and remain under original-code investigation by `original_texture_context`; empty stubs would be incorrect.
- **Material credit just imported:** native `b74148e` generic source-count support and manifest12 SHA-256 `d4dd8dbb275f2c395abb9bfb52edbdf98a37f73d070b1559e46cab2c74a81e03` add HGrad and EMS. Root-audited authoritative checkpoint is `frontend-batch12-checkpoint-results-v2.json`, SHA-256 `40366ab55123af9d9dc1209565bbf33a1e72d9e91a1d498f74518982d92b388f`; 12 source blobs/snapshots, 18 binary records/six immutable binaries and 407 artifact hashes pass. Six targeted plus six fixed-owner probes and both joins pass. **114 manifest owners were not rerun on that source**; the prior independently verified full118 checkpoint remains the evidence for them. The original `9f7fbb6c...` receipt is preserved: v2 corrects only its builds dictionary to the required list. Importer updates six records, including two new passes; no credit for unrun work. Producer audit passes 17 tests, 120 bundles/2,176 cases and preserves all 2,144 previous cases.
- **Intermediate event receipts retained:** ContinueText checkpoint `622d6ce96f43da4fce931252351c6730a12e9ad96334ab25b98c3dac952affd5`; StartMusic `fdb155a505bd245d51f350ddf12b483d0f6a1cb404101a1b1218c6c907cfb27f`. Root full audits remain pending. ContinueText's artifact list includes a now-changed generator worktree path: resolve only that exact source/SHA against its matching immutable source snapshot and committed blob, never waive a hash mismatch. Later collectors bind source artifact paths directly to immutable snapshots.
- **Next RootLayout material:** source producer `515d580` prepares the original Keybind regular Texture, exact 64x64 BGRA8/sRGB/one-mip source and all seven original shaders. Candidate3 bundle SHA-256 `072453e01e19e50574d57b7c93ae5721d24481b0023af504461fcea6fe67dd18`; evidence2 `57ecd825d6de29749bcf68d0e60cac3423463dd428afd47eea3f985f7493004a`, binding 72 frozen inputs. All 18 producer tests pass; root reviewed the source, but independent final receipt/export audit remains pending. Native/Engine bridge drafts are off-workspace and uncompiled. Do not apply them during the controlled event build. Preserve absent parameter identity; do not invent a TextureParameter name or replace the original source texture.
- **Original texture mechanics:** source evidence now also covers ComicBubble instance layer/parent reference ordering and logical Texture2D dimensions. v4 receipt `95b4887076fbda3e278d53e8de93a0fc6063932e77e862928de22cd941166c5b` and v5 `c70a12b30eea35dfbeae40679050a89231fe520c3a367c79269d808f52aede44` await root audits. TextureProperty writes only R/G; remaining lanes and missing-resource caller behavior are unresolved, so do not enable general numeric flattening. Original OpacityMaskClipValue constructor default is proven as bits `0x3eaaa64c` (0.3333f), not the CUE parser fallback 0.333f; receipt `44f0c058049624f82751f6f375ec601bb59f1de85d43957c5c00c9a32f439bc6` awaits root audit. This establishes a constructor default, not original pixel parity.
- **Storage and preservation:** seven old immutable binary snapshot directories were moved byte-for-byte to `/mnt/vibrato/fortnite-evidence/frozen-runtime/` with original absolute paths retained as symlinks; 28,017,289,490 bytes verified. Journal `generated/material_pipeline/evidence-snapshot-relocation-1.jsonl`. New snapshots go directly to that drive. No active stage, original extraction, source binary, bundle or manifest was moved/deleted. Preserve the five old Engine edits and external CUE static-mesh edit. Engine still `8fba83b44`; portable patch includes it. No remote/push.
- **Persistent delegation:** root plans, orchestrates, reviews and also does difficult implementation directly. Small read-only: `gpt-6-luna` medium; default delegated work: `gpt-6.1-terra` medium/high; really complex: `gpt-6.1-sol` medium; always `fork_turns: "none"`, at most three active workers. Native generation/promotion/staging/builds have one owner. Current tasks: Engine owner controlled event build; Sol original Native activation-event behavior; dashboard check completed. Root owns independent audits, ledger and handoff.

## Previous snapshot (historical; superseded above)

- **Goal:** feature-complete rebuilt Fortnite 12.60 client and reconstructed multiplayer server; all four dashboard milestones remain open. The revised **20.951%** estimates effort across 107 work items, with 1,855 total weight. Material engineering and actual-owner compatibility receive separate bounded ledger credit; real widget integration and original visual parity remain unfinished. Combined vertical-fill/unit-square treemaps and historical measurements are preserved.
- **Verified source:** native `aeabd2c`, Engine `8fba83b44`. Both non-editor builds and fresh default/Athena direct joins pass in immutable `generated/material_pipeline/duplicate-defaults-checkpoint-results.json`, SHA-256 `7b63e69b2051cbca44c832e96926e53d331df1a256dc148df0e62b13decf05f6`. Client 136.24 s / server 121.50 s; joins 21.663 / 21.589 s. Root independently verifies 12 committed source blobs/snapshots, all 18 binary metadata records and six immutable executable/symbol/debug snapshots, 2,228 artifact/log hashes, all 118 successful owner seven-shader payload sets and actual server Join succeeded/client Welcomed markers. All six fixed-owner regression receipts and all 20 duplicate-default words pass. Audit: `generated/material_pipeline/duplicate-defaults-root-checkpoint-audit-1.json`. Previous batch-11 checkpoint `e6334d7a...401df` preserves its 95 passes/Throbber failure/22 unattempted; do not overwrite it. No interactive frontend or full gameplay/visual/wire parity is established.
- **Four original owners now load:** Locker, GradientPanel, its PowerToastGlow instance and MenuButton preserve all 19 source records each, convert the seven Slate shaders their actual owners require, and attach on both threads with zero missing. PowerToast verifies five source parameter identities, eight graph/eight owner cases and a 3,952-pixel color change. The real parent is translucent (blend 2, flags 2816); its instance is opaque (blend 0, flags 2848), preserving the actual override. Six parent graph dependency packages were recovered verbatim.
- **MenuButton uses the real View buffer:** original metadata proves GameTime at byte 2264; native compiled metadata agrees. Current View is 4,032 bytes/61 resources/hash 130419238, while the original full struct is 3,952 bytes; only the observed GameTime access is proven compatible. Material is 304 bytes/hash 20447487. The original program changes 1,851 pixels between GameTime 0/3.25, remains identical when only RealTime changes, and changes all 4,096 pixels for a separate fixed-time Primary Color override. Eight numeric/four View cases pass. Artifacts/log: `generated/frontend-probes/legacy-numeric-menu-artifacts-4/` and `legacy-numeric-menu-4.log`; root validation receipt is beside the directory.
- **Donut texture owner verified:** original indexed PC icon and the real App2 instance Icon override sample through the original shader. Source textures are 128×128 BC1/sRGB/eight mips. Layout is 128 bytes/hash 92801271, texture/samplers at 96/104/112/120; five graph/five owner cases, seven required/zero missing and 19 retained source records. Pinned v3 bundle `donut-ingress-bundle.json`, SHA-1 `e794ad691e278ecf653dfef34c0ae07375f32d7d`. The cache bridge restores indexed textures only, not full cached-expression fidelity, and synchronous GT PostLoad loading warns of streaming hitches.
- **MainL original instance verified:** its own 9,290-byte static-permutation resource retains 19 records, seven required/zero missing; nine source overrides and six graph/six owner uniform cases pass. Real GradientPanel parent is translucent (flags 2816), MainL is opaque (2848). Layout 176 bytes/hash 12058879 with samplers 160/168; map identity hash `2afda3622b1340649ca10286e313d6b3c8d9a5a5`. Checked bundle `mainl-ingress-bundle-checked.json`, SHA-1 `61800f5706ffab57cc5d50ea4eaa9cf642564662`, resource SHA-1 `5837fd2391001dbf6bafb3b06db5cd050399fbf4`; earlier `fb7157...` bundle is retained unbuilt history.
- **M1 remains unfinished:** no interactive root menu is displayed. RootLayout/LoginScreen now reach original Paper2D and fail its frozen shader-map reader. FeatureButton is among the 95 passing manifest owners that attach normally in the frozen baseline. Strict SPIR-V access proof `e4ab9b0` covers all 49 selected View-consuming PS programs: only GameTime at 2264 is semantically accessed; full View ABI and original pixel parity remain unproven. Float4 loads physically cover 2256..2272, but all uses select component z; pointer/value escapes and other fields are rejected. Original metadata remains `view-identity/evidence.json`.
- **Current client / server state:** paired frozen baseline is native `aeabd2c` / Engine `8fba83b44`, with 118/118 manifest owners, six fixed owners, both builds and fresh joins green. Throbber preserves expression-local white/gray defaults and scalar signed zero; named overrides affect both duplicate identities consistently. Root verifies the seven payload pairs (layout hash 16253183) and ten default plus ten override words. Native SOURCE RELEASE is explicit; `donut_texture_native` applied only the reviewed generic 14-record count candidate to Probe.cpp, targeted syntax passes but it is currently uncommitted/unbuilt. It derives finite integral 7..256 source counts, matches all declared/actual group and total counts, preserves every original record and fixed-six exact19 checks. Reader/header unchanged. Paper2D Cook3 and root probes are complete; sole Engine owner will build/test the native change once the final fresh manifest is pinned; do not conflate source edits with the frozen baseline.
- **Critical root blocker:** with current 118-owner manifest, RootLayout/LoginScreen both reach `/Paper2D/TranslucentUnlitSpriteMaterial.TranslucentUnlitSpriteMaterial`, then fail the current frozen shader reader (`probe-batch11-{rootlayout,login}-trace-1.log`, `batch11-root-loads-1.result.json`). All prior converted Game owners pass normal completeness gates. This is an original stock plugin MI, not a missing file; raw original pair is already staged verbatim. Parent is `/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial`. No interactive root menu or widget construction is verified.
- **Paper2D next work:** `75c6cfa` reports original and current material properties/connected graph, `f95a334` proves the current source instance's active legacy translucent override with exact export consumption, and `b45b992` recovers the original instance's compiled base-pass sprite contribution: `E=max(lerp(SpriteTexture.rgb*COLOR.rgb,SelectionColor.rgb,SelectionColor.a),0)`, `A=saturate(SpriteTexture.a*COLOR.a)`. Original SelectionColor defaults to zero. The current five-node parent graph agrees; original/current instances actively override blend to translucent, inheriting unlit/two-sided parent properties. The original prefix stops before unknown FHitProxyVS (1,610 bytes untouched); no full-map/all-pass/pixel claim. Tracked rationale: `material_pipeline/paper2d_shader_semantics.md` and `paper2d_legacy_override_evidence.md`; generated receipts in `paper2d-shader-semantics/` and `paper2d-native-override-evidence/receipt-v2.json`. Sole UE owner prepared isolated no-C++ Paper2D projects with existing Oct30 Editor. Cook1 failed on a missing SteamVR Editor module. Cook2 exited0 but produced a real parent default fallback because Niagara VF shaders were absent; its outputs remain unvalidated. Cook3 enables matching Niagara and Paper2D and exits0 in 9.58 s. Actual parent and MI direct loads exit0 with no fallback, effective properties match, original source/editor hashes remain unchanged, and clone2 overlays only four material package files while preserving original Dummy/global archives. Frozen experiment receipt `generated/paper2d-isolated-cook-3/experiment.receipt.json`, SHA `0ad861fb41eb4ad99ae2c8f18b8ba594860de7921b9f37d1b2570ab69d3b0e2e`; supplemental `global-shader-archives-unchanged.json` verifies all three original archives unchanged. Root/Login still exit139 at the next original owners M_Buttonborder_DF / M_UI_Border_HGrad (DerivedType1064). This is verified deserialization/asset-load progress, not all-pass, pixel, widget or interactive-menu parity. Overlay only the cooked framework material pair into a NEW runtime stage clone, retain original DummySpriteTexture and immutable original raw stage; then probe root construction. Existing Oct30 Editor binaries must not load the newer game C++ module. Cook/loading/all-pass/pixels remain unverified.
- **Numeric batch source:** latest immutable `generated/material_pipeline/frontend-numeric-batch-11/manifest.json` SHA-256 `a38a5e5fdd04637326f662cac4462185d3470734ccc2c3a9b1fa9d886f331523`. It classifies 118 eligible owners (44 own maps/74 inherited), five previously weighted/frozen owners, 24 rejected (22 texture-dependent, two original-record-count prototype exclusions). Six fixed bundles must also be supplied. Source producer/verifier `00c5abe` accepts original Material CB declarations with positive 16-byte-aligned span ≤ the full numeric prefix; AngledBox shaders declare 608/720 versus actual 736 numeric bytes and complete layout 752. Original bound buffer role, full layout/hash/data and instructions remain exact. Sixteen producer tests/audit pass 118 bundles, 2,144 numeric cases and all 1,676 batch-10 cases preserved; seven verifier tests pass. The two remaining nontexture exclusions are Border_HGrad and inherited EMS: source has a complete 14-record map (seven Slate/seven LocalVF), MD_UI/opaque/unlit, layout128/112/120; Python producer `c277568` now accepts bounded 7..256 full source counts with exact declared/flattened/group counts, existing LocalVF shader allowlists, canonical seven unnumbered Slate records, exact EOF and all archive associations retained. Actual HGrad validates all 14 receipts; portable corrupt-report coverage `5a104b9` plus the actual fixture gives 17 tests passing (two optional generated audits skipped). Historical fixed-six exporters remain exact19. Native corresponding source-derived guards are now applied after explicit release and pass targeted syntax (4.41 s); runtime validation is pending. A fresh batch-12 manifest is being generated. No new runtime credit is granted for those two owners yet.
- **Complete structural source inventory:** `ef582dc` / `984223b` add original-proven Max, Saturate, Texture and TextureProperty serialization. `generated/frontend-probes/frontend-material-inventory-textures/summary.json`, SHA-256 `06f45720da994337266ac1a5774116675a88aa15f36786554fadbdd31e0c0254`, covers 147 material exports from 157 candidates: 38 base maps, 27 static-instance maps and 82 inherited instances. All 65 owned resources decode to exact EOF with original archive associations. This is decoding, not conversion: Texture/TextureProperty numeric evaluation and runtime flattening still fail closed. Original evidence: `original-max-saturate-evidence.json` SHA `a37af29d...4203`, `original-texture-expression-evidence.json` SHA `d506041d...806`; tracked rationale in `material_pipeline/original_max_saturate.md` and `original_texture_expression.md`. TextureProperty original serialization is one child then uint8 property; Texture base is 16 bytes without ParameterInfo. Runtime default texture indices, property evaluation/context and original pixel parity remain separate unfinished work.
- **Provenance and preserved failures:** Engine now copies each selected serialized resource's own immutable sidecar into the final material resource in all three quality/feature branches and both reload sites; constant getter exposes it. Pointer-only map assignment clears stale receipts. Normal `FindId` map reuse stays intact. Source+destination loss was proven by both typed and raw GDB (`gdb-batch9-feature-provenance-{2,3}.log`), then live tests passed. Strict native checks require actual final-resource provenance, callback receipt, exact bytes, ordered names, original map identity, base shader identity, complete original record set, canonical completeness and GT/RT attachment. Previous baseline snapshots remain immutable: `frontend-batch7-intermediate-checkpoint-results.json` SHA `4b76ea31...cb79` (41 attempts/38 exit0/two zero-delta diagnostic failures/one legitimate-reuse diagnostic failure/nine unattempted); root independently verified nine committed blobs+snapshots, nine binary/symbol copies and 810 artifacts. `frontend-batch10-intermediate-checkpoint-results.json` SHA `8d0aa228...97c9` preserves five targeted passes, failed unpinned AngledBox parent and exact-bit proof. The latter parent was numeric-only despite unused editor texture nodes; its true issue was the now-corrected trimmed CBV guard. All old logs/images/manifests are retained.
- **Dashboard accounting:** `669e5b5` preserves total weight 1,855 and combined treemap. UI weight-10 group has four leaves (6/2/1/1): named ingress engineering; remaining known closure compatibility; real widget integration; original visual parity. Of 142 closure owners (excluding five separately weighted owners), **119/142** now pass: prior MainL plus all 118 independently reverified current manifest successes. Effort **20.951%**, 107 leaves; all four milestones remain false. Importer `53934e5` fixes an old log-format mismatch that skipped all new successes: shared-map provenance is accepted only for the same source-resource SHA-256 and shader identity, retaining exact owner/resource and both-thread success checks. Eight progress tests pass. The importer binds frozen source commits, manifest/input SHA, immutable client/symbol snapshots, full runtime checkpoint and each payload/log; failures/unrun/source eligibility receive no credit. `0333320` corrects stale treemap documentation; rendered combined fill/squares were rechecked across desktop/mobile/sorts/static SVG.
- **Work ownership:** sole UE build/stage/runtime owner `donut_texture_engine`; native owner `donut_texture_native` applied the reviewed 14-record generic map support after explicit release; targeted syntax passes, unchanged Probe.cpp needs its own commit before build. Root is generating fresh `frontend-numeric-batch-12` (no overwrite); final manifest hash/120-owner classification still require verification before build. Primary owns portable Engine patch, handoff, final independent verification and dashboard imports. Portable ingress patch now includes Engine `8fba83b44` expression-local default fix and reverse-checks clean; earlier series through `c13bb7dd2` was committed in `d29b6e7`. No concurrent UE build/cook or frozen-source edits. Original extraction/keys, five preexisting Engine edits and external CUE static-mesh edit stay preserved. Complex Sol-medium `original_texture_context` owns new texture-context evidence/tools only, recovering original Texture virtual+0x50 index resolution without touching frozen native/Engine inputs or running UE. Committed original texture mechanics `281c195` prove the native material reference array and PostLoad expression traversal. Root verifies all 69 artifacts, nine inputs and six vtable slots in immutable `original-texture-context-evidence.json`, SHA `f997c42d5793527f968f193dd0e875b77e348729c0198e6c2a925401ce5ed0a0`. Parent direct textures deduplicate by pointer; function texture append is a separate path. `6de099b` now proves ComicBubble parent texture indices through seven complete function reports and original capability virtuals: index0=Clouds, index1=BubbleTail; shader uniform order is Tail(index1,sampler2), Clouds(index0,sampler0). Root independently replays exact proof SHA `e7cf1f59...6a51`, verifies 75 frozen v2 artifacts, nine original vtable slots and all source inputs. Authoritative v3 receipt `original-texture-context-evidence-v3.json`, SHA `5617b511c2abe2553c8186182eec24f548fdff4872fbd366bea70f8c56f8cb08`, pins immutable tool source snapshots and prior v2 frozen binding; f997 is preserved. Audit `comicbubble-texture-context-root-audit-1.json`. Sol is now recovering static-instance reference-array layer/parent order; texture dimensions, general TextureProperty evaluation and actual ingress/draw remain unfinished. Persistent Luna/Terra/Sol model settings below remain authoritative.
- **Frontend context routing:** `d909b82` plus the latest tool extension prove seven original class literals, including UserWidget, PlayerController, LocalPlayer, local-player/game-instance subsystems and BlueprintContextBase, with fourteen bounded function snapshots. New proof `generated/frontend-native-evidence/context-accessor-identities-2/receipt.json`, SHA-256 `7f1db78f2e0abf859fca0fdcd738a8a4402ee566d5b3410fd56dd2db2521e7b1`, uses the pinned original memory image and direct objdump without Ghidra. Original custom registry validates class keys against BlueprintContextBase and compares exact validated class pointers. Constructor callback globals and handles are recovered, but delegate semantics, context insertion, GC ownership and teardown still need recovery. New five-function decompile `context-manager-decomp-1/` discloses bookkeeping updates. Rationale: `frontend/context-lookup-evidence.md`; no GetContext behavior was ported or credited.
- **Tools and evidence:** `6e59ff6` adds full CUE scalar/vector ParameterInfo plus immutable v2 numeric owner bundles and three inheritance tests; `71e44ff` captures the original View member chain and source-derived current prefix. The active Menu bundle is `generated/material_pipeline/menu-button-ingress-bundle-active-time.json`, pin `0b49a605763b907dbdf1f0041715208a81d4744a`. Earlier 0/1-second probes failed to exercise the original 3.0..4.5 shine window and remain retained; the unused quarter-time bundle is not the active pin. Verifier tests pass five, and typed View/Material bindings plus both layout hashes are independently checked.
- **Preserve:** one owner for native/Engine builds and staging; immutable extraction/external keys; five preexisting dirty Engine files plus the CUE static-mesh edit. Engine branch `codex/fortnite-12.60-compat`, never push to Epic; main has no remote. The portable ingress patch includes the committed Engine series through `8fba83b44` and reverse-checks clean. Alternate STORE_ONLY_ACTIVE_SHADERMAPS reload has earlier syntax-only coverage, not runtime proof. Existing unrelated Water startup warnings remain; tested owner package dependencies have no missing imports.
- **Agent preference:** primary plans, orchestrates, reviews and also performs difficult work directly when useful. Luna medium for small read-only tasks; Terra medium/high for regular work; Sol medium for really complex work; always `fork_turns: "none"`, at most three subagents. Preserve through compaction and future sessions.

## Detailed evidence for this checkpoint

- **Latest numeric/View checkpoint:** final commands and immutable proof are in `generated/material_pipeline/numeric-menu-checkpoint-results.json` (native `ebea44e`, Engine `6925baad3`). Client4/server2 and both joins exit 0. Root verified committed source blobs, current source/staged binaries and all 74 recorded file hashes. Independent final verifiers: `legacy-numeric-menu-artifacts-4-root-validation.json`, `legacy-menu-final-power-root-validation.json`, `legacy-menu-final-locker-root-validation.json`. Menu readbacks prove two GameTime/RealTime isolation pairs and 1,851 changed pixels; fixed-time red/green changes 4,096. Native `25bf1c4`/Engine `6925baad3` is the separately retained Power checkpoint. Current ledger commit `769ff4f` adds Power/Menu credit and refreshes builds/joins; older receipt sources below are historical.
- **Current texture and next material boundary:** Donut is now verified as described above and `material_donut_asset_ingress` receives bounded ledger credit in `dd4cc90`. MainL own-resource reports are `generated/material_pipeline/mainl-{inline-resource,legacy-prefix}.json`; original CUE reports are `generated/frontend-probes/material-mainl.json` and `mainl-export-map.json`. Fresh mapped/reflection receipts: `mainl-mapped/mapped-translation-report.json` and `mainl-mapped-spirv-reflection.json`. Its own material uses D3D b1 span 160 with no View or texture resource table entries. Do not reuse parent resource bytes for its distinct static permutation. The full bounded inventory is complete and the numeric batch details above supersede this earlier individual-owner boundary.

- **Latest commands and bounded checks:** final client4 build exits 0 (110.23 s), server5 exits 0 (104.54 s), owner3 exits 0 (4.785 s), default/Athena join wrappers both exit 0. Exact commands and hashes are in the receipts above; owner command includes `-FortniteLegacyMaterialIngress=<absolute locker-ingress-bundle.json> -FortniteProbeLegacyOriginalMaterial -FortniteLegacyFieldPaths` on the isolated Vulkan stage. Three alternate-branch syntax commands/results are in `generated/material_pipeline/reload-store-only-syntax/results.json`; runtime reload is not proved. Invalid bundle rejection is recorded separately: probe-only exits 1, forced corrupt-package load sets archive error then ASL fatals (139); this is fail-closed rejection, not graceful recovery.
- **Dashboard evidence verification:** `python3 -I porting/material_pipeline/verify_progress_evidence.py all --report porting/generated/material_pipeline/dashboard-material-evidence.json` independently rechecks all four original maps (76 records/44 library shaders), all 44 mapped modules (180 descriptors/64 matched resource entries), four numeric layouts and the earlier eight-case detached draw. `python3 -I porting/material_pipeline/test_export_ingress_bundle.py` passes four tests; a corrupted original mesh record is rejected. `python3 -I porting/progress/test_history.py` passes four tests. `python3 -I porting/server/test_services.py` passes 72 tests (`generated/frontend-probes/dashboard-backend-tests-20261101.log`). The frontend import-manifest command in `asset_pipeline/README.md` verifies six source GLBs against frozen tool/input/output hashes; import, cook and runtime remain unverified.
- **Final artifact command:** `python3 -I porting/material_pipeline/verify_runtime_artifacts.py --fixture porting/generated/material_pipeline/locker-runtime-fixture.json --artifacts porting/generated/frontend-probes/legacy-material-ingress-artifacts-3 --report porting/generated/frontend-probes/legacy-material-ingress-artifacts-3-validation.json` passes all seven payloads/modules. Layout hash is 22544639; original mapped instructions differ only in entry-point names. Current owner readbacks retain the earlier red/green SHA-1 values and 2,832-pixel difference. Original-client pixel comparison remains unverified.

- **Earlier detached-rendering checkpoint:** native `afb32b7`, engine `757e0087a`. Both non-editor targets build; current default and Athena joins pass without diagnostic flags, with matching source/staged binary hashes (`generated/frontend-probes/runtime-material-joins-20261101T021940Z/`). Ledger `34deadf` records those joins/checksum and the detached original-material draw. The root independently validated all seven converted shader payloads and the two RGBA readbacks (`legacy-material-artifacts-2-validation.json`); PNG previews are beside the raw reads. The normal frontend still needs asset ingress/attachment, so all four milestones remain open.
- **Earlier native source `a494a4a`, closure tooling `2d99465`, engine `f716e0cb6`.** Its client and dedicated server builds exit 0 (`generated/frontend-probes/build-original-vulkan-client-guard.log` and `build-original-vulkan-server-guard.log`). Default and Athena smoke joins with freshly built binaries exit 0 without the probe flag in an isolated matched stage; Athena reaches pawn-ready on both peers (`generated/frontend-probes/original-vulkan-joins/`, including results, logs, exit statuses and binary hashes). The broader historical `907e1dc` build and runtime suites pass separately: `generated/frontend-probes/build-getgameversion.log` and `generated/regress/20261101T002525Z-2242896/result.json` prove builds; `generated/regress/20261101T002339Z-2232871/result.json` proves restage, both joins, NetSerialize, 4,617/4,617 offline checksums, 251 behavior splices, 71 backend tests and schema checks. Generator checks pass 141. Earlier run `20261101T002007Z-2216551` has a wrapper FAIL solely because its old mtime rule rejected an already-current client despite Build.sh exiting 0. The external local-model wrapper now records both target hashes and accepts a valid incremental build. Do not hide the failed record or call it a compiler failure.
- **Actual UI dependency closure:** isolated `generated/frontend-probes/closure-fixed` visits 721 packages, 0 failures/0 pending, including Engine content after the corrected loose mount. `runtime-fixed-manifest.json` records the separate stage overlay without overwriting existing converted files. `UIManager_Startup_C` loads as BlueprintGeneratedClass (exit 0); RootLayout_Athena and LoginScreen still crash in original inline material shader-map parsing. The prior banner WidgetBlueprintGeneratedClass also loads with `-FortniteLegacyFieldPaths`; this does not establish widget construction/display or M1.
- **Material recovery is concrete but incomplete.** Primary implementation `8d3a3cb` reads the actual legacy library layout: version 2, 29-byte shader records and contiguous zlib code, unlike public UE 4.25's same-version six-array/LZ4 layout. The complete game index has 213,239 shaders; Global has 1,899. Four failing UI material files contain exact candidate hashes for 44 unique original DXBC shaders; all 44 are recovered. `53678a0` adds reproducible translation; `532d81a` flags diagnostics. All 44 translate and pass Vulkan 1.0 structural validation with both vkd3d 1.2 and locally built official Wine vkd3d 2.0; 12 emit unhandled modifier diagnostics with 1.2 and none with 2.0 (`f3e7403`). The descriptor set, binding and storage-class inventories are unchanged across versions; semantic equivalence is unproved. `df3986e` decodes the original D3D wrapper with exact prefix/DXBC/optional framing for all 44: 19 have active resource-table bits, and the maps contain 28 SRV, 18 sampler, 18 texture and zero UAV entries. `a02640f` inventories 180 translated SPIR-V descriptors across all 44, all in set 0. `c608eab` uses vkd3d scan and explicit interface mappings: the mapped receipt has 44 validated shaders, 180 descriptors, all 64 original resource-table entries matched, zero unmatched and zero compiler diagnostics. It records descriptor-set and binding literal word offsets for UE remapping; an independent audit checked hashes, offsets and counts. Shader 25 at binding base 32 changes only five Binding literal words. The detached Locker bridge now exercises headers and live numeric values as described below; the later Locker quad draw is described below; normal attachment remains unresolved. Reports/code/tools are ignored under `generated/material_pipeline/`; read its tracked README. Tests include seven reader cases, tampered-wrapper rejection, original/native archive reads and the 44-module run. A Luna read-only review independently checked the input/output hashes.
- **Material constants and Vulkan factory:** `e78b1ab` reflects actual mapped modules: 100 uniform buffers, 28 uniform texel buffers, 26 sampled images and 26 samplers, with declared spans and stage interfaces. `9ef42e1` and `9e737fb` evaluate serialized defaults: 96/320/288/192 bytes for Donut/Locker/MenuButton/PowerToast, matching their debug first-resource offsets; six tests pass. `556e382` and `848489c` link 76 parsed Slate and mesh records and 44 Material uniform-buffer bindings through original serialized slots to explicit descriptors and evaluated bytes; all declared spans fit, and three tests pass (`generated/material_pipeline/material-constant-bindings.json`). Floating-point, instance inheritance and resource values remain limited. `0710dfd` builds the client and tests original geometry shaders 26 and 2459 in isolated software Vulkan: real `FVulkanShaderHeader` round trip and RHI factory pass and modified SPIR-V validates, but no Vulkan module, pipeline or draw is created. The successful `generated/frontend-probes/original-vulkan-probe-overlay.log` uses an overlay of 330 cooked Engine files and two matching archives documented in `original-vulkan-stage/overlay-manifest.json`; preserve the three earlier exit-139 graphics-startup logs from stale baseline material/archive files.
- **Next material work:** validate compiler behavior and semantics, reconstruct the decoded shader maps at runtime, bridge those shaders to UE's Vulkan resources, and test real draws against original output. GDB evidence `generated/frontend-probes/gdb-material-cursor-2.log` proves the alleged frozen-content size is ASCII `M_UI` (`0x49555f4d`), not a huge valid blob. `bd1c2e9` locates one original inline resource uniquely against the export map in each of the four failing material packages (bounds in `material_pipeline/README.md`). For Donut the resource spans `.uexp` offsets 2181..10601; the `M_UI` name bytes begin at 2225. A zero word immediately after the shader-map ID hash is read as `MaxFieldAlignment` by the local layout, then the original name length 16 is read as layout flags and `M_UI` as frozen size. The strict parser now decodes all four original serialized shader maps to exact EOF, including packed flags and 19 sequential records per material (seven Slate, 12 LocalVF), zero pipelines and cooking marker 1. All 76 records have archive hash, frequency and uncompressed size validated; 17 isolated tests pass. The texture record is `int32`, `int16`, `int16`, `int32`, `bool32` (16 bytes, as in 4.24); discard the earlier 4.23/five-word comparison and first-unknown offsets. The reports mark complete serialized maps but explicitly do not verify runtime loading. The captured archive Tell 2020 and `.uexp` offsets use different coordinate systems; discard the earlier inferred `+205` origin. PowerToast names parent `M_UI_GradientPanel` as shader identity. Do not replace shaders/materials with invented approximations or skip the read as a parity solution. Original arithmetic graph nodes are stripped. No UI root has been displayed.
- **Other frontend prerequisites:** faithful dynamic GetGameVersion now runs and logs `v4.25 (0)` for this engine's build/branch/CL inputs; the original client log reports `++Fortnite+Release-12.60-CL-13315662` (engine and compatible CL 13315662), despite the workspace folder name `CL-13477524`; network CL is separately 12896485. Reproducing its version string needs project-specific build metadata, rebuild and runtime verification. `22841af` records native helper probe chains for ExecuteClientEvent, GetFrontEndCamera and GetContext; new ignored `frontend-native-evidence` event-registry, event-registration and event-registration-lifecycle evidence identifies GameInstance class `0x1428ea7f0`, FortClientEventInterface `0x142877b10`, and the weak listener registry at `+0x6c0` with registration and removal. The fallback is `HandleClientEvent_Undefined`; lifecycle ownership and argument behavior remain unknown, as do other object and manager identities, preventing faithful implementation. SetInputMode, SetFrontEndCamera and UpdateWatermark also remain open. Preserve evidence under generated/frontend-native-evidence and frontend-next-*. `95f1495` selects native FortGameInstance in config: an isolated stage loaded Entry, showed one live instance and completed a direct join; its UI methods remain largely stubbed. Recover authentic game-instance UI initialization after material/class loading; native defaults still Entry/Athena. No new helper was ported from these probes.
- **Detached Locker runtime bridge:** Engine `757e0087a` implements neutral IR/API, live current preshaders, seven registered Slate constructors and checked known-output-hash finalization with normal freeze/resource initialization; main `4a95007` contains the portable `patches/ue425-legacy-slate-material-bridge.patch` (reverse check passed). Native `2144eba` accepts `-FortniteLegacyMaterialFixture=<.../locker-runtime-fixture.json>` and `-FortniteLegacyMaterialArtifactDir=<fresh existing empty dir>`. The fixture (SHA-1 `3c379d3b0b97bb8e57e9b7b8b7286182ac78a700`) has 220 nodes, seven records and eight cases; 312 meaningful numeric bytes are checked within a 336-byte layout. Non-editor client build passed (821 actions, 403.7 s; incremental 126.7 s); the dedicated server build passed in 436.1 s against Engine `757e0087a` and native `2144eba`. Actual software Vulkan/Xvfb run exited 0, passed eight live `FillUniformBuffer` cases, froze the detached map and constructed seven RHI shaders (`generated/frontend-probes/legacy-material-runtime-1.log`). Independent verification of 14 artifacts and seven actual SPIR-V/payloads passed: instructions unchanged except entry name, header offsets match descriptors, current layout hash 22544639 derives from original shape, and SourceHash/CRC/embedded code match (`generated/frontend-probes/legacy-material-artifacts-1-validation.json`; three tests). Native `afb32b7` adds the opt-in draw; its incremental server build passed in 117.39 s (`generated/material_pipeline/build-runtime-material-server-2.log`), and its actual Vulkan run exited 0: eight pre-freeze and eight post-freeze uniform cases passed, and a 64×64 Locker quad readback changed 2,832 of 4,096 pixels between two `RBGA1` overrides; center pixels were red `(255,0,0,255)` and green `(0,255,0,255)` (`generated/frontend-probes/legacy-material-runtime-draw-1.log`, `legacy-material-artifacts-2/LockerQuad.json` and associated RGBA files). Comparison against original-client rendered output remains unverified. Next prove normal material attachment, required mesh shaders and visual parity. See `material_pipeline/UE_BRIDGE.md`.
- **Content:** six directly imported frontend/lobby meshes now have hash-verified GLBs and original-path import settings (`057df9b`, selection and builder in asset_pipeline). Three exporter plus four manifest tests pass; UE import/cook, original materials/collision and runtime geometry are unverified. The frozen CUE tool snapshot under generated/asset_pipeline/tool-snapshot-frontend protects provenance. Coordinate shared CUE rebuilds before exports. Full extraction remains complete; most usable content, skeletal animation and audio remain open.
- **Backend/multiplayer:** profile-parser, session-key, websocket envelope/Play and ReadyCheck corrections plus party-roster session-key handling pass 72 backend tests (`365693f`). Party delimiter, admission, capacity and error status are local policies, not established Epic behavior; no reconstructed-client end-to-end lobby/matchmaking exists. Server-authority inventory and most gameplay remain unfinished. All four product milestones stay open; dashboard weighted effort remains about 24.10%, not gameplay completeness.
- **Ownership/preservation:** one native/engine/build/stage owner; original extraction is immutable and keys stay out of logs/Git. Preserve five pre-existing uncommitted engine files (gitdeps, three GLTF importer files, Linker) and the CUE4Parse static-mesh edit. Engine's opt-in compatibility branch is codex/fortnite-12.60-compat; never push it to Epic upstream. Main porting repo has no remote. Check for active jobs before starting shared builds or probes.
- **Dashboard refresh:** the post-commit hook now finishes rendering before returning. Normal renders repair missing reachable commit points without replacing historical measurements; use `--seed` to recompute every point. The current smoke/checksum ledger judges the final isolated join logs with wrapper exits and independently checked binary-hash provenance (`34deadf`); `cf75423` is the earlier baseline. Rewritten Vulkan modules also have a root-reviewed receipt at `generated/frontend-probes/original-vulkan-rewrite-validation.json`.
- **Next M1 implementation:** `material_pipeline/UE_MATERIAL_INGRESS.md` (`b0597c2`) identifies the bounded, opt-in provider before the current shader-map deserializer, complete original-record preservation, explicit selected-resource bounds, and actual-owner `IsComplete` validation before normal game-thread attachment. Current non-editor MD_UI LocalVF rules may exclude the original mesh extras, but only the real owner's required layout can establish this. Async deserialization must not call the diagnostic render-command flush. Other layouts, Donut textures, View buffers, compilation flags and Editor identity remain required work.
- **Persistent agent configuration (user update 2026-11-01):** primary plans, orchestrates and reviews, and works directly when useful, including difficult implementation/debugging/problem solving. Use `gpt-6-luna` medium for small read-only tasks; `gpt-6.1-terra` medium or high for regular delegated work (high when complexity warrants); `gpt-6.1-sol` medium for really complex delegated tasks. Always `fork_turns: "none"`, at most three active subagents. Preserve this through compaction and all future sessions. This supersedes the earlier `gpt-6-sol` default. Global/workspace/project AGENTS and project memory contain these instructions.

### Reproduce the current evidence import

From the repository root (the runtime is never launched by this importer):

```sh
python3 -I porting/progress/record_frontend_materials.py \
  --receipt porting/generated/material_pipeline/duplicate-defaults-full118.result.json \
  --manifest porting/generated/material_pipeline/frontend-numeric-batch-11/manifest.json \
  --inventory porting/generated/frontend-probes/frontend-material-inventory-textures/summary.json \
  --manifest-sha256 a38a5e5fdd04637326f662cac4462185d3470734ccc2c3a9b1fa9d886f331523 \
  --inputs porting/generated/material_pipeline/duplicate-defaults-build-inputs-1.json \
  --source-native-commit aeabd2c --engine-commit 8fba83b44 \
  --checkpoint porting/generated/material_pipeline/duplicate-defaults-checkpoint-results.json \
  --checkpoint-sha256 7b63e69b2051cbca44c832e96926e53d331df1a256dc148df0e62b13decf05f6
python3 -I -m unittest discover -s porting/progress -p 'test_*.py'
```

Outcome: 118 passing batch owners imported/upgraded, 119/142 closure owners pass; eight tests pass. The earlier batch-11 reimport was byte-identical; current source-bound import upgrades 118 successes and preserves all unrelated ledger checks. Dashboard reads committed ledger bytes; its post-commit hook renders the new count and retains history.

Everything below is historical context. This snapshot supersedes earlier status,
counts, process claims and next-step instructions when they differ; verify before
following any older command.

Written when the session wound down (usage nearly out). Read `README.md` (goal), `master_plan.md` and
this file first. Everything below is what was true at the last commit on `main`.

## Third session update (2026-10-31, later; supersedes the numbers below where they differ)

Everything here is committed on `main`; the sections below are still correct for workflow and pitfalls.

- **Class parity is 4,572 of 4,572 offline** (`ec52422`; the generator emits net-service RPCs, const RPCs, namespaced enums and
  cross-module types). Build green, both smokes pass, network checksum 683914758 on both peers. `class_checksum.py` is the check.
- **Behaviour: 249 bodies spliced** (10th pass `589e1c1`; the 11th pass added the `struct_net` kind: five recovered
  `NetSerialize` bodies, `b6c483a`). A 39-case round-trip self-test passes (`351dffe`, run the binary with
  `-NetSerializeSelfTest`). `FAthenaPawnReplayData` still uses the marked fallback (its wire depends on runtime state and
  key-dependent calls). UE 4.25 rule: in `struct_net` snippets write `Ar << MyBool;`, there is no `SerializeBool` in non-editor builds.
  The 12th pass (`1fa3ef4`) ported nothing: every remaining target is blocked on unnamed members or helpers.
- **Vtable slot map** (`f2cecde`): `porting/behavior/build_vtable_slots.py` (5 s, writes the git-ignored `vtable_slots.json`) and
  `vtable_lookup.py <class> <hexoffset>`; 90% of slots validated against the dump. Game-declared virtuals have no names.
- **Full asset extraction is done**: all 31 paks, 963,427 distinct files, 88.3 GB, in `/mnt/vibrato/fortnite-extracted-full`
  (verified, duplicates audited; `porting/extraction_status.md`, `Cue4Inspect --extract-all`). The older closure tree is
  `porting/generated/extracted` (symlink to `/mnt/vibrato/fortnite-extracted`). No audio middleware files exist; audio is cooked
  SoundWave bulk (`.ubulk`). Raw extraction is its own dashboard leaf; conversion into usable UE content is barely started
  (77 of 4,719 static meshes, no skeletal meshes or animation).
- **Backend evidence** (`e8942b4`): `porting/server/EVIDENCE.md` and `endpoint_evidence.json` classify each served route
  (binary, wine_log, reflection, asset, convention). No real client capture can ever be obtained (Epic's servers are shut down), so
  the backend must follow what the client binary, the reflected structs and the paks require. 70 tests pass.
  **Open: the client's JSON response parser is not located.** Stock UE 4.25 writes camelCase keys but reads exact PascalCase names,
  so the shipped client uses its own parser; the exe has camelCase key text. Find it from the key-string load sites that
  `evidence_scan.py` lists, then confirm required keys and case sensitivity. 151 client routes in the exe are not served yet;
  `public_catalog` is 2 of 18 and the remaining responses have no struct evidence yet.
- **Server-side behaviour risk** (a design concern, not yet measured): the vanilla client sees only replicated results of server
  logic (storm, bus, damage, eliminations, match end, AI, loot). Much of that logic is in the client exe (server code is compiled
  in: `AFortGameModeAthena`, `Server*_Implementation`, authority branches) and the rest is data in the paks, but logic that exists
  only in Epic's dedicated-server binary would have no source, and timing and RNG cannot be checked without a reference server.
  An inventory (every replicated property and `Client*`/multicast RPC mapped to its server-side writer in the exe) is being built:
  see `porting/server_authority_inventory.md` once it lands.
- **Census** (`census_ui_audio.py`) is a gap-finder, not a prerequisite. Its 48-worker run over 108,701 packages keeps results in
  memory and writes only at the end (no checkpoint, no progress line); it can be paused with SIGSTOP/SIGCONT but a kill loses
  everything. `porting/native_class_coverage.md` (exe class list versus promoted set) replaces it for class selection.
- **Dashboard**: overall about 21.8%; leaf `assets.pak_extraction` added. Re-render with `python3 -I porting/progress/progress.py`
  after a commit (the current hook finishes rendering before returning).
- **Process notes**: the Bash tool moves a foreground command to the background at about 180 s even with a 600000 ms timeout, so
  run UE builds in the background tee'd to a log in `porting/generated/` and wait with one `until ! kill -0 <pid>` loop. Stage
  specific paths when committing; the working tree usually holds other agents' in-flight edits.

### Wind-down state (end of the third session)

- `main` is clean at the last commit; no agent edits are in flight except the package builder (outside the repo). Class parity is
  4,591 of 4,591 (stage 1: the 19 FortniteUI interfaces, `443ef8e`). Class coverage analysis: `porting/native_class_coverage.md`
  (4,591 of 4,763 game classes generated; causes C1 to C5).
- **Next class step (stage 2) failed and was reverted**: SocialUMG and KairosSceneCapture promote but the UE build fails in
  UHT-generated code (a forward-declared FortniteGame class has no header or API macro in the generated `.cpp`). Cause and fix
  are in `porting/native_classes_status.md` ("Coverage stages"). The fix belongs in `gen_native_classes.py`; then retry batch 1
  (KairosSceneCapture goes in the three `*.Target.cs` `ExtraModuleNames`, not FortniteGame.Build.cs, to avoid a cycle), then the
  other 31 modules in batches of one to three; NVIDIAGfeSDK is skipped.
- **Server-authority inventory is PARTIAL** (`porting/server_authority_inventory.md`, `a0fbe1f`): the observable surface is
  complete (1,715 replicated properties, 734 net RPCs, 240 server-to-client); writer attribution is incomplete. Resume with the
  scripts and the saved `scan_c` data in `/opt/fortnite/local-model-package/inventory-wip/` (README there): rerun `analyze.py` and
  `picks.py` on `scan_c` first; the 1.1 GB `full.asm` is regenerated by the objdump step in its README.
- **Local-model package** (work queue, runbooks, scripts for a smaller model): `/opt/fortnite/local-model-package/` (outside the
  repo; start at its `README.md`). The model may commit; the rules there match this file's rules.
- **Census still running** (`python3 -I porting/census_ui_audio.py`, PID 3313931, started about 19:40, 48 Cue4Inspect workers, no
  checkpoint, writes `ui-audio-census.json` only at the end). It is low value (`native_class_coverage.md` replaced it for class
  selection); let it finish, or `kill -STOP 3313931` / `kill -CONT 3313931` to pause it. Killing it loses the run. It slows UE
  builds (about 2x).
- Dashboard: overall 24.07% at `1ca5d96` (denominators recounted from the full extraction; static meshes 77 of 4,727).
- Open questions, in order of value: (1) the client's JSON response parser (see `porting/server/EVIDENCE.md`); (2) the generator
  fix above and the 34 missing runtime modules; (3) server-authority writer attribution, then porting the match-flow behaviours
  (safe zone, game phase, drop zone, supply drops, bots, match end); (4) conversion of extracted assets into usable UE content
  (meshes, textures, skeletal meshes, audio) which is barely started.

## Where things stand

Updated 2026-10-31 at the end of the second session. Everything below is on `main` (HEAD `b9077d8` or later);
`main` is clean, no agent or build is running, and `/opt/fortnite/gen-worktree` is a leftover worktree on
`gen/helpers` (already merged; `git worktree remove /opt/fortnite/gen-worktree` when you no longer need it).

- **Dashboard:** about 20% overall (20.35% at `8c61c28`, 0 of 4 milestones; served on port 8765 when
  `progress/serve.sh` runs). The goal is far from met: no Fortnite gameplay runs, and function bodies are still
  mostly stubs (237 of 20,574 native functions ported). The evidence ledger holds the native join, checksum and
  self-test records recorded at `97d4db3`; re-record after big changes (see "Dashboard").
- **Verified at `eb27249` (build green in 246 s, both smoke variants pass, checksum parity unchanged):**
  - 4,463 classes promoted from the exe's reflection tables, class set of 4,572 (FortniteGame, FortniteUI,
    CommonInput, CommonUI, PlayspaceFramework, McpProfileSys, ModularGameplay, MeshNetwork, ...).
  - Offline class checksum parity with the original: 4,535 of 4,572 (37 remain: struct in another module 16,
    RPC flags without a role 13, unresolved members 5, enums 3).
  - Native client joins the native dedicated server with network checksum 683914758 and an Athena pawn
    (authority 1 on the server, 0 on the client).
  - Behaviour is spliced by the generator (`gen_native_classes.py`; `--no-behavior` or `FN_BEHAVIOR_SPLICE=0`
    turns it off): 237 entries of kinds getter, exec, onrep, impl, validate, helper, static_helper and
    member_field (16 non-reflected fields). Contract in `behavior/README.md`.
  - Function and delegate parameters are generated in binary offset order (`--param-order table` is the old
    order); 91 more delegate members emitted; replication conditions (`replication_conditions.json`, 1,659 of
    1,715 properties at high confidence) and `NetDeltaSerialize` traits for 43 fast-array structs.
- **Not runtime-tested:** the fast-array delta path, the conditional replication and the FText broadcast bodies
  compile and the join works, but no smoke exercises them. Nothing asserts that the C++ layout of the new
  non-reflected fields equals the binary offsets.
- **Known wire gaps:** six used structs with a custom non-delta `NetSerialize` (`SharedRepMovement`,
  `ReplicatedPhysicsPawnState`, `FortAISharedRepMovement`, `FortAIAttributeReplicationProxy`,
  `AthenaPawnReplayData`, `CreateBuildingActorData`) use a marked default-serialization fallback; their bodies
  are not recovered. Push-model flags are not emitted. 51 replication conditions are unknown (`COND_None` + TODO).
- **Backend:** `server/` has a tested local service layer (OAuth, accounts, MCP profile, party, presence,
  lightswitch, cloud storage; `python3 -I porting/server/test_services.py`, 61 tests). Each endpoint is labelled
  observed, inferred or guess in `server_surface.md`; no real client capture has driven them yet.
- **Extraction (AES keys in use):** the keys live in `/home/vibrato/.config/fortnite-port/aes_keys` (outside the
  repo, mode 600; built from `/opt/fortnite/12.60.md`; verified 31 of 31 paks mount). Agents may use them with
  `--key-file=<path>` and must never print, log or commit them. All extractions live in ONE tree,
  `porting/generated/extracted` (git-ignored). State when stopped: about 77,000 files, 8.8 GB, on `/`. The four
  Frontend/lobby entry closures are complete (0 pending); 4,680 packages recovered by the census; all 2,175
  registry WidgetBlueprints are extracted (`generated/widget-blueprints.txt`). The seeded soft-reference closure
  (`--save-closure --seed-list generated/widget-blueprints.txt --follow-soft`) was killed at about 77k files with
  a large frontier; re-running is safe (files are rewritten) and the queued-set edit in `Program.cs` makes the
  pending count unique. Census numbers in `ui_audio_census.md` are from before the soft closure (4,680 packages);
  `census_ui_audio.py` already has `widget_presence()` for the next run.

## How to resume

1. Check `git status` and `git log`; `main` should be clean. Rebuild only if you changed generator inputs:
   ```sh
   bash porting/promote_native_classes.sh      # regenerates native/Source/*/Generated (splice on)
   cd /opt/fortnite/UnrealEngine
   ulimit -c 0
   Engine/Build/BatchFiles/Linux/Build.sh FortniteLinux FortniteLinuxServer Linux Development \
     -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject -NoHotReload
   ```
   Pass an explicit Bash tool timeout of 600000 ms (the default is 120 s and the build would move to the
   background). Restage by COPYING the two binaries into
   `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux` (no `BuildCookRun -archive`; it
   would clobber the stage). The staged client needs the CEF3 libraries hard-linked (`native/README.md`,
   "Staging the client"). A full build takes about 4-5 minutes.
2. Run BOTH smoke variants and require exit 0 (use `env -u LD_LIBRARY_PATH`, `stdbuf -oL -eL`, and a `rg`
   shim that execs `grep -E` on PATH; `rg` is not installed; a shim lives in the old session scratchpad):
   ```sh
   bash porting/server/smoke_join.sh
   bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
   ```
3. Run `python3 -I porting/class_checksum.py` and `python3 -I porting/behavior/check_behavior.py --gen-root
   porting/native/Source/FortniteGame/Generated --splice-report porting/generated/reflection/behavior_splice.json`.
   `check_behavior.py` does NOT compile-check: only a build catches a snippet error (the ninth pass found three
   UE 4.25 access problems that way: `UWorld::GameState` and `AActor::Role` are private, and the delegate
   spelling is `TMulticastDelegate<void, Args...>`).
4. Parallel work: generator edits run in a git worktree (`git worktree add /opt/fortnite/gen-worktree -b <branch> main`,
   symlink `porting/generated` and `porting/native/Plugins` read-only) while one agent owns `porting/native/` and
   the UE build. Merging a generator branch can conflict on `decompilation_notes.md` (every pass appends a
   section); keep both.

## Next steps, in order

1. **Finish extraction.** Move `porting/generated/extracted` to `/mnt/vibrato/fortnite-extracted` (871 GB free;
   `/` has about 70 GB and needs a 12-20 GB floor for builds): stop all Cue4Inspect processes, `rsync -a`, verify
   file count, bytes and a sample with `cmp`, rename the old dir, symlink the same path, test one small
   `--save-closure` through the symlink, then delete the old dir. Then run the seeded soft closure to frontier 0
   in the background with a log, re-run `python3 -I porting/census_ui_audio.py`, update `ui_audio_census.md`
   and use its ranked list of missing `/Script` classes (`FortSocialNameTextBlock`, `FortKismetLibrary` is
   hand-written, plugin classes for Paper2D, Niagara, GameplayAbilities) to extend `native_class_set.txt`.
2. **More behaviour.** Unported but unblocked-by-naming: `OnRep_Kills`, `OnRep_AIKills`, `OnRep_DeathInfo`
   (global Broadcast `0x140684de0` unidentified), `OnRep_TotalPlayerScore`, `OnRep_TeamKillScore`, and the
   PlayerController/PlayerState callers (each needs its own `static_helper` for `GetFortGameStateAthena`).
   Queue and identities: `behavior/CALLEE_PORT_QUEUE.md`, `decompilation_notes.md` (Eighth and Ninth pass).
3. **Recover the six fallback NetSerialize bodies** and the push-model flags; add a runtime test of replication
   conditions and fast arrays.
4. Remaining class blockers: 16 structs in other modules, 13 RPCs without a role, unresolved members.
5. Everything else on the master plan: Frontend/lobby UI from the extracted widget graph, audio playback,
   cooked Blueprint loading (field-path crashes), Apollo streamed levels, skeletal meshes and animation, and
   driving the backend with a real client capture.

## Still to do

- **Ghidra project and dump** live under `/opt/fortnite/ghidra-projects/fortnite-slim` and
  `porting/generated/dump/` (git-ignored). Do not re-run the full auto-analysis: it dies of heap
  exhaustion (Address Table analyzer). The slim analysis (about 25 minutes, 48 GB heap, address-table and
  decompiler analyzers off, `ghidra/SlimAnalysisOptions.java`) is the working configuration.

## Hard rules and decisions

- **Observed original-client access gate.** The Wine run stopped at `CheckEntitledToPlay`.
  Offline checks (`class_checksum.py`) and native client/server joins provide partial evidence;
  they do not establish original-client wire parity. The runtime dump carries no keys.
- The supplied `*.id0/.id1/.nam` IDA files next to the exe are untrusted (unknown source). Not used.
- The exe's `.text` is encrypted on disk. Code was recovered from a read-only memory dump of our own Wine
  run (`dump_runtime_image.py`, `dump_to_pe.py`, `binary_analysis.md` section 6). Never write to that process.
- No keys or credentials in tracked files. Never force push. There is no git remote.
- Stage specific paths when committing. Subagents do not commit.
- Subagents: at most 3 at once, one owner for `porting/native/` and the UE build.

## Pitfalls that cost time

- The Bash tool default timeout is 120 s; pass `timeout: 600000` explicitly for UE builds or they move to the background.
- `check_behavior.py` does not compile; a snippet written in the old reversed delegate parameter order passes it
  but breaks the build. Read the generated header for the signature first.
- A `git merge` of a generator branch can conflict on `decompilation_notes.md` (every pass appends a section); keep both.
- IpNetDriver is **UDP**: check `ss -lun`, not `ss -ltn`. Redirected server stdout is block-buffered; use
  `stdbuf -oL -eL`.
- Never wait with `pgrep -f '<pattern that appears in your own command>'` or `pkill -f` with such a
  pattern: it matches itself, hangs or kills your own shell. Wait on a PID (`kill -0`) or a log line.
- Run UE builds in the foreground with the tool timeout set to 600000 ms, no `timeout` wrapper; a build
  longer than the tool limit is silently moved to the background and the agent "stops".
- `ulimit -c 0` before every build or test: crash tests leave multi-GB core files.
- Smoke script pawn check runs only in the Athena variant; run both variants.
- Wine runs need an idle machine: under UE-build load the client may not reach the Frontend in 90 s.

## Dashboard

- Model: `progress/components.json`; rendered by `progress/progress.py` (post-commit hook). Denominators in
  `progress/totals.json` (refresh with `python3 -I porting/progress/refresh_totals.py` after extraction
  changes). Measures: `checks`, `census`, `routes`, `tracked_count`, `tracked_regex`, `ledger`, `fraction`,
  `local_glob`; `README.md` has the objectivity audit and which leaves are still manual (each with a ceiling).
- **The evidence ledger holds the native join, checksum and self-test records** (recorded at `97d4db3`; `progress.py` reads
  the committed tree, so commit `progress/evidence.json` after re-recording). Re-record on an idle machine after
  a change that affects them:
  ```sh
  python3 -I porting/progress/record_evidence.py smoke_default -- bash porting/server/smoke_join.sh
  python3 -I porting/progress/record_evidence.py smoke_athena -- bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
  ```
  The `wine_*` checks (`progress/README.md`, "Standard suite") need a run of
  `porting/run_original_direct.sh 240` first, then `--log` against the client log; `wine_entitlement_passed`
  and `wine_lobby_reached` have no recorded passing result.

## Storage

- `/opt/fortnite/FortniteClient-12.60-CL-13477524/Fortnite` is a symlink to `/mnt/vibrato/Fortnite` (the 86 GB
  original client; checksum-verified copy). UE source and build state stay on `/` (about 85 GB free).
- The Water plugin is Epic's 4.26 plugin adapted to 4.25; it is not in Git. Recreate it with
  `bash porting/prepare_water_plugin.sh`; regenerate its patch with `porting/regen_water_patch.sh`.

## Map of the tooling (all under `porting/`)

| Area | Files |
|---|---|
| Exe analysis | `binary_analysis.md`, `extract_reflection.py`, `extract_types.py`, `extract_native_funcs.py`, `resolve_class_parents.py`, `resolve_class_refs.py`, `class_checksum.py`, `validate_types.py` |
| Ghidra (PyGhidra, repo venv `porting/.venv`) | `ghidra/` (class names, type xrefs, decompile, improve, callee probe), `ghidra/name_tables/` |
| Generation | `gen_native_classes.py` (+ `.md`), `promote_native_classes.sh`, `native_class_set.txt`, `native_class_blocklist.txt`, `gen_stubs.py`, `native_classes_status.md` |
| Behaviour | `behavior/` (snippets, manifest, checker, bool bit decoder, blocking callees) |
| Backend | `server/` (mock Epic services, matchmaking, websocket, smoke tests) |
| Original client | `run_original_client.sh`, `run_original_direct.sh`, `direct_connect_results.md` |
| Progress | `progress/` |
