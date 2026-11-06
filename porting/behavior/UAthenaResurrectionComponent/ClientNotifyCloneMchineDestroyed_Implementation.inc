// CONFIDENCE: high
// Native target 0x141b3c840.
if (GetOwnerRole() == ROLE_AutonomousProxy && DestroyedSound)
{
    UGameplayStatics::PlaySound2D(GetOwner(), DestroyedSound, 1.0f, 1.0f, 0.0f, nullptr, nullptr);
}
