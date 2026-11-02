# CommonButton Initialize producer and immediate inherited lifecycle

Candidate only, restricted to this directory. No generator/native/Engine edits,
promotion, generation, build, staging, runtime execution, or commits. Root owns
integration/builds. Preserve bounded delegated-task rotation, economical reads,
and Luna medium / Terra medium-high / Sol medium with fork_turns none.

`definitions.inc` is the complete producer at original `1407cd7d0`, including
parent false return, transactional ConstructWidget, settings, weak store,
nonnull-old-root containment and **all five** original bindings in order.
It directly copies UButton's click/touch/press/focus fields, rather than adding
setter side effects absent in the original. The two native enable setters already
exist in the generated internal button and implement the original live Slate
mirrors and effective interaction-change notifications. Neither the internal
allocation nor AddChild/slot cast gains an invented failure guard.

`declarations.inc` adds Initialize and three source aliases for the unreflected
bytes +2a2/+2a3/+2a4. Original constructor `140796950:140796b69` never writes
these bytes; UObject zero initialization supplies value 0 (DownAndUp for all
three original enum roles). These remain real shared instance fields, not local
constants in Initialize. Existing native interaction/button bits, inherited
bIsFocusable and weak NativeInternalButton are reused.

Null old root deliberately leaves the constructed internal only weakly reachable
from CommonButton, never replaces root, and never binds callbacks. The internal
may expire through GC. Outer WidgetTree is not a strong child-retention edge.
There is no AddToRoot, UPROPERTY strong internal, cached strong pointer, invented
AllWidgets insertion, detached forced state or lifecycle reset. Nonnull old root
gets wrapped through the real panel slot, padding 0, fill/fill; tree RootWidget
then strongly owns internal, its Slots strongly own slot, and slot Content strongly
owns old root. Target WidgetTree::ConstructWidget uses RF_Transactional for this
UButton-derived class and does not add a runtime retention array. AllWidgets is
WITH_EDITORONLY_DATA and populated by PreSave, not ConstructWidget.

## Newly recovered immediate inherited methods

`original-policy.json` pins 8 bounded windows, 21 relevant vtable entries, four
names and original class/callback metadata. No full image read/hash was performed.
Prior full-image provenance receipt is
`/mnt/vibrato/fortnite-work/common-button-initialize-closure-1/HANDOFF.md`.
`disassemble.py START END` seeks only the requested memory-layout image window.

Original CommonUserWidget and UUserWidget share Initialize `143d64300`,
RebuildWidget `143d801e0`, ReleaseSlateResources `143d83b50`, and BeginDestroy
`143d46330`. Therefore **reuse the real target engine parent methods**, without
new CommonButton overrides of those three lifecycle methods:

* Complete parent Initialize `143d64300:143d6447d` checks +1b4 bit 0x20 and
  returns false immediately when initialized. Otherwise it sets the bit **before**
  any initialization callbacks, inherits owning-user-widget player context,
  initializes blueprint widget data or native class virtual +2d0, constructs
  transient WidgetTree if missing (+1a8), marks non-cooked tree transient, runs
  named-slot initialization, and dispatches NativeOnInitialized virtual +308 when
  player context is valid. It returns true after that call. Parent callbacks may
  reenter before the CommonButton weak store; a reentrant Initialize is rejected
  by the already-set bit. Candidate must call Super before constructing internal.
* Target UUserWidget::Initialize in
  `Engine/Source/Runtime/UMG/Private/UserWidget.cpp:315` matches this ordering.
  Its CanInitialize archetype/CDO checks are inside WITH_EDITOR || UE_BUILD_DEBUG;
  shipping CanInitialize is true. Target design-time guards are editor behavior;
  original shipping valid-context path dispatches directly. Keep the normal
  engine parent gate rather than invent a second initialization flag.
