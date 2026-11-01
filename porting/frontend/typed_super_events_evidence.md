# Original typed Blueprint super events

The image identities are documented in [login_status_event_evidence.md](login_status_event_evidence.md). The original class link, constructor, FFunctionParams and all property bytes below match in normalized runtime.exe and raw runtime.bin. Source reflection identifies owner/function and provides ordered property arrays; byte checks below independently validate outer, constructor links, flags, property types/flags, FName string, object flags, dimension, offset or bool callback and referenced type constructor. All 11 FFunctionParams have no return property. Raw parameter array order is kept distinct from C++ argument order.

## `/Script/CommonUI.CommonButton:OnActionProgress`

Class `0x145a01210`, owner `0x140805cb0`, function `0x140806e70`, FFunctionParams `0x1478338d0`, size 4, flags `0x8080800`. Raw FFunctionParams:

```text
b05c80400100000000000000000000005829a04501000000000000000000000000000000000000000400000000000000080ba0450100000001000000450000000008080800000000
```

Properties in original array order:

- `HeldPercent` at `0x1478338a8`: `8028a04501000000000000000000000080000000000010000a000000450000000100000000000000b05c8040010000000000000000000000`.

## `/Script/CommonUI.CommonButton:OnTriggeredInputActionChanged`

Class `0x145a01210`, owner `0x140805cb0`, function `0x140806fc0`, FFunctionParams `0x147833a50`, size 16, flags `0x8480800`. Raw FFunctionParams:

```text
b05c8040010000000000000000000000382aa04501000000000000000000000000000000000000001000000000000000280ba0450100000001000000450000000008480800000000
```

Properties in original array order:

- `NewTriggeredAction` at `0x147833a18`: `202aa04501000000000000000000000082010008000010001900000045000000010000000000000080286f44010000000000000000000000`.

## `/Script/FortniteUI.EmergencyNoticeBase:ShowNotice`

Class `0x146ecd3e0`, owner `0x1451e6f80`, function `0x1451e70a0`, FFunctionParams `0x147e6d2a0`, size 48, flags `0x8480800`. Raw FFunctionParams:

```text
806f1e4501000000000000000000000028e8ec4601000000000000000000000000000000000000003000000000000000a0d3ec460100000002000000450000000008480800000000
```

Properties in original array order:

- `Body` at `0x147e6d248`: `f0a9a14501000000000000000000000082010008000010001d0000004500000001000000180000006051b445010000000000000000000000`.
- `Title` at `0x147e6d270`: `6051b44501000000000000000000000082010008000010001d0000004500000001000000000000000000000000000000806f1e4501000000`.

## `/Script/FortniteUI.FortConfirmationWindow:ConfigureConfirmationButton`

Class `0x146f02cc0`, owner `0x145215cf0`, function `0x145215d90`, FFunctionParams `0x147e8fb40`, size 192, flags `0x8480800`. Raw FFunctionParams:

```text
f05c2145010000000000000000000000683ef0460100000000000000000000000000000000000000c000000000000000a82bf0460100000003000000450000000008480800000000
```

Properties in original array order:

- `bSimpleConfirm` at `0x147e8f758`: `583ef04601000000000000000000000080000000000010002c000000450000000100000001000000c0000000000000008062744001000000`. SetBit `0x140746280`: `c681b800000001c3`.
- `Action` at `0x147e8fab8`: `e4f524460100000000000000000000008201000800001000190000004500000001000000080000004050b2420100000070c29e4501000000`.
- `Button` at `0x147e8fae8`: `70c29e4501000000000000000000000080000800000010001200000045000000010000000000000000788040010000002040f04601000000`.

## `/Script/FortniteUI.FortConfirmationWindow:ConfigureDeclineButton`

Class `0x146f02cc0`, owner `0x145215cf0`, function `0x145215dc0`, FFunctionParams `0x147e8fbf0`, size 8, flags `0x8080800`. Raw FFunctionParams:

```text
f05c2145010000000000000000000000883ef04601000000000000000000000000000000000000000800000000000000c02bf0460100000001000000450000000008080800000000
```

Properties in original array order:

- `Button` at `0x147e8fb88`: `70c29e450100000000000000000000008000080000001000120000004500000001000000000000000078804001000000e048f04601000000`.

## `/Script/FortniteUI.FortErrorWindow:OnOverrideCloseButtonText`

Class `0x146f20c20`, owner `0x14522c380`, function `0x14522c3b0`, FFunctionParams `0x147e9f8a0`, size 24, flags `0x8480800`. Raw FFunctionParams:

```text
80c32245010000000000000000000000d01ef24601000000000000000000000000000000000000001800000000000000380bf2460100000001000000450000000008480800000000
```

Properties in original array order:

- `OverrideText` at `0x147e9f870`: `c01ef24601000000000000000000000082010008000010001d000000450000000100000000000000000000000000000080c3224501000000`.

## `/Script/FortniteUI.FortLetoLayoutBorder:UpdateBorderWidth`

Class `0x146fc4dd0`, owner `0x1452c3e10`, function `0x1452c3ea0`, FFunctionParams `0x147f12ad0`, size 4, flags `0x8080800`. Raw FFunctionParams:

```text
103e2c45010000000000000000000000505ffc4601000000000000000000000000000000000000000400000000000000904dfc460100000001000000450000000008080800000000
```

