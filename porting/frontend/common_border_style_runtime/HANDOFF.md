# CommonBorder SetStyle candidate (2026-11-02)

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
and minimum padding, sets bShowEffectWhenDisabled false and Visibility to
SelfHitTestInvisible. Confirm inherited property offsets in the integration batch.

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
