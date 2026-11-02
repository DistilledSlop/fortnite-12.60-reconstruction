# CommonActionWidget brush and gamepad getter candidates

The unreflected `NativeGetGamepadInputType()` is an independent source alias for
`0x14081b530`: return the existing `GamepadInputType` byte. It is not a UFUNCTION.
Hook `validate_source(image_path)` and `validate_types(gate)` before generation,
`apply_gamepad(model, block, owner_fns)` on the subsystem model, and append
`gamepad_declarations(model)` and `gamepad_definitions(model)` to the generated
header and source. Its field and exact enum metadata, owner, source bytes and
name collisions are guarded. Normal generated includes already define the enum.
This getter does not implement gamepad state producers.

`icon-definitions.inc` is the source-shaped receiver and full brush calculation
for original `0x1407c2ee0`/`0x1407c2f50`; it is **not ready to register**.
`apply_icon` rejects generation until a separately recovered lifecycle policy sets
`common_action_binding_key_lifecycle_verified` and provides a signed member
`NativeBindingKey`, initialized and mutated with original semantics. That marker
must only be set after proof and implementation of all relevant writes and
binding/unbinding. Do not bypass it or add a permanently -1 member. Weak owner
resolution uses the existing record and the actual UWidget owner contract of
`CommonActionBindingData::Create`. It performs subsystem lookup before rows,
keeps the original per-row getter order and unchecked gamepad index, and calls
the final getters without a null-subsystem guard. Both failure paths share one
lazy static `FSlateNoResource`. Row lookup retains the original diagnostic text.
Settings come from the actual module instance and perform the source-proven cache
validation/preparation before the platform copy. Integration additionally needs
`CommonActionBindingData.h`, `UCommonInputSubsystem.h`, `UCommonInputSettings.h`,
`CommonInputModuleRuntime.h`, `CommonInputControllerBrushLookup.h`,
`FCommonInputActionData.h`, `FCommonInputPlatformData.h`, and
`Engine/LocalPlayer.h` in the generated source, and the corresponding module
build dependencies. Run `validate_source`, `validate_types`, and
`apply_icon(model, block, owner_fns, native_execs)` with the original registration;
append `icon_definitions(model)` to source and check its rendered presence. The
policy guards owner, ambiguous original metadata, InputActions and registration;
the source proof is in `../common_action_widget_icon_candidate/HANDOFF.md`.
These fragments have not been compiled or runtime tested.

SetInputAction and refresh must not be stubbed. `0x1407f6820` changes the key and
binding state, and `0x140802150` unbinds/rebinds when changing keys; their full
reentry and receiver lifecycle are not yet established. Refresh additionally has
an unidentified global gate and unresolved Slate object/method lifecycle; see
`/mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/HANDOFF.md`.
The existing widget has no recovered state graph. This candidate supplies no
refresh, no widget constructor, no lifecycle claim and no user-facing completion.

Validation: `python3 -m unittest discover -s porting/frontend/common_action_widget_runtime_candidate -p 'test_*.py'`;
source replay reads bounded windows from the original mapped image. Existing
`python3 porting/frontend/common_action_widget_icon_candidate/replay.py` also
checks the original reflection and native join. No native, generator, build or
runtime files were changed here.
