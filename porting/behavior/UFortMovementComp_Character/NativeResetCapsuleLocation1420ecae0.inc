// Original address: 0x1420ecae0
// CONFIDENCE: high
if (CharacterOwner)
{
    const ACharacter* DefaultChar = GetDefault<ACharacter>(CharacterOwner->GetClass());
    if (DefaultChar && DefaultChar->GetCapsuleComponent() && UpdatedComponent)
    {
        UpdatedComponent->SetRelativeLocation(DefaultChar->GetCapsuleComponent()->GetRelativeLocation());
    }
}
