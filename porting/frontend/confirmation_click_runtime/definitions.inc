// Definitions for the two recovered click handlers. Follows the original control flow.
// Unresolved callees are declared in declarations.inc and intentionally have no bodies here.

// Body 144f54030. Original order: GetOwningPlayer, IsA AFortPlayerControllerAthena,
// client-bot flags, latent/ordinary dispatch on window +0x570 and the button array.
void UFortConfirmationWindow::BotClickButton()
{
    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(GetOwningPlayer());
    if (!PlayerController)
        return;

    // Original test: byte +0x1838 or +0x1818 on the controller. Same expression as IsClientBot 142a47ff0.
    if (!PlayerController->bNativeClientBotFlag1838 && !PlayerController->bNativeClientBotFlag1818)
        return;

    if (NativeConfirmButtonInputActionCount570() > 0 && ConfirmationButtons.Num() > 0)
    {
        // Original tail-jumps to 144f5fbc0 with rdx = ConfirmationButtons[0].Button.
        NativeConfirmationButtonClicked144f5fbc0(ConfirmationButtons[0].Button);
        return;
    }

    // Original calls 144f5fce0 with rdx = raw +0x4a8 pointer (ignored by the callee).
    NativeDeclineButtonClicked144f5fce0(NativeDeclineButtonPtr4A8);
}

// Body 144f5f9f0. Original order: call 141ed81a0(true, byte +0x448); if byte +0x304 is set,
// tail-call vtable +0x4c0; otherwise tail-call 1407b7ed0, which returns unless byte +0x288 is set
// and then tail-calls vtable +0x4a8.
void UFortRestartClientModal::HandleButtonClicked()
{
    NativeRestartPlatformRequest141ed81a0(true, bNativeRestartFlag448);

    if (bNativeStackMembership304)
    {
        NativeDispatchSlot4C0();
        return;
    }

    // Fallback 1407b7ed0 (verified: cmpb 0x288, je ret, jmp *0x4a8).
    if (!bNativeFlag288)
        return;
    NativeDispatchSlot4A8();
}
