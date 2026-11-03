// CONFIDENCE: high; native 0x141a23e00 gates on authority and an existing capsule.
if (HasAuthority())
{
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        CapsuleHalfHeightAthena = HalfHeight;
        Capsule->SetCapsuleSize(Capsule->GetUnscaledCapsuleRadius(), HalfHeight, true);
    }
}
