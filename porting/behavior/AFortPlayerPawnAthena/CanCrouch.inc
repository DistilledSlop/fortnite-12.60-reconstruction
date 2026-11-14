// CONFIDENCE: HIGH. Complete original override; class lifecycle acceptance remains separate.
if (!ACharacter::CanCrouch())
{
    return false;
}
if (EmoteCount != 0)
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
return !IsInVehicle() && !bLocalIsSkydiving;
