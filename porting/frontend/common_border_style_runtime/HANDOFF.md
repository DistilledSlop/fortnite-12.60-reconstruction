# CommonBorder setter graph integration candidate (2026-11-02, corrected)

Generator hooks are ready but are not registered, generated, compiled, or run.
`policy.py` supplies `validate_original(image)`, `validate_classes(blocks,
name_to_block, native_funcs)`, `validate_types(type_gate)`, `apply(model, block,
owner_fns)`, `declarations(model)`, `definitions(model)`, and
`validate_output(border_model, border_header, border_source, style_model,
style_header)`. Register all of these together. `validate_output` requires the
style model/output too; omission must fail integration. Definitions include the
required Slate and style headers. CommonUI already depends on Slate and SlateCore.
No generated or Engine files were changed in this task.

**Constructor correction:** 0x14079686f clears inherited `Background.DrawAs`,
not `bShowEffectWhenDisabled`. UBorder Background reflection offset 0x158 plus
SlateBrush.DrawAs offset 0x6c equals 0x1c4; original enum NoDrawType is zero.
The reflected bShowEffectWhenDisabled setter thunk 0x143c98170 writes bit 1 at
0x122, so no derived-constructor write to that field is present. Preserve the
UBorder constructor default. UWidget Visibility reflection offset 0xc3 and
ESlateVisibility enum value 4 establish SelfHitTestInvisible. Both enum records,
brush member, inherited field metadata, setters, class accessors and original
code windows are now pinned. `bReducePaddingBySafezone` thunk writes byte 0x278.
The UE 4.25 headers expose DrawAs, GetDefaultObject(false), GetGameViewport,
SViewport::GetViewportInterface, ISlateViewport and GetSafeZoneSize as used;
actual compilation is still required.

The bounded original replay and exact class/function/native/type gates passed on
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
and current `porting/generated/reflection` inputs. A changed reflected offset
and duplicate model application were rejected. Commands: `python3 -m py_compile
porting/frontend/common_border_style_runtime/policy.py`; Python import of the
policy, `gen_native_classes.TypeTables`, and invocation of the four original,
class, type and model hooks (2 Nov 2026). The image was read only at bounded
windows and pointer locations; no whole-image hash. Real GUI/runtime validation
of repeated and null setters, existing versus missing CDO, brush copy, virtual
reentry, and safe-zone behavior remains for the integrating owner. This graph is
not full CommonBorder lifecycle parity: rebuilding/releasing widgets and safe-zone
change notifications remain separate work.

# Earlier candidate notes (superseded where the corrected section differs)

Unregistered and unbuilt candidate. `definitions.inc` covers the setter, widget
constructor, virtual synchronization and immediate safe-zone padding helper.
It requires the generated class to declare an object-initializer constructor,
`UFUNCTION(BlueprintCallable) void SetStyle(TSubclassOf<UCommonBorderStyle> InStyle)`,
`virtual void SynchronizeProperties() override`, and private
`void NativeUpdateSafeZonePadding()`; source needs UCommonBorderStyle.h,
Framework/Application/SlateApplication.h, Widgets/SViewport.h,
Widgets/Layout/SBorder.h and the ISlateViewport declaration. The standard
FSlateBrush constructor matches the observed 0x140796890 style constructor's
32x32, zero margin, white tint and Image draw default; no invented CDO is used.
The widget constructor 0x140796840 calls UBorder, zeroes Style, safe-zone flag
and minimum padding, sets Background.DrawAs to NoDrawType and Visibility to
SelfHitTestInvisible. The earlier bShowEffectWhenDisabled attribution was wrong; see the correction above.

Original class records 0x145a01330 (CommonBorder) and 0x145a01290 (style),
class accessors 0x140807640 and 0x140807530. Function block 0x147831b20,
outer 0x140807730, create 0x140807810, flags 0x4020401, Class parameter
InStyle meta accessor 0x1408077f0 -> 0x140807530. Native array 0x145a043b0
contains `SetStyle` string 0x145a03128 and exec 0x14080d670, which calls
receiver 0x1407f9c50. Receiver stores the raw class unconditionally, then
checks MyBorder and calls virtual slot 0x298; null and equal values still store
and still synchronize when live. Slot of constructor-installed vtable
0x1459efb98 resolves to override 0x1407fdf60. The override type-checks Style,
reads its existing CDO without creating it, type-checks the CDO, copies its
Background into inherited Background, then calls UBorder synchronization and
padding helper 0x1407f24b0. No CDO or incompatible class leaves old Background
intact. Synchronization is virtual and may reenter; do not cache state across it.

Helper uses GetGameViewport, pins its viewport interface and calls
GetSafeZoneSize with its pixel size if available. Zero margin otherwise. Only if
bReducePaddingBySafezone and MyBorder exist, it sets the Slate border padding to
per-side max(MinimumPadding, Padding - SafeMargin), without altering Padding.
The parent synchronization runs first. Further lifecycle entry points and safe
zone change notifications remain outside this narrow setter dependency endpoint;
do not claim full CommonBorder parity. Verify the engine's corresponding layout
and exact constructor visibility field before promotion.

`policy.py` validates bounded source windows, vtable and native-name/executable
join only; it is not a generator policy and does not validate metadata or model
collisions. Integration must fail closed on exact owner, complete original function
and property metadata, class/source joins, declarations, dependencies, rendered
output and reentry; compile and run a real Slate border flow with repeated/null
styles, CDO availability, brush copy and safe-zone padding. No full image hash,
generation, native edits, build or runtime execution occurred here.

Bounded source read: `python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py` on code windows listed in `original-policy.json`, plus exec/class accessor, virtual and padding helper windows. Replay PASS:
`python3 -c "import importlib.util; s=importlib.util.spec_from_file_location('p','porting/frontend/common_border_style_runtime/policy.py'); m=importlib.util.module_from_spec(s); s.loader.exec_module(m); m.validate_original('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); print('bounded original replay PASS')"`.
