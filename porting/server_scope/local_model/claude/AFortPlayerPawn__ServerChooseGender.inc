// CONFIDENCE: HIGH. Original addresses: exec 0x142a7e810 (server RPC wrapper); validate 0x14234a140 (already ported as ServerChooseGender_Validate: Gender < 4); implementation 0x14234a0f0 (ported in AFortPlayerPawn__ServerChooseGender_Implementation.inc).
// The exec gates the implementation on the validate function and reports a validation failure otherwise.
if (!ServerChooseGender_Validate(Gender))
{
    RPC_ValidateFailed(TEXT("ServerChooseGender_Validate"));
    return;
}
ServerChooseGender_Implementation(Gender);
