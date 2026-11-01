# Original parameterless Blueprint super events

The fixed-closure header inventory and classifier are described in [native_super_import_probe/README.md](native_super_import_probe/README.md). The original normalized PE and raw memory dump and their hashes are identified in [login_status_event_evidence.md](login_status_event_evidence.md). For each row below, the two images have identical class blocks, class-link arrays, function-constructor instruction slices and 72-byte FFunctionParams. The class link pairs the exact function constructor and original name pointer; FFunctionParams outer matches the owner constructor, property-array pointer, property count and struct size are all zero, object flags are `0x45`, and the function flags match the table. These source facts identify declarations, not their runtime behavior.

| Target | Class block | Owner constructor | Function constructor | FFunctionParams | Flags |
|---|---|---|---|---|---|
| `/Script/CommonUI.CommonActivatableWidget:BP_OnActivated` | `0x145a004e0` | `0x1408079b0` | `0x140805d90` | `0x147830d50` | `0x8080800` |
| `/Script/CommonUI.CommonButton:BP_OnClicked` | `0x145a01210` | `0x140805cb0` | `0x140806710` | `0x147832cf0` | `0x8080800` |
| `/Script/CommonUI.CommonButton:BP_OnDisabled` | `0x145a01210` | `0x140805cb0` | `0x1408067a0` | `0x147832d90` | `0x8080800` |
| `/Script/CommonUI.CommonButton:BP_OnEnabled` | `0x145a01210` | `0x140805cb0` | `0x140806800` | `0x147832e30` | `0x8080800` |
| `/Script/CommonUI.CommonButton:BP_OnUnhovered` | `0x145a01210` | `0x140805cb0` | `0x140806890` | `0x147832f20` | `0x8080800` |
| `/Script/CommonUI.CommonButton:OnActionComplete` | `0x145a01210` | `0x140805cb0` | `0x140806e40` | `0x147833860` | `0x8080800` |
| `/Script/CommonUI.CommonButton:OnCurrentTextStyleChanged` | `0x145a01210` | `0x140805cb0` | `0x140806ea0` | `0x147833920` | `0x8080800` |
| `/Script/FortniteUI.EmergencyNoticeBase:HideNotice` | `0x146ecd3e0` | `0x1451e6f80` | `0x1451e7070` | `0x147e6d200` | `0x8080800` |
| `/Script/FortniteUI.FortFriendSubscriptionToastWidget:OnPlayCloseAnimation` | `0x146f2c4c0` | `0x145235570` | `0x145235650` | `0x147ea4b80` | `0x8080800` |
| `/Script/FortniteUI.FortFriendSubscriptionToastWidget:OnPlayOpenAnimation` | `0x146f2c4c0` | `0x145235570` | `0x145235680` | `0x147ea4c00` | `0x8080800` |
| `/Script/FortniteUI.FortPlayerPowerRatingToast:ShowToast` | `0x146f9f600` | `0x1452a12b0` | `0x1452a13d0` | `0x147efb820` | `0x8080800` |
| `/Script/FortniteUI.FortVoiceChannelChangePopup:OnPlayCloseAnimation` | `0x14701c210` | `0x14531fef0` | `0x1453202a0` | `0x147f544d0` | `0x8080800` |
| `/Script/FortniteUI.FortVoiceChannelChangePopup:OnPlayOpenAnimation` | `0x14701c210` | `0x14531fef0` | `0x145320310` | `0x147f54520` | `0x8080800` |
| `/Script/FortniteUI.FortWebLoginWidget:DismissWidget` | `0x14701c570` | `0x1453208a0` | `0x1453208d0` | `0x147f55710` | `0xc020800` |

The raw 72-byte FFunctionParams, in table order:

```text
b0798040010000000000000000000000d019a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c80400100000000000000000000002026a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c80400100000000000000000000005026a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c8040010000000000000000000000c826a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c80400100000000000000000000002027a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c80400100000000000000000000004029a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b05c8040010000000000000000000000a829a04501000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
806f1e4501000000000000000000000018e8ec4601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
7055234501000000000000000000000048dbf24601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
7055234501000000000000000000000060dbf24601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
b0122a450100000000000000000000005005fa4601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
f0fe314501000000000000000000000048dbf24601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
f0fe314501000000000000000000000060dbf24601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008080800000000
a008324501000000000000000000000050db014701000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008020c00000000
```

