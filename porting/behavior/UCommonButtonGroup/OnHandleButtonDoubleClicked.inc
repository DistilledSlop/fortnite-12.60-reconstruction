// CONFIDENCE: high. Native target 0x1407d6800.
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
    OnButtonDoubleClicked.Broadcast(BaseButton, ButtonIndex);
}
