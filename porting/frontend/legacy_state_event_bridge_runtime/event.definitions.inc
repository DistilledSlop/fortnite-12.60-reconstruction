// 145160ac0 -> 1453181c0, original slot 510.
void UFortUIStateWidget_NUI::NativeActivateLegacyState510(uint8 PreviousState)
{
    OnEnterState(static_cast<EFortUIState>(PreviousState));
}

// 145160ec0 -> 145318200, original slot 518.
void UFortUIStateWidget_NUI::NativeDeactivateLegacyState518(uint8 NextState)
{
    OnExitState(static_cast<EFortUIState>(NextState));
}
