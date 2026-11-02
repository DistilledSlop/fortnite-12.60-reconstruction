# CommonButton focus, hover, selection and accepted click source candidate

Unregistered and uncompiled candidate only. `declarations.inc`, `definitions.inc`,
`routing-definitions.inc`, and the required panel friend insertion are intended as
one coherent generator batch, not standalone replacements for existing APIs. They
use existing style, selected and base click policies and the actual action widget,
CommonInput settings and panel action map. No Native/generated files, builds or
runtime work were touched. The generator must prove reflection signatures, reject
collisions and require the action-widget receiver, style, selected, click, panel,
CommonInput module and Initialize policies; it must insert the panel friend as well.
The row producer SetTriggeredInputAction and other selection producers still need
real registration and side effects. Until those policies and a combined GUI test
exist this is not production-ready and Initialize must not bind it.

Original 1407c6960 gate encloses all three operations, not only selection. Focus
1407c83d0 checks the original predicate 1406801e0, which is exactly `xor al,al;
ret` in this image; bShouldSelectUponReceivingFocus still gates selection and
hover dispatch. Original input-type 2 is Touch. Hover native slots b08/b20 and
focus b38 are multicasts, not singlecasts. Hover calls real UUserWidget::Invalidate
(Layout) then fresh text-style and action-widget virtuals. Selection rereads its
state after style callbacks; the internal weak pointer is dereferenced without a
new guard in original 1407f7830. Preserve these callback and weak lifetime details.

Action update selects TriggeringInputAction then NativeTriggeredInputAction then
module-owned settings' DefaultClickAction only when hovered; invokes actual
SetInputAction then visibility refresh. Visibility refresh resolves input before
reading state and reloads InputActionWidget before writing NativeHidden and calling
the real refresh. Native routing scans the real panel stack backwards and stops on
the first active matching map entry even if the commit delegate is unbound. The
original passes the raw low byte of its map slot as initial bool storage. The candidate
copies the byte into one-byte bool storage and passes it by reference without a C++
bool value read; its output is copied back as a byte and discarded. This requires
validation with a native callback that inspects and changes raw storage, as C++ does
not guarantee the interpretation of noncanonical bool representations. It accesses the same
private NativeActionRecords as panel outro; `panel-friend.inc` is required.

Review requirements before promotion: independently pin the Slate singleton identity and module null assumptions against
original, ensure
SetInputAction refresh candidate and panel active accessor match the target build,
check native event signature reflection, ensure SetIsSelected/SetSelectedInternal/
ClearSelection producer wiring is real, and resolve any UHT/compiler differences.
Read the complete source-evidence and handoffs in button-base-click-contract-1,
button-click-context-resolver-1, panel-commit-producer-trace-1,
button-action-widget-update-1, button-native-multicast-correction-1,
common-button-selection-interaction-refresh-1, common-button-selected-transition-
contract-1, common-button-initialize-closure-1 and the maintained click, selected,
style, action widget and module policies. `NativeTriggeredInputAction` has no
producer here and cannot be treated as a fabricated default. Exact original
windows inspected without a full image hash: 1407c83d0..846c, 1407c6ed0..6f62,
1407c6960..69d1, 1407f9880..994f, 140803050..30ed, 1406801e0..1e2,
1407d2cd0..2d2a, 1407d4290..42ea, 140801d90..1e7a, 1407f7830..78b1,
1407f7e90..7f3c, 1407bd360..d49e. Command for each:
`PYTHONPATH=/tmp/common-input-capstone python3 /mnt/vibrato/fortnite-work/common-button-initialize-closure-1/range.py START END`.
Bounded source and metadata guards were run successfully as described below. No build,
generation or runtime test was run; the candidate is subject to source review.

Sound: original 1407f98f4 loads global 148271318, passes NormalStyle pressed
sound +5a8 and user index zero to 143022250, which loads its receiver +330,
loads that object’s vtable and tail-calls slot zero. Target engine
SlateApplication.cpp:885-887 implements PlaySound as SlateSoundDevice->PlaySound
with exactly the same two forwarded arguments; FSlateApplication::Get() returns
CurrentApplication and SButton.cpp:450-458 calls it for pressed/hover sounds. The
identity of original global 148271318 as CurrentApplication is a strong structural
inference, not independently pinned: until joined, sound mapping is not proven.

`policy.py` guards fifteen original instruction spans via `os.pread` only, complete
reflected class property/event records and native-exec joins, helper collisions and
rendered shared dependencies. Original instruction-window and metadata checks
PASS via `python3 porting/frontend/common_button_focus_selection_runtime/test_policy.py`,
including rejection of mutated property, native exec and source bytes; no full image hash. The range.py command
shown above unfortunately reads the full mapped image into memory even though it
prints only bounded ranges; it did not hash it. Future source reads must use
`os.pread` instead. The original records and span digests are in original-policy.json.
NativeCommit has no maintained producer or reflection bridge yet. The original
concrete dynamic delegate 1407bc8a0 reads the raw byte, normalizes it with setne,
invokes ProcessEvent, and copies the result byte back. Any future adapter must use
byte-copy normalization before reflection, never evaluate a noncanonical bool.
Add a focused raw-byte delegate fixture in the coherent integration batch.

Input settings metadata independently ties InputDataClass to original offset 0x188
and UCommonUIInputData.DefaultClickAction to 0x28; policy guards both exact records.
A bounded 4 MiB scan at 0x143000000 found 145 loads of global 148271318 in the
Slate code region; that correlation is not by itself a singleton identity proof.

Fallback cache call order: bounded original 140802fe9 calls module getter
1407c4fc0 then directly invokes 14081b6f0, which reads settings+188, validates
class ancestry, gets/creates/reloads its CDO and copies +28; its complete
14081b6f0..14081b789 body contains no call to cache validation or loading. Module
virtual+40 is the proven field getter (140680340), and module GetCommonInputSettings
implements that lookup. Thus this particular original path does not invoke the
separate 14081d830/14081c1e0 cache validation/initialization path; inserting
NativeGetCurrentPlatformData here would add observable cache and load effects.
The candidate now preserves the original CDO creation/reload and null check.
If another lifecycle path is expected to prepare InputDataClass, it needs its real
producer; this callback must not synthesize a new cache preparation step. The
existing protected `ValidateNativeInputDataCache()` is the exact cache-validation
hook when independently proven as a caller, and NativeGetCurrentPlatformData()
invokes it for the original platform-data getter.

Generator hooks: before writes call `validate_original_image(image_path)`,
`validate_classes(blocks, name_to_block, native_funcs)`, then `apply(model,
block, owner_fns)` after style/selection/click policies, and `validate_output`
with rendered button, panel and action-widget headers. Splice declarations and
definitions only for the applied model, insert `panel-friend.inc` in the panel,
and separately gate the already-integrated action-widget runtime/module/cache and
Initialize lifecycle. `apply` deliberately requires the extra model flag
`common_button_feedback_sound_identity_proven`; no supplied policy sets it. Root
must add evidence-backed validation of original global 0x148271318 and sound
receiver 0x143022250 before setting that flag. Do not bypass it for a build.
