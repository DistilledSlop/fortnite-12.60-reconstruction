# Common analog cursor Tick and eligibility

Source-only recovery; no generated or engine source was modified. The mapped
image is `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
mapped base `0x140000000`. Reads used explicit address/length disassembly and
small literal/float reads, without reading or hashing the whole image.

| Original | Recovered source |
| --- | --- |
| `1407fef70` | `FCommonAnalogCursor::Tick`: visibility first, gamepad gate and viewport eligibility, base accelerated cursor or focus-following branch, then independent right-stick scroll dispatch |
| `1407ec060..1407ec1b2` | `NativeUpdateCursorVisibility` |
| `1407cf210..1407cf45b` | `NativeShouldHandleInput` |
| `1407cff60..1407cff97` | `IsRelevantInput(FAnalogInputEvent)` |
| `1407cffa0..1407d0075` | key relevancy, with Virtual_Accept capture-mode fallback |
| `1407f8f80..1407f926e` | normalized viewport positioning and zero-position fallback |
| `1407b8c50..1407b8cfd` | ListView, ScrollBar, ScrollBox orientation dispatch, vertical fallback |
| `1407aa7a0..1407aa8c7` | active route recursion and invalid routed-widget pruning |

The Tick source uses real UE 4.26 public APIs verified against local headers:
`FSlateUser`, `ITableViewMetadata::GatherSelectedRows`, `ITableRow::AsWidget`,
`IGameLayerManager::GetPlayerWidgetHostGeometry`, geometry transforms, and
`ProcessMouseWheelOrGestureEvent`. `1407c5780` returns metadata literal
`ITableViewMetadata`; it is different from the CommonButton metadata getter.
Static class getter literals at `14683e54a`, `14684352a`, `146842f32` identify the
three orientation types as ListView, ScrollBar, ScrollBox. `1407c1750` gets the
UCommonUIInputSettings CDO: scroll fields +68/+70 map to AnalogCursorSettings
ScrollDeadZone/ScrollMultiplier. Scroll's denominator is intentionally
`ScrollDeadZone - 1`; Tick decrements +d0 but does not reset that countdown.

Constants: `1459c03a0` = 1e-8 (size/scroll epsilon), `1459a86f0` = 0.5,
`1459ffe78` = 0.04 (right-stick length squared), `1459c03a4` = 1e-4 (matrix
comparison tolerance). Translation components are compared exactly.
Fallback normalized position at `148218628` is (0,0).

The `1459fc448` vtable establishes the actual overloads: +60 inherits pointer
relevancy, +68 overrides analog relevancy, +70 overrides key relevancy, +78
inherits FInputEvent relevancy. Do not override FInputEvent with the analog gate:
that would affect mouse processing and defeat the key capture-mode fallback.

Integration: add `friend class FCommonAnalogCursor;` to the router for actual
NativeActionInputConfig/NativeActiveRouteRoot access. Add scroll members/bodies
to FCommonActionRouteNode; include the tick member fragment and cpp in the
cursor source overlay. Remove the earlier FInputEvent override declaration.
Include Rendering/SlateRenderTransform.h and forward declare UWidget in the
cursor header. Existing Router, PressedMouseKeys, d4/d5/d6 aliases are reused.

Validation is source/API review, not an engine build. The surrounding generated
overlay still requires primary-agent integration. Initialization virtual +98
(`1407cd650`) is separate from Tick and must be retained by the constructor/
factory surface; it applies settings and binds the input-method callback.
