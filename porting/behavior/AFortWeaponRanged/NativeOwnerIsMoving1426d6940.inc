// CONFIDENCE: HIGH
// Original 0x1426d6940.
AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GetInstigator());
if (!PlayerPawn)
{
    return false;
}
UCharacterMovementComponent* Movement = PlayerPawn->GetCharacterMovement();
if (!Movement)
{
    UE_LOG(LogFort, Error, TEXT("AFortWeaponRanged::OwnerIsMoving() %s: The player pawn's movement component is null when it shouldn't be!"), *GetName());
    return false;
}
// Original 1440afb70 returns the reflected Acceleration vector at +22c.
const FVector Acceleration = Movement->GetCurrentAcceleration();
bool bMoving = Acceleration.X * Acceleration.X + Acceleration.Y * Acceleration.Y > 0.0f || PlayerPawn->bIsSlopeSliding;
if (bFirstShotAccuracyCheckVehicleMovement)
{
    AFortAthenaVehicle* Vehicle = PlayerPawn->NativeGetAthenaVehicle1423254c0();
    if (Vehicle)
    {
        const FVector VehicleVelocity = Vehicle->GetVelocity();
        const float MaxRestSpeed = static_cast<UFortPhysicsVehicleConfigs*>(Vehicle->NativeVehicleConfigs738)->MaxRestSpeed;
        bMoving = (FMath::Abs(VehicleVelocity.X) > MaxRestSpeed ||
                   FMath::Abs(VehicleVelocity.Y) > MaxRestSpeed ||
                   FMath::Abs(VehicleVelocity.Z) > MaxRestSpeed) || bMoving;
    }
}
return bMoving;
