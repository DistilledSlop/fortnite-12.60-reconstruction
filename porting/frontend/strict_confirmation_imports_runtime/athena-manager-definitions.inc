// Original startup/Athena manager native IFortUIManagerInterface slot +0x40.
void UFortUIManager_Athena::NativeShowConfirmation(
    UFortLocalPlayer* Player, const FFortDialogDescription_NUI& Description)
{
    if (UFortRootViewportLayout_Athena** Root = PlayerWidgets.Find(Player))
        if (*Root)
            (*Root)->NativeAppendConfirmation145171a60(Description);
}