* Original inherited RebuildWidget starts at `143d801e0`, tests that same bit,
  and calls virtual Initialize +2c0 only when not initialized (`143d80217:22d`).
  Then valid player context propagates through the actual tree. Its root path
  at `143d802ba:310` invokes the root TakeWidget helper; a null root constructs
  a spacer. Target UserWidget.cpp:880 has the same dispatch/ownership shape.
  Our pinned window covers entry through null-root construction setup, not the
  entire shared-pointer cleanup suffix; no suffix parity claim is needed here.
* Original ReleaseSlateResources `143d83b50:70` checks WidgetTree and RootWidget
  then tail-dispatches root virtual +260 with the release-children argument.
  It does not clear +ad0 or replace root. Target UserWidget.cpp:472 delegates to
  the inherited base and releases root through GetRootWidget. Existing internal
  ReleaseSlateResources handles its real Slate pointers. Retain these methods.
* Original BeginDestroy `143d46330:3c6` calls UObject parent cleanup, virtual
  ReleaseSlateResources(false), virtual RemoveFromParent +280, pins its weak
  SObjectWidget and calls reset helper `143d85500` if live. It has no write to
  CommonButton weak internal. Target UserWidget.cpp:425 follows its inherited
  release and RemoveFromParent/reset path. GC invalidation uses the engine's
  normal object-index/serial weak pointer; no CommonButton clearing hook is added.
* Original UUserWidget OnWidgetRebuilt `143d70ab0:143d70b23` traverses the root
  for navigation, then virtual +310 NativePreConstruct and +318 NativeConstruct.
  Target UserWidget.cpp:910 does likewise outside editor design time. A rebuilt
  tree does not rerun CommonButton Initialize once the parent flag is set.

CommonButton has its own OnWidgetRebuilt, NativeConstruct and NativeDestruct
(as pinned in the vtable table), whose action registration/listener/metadata
semantics are separate from this immediate producer. Their known source is in
`common-button-lifecycle-slots-1` and `common-button-construct-bindings-1`; this
candidate does not replace them with parent-only overrides or claim full widget
lifecycle parity. Root should integrate the producer with the authentic called
receivers/selection/style batch and retain the broader lifecycle work on its
actual dependency path, rather than invent retention in this producer.

## Exact integration hooks and receiver dependency

Import policy, run `validate_original(original_memory_image)` once in the bounded
source batch and `validate_classes(blocks_by_va, name_to_block, delegates)` before
model mutation. After the existing CommonButton style/click/double-click/selection
policies, call `apply(model, block, owner_fns, native_execs)`. Append declarations
and definitions through the same fragment mechanism as those policies. After the
complete final header/cpp have been rendered, run
`validate_output(model, button_header, button_cpp)` before any writes. Do not
register a reflected Initialize function; this is the actual native override.

Binding dependencies are real reflected `void HandleButtonClicked()`,
`void HandleButtonPressed()`, `void HandleButtonReleased()`; existing native
`FReply NativeHandleDoubleClicked()`; and sibling native
`void NativeHandleFocusReceived()` from `common_button_focus_selection_runtime`.
Native aliases are project source names, not claims of recovered original C++
spellings. UObject native delegates and AddUniqueDynamic keep weak receivers;
no callback lambdas or placeholder bodies are supplied here. Class policy pins
original owning CommonButton's zero-parameter signatures, including selecting
its unique HandleButtonClicked candidate from the ambiguous shared-name record.
Rendered-output policy requires all five definitions and weak internal storage;
that structural check alone does not prove the receiving graph complete. Root
must review/validate that sibling's full called bodies before registration.

Validation completed:

```
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_button_initialize_runtime/policy.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin
# PASS: 8 bounded source windows, 21 lifecycle slots, 4 strings
python3 -m py_compile porting/frontend/common_button_initialize_runtime/{policy,disassemble}.py
# exit 0
```

A one-shot metadata call loaded the original reflection JSON,
selected classes_in_groups_with_fort_names into blocks_by_va, then called
`policy.validate_classes(blocks, {'UCommonButton':'0x145a01210'})`:
PASS original class identity and all three CommonButton callback signatures.
This is source/metadata validation, not generation, compilation or UE execution.
