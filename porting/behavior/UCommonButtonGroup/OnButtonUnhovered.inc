// CONFIDENCE: high. Native target 0x1407d5640.
if (NativeHoveredButtonIndex != INDEX_NONE)
{
    UCommonButton* Hovered = NativeButtonGroupWidgets.IsValidIndex(NativeHoveredButtonIndex) ? NativeButtonGroupWidgets[NativeHoveredButtonIndex].Get() : nullptr;
    if (Hovered == BaseButton)
    {
        NativeHoveredButtonIndex = INDEX_NONE;
        OnHoveredButtonChanged.Broadcast(nullptr, INDEX_NONE);
    }
}
