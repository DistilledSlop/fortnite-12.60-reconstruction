// Additional router.cpp includes: Containers/Ticker.h, UCommonUISubsystem.h,
// UCommonUIInputSettings.h, UObject/UObjectGlobals.h and Input/Events.h.

void UCommonUIActionRouter::Initialize(FSubsystemCollectionBase& Collection)
{
    NativeInitializeActionRouteDiscovery(Collection);
    FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this,
        &UCommonUIActionRouter::NativeHandleActionBindingGarbageCollection);
    NativeActionInputPreProcessor = NativeCreateActionInputPreProcessor();
    auto* UISubsystem = GetLocalPlayer()->GetGameInstance()->GetSubsystem<UCommonUISubsystem>();
    UISubsystem->NativeLegacyInputManagerChanged.AddUObject(this,
        &UCommonUIActionRouter::NativeHandleLegacyInputManagerChanged);
    NativeHandleLegacyInputManagerChanged(UISubsystem->CommonInputManager != nullptr);
    FSlateApplication::Get().OnFocusChanging().AddUObject(this,
        &UCommonUIActionRouter::NativeHandleActionFocusChanging);
}
void UCommonUIActionRouter::NativeHandleLegacyInputManagerChanged(bool bHasLegacyManager)
{
    bNativeActionRoutingBlocked = bHasLegacyManager;
    if (bHasLegacyManager)
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(NativeActionInputPreProcessor);
        if (bNativeActionRoutingEnabled)
        {
            FTicker::GetCoreTicker().RemoveTicker(NativeActionRouteTicker);
            NativeSetActiveActionRouteRoot(nullptr);
        }
    }
    else
    {
        const auto* Settings = GetDefault<UCommonUIInputSettings>();
        FSlateApplication::Get().RegisterInputPreProcessor(NativeActionInputPreProcessor, Settings->AnalogCursorSettings.PreprocessorPriority);
        if (bNativeActionRoutingEnabled)
        {
            FTicker::GetCoreTicker().RemoveTicker(NativeActionRouteTicker);
            NativeActionRouteTicker = FTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateUObject(this, &UCommonUIActionRouter::NativeTickActionRoutes), 0.0f);
        }
    }
}
void UCommonUIActionRouter::Deinitialize()
{
    FSlateApplication::Get().OnFocusChanging().RemoveAll(this);
    FSlateApplication::Get().UnregisterInputPreProcessor(NativeActionInputPreProcessor);
    FTicker::GetCoreTicker().RemoveTicker(NativeActionRouteTicker);
    NativeSetActiveActionRouteRoot(nullptr);
    NativeQueuedActionKeys.Empty();
    // The original does not remove the weak-UObject-bound rebuilt, post-GC or
    // subsystem callbacks, reset the processor, or call a Super body here.
}
void UCommonUIActionRouter::NativeHandleActionBindingGarbageCollection()
{
    CommonActionBindingData::CompactInvalidBindings();
}
void UCommonUIActionRouter::NativeHandleActionFocusChanging(const FFocusEvent& Event,
    const FWeakWidgetPath& OldPath, const TSharedPtr<SWidget>& OldFocus,
    const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewFocus)
{
    (void)OldPath;
    (void)NewPath;
    (void)NewFocus;
    if (Event.GetCause() != EFocusCause::SetDirectly) return;
    const auto* Player = GetLocalPlayer();
    const int32 PlayerIndex = Player->GetGameInstance()->GetLocalPlayers().IndexOfByKey(Player);
    if (Event.GetUser() != static_cast<uint32>(PlayerIndex)) return;
    if (NativeActiveRouteRoot.IsValid() && NativeActiveRouteRoot->IsExclusiveSlateDescendant(OldFocus))
    {
        const auto Selected = NativeActiveRouteRoot->GetSelectedNode();
        if (Selected.IsValid()) Selected->SaveFocus();
    }
}
