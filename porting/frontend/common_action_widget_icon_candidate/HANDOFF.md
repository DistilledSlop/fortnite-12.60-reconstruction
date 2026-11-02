# CommonActionWidget.GetIcon: receiver and brush construction recovery

`GetIcon` is **not ready to integrate**. The maintained CommonActionBindingData::Find/Owner APIs supply the process registry; widget-specific key/owner lifecycle adapters and the CommonInput module/settings cache accessor still need integration. This directory records source-shaped behavior, not compilable stand-ins. No native, generator, build, or runtime changes were made.

Reflection class block `0x145a01470`, no-register `0x1408072e0`: GetIcon create `0x140807870`, matching candidate outer `0x140807790`, flags `0x54020401`, one return property at zero, struct getter `0x142fdb590` (SlateBrush). `native_funcs.json` joins the same create function to exec `0x14080a9f0`. The reflection name is ambiguous across classes, so use this outer/create/native join, not the name alone. Exec advances the VM and calls **receiver body `0x1407c2ee0`**, then assigns its temporary brush into the VM result and destroys it. `InputActions` is the array at widget +0x230.

Receiver `0x1407c2ee0` reads the widget's unreflected signed registry key at +0x240 and calls `0x1407bd730`. If its record exists and the weak reference at record +0x10 resolves, it calls virtual +0x290 on that target; otherwise it calls the same virtual on the widget. This virtual is `GetOwningLocalPlayer`; it passes that player to subsystem lookup `0x14081b480`, then unconditionally calls brush helper `0x1407c2f50(subsystem, InputActions)`. It does not check the subsystem or the array before that call. In particular, `GetOwningLocalPlayer()` on the widget alone is not equivalent. Registry creation, ownership, and weak-target lifecycle remain open; see `../common_action_display_text/HANDOFF.md`.

Source-shaped construction below uses descriptive operations at unresolved boundaries, not proposed substitute API names:

```cpp
// Receiver, after authentic registry lookup and weak resolution:
ULocalPlayer* Player = ResolvedRegistryTarget
    ? ResolvedRegistryTarget->GetOwningLocalPlayer()
    : GetOwningLocalPlayer();
UCommonInputSubsystem* Input = /* 0x14081b480 subsystem lookup for Player */;
return BuildActionBrush(Input, InputActions);

// BuildActionBrush, after authentic row and settings accessors exist:
TArray<FKey> Keys;
for (const FDataTableRowHandle& Handle : InputActions)
{
    const FCommonInputActionData* Row = /* typed GetRow, original context string */;
    if (!Row)
        return FSlateNoResource(); // original copies its function-local static
    const FCommonInputTypeInfo* Info = &Row->KeyboardInputTypeInfo;
    if (Input)
    {
        const auto Type = Input->GetCurrentInputType(); // 0x14081b540
        const auto Gamepad = /* 0x14081b530 on Input */;
        if (Type == ECommonInputType::Gamepad)
            Info = &Row->GamepadInputTypeInfos[static_cast<uint8>(Gamepad)];
        else if (Type == ECommonInputType::Touch)
            Info = &Row->TouchInputTypeInfo;
    }
    Keys.Add(Info->Key);
}
FSlateBrush Brush;
const auto Gamepad = /* 0x14081b530 on Input */;
const auto Type = Input->GetCurrentInputType(); // 0x14081b540
const FCommonInputPlatformData Platform = /* 0x14081b550: prepared settings/platform copy */;
if (FindCommonInputPlatformBrush(Platform, Keys, Type, Gamepad, Brush))
    return Brush;
return FSlateNoResource(); // original copies its function-local static
```

This sketch deliberately does not declare or stub unresolved APIs. The registry resolution above is itself conditional on a record existing. Row helper `0x14078c640` uses typed `GetRow` with context `CommonUIUtils::GetInputActionData couldn't find the row passed in, check data table if its missing.`; it can log and an unsuccessful row immediately returns the fallback. Selection invokes both getters once per successful row when subsystem exists, in type-then-gamepad order; gamepad indexing is unchecked. It copies only keys, preserving order and duplicates. After all rows (including an empty array), the subsystem is dereferenced without a null guard, in gamepad-then-type order, followed by `0x14081b550`; do not add a null guard or infer safety from the earlier lookup. The platform lookup is already integrated as `FindCommonInputPlatformBrush` with the same controller/key selection semantics. The exact production access path must retain settings preparation/cache side effects and return the copied platform data; a CDO, fabricated defaults or a no-op accessor is not equivalent. Current generated subsystem header exposes the current-input getter but not an authenticated accessible gamepad getter; integration needs the real source access for `0x14081b530` as well.

Fallback is function-local static `0x1407c3e30`, copy-constructed on both failures. Its constructor `0x140795550` calls the SlateBrush constructor with no-draw, NAME_None, zero margin/image size, no tile and no image, matching the engine `FSlateNoResource` constructor; use the real type and static lifecycle in production. The separate original brush proof covers selection and destructors: `/mnt/vibrato/fortnite-work/common-action-widget-brush-contract-1/HANDOFF.md`. The sketch is a contract, not a validated implementation, and does not cover refresh or SetInputAction.

Replay from repo root: `python3 porting/frontend/common_action_widget_icon_candidate/replay.py` — PASS, owner and ambiguous reflected record, native join, input property, original image SHA-256 and bounded byte windows for the entire exec, receiver, brush helper, fallback, and constructor. Disassembly used `python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py 14080a9f0:100 140807870:80` and `.../decode.py 1407c2ee0:80 14080aae0:b0 1407c3e30:82 14078c640:150`; helper getter calls inspected with `.../decode.py 1407c2f50:4c7`. No per-helper test, build, or commit was performed.
