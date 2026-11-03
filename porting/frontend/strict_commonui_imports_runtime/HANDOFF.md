# Strict CommonUI input import component

Only this new directory changed. No generator registration, Native/Engine edits,
generation, build, stage or commit. Root owns integration and actual UE validation.

## Panel import component

`policy.py` supplies executable source for `HasInputActionHandler` together with
all 9 registration entry points, both removals and the actual native destructor.
The query reads the existing real `NativeActionRecords` map (+360) and recovered
`NativeActionBindingRows` map (+310). Neither is a query-only replacement map.
The shared producer calls the already-maintained
`UCommonUserWidget::NativeRegisterActionBinding`, including its actual router
association and widget handle array. Existing resolved entries suppress duplicate
registration; stale signed IDs are removed before trying again. Failed actual
router registration, or an explicit popup, creates the complete legacy Panel
record used by the existing held/click consumers. Both route and legacy removals,
held cleanup, actual manager+78 notification, and destructor cleanup are included.

Source behavior preserved: invalid row is table=null AND name=None; query checks
legacy before routed map; no popup forces the router attempt before row-validity
fallback; routed descriptors do not receive legacy progress delegates; the public
DataTable-based Add methods reject null DataTable; remove-one prefers legacy when
both maps contain the key; remove-all clears legacy, notifies, then unregisters
routed; destructor clears records and panel dynamic activation delegates before
the inherited actual widget teardown.

Integration: call `validate_classes(blocks_by_va, name_to_block, native_funcs)`
and `validate_original_image(image)` before writes. Call `apply` after existing
Panel intro/outro/manager suspension policies; splice `header_prelude`,
`declarations`, `definitions`; call `validate_dependencies(models_by_name)` after
all models exist, then `finalize_header(model, header)` after header rendering,
and `validate_output` afterward. The finalizer inserts the single +310 native map
immediately before the existing +360 NativeActionRecords declaration. It does not
create a duplicate legacy map. Their actual C++ construction order is routed then
legacy, with implicit destruction in reverse; this does not assert shipping ABI
byte offsets. The new map has no UPROPERTY: its values are registry IDs and its
row references follow the original unreflected map ownership. Existing legacy
record/weak popup/delegate ownership remains owned by the outro component. The dependency gate
requires the real connected route policy on CommonUserWidget, action router and
activatable widget, and actual manager notification member. This policy does not
activate the continuation fragments below.

`python3 porting/frontend/strict_commonui_imports_runtime/test_policy.py` PASS:
3 policy cases, all 12 function/exec record mutation checks, dependency/collision
and rendered-source checks, 21 bounded original byte windows. HasInputActionHandler's
invalid-row warning is retained exactly under LogCommonUI: original category
1480d1df0, literal1459ef2c0, constructor1401c5610 (defaultLog5/compileAll7).
All 12 manually declared functions are removed from the skipped-function inventory;
their names are available in model.strict_commonui_panel_functions_emitted without
adding duplicate generated declarations. This is static
verification, not a UE compile/runtime test. Root must compile the coherent batch
and exercise registration, fallback, remove-one/remove-all, reentry and teardown.

## Button/global-handler continuation: source, not activated

`button-*.inc` supplies the actual triggering setter, setup/teardown, construct/
destruct listener ordering, dynamic callbacks and progress-material updates.
`CommonLegacyGlobalActionNative.h` and `global-handler-*.inc` supply the actual
GlobalInputHandler+50 row-owner array and complete registration/removal. A repeat
registration replaces commit and priority but appends progress/complete listeners;
sorting happens on both branches. Exact sort compares differing rows' FNames;
equal rows compare live-owner status and inherited UUserWidget.Priority, not the
stored InputPriority argument. Remove compares full row and resolved weak owner,
removing all matches stably. This is not the Panel+360 map.

**Correction to older `common_button_public_producers_runtime/HANDOFF.md`:**
SetTriggeringInputAction does not return merely because its old row is non-null.
1407fa9e0 calls14418a080(existing,incoming), and14418a080 is full row inequality.
The setter runs for any changed row: teardown, copy, fresh virtual setup+488,
fresh virtual action widget update+4a0. The new source uses this exact condition.

