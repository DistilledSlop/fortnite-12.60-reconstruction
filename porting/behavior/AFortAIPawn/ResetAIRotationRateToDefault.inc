// CONFIDENCE: high
// Native helper in reflected wrapper 0x1427a57e0.
if (GetLocalRole() == ROLE_Authority)
{
    const AFortAIPawn* DefaultPawn = GetClass()->GetDefaultObject<AFortAIPawn>();
    const UCharacterMovementComponent* DefaultMovement = DefaultPawn->GetCharacterMovement();
    if (DefaultMovement)
    {
        CurrentAIRotationRate = DefaultMovement->RotationRate;
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->RotationRate = DefaultMovement->RotationRate;
        }
    }
}
