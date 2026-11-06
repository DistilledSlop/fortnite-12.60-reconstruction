// CONFIDENCE: HIGH
// Original exec 0x142a7cc20; native body 0x142325930.
if (!CurrentWaterBody)
{
    return -1.0f;
}
return static_cast<UFortMovementComp_Character*>(GetCharacterMovement())->NativeRecoveredFloat_0x1d08;
