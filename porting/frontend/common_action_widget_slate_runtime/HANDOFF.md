# CommonActionWidget Slate lifecycle candidate

Unintegrated and uncompiled candidate; no native/generator/build/commit changes.
Root owns integration and must review it in the coherent receiver/Slate batch.
The paired receiver candidate supplies `NativeBindingKey`, `NativeHidden`,
`NativeRefreshInputAction()` and `NativeBindInputMethodChanged(bool)`; its
policy marker is `common_action_widget_receivers_runtime`. This policy requires
that marker. Only the combined integration may set the existing icon policy's
`common_action_binding_key_lifecycle_verified`, after confirming all mutations,
callback, Slate hooks, refresh and button producers are present.

Constructor `140796260:291` default-constructs both reflected brushes and cached
brush as FSlateBrush, key -1, empty arrays/shared pointers and hidden false;
then assigns `IconRimBrush` from the lazy FSlateNoResource (`1407c3e30`).
The fragment uses a local static; integration should share the icon candidate's
`CommonActionWidgetNoResource()` singleton so both original uses share one object.
ProgressMaterialParam and ProgressDynamicMaterial are UObject-zero-initialized;
the original constructor does not explicitly overwrite them. The four shared
members are declared in box/icon/progress/rim order, then the cached brush, to
preserve reverse destruction order; the receiver's key and bool are trivial.
No explicit destructor or delegate removal is invented.

`1407e6fc0` initially checks ProgressDynamicMaterial; when null it examines
ProgressMaterialBrush resource: a dynamic material skips creation and leaves
ProgressDynamicMaterial null; a non-dynamic UMaterialInterface creates a dynamic
instance with null outer and stores it in both the property and brush; other
resources set the property null. Static-class identities match the original
reflection index, `144796ba0` UMaterialInstanceDynamic and `1447976c0`
UMaterialInterface; `144324750` is the create receiver, `142fd1200` brush
SetResourceObject. Construct box centered, then progress, icon, rim SImages
with unbound brush pointer attributes, then centered overlay slots rim/icon/
progress, zero default padding, SOverlay default visibility, box content
assigned last. Keep local strong refs: original returns the locally created box
and uses locally created images, even if member state changes during construction.

OnWidgetRebuilt calls Super, refresh, then bind(true). Release resets progress,
icon and box in that order, calls bind(false), which freshly resolves the current
owner/subsystem and removes all bindings, then calls Super. Rim is not reset by
release. The binder must preserve exact receiver semantics. Brush references in
Slate point into the UObject; normal lifetime and actual widget teardown matter.

Generation: run `validate_source(image_path)` against mapped original, then
`apply(model, block, owner_fns)` after receiver policy, append `declarations()`
and `definitions()`, and `validate_rendered()` before output. Add header forward
declarations for SBox/SImage (or their includes) and source includes
`Widgets/Layout/SBox.h`, `Widgets/SOverlay.h`, `Widgets/Images/SImage.h`,
`Brushes/SlateNoResource.h`, `Materials/MaterialInterface.h`, and
`Materials/MaterialInstanceDynamic.h`; engine Slate/SlateCore dependencies must
exist. Integrator must verify constructor emission and policy collisions, and
batch a build plus affected actual UI flow. `NativeHidden` and key declarations
belong to receiver; do not duplicate them. No claim of runtime or visual parity.

Validation: `python3 porting/frontend/common_action_widget_slate_runtime/replay.py`
PASS, exact reflection and six bounded SHA-256 code windows via seek/read; no
whole-image hash. Command `python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py 140796260:260 1407e6fc0:cd0 1407ee5e0:154 1407dff40:40 > /tmp/widget-slate-dis`
and targeted `1407964ae:80 144796ba0:60 1447976c0:60 144324750:70 142fd1200:40`
provided disassembly. Existing key evidence and scratch Slate mapping handoffs
contain vtable and graph proofs. Unresolved: no compiled/source-rendered policy
validation or runtime verification yet; constructor, engine Slate API details,
material assumptions and preservation of destructor/reentry behavior need review
in the integration. No feature completion is implied.
