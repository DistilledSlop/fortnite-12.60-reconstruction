// CONFIDENCE: high. Native target 0x1407d55f0.
int32 ButtonIndex = INDEX_NONE;
for (int32 Index = 0; Index < NativeButtonGroupWidgets.Num(); ++Index)
{
    if (NativeButtonGroupWidgets[Index].Get() == BaseButton)
    {
        ButtonIndex = Index;
        break;
    }
}
NativeHoveredButtonIndex = ButtonIndex;
if (ButtonIndex != INDEX_NONE)
{
    OnHoveredButtonChanged.Broadcast(BaseButton, ButtonIndex);
}
