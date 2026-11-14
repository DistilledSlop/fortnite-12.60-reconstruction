// 0x1427a2230: migrate the deprecated full-byte bool only while loading.
if (Ar.IsLoading()
    && ControlRecoveryBehavior == EFortControlRecoveryBehavior::DefaultControl
    && bAllowChainStun)
{
    ControlRecoveryBehavior = EFortControlRecoveryBehavior::ChainControl;
}
