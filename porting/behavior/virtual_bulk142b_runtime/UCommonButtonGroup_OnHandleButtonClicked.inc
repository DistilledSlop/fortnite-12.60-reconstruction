// CONFIDENCE: high. Native target 0x1407d67a0.
int32 ButtonIndex = INDEX_NONE;
for (int32 Index = 0; Index < NativeButtonGroupWidgets.Num(); ++Index)
{
    if (NativeButtonGroupWidgets[Index].Get() == BaseButton)
    {
        ButtonIndex = Index;
        break;
    }
}
if (ButtonIndex != INDEX_NONE)
{
    OnButtonClicked.Broadcast(BaseButton, ButtonIndex);
    NativeButtonClicked.Broadcast(BaseButton, ButtonIndex);
}
