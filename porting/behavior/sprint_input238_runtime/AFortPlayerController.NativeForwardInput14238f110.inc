// CONFIDENCE: HIGH
// Original 0x14238f110; full recovered body. CONFIDENCE: HIGH.
if (MyFortPawn && MyFortPawn->VehicleStateLocal.Vehicle &&
    Cast<IFortVehicleInterface>(MyFortPawn->VehicleStateLocal.Vehicle))
{
    if (MyFortPawn)
    {
        if (AFortAthenaVehicle* Athena = Cast<AFortAthenaVehicle>(MyFortPawn->VehicleStateLocal.Vehicle))
        {
            return Athena->NativeForwardSpeed141916010() > 1.e-8f && NativeSprintInputGate14238e8e0();
        }
        if (MyFortPawn)
        {
            if (AFortCharacterVehicle* CharacterVehicle = Cast<AFortCharacterVehicle>(MyFortPawn->VehicleStateLocal.Vehicle))
            {
                if (!CharacterVehicle->GetMovementComponent() || !CharacterVehicle->InputComponent)
                {
                    return false;
                }
                const FVector Forward = CharacterVehicle->GetRootComponent()
                    ? CharacterVehicle->GetRootComponent()->GetForwardVector() : FVector::ForwardVector;
                const FVector Input = CharacterVehicle->GetMovementComponent()->GetLastInputVector();
                return Forward.X * Input.X + Forward.Y * Input.Y + Forward.Z * Input.Z > 0.0f;
            }
        }
    }
}
if (MyFortPawn && MyFortPawn->GetMovementComponent() && MyFortPawn->InputComponent)
{
    const FVector Forward = MyFortPawn->GetRootComponent()
        ? MyFortPawn->GetRootComponent()->GetForwardVector() : FVector::ForwardVector;
    const FVector Input = MyFortPawn->GetMovementComponent()->GetLastInputVector();
    return Forward.X * Input.X + Forward.Y * Input.Y + Forward.Z * Input.Z > 0.0f;
}
return false;
