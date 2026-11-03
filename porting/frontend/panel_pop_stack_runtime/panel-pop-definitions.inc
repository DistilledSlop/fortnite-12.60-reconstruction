// panel_pop_stack_runtime: definitions recovered from image-125. Fragment only, not compiled.
// Unresolved callees keep their binary address as the name. Verified control flow is in NOTES.md.

// 0x1407e1130: vtable +0x4c0 on UCommonActivatablePanel. Leaf thunk.
void UCommonActivatablePanel::<unresolved slot +0x4c0>()
{
    // 0x2ee7880 = FWeakObjectPtr::IsValid on this+0x3b0
    if (!NativeInputManager.IsValid())
    {
        return;
    }
    // 0x2ee5dd0 = FWeakObjectPtr::Get on this+0x3b0. The binary passes the result without a null check.
    UCommonInputManager* Manager = NativeInputManager.Get();
    Manager->PopActivatablePanel(this); // 0x1407e0c80
}

// 0x1407e0c80: UCommonInputManager::PopActivatablePanel(UCommonActivatablePanel* ActivatablePanel)
void UCommonInputManager::PopActivatablePanel(UCommonActivatablePanel* ActivatablePanel)
{
    // 0x1407ad3f0: predicted stack copy (legacy name NativePredictedPanelStack)
    const TArray<UCommonActivatablePanel*> Predicted = NativePredictedPanelStack();

    // The binary also scans ActivatablePanelStack (+0xb8, count +0xc0) into a flag (r12b). That flag only drives verbose logging.
    const int32 Index = Predicted.Find(ActivatablePanel);
    if (!ActivatablePanel || Index == INDEX_NONE)
    {
        // 0x1407e10b9 path: verbose logging, then return
        return;
    }

    // Walk predicted stack top-down. Each entry gets a Pop FOperation (Operation = 3, Panel = entry).
    // 0x2c9ff10 runs per entry (unresolved). 0x1407a95e0 receives the op and returns a panel pointer.
    // The loop breaks when that return equals the target. Inferred from one compare, not from source.
    for (int32 I = Predicted.Num() - 1; I >= 0; --I)
    {
        // 0x2c9ff10 (unresolved, per-entry helper)
        FOperation Pop;
        Pop.Operation = EOperation::Pop;
        Pop.Panel = Predicted[I];
        const UCommonActivatablePanel* Returned = Native_0x1407a95e0(Pop); // unresolved name, clean-operation helper
        if (Returned == ActivatablePanel)
        {
            break;
        }
    }

    // Index > 0: enqueue Intro for predicted[Index-1] (op type 0 at 0x1407e0ff3)
    if (Index > 0)
    {
        Operations.Add(CommonLegacyPanelQueue::Make(EOperation::Intro, Predicted[Index - 1], true, true, true));
    }

    // 0x1407e104b: start processing only when not already processing (0xe0 byte).
    // No suspension check appears here in the binary.
    if (!bNativeProcessingOperations)
    {
        NativeBeginOperationProcessing(); // 0x1407ab920
    }
}