`legacy-lifecycle-*.inc` and `legacy-subsystem-declarations.inc` recover the real
CommonUISubsystem enable/disable producer1407f7b90 and manager PostInitProperties
1407e1ed0. Creation honors the caller's selected policy; it creates the actual
global handler, assigns weak owner/input subsystem, binds the real suspension
callback and emits NativeLegacyInputManagerChanged. Disable removes the binding,
marks the manager pending kill, clears the field and emits false. These fragments
are intentionally NOT enabled by policy.py because the manager's actual tickable
and legacy held-input consumer closure remains absent. No no-op tick, empty
manager or unconditional registration fallback was added to pass strict imports.

## Concrete remaining closure for button/modal

Manager Tick must call the queue's NativeTickOperationSuspension and, only when
false and both held-interface words exist, invoke the actual native interface
held tick. The interface's proper seven slots and both concrete implementations
must be emitted; it cannot be replaced with casts to guessed vtable offsets.
Native member names below are descriptive; original interface source names were
not recovered.

* Global constructor140796d50 installs its action-handler interface at object+28,
  vtable1459f1338. Slots0/8/10/18/20/28/30 are1407a5da8,140695430,
  1407c8be0,1407cab10,1407cc2b0,140802440,140800b70.
* Panel constructor140807e00 installs its action-handler interface at object+2d8,
  vtable1459ee038. Slots0/8/10/18/20/28/30 are1407a5d84,1408056ac,
  1407c87e0,1407ca7f0,1407cbf10,140802200,140800860.
  The earlier1459ee010 is another secondary table, not this interface's start.
* Panel tick140802200:23b increments held duration, resolves the actual row,
  gets current input+gamepad data, finds actual legacy record and requires hold
  enabled/time>0. Broadcast clamped progress; strictly duration>HoldTime completes
  (not >=), then complete multicast, popup RequestClose (1407f17d0), commit with
  reused sparse-map-index initial out byte, then real NativeClearHeldAction.
* Global tick140802440:3e9 checks held owner weak validity AND cached Slate widget
  before incrementing. It finds full row-owner record, resolves row/input type,
  requires hold enabled/time>0, broadcasts clamped progress, and on strictly
  duration>HoldTime calls actual owner eligibility1407d0160 before complete,
  commit(initial bool false) and1407ae750. A missing cached owner widget also calls
  1407ae750. Other invalid lookup/hold conditions leave held state intact.
* Global clear1407ae750 does not simply null the manager interface: it clears
  global held owner, row and duration, then stores this global handler's actual
  interface pair back into its Outer manager. This must remain a real interface.
* Eligibility1407d0160 first checks GetIsEnabled and, for CommonButton, actual
  interaction helper1407cf620, then continues through Slate widget/geometry input
  eligibility. It is roughly0x4f0 bytes and the latter half is not yet translated.
* The three input dispatch slots funnel through1407c8d40; full generic input
  dispatch, hold producers and state lookup140800b70 remain to translate. Merely
  adding the registration array with no consumers must not activate the setter.
* The subsystem's original Deinitialize1407b8610 removes its manager suspension
  binding and marks manager pending kill without clearing/broadcasting. It is
  distinct from disable; don't implement Deinitialize as SetLegacyInputEnabled(false).
  Original Initialize1407ce490 clears four global byte variables; it creates no
  manager. The actual FortUIManager selected policy caller14515a790 remains the
  required wiring point; never force-enable legacy input.
* FortRestartClientModal remains disabled: full queue candidate is available,
  but manager interface/tick lifecycle above is still required. Its exit-flag
  producer144f7b54f must be independently checked at call
  sites; no fabricated default-only source was enabled here.

`legacy-continuation-original.json` pins 19 bounded implementation/continuation
windows and three virtual entries. Reads used os.pread or bounded decoder calls;
no whole-image load/hash. Decoder:
`PYTHONPATH=/tmp/common-input-capstone python3
/mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA:SIZE`.
Original image is `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
VA base140000000. The continuation source has not been compiled and should be
reviewed against these exact windows before future promotion.
