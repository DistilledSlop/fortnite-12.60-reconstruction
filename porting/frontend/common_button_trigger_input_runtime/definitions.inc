// Source for original 1407fa9e0, verified against image-125 window sha256 (95 bytes).
// Recovered order: full row compare (14418a080: DataTable ptr, FName Number, FName ComparisonIndex),
// return if equal; teardown 140801750 on the existing row; copy 16-byte row from incoming;
// virtual +488 (NativeBindTriggeringInputAction, setup 1407abbc0); virtual +4a0 (NativeUpdateActionWidget).
// Depends on NativeUnbind/NativeBind bodies, which are in the gated button-definitions.inc.
void UCommonButton::SetTriggeringInputAction(FDataTableRowHandle InputActionRow)
{
    if (TriggeringInputAction == InputActionRow) return;
    NativeUnbindTriggeringInputAction();
    TriggeringInputAction = InputActionRow;
    NativeBindTriggeringInputAction();
    NativeUpdateActionWidget();
}
