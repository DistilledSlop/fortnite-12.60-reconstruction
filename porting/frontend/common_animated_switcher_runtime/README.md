# Common animated Slate switcher candidate

`SCommonAnimatedSwitcher.{h,cpp}` reconstructs the real Slate subclass at
original constructor 0x14078ef30 (vtable 0x1459ee5e0), Construct 0x1407b2260,
request 0x140800440, timer 0x140803cb0, transform scalar 0x1407c5610 and paint
0x1407db9b0. It uses the actual SWidgetSwitcher and FCurveSequence, including
one half-duration eased curve, weak target identity, phase, transition timer,
synchronous callbacks and interruption/re-entry ordering. Its scalar, opacity,
transform and visibility constants come from original instructions and bytes.
It does not hardcode the owning container's duration or transition settings.

`container.{declarations,definitions}.inc` shows the owning container's
original +0x1a8 overlay, +0x1b8 collapsed spacer and +0x1c8 switcher lifetime,
RebuildWidget 0x1407e7d70, release 0x1407ee760, switcher creator and the
0x1407c19a0/0x140800440 adapters. The rebuild binds int32 callback 0x1407c61f0
and bool callback 0x1407cbc30, passes reflected TransitionType,
TransitionCurveType and TransitionDuration, puts switcher then spacer in overlay,
and adds an initial SNullWidget slot after the overlay is built. The bool
callback sets the spacer Visible for true (original global byte 0x19) and
Collapsed for false (0x02). Release resets the three Slate shared references
in order and then the FUserWidgetPool Slate cache. Setter callers are in
root_layer_clear_runtime/modal-queue.definitions.inc; merge its declaration
with container.declarations.inc without duplicating the three switcher accessors.

The source also supplies the index callback 0x1407c61f0, Slate-to-activatable
resolution 0x1407a8ae0, removal 0x1407eefa0, captured deactivation callback
0x1407c65f0, and the one-shot ticker 0x1407bc5a0. Its weak ticker releases
strong refs retained in the native deferred array; the active index callback
broadcasts a real native displayed-widget multicast after the activation path.
The constructor fragment initializes pool ownership and duration 0.4, and sets
the initial UWidget visibility Collapsed. The original ctor writes byte +0xc3
with 1; the vtable's UWidget SetVisibility target 0x143d91830 writes the same
byte from ESlateVisibility, independently proving its identity. Original
missing-target and invalid-pending diagnostics use LogSlate at Verbose; the
category initializer 0x140489590 names LogSlate at 0x146644b00.

The constructor, member declarations, native delegates and container source
fragments require generator integration, and must be merged with the queue
clear component (which owns NativePendingDeactivation1e8). The original queue
add/remove producer and the broader startup providers remain separate work.

Original source evidence: /mnt/vibrato/fortnite-work/animated-switcher-request-interruption-1/HANDOFF.md,
animated-switcher-visual-tick-1/HANDOFF.md, activatable-queue-rebuild-contract-1/HANDOFF.md,
and queue-rebuild-arguments-1/HANDOFF.md. Additional bounded instruction
windows and original bytes are pinned in original-windows.json; no whole-image
read or digest was performed. Source has not been compiled or runtime tested.


`policy.py` provides bounded `validate_source`, complete class/property and
ClearWidgets native-exec joins via `validate_classes`, enum joins via
`validate_types`, per-model `apply`, post-include-reset `configure_model`,
`declarations`, `definitions`, support `sources`, and `validate_output`.
Container `apply` requires the caller to supply the exact
`REQUIRED_DEPENDENCIES` set after it has established the real
UCommonActivatableWidget activation/deactivation and native deactivation
delegate. It emits ClearWidgets as the original BlueprintCallable function,
the constructor, and native fields in original relative offset order. Queue
`apply` validates its original identity and parent. The policy owns merged
modal-queue code and must replace, not accompany, the duplicate fragments from
root_layer_clear_runtime. Call the exported NativeClearModalQueue1407ae7f0
through CommonAnimatedSwitcherNative.h across module boundaries. Support files
and all class fragments still require generator integration and a UE batch build.
