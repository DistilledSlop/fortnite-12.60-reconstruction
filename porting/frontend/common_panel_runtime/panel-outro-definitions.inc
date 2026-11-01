void UCommonActivatablePanel::NativeDeactivateRequest()
{
    if (!bNativeIsIntroed) return;
    NativeClearHeldAction();
    OnBeginOutro();
#if CSV_PROFILER
    {
        const FString Name = GetFName().ToString();
        FCsvProfiler::RecordEventf(CSV_CATEGORY_INDEX(Slate), TEXT("Deactivate: %s"), *Name);
    }
#endif
}
void UCommonActivatablePanel::OnBeginOutro_Implementation() { EndOutro(); }
void UCommonActivatablePanel::EndOutro()
{
    CommonActivationCore::EndOutro([this]{bNativeIsIntroed=false;},[this]{NativeOutroComplete();},
        [this](bool Value){bIsActive=Value;},[this]{NativeHandleDeactivated();});
}
void UCommonActivatablePanel::NativeHandleDeactivated()
{
    ULocalPlayer* Player=GetOwningLocalPlayer();
    UCommonInputSubsystem* Input=Player?Player->GetSubsystem<UCommonInputSubsystem>():nullptr;
    if(Input&&NativeInputMethodHandle.IsValid())Input->NativeInputMethodChanged.Remove(NativeInputMethodHandle);
    NativeClearHeldAction();
    NativePreDeactivation.ExecuteIfBound(this);
    Super::NativeHandleDeactivated();
    OnWidgetDeactivated.Broadcast(this);
    NativePostDeactivation.ExecuteIfBound(this);
}
void UCommonActivatablePanel::NativeClearHeldAction()
{
    if(!NativeInputManager.IsValid())return;
    const bool HasKey=NativeHeldAction.DataTable!=nullptr || NativeHeldAction.RowName!=NAME_None;
    // Original COMISS zero,duration; JAE skips ordered nonpositive values.
    if(HasKey&&!(NativeHeldActionDuration<=0.0f))
        if(auto* Entry=NativeActionRecords.Find(NativeHeldAction))Entry->NativeProgress.Broadcast(0.0f);
    NativeHeldActionDuration=0.0f;
    NativeHeldAction=FDataTableRowHandle();
    // Original performs a second weak-pointer resolve after progress broadcast.
    UCommonInputManager* Manager=NativeInputManager.Get();
    Manager->CurrentlyHeldActionInputHandler.SetObject(nullptr);
    Manager->CurrentlyHeldActionInputHandler.SetInterface(nullptr);
}
// Original primary vtable slot 4d8 -> 1406801c0: c2 00 00.
void UCommonActivatablePanel::NativeOutroComplete() {}
