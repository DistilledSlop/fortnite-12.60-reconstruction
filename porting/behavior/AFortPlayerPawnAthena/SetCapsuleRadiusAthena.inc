// CONFIDENCE: high; native 0x141a23e40 gates on authority and an existing capsule.
if (HasAuthority())
{
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        CapsuleRadiusAthena = Radius;
        Capsule->SetCapsuleSize(Radius, Capsule->GetUnscaledCapsuleHalfHeight(), true);
    }
}
