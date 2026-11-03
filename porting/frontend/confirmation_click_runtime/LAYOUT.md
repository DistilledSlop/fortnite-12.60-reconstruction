# Layout facts for confirmation_click_runtime (NOTES.md blockers 1, 2, 4, 5, 6, 8)

Scope: offsets and ownership only. No build, no run, no native or generator edits. Reflection source: `porting/generated/reflection/fortnite_game_reflection.json` (class entries are indexed in `classes_in_groups_with_fort_names`; class names are `null` in the JSON, so classes were identified by property and function names). Header source: `porting/native/Source/...` generated headers.

Tags: **verified** = read directly from the cited data. **inferred** = derived from layout arithmetic or naming, not checked against disassembly.

## Class identities (reflection index)

| Class | Index | Evidence | Tag |
|---|---|---|---|
| UFortConfirmationWindow | 5769 | props `ConfirmationDescription` 1200, `DeclineButton` 1192, `ConfirmationButtons` 1176, `EntryBox_SimpleButtons` 1096; funcs `ConfigureConfirmationButton`, `ConfigureDeclineButton` | verified |
| UFortActivatablePanel | 5585 | props `PrimaryContentSetup` 1052, `InputPriority` 1050 (Enum); bools are bitfields (offset None) | verified |
| UFortRestartClientModal | 6356 | 0 reflected props; funcs `OnButtonSet`, `HandleButtonClicked`; header declares no members | verified |
| UCommonActivatablePanel | 90 | props `OnWidgetActivated` 736, `OnWidgetDeactivated` 752 (16 bytes each); bitfields `bShouldBypassStack`, `bExposeActionsExternally`, `bConsumeAllActions` | verified |
| UCommonActivatableWidget | 91 | props `ActivatedVisibility` 606, `DeactivatedVisibility` 608, `BP_OnWidgetActivated` 616, `BP_OnWidgetDeactivated` 632 (16 bytes each, so ends at 648 = 0x288); bitfields incl. `bIsActive` | verified |
| UCommonUserWidget | 143 | prop `bConsumePointerInput` (bitfield); header notes non-reflected field at 0x230 | verified |

Parent chain (from headers): UFortRestartClientModal -> UFortActivatablePanel -> UCommonActivatablePanel -> UCommonActivatableWidget -> UCommonUserWidget -> UUserWidget. The reflection `base_fn` field is shared across unrelated classes and was not used.

## Blocker 1: window +0x570 (int32)

- `ConfirmationDescription` (FFortDialogDescription_NUI) is at window +1200 (0x4B0). Verified (reflection).
- Inside that struct, `ConfirmButtonInputActions` is `TArray<FConfirmationDialogAction>` at struct offset 184 (0xB8). Verified (header `FFortDialogDescription_NUI.h`, offsets from reflection).
- Window +1200 + 184 = +1384 (0x568). A TArray is pointer then int32 `ArrayNum` then int32 `ArrayMax` (see `ArrayNum`/`ArrayMax` members in UE 4.25 `Containers/Array.h`). The `ArrayNum` slot is therefore +8, which gives window +0x570.
- Result: **window +0x570 is `ConfirmationDescription.ConfirmButtonInputActions.ArrayNum`** (the element count). The pointer-first layout is inferred from the standard FScriptArray shape, not from disassembly.
- Not the same as `ConfirmationButtons`, which is a separate TArray at window +1176 (0x498), with its count at +0x4A0.
- The `ShowAdvancedLatentActionConfirmation` function param at offset 192 (reflection, class 5636) is a function frame offset, not the struct layout. It was not used as evidence.

## Blocker 2: window +0x4A8 (raw button pointer)

- Reflection `DeclineButton`, kind Object, offset 1192 (0x4A8), flags 0x40000000082008 (Transient). Verified.
- Type: `UCommonButton*` (header `UFortConfirmationWindow.h` declares `UCommonButton* DeclineButton`). Verified.
- Writer: not determined from these sources. Still open.

## Blocker 4: window vtable +0x288 (GetOwningPlayer)

