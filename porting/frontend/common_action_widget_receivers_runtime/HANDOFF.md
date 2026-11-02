# CommonActionWidget receiver candidate

Generator-ready fragments, unintegrated and unbuilt. Implements the key setter,
both reflected action setters, native weak multicast binding/callback, and refresh.
Shared Slate graph and constructor are owned by the sibling Slate candidate;
GetIcon and explicit-subsystem brush helper are owned by the existing icon candidate.
The lifecycle verified marker is valid only when all three are integrated together.

Fresh owner/subsystem resolution is deliberate at each remove and rebind, and twice
in successful refresh. The first subsystem selects first-row hold info, the second
builds the icon. Keep the callback's refresh-before-broadcast, repeated collapsed
visibility calls, and unguarded progress/box usage after icon validity. The action
array setter takes by value to preserve the reflected exec's owning copy boundary;
the single-row setter resets the array retaining its allocation, as in the original.
No registry key ownership is assumed. Real multicast RemoveAll/AddUObject preserve
mutation/weak UObject behavior. Gate uses the live CommonInput.ShowKeys getter.

Policy `validate_source(image_path)` checks the key lifecycle contract and three
additional SHA-256 bounded windows via pread, no whole-image read. `apply(model,
block, owner_fns, native_execs)` checks class/reflection/native setter joins, field,
and collisions; emit `declarations(model)` and `definitions(model)`. Source replay:
`python3 porting/frontend/common_action_widget_key_lifecycle_evidence/replay.py`
PASS, `python3 porting/frontend/common_action_widget_refresh_gate_evidence/replay.py`
PASS; a direct `validate_source` on the retained evidence image PASS. No build or
runtime claim. Root integrates and owns generation/build; no staging or commits.

Shared aliases: NativeBindingKey, NativeHidden (declared here, initialized by Slate
constructor); NativeBox, NativeIcon, NativeProgress, NativeCachedBrush (Slate);
NativeGetIconForSubsystem(UCommonInputSubsystem*) const (icon). Header needs
UCommonInputSubsystem declaration and ECommonInputType definition/forward enum;
source needs CommonActionBindingData.h, UCommonInputSubsystem.h,
CommonInputModuleRuntime.h, FCommonInputActionData.h, Engine/LocalPlayer.h,
Widgets/Images/SImage.h, Widgets/Layout/SBox.h. Root must verify include visibility,
UHT, compile and integrated frontend behavior.

## Additional reflected APIs (2026-11-02)

`GetDisplayText`, `IsHeldAction`, and `SetIconRimBrush` are now in the same
candidate and guarded by exact class-owned reflection and native joins. Display
and hold receivers resolve the subsystem before checking the array, resolve only
row zero with the original diagnostic, call type then gamepad even for keyboard
or touch, and use the selected info's hold flag. Display returns the original
FText copy (hold/normal) or the engine empty FText. The gamepad index remains
unchecked. Rim exec `14080c510` copies the brush into its receiver argument;
`1407f67d0` copies that value into `IconRimBrush` and destroys the argument. It
has no Slate setter, explicit invalidation, refresh, or visibility update. The
Slate rim already references that member. These code paths are pinned along with
all three exec wrappers and empty-text constructor by seven additional bounded
windows in `refresh-windows.json` (29 total with the inherited lifecycle windows).

Direct policy `validate_source` and `apply` against retained image and live
reflection/native metadata: PASS, five reflected APIs emitted. No build or runtime
execution. Root owns integration and validation.
