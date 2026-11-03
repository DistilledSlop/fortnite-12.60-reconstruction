// Definitions verified against original 1407ffc50 (manager tick) and the owner-getter
// slot bodies (global 140695430, panel 1408056ac -> 1406a7c40).
// Held tick bodies (global 140802440, panel 140802200) and dispatch slots are BLOCKED; see NOTES.md.

// Original 1407ffc50, entered through secondary tickable table 1459ed5f8 (this+0x28).
// Its direct-access fields are the manager fields listed in declarations.inc.
void UCommonInputManager::Tick(float DeltaSeconds)
{
    // Original: if the suspension branch ran, return without a held tick.
    // Otherwise, when both held-pair words are nonnull, call slot +28 with XMM1 unchanged.
    if (!NativeTickOperationSuspension(DeltaSeconds))
    {
        if (CurrentlyHeldActionInputHandler.Object != nullptr
            && CurrentlyHeldActionInputHandler.Interface != nullptr)
        {
            CurrentlyHeldActionInputHandler.Interface->NativeHeldTick(DeltaSeconds);
        }
    }
}

bool UCommonInputManager::NativeTickOperationSuspension(float DeltaSeconds)
{
    // Original: if fd == 0, jump to the held path (return false).
    if (!bNativeOperationSuspended)
    {
        return false;
    }

    // Original: while processing (fc) or any operation (count at f0) remains, the branch
    // returns and the idle counter (+100) is left unchanged.
    if (!bNativeOperationProcessing && NativeOperations.Num() == 0)
    {
        // Original: pre-increment of the dword at +100, then compare > 2.
        ++NativeIdleTickCount;
        if (NativeIdleTickCount > 2)
        {
            bNativeOperationSuspended = false;
            // Original 1407ffcb3: optional Verbose log via 142c54b70 when the category
            // verbosity byte is >= 6; message text not recovered.
            // Original tail-calls the multicast broadcast at +90 with dl = 0 (false).
            NativeInputSuspensionChanged.Broadcast(false);
        }
    }

    // The suspension branch ran. The held tick is never executed on this invocation.
    (void)DeltaSeconds; // XMM1 is not read by the suspension branch.
    return true;
}

// Owner getter, slot +08. Original global 140695430 is lea rax,[rcx-0x28]; ret.
// Original panel 1408056ac/1406a7c40 is lea rax,[rcx-0x2d8] composed through two
// adjustments. In this override `this` is already the concrete owner, so the
// compiler-generated secondary-base adjustor supplies the same -0x28 / -0x2d8.
UObject* UCommonGlobalInputHandler::NativeGetOwnerObject() const
{
    return const_cast<UCommonGlobalInputHandler*>(this);
}

UObject* UCommonActivatablePanel::NativeGetOwnerObject() const
{
    return const_cast<UCommonActivatablePanel*>(this);
}
