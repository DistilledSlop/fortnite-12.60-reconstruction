// 1407ffd30, complete discovery prefix + queued repeat tail.
bool UCommonUIActionRouter::NativeTickActionRoutes(float DeltaTime)
{
    NativeTickActionRouteDiscovery();
    const uint8 InputMode = static_cast<uint8>(NativeActionInputConfig.IsSet()
        ? NativeActionInputConfig.GetValue().InputMode : ECommonInputMode::All);
    for (const FKey& Key : NativeQueuedActionKeys)
    {
        const auto Result = NativeStandaloneActionRoute->ProcessHoldInput(InputMode, Key, IE_Repeat);
        if (bNativeActionRoutingEnabled && NativeActiveRouteRoot.IsValid()
            && Result == ECommonActionHoldDispatchResult::Unhandled)
        {
            NativeActiveRouteRoot->ProcessHoldInput(InputMode, Key, IE_Repeat);
        }
    }
    return true;
}
