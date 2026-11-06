// CONFIDENCE: high; native tests enabled bit before the reflected component and its proven active getter.
{
    if (!bIsCreativeQuickbarEnabled)
    {
        return false;
    }
    const UCreativeQuickbarComponent* Quickbar = CreativeQuickbarComponent;
    return Quickbar && Quickbar->IsQuickbarActive();
}
