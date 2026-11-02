UClass* UFortUIDataConfiguration::NativeGetLegacyStateClass260(uint8 Selector)
{
    // The original's alternate selector path calls 140680300, which returns
    // null unconditionally in this image, before continuing here.
    const FStateWidgetEntry& Entry = Selector == 4 && NativeGetGameMode141ed4e30() == 0
        ? CampaignFrontEndWidget : StateWidgetClasses[Selector];
    return NativeLoadStateClass144da33b0(Entry.Class, true);
}

UClass* UFortUIDataConfiguration::NativeGetHUDClass268()
{
    return NativeLoadHUDClass144c7d620(AthenaStateWidget_HUD, true);
}
