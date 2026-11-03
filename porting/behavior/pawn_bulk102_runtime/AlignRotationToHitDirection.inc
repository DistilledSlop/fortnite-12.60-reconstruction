// CONFIDENCE: high
// Native helper 0x142303f70, reflected wrapper 0x142a1afb0.
const FRotator HitRotation = HitDirection.GetSafeNormal2D().Rotation();
AFortAIController* AIController = Cast<AFortAIController>(Controller);
if (!Controller || AIController)
{
    SetActorRotation(HitRotation);
    if (AIController)
    {
        AIController->SetControlRotation(HitRotation);
    }
}
