void UFortActivatablePanel::NativeHandleActivated()
{
    Super::NativeHandleActivated();
    bIsAlreadyOnContentPanelStack = bIsPushedOnToContentPanelStack;
}