Properties in original array order:

- `HalfBorderWidth` at `0x147f12aa0`: `405ffc4601000000000000000000000080000000000010000a0000004500000001000000000000000000000000000000103e2c4501000000`.

## `/Script/FortniteUI.FortRestartClientModal:OnButtonSet`

Class `0x146fc2220`, owner `0x1452bfd00`, function `0x1452bfd60`, FFunctionParams `0x147f0ece0`, size 2, flags `0x8080800`. Raw FFunctionParams:

```text
00fd2b45010000000000000000000000f831fc4601000000000000000000000000000000000000000200000000000000e021fc460100000002000000450000000008080800000000
```

Properties in original array order:

- `bShouldShow` at `0x147f0ec68`: `3099e84601000000000000000000000080000000000010002c0000004500000001000000010000000200000000000000e079754001000000`. SetBit `0x1407579e0`: `c6410101c333d248`.
- `InbShouldExit` at `0x147f0eca0`: `e831fc4601000000000000000000000080000000000010002c0000004500000001000000010000000200000000000000a066694001000000`. SetBit `0x1406966a0`: `c60101c34881ec08`.

## `/Script/FortniteUI.FortToastDisplayArea:OnDisplayToast`

Class `0x147000850`, owner `0x145300cb0`, function `0x145300da0`, FFunctionParams `0x147f3d510`, size 8, flags `0x8020800`. Raw FFunctionParams:

```text
b00c30450100000000000000000000000024f24601000000000000000000000000000000000000000800000000000000e80600470100000001000000450000000008020800000000
```

Properties in original array order:

- `NewToast` at `0x147f3d4a8`: `10150047010000000000000000000000800000000000100012000000450000000100000000000000606cb242010000002018004701000000`.

## `/Script/FortniteUI.FortVoiceChannelChangePopup:OnInputModeChanged`

Class `0x14701c210`, owner `0x14531fef0`, function `0x145320240`, FFunctionParams `0x147f54480`, size 1, flags `0x8080800`. Raw FFunctionParams:

```text
f0fe31450100000000000000000000009015a04501000000000000000000000000000000000000000100000000000000e0be01470100000002000000450000000008080800000000
```

Properties in original array order:

- `NewInputType` at `0x147f54418`: `c0eaa04501000000000000000000000080000000000010001e00000045000000010000000000000020ee81400100000010a39a4501000000`.
- `UnderlyingType` at `0x147f54448`: `10a39a4501000000000000000000000000000000000000000000000045000000010000000000000000000000000000000000000000000000`.

## `/Script/FortniteUI.FortWebLoginWidget:DisplayWidget`

Class `0x14701c570`, owner `0x1453208a0`, function `0x145320900`, FFunctionParams `0x147f55790`, size 8, flags `0xc020800`. Raw FFunctionParams:

```text
a0083245010000000000000000000000b8b02e460100000000000000000000000000000000000000080000000000000030c501470100000001000000450000000008020c00000000
```

Properties in original array order:

- `WebWidget` at `0x147f55758`: `e83943460100000000000000000000008000080000001000120000004500000001000000000000005036dc43010000000000000000000000`.

## Parameter order and scoped resolution

`bSimpleConfirm` SetBit `c681b800000001c3` writes byte 184: C++ order Button@0, Action@8, bSimpleConfirm@184. `InbShouldExit` SetBit `c60101c3` writes byte 0; `bShouldShow` SetBit `c6410101c3` writes byte 1, opposite the serialized property order. Title@0 precedes Body@24. Const-reference properties carry `0x0010000008000182`. The source enum class and its underlying byte are established in [splash_input_event_evidence.md](splash_input_event_evidence.md). Exact UHT output and any generated transitive struct closure require controlled validation; none is evidence of audio/UI behavior.

The `0x0000000000080000` object-property bit is `CPF_InstancedReference`; `CPF_ExportObject` is `0x8` (CoreUObject `ObjectMacros.h`). Original class flags for `UVisual`/`UWidget`/`UCommonButton` are respectively `0x3000a0`/`0xb000a1`/`0xb010a1`, all including `CLASS_DefaultToInstanced` (`0x00200000`). Current Engine `Visual.h` declares `UCLASS(DefaultToInstanced)` and the widget classes inherit it. UHT `HeaderParser.cpp:6813` disallows both instancing flags during parameter type parsing; object-property creation at lines 839–842 subsequently adds InstancedReference for a DefaultToInstanced class without adding ExportObject. Current generated `PanelWidget.gen.cpp` AddChild `UWidget* Content` and `FOnTabButtonRemoved.gen.cpp` `UCommonButton* TabButton` both show the identical original `0x0010000000080080` flags. No special parameter specifier or class flag change is warranted; the new event UHT artifacts still require direct verification.

The real TypeGate model for original `/Script/FortniteGame.FConfirmationDialogAction` is size 176, alignment 8, fields `DisplayName` FText at 0, `ResultName` FName at 24, `Icon` FSlateBrush at 32, and `ActionName` FName at 168. Its only referenced modeled type is the existing `/Script/SlateCore.FSlateBrush`, included via `Styling/SlateBrush.h`. The expected generated closure therefore adds `FConfirmationDialogAction.h` with this Engine include; an isolated generation and actual UHT size/offset proof are required before relying on that result.