- Not resolved in this pass. The reflection for class 3843 has a `GetOwningPlayer` function, but its owning class and vtable slot are not linked to the window in the JSON. Slot index would be 0x288 / 8 = 81 (inferred from the 8-byte pointer size).
- Status: **unverified**. Do not treat the NOTES claim that `GetOwningPlayer` is the base override as confirmed.

## Blockers 5 and 6: restart modal byte +0x288 and vtable slots

### Byte +0x288 (648)

- Modal has no reflected or header-declared members (class 6356), so the byte is in a parent.
- UFortActivatablePanel's first reflected member is at 1050, so the byte is below it.
- UCommonActivatableWidget's last reflected member `BP_OnWidgetDeactivated` ends at 648 (0x288). Its bitfield `bIsActive` is declared last (reflection table index 0). **Inferred:** byte +0x288 is the storage byte of `UCommonActivatableWidget::bIsActive`. This is the most likely owner. The alternative, `UCommonActivatablePanel`'s first non-reflected member (for example `bNativeIsIntroed`), is possible if the Widget's bitfield shares a byte with other storage. This was not checked with disassembly.
- Semantic fit: the fallback at 1407b7ed0 (`cmpb $0, 0x288(%rcx); je ret; jmp *0x4a8(%rax)`, verified in NOTES.md) returns when the byte is zero, which matches an `IsActivated`-style gate.

### Byte +0x304 (772)

- The panel's reflected `OnWidgetDeactivated` ends at 768 (0x300). UFortActivatablePanel's first reflected member is at 1050.
- So +0x304 lies in the non-reflected tail of **UCommonActivatablePanel** (between 768 and 1050). **Inferred.** The header `UCommonActivatablePanel.h` has non-reflected members (`bNativeIsIntroed`, `NativeInputManager`, `NativeInputMethodHandle`, `NativePreActivation`, `NativePostActivation`, `NativeHeldActionDuration`, `NativeHeldAction`, `NativeActionBindingRows`, `NativeActionRecords`, `NativePreDeactivation`, `NativePostDeactivation`). Their exact offsets are not given, and the header's declaration order does not reproduce the 768 to 1050 gap, so the specific member is **unresolved**.

### Vtable slots (8-byte slots, slot = offset / 8)

- Anchors from headers (verified as text in the headers): `UCommonUserWidget::OnWidgetRebuilt` at original +0x2b0 (slot 86), `NativeDestruct` at original +0x320 (slot 100), `UCommonActivatableWidget` `NativeDestruct` at +0x320.
- Modal +0x288 = slot 81, +0x4A8 = slot 149, +0x4C0 = slot 152.
- The UE 4.25 virtual order for UUserWidget and UCommonActivatableWidget was not reconstructed here. The slot names are therefore **unverified**.
- Carried from NOTES.md (not re-checked here): +0x4C0 is the shipping target 1407e1130. +0x4A8 has no name in the notes.
- Next step if needed: objdump the modal vtable (or the call sites in body 144f5f9f0) to match slots 149 and 152 to the known UCommonActivatablePanel overrides (`NativeDeactivateRequest`, `NativeHandleDeactivated`, and similar). This was not done.

## Blocker 8: byte +0x304 membership

- Membership is tied to the operation queue (see NOTES.md). The layout result above fixes the owner as UCommonActivatablePanel (inferred), not the modal. The specific member is unresolved.

## Summary

| Item | Result | Tag |
|---|---|---|
| Window +0x570 | ConfirmationDescription.ConfirmButtonInputActions.ArrayNum (int32) | verified (struct offsets), inferred (ArrayNum at +8) |
| Window +0x4A8 | DeclineButton, UCommonButton* | verified |
| Modal +0x288 | Likely UCommonActivatableWidget::bIsActive storage byte | inferred |
| Modal +0x304 | Non-reflected tail of UCommonActivatablePanel, member unresolved | inferred |
| Window vtable +0x288 | Not resolved | unverified |
| Modal vtable +0x288, +0x4A8, +0x4C0 | Not resolved (+0x4C0 carried from NOTES) | unverified |