Flags `0x08080800` require protected BlueprintImplementableEvent declarations. `DismissWidget` has `0x0c020800`: public, BlueprintCallable, Event and BlueprintEvent. None is Native. UHT generates the forwarders; no native body is inferred. Exact UHT flags and class loading require controlled-build verification.

The following 11 remaining absent non-Native events have parameters. Their declaration emittability and exact UHT flags are pending separate type, reference and return-signature verification. The rows list decoded serialized records, including any enum underlying-type record, not necessarily argument count.

| Target | Flags | Serialized parameter records (name:kind; referenced type constructor when present) |
|---|---|---|
| `/Script/CommonUI.CommonButton:OnActionProgress` | `0x8080800` | `HeldPercent:Float` |
| `/Script/CommonUI.CommonButton:OnTriggeredInputActionChanged` | `0x8480800` | `NewTriggeredAction:Struct {'struct': '0x1446f2880'}` |
| `/Script/FortniteUI.EmergencyNoticeBase:ShowNotice` | `0x8480800` | `Body:Text`, `Title:Text` |
| `/Script/FortniteUI.FortConfirmationWindow:ConfigureConfirmationButton` | `0x8480800` | `bSimpleConfirm:Bool`, `Action:Struct {'struct': '0x142b25040'}`, `Button:Object {'class': '0x140807800'}` |
| `/Script/FortniteUI.FortConfirmationWindow:ConfigureDeclineButton` | `0x8080800` | `Button:Object {'class': '0x140807800'}` |
| `/Script/FortniteUI.FortErrorWindow:OnOverrideCloseButtonText` | `0x8480800` | `OverrideText:Text` |
| `/Script/FortniteUI.FortLetoLayoutBorder:UpdateBorderWidth` | `0x8080800` | `HalfBorderWidth:Float` |
| `/Script/FortniteUI.FortRestartClientModal:OnButtonSet` | `0x8080800` | `bShouldShow:Bool`, `InbShouldExit:Bool` |
| `/Script/FortniteUI.FortToastDisplayArea:OnDisplayToast` | `0x8020800` | `NewToast:Object {'class': '0x142b26c60'}` |
| `/Script/FortniteUI.FortVoiceChannelChangePopup:OnInputModeChanged` | `0x8080800` | `NewInputType:Enum {'enum': '0x14081ee20'}`, `UnderlyingType:Byte` |
| `/Script/FortniteUI.FortWebLoginWidget:DisplayWidget` | `0xc020800` | `WebWidget:Object {'class': '0x143dc3650'}` |

Native `CommonActivatablePanel.OnBeginIntro` and `OnBeginOutro` are outside this batch: both require native behavior evidence.

All 11 decoded parameter tables have no return-property record, so the observed return is `void`. Source property types and current emittability assessment are:

- `OnActionProgress`: `float HeldPercent`; directly resolvable, UHT proof pending.
- `OnTriggeredInputActionChanged`: const-reference `FDataTableRowHandle NewTriggeredAction` (Engine struct); its Engine header/type-gate resolution needs verification.
- `ShowNotice`: const-reference `FText Title` and `FText Body`; source offsets 0 and 24 establish that order, UHT proof pending.
- `ConfigureConfirmationButton`: `UCommonButton* Button`, const-reference `FConfirmationDialogAction Action`, and `bool bSimpleConfirm`. The bool offset is absent from this decoded report, so its placement and the cross-module struct admission need original-byte/UHT verification.
- `ConfigureDeclineButton`: `UCommonButton* Button`; cross-module header/forward declaration and UHT proof pending.
- `OnOverrideCloseButtonText`: const-reference `FText OverrideText`; UHT proof pending.
- `UpdateBorderWidth`: `float HalfBorderWidth`; directly resolvable, UHT proof pending.
- `OnButtonSet`: `bool bShouldShow`, `bool InbShouldExit`; both decoded offsets are absent, so argument order needs primary-source confirmation.
- `OnDisplayToast`: `UFortUINotification* NewToast`; header/forward declaration and UHT proof pending.
- `OnInputModeChanged`: `ECommonInputType NewInputType`; this requires a narrowly source-evidenced CommonInput type-gate admission analogous to the splash event.
- `DisplayWidget`: `UWidget* WebWidget`; Engine class resolution and UHT proof pending.

These are an assessment, not generated signatures: none of the 11 was added in this batch. In particular, const-reference reflection flags require deliberate event handling; the general RPC parameter decoder alone does not establish the desired declaration.
