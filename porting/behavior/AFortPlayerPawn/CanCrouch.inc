// CONFIDENCE: HIGH. Complete original override; class lifecycle acceptance remains separate.
if (!ACharacter::CanCrouch())
{
    return false;
}
if (bIsDBNO)
{
    return false;
}
if (!GetCharacterMovement()->IsMovingOnGround())
{
    return false;
}
if (!GetCharacterMovement()->CanCrouchInCurrentState())
{
    return false;
}
if (!NativeCanPerformAction14230c260(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x698)))
{
    return false;
}
return !IsInVehicle() && !bLocalIsSkydiving;
